#pragma once
#include "MultiChannelConvAnim.hpp"

class Conv3Anim : public MultiChannelConvAnim {  
public:
    Conv3Anim();

    std::string getTitle() const override { return "Conv3 卷积层动画"; }
    std::string getDescription() const override { 
        return "输入: 16×16, 输出: 16×16\n只显示第一个卷积核的第一个输入通道"; 
    }
    
    virtual bool load(const std::string& modelDir) override;
    virtual int getWeightOffset() const override { return 144 + 16 + 4608 + 32; }
    virtual std::string getLayerName() const override { return "Conv3"; }
    
protected:
    virtual bool loadLayerInput(const std::string& modelDir) override;
};