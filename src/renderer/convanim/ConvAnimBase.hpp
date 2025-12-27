#pragma once
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <vector>
#include <string>

class ConvAnimBase {
public:
    virtual ~ConvAnimBase() = default;
    
    // 加载数据
    virtual bool load(const std::string& modelDir) = 0;
    
    // 动画控制
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void reset() = 0;
    virtual void update(float dt) = 0;
    
    virtual void step() = 0;                           // 单步执行
    virtual void setAnimationSpeed(float speed) = 0;  
    virtual float getAnimationSpeed() const = 0;
           
    // 获取动画状态
    virtual bool isPlaying() const = 0;
    virtual std::string getTitle() const = 0;
    virtual std::string getDescription() const = 0;
    
    // 获取纹理
    virtual const sf::Texture& getInputTexture() const = 0;
    virtual const sf::Texture& getKernelTexture() const = 0;
    virtual const sf::Texture& getOutputTexture() const = 0;
    virtual const sf::Texture& getKernelFrameTexture() const = 0;
    
    // 获取当前参数
    virtual float getDotProduct() const = 0;
    virtual int getCurrentX() const = 0;
    virtual int getCurrentY() const = 0;
    virtual int getInputWidth() const = 0;
    virtual int getInputHeight() const = 0;
    virtual int getKernelSize() const = 0;
    virtual int getOutputWidth() const = 0;
    virtual int getOutputHeight() const = 0;
    
    // 获取卷积权重
    virtual const std::vector<float>& getKernelWeights() const = 0;
    virtual const std::vector<float>& getInputData() const = 0;
    virtual const std::vector<float>& getOutputData() const = 0;

    // 卷积核选择相关
    virtual int getNumKernels() const = 0;       // 获取卷积核数量
    virtual int getKernelIndex() const = 0;      // 获取当前索引
    virtual void setKernelIndex(int index) = 0;  // 设置卷积核索引
    
    
};