#pragma once
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <string>

class ConvDetailBase {
public:
    virtual ~ConvDetailBase() = default;
    
    virtual bool initialize() = 0;
    
    virtual void drawHotspots(ImVec2 contentSize, ImVec2 imagePos) = 0;
    
    virtual void handleMouse(const sf::Vector2f& mousePos, ImVec2 contentSize, ImVec2 imagePos) = 0;

    virtual void handleButtons() = 0;

    virtual std::string getLayerName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual size_t getHotspotCount() const = 0;
};