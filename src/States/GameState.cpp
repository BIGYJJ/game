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

    // ========================================
    // 初始化物品系统（必须在其他系统之前）
    // ========================================
    initItemSystem();

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
    std::cout << "  I/B         - Toggle Inventory" << std::endl;
    std::cout << "  F1 - Farm Map | F2 - Forest Map | F3 - Reload" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "========================================\n" << std::endl;
}

void GameState::initItemSystem() {
    std::cout << "[ItemSystem] Initializing..." << std::endl;
    
    // 初始化物品数据库
    ItemDatabase::getInstance().initialize();
    ItemDatabase::getInstance().loadTextures("");
    
    // 创建背包
    inventory = std::make_unique<Inventory>();
    
    // 设置使用物品回调
    inventory->setOnUseItem([this](const ItemStack& item, const ItemData* data) {
        return onUseItem(item, data);
    });
    
    // 设置物品添加回调（用于显示提示）
    inventory->setOnItemAdded([](const ItemStack& item, int slotIndex) {
        const ItemData* data = ItemDatabase::getInstance().getItemData(item.itemId);
        if (data) {
            std::cout << "[Inventory] +" << item.count << " " << data->name << std::endl;
        }
    });
    
    // 创建掉落物品管理器
    droppedItemManager = std::make_unique<DroppedItemManager>();
    droppedItemManager->init("../../assets");
    
    // 设置拾取回调
    droppedItemManager->setOnItemPickup([this](const ItemStack& item) {
        // 添加到背包
        int added = inventory->addItemStack(item);
        
        // 添加到事件日志（只有成功拾取的物品）
        if (added > 0 && eventLogPanel) {
            const ItemData* data = ItemDatabase::getInstance().getItemData(item.itemId);
            if (data) {
                eventLogPanel->addItemObtained(data->name, added, item.itemId);
            }
        }
        
        if (added < item.count) {
            // 背包已满，重新掉落未能拾取的物品
            if (player) {
                sf::Vector2f pos = player->getPosition();
                droppedItemManager->spawnItem(item.itemId, item.count - added, pos.x, pos.y);
            }
            // 警告背包已满
            if (eventLogPanel) {
                eventLogPanel->addWarning("背包已满！");
            }
        }
    });
    
    // 测试：给玩家一些初始物品
    inventory->addItem("wood", 5);
    inventory->addItem("apple", 3);
    
    std::cout << "[ItemSystem] Initialized successfully" << std::endl;
}

void GameState::initUI(sf::RenderWindow& window) {
    // Create stats panel
    statsPanel = std::make_unique<StatsPanel>();
    
    // Try to load icon from various paths
    std::vector<std::string> iconPaths = {
        "../../assets/ui/icon1.png",
        "../../assets/ui/Statspanel.png",
        "../../assets/icons/icon1.png",
        "assets/ui/icon1.png",
        "assets/icons/icon1.png",
        "assets/icon1.png",
        "../assets/ui/icon1.png"
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
        statsPanel->init("../../assets/ui/icon1.png");
        std::cout << "[WARNING] Stats icon not found, using placeholder" << std::endl;
    }
    
    // Position icon at bottom-left of screen
    sf::Vector2u windowSize = window.getSize();
    statsPanel->setIconPosition(20.0f, windowSize.y - 80.0f);
    
    // ========================================
    // 创建背包面板
    // ========================================
    inventoryPanel = std::make_unique<InventoryPanel>();
    inventoryPanel->init("../../assets/ui/inventory_icon.png");
    inventoryPanel->setInventory(inventory.get());
    inventoryPanel->setIconPosition(150.0f, windowSize.y - 80.0f);  // 往右移动更多
    
    // 设置丢弃物品回调
    inventoryPanel->setOnDropItem([this](const ItemStack& item, sf::Vector2f pos) {
        if (player && droppedItemManager) {
            sf::Vector2f playerPos = player->getPosition();
            // 在玩家位置前方掉落
            droppedItemManager->spawnItem(item.itemId, item.count, playerPos.x + 30, playerPos.y);
        }
    });
    
    // ========================================
    // 创建事件日志面板
    // ========================================
    eventLogPanel = std::make_unique<EventLogPanel>();
    eventLogPanel->init();
    
    // 设置面板位置（屏幕右上角）
    float logPanelWidth = 300.0f;
    float logPanelHeight = 250.0f;
    eventLogPanel->setSize(logPanelWidth, logPanelHeight);
    eventLogPanel->setPosition(windowSize.x - logPanelWidth - 15.0f, 15.0f);
    
    // 添加欢迎消息
    eventLogPanel->addMessage("欢迎来到像素农场！", EventType::System);
    
    std::cout << "[OK] UI initialized" << std::endl;
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
    
    if (std::filesystem::exists(mapPath)) {
        std::cout << "[OK] Map file found: " << mapPath << std::endl;
    } else {
        std::cerr << "[ERROR] Map file NOT found: " << mapPath << std::endl;
        std::cerr << "[ERROR] Full path would be: " 
                  << std::filesystem::absolute(mapPath).string() << std::endl;
        
        if (std::filesystem::exists("assets")) {
            std::cout << "[DEBUG] Contents of 'assets' directory:" << std::endl;
            for (const auto& entry : std::filesystem::recursive_directory_iterator("assets")) {
                std::cout << "  " << entry.path().string() << std::endl;
            }
        } else {
            std::cerr << "[ERROR] 'assets' directory does not exist!" << std::endl;
        }
    }
    
    tileMap->loadFromTiled(mapPath, 48);
}

void GameState::handleInput(const sf::Event& event) {
    // Handle UI input first (inventory panel has priority)
    if (inventoryPanel) {
        inventoryPanel->handleEvent(event, game->getWindow());
        // 如果背包打开，不处理其他输入
        if (inventoryPanel->isOpen()) {
            return;
        }
    }
    
    if (statsPanel) {
        statsPanel->handleEvent(event, game->getWindow());
    }
    
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Escape:
                // 如果背包打开，先关闭背包
                if (inventoryPanel && inventoryPanel->isOpen()) {
                    inventoryPanel->close();
                } else {
                    requestPop();
                }
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
                    int oldLevel = player->getStats().getLevel();
                    player->getStats().addExp(50);
                    int newLevel = player->getStats().getLevel();
                    
                    if (eventLogPanel) {
                        eventLogPanel->addExpObtained(50, "测试");
                        if (newLevel > oldLevel) {
                            eventLogPanel->addLevelUp(newLevel);
                        }
                    }
                    std::cout << "[DEBUG] +50 EXP" << std::endl;
                }
                break;
                
            case sf::Keyboard::G:
                if (player) {
                    player->getStats().addGold(100);
                    if (eventLogPanel) {
                        eventLogPanel->addGoldObtained(100);
                    }
                    std::cout << "[DEBUG] +100 Gold" << std::endl;
                }
                break;
                
            // Debug: Add test items
            case sf::Keyboard::T:
                if (inventory) {
                    inventory->addItem("wood", 10);
                    inventory->addItem("cherry", 5);
                    if (eventLogPanel) {
                        eventLogPanel->addItemObtained("木材", 10, "wood");
                        eventLogPanel->addItemObtained("樱桃", 5, "cherry");
                    }
                    std::cout << "[DEBUG] Added test items" << std::endl;
                }
                break;
                
            default:
                break;
        }
    }
}

void GameState::update(float dt) {
    // 如果背包打开，暂停游戏逻辑
    if (inventoryPanel && inventoryPanel->isOpen()) {
        inventoryPanel->update(dt);
        return;
    }
    
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
        
        // Handle item pickup
        handleItemPickup();
        
        // Update stats panel with current player stats
        if (statsPanel) {
            statsPanel->updateStats(player->getStats());
        }
        
        // Update inventory panel gold display
        if (inventoryPanel) {
            inventoryPanel->setGold(player->getStats().getGold());
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
    
    // Update dropped items
    if (droppedItemManager) {
        droppedItemManager->update(dt);
    }
    
    // Update UI
    if (statsPanel) {
        statsPanel->update(dt);
    }
    if (inventoryPanel) {
        inventoryPanel->update(dt);
    }
    if (eventLogPanel) {
        eventLogPanel->update(dt);
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
    
    // Render dropped items (below trees)
    if (droppedItemManager && camera) {
        droppedItemManager->render(window, camera->getView());
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
    if (treeManager && camera && !(inventoryPanel && inventoryPanel->isOpen())) {
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
    
    // Render inventory panel
    if (inventoryPanel) {
        inventoryPanel->render(window);
    }
    
    // Render event log panel (top-right)
    if (eventLogPanel) {
        eventLogPanel->render(window);
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
        
        // Clear dropped items
        if (droppedItemManager) {
            droppedItemManager->clearAll();
        }
        
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
        
        // 添加地图切换提示到事件日志
        if (eventLogPanel) {
            std::string mapName = (newMap == MapType::Farm) ? "农场" : "森林";
            eventLogPanel->addMessage("已进入 " + mapName + " 地图", EventType::System);
        }
    }
}

void GameState::initTrees() {
    if (!treeManager || !tileMap) return;
    
    const auto& objects = tileMap->getObjects();
    float displayScale = (float)tileMap->getTileSize() / 32.0f;
    
    std::cout << "[Trees] Loading trees from " << objects.size() << " map objects..." << std::endl;
    
    for (const auto& obj : objects) {
        if (obj.gid <= 0) continue;
        
        std::string objType = obj.type;
        std::string objName = obj.name;
        
        if (objType.empty() && obj.tileProperty) {
            objType = obj.tileProperty->type;
        }
        if (objName.empty() && obj.tileProperty) {
            objName = obj.tileProperty->name;
        }
        
        bool isTree = false;
        if (objType == "tree") {
            isTree = true;
        } else if (objName.find("tree") != std::string::npos) {
            isTree = true;
        }
        
        if (!isTree) {
            std::cout << "[Objects] Skipping non-tree object: " << objName 
                      << " (type=" << objType << ")" << std::endl;
            continue;
        }
        
        float x = obj.x * displayScale;
        float y = obj.y * displayScale;
        float width = obj.width * displayScale;
        float height = obj.height * displayScale;
        
        Tree* tree = nullptr;
        
        if (obj.tileProperty) {
            tree = treeManager->addTreeFromProperty(x, y, obj.tileProperty);
            std::cout << "[Trees] Created from TileProperty: " << obj.tileProperty->name 
                      << " HP=" << obj.tileProperty->hp << std::endl;
        } else {
            std::string treeType = obj.name.empty() ? "tree1" : obj.name;
            tree = treeManager->addTree(x, y, treeType);
            std::cout << "[Trees] Created from name: " << treeType << std::endl;
        }
        
        if (tree) {
            tree->setSize(width, height);
            
            // 注意：不再自动设置 canTransform
            // 如果需要树木变换功能，取消下面的注释
            // if (tree->getTreeType() == "tree1") {
            //     tree->setCanTransform(true);
            // }
            
            // ========================================
            // 设置销毁回调 - 生成掉落物品和奖励
            // ========================================
            tree->setOnDestroyed([this](Tree& t) {
                // 使用递减概率计算掉落
                auto drops = t.generateDrops();
                
                if (!drops.empty() && droppedItemManager) {
                    sf::Vector2f treePos = t.getPosition();
                    // 在树的位置生成掉落物品
                    droppedItemManager->spawnItems(drops, treePos.x, treePos.y - 20);
                    
                    // 添加掉落物品到事件日志
                    if (eventLogPanel) {
                        for (const auto& drop : drops) {
                            const ItemData* data = ItemDatabase::getInstance().getItemData(drop.first);
                            if (data) {
                                eventLogPanel->addItemObtained(data->name, drop.second, drop.first);
                            }
                        }
                    }
                }
                
                // 添加经验和金币奖励（从 tsx 配置读取）
                if (player) {
                    int exp = t.getExpReward();
                    int gold = t.getGoldReward();
                    
                    // 检查是否升级
                    int oldLevel = player->getStats().getLevel();
                    player->getStats().addExp(exp);
                    int newLevel = player->getStats().getLevel();
                    
                    player->getStats().addGold(gold);
                    player->getStats().addSkillExp(LifeSkill::Farming, exp / 2);
                    
                    // 添加到事件日志
                    if (eventLogPanel) {
                        eventLogPanel->addTreeChopped(t.getName());
                        if (exp > 0) {
                            eventLogPanel->addExpObtained(exp, "砍伐");
                        }
                        if (gold > 0) {
                            eventLogPanel->addGoldObtained(gold);
                        }
                        if (newLevel > oldLevel) {
                            eventLogPanel->addLevelUp(newLevel);
                        }
                    }
                    
                    std::cout << "[Reward] +" << exp << " EXP, +" << gold << " Gold" << std::endl;
                }
            });
            
            // ========================================
            // 设置果实采摘回调 - 生成果实掉落
            // ========================================
            tree->setOnFruitHarvested([this](Tree& t) {
                auto drops = t.generateFruitDrops();
                
                if (!drops.empty() && droppedItemManager) {
                    sf::Vector2f treePos = t.getPosition();
                    droppedItemManager->spawnItems(drops, treePos.x, treePos.y - 20);
                    
                    // 添加到事件日志
                    if (eventLogPanel) {
                        for (const auto& drop : drops) {
                            const ItemData* data = ItemDatabase::getInstance().getItemData(drop.first);
                            if (data) {
                                eventLogPanel->addFruitHarvested(data->name, drop.second);
                            }
                        }
                    }
                }
                
                if (player) {
                    int exp = t.getExpReward() / 2;  // 采摘经验减半
                    int oldLevel = player->getStats().getLevel();
                    player->getStats().addExp(exp);
                    int newLevel = player->getStats().getLevel();
                    
                    if (eventLogPanel && exp > 0) {
                        eventLogPanel->addExpObtained(exp, "采摘");
                        if (newLevel > oldLevel) {
                            eventLogPanel->addLevelUp(newLevel);
                        }
                    }
                }
            });
            
            tree->setOnGrowthStageChanged([this](Tree& t) {
                std::cout << "[Tree] " << t.getName() << " changed to: " 
                          << t.getGrowthStageName() << std::endl;
                
                // 如果树木成熟，添加事件日志
                if (eventLogPanel && t.getGrowthStageName() == "Mature") {
                    eventLogPanel->addTreeMature(t.getName());
                }
            });
        }
    }
    
    tileMap->clearObjects();
    
    std::cout << "[Trees] Total trees loaded: " << treeManager->getTreeCount() << std::endl;
}

void GameState::handlePlayerAttack() {
    if (!player || !treeManager) return;
    
    bool isCurrentlyAttacking = player->isAttacking();
    
    if (isCurrentlyAttacking && !wasAttacking) {
        sf::Vector2f attackCenter = player->getAttackCenter();
        float attackRadius = player->getAttackRadius();
        float damage = player->performAttack();
        
        auto hitTrees = treeManager->damageTreesInRange(attackCenter, attackRadius, damage);
        
        if (!hitTrees.empty()) {
            std::cout << "[Attack] Hit " << hitTrees.size() << " tree(s) for " 
                      << damage << " damage" << std::endl;
        }
    }
    
    wasAttacking = isCurrentlyAttacking;
}

void GameState::handleItemPickup() {
    if (!player || !droppedItemManager) return;
    
    sf::Vector2f playerPos = player->getPosition();
    
    // 自动拾取范围内的物品
    auto pickedUp = droppedItemManager->pickupItemsInRange(playerPos, PICKUP_RANGE);
    
    // 物品会通过回调自动添加到背包
}

bool GameState::onUseItem(const ItemStack& item, const ItemData* data) {
    if (!player || !data) return false;
    
    // 处理消耗品效果
    for (const auto& effect : data->effects) {
        switch (effect.type) {
            case EffectType::RestoreHealth:
                player->getStats().heal(effect.value);
                if (eventLogPanel) {
                    eventLogPanel->addMessage("恢复 " + std::to_string((int)effect.value) + " 生命值", EventType::System);
                }
                std::cout << "[Effect] Restored " << effect.value << " HP" << std::endl;
                break;
                
            case EffectType::RestoreStamina:
                player->getStats().restoreStamina(effect.value);
                if (eventLogPanel) {
                    eventLogPanel->addMessage("恢复 " + std::to_string((int)effect.value) + " 体力", EventType::System);
                }
                std::cout << "[Effect] Restored " << effect.value << " Stamina" << std::endl;
                break;
                
            case EffectType::BuffAttack:
                // TODO: 实现增益效果
                if (eventLogPanel) {
                    eventLogPanel->addMessage("攻击力提升 +" + std::to_string((int)effect.value), EventType::Combat);
                }
                std::cout << "[Effect] Attack buff +" << effect.value << std::endl;
                break;
                
            default:
                break;
        }
    }
    
    return true;
}

std::string GameState::getMapName(MapType mapType) const {
    switch (mapType) {
        case MapType::Farm:   return "Farm";
        case MapType::Forest: return "Forest";
        default:              return "Unknown";
    }
}