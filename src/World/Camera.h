#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>

class Camera {
public:
    Camera(sf::Vector2u windowSize) 
        : smoothness(5.0f)
        , minBounds(0.0f, 0.0f)
        , maxBounds(10000.0f, 10000.0f)
    {
        view.setSize(windowSize.x, windowSize.y);
        view.setCenter(windowSize.x / 2.0f, windowSize.y / 2.0f);
        targetPosition = view.getCenter();
    }
    
    // 设置摄像机边界（地图大小）
    void setBounds(sf::Vector2f min, sf::Vector2f max) {
        minBounds = min;
        maxBounds = max;
    }
    
    // 设置摄像机边界（从地图尺寸）
    void setBounds(sf::Vector2i mapSize) {
        float halfWidth = view.getSize().x / 2.0f;
        float halfHeight = view.getSize().y / 2.0f;
        
        minBounds = sf::Vector2f(halfWidth, halfHeight);
        maxBounds = sf::Vector2f(
            mapSize.x - halfWidth,
            mapSize.y - halfHeight
        );
    }
    
    // 跟随目标（带平滑插值）
    void follow(sf::Vector2f position, float dt) {
        targetPosition = position;
        
        // 平滑跟随
        sf::Vector2f currentCenter = view.getCenter();
        sf::Vector2f newCenter = currentCenter + (targetPosition - currentCenter) * smoothness * dt;
        
        // 限制在地图边界内
        newCenter.x = std::max(minBounds.x, std::min(newCenter.x, maxBounds.x));
        newCenter.y = std::max(minBounds.y, std::min(newCenter.y, maxBounds.y));
        
        view.setCenter(newCenter);
    }
    
    // 立即移动到目标位置（无平滑）
    void snapTo(sf::Vector2f position) {
        targetPosition = position;
        
        // 限制在地图边界内
        position.x = std::max(minBounds.x, std::min(position.x, maxBounds.x));
        position.y = std::max(minBounds.y, std::min(position.y, maxBounds.y));
        
        view.setCenter(position);
    }
    
    // 缩放
    void zoom(float factor) {
        view.zoom(factor);
    }
    
    // 设置平滑度（0-10，越大越快）
    void setSmoothness(float s) {
        smoothness = std::max(0.1f, std::min(s, 10.0f));
    }
    
    // 获取视图
    const sf::View& getView() const { return view; }
    sf::View& getView() { return view; }
    
    // 获取可见区域
    sf::FloatRect getVisibleArea() const {
        sf::Vector2f center = view.getCenter();
        sf::Vector2f size = view.getSize();
        return sf::FloatRect(
            center.x - size.x / 2.0f,
            center.y - size.y / 2.0f,
            size.x,
            size.y
        );
    }
    
private:
    sf::View view;
    sf::Vector2f targetPosition;
    float smoothness;
    
    sf::Vector2f minBounds;
    sf::Vector2f maxBounds;
};