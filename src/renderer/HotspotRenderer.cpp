#include "renderer/HotspotRenderer.hpp"
#include <iostream>

void HotspotRenderer::build(const ModelLoader& modelLoader, const sf::Sprite& backgroundSprite) {
    hotspotShapes.clear();
    hotspotToLayer.clear();
    hotspotDescriptions.clear();
    
    // 获取热点数据
    const auto& hotspots = modelLoader.get_all_hotspots();
    const auto& layers = modelLoader.layers;
    
    // 构建热点形状
    for (const auto& [name, hotspot] : hotspots) {
        sf::ConvexShape shape;
        
        if (hotspot.type == "rect" && hotspot.pts.size() >= 4) {
            shape = createRectShape(hotspot.pts, backgroundSprite.getTransform());
        } else if (hotspot.type == "poly" && !hotspot.pts.empty()) {
            shape = createPolyShape(hotspot.pts, backgroundSprite.getTransform());
        } else {
            continue;
        }
        
        // 设置样式（调试用）
        shape.setFillColor(sf::Color(0, 0, 0, 0));
        shape.setOutlineColor(sf::Color(0,0,0,0));
        shape.setOutlineThickness(0.0f);
        
        hotspotShapes.emplace_back(name, std::move(shape));
        hotspotDescriptions[name] = hotspot.description;
        
        // 关联热点和对应的层
        for (const auto& layer : layers) {
            if (layer.name.find(name) != std::string::npos) {
                hotspotToLayer[name] = &layer;
                break;
            }
        }
    }
    
    std::cout << "构建了 " << hotspotShapes.size() << " 个热点区域" << std::endl;

    // 初始化详细按钮显示状态
    showDetailButtons_["conv1"] = true;
    showDetailButtons_["conv2"] = true; 
    showDetailButtons_["conv3"] = true;
    showDetailButtons_["conv4"] = true;
}

sf::ConvexShape HotspotRenderer::createRectShape(const std::vector<sf::Vector2f>& pts, const sf::Transform& transform) {
    sf::ConvexShape rect(4);
    for (size_t i = 0; i < 4; ++i) {
        rect.setPoint(i, transform.transformPoint(pts[i]));
    }
    return rect;
}

sf::ConvexShape HotspotRenderer::createPolyShape(const std::vector<sf::Vector2f>& pts, const sf::Transform& transform) {
    sf::ConvexShape poly(pts.size());
    for (size_t i = 0; i < pts.size(); ++i) {
        poly.setPoint(i, transform.transformPoint(pts[i]));
    }
    return poly;
}

void HotspotRenderer::handleMouse(const sf::RenderWindow& win) {
    hoveredHotspot = nullptr;
    auto mousePos = sf::Mouse::getPosition(win);
    sf::Vector2f worldPos = win.mapPixelToCoords(mousePos);

    if (!shouldHandleMainHotspots()) {
        return;
    }
    
    // 检查鼠标是否在热点区域内
    for (const auto& [name, shape] : hotspotShapes) {
        if (shape.getGlobalBounds().contains(worldPos)) {
            if (shape.getPointCount() > 0) {
                hoveredHotspot = &name;
                currentHoveredHotspot_ = name; // 记录当前悬停的热点
                break;
            }
        }
    }
    // 如果没有悬停在任何热点上，清空记录
    if (!hoveredHotspot) {
        currentHoveredHotspot_.clear();
    }
}


bool HotspotRenderer::shouldHandleMainHotspots() const {
    // 详细窗口打开时不处理主界面热点
    if (layerDetailRenderer_ && layerDetailRenderer_->isAnyWindowOpen()) {
        return false;
    }
    return true;
}

//热点绘制（调试用）
void HotspotRenderer::draw(sf::RenderTarget& tgt) {
    return;
    
    for (const auto& [name, originalShape] : hotspotShapes) {
        sf::ConvexShape shape = originalShape;
        
        // 高亮显示悬停的热点
        if (hoveredHotspot && *hoveredHotspot == name) {
            shape.setFillColor(sf::Color(255, 255, 0, 80));  // 悬停时黄色高亮
            shape.setOutlineColor(sf::Color::Yellow);
        } else {
            shape.setFillColor(sf::Color(0, 255, 0, 60));    // 正常绿色
            shape.setOutlineColor(sf::Color::Green);
        }
        tgt.draw(shape);
    }
}

void HotspotRenderer::drawImGuiTooltip() {
    if (!hoveredHotspot || !window) return;
    
    ImGui::BeginTooltip();
    
    if (hotspotDescriptions.count(*hoveredHotspot)) {
        ImGui::Text("描述: %s", hotspotDescriptions[*hoveredHotspot].c_str());
    }
    
    ImGui::EndTooltip();
}

void HotspotRenderer::handleMouseAndDrawUI() {
    if (!window) return;
    
    // 1. 先绘制tooltip（如果鼠标悬停在热点上）
    drawImGuiTooltip();
    
    // 2. 如果悬停在卷积层上，绘制详细结构按钮
    if (!currentHoveredHotspot_.empty() && 
        currentHoveredHotspot_.find("conv") != std::string::npos) {
        // 找到对应的热点形状
        for (const auto& [name, shape] : hotspotShapes) {
            if (name == currentHoveredHotspot_) {
                drawDetailButton(name, shape);
                break;
            }
        }
    }
}

void HotspotRenderer::drawDetailButton(const std::string& hotspotName, const sf::ConvexShape& shape) {
    // 获取热点区域的中心点
    sf::FloatRect bounds = shape.getGlobalBounds();
    sf::Vector2f center = sf::Vector2f(
        bounds.left + bounds.width / 2.0f,
        bounds.top + bounds.height / 2.0f
    );
    
    // 转换为屏幕坐标
    sf::Vector2i screenCenter = window->mapCoordsToPixel(center);
    
    // 设置按钮位置（热点区域中央）
    ImGui::SetNextWindowPos(ImVec2(
        static_cast<float>(screenCenter.x - 75),  
        static_cast<float>(screenCenter.y - 12)
    ), ImGuiCond_Always);
    
    ImGui::SetNextWindowSize(ImVec2(150, 24), ImGuiCond_Always);
    
    // 透明样式
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.3f));
    
    std::string windowName = "##detail_" + hotspotName;
    if (ImGui::Begin(windowName.c_str(), nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoSavedSettings)) {
        
        // 绘制查看详细结构按钮
        if (ImGui::Button("查看详细结构", ImVec2(150, 24))) {
            std::cout << "打开详细结构: " << hotspotName << std::endl;
            if (layerDetailRenderer_) {
                layerDetailRenderer_->setVisible(hotspotName, true);
            }
        }
    }
    
    ImGui::End();
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(2);
}