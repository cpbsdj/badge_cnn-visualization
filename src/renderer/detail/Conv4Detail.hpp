#pragma once
#include "renderer/detail/ConvDetailBase.hpp"
#include "renderer/convanim/ConvAnimBase.hpp"
#include "renderer/convanim/animations/Conv4Anim.hpp"
#include "renderer/convanim/ConvAnimPanel.hpp"
#include <vector>

class Conv4Detail : public ConvDetailBase {
public:
    bool initialize() override;
    void drawHotspots(ImVec2 contentSize, ImVec2 imagePos) override;
    void handleMouse(const sf::Vector2f& mousePos, ImVec2 contentSize, ImVec2 imagePos) override;

    void handleButtons() override;
    
    std::string getLayerName() const override { return "conv4"; }
    std::string getDescription() const override { 
        return "第四卷积层详细结构: 64→64通道, 3×3卷积核, ReLU激活, 最大池化2×2"; 
    }
    size_t getHotspotCount() const override { return hotspots_.size(); }
    
private:
    struct Hotspot {
        std::string name;
        std::string description;
        sf::FloatRect area;  // 百分比坐标
        bool hovered = false;
    };
    
    std::vector<Hotspot> hotspots_;
    void initializeHotspots();
    void drawButton(const Hotspot& hotspot, ImVec2 contentSize, ImVec2 imagePos);
    void drawHotspot(const Hotspot& hotspot, ImVec2 contentSize, ImVec2 imagePos);

    std::string getButtonText(const std::string& hotspotName) const;

    //卷积动画
    std::unique_ptr<Conv4Anim> animator;
    bool showAnimation = false;
};