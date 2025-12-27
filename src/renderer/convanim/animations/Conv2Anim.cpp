#include "Conv2Anim.hpp"
#include <iostream>

Conv2Anim::Conv2Anim() {
    
    
    inputWidth = 32;
    inputHeight = 32;
    padInputWidth = 34;
    padInputHeight = 34;
    kernelCount = 1;  // 只显示一个核
    outputWidth = 32;
    outputHeight = 32;
    
    kernelFrameTex.create(padInputWidth, padInputHeight);
    kernelTex.create(kernelSize, kernelSize);
    outputTex.create(outputWidth, outputHeight);
}



bool Conv2Anim::load(const std::string& modelDir) {
    std::cout << "=== 加载Conv2动画 ===" << std::endl;
    
    // 1. 加载输入（conv1的输出，只取第一个通道）
    if (!loadLayerInput(modelDir)) {
        std::cerr << "加载输入失败" << std::endl;
        return false;
    }
    
    // 2. 加载权重（第一个核的第一个输入通道）
    std::string weightPath = modelDir + "/weights.bin";
    if (!loadKernelWeights(weightPath, getWeightOffset(), 16)) {
        std::cerr << "加载权重失败" << std::endl;
        return false;
    }
    
    // 3. 计算输出
    calculateOutput();
    refreshTextures();
    
    std::cout << "Conv2动画加载完成" << std::endl;
    return true;
}

bool Conv2Anim::loadLayerInput(const std::string& modelDir) {
    std::string inputPath = modelDir + "/m_ustc_conv1_output.bin";
    std::cout << "  加载: " << inputPath << std::endl;
    
    // conv1输出: 16×32×32，只取第一个通道
    return loadSingleChannel(inputPath, 32, 32, 16, 0);
}