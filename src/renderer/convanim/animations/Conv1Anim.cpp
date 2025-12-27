#include "renderer/convanim/animations/Conv1Anim.hpp"
#include <fstream>
#include <iostream>
#include <cmath>

Conv1Anim::Conv1Anim() {
    int padding = 1;        // padding=1
    kernelSize = 3;         // 卷积核大小
    int stride = 1;         // stride=1
    int poolSize = 2;       // 池化大小
    int poolStride = 2;     // 池化步长

    inputWidth = 64;    // 输入宽度
    inputHeight = 64;   // 输入高度
    padInputWidth = inputWidth + 2*padding;   // 带padding的输入宽度
    padInputHeight = inputHeight + 2*padding; // 带padding的输入高度

    int inputChannels = 1;   // 输入通道
    kernelCount = 16;        // 卷积核数量
    int kernelChannels = 1;  // 卷积核通道数
    currentKernelIndex = 0;
    
    int convOutSize = 64;
    outputWidth = 64;
    outputHeight = 64;
    
    std::vector<float>& weights = getKernelWeights();
    weights.resize(kernelCount * kernelChannels * kernelSize * kernelSize);
    
    // 初始化纹理
    inputTex.create(padInputWidth, padInputHeight);
    kernelTex.create(kernelSize, kernelSize);
    outputTex.create(outputWidth, outputHeight);
    kernelFrameTex.create(padInputWidth, padInputHeight);
    kernel.resize(kernelSize * kernelSize);
}

bool Conv1Anim::load(const std::string& modelDir) {
   std::cout << "=== 加载Conv1动画数据 (校徽分类器) ===" << std::endl;
    std::cout << "模型目录: " << modelDir << std::endl;
    
    // 1. 加载校徽图片
    std::string imagePath = "/workspace/python/data/mol_ustc_test/ustc.jpg";
    std::cout << "加载校徽图片: " << imagePath << std::endl;
    if (!loadUstcImage(imagePath)) {
        std::cerr << "无法加载校徽图片" << std::endl;
        return false;
    }
    
    // 2. 加载权重
    std::string weightPath = modelDir + "/weights.bin";
    std::cout << "加载权重: " << weightPath << std::endl;
    if (!loadWeights(weightPath)) {
        std::cerr << "无法加载权重" << std::endl;
        return false;
    }
    
    // 3. 计算输出特征图
    calculateOutput();
    
    // 4. 生成纹理
    refreshTextures();
    
    std::cout << "Conv1动画加载完成" << std::endl;
    std::cout << "  输入尺寸: " << inputWidth << "×" << inputHeight << " 灰度图" << std::endl;
    std::cout << "  卷积核: " << kernelCount << "个" << kernelSize << "×" << kernelSize << " 滤波器" << std::endl;
    std::cout << "  输出: " << kernelCount << "个" << outputWidth << "×" << outputHeight << " 特征图" << std::endl;
    
    return true;
}


bool Conv1Anim::loadUstcImage(const std::string& imagePath) {
    sf::Image image;
    if (!image.loadFromFile(imagePath)) return false;
    
    sf::Vector2u originalSize = image.getSize();
    
    // 灰度化
    sf::Image grayscaleImage;
    grayscaleImage.create(originalSize.x, originalSize.y);
    for (unsigned int y = 0; y < originalSize.y; ++y) {
        for (unsigned int x = 0; x < originalSize.x; ++x) {
            sf::Color pixel = image.getPixel(x, y);
            uint8_t gray = (pixel.r + pixel.g + pixel.b) / 3;
            grayscaleImage.setPixel(x, y, sf::Color(gray, gray, gray));
        }
    }
    
    // 使用SFML纹理缩放
    sf::Texture texture;
    texture.loadFromImage(grayscaleImage);
    
    // 计算缩放比例（保持宽高比）
    float scale = 64.0f / std::min(originalSize.x, originalSize.y);
    unsigned int scaledWidth = originalSize.x * scale;
    unsigned int scaledHeight = originalSize.y * scale;
    
    sf::RenderTexture renderTexture;
    renderTexture.create(scaledWidth, scaledHeight);
    
    sf::Sprite sprite(texture);
    sprite.setScale(scale, scale);
    renderTexture.draw(sprite);
    renderTexture.display();
    
    sf::Image scaledImage = renderTexture.getTexture().copyToImage();
    
    // 中心裁剪到64×64（不带padding的原始数据）
    unsigned int startX = (scaledWidth > 64) ? (scaledWidth - 64) / 2 : 0;
    unsigned int startY = (scaledHeight > 64) ? (scaledHeight - 64) / 2 : 0;
    
    // 存储原始64×64数据（不带padding）
    input.resize(64 * 64);
    for (unsigned int y = 0; y < 64; ++y) {
        for (unsigned int x = 0; x < 64; ++x) {
            sf::Color pixel = scaledImage.getPixel(startX + x, startY + y);
            float gray = pixel.r / 255.0f;            // [0,1]
            input[y * 64 + x] = gray * 2.0f - 1.0f;   // [-1,1]
        }
    }
    
    // 创建带padding的输入数据（66×66）
    paddedInput.resize(padInputHeight * padInputWidth, 0.0f);
    
    // 将原始数据放入中心，四周填充0
    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 64; ++x) {
            int paddedX = x + 1;
            int paddedY = y + 1;
            paddedInput[paddedY * padInputWidth + paddedX] = input[y * 64 + x];
        }
    }
    
    // 现在已经改为使用KernelFrameTexture而不是inputTex
    std::vector<float> displayInput = paddedInput;
    for (auto& val : displayInput) val = (val + 1.0f) * 0.5f;
    binToTexture(displayInput, padInputWidth, padInputHeight, inputTex);
    
    return true;
}


bool Conv1Anim::loadWeights(const std::string& weightPath) {
    std::ifstream file(weightPath, std::ios::binary);
    if (!file) {
        std::cerr << "无法打开权重文件: " << weightPath << std::endl;
        createTestWeights();
        return true;
    }
    
    // 读取权重数量
    uint32_t numWeights = 0;
    file.read(reinterpret_cast<char*>(&numWeights), sizeof(uint32_t));
    std::cout << "权重数量: " << numWeights << std::endl;
    
    // conv1 权重: 16(输出通道) × 1(输入通道) × 3 × 3 = 144
    int conv1WeightCount = 16 * 1 * 3 * 3;
    if (numWeights < conv1WeightCount) {
        std::cerr << "权重数量不足" << std::endl;
        file.close();
        createTestWeights();
        return true;
    }
    
    // 读取所有conv1权重
    std::vector<float> allConv1Weights(conv1WeightCount);
    file.read(reinterpret_cast<char*>(allConv1Weights.data()), conv1WeightCount * sizeof(float));
    file.close();
    
    std::cout << "conv1权重: " << conv1WeightCount << " 个参数" << std::endl;
    
    // 将权重分组存储到16核
    int kernelSize2d = kernelSize * kernelSize;
    allKernels.resize(16, std::vector<float>(kernelSize2d));
    
    for (int k = 0; k < 16; ++k) {
        int startIdx = k * kernelSize2d;
        std::copy(
            allConv1Weights.begin() + startIdx,
            allConv1Weights.begin() + startIdx + kernelSize2d,
            allKernels[k].begin()
        );
    }
    
    // 设置当前卷积核为第一个
    setKernelIndex(0);
    
    std::cout << "已" << allKernels.size() << "个卷积核" << std::endl;

    return true;
}

// 暂时未使用，保留接口
void Conv1Anim::loadInputData(const std::string& inputPath) {
    std::ifstream file(inputPath, std::ios::binary);
    if (!file) {
        std::cerr << "无法打开输入文件: " << inputPath << std::endl;
        
        // 创建测试输入（棋盘格）
        input.resize(inputWidth * inputHeight);
        for (int y = 0; y < inputHeight; ++y) {
            for (int x = 0; x < inputWidth; ++x) {
                // 棋盘格图案
                float value = ((x / 4 + y / 4) % 2) ? 1.0f : 0.0f;
                input[y * inputWidth + x] = value;
            }
        }
        std::cout << "使用测试输入" << std::endl;
        return;
    }
    
    // 读取输入数据
    input.resize(inputWidth * inputHeight);
    file.read(reinterpret_cast<char*>(input.data()), input.size() * sizeof(float));
    file.close();

    // 创建带padding的输入数据（66×66）
    paddedInput.resize(padInputHeight * padInputWidth, 0.0f);
    
    // 将原始数据放入中心，四周填充0
    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 64; ++x) {
            int paddedX = x + 1;
            int paddedY = y + 1;
            paddedInput[paddedY * padInputWidth + paddedX] = input[y * 64 + x];
        }
    }
    // 打印输入数据范围（调试用）
    /*
    float minVal = *std::min_element(input.begin(), input.end());
    float maxVal = *std::max_element(input.begin(), input.end());
    std::cout << "输入数据范围: [" << minVal << ", " << maxVal << "]" << std::endl;*/
}

void Conv1Anim::createTestWeights() {
    std::cout << "创建测试卷积核..." << std::endl;
    int firstKernelSize = 3 * 3;  // 9
    kernel.resize(firstKernelSize);
    
    float testKernel[9] = {
        -1, 0, 1,
        -2, 0, 2,
        -1, 0, 1
    };
    
    std::copy(testKernel, testKernel + 9, kernel.begin());
    
    std::cout << "测试卷积核创建完成" << std::endl;
}

// 暂时未使用，保留接口
void Conv1Anim::loadOutputData(const std::string& outputPath) {
    std::ifstream file(outputPath, std::ios::binary);
    if (!file) {
        std::cerr << "无法打开输出文件: " << outputPath << std::endl;
        
        // 创建输出
        output.resize(outputWidth * outputHeight);
        for (int y = 0; y < outputHeight; ++y) {
            for (int x = 0; x < outputWidth; ++x) {
                float sum = 0.0f;
                for (int ky = 0; ky < kernelSize; ++ky) {
                    for (int kx = 0; kx < kernelSize; ++kx) {
                        int ix = x + kx;
                        int iy = y + ky;
                        sum += paddedInput[iy * padInputWidth + ix] * kernel[ky * kernelSize + kx];
                    }
                }
                output[y * outputWidth + x] = sum;
            }
        }
        std::cout << "使用模拟输出" << std::endl;
        return;
    }
    
    // 读取输出数据
    output.resize(outputWidth * outputHeight);
    file.read(reinterpret_cast<char*>(output.data()), output.size() * sizeof(float));
    
    // 打印输出数据范围
    float minVal = *std::min_element(output.begin(), output.end());
    float maxVal = *std::max_element(output.begin(), output.end());
    std::cout << "输出数据范围: [" << minVal << ", " << maxVal << "]" << std::endl;
}

void Conv1Anim::setKernelIndex(int index) {
    if (index >= 0 && index < static_cast<int>(allKernels.size())) {
        currentKernelIndex = index;
        kernel = allKernels[index];  // 更新当前卷积核
        std::cout << "切换到卷积核 #" << (index + 1) << std::endl;
        
        // 重新计算输出
        calculateOutput();
        refreshTextures();
    } else {
        std::cerr << "无效的卷积核索引: " << index << std::endl;
    }
}

void Conv1Anim::updateCurrentKernel() {
    if (!allKernels.empty() && currentKernelIndex < static_cast<int>(allKernels.size())) {
        kernel = allKernels[currentKernelIndex];
    }
}

void Conv1Anim::calculateOutput() {
    output.resize(outputWidth * outputHeight, 0.0f);
    
    std::cout << "计算卷积..." << std::endl;
    std::cout << "使用卷积核 #" << (currentKernelIndex + 1) << " 计算输出..." << std::endl;
    
    // 计算每个输出位置（在66×66的输入上滑动）
    for (int y = 0; y < outputHeight; ++y) {
        for (int x = 0; x < outputWidth; ++x) {
            float sum = 0.0f;

            // 在带padding的输入上计算卷积
            for (int ky = 0; ky < kernelSize; ++ky) {
                for (int kx = 0; kx < kernelSize; ++kx) {
                    int inputX = x + kx;
                    int inputY = y + ky;
                    sum += paddedInput[inputY * padInputWidth + inputX] * kernel[ky * kernelSize + kx];
                }
            }
            output[y * outputWidth + x] = sum;
        }
    }
}


void Conv1Anim::play() { playing = true; }
void Conv1Anim::pause() { playing = false; }
void Conv1Anim::reset() { 
    currentX = 0; 
    currentY = 0; 
    timer = 0.0f;
    refreshTextures();
}

// 添加单步执行函数
void Conv1Anim::step() {
    if (!playing) {           // 只有在暂停状态下才能单步
        moveToNextPosition();
        refreshTextures();
    } else {
        std::cout << "播放状态下无法单步执行" << std::endl;
    }
}

// 移动卷积核到下一个位置
void Conv1Anim::moveToNextPosition() {
    // 移动到下一个位置
    currentX++;
    if (currentX >= outputWidth) {
        currentX = 0;
        currentY++;
        if (currentY >= outputHeight) {
            currentY = 0;
            //std::cout << "卷积完成一轮" << std::endl;
        }
    }
}

void Conv1Anim::update(float dt) {
    if (!playing) return;
    
    timer += dt;
    if (timer < frameDuration) return;
    timer -= frameDuration;
    
    // 移动到下一个位置
    moveToNextPosition();
    
    // 更新纹理
    refreshTextures();
}


void Conv1Anim::refreshTextures() {
    // 更新输入纹理（高亮当前卷积区域）
    refreshKernelFrameTexture();
    
    // 更新卷积核纹理
    refreshKernelTexture();
    
    // 更新输出纹理
    refreshOutputTexture();
}

void Conv1Anim::refreshKernelFrameTexture() {
    if (paddedInput.empty()) {
        std::cerr << "paddedInput未初始化" << std::endl;
        return;
    }
    
    std::vector<sf::Uint8> pixels(padInputWidth * padInputHeight * 4, 0);

    // 计算最小最大值用于归一化
    float minVal = *std::min_element(paddedInput.begin(), paddedInput.end());
    float maxVal = *std::max_element(paddedInput.begin(), paddedInput.end());
    float range = maxVal - minVal;
    
    // 复制带padding的输入数据到纹理
    for (int y = 0; y < padInputHeight; ++y) {
        for (int x = 0; x < padInputWidth; ++x) {
            size_t idx = y * padInputWidth + x;
            if (idx >= paddedInput.size()) {
                continue;
            }
            float val = paddedInput[idx];
            uint8_t gray = range > 0 ? 
                static_cast<uint8_t>(((val - minVal) / range) * 255) : 128;

            int pixelIdx = (y * padInputWidth + x) * 4;
            if (pixelIdx + 3 < pixels.size()) {
                pixels[pixelIdx] = gray;      // R
                pixels[pixelIdx + 1] = gray;  // G
                pixels[pixelIdx + 2] = gray;  // B
                pixels[pixelIdx + 3] = 255;   // A
            }
        }
    }
    
    // 高亮当前卷积区域（添加边界检查）
    for (int dy = 0; dy < kernelSize; ++dy) {
        for (int dx = 0; dx < kernelSize; ++dx) {
            int x = currentX + dx;
            int y = currentY + dy;
            
            if (x >= 0 && x < padInputWidth && y >= 0 && y < padInputHeight) {
                int idx = (y * padInputWidth + x) * 4;
                if (idx + 3 < pixels.size()) {
                    pixels[idx] = 255;      // 黄色高亮
                    pixels[idx + 1] = 255;  
                    pixels[idx + 2] = 0;    
                    pixels[idx + 3] = 255;  // 不透明
                }
            }
        }
    }
    
    kernelFrameTex.update(pixels.data());
}

void Conv1Anim::refreshKernelTexture() {
    int displayWidth = kernelSize;
    int displayHeight = kernelSize;
    
    std::vector<sf::Uint8> pixels(displayWidth * displayHeight * 4, 0);

    // 计算最小最大值用于归一化
    float minVal = *std::min_element(paddedInput.begin(), paddedInput.end());
    float maxVal = *std::max_element(paddedInput.begin(), paddedInput.end());
    float range = maxVal - minVal;
    
    for (int y = 0; y < kernelSize; ++y) {
        for (int x = 0; x < kernelSize; ++x) {
            // 覆盖的输入像素
            int inputX = currentX + x;
            int inputY = currentY + y;
            float inputVal = 0.0f;
            inputVal = paddedInput[inputY * padInputWidth + inputX];

            uint8_t inputGray = range > 0 ? 
                static_cast<uint8_t>(((inputVal - minVal) / range) * 255) : 128;
        
            int idx = (y * displayWidth + x) * 4;
            pixels[idx] = inputGray;      // R
            pixels[idx + 1] = inputGray;  // G
            pixels[idx + 2] = inputGray;  // B
            pixels[idx + 3] = 255;        // A
        }
    }
    
    // 更新纹理尺寸
    if (kernelTex.getSize().x == 0) {
        kernelTex.create(displayWidth, displayHeight);
    }
    kernelTex.update(pixels.data());
}

void Conv1Anim::refreshOutputTexture() {
    std::vector<sf::Uint8> pixels(outputWidth * outputHeight * 4, 0);
    
    // 计算最小最大值用于归一化
    float minVal = *std::min_element(output.begin(), output.end());
    float maxVal = *std::max_element(output.begin(), output.end());
    float range = maxVal - minVal;
    
    for (int y = 0; y < outputHeight; ++y) {
        for (int x = 0; x < outputWidth; ++x) {
            float val = output[y * outputWidth + x];
            uint8_t gray = range > 0 ? 
                static_cast<uint8_t>(((val - minVal) / range) * 255) : 128;
            
            int idx = (y * outputWidth + x) * 4;
            pixels[idx] = gray;      // R
            pixels[idx + 1] = gray;  // G
            pixels[idx + 2] = gray;  // B
            pixels[idx + 3] = 255;   // A
            
            // 高亮当前计算位置
            if (x == currentX && y == currentY) {
                pixels[idx] = 255;      // 红色
                pixels[idx + 1] = 255;  // 绿色
                pixels[idx + 2] = 0;    // 无蓝色
            }
        }
    }
    
    outputTex.update(pixels.data());
}

float Conv1Anim::calculateDotProduct() const {
    float sum = 0.0f;
    for (int ky = 0; ky < kernelSize; ++ky) {
        for (int kx = 0; kx < kernelSize; ++kx) {
            int ix = currentX + kx;
            int iy = currentY + ky;
            sum += kernel[ky * kernelSize + kx] * paddedInput[iy * padInputWidth + ix];
        }
    }
    return sum;
}

float Conv1Anim::getDotProduct() const {
    return calculateDotProduct();
}

// 暂时未使用，保留接口
void Conv1Anim::binToTexture(const std::vector<float>& data, 
                           int width, int height, 
                           sf::Texture& texture) {
    std::vector<sf::Uint8> pixels(width * height * 4, 0);
    
    // 归一化到0-1
    float minVal = *std::min_element(data.begin(), data.end());
    float maxVal = *std::max_element(data.begin(), data.end());
    float range = maxVal - minVal;
    
    for (int i = 0; i < width * height; ++i) {
        float val = data[i];
        uint8_t gray = range > 0 ? 
            static_cast<uint8_t>(((val - minVal) / range) * 255) : 128;
        
        pixels[i * 4] = gray;      // R
        pixels[i * 4 + 1] = gray;  // G
        pixels[i * 4 + 2] = gray;  // B
        pixels[i * 4 + 3] = 255;   // A
    }
    
    texture.update(pixels.data());
}