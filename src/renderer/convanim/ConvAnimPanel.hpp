#pragma once
#include "renderer/convanim/animations/Conv1Anim.hpp"
#include "renderer/convanim/animations/Conv2Anim.hpp"
#include "renderer/convanim/animations/Conv3Anim.hpp"
#include "renderer/convanim/animations/Conv4Anim.hpp"
#include <imgui.h>
#include <SFML/Graphics.hpp>
#include <string>
#include <memory>

class ConvAnimPanel {
public:
    // 创建动画实例
    static std::unique_ptr<ConvAnimBase> createAnimator(int layer);
    
    // 绘制动画面板
    static void show(const std::string& title, bool* open, 
                     ConvAnimBase& anim, float& deltaTime);
    
private:
    static void showInputWindow(ConvAnimBase& anim, const char* id);
    static void showKernelWindow(ConvAnimBase& anim, const char* id);
    static void showOutputWindow(ConvAnimBase& anim, const char* id);
    static void showControls(ConvAnimBase& anim, const char* id);
};