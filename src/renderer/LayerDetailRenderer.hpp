#pragma once
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <string>
#include "renderer/detail/ConvDetailBase.hpp"

class LayerDetailRenderer {
public:
    LayerDetailRenderer() ;

    // 加载图层详细背景图
    bool loadTexture(const std::string& layerName, const std::string& texturePath);
    
    // 设置弹窗显示状态
    void setVisible(const std::string& layerName, bool visible);
    bool isVisible(const std::string& layerName) const;
    
    // 绘制弹窗
    void draw();
    
    // 获取弹窗是否刚刚打开（用于初始化）
    bool justOpened(const std::string& layerName) const;

    // 检查是否有详细窗口打开
    bool isAnyWindowOpen() const;

    // 创建详细交互器
    void createDetailRenderer(const std::string& layerName);

    // 检测鼠标位置
    void handleMouse(const sf::Vector2f& mousePos);

    // 处理按钮点击
    void handleButtons();

private:
    struct LayerDetail {
        sf::Texture texture;
        bool visible = false;
        bool justOpened = false;
        std::string title;
        std::unique_ptr<ConvDetailBase> detailRenderer;

        // 保存窗口信息
        ImVec2 windowPos;
        ImVec2 windowSize;
        ImVec2 contentSize;
        ImVec2 imagePos;
    };
    
    std::unordered_map<std::string, LayerDetail> layers_;
    
    void drawDetailWindow(const std::string& layerName, LayerDetail& detail);
};