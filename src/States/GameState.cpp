#include "GameState.h"
#include "../Core/Game.h"
#include <iostream>
#include <filesystem>  // For path debugging

GameState::GameState(Game* game, MapType mapType) 
    : State(game)
    , currentMap(mapType) 
{
    std::cout << "========================================" << std::endl;
    std::cout << "  Pixel Farm RPG - Scene Initialization" << std::endl;
    std::cout << "========================================" << std::endl;

    // Debug: Print current working directory
    std::cout << "[DEBUG] Working directory: " 
              << std::filesystem::current_path().string() << std::endl;

    // Create TileMap
    tileMap = std::make_unique<TileMap>();
    
    // Load map
    loadMap(mapType);
    
    // Set player position to map center
    sf::Vector2i mapSize = tileMap->getMapSize();
    player = std::make_unique<Player>(mapSize.x / 2.0f, mapSize.y / 2.0f);
    
    // Create camera
    camera = std::make_unique<Camera>(game->getWindow().getSize());
    camera->setBounds(mapSize);
    camera->snapTo(player->getPosition());
    camera->setSmoothness(6.0f);
    
    // Create time system
    timeSystem = std::make_unique<TimeSystem>();
    
    std::cout << "[OK] Player Position: (" << player->getPosition().x 
              << ", " << player->getPosition().y << ")" << std::endl;
    std::cout << "\nControls:" << std::endl;
    std::cout << "  WASD/Arrows - Move" << std::endl;
    std::cout << "  Space       - Attack" << std::endl;
    std::cout << "  F1 - Farm Map | F2 - Forest Map | F3 - Reload" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "========================================\n" << std::endl;
}

void GameState::loadMap(MapType mapType) {
    std::string mapPath;
    
    switch (mapType) {
        case MapType::Farm:
            mapPath = "assets/game_source/part1.tmj";
            break;
            
        case MapType::Forest:
            mapPath = "assets/map/forest.tmj";
            break;
    }
    
    // Debug: Check if file exists before loading
    if (std::filesystem::exists(mapPath)) {
        std::cout << "[OK] Map file found: " << mapPath << std::endl;
    } else {
        std::cerr << "[ERROR] Map file NOT found: " << mapPath << std::endl;
        std::cerr << "[ERROR] Full path would be: " 
                  << std::filesystem::absolute(mapPath).string() << std::endl;
        
        // List contents of assets directory for debugging
        if (std::filesystem::exists("assets")) {
            std::cout << "[DEBUG] Contents of 'assets' directory:" << std::endl;
            for (const auto& entry : std::filesystem::recursive_directory_iterator("assets")) {
                std::cout << "  " << entry.path().string() << std::endl;
            }
        } else {
            std::cerr << "[ERROR] 'assets' directory does not exist!" << std::endl;
        }
    }
    
    // Load the Tiled exported .tmj file
    // Second parameter is the display tile size (adjustable based on window)
    tileMap->loadFromTiled(mapPath, 48);
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
                std::cout << "Reloading map..." << std::endl;
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
        
        // Collision detection
        if (tileMap->isColliding(player->getCollisionBox())) {
            player->setPosition(oldPos);
        }
        
        // Boundary detection
        sf::Vector2f pos = player->getPosition();
        sf::Vector2i mapSize = tileMap->getMapSize();
        
        float margin = (float)tileMap->getTileSize();
        pos.x = std::max(margin, std::min(pos.x, mapSize.x - margin));
        pos.y = std::max(margin, std::min(pos.y, mapSize.y - margin));
        player->setPosition(pos);
    }
    
    // Update camera to follow player
    if (camera && player) {
        camera->follow(player->getPosition(), dt);
    }
    
    // Update time system
    if (timeSystem) {
        timeSystem->update(dt);
    }
}

void GameState::render(sf::RenderWindow& window) {
    // Apply camera view
    if (camera) {
        window.setView(camera->getView());
    }
    
    // Render tile map
    if (tileMap && camera) {
        tileMap->render(window, camera->getView());
    }
    
    // Render player
    if (player) {
        player->render(window);
    }
    
    // Reset to default view for UI
    window.setView(window.getDefaultView());
    
    // Render UI overlay
    renderUI(window);
}

void GameState::renderUI(sf::RenderWindow& window) {
    // UI background panel
    sf::RectangleShape uiBackground(sf::Vector2f(500, 80));
    uiBackground.setPosition(20, 20);
    uiBackground.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(uiBackground);
}

void GameState::switchMap(MapType newMap) {
    if (newMap != currentMap) {
        std::cout << "\n------------------------------------" << std::endl;
        std::cout << "Switching Map: " << getMapName(currentMap) 
                  << " -> " << getMapName(newMap) << std::endl;
        
        currentMap = newMap;
        loadMap(newMap);
        
        // Reset player position to map center
        if (player) {
            sf::Vector2i mapSize = tileMap->getMapSize();
            player->setPosition(mapSize.x / 2.0f, mapSize.y / 2.0f);
        }
        
        // Update camera bounds and position
        if (camera) {
            camera->setBounds(tileMap->getMapSize());
            camera->snapTo(player->getPosition());
        }
        
        std::cout << "[OK] Map switch complete" << std::endl;
        std::cout << "------------------------------------\n" << std::endl;
    }
}

std::string GameState::getMapName(MapType mapType) const {
    switch (mapType) {
        case MapType::Farm:   return "Farm";
        case MapType::Forest: return "Forest";
        default:              return "Unknown";
    }
}