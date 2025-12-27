#pragma once
#include "renderer/convanim/ConvAnimBase.hpp"
#include <vector>
#include <SFML/Graphics.hpp>

class Conv1Anim : public ConvAnimBase {
public:
    Conv1Anim();

    bool load(const std::string& modelDir) override;
    void play() override;
    void pause() override;
    void reset() override;
    void step() override;
    void update(float dt) override;
    bool isPlaying() const override { return playing; }
    std::string getTitle() const override { return "第一卷积层动画"; }
    std::string getDescription() const override { 
        return "卷积核: 3×3, 输入: 64×64, 输出: 64×64\n通道: 1→16, stride=1"; 
    }
    
    // 获取纹理
    const sf::Texture& getInputTexture() const override { return inputTex; }
    const sf::Texture& getKernelTexture() const override { return kernelTex; }
    const sf::Texture& getOutputTexture() const override { return outputTex; }
    const sf::Texture& getKernelFrameTexture() const override { return kernelFrameTex; }
    
    // 获取当前参数
    float getDotProduct() const override;
    int getCurrentX() const override { return currentX; }
    int getCurrentY() const override { return currentY; }
    int getInputWidth() const override { return inputWidth; }
    int getInputHeight() const override { return inputHeight; }
    int getKernelSize() const override { return kernelSize; }
    int getOutputWidth() const override { return outputWidth; }
    int getOutputHeight() const override { return outputHeight; }
    
    // 获取数据
    const std::vector<float>& getKernelWeights() const override { return kernel; }
    std::vector<float>& getKernelWeights() { return kernel; }
    const std::vector<float>& getInputData() const override { return input; }
    const std::vector<float>& getOutputData() const override { return output; }
    

    // 设置播放速度
    void setAnimationSpeed(float speed) override { 
        if (speed > 0) {
            animationSpeed = speed;
            frameDuration = 1.0f / speed;  // 速度转换为帧间隔
        }
    }
    float getAnimationSpeed() const override { return animationSpeed; }

    // 卷积核选择相关
    int getNumKernels() const override { return kernelCount; }
    int getKernelIndex() const override { return currentKernelIndex; }
    void setKernelIndex(int index) override;
    

protected:
    // 数据
    std::vector<float> input;      // 输入特征图
    std::vector<float> kernel;     // 卷积核权重
    std::vector<float> output;     // 输出特征图
    std::vector<float> paddedInput;

    // 纹理
    sf::Texture inputTex;          // 输入纹理
    sf::Texture kernelTex;         // 卷积核纹理
    sf::Texture outputTex;         // 输出纹理
    sf::Texture kernelFrameTex;    // 卷积核边框纹理
    
    // 动画状态
    bool playing = false;
    float timer = 0.0f;
    float frameDuration = 0.1f;    // 每帧持续时间(秒)
    float animationSpeed = 10.0f;  // 动画速度(帧/秒)
    int currentX = 0;              // 当前卷积位置x
    int currentY = 0;              // 当前卷积位置y
    
    // 尺寸
    int inputWidth = 64;
    int inputHeight = 64;
    int padInputWidth = 66;
    int padInputHeight = 66;
    int kernelSize = 3;
    int outputWidth = 64;
    int outputHeight = 64;
    int kernelCount = 16;
    
   //辅助方法
    bool loadWeights(const std::string& weightPath);
    bool loadUstcImage(const std::string& imagePath);
    void loadInputData(const std::string& inputPath);
    void loadOutputData(const std::string& outputPath);
    void refreshTextures();
    void refreshKernelTexture();
    void refreshOutputTexture();
    void refreshKernelFrameTexture();
    void createTestWeights();
    void calculateOutput();

    // 移动卷积核到下一个位置
    void moveToNextPosition();
    
    // 计算当前点积
    float calculateDotProduct() const;
    
    // 像素数据转纹理
    void binToTexture(const std::vector<float>& data, 
                     int width, int height, 
                     sf::Texture& texture);



    int currentKernelIndex = 0;                  // 当前选择的卷积核索引
    std::vector<std::vector<float>> allKernels;  // 存储所有16个卷积核
    
    // 更新当前卷积核
    void updateCurrentKernel();
};