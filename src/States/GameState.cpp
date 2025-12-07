#include "GameState.h"
#include "../Core/Game.h"
#include <iostream>

GameState::GameState(Game* game, MapType mapType) 
    : State(game)
    , currentMap(mapType) 
{
    std::cout << "========================================" << std::endl;
    std::cout << "  Pixel Farm RPG - 游戏场景初始化" << std::endl;
    std::cout << "========================================" << std::endl;

    // 创建 TileMap
    tileMap = std::make_unique<TileMap>();
    
    // 加载地图
    loadMap(mapType);
    
    // 玩家位置设为地图中央
    sf::Vector2i mapSize = tileMap->getMapSize();
    player = std::make_unique<Player>(mapSize.x / 2.0f, mapSize.y / 2.0f);
    
    // 创建摄像机
    camera = std::make_unique<Camera>(game->getWindow().getSize());
    camera->setBounds(mapSize);
    camera->snapTo(player->getPosition());
    camera->setSmoothness(6.0f);
    
    // 创建时间系统
    timeSystem = std::make_unique<TimeSystem>();
    
    std::cout << "✓ 玩家位置: (" << player->getPosition().x << ", " << player->getPosition().y << ")" << std::endl;
    std::cout << "\n控制说明:" << std::endl;
    std::cout << "  WASD/方向键 - 移动" << std::endl;
    std::cout << "  空格键 - 攻击" << std::endl;
    std::cout << "  F1 - 农场地图 | F2 - 森林地图 | F3 - 重载" << std::endl;
    std::cout << "  ESC - 退出" << std::endl;
    std::cout << "========================================\n" << std::endl;
}

void GameState::loadMap(MapType mapType) {
    switch (mapType) {
        case MapType::Farm:
            // =============================================
            // 直接加载 Tiled 导出的 .tmj 文件！
            // 参数2是显示时的瓦片大小，可以根据窗口调整
            // =============================================
            tileMap->loadFromTiled("../../assets/map/farm.tmj", 48);
            break;
            
        case MapType::Forest:
            tileMap->loadFromTiled("../../assets/map/forest.tmj", 48);
            break;
    }
}

void GameState::handleInput(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Escape:
                requestPop();
                break;
                
            case sf::Keyboard::F1:
                switchMap(MapType::Farm);
                break;
                
            case sf::Keyboard::F2:
                switchMap(MapType::Forest);
                break;
                
            case sf::Keyboard::F3: {
                std::cout << "重新加载地图..." << std::endl;
                loadMap(currentMap);
                sf::Vector2i mapSize = tileMap->getMapSize();
                player->setPosition(mapSize.x / 2.0f, mapSize.y / 2.0f);
                camera->snapTo(player->getPosition());
                break;
            }
                
            default:
                break;
        }
    }
}

void GameState::update(float dt) {
    if (player) {
        sf::Vector2f oldPos = player->getPosition();
        
        player->update(dt);
        
        // 碰撞检测
        if (tileMap->isColliding(player->getCollisionBox())) {
            player->setPosition(oldPos);
        }
        
        // 边界检测
        sf::Vector2f pos = player->getPosition();
        sf::Vector2i mapSize = tileMap->getMapSize();
        
        float margin = (float)tileMap->getTileSize();
        pos.x = std::max(margin, std::min(pos.x, mapSize.x - margin));
        pos.y = std::max(margin, std::min(pos.y, mapSize.y - margin));
        player->setPosition(pos);
    }
    
    if (camera && player) {
        camera->follow(player->getPosition(), dt);
    }
    
    if (timeSystem) {
        timeSystem->update(dt);
    }
}

void GameState::render(sf::RenderWindow& window) {
    if (camera) {
        window.setView(camera->getView());
    }
    
    if (tileMap && camera) {
        tileMap->render(window, camera->getView());
    }
    
    if (player) {
        player->render(window);
    }
    
    window.setView(window.getDefaultView());
    
    renderUI(window);
}

void GameState::renderUI(sf::RenderWindow& window) {
    sf::RectangleShape uiBackground(sf::Vector2f(500, 80));
    uiBackground.setPosition(20, 20);
    uiBackground.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(uiBackground);
}

void GameState::switchMap(MapType newMap) {
    if (newMap != currentMap) {
        std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        std::cout << "切换地图: " << getMapName(currentMap) 
                  << " → " << getMapName(newMap) << std::endl;
        
        currentMap = newMap;
        loadMap(newMap);
        
        if (player) {
            sf::Vector2i mapSize = tileMap->getMapSize();
            player->setPosition(mapSize.x / 2.0f, mapSize.y / 2.0f);
        }
        
        if (camera) {
            camera->setBounds(tileMap->getMapSize());
            camera->snapTo(player->getPosition());
        }
        
        std::cout << "✓ 地图切换完成" << std::endl;
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    }
}

std::string GameState::getMapName(MapType mapType) const {
    switch (mapType) {
        case MapType::Farm:   return "农场 (Farm)";
        case MapType::Forest: return "森林 (Forest)";
        default:              return "未知";
    }
}