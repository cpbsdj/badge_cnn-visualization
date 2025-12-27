#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class BackgroundRenderer {
public:
    BackgroundRenderer();
    
    // 加载背景图片
    bool load(const std::string& pngPath);
    
    // 根据当前窗口大小重新计算缩放和居中
    void updateLayout(const sf::Vector2u& winSize);
    
    // 设置视图（用于拖拽/缩放）
    void setView(const sf::View& v);
    
    // 绘制背景
    void draw(sf::RenderTarget& target);
    
    // 获取背景精灵（用于碰撞检测等）
    const sf::Sprite& getSprite() const { return spr; }
    
    // 获取背景在世界坐标系中的尺寸
    sf::Vector2f getWorldSize() const { return imgSize; }
    
    // 获取背景在屏幕上的实际尺寸（考虑缩放后）
    sf::Vector2f getScaledSize() const;
    
    // 屏幕坐标转世界坐标（用于热点检测）
    sf::Vector2f screenToWorld(const sf::Vector2i& screenPos, const sf::RenderWindow& window) const;
    
    // 世界坐标转屏幕坐标
    sf::Vector2f worldToScreen(const sf::Vector2f& worldPos, const sf::RenderWindow& window) const;
    
    // 检查点是否在背景范围内
    bool isPointInBackground(const sf::Vector2f& worldPos) const;
    
    // 获取背景位置和缩放信息
    sf::Vector2f getPosition() const { return spr.getPosition(); }
    sf::Vector2f getScale() const { return spr.getScale(); }

private:
    sf::Texture tex;
    sf::Sprite spr;
    sf::View view;
    sf::Vector2f imgSize;  // 背景图片原始尺寸
};