#pragma once
#include <SFML/Graphics.hpp>

// 暂时的简化版Camera，后续可以扩展
class Camera {
public:
    Camera(sf::Vector2u windowSize) {
        view.setSize(windowSize.x, windowSize.y);
        view.setCenter(windowSize.x / 2.0f, windowSize.y / 2.0f);
    }
    
    void follow(sf::Vector2f position) {
        // 暂时什么都不做
    }
    
    const sf::View& getView() const { return view; }
    
private:
    sf::View view;
};