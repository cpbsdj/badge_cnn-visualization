#pragma once
#include "MultiChannelConvAnim.hpp"

class Conv2Anim : public MultiChannelConvAnim {
public:
    Conv2Anim();

    std::string getTitle() const override { return "Conv2 卷积层动画"; }
    std::string getDescription() const override { 
        return "输入: 32×32, 输出: 32×32\n只显示第一个卷积核的第一个输入通道"; 
    }
    
    virtual bool load(const std::string& modelDir) override;
    /*
    / conv1权重: 16×1×3×3 = 144
    // conv1偏置: 16
    // conv2权重: 32×16×3×3 = 4608  
    // conv2偏置: 32
    // conv3权重: 64×32×3×3 = 18432
    // conv3偏置: 64
    */
    virtual int getWeightOffset() const override { return 144 + 16 ; }
    virtual std::string getLayerName() const override { return "Conv2"; }
    
protected:
    virtual bool loadLayerInput(const std::string& modelDir) override;
};