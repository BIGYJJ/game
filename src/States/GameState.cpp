#include "GameState.h"
#include "../Core/Game.h"
#include <iostream>

GameState::GameState(Game* game) : State(game) {
    std::cout << "The game scene is being initialized..." << std::endl;

    // --- 1. 设置草坪背景 ---
    if (!bgTexture.loadFromFile("../../assets/grass.jpg")) {
        std::cerr << "Error: Failed to load grass.png" << std::endl;
    } else {
        // 开启平铺模式
        bgTexture.setRepeated(true);
        bgSprite.setTexture(bgTexture);
        
        // 设置纹理区域覆盖整个窗口大小
        sf::Vector2u windowSize = game->getWindow().getSize();
        bgSprite.setTextureRect(sf::IntRect(0, 0, windowSize.x, windowSize.y));
    }
    
    // 创建玩家
    player = std::make_unique<Player>(640, 360);
    
    // (其他的 TileMap/Camera 初始化代码保持不变...)
    tileMap = std::make_unique<TileMap>(50, 50, 32);
    camera = std::make_unique<Camera>(game->getWindow().getSize());
    timeSystem = std::make_unique<TimeSystem>();
}

void GameState::handleInput(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            requestPop();
        }
    }
}

void GameState::update(float dt) {
    if (player) {
        player->update(dt);
    }
    // (其他 update 代码保持不变...)
}

void GameState::render(sf::RenderWindow& window) {
    // 1. 先画背景
    window.draw(bgSprite);

    // 2. 再画其他的 (TileMap暂时不画或者画在背景之上)
    
    // 3. 最后画玩家
    if (player) {
        player->render(window);
    }
    
    // (不需要再画那个白色的 playerSprite 方块了，删掉原来的测试代码)
}

void GameState::loadWorld() {
}