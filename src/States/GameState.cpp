#include "GameState.h"
#include "../Core/Game.h"
#include "../Map_Data/MapData_Farm.h"
#include "../Map_Data/MapData_Forest.h"
#include <iostream>

GameState::GameState(Game* game, MapType mapType) 
    : State(game)
    , currentMap(mapType) 
{
    std::cout << "========================================" << std::endl;
    std::cout << "  Pixel Farm RPG - 游戏场景初始化" << std::endl;
    std::cout << "========================================" << std::endl;

    // 【修复】瓦片大小从32改为72，适配2560x1440窗口
    // 地图尺寸：30×20×72 = 2160×1440像素（完美填充窗口高度）
    tileMap = std::make_unique<TileMap>(36, 20, 72);
    
    // 加载地图
    loadMap(mapType);
    
    // 【修复】玩家位置也要调整（72×30/2=1080, 72×20/2=720）
    player = std::make_unique<Player>(1296, 720);
    
    // 创建摄像机
    camera = std::make_unique<Camera>(game->getWindow().getSize());
    camera->setBounds(tileMap->getMapSize());
    camera->snapTo(player->getPosition());
    camera->setSmoothness(6.0f);
    
    // 创建时间系统
    timeSystem = std::make_unique<TimeSystem>();
    
    std::cout << "✓ 地图加载完成: " << getMapName(mapType) << std::endl;
    std::cout << "✓ 地图尺寸: " << tileMap->getWidth() << "x" << tileMap->getHeight() 
              << " 瓦片 = " << tileMap->getMapSize().x << "×" << tileMap->getMapSize().y << " 像素" << std::endl;
    std::cout << "✓ 窗口尺寸: 2560×1440 像素" << std::endl;
    std::cout << "✓ 覆盖率: " << (int)(tileMap->getMapSize().x * 100.0 / 2560) << "%×"
              << (int)(tileMap->getMapSize().y * 100.0 / 1440) << "%" << std::endl;
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
            tileMap->initializeFromArray(Maps::FARM_GROUND, Maps::FARM_DECORATION);
            break;
            
        case MapType::Forest:
            tileMap->initializeFromArray(Maps::FOREST_GROUND, Maps::FOREST_DECORATION);
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
                
            case sf::Keyboard::F3:
                std::cout << "重新加载地图..." << std::endl;
                loadMap(currentMap);
                player->setPosition(1080, 720);  // 使用72像素的中心位置
                camera->snapTo(player->getPosition());
                break;
                
            default:
                break;
        }
    }
}

void GameState::update(float dt) {
    // 更新玩家
    if (player) {
        // 保存旧位置
        sf::Vector2f oldPos = player->getPosition();
        
        // 更新玩家状态
        player->update(dt);
        
        // 碰撞检测
        if (tileMap->isColliding(player->getCollisionBox())) {
            // 发生碰撞，恢复到旧位置
            player->setPosition(oldPos);
        }
        
        // 边界检测（防止玩家走出地图）
        sf::Vector2f pos = player->getPosition();
        sf::Vector2i mapSize = tileMap->getMapSize();
        
        // 限制在地图内（留一些边距）
        const float margin = 72.0f;  // 边距也要调整为瓦片大小
        pos.x = std::max(margin, std::min(pos.x, mapSize.x - margin));
        pos.y = std::max(margin, std::min(pos.y, mapSize.y - margin));
        player->setPosition(pos);
    }
    
    // 更新摄像机
    if (camera && player) {
        camera->follow(player->getPosition(), dt);
    }
    
    // 更新时间系统
    if (timeSystem) {
        timeSystem->update(dt);
    }
}

void GameState::render(sf::RenderWindow& window) {
    // 应用摄像机视图
    if (camera) {
        window.setView(camera->getView());
    }
    
    // 渲染地图
    if (tileMap && camera) {
        tileMap->render(window, camera->getView());
    }
    
    // 渲染玩家
    if (player) {
        player->render(window);
    }
    
    // 重置视图用于UI
    window.setView(window.getDefaultView());
    
    // 渲染UI
    renderUI(window);
}

void GameState::renderUI(sf::RenderWindow& window) {
    // 创建半透明背景
    sf::RectangleShape uiBackground(sf::Vector2f(500, 80));
    uiBackground.setPosition(20, 20);
    uiBackground.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(uiBackground);
    
    // 这里可以添加字体和文本
    // 由于没有字体文件，这里先注释掉
    /*
    static sf::Font font;
    static bool fontLoaded = false;
    
    if (!fontLoaded) {
        if (font.loadFromFile("../../assets/font.ttf")) {
            fontLoaded = true;
        }
    }
    
    if (fontLoaded) {
        sf::Text mapText;
        mapText.setFont(font);
        mapText.setString("当前地图: " + getMapName(currentMap));
        mapText.setCharacterSize(24);
        mapText.setFillColor(sf::Color::White);
        mapText.setPosition(30, 30);
        window.draw(mapText);
        
        sf::Text controlText;
        controlText.setFont(font);
        controlText.setString("F1: 农场 | F2: 森林 | F3: 重载 | ESC: 退出");
        controlText.setCharacterSize(18);
        controlText.setFillColor(sf::Color(220, 220, 220));
        controlText.setPosition(30, 60);
        window.draw(controlText);
    }
    */
}

void GameState::switchMap(MapType newMap) {
    if (newMap != currentMap) {
        std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        std::cout << "切换地图: " << getMapName(currentMap) 
                  << " → " << getMapName(newMap) << std::endl;
        
        currentMap = newMap;
        loadMap(newMap);
        
        // 重置玩家到新地图中心
        if (player) {
            player->setPosition(1080, 720);  // 72像素瓦片的中心位置
        }
        
        // 更新摄像机
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