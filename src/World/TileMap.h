#pragma once
#include <SFML/Graphics.hpp>

// 暂时的简化版TileMap，后续可以扩展
class TileMap {
public:
    TileMap(int width, int height, int tileSize) 
        : width(width), height(height), tileSize(tileSize) {}
    
    void render(sf::RenderWindow& window, const sf::View& view) {
        // 暂时什么都不做
    }
    
private:
    int width;
    int height;
    int tileSize;
};