#include "BackgroundRenderer.hpp"
#include <iostream>

BackgroundRenderer::BackgroundRenderer() 
    : imgSize(0.f, 0.f) {
}

bool BackgroundRenderer::load(const std::string& pngPath) {
    if (!tex.loadFromFile(pngPath)) {
        std::cerr << "无法加载背景图片: " << pngPath << std::endl;
        return false;
    }
    
    tex.setSmooth(true);
    spr.setTexture(tex);
    imgSize = sf::Vector2f(tex.getSize().x, tex.getSize().y);
    
    std::cout << "背景图片加载成功: " << pngPath << std::endl;
    std::cout << "图片尺寸: " << imgSize.x << " x " << imgSize.y << std::endl;
    
    return true;
}

void BackgroundRenderer::updateLayout(const sf::Vector2u& winSize) {
    if (imgSize.x == 0 || imgSize.y == 0) return;
    
    float scale = winSize.x / imgSize.x;           // 以宽为基准
    float offsetY = (winSize.y - imgSize.y * scale) * 0.5f;

    spr.setScale(scale, scale);
    spr.setPosition(0.f, offsetY);
    
    // 同时更新视图
    view.setCenter(imgSize.x * 0.5f, imgSize.y * 0.5f);
    view.setSize(imgSize.x, imgSize.y);
}

void BackgroundRenderer::setView(const sf::View& v) {
    view = v;
}

void BackgroundRenderer::draw(sf::RenderTarget& target) {
    // 保存当前视图
    sf::View originalView = target.getView();
    
    // 设置背景视图
    target.setView(view);
    
    // 绘制背景
    target.draw(spr);
    
    // 恢复原始视图
    target.setView(originalView);
}

sf::Vector2f BackgroundRenderer::getScaledSize() const {
    return sf::Vector2f(
        imgSize.x * spr.getScale().x,
        imgSize.y * spr.getScale().y
    );
}

sf::Vector2f BackgroundRenderer::screenToWorld(const sf::Vector2i& screenPos, const sf::RenderWindow& window) const {
    // 将屏幕坐标转换为相对于背景视图的坐标
    sf::Vector2f worldPos = window.mapPixelToCoords(screenPos, view);
    return worldPos;
}

sf::Vector2f BackgroundRenderer::worldToScreen(const sf::Vector2f& worldPos, const sf::RenderWindow& window) const {
    // 将世界坐标转换为屏幕坐标
    sf::Vector2i screenPos = window.mapCoordsToPixel(worldPos, view);
    return sf::Vector2f(screenPos);
}

bool BackgroundRenderer::isPointInBackground(const sf::Vector2f& worldPos) const {
    // 获取背景的全局边界
    sf::FloatRect bounds = spr.getGlobalBounds();
    
    // 检查点是否在边界内
    return bounds.contains(worldPos);
}