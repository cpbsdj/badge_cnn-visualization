#include "renderer/LayerDetailRenderer.hpp"
#include "renderer/LayerDetailRenderer.hpp"
#include "renderer/detail/Conv1Detail.hpp"
#include "renderer/detail/Conv2Detail.hpp"
#include "renderer/detail/Conv3Detail.hpp"
#include "renderer/detail/Conv4Detail.hpp"
#include <iostream>


void LayerDetailRenderer::createDetailRenderer(const std::string& layerName) {
    if (layers_.find(layerName) == layers_.end()) {
        std::cerr << "错误: 未知图层 " << layerName << std::endl;
        return;
    }
    
    std::cout << "创建detailRenderer: " << layerName << std::endl;
    
    if (layerName == "conv1") {
        layers_[layerName].detailRenderer = std::make_unique<Conv1Detail>();
    } else if (layerName == "conv2") {
        layers_[layerName].detailRenderer = std::make_unique<Conv2Detail>();
    } else if (layerName == "conv3") {
        layers_[layerName].detailRenderer = std::make_unique<Conv3Detail>();
    } else if (layerName == "conv4") {
        layers_[layerName].detailRenderer = std::make_unique<Conv4Detail>();
    }
    
    // 初始化详细交互器
    if (layers_[layerName].detailRenderer) {
        if (layers_[layerName].detailRenderer->initialize()) {
            std::cout << layerName << " detailRenderer初始化成功" << std::endl;
            std::cout << "  热点数量: " << layers_[layerName].detailRenderer->getHotspotCount() << std::endl;
        } else {
            std::cerr << layerName << " detailRenderer初始化失败" << std::endl;
        }
    } else {
        std::cerr << layerName << " detailRenderer创建失败" << std::endl;
    }
}

LayerDetailRenderer::LayerDetailRenderer() {
    // 初始化图层信息
    layers_["conv1"] = LayerDetail();
    layers_["conv1"].title = "第一卷积层详细结构";
    
    layers_["conv2"] = LayerDetail();
    layers_["conv2"].title = "第二卷积层详细结构";
    
    layers_["conv3"] = LayerDetail();
    layers_["conv3"].title = "第三卷积层详细结构";
    
    layers_["conv4"] = LayerDetail();
    layers_["conv4"].title = "第四卷积层详细结构";

    createDetailRenderer("conv1");
    createDetailRenderer("conv2"); 
    createDetailRenderer("conv3");
    createDetailRenderer("conv4");
}

bool LayerDetailRenderer::loadTexture(const std::string& layerName, const std::string& texturePath) {
    if (layers_.find(layerName) == layers_.end()) {
        std::cerr << "未知的图层: " << layerName << std::endl;
        return false;
    }
    
    if (!layers_[layerName].texture.loadFromFile(texturePath)) {
        std::cerr << "无法加载纹理: " << texturePath << std::endl;
        return false;
    }

    std::cout << "加载详细结构纹理: " << texturePath << std::endl;
    return true;
}

void LayerDetailRenderer::setVisible(const std::string& layerName, bool visible) {
    if (layers_.find(layerName) != layers_.end()) {
        if (!layers_[layerName].visible && visible) {
            layers_[layerName].justOpened = true;
        }
        layers_[layerName].visible = visible;
    }
}

bool LayerDetailRenderer::isVisible(const std::string& layerName) const {
    auto it = layers_.find(layerName);
    return it != layers_.end() && it->second.visible;
}

bool LayerDetailRenderer::justOpened(const std::string& layerName) const {
    auto it = layers_.find(layerName);
    return it != layers_.end() && it->second.justOpened;
}

void LayerDetailRenderer::draw() {
    for (auto& [layerName, detail] : layers_) {
        if (detail.visible) {
            drawDetailWindow(layerName, detail);
        }
    }
}

bool LayerDetailRenderer::isAnyWindowOpen() const {
    for (const auto& [name, detail] : layers_) {
        if (detail.visible) {
            return true;
        }
    }
    return false;
}

void LayerDetailRenderer::drawDetailWindow(const std::string& layerName, LayerDetail& detail) {
    // 设置窗口大小和位置（居中显示）
    ImGui::SetNextWindowSize(ImVec2(1470, 840), ImGuiCond_Always);
    
    // 开始绘制窗口
    if (ImGui::Begin(detail.title.c_str(), &detail.visible, 
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar)) {
        
        // 首次打开时居中显示
        if (detail.justOpened) {
            ImGui::SetWindowPos(ImVec2(
                (ImGui::GetIO().DisplaySize.x - ImGui::GetWindowWidth()) * 0.5f,
                (ImGui::GetIO().DisplaySize.y - ImGui::GetWindowHeight()) * 0.5f
            ));
            detail.justOpened = false;
        }
        
        // 获取窗口内容区域大小
        ImVec2 contentSize = ImGui::GetContentRegionAvail();

        // 保存图片的起始位置
        ImVec2 imagePos = ImGui::GetCursorScreenPos();

        // 保存窗口信息
        detail.windowPos = ImGui::GetWindowPos();
        detail.windowSize = ImGui::GetWindowSize();
        detail.contentSize = ImGui::GetContentRegionAvail();
        detail.imagePos = ImGui::GetCursorScreenPos();
        
        // 显示背景图片
        ImGui::Image(
            (void*)(intptr_t)detail.texture.getNativeHandle(),
            contentSize,
            ImVec2(0, 0), ImVec2(1, 1)
        );
        
        // 绘制热点
        if (detail.detailRenderer) {
            //std::cout << "绘制热点: " << layerName << std::endl;
            detail.detailRenderer->drawHotspots(contentSize, imagePos);
        }
        
        ImGui::End();
    } else {
        detail.visible = false;
    }
}

void LayerDetailRenderer::handleMouse(const sf::Vector2f& mousePos) {
    for (auto& [layerName, detail] : layers_) {
        if (detail.visible && detail.detailRenderer) {

            ImVec2 windowPos = detail.windowPos;
            ImVec2 windowSize = detail.windowSize;
            ImVec2 contentSize = detail.contentSize;
            ImVec2 imagePos = detail.imagePos;
            
            // 检查鼠标是否在当前窗口内
            if (mousePos.x >= windowPos.x && mousePos.x <= windowPos.x + windowSize.x &&
                mousePos.y >= windowPos.y && mousePos.y <= windowPos.y + windowSize.y) {
                
                // 处理该窗口的鼠标交互
                detail.detailRenderer->handleMouse(mousePos, contentSize, imagePos);
            }
        }
    }
}

void LayerDetailRenderer::handleButtons() {
    // 遍历所有图层，处理按钮点击
    for (auto& [layerName, detail] : layers_) {
        if (detail.visible && detail.detailRenderer) {
            detail.detailRenderer->handleButtons();
        }
    }
}