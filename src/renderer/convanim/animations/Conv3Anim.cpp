#include "Conv3Anim.hpp"
#include <iostream>

Conv3Anim::Conv3Anim() {
    
    inputWidth = 16;
    inputHeight = 16;
    padInputWidth = 18;
    padInputHeight = 18;
    kernelCount = 1;
    outputWidth = 16;
    outputHeight = 16;
    
    kernelFrameTex.create(padInputWidth, padInputHeight);
    kernelTex.create(kernelSize, kernelSize);
    outputTex.create(outputWidth, outputHeight);
}



bool Conv3Anim::load(const std::string& modelDir) {
    std::cout << "=== 加载Conv3动画 ===" << std::endl;
    
    if (!loadLayerInput(modelDir)) {
        return false;
    }
    
    std::string weightPath = modelDir + "/weights.bin";
    if (!loadKernelWeights(weightPath, getWeightOffset(), 32)) {
        return false;
    }
    
    calculateOutput();
    refreshTextures();
    
    std::cout << "Conv3动画加载完成" << std::endl;
    return true;
}

bool Conv3Anim::loadLayerInput(const std::string& modelDir) {
    std::string inputPath = modelDir + "/m_ustc_conv2_output.bin";
    std::cout << "  加载: " << inputPath << std::endl;
    
    // conv2输出: 32×16×16，只取第一个通道
    return loadSingleChannel(inputPath, 16, 16, 32, 0);
}