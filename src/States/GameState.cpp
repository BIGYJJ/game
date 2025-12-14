#include "GameState.h"
#include "../Core/Game.h"
#include <iostream>
#include <filesystem>  // For path debugging

GameState::GameState(Game* game, MapType mapType) 
    : State(game)
    , currentMap(mapType)
    , wasAttacking(false)
{
    std::cout << "========================================" << std::endl;
    std::cout << "  Pixel Farm RPG - Scene Initialization" << std::endl;
    std::cout << "========================================" << std::endl;

    // Debug: Print current working directory
    std::cout << "[DEBUG] Working directory: " 
              << std::filesystem::current_path().string() << std::endl;

    // Create TileMap
    tileMap = std::make_unique<TileMap>();
    
    // Create TreeManager
    treeManager = std::make_unique<TreeManager>();
    treeManager->init("../../assets");
    
    // Load map
    loadMap(mapType);
    
    // Initialize trees from map objects
    initTrees();
    
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
    
    // Initialize UI
    initUI(game->getWindow());
    
    std::cout << "[OK] Player Position: (" << player->getPosition().x 
              << ", " << player->getPosition().y << ")" << std::endl;
    std::cout << "\nControls:" << std::endl;
    std::cout << "  WASD/Arrows - Move" << std::endl;
    std::cout << "  Space       - Attack (chop trees!)" << std::endl;
    std::cout << "  Tab/C       - Toggle Stats Panel" << std::endl;
    std::cout << "  F1 - Farm Map | F2 - Forest Map | F3 - Reload" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "========================================\n" << std::endl;
}

void GameState::initUI(sf::RenderWindow& window) {
    // Create stats panel
    statsPanel = std::make_unique<StatsPanel>();
    
    // Try to load icon from various paths
    // Note: Working directory is usually build/bin/, so ../../ goes back to project root
    std::vector<std::string> iconPaths = {
        "../../assets/ui/icon1.png",          // From build/bin/
        "../../assets/ui/Statspanel.png",     // Alternative name
        "../../assets/icons/icon1.png",
        "assets/ui/icon1.png",                // From project root
        "assets/icons/icon1.png",
        "assets/icon1.png",
        "../assets/ui/icon1.png"              // From build/
    };
    
    bool iconLoaded = false;
    for (const auto& path : iconPaths) {
        if (std::filesystem::exists(path)) {
            statsPanel->init(path);
            iconLoaded = true;
            std::cout << "[OK] Stats icon loaded: " << path << std::endl;
            break;
        }
    }
    
    if (!iconLoaded) {
        // Use default (will create placeholder)
        statsPanel->init("../../assets/ui/icon1.png");
        std::cout << "[WARNING] Stats icon not found, using placeholder" << std::endl;
        std::cout << "[HINT] Place icon at: assets/ui/icon1.png" << std::endl;
    }
    
    // Position icon at bottom-left of screen
    sf::Vector2u windowSize = window.getSize();
    statsPanel->setIconPosition(20.0f, windowSize.y - 80.0f);
    
    std::cout << "[OK] Stats Panel initialized" << std::endl;
}

void GameState::loadMap(MapType mapType) {
    std::string mapPath;
    
    switch (mapType) {
        case MapType::Farm:
            mapPath = "assets/game_source/part1.tmj";
            break;
            
        case MapType::Forest:
            mapPath = "assets/map/forest1.tmj";
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
    // Handle UI input first
    if (statsPanel) {
        statsPanel->handleEvent(event, game->getWindow());
    }
    
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
            
            // Debug: Add experience/gold for testing
            case sf::Keyboard::E:
                if (player) {
                    player->getStats().addExp(50);
                    std::cout << "[DEBUG] +50 EXP" << std::endl;
                }
                break;
                
            case sf::Keyboard::G:
                if (player) {
                    player->getStats().addGold(100);
                    std::cout << "[DEBUG] +100 Gold" << std::endl;
                }
                break;
                
            default:
                break;
        }
    }
}

void GameState::update(float dt) {
    if (player) {
        sf::Vector2f oldPos = player->getPosition();
        
        player->update(dt);
        
        // Collision detection with tilemap
        if (tileMap->isColliding(player->getCollisionBox())) {
            player->setPosition(oldPos);
        }
        
        // Collision detection with trees
        if (treeManager && treeManager->isCollidingWithAnyTree(player->getCollisionBox())) {
            player->setPosition(oldPos);
        }
        
        // Boundary detection
        sf::Vector2f pos = player->getPosition();
        sf::Vector2i mapSize = tileMap->getMapSize();
        
        float margin = (float)tileMap->getTileSize();
        pos.x = std::max(margin, std::min(pos.x, mapSize.x - margin));
        pos.y = std::max(margin, std::min(pos.y, mapSize.y - margin));
        player->setPosition(pos);
        
        // Handle attack - check for tree hits
        handlePlayerAttack();
        
        // Update stats panel with current player stats
        if (statsPanel) {
            statsPanel->updateStats(player->getStats());
        }
    }
    
    // Update camera to follow player
    if (camera && player) {
        camera->follow(player->getPosition(), dt);
    }
    
    // Update time system
    if (timeSystem) {
        timeSystem->update(dt);
    }
    
    // Update trees
    if (treeManager) {
        treeManager->update(dt);
    }
    
    // Update UI
    if (statsPanel) {
        statsPanel->update(dt);
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
    
    // Render trees
    if (treeManager && camera) {
        treeManager->render(window, camera->getView());
    }
    
    // Render player
    if (player) {
        player->render(window);
    }
    
    // Reset to default view for UI
    window.setView(window.getDefaultView());
    
    // Render UI overlay
    renderUI(window);
    
    // Render tree tooltips (needs world mouse position)
    if (treeManager && camera) {
        sf::Vector2i mouseScreenPos = sf::Mouse::getPosition(window);
        sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mouseScreenPos, camera->getView());
        treeManager->renderTooltips(window, mouseWorldPos);
    }
}

void GameState::renderUI(sf::RenderWindow& window) {
    // UI background panel (top-left info)
    sf::RectangleShape uiBackground(sf::Vector2f(300, 60));
    uiBackground.setPosition(20, 20);
    uiBackground.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(uiBackground);
    
    // Render stats panel (bottom-left)
    if (statsPanel) {
        statsPanel->render(window);
    }
}

void GameState::switchMap(MapType newMap) {
    if (newMap != currentMap) {
        std::cout << "\n------------------------------------" << std::endl;
        std::cout << "Switching Map: " << getMapName(currentMap) 
                  << " -> " << getMapName(newMap) << std::endl;
        
        currentMap = newMap;
        loadMap(newMap);
        
        // Clear and reload trees
        if (treeManager) {
            treeManager->clearAllTrees();
        }
        initTrees();
        
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

void GameState::initTrees() {
    if (!treeManager || !tileMap) return;
    
    // 从地图对象层加载树木
    const auto& objects = tileMap->getObjects();
    float displayScale = (float)tileMap->getTileSize() / 32.0f;  // 32是源tile大小
    
    std::cout << "[Trees] Loading trees from " << objects.size() << " map objects..." << std::endl;
    
    for (const auto& obj : objects) {
        if (obj.gid <= 0) continue;
        
        // ========================================
        // 检查对象类型（从tsx文件动态读取）
        // 支持多种方式识别树木：
        //   1. type 属性 == "tree"
        //   2. name 属性包含 "tree"
        //   3. tileset 名称包含 "tree"
        // ========================================
        std::string objType = obj.type;
        std::string objName = obj.name;
        
        if (objType.empty() && obj.tileProperty) {
            objType = obj.tileProperty->type;
        }
        if (objName.empty() && obj.tileProperty) {
            objName = obj.tileProperty->name;
        }
        
        // 判断是否为树木
        bool isTree = false;
        if (objType == "tree") {
            isTree = true;
        } else if (objName.find("tree") != std::string::npos) {
            // name 包含 "tree"（如 tree1, apple_tree, cherry_tree）
            isTree = true;
        }
        
        if (!isTree) {
            std::cout << "[Objects] Skipping non-tree object: " << objName 
                      << " (type=" << objType << ")" << std::endl;
            continue;
        }
        
        // 对象的y坐标是底部
        float x = obj.x * displayScale;
        float y = obj.y * displayScale;
        
        // 计算显示尺寸
        float width = obj.width * displayScale;
        float height = obj.height * displayScale;
        
        Tree* tree = nullptr;
        
        // ========================================
        // 使用动态属性创建树木（推荐方式）
        // 不再硬编码 gid -> 树类型 的映射
        // ========================================
        if (obj.tileProperty) {
            tree = treeManager->addTreeFromProperty(x, y, obj.tileProperty);
            std::cout << "[Trees] Created from TileProperty: " << obj.tileProperty->name 
                      << " HP=" << obj.tileProperty->hp << std::endl;
        } else {
            // 后备方案：使用对象名称
            std::string treeType = obj.name.empty() ? "tree1" : obj.name;
            tree = treeManager->addTree(x, y, treeType);
            std::cout << "[Trees] Created from name: " << treeType << std::endl;
        }
        
        if (tree) {
            // 设置正确的尺寸（与地图对象一致）
            tree->setSize(width, height);
            
            // 普通树可以在成熟后变成果树
            if (tree->getTreeType() == "tree1") {
                tree->setCanTransform(true);
            }
            
            // 设置销毁回调
            tree->setOnDestroyed([this](Tree& t) {
                // 树木被砍倒时的处理
                auto drops = t.generateDrops();
                for (const auto& drop : drops) {
                    // 这里可以添加到玩家背包
                    std::cout << "[Drops] Player gets: " << drop.first 
                              << " x" << drop.second << std::endl;
                    // 添加经验
                    if (player) {
                        player->getStats().addExp(10);
                        player->getStats().addSkillExp(LifeSkill::Farming, 5);
                    }
                }
            });
            
            // 设置果实采摘回调
            tree->setOnFruitHarvested([this](Tree& t) {
                auto drops = t.generateFruitDrops();
                for (const auto& drop : drops) {
                    std::cout << "[Harvest] Player gets: " << drop.first 
                              << " x" << drop.second << std::endl;
                    if (player) {
                        player->getStats().addExp(5);
                    }
                }
            });
            
            // 设置生长阶段变化回调
            tree->setOnGrowthStageChanged([](Tree& t) {
                std::cout << "[Tree] " << t.getName() << " changed to: " 
                          << t.getGrowthStageName() << std::endl;
            });
        }
    }
    
    // 清除TileMap中的对象，避免重复渲染
    // TreeManager现在完全接管树木的渲染和管理
    tileMap->clearObjects();
    
    std::cout << "[Trees] Total trees loaded: " << treeManager->getTreeCount() << std::endl;
}

void GameState::handlePlayerAttack() {
    if (!player || !treeManager) return;
    
    bool isCurrentlyAttacking = player->isAttacking();
    
    // 只在攻击动画开始时检测一次
    if (isCurrentlyAttacking && !wasAttacking) {
        // 获取攻击范围
        sf::Vector2f attackCenter = player->getAttackCenter();
        float attackRadius = player->getAttackRadius();
        
        // 计算伤害
        float damage = player->performAttack();
        
        // 对范围内的树木造成伤害
        auto hitTrees = treeManager->damageTreesInRange(attackCenter, attackRadius, damage);
        
        if (!hitTrees.empty()) {
            std::cout << "[Attack] Hit " << hitTrees.size() << " tree(s) for " 
                      << damage << " damage" << std::endl;
        }
    }
    
    wasAttacking = isCurrentlyAttacking;
}

std::string GameState::getMapName(MapType mapType) const {
    switch (mapType) {
        case MapType::Farm:   return "Farm";
        case MapType::Forest: return "Forest";
        default:              return "Unknown";
    }
}