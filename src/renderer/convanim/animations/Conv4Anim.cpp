#include "Conv4Anim.hpp"
#include <iostream>

Conv4Anim::Conv4Anim() {
    
    inputWidth = 8;
    inputHeight = 8;
    padInputWidth = 10;
    padInputHeight = 10;
    kernelCount = 1;
    outputWidth = 8;
    outputHeight = 8;

    kernelFrameTex.create(padInputWidth, padInputHeight);
    kernelTex.create(kernelSize, kernelSize);
    outputTex.create(outputWidth, outputHeight);
}


bool Conv4Anim::load(const std::string& modelDir) {
    std::cout << "=== 加载Conv4动画 ===" << std::endl;
    
    if (!loadLayerInput(modelDir)) {
        return false;
    }
    
    std::string weightPath = modelDir + "/weights.bin";
    if (!loadKernelWeights(weightPath, getWeightOffset(), 64)) {
        return false;
    }
    
    calculateOutput();
    refreshTextures();
    
    std::cout << "Conv4动画加载完成" << std::endl;
    return true;
}

bool Conv4Anim::loadLayerInput(const std::string& modelDir) {
    std::string inputPath = modelDir + "/m_ustc_conv3_output.bin";
    std::cout << "  加载: " << inputPath << std::endl;
    
    // conv3输出: 64×8×8，只取第一个通道
    return loadSingleChannel(inputPath, 8, 8, 64, 0);
}