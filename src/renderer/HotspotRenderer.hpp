#pragma once
#include "loader/ModelLoader.hpp"
#include "renderer/LayerDetailRenderer.hpp"
#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include <unordered_map>

class HotspotRenderer {
public:
    // 设置窗口
    void setWindow(sf::RenderWindow* w) { window = w; }

    // 功能接口
    void build(const ModelLoader& modelLoader, const sf::Sprite& backgroundSprite);
    // 检测鼠标位置
    void handleMouse(const sf::RenderWindow& win);
    
    // 绘制功能
    void draw(sf::RenderTarget& tgt);
    void drawImGuiTooltip();
    void handleMouseAndDrawUI();
    
    // 热点交互状态
    bool isHotspotHovered() const { return hoveredHotspot != nullptr; }
    const std::string& getHoveredHotspotName() const { 
        return hoveredHotspot ? *hoveredHotspot : emptyString;
    }

    // 数据成员
    sf::RenderWindow* window = nullptr;

    // 设置详细渲染器的方法
    void setLayerDetailRenderer(LayerDetailRenderer* renderer) { 
        layerDetailRenderer_ = renderer; 
    }
    
    // 是否处理主热点交互（用于禁用主热点时）
    bool shouldHandleMainHotspots() const;

private:
    // 热点形状数据：热点名称 + 形状
    std::vector<std::pair<std::string, sf::ConvexShape>> hotspotShapes;
    
    // 当前悬停的热点名称指针
    const std::string* hoveredHotspot = nullptr;
    
    // 空字符串引用（用于返回默认值）
    static inline std::string emptyString = "";
    
    // 热点对应的层信息（用于显示详细信息）
    std::unordered_map<std::string, const Layer*> hotspotToLayer;
    
    // 热点描述信息
    std::unordered_map<std::string, std::string> hotspotDescriptions;
    
    // 创建矩形热点形状
    sf::ConvexShape createRectShape(const std::vector<sf::Vector2f>& pts, const sf::Transform& transform);
    
    // 创建多边形热点形状
    sf::ConvexShape createPolyShape(const std::vector<sf::Vector2f>& pts, const sf::Transform& transform);
    
    // 绘制详细结构按钮
    void drawDetailButton(const std::string& hotspotName, const sf::ConvexShape& shape);
    
    // 当前悬停的热点名称
    std::string currentHoveredHotspot_;

    // 图层详细信息渲染窗口
    LayerDetailRenderer* layerDetailRenderer_ = nullptr;
    std::unordered_map<std::string, bool> showDetailButtons_;  // 控制按钮显示
};