#include "renderer/convanim/animations/MultiChannelConvAnim.hpp"
#include <fstream>
#include <iostream>
    
bool MultiChannelConvAnim::loadSingleChannel(const std::string& filepath, 
                                            int width, int height, 
                                            int totalChannels, int channel) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        std::cout << "无法打开文件: " << filepath << std::endl;
        return false;
    }
    
    int totalSize = width * height * totalChannels;
    size_t expectedSize = totalSize * sizeof(float);
    std::vector<float> allData(totalSize);
    
    if (!file.read(reinterpret_cast<char*>(allData.data()), expectedSize)) {
        std::cout << "读取文件失败" << std::endl;
        return false;
    }
    file.close();
    
    input.resize(width * height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int srcIdx = (y * width + x) * totalChannels + channel;
            int dstIdx = y * width + x;
            if (srcIdx < totalSize && dstIdx < input.size()) {
                input[dstIdx] = allData[srcIdx];
            }
        }
    }
    
    // 创建带padding的输入数据
    return createPaddedInput(width, height);
}

bool MultiChannelConvAnim::createPaddedInput(int width, int height) {
    paddedInput.resize(padInputHeight * padInputWidth, 0.0f);
    
    // 将原始数据放入中心，四周填充0
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int paddedX = x + 1;
            int paddedY = y + 1;
            paddedInput[paddedY * padInputWidth + paddedX] = input[y * width + x];
        }
    }
    
    return true;
}

bool MultiChannelConvAnim::loadKernelWeights(const std::string& weightPath, 
                                            int offset, int kernelChannels) {
    std::ifstream file(weightPath, std::ios::binary);
    if (!file) {
        std::cout << "无法打开权重文件" << std::endl;
        return false;
    }
    
    // 跳过文件头和前offset个权重
    uint32_t numWeights = 0;
    file.read(reinterpret_cast<char*>(&numWeights), sizeof(uint32_t));
    file.seekg(offset * sizeof(float), std::ios::cur);
    
    // 读取第一个核的第一个通道（3×3=9个权重）
    kernel.resize(9);
    file.read(reinterpret_cast<char*>(kernel.data()), 9 * sizeof(float));
    
    std::cout << "加载卷积核权重" << std::endl;
    return true;
}