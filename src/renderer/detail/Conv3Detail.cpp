#include "renderer/detail/Conv3Detail.hpp"
#include <iostream>

bool Conv3Detail::initialize() {
    initializeHotspots();
    std::cout << "conv3热点交互初始化完成，热点数量: " << hotspots_.size() << std::endl;
     
     // 创建动画器
        animator = std::make_unique<Conv3Anim>();
        
        // 加载数据
        bool success = animator->load("/workspace/assets/model");
        
        std::cout << "conv3热点交互初始化完成" 
                  << (success ? " (包含动画)" : " (动画加载失败)") << std::endl;

    return true;
}

void Conv3Detail::initializeHotspots() {
    hotspots_.clear();  // 清空现有热点

    hotspots_ = {
        {
            "conv3_kernel", 
            "卷积核: 64组3×3滤波器\n每组处理32个输入通道",
            sf::FloatRect(0.31f, 0.31f, 0.15f, 0.32f)
        },
        {
            "batchnorm + activation",
            "批量归一化 + ReLU激活", 
            sf::FloatRect(0.50f, 0.31f, 0.15f, 0.32f)
        },
        {
            "pooling",
            "池化层: 2×2最大池化",
            sf::FloatRect(0.68f, 0.35f, 0.10f, 0.20f)
        },
        
    };
}

std::string Conv3Detail::getButtonText(const std::string& hotspotName) const {
    // 根据热点名称返回对应的按钮文本
    if (hotspotName == "conv3_kernel") {
        return "卷积动画";
    } else if (hotspotName == "batchnorm + activation") {
        return "归一化与ReLU激活";
    } else if (hotspotName == "pooling") {
        return "池化动画";
    }
    return "按钮";
}

void Conv3Detail::drawHotspots(ImVec2 contentSize, ImVec2 imagePos) {

    
    /*
    // 绘制热点区域（调试用）
    for (const auto& hotspot : hotspots_) {
        drawHotspot(hotspot, contentSize, imagePos);
    }*/
    

    // 绘制按钮（在热点区域中央）
    for (const auto& hotspot : hotspots_) {
        if (hotspot.hovered) {
            drawButton(hotspot, contentSize, imagePos);
        }
    }

    // 绘制工具提示
    for (const auto& hotspot : hotspots_) {
        if (hotspot.hovered) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", hotspot.name.c_str());
            ImGui::Separator();
            ImGui::Text("%s", hotspot.description.c_str());
            ImGui::EndTooltip();
            break;
        }
    }
}

void Conv3Detail::drawHotspot(const Hotspot& hotspot, ImVec2 contentSize, ImVec2 imagePos) {

    std::cout << "Conv3Detail::drawHotspots called. Hotspot count: " << hotspots_.size() << std::endl;

    // 将百分比坐标转换为实际像素坐标
    ImVec2 pos(
        imagePos.x + hotspot.area.left * contentSize.x,
        imagePos.y + hotspot.area.top * contentSize.y
    );
    ImVec2 size(
        hotspot.area.width * contentSize.x,
        hotspot.area.height * contentSize.y
    );
    
    // 绘制半透明热点区域（仅调试显示）
    if (1){//hotspot.hovered) {
        ImGui::GetWindowDrawList()->AddRectFilled(
            ImVec2(pos.x, pos.y),
            ImVec2(pos.x + size.x, pos.y + size.y),
            IM_COL32(255, 255, 0, 60)
        );
        
        // 绘制边框
        ImGui::GetWindowDrawList()->AddRect(
            ImVec2(pos.x, pos.y),
            ImVec2(pos.x + size.x, pos.y + size.y),
            IM_COL32(255, 255, 0, 200),
            2.0f, 0, 3.0f
        );
    }
}

void Conv3Detail::handleMouse(const sf::Vector2f& mousePos, ImVec2 contentSize, ImVec2 imagePos) {
    // 重置悬停状态
    for (auto& hotspot : hotspots_) {
        hotspot.hovered = false;
    }
    
    // 检查鼠标是否在热点区域内
    for (auto& hotspot : hotspots_) {
        // 计算热点区域的屏幕坐标
        float left = imagePos.x + hotspot.area.left * contentSize.x;
        float top = imagePos.y + hotspot.area.top * contentSize.y;
        float right = left + hotspot.area.width * contentSize.x;
        float bottom = top + hotspot.area.height * contentSize.y;
        
        if (mousePos.x >= left && mousePos.x <= right &&
            mousePos.y >= top && mousePos.y <= bottom) {
            hotspot.hovered = true;
            //std::cout << "悬停在热点: " << hotspot.name << std::endl;
            break; // 一次只悬停一个热点
        }
    }
}

void Conv3Detail::drawButton(const Hotspot& hotspot, ImVec2 contentSize, ImVec2 imagePos) {
    // 计算热点区域的中心位置
    float centerX = imagePos.x + (hotspot.area.left + hotspot.area.width / 2) * contentSize.x;
    float centerY = imagePos.y + (hotspot.area.top + hotspot.area.height / 2) * contentSize.y;
    
    // 按钮大小
    float buttonWidth = 120.0f;
    float buttonHeight = 30.0f;
    
    // 计算按钮位置（居中）
    float buttonLeft = centerX - buttonWidth / 2;
    float buttonTop = centerY - buttonHeight / 2;
    
    // 设置按钮位置
    ImGui::SetCursorScreenPos(ImVec2(buttonLeft, buttonTop));
    
    // 获取按钮文本
    std::string buttonText = getButtonText(hotspot.name);
    
    // 绘制按钮
    if (ImGui::Button(buttonText.c_str(), ImVec2(buttonWidth, buttonHeight))){
        std::cout << "点击按钮: " << std::endl;
        
        // 根据按钮类型触发不同功能
        if (hotspot.name == "conv3_kernel") {
            //std::cout << "打开卷积动画窗口" << std::endl;
            showAnimation = true;
        }
        else if (hotspot.name == "batchnorm + activation") {
            std::cout << "打开归一化与ReLU激活窗口" << std::endl;
            // 这里后续实现激活函数演示
        }
        else if (hotspot.name == "pooling") {
            std::cout << "打开池化动画窗口" << std::endl;
            // 这里后续实现池化动画
        }
    }
}

void Conv3Detail::handleButtons() {
    
    if (showAnimation) {
            static float deltaTime = 0.0f;
            deltaTime = 0.016f;  // 模拟60fps
            ConvAnimPanel::show("卷积动画窗口", &showAnimation, *animator, deltaTime);
        }
}