#pragma once
#include "MultiChannelConvAnim.hpp"

class Conv4Anim : public MultiChannelConvAnim {
public:
    Conv4Anim();
    
    std::string getTitle() const override { return "Conv4 卷积层动画"; }
    std::string getDescription() const override { 
        return "输入: 8×8, 输出: 8×8\n只显示第一个卷积核的第一个输入通道"; 
    }

    virtual bool load(const std::string& modelDir) override;
    
    virtual int getWeightOffset() const override { return 144 + 16 + 4608 + 32 + 18432 + 64; }
    virtual std::string getLayerName() const override { return "Conv4"; }
    
protected:
    virtual bool loadLayerInput(const std::string& modelDir) override;
};