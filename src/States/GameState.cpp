#include "GameState.h"
#include "../Core/Game.h"
#include <iostream>
#include <filesystem>  // For path debugging
#include "../Entity/Rabbit.h"

GameState::GameState(Game* game, MapType mapType) 
    : State(game)
    , currentMap(mapType)
    , wasAttacking(false)
    , rng(std::random_device{}())
{
    // Initialize available tree types from tree.tsx
    availableTreeTypes = {"tree1", "apple_tree", "cherry_tree", "cherry_blossom_tree"};
    
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
    
    // Create RabbitManager
    rabbitManager = std::make_unique<RabbitManager>();
    rabbitManager->init("../../assets/rabbit_spritesheet.png");
    
    // Load map
    loadMap(mapType);
    
    // Initialize trees from map objects
    initTrees();
    
    // Initialize rabbits
    initRabbits();
    
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
    std::cout << "  Tab         - Toggle Stats Panel" << std::endl;
    std::cout << "  I/B         - Toggle Inventory (3 categories)" << std::endl;
    std::cout << "  1/2/3       - Switch inventory category" << std::endl;
    std::cout << "  E           - Toggle Equipment Panel" << std::endl;
    std::cout << "  C           - Toggle Crafting Workshop" << std::endl;
    std::cout << "  F1 - Farm Map | F2 - Forest Map | F3 - Reload" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "========================================\n" << std::endl;
}

void GameState::initItemSystem() {
    std::cout << "[ItemSystem] Initializing..." << std::endl;
    
    // 初始化物品数据库
    ItemDatabase::getInstance().initialize();
    ItemDatabase::getInstance().loadTextures("");
    
    // 初始化装备管理器
    EquipmentManager::getInstance().initialize();
    
    // 初始化合成管理器
    CraftingManager::getInstance().initialize();
    
    // 创建分类背包
    categoryInventory = std::make_unique<CategoryInventory>();
    
    // 创建玩家装备栏
    playerEquipment = std::make_unique<PlayerEquipment>();
    
    // 设置使用物品回调
    categoryInventory->setOnUseItem([this](const ItemStack& item, const ItemData* data) {
        return onUseItem(item, data);
    });
    
    // 设置卖出物品回调
    categoryInventory->setOnSellItem([this](const ItemStack& item, int sellPrice) {
        onSellItem(item, sellPrice);
    });
    
    // 设置种植种子回调
    categoryInventory->setOnPlantSeed([this](const ItemStack& seed) {
        return onPlantSeed(seed);
    });
    
    // 设置装备物品回调
    categoryInventory->setOnEquipItem([this](const ItemStack& item) {
        if (!playerEquipment) return false;
        
        const EquipmentData* equipData = EquipmentManager::getInstance().getEquipmentData(item.itemId);
        if (!equipData) return false;
        
        // 检查等级需求
        if (player && player->getStats().getLevel() < equipData->requiredLevel) {
            if (eventLogPanel) {
                eventLogPanel->addWarning("等级不足，需要 Lv." + std::to_string(equipData->requiredLevel));
            }
            return false;
        }
        
        // 尝试装备，获取旧装备
        ItemStack oldItem = playerEquipment->equip(*equipData);
        
        if (eventLogPanel) {
            eventLogPanel->addMessage("装备了 " + equipData->name, EventType::System);
        }
        
        // 如果有旧装备，放回背包
        if (!oldItem.isEmpty()) {
            if (eventLogPanel) {
                const ItemData* oldData = ItemDatabase::getInstance().getItemData(oldItem.itemId);
                if (oldData) {
                    eventLogPanel->addMessage("卸下了 " + oldData->name, EventType::System);
                }
            }
            categoryInventory->addItem(oldItem.itemId, oldItem.count);
        }
        
        std::cout << "[Equip] Equipped " << item.itemId << std::endl;
        return true;
    });
    
    // 设置物品添加回调（用于显示提示）
    categoryInventory->setOnItemAdded([](const ItemStack& item, int slotIndex, InventoryCategory category) {
        (void)slotIndex;  // 未使用参数
        (void)category;   // 未使用参数
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
        // 添加到分类背包
        int added = categoryInventory->addItem(item.itemId, item.count);
        
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
    
    // 注意：合成成功回调已在 initUI() 中通过 craftingPanel->setOnCraftSuccess() 设置
    
    // 测试：给玩家一些初始物品
    categoryInventory->addItem("wood", 10);
    categoryInventory->addItem("stick", 5);
    categoryInventory->addItem("stone", 5);
    categoryInventory->addItem("apple", 3);
    categoryInventory->addItem("seed", 3);
    
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
    // 创建分类背包面板
    // ========================================
    categoryInventoryPanel = std::make_unique<CategoryInventoryPanel>();
    categoryInventoryPanel->init("../../assets/ui/inventory_icon.png");
    categoryInventoryPanel->setInventory(categoryInventory.get());
    categoryInventoryPanel->setPlayerEquipment(playerEquipment.get());
    categoryInventoryPanel->setIconPosition(90.0f, windowSize.y - 80.0f);
    
    // 设置丢弃物品回调
    categoryInventoryPanel->setOnDropItem([this](const ItemStack& item, sf::Vector2f pos) {
        if (player && droppedItemManager) {
            sf::Vector2f playerPos = player->getPosition();
            droppedItemManager->spawnItem(item.itemId, item.count, playerPos.x + 30, playerPos.y);
        }
    });
    
    // ========================================
    // 创建装备栏面板
    // ========================================
    equipmentPanel = std::make_unique<EquipmentPanel>();
    equipmentPanel->init("../../assets/ui/equipment_icon.png");
    equipmentPanel->setPlayerEquipment(playerEquipment.get());
    equipmentPanel->setIconPosition(160.0f, windowSize.y - 80.0f);
    
    // 设置卸下装备回调
    equipmentPanel->setOnUnequip([this](EquipmentSlot slot) {
        onUnequipItem(slot);
    });
    
    // ========================================
    // 创建工作台/合成面板
    // ========================================
    craftingPanel = std::make_unique<CraftingPanel>();
    craftingPanel->init("../../assets/ui/crafting_icon.png");
    craftingPanel->setInventory(categoryInventory.get());
    craftingPanel->setIconPosition(230.0f, windowSize.y - 80.0f);
    
    // 设置合成成功回调
    craftingPanel->setOnCraftSuccess([this](const std::string& itemId, int count) {
        // 添加到背包
        int added = categoryInventory->addItem(itemId, count);
        
        if (eventLogPanel) {
            const ItemData* data = ItemDatabase::getInstance().getItemData(itemId);
            if (data) {
                eventLogPanel->addMessage("合成: " + data->name + " x" + std::to_string(count), EventType::System);
            }
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
    eventLogPanel->addMessage("按 I/B 打开背包", EventType::System);
    eventLogPanel->addMessage("按 E 打开装备栏", EventType::System);
    eventLogPanel->addMessage("按 C 打开工作台", EventType::System);
    
    std::cout << "[OK] UI initialized" << std::endl;
    std::cout << "  - I/B: 分类背包 (材料/消耗品/装备)" << std::endl;
    std::cout << "  - E: 装备栏" << std::endl;
    std::cout << "  - C: 工作台/合成" << std::endl;
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
    // 处理分类背包面板事件
    if (categoryInventoryPanel) {
        categoryInventoryPanel->handleEvent(event, game->getWindow());
        if (categoryInventoryPanel->isOpen()) {
            return;
        }
    }
    
    // 处理装备面板事件
    if (equipmentPanel) {
        equipmentPanel->handleEvent(event, game->getWindow());
        if (equipmentPanel->isOpen()) {
            return;
        }
    }
    
    // 处理合成面板事件
    if (craftingPanel) {
        craftingPanel->handleEvent(event, game->getWindow());
        if (craftingPanel->isOpen()) {
            return;
        }
    }
    
    if (statsPanel) {
        statsPanel->handleEvent(event, game->getWindow());
    }
    
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Escape:
                // 如果任何面板打开，先关闭
                if (categoryInventoryPanel && categoryInventoryPanel->isOpen()) {
                    categoryInventoryPanel->close();
                } else if (equipmentPanel && equipmentPanel->isOpen()) {
                    equipmentPanel->close();
                } else if (craftingPanel && craftingPanel->isOpen()) {
                    craftingPanel->close();
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
            
            // 装备面板快捷键
            case sf::Keyboard::E:
                if (equipmentPanel) {
                    equipmentPanel->toggle();
                }
                break;
            
            // 合成面板快捷键
            case sf::Keyboard::C:
                if (craftingPanel) {
                    craftingPanel->toggle();
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
                if (categoryInventory) {
                    categoryInventory->addItem("wood", 10);
                    categoryInventory->addItem("cherry", 5);
                    categoryInventory->addItem("stone", 5);
                    categoryInventory->addItem("stick", 10);
                    if (eventLogPanel) {
                        eventLogPanel->addItemObtained("木材", 10, "wood");
                        eventLogPanel->addItemObtained("樱桃", 5, "cherry");
                        eventLogPanel->addItemObtained("石头", 5, "stone");
                        eventLogPanel->addItemObtained("树枝", 10, "stick");
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
    // 检查任何面板是否打开
    bool anyPanelOpen = (categoryInventoryPanel && categoryInventoryPanel->isOpen()) ||
                        (equipmentPanel && equipmentPanel->isOpen()) ||
                        (craftingPanel && craftingPanel->isOpen());
    
    // 如果任何面板打开，更新面板但暂停游戏逻辑
    if (anyPanelOpen) {
        if (categoryInventoryPanel) categoryInventoryPanel->update(dt);
        if (equipmentPanel) equipmentPanel->update(dt);
        if (craftingPanel) craftingPanel->update(dt);
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
        
        // Collision detection with rabbits
        if (rabbitManager && rabbitManager->isCollidingWithAnyRabbit(player->getCollisionBox())) {
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
        
        // Update category inventory panel gold display
        if (categoryInventoryPanel) {
            categoryInventoryPanel->setGold(player->getStats().getGold());
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
    
    // Update rabbits
    if (rabbitManager && player) {
        rabbitManager->update(dt, player->getPosition());
    }
    
    // Update dropped items
    if (droppedItemManager) {
        droppedItemManager->update(dt);
    }
    
    // Update UI
    if (statsPanel) {
        statsPanel->update(dt);
    }
    if (categoryInventoryPanel) {
        categoryInventoryPanel->update(dt);
    }
    if (equipmentPanel) {
        equipmentPanel->update(dt);
    }
    if (craftingPanel) {
        craftingPanel->update(dt);
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
    
    // Render rabbits
    if (rabbitManager && camera) {
        rabbitManager->render(window, camera->getView());
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
    bool anyPanelOpen = (categoryInventoryPanel && categoryInventoryPanel->isOpen()) ||
                        (equipmentPanel && equipmentPanel->isOpen()) ||
                        (craftingPanel && craftingPanel->isOpen());
    if (camera && !anyPanelOpen) {
        sf::Vector2i mouseScreenPos = sf::Mouse::getPosition(window);
        sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mouseScreenPos, camera->getView());
        
        if (treeManager) {
            treeManager->renderTooltips(window, mouseWorldPos);
        }
        if (rabbitManager) {
            rabbitManager->renderTooltips(window, mouseWorldPos);
        }
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
    
    // Render category inventory panel
    if (categoryInventoryPanel) {
        categoryInventoryPanel->render(window);
    }
    
    // Render equipment panel
    if (equipmentPanel) {
        equipmentPanel->render(window);
    }
    
    // Render crafting panel
    if (craftingPanel) {
        craftingPanel->render(window);
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

void GameState::initRabbits() {
    if (!rabbitManager || !tileMap) return;
    
    // 在当前地图随机生成20只兔子
    sf::Vector2i mapSize = tileMap->getMapSize();
    int tileSize = tileMap->getTileSize();
    
    rabbitManager->spawnRandomRabbits(20, mapSize, tileSize);
    
    // 设置兔子攻击玩家的回调
    for (auto& rabbit : rabbitManager->getRabbits()) {
        rabbit->setOnAttack([this](Rabbit& r) {
            if (!player) return;
            
            float damage = r.performAttack();
            bool usedSkill = r.hasTriggeredSkill();
            
            // 玩家闪避判定
            if (player->getStats().rollDodge(0)) {
                if (eventLogPanel) {
                    eventLogPanel->addMessage("闪避了 " + r.getName() + " 的攻击!", EventType::Combat);
                }
                std::cout << "[Combat] Player dodged rabbit attack!" << std::endl;
                return;
            }
            
            // 计算实际伤害
            float actualDamage = player->getStats().calculateDamageTaken(damage);
            
            // 获取兔子位置用于击退计算
            sf::Vector2f rabbitPos = r.getPosition();
            sf::FloatRect rabbitBounds = r.getBounds();
            sf::Vector2f rabbitCenter(rabbitPos.x + rabbitBounds.width / 2.0f, 
                                      rabbitPos.y + rabbitBounds.height / 2.0f);
            
            // 受伤并击退（传入攻击者位置）
            player->receiveDamage(actualDamage, rabbitCenter);
            
            if (eventLogPanel) {
                std::string attackMsg;
                if (usedSkill) {
                    const RabbitSkill& skill = r.getSkill();
                    attackMsg = r.getName() + " 使用了 [" + skill.name + "]! -" + 
                               std::to_string(static_cast<int>(actualDamage)) + " HP";
                } else {
                    attackMsg = r.getName() + " 攻击了你! -" + 
                               std::to_string(static_cast<int>(actualDamage)) + " HP";
                }
                eventLogPanel->addMessage(attackMsg, EventType::Combat);
            }
            
            std::cout << "[Combat] Rabbit attacked player for " << actualDamage << " damage"
                      << (usedSkill ? " (SKILL!)" : "") << std::endl;
            
            if (player->isDead()) {
                if (eventLogPanel) {
                    eventLogPanel->addMessage("你被击败了...", EventType::Combat);
                }
            }
        });
    }
    
    std::cout << "[Rabbits] Spawned " << rabbitManager->getRabbitCount() << " rabbits" << std::endl;
}

void GameState::handlePlayerAttack() {
    if (!player) return;
    
    bool isCurrentlyAttacking = player->isAttacking();
    
    if (isCurrentlyAttacking && !wasAttacking) {
        sf::Vector2f attackCenter = player->getAttackCenter();
        float attackRadius = player->getAttackRadius();
        float damage = player->performAttack();
        
        // 检查是否装备了无视防御的武器（斧头）
        bool ignoreDefense = false;
        if (playerEquipment && playerEquipment->hasIgnoreDefense()) {
            ignoreDefense = true;
        }
        
        // 攻击树木
        if (treeManager) {
            auto hitTrees = treeManager->damageTreesInRange(attackCenter, attackRadius, damage);
            
            if (!hitTrees.empty()) {
                std::cout << "[Attack] Hit " << hitTrees.size() << " tree(s) for " 
                          << damage << " damage" << (ignoreDefense ? " (ignore defense)" : "") << std::endl;
            }
        }
        
        // 攻击兔子
        if (rabbitManager) {
            auto hitRabbits = rabbitManager->damageRabbitsInRange(attackCenter, attackRadius, damage, ignoreDefense);
            
            for (auto* rabbit : hitRabbits) {
                if (rabbit->isDead()) {
                    // 兔子死亡，生成掉落物
                    auto drops = rabbit->generateDrops();
                    
                    if (!drops.empty() && droppedItemManager) {
                        sf::Vector2f rabbitPos = rabbit->getPosition();
                        droppedItemManager->spawnItems(drops, rabbitPos.x, rabbitPos.y);
                        
                        // 添加到事件日志
                        if (eventLogPanel) {
                            eventLogPanel->addMessage("击杀了 " + rabbit->getName(), EventType::Combat);
                            for (const auto& drop : drops) {
                                const ItemData* data = ItemDatabase::getInstance().getItemData(drop.first);
                                if (data) {
                                    eventLogPanel->addItemObtained(data->name, drop.second, drop.first);
                                }
                            }
                        }
                    }
                    
                    // 获得经验和金币
                    int exp = rabbit->getExpReward();
                    int gold = rabbit->getGoldReward();
                    
                    int oldLevel = player->getStats().getLevel();
                    player->getStats().addExp(exp);
                    int newLevel = player->getStats().getLevel();
                    player->getStats().addGold(gold);
                    
                    if (eventLogPanel) {
                        if (exp > 0) eventLogPanel->addExpObtained(exp, "击杀");
                        if (gold > 0) eventLogPanel->addGoldObtained(gold);
                        if (newLevel > oldLevel) eventLogPanel->addLevelUp(newLevel);
                    }
                    
                    std::cout << "[Rabbit] Killed! +" << exp << " EXP, +" << gold << " Gold" << std::endl;
                }
            }
            
            if (!hitRabbits.empty()) {
                std::cout << "[Attack] Hit " << hitRabbits.size() << " rabbit(s) for " 
                          << damage << " damage" << std::endl;
            }
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

// ============================================================================
// 卖出物品回调
// ============================================================================
void GameState::onSellItem(const ItemStack& item, int sellPrice) {
    if (!player) return;
    
    player->getStats().addGold(sellPrice);
    
    if (eventLogPanel) {
        const ItemData* data = ItemDatabase::getInstance().getItemData(item.itemId);
        if (data) {
            eventLogPanel->addMessage("卖出 " + data->name + " x" + std::to_string(item.count) + 
                                     " 获得 " + std::to_string(sellPrice) + " 金币", EventType::System);
        }
        eventLogPanel->addGoldObtained(sellPrice);
    }
    
    std::cout << "[Sell] Sold " << item.itemId << " x" << item.count 
              << " for " << sellPrice << " gold" << std::endl;
}

// ============================================================================
// 种植种子回调 - 随机生成tree.tsx中的树木类型
// ============================================================================
bool GameState::onPlantSeed(const ItemStack& seed) {
    if (!player || !treeManager) return false;
    
    sf::Vector2f playerPos = player->getPosition();
    
    // 检查玩家周围是否可以种植（简单检查）
    // TODO: 更复杂的地形检测
    
    // 随机选择树木类型
    std::uniform_int_distribution<int> dist(0, availableTreeTypes.size() - 1);
    std::string treeType = availableTreeTypes[dist(rng)];
    
    // 在玩家前方种植
    float plantX = playerPos.x + 50;
    float plantY = playerPos.y;
    
    // 创建树木
    Tree* newTree = treeManager->addTree(plantX, plantY, treeType);
    
    if (newTree) {
        newTree->setSize(64, 64);
        
        // 设置销毁回调
        newTree->setOnDestroyed([this](Tree& t) {
            auto drops = t.generateDrops();
            
            if (!drops.empty() && droppedItemManager) {
                sf::Vector2f treePos = t.getPosition();
                droppedItemManager->spawnItems(drops, treePos.x, treePos.y - 20);
                
                if (eventLogPanel) {
                    for (const auto& drop : drops) {
                        const ItemData* data = ItemDatabase::getInstance().getItemData(drop.first);
                        if (data) {
                            eventLogPanel->addItemObtained(data->name, drop.second, drop.first);
                        }
                    }
                }
            }
            
            if (player) {
                int exp = t.getExpReward();
                int gold = t.getGoldReward();
                
                int oldLevel = player->getStats().getLevel();
                player->getStats().addExp(exp);
                int newLevel = player->getStats().getLevel();
                
                player->getStats().addGold(gold);
                
                if (eventLogPanel) {
                    eventLogPanel->addTreeChopped(t.getName());
                    if (exp > 0) eventLogPanel->addExpObtained(exp, "砍伐");
                    if (gold > 0) eventLogPanel->addGoldObtained(gold);
                    if (newLevel > oldLevel) eventLogPanel->addLevelUp(newLevel);
                }
            }
        });
        
        // 设置果实采摘回调
        newTree->setOnFruitHarvested([this](Tree& t) {
            auto drops = t.generateFruitDrops();
            
            if (!drops.empty() && droppedItemManager) {
                sf::Vector2f treePos = t.getPosition();
                droppedItemManager->spawnItems(drops, treePos.x, treePos.y - 20);
                
                if (eventLogPanel) {
                    for (const auto& drop : drops) {
                        const ItemData* data = ItemDatabase::getInstance().getItemData(drop.first);
                        if (data) {
                            eventLogPanel->addFruitHarvested(data->name, drop.second);
                        }
                    }
                }
            }
        });
        
        if (eventLogPanel) {
            eventLogPanel->addMessage("种下了种子，长出了 " + newTree->getName(), EventType::System);
        }
        
        std::cout << "[Plant] Planted seed, grew into " << treeType << " at (" 
                  << plantX << ", " << plantY << ")" << std::endl;
        
        return true;
    }
    
    return false;
}

// ============================================================================
// 装备物品回调
// ============================================================================
void GameState::onEquipItem(const ItemStack& item) {
    if (!playerEquipment) return;
    
    const EquipmentData* equipData = EquipmentManager::getInstance().getEquipmentData(item.itemId);
    if (!equipData) return;
    
    // 检查等级需求
    if (player && player->getStats().getLevel() < equipData->requiredLevel) {
        if (eventLogPanel) {
            eventLogPanel->addWarning("等级不足，需要 Lv." + std::to_string(equipData->requiredLevel));
        }
        return;
    }
    
    // 尝试装备
    ItemStack oldItem = playerEquipment->equip(*equipData);
    
    if (eventLogPanel) {
        eventLogPanel->addMessage("装备了 " + equipData->name, EventType::System);
        
        if (!oldItem.isEmpty()) {
            const ItemData* oldData = ItemDatabase::getInstance().getItemData(oldItem.itemId);
            if (oldData) {
                eventLogPanel->addMessage("卸下了 " + oldData->name, EventType::System);
            }
            // 将旧装备放回背包
            categoryInventory->addItem(oldItem.itemId, oldItem.count);
        }
    }
    
    std::cout << "[Equip] Equipped " << item.itemId << std::endl;
}

// ============================================================================
// 卸下装备回调
// ============================================================================
void GameState::onUnequipItem(EquipmentSlot slot) {
    if (!playerEquipment || !categoryInventory) return;
    
    ItemStack unequipped = playerEquipment->unequipToStack(slot);
    
    if (!unequipped.isEmpty()) {
        // 放回背包
        int added = categoryInventory->addItem(unequipped.itemId, unequipped.count);
        
        if (eventLogPanel) {
            const ItemData* data = ItemDatabase::getInstance().getItemData(unequipped.itemId);
            if (data) {
                eventLogPanel->addMessage("卸下了 " + data->name, EventType::System);
            }
            
            if (added < unequipped.count) {
                eventLogPanel->addWarning("背包已满，部分装备无法放入！");
            }
        }
        
        std::cout << "[Unequip] Unequipped " << unequipped.itemId << std::endl;
    }
}