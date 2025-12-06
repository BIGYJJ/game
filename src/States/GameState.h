#pragma once
#include "State.h"
#include "../Entity/Player.h"
#include "../World/TileMap.h"
#include "../World/Camera.h"
#include "../Systems/TimeSystem.h"
#include <memory>

class GameState : public State {
public:
    GameState(Game* game);
    
    void handleInput(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

private:
    void loadWorld();

    std::unique_ptr<Player> player;
    std::unique_ptr<TileMap> tileMap;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<TimeSystem> timeSystem;

    // 背景相关 
    sf::Texture bgTexture;
    sf::Sprite bgSprite;
};