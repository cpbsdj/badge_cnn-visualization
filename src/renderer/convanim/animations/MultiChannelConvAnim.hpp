#pragma once
#include "renderer/convanim/animations/Conv1Anim.hpp"

class MultiChannelConvAnim : public Conv1Anim {
public:
    virtual bool load(const std::string& modelDir) override = 0;
    virtual int getWeightOffset() const = 0;       // 权重偏移量
    virtual std::string getLayerName() const = 0;  // 层名
    
protected:
    virtual bool loadLayerInput(const std::string& modelDir) = 0;
    
    // 加载多通道数据中的单个通道
    bool loadSingleChannel(const std::string& filepath, int width, int height, int totalChannels, int channel = 0);
    
    // 加载卷积核权重（第一个核的第一个输入通道）
    bool loadKernelWeights(const std::string& weightPath, int offset, int kernelChannels = 1);

    bool createPaddedInput(int width, int height);
};