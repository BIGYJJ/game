#pragma once
#include "State.h"
#include "../Entity/Player.h"
#include "../World/TileMap.h"
#include "../World/Camera.h"
#include "../Systems/TimeSystem.h"
#include <memory>
#include <string>

// Map type enumeration
enum class MapType {
    Farm,    // Farm map
    Forest   // Forest map
};

class GameState : public State {
public:
    // Constructor - can specify initial map type
    GameState(Game* game, MapType mapType = MapType::Farm);
    
    // Event handling
    void handleInput(const sf::Event& event) override;
    
    // Update game logic
    void update(float dt) override;
    
    // Render
    void render(sf::RenderWindow& window) override;

private:
    // Load specified map
    void loadMap(MapType mapType);
    
    // Switch to new map
    void switchMap(MapType newMap);
    
    // Render UI layer
    void renderUI(sf::RenderWindow& window);
    
    // Get map name string
    std::string getMapName(MapType mapType) const;

private:
    // Game objects
    std::unique_ptr<Player> player;
    std::unique_ptr<TileMap> tileMap;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<TimeSystem> timeSystem;
    
    // Current map type
    MapType currentMap;
};