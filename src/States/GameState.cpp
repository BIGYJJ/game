#include "GameState.h"
#include "../Core/Game.h"
#include <iostream>
#include <filesystem>  // For path debugging
#include <cmath>       // For sqrt in collision
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
    
    // Initialize stone builds from map objects
    stoneBuildManager = std::make_unique<StoneBuildManager>();
    stoneBuildManager->init("../../assets");
    initStoneBuilds();
    tileMap->removeStoneObjects();  // 由StoneBuildManager接管渲染
    
    // Initialize wild plants from map objects
    wildPlantManager = std::make_unique<WildPlantManager>();
    wildPlantManager->init("../../assets");
    initWildPlants();
    tileMap->removeWildPlantObjects();  // 由WildPlantManager接管渲染
    
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
    
    // Initialize Pet System
    initPetSystem();
    
    std::cout << "[OK] Player Position: (" << player->getPosition().x 
              << ", " << player->getPosition().y << ")" << std::endl;
    std::cout << "\nControls:" << std::endl;
    std::cout << "  WASD/Arrows - Move" << std::endl;
    std::cout << "  Space       - Attack (chop trees, mine stones!)" << std::endl;
    std::cout << "  V           - Pickup wild plants" << std::endl;
    std::cout << "  Tab         - Toggle Stats Panel" << std::endl;
    std::cout << "  I/B         - Toggle Inventory (3 categories)" << std::endl;
    std::cout << "  1/2/3       - Switch inventory category" << std::endl;
    std::cout << "  E           - Toggle Equipment Panel" << std::endl;
    std::cout << "  C           - Toggle Crafting Workshop" << std::endl;
    std::cout << "  P           - Toggle Pet Panel" << std::endl;
    std::cout << "  H           - Toggle Hatch Panel" << std::endl;
    std::cout << "  F1 - Farm Map | F2 - Forest Map | F3 - Reload" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // 【调试】打印 TileMap 中剩余的对象（应该没有树木、石头、植物）
    if (tileMap) {
        tileMap->debugPrintRemainingObjects();
    }
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
    categoryInventory->addItem("rabbit_essence", 1);  // 初始赠送一个兔子精元
    categoryInventory->addItem("rabbit_fur", 10);     // 初始赠送一些兔毛作为强化剂
    
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
    std::cout << "  - P: 宠物栏" << std::endl;
    std::cout << "  - H: 孵化栏" << std::endl;
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
    // 记录面板状态（用于检测打开事件）
    bool petWasOpen = petPanel && petPanel->isOpen();
    bool hatchWasOpen = hatchPanel && hatchPanel->isOpen();
    
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
    
    // 处理宠物面板事件
    if (petPanel) {
        petPanel->handleEvent(event, game->getWindow());
        // 如果刚刚打开，更新物品数量
        if (!petWasOpen && petPanel->isOpen()) {
            updatePetPanelItemCounts();
        }
        if (petPanel->isOpen()) {
            return;
        }
    }
    
    // 处理孵化面板事件
    if (hatchPanel) {
        hatchPanel->handleEvent(event, game->getWindow());
        // 如果刚刚打开，更新物品数量
        if (!hatchWasOpen && hatchPanel->isOpen()) {
            updatePetPanelItemCounts();
        }
        if (hatchPanel->isOpen()) {
            return;
        }
    }
    
    if (statsPanel) {
        statsPanel->handleEvent(event, game->getWindow());
    }
    
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {
        handlePetCommand(event);
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
                } else if (petPanel && petPanel->isOpen()) {
                    petPanel->close();
                } else if (hatchPanel && hatchPanel->isOpen()) {
                    hatchPanel->close();
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
            
            // 宠物面板快捷键
            case sf::Keyboard::P:
                if (petPanel) {
                    petPanel->toggle();
                    // 每次打开都更新物品数量
                    updatePetPanelItemCounts();
                }
                break;
            
            // 孵化面板快捷键
            case sf::Keyboard::H:
                if (hatchPanel) {
                    hatchPanel->toggle();
                    // 每次打开都更新物品数量
                    updatePetPanelItemCounts();
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
                        (craftingPanel && craftingPanel->isOpen()) ||
                        (petPanel && petPanel->isOpen()) ||
                        (hatchPanel && hatchPanel->isOpen());
    
    // 如果任何面板打开，更新面板但暂停游戏逻辑
    if (anyPanelOpen) {
        if (categoryInventoryPanel) categoryInventoryPanel->update(dt);
        if (equipmentPanel) equipmentPanel->update(dt);
        if (craftingPanel) craftingPanel->update(dt);
        if (petPanel) petPanel->update(dt);
        if (hatchPanel) hatchPanel->update(dt);
        return;
    }
    
    if (player) {
        sf::Vector2f oldPos = player->getPosition();
        bool playerWasMoving = player->isMoving();  // 记录玩家是否在主动移动
        
        player->update(dt);
        
        // Collision detection with tilemap (地图碰撞仍然是阻挡型)
        if (tileMap->isColliding(player->getCollisionBox())) {
            player->setPosition(oldPos);
        }
        
        // Collision detection with trees (树木碰撞仍然是阻挡型)
        if (treeManager && treeManager->isCollidingWithAnyTree(player->getCollisionBox())) {
            player->setPosition(oldPos);
        }
        
        // 【新增】Collision detection with stone builds (石头碰撞检测)
        if (stoneBuildManager && stoneBuildManager->isCollidingWithAnyStone(player->getCollisionBox())) {
            player->setPosition(oldPos);
        }
        
        // ====================================================
        // 推挤碰撞处理：谁移动谁推开对方
        // ====================================================
        if (rabbitManager) {
            sf::FloatRect playerBox = player->getCollisionBox();
            
            if (playerWasMoving) {
                // 玩家主动移动 → 推开碰到的兔子
                rabbitManager->pushRabbitsFromRect(playerBox, 1.0f);
            } else {
                // 玩家没有移动 → 检查是否有兔子主动撞过来
                auto movingRabbits = rabbitManager->getMovingRabbitsCollidingWith(playerBox);
                
                for (Rabbit* rabbit : movingRabbits) {
                    // 兔子主动移动碰到玩家 → 推开玩家
                    sf::FloatRect rabbitBox = rabbit->getCollisionBox();
                    
                    sf::Vector2f playerCenter(
                        playerBox.left + playerBox.width / 2.0f,
                        playerBox.top + playerBox.height / 2.0f
                    );
                    sf::Vector2f rabbitCenter(
                        rabbitBox.left + rabbitBox.width / 2.0f,
                        rabbitBox.top + rabbitBox.height / 2.0f
                    );
                    
                    // 推挤方向：从兔子指向玩家
                    sf::Vector2f pushDir = playerCenter - rabbitCenter;
                    float length = std::sqrt(pushDir.x * pushDir.x + pushDir.y * pushDir.y);
                    
                    if (length > 0.001f) {
                        pushDir /= length;
                        
                        float overlapX = (playerBox.width + rabbitBox.width) / 2.0f - 
                                        std::abs(playerCenter.x - rabbitCenter.x);
                        float overlapY = (playerBox.height + rabbitBox.height) / 2.0f - 
                                        std::abs(playerCenter.y - rabbitCenter.y);
                        
                        float pushDistance = std::min(overlapX, overlapY) + 2.0f;
                        player->applyPush(pushDir * pushDistance);
                    }
                }
            }
        }
        
        // Boundary detection
        sf::Vector2f pos = player->getPosition();
        sf::Vector2i mapSize = tileMap->getMapSize();
        
        float margin = (float)tileMap->getTileSize();
        pos.x = std::max(margin, std::min(pos.x, mapSize.x - margin));
        pos.y = std::max(margin, std::min(pos.y, mapSize.y - margin));
        player->setPosition(pos);
        
        handlePlayerAttack();

        handlePetAttack();
        
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
    
    // Update stone builds
    if (stoneBuildManager) {
        stoneBuildManager->update(dt);
    }
    
    // Update wild plants
    if (wildPlantManager) {
        wildPlantManager->update(dt);
    }
    
    // Handle wild plant pickup (F key)
    handlePlantPickup();
    
    // Update rabbits
    if (rabbitManager && player) {
        rabbitManager->update(dt, player->getPosition());
    }
    
    // Update pet system
    if (petManager && player) {
        bool playerAttacking = player->isAttacking();
        petManager->update(dt, player->getPosition(), playerAttacking);
        
        // 宠物碰撞处理（与兔子的推挤碰撞）
        Pet* pet = petManager->getCurrentPet();
        if (pet && pet->isInCombat() && rabbitManager) {
            // 寻找最近的活兔子
            Rabbit* nearestRabbit = nullptr;
            float minDistance = 300.0f; // 索敌范围 300 像素
            sf::Vector2f petPos = pet->getPosition();
            
            for (auto& rabbit : rabbitManager->getRabbits()) {
                if (rabbit->isDead()) continue;
                
                sf::Vector2f rPos = rabbit->getPosition();
                float dx = rPos.x - petPos.x;
                float dy = rPos.y - petPos.y;
                float dist = std::sqrt(dx*dx + dy*dy);
                
                if (dist < minDistance) {
                    minDistance = dist;
                    nearestRabbit = rabbit.get();
                }
            }
            
            // 如果找到了最近的敌人，告诉宠物“去打它”
            if (nearestRabbit) {
                sf::FloatRect bounds = nearestRabbit->getBounds();
                sf::Vector2f targetCenter(bounds.left + bounds.width/2, bounds.top + bounds.height/2);
                
                // 使用普通的目标设置，而不是 commandAttack
                // 这样不会干扰玩家随后的右键点击
                pet->setAttackTarget(targetCenter); 
            }
        }

        if (pet && rabbitManager) {
            sf::FloatRect petBox = pet->getCollisionBox();
            
            // 宠物推开兔子
            rabbitManager->pushRabbitsFromRect(petBox, 0.8f);
            
            // 兔子也可能推开宠物（互相推挤）
            auto collidingRabbits = rabbitManager->getRabbitsCollidingWith(petBox);
            for (Rabbit* rabbit : collidingRabbits) {
                sf::FloatRect rabbitBox = rabbit->getCollisionBox();
                
                sf::Vector2f petCenter(
                    petBox.left + petBox.width / 2.0f,
                    petBox.top + petBox.height / 2.0f
                );
                sf::Vector2f rabbitCenter(
                    rabbitBox.left + rabbitBox.width / 2.0f,
                    rabbitBox.top + rabbitBox.height / 2.0f
                );
                
                // 推挤方向：从兔子指向宠物
                sf::Vector2f pushDir = petCenter - rabbitCenter;
                float length = std::sqrt(pushDir.x * pushDir.x + pushDir.y * pushDir.y);
                
                if (length > 0.001f) {
                    pushDir /= length;
                    
                    float overlapX = (petBox.width + rabbitBox.width) / 2.0f - 
                                    std::abs(petCenter.x - rabbitCenter.x);
                    float overlapY = (petBox.height + rabbitBox.height) / 2.0f - 
                                    std::abs(petCenter.y - rabbitCenter.y);
                    
                    float pushDistance = std::min(overlapX, overlapY) * 0.5f;
                    sf::Vector2f newPos = pet->getPosition() + pushDir * pushDistance;
                    pet->setPosition(newPos);
                }
            }
        }
        
        // 检查宠物物品掉落（如兔毛）
        std::string dropItem = petManager->checkPetItemDrop(dt);
        if (!dropItem.empty() && categoryInventory) {
            int added = categoryInventory->addItem(dropItem, 1);
            if (added > 0 && eventLogPanel) {
                const ItemData* data = ItemDatabase::getInstance().getItemData(dropItem);
                if (data) {
                    eventLogPanel->addItemObtained(data->name, 1, dropItem);
                }
            }
        }
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
    if (petPanel) {
        petPanel->update(dt);
    }
    if (hatchPanel) {
        hatchPanel->update(dt);
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
    
    // Render stone builds
    if (stoneBuildManager && camera) {
        stoneBuildManager->render(window, camera->getView());
    }
    
    // Render wild plants
    if (wildPlantManager && camera) {
        wildPlantManager->render(window, camera->getView());
    }
    
    // Render rabbits
    if (rabbitManager && camera) {
        rabbitManager->render(window, camera->getView());
    }
    
    // Render player
    if (player) {
        player->render(window);
    }
    
    // Render pet (follows player)
    if (petManager && camera) {
        petManager->render(window);
    }
    
    // Reset to default view for UI
    window.setView(window.getDefaultView());
    
    // Render UI overlay
    renderUI(window);
    
    // Render tree tooltips (needs world mouse position)
    bool anyPanelOpen = (categoryInventoryPanel && categoryInventoryPanel->isOpen()) ||
                        (equipmentPanel && equipmentPanel->isOpen()) ||
                        (craftingPanel && craftingPanel->isOpen()) ||
                        (petPanel && petPanel->isOpen()) ||
                        (hatchPanel && hatchPanel->isOpen());
    if (camera && !anyPanelOpen) {
        sf::Vector2i mouseScreenPos = sf::Mouse::getPosition(window);
        sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mouseScreenPos, camera->getView());
        
        if (treeManager) {
            treeManager->renderTooltips(window, mouseWorldPos);
        }
        if (rabbitManager) {
            rabbitManager->renderTooltips(window, mouseWorldPos);
        }
        if (stoneBuildManager) {
            stoneBuildManager->renderTooltips(window, mouseWorldPos);
        }
        if (wildPlantManager) {
            wildPlantManager->renderTooltips(window, mouseWorldPos);
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
    
    // Render pet panel
    if (petPanel) {
        petPanel->render(window);
    }
    
    // Render hatch panel
    if (hatchPanel) {
        hatchPanel->render(window);
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
        
        // Clear and reload stone builds
        if (stoneBuildManager) {
            stoneBuildManager->clearAllStones();
        }
        initStoneBuilds();
        tileMap->removeStoneObjects();  // 由StoneBuildManager接管渲染
        
        // Clear and reload wild plants
        if (wildPlantManager) {
            wildPlantManager->clearAllPlants();
        }
        initWildPlants();
        tileMap->removeWildPlantObjects();  // 由WildPlantManager接管渲染
        
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
            
            // 【修复】强制禁用变换功能，防止树在被砍时"复活"
            tree->setCanTransform(false);
            
            std::cout << "[Trees] Tree created: " << tree->getName() 
                      << " type=" << tree->getTreeType()
                      << " canTransform=" << tree->canBeTransformed() << std::endl;
            
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
    
    tileMap->removeTreeObjects();
    
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
        rabbit->setOnAttackRabbit([this](Rabbit& r){
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
            
            if (petManager) {
                Pet* pet = petManager->getCurrentPet();
                if (pet) {
                    pet->enterCombatMode(3.0f); // <--- 关键：激活持续战斗
                    
                    // 并且立刻把攻击者设为目标
                    pet->commandAttack(r.getPosition());
                    
                    if (eventLogPanel) {
                        eventLogPanel->addMessage(pet->getName() + " 护主心切，进入战斗状态!", EventType::Combat);
                    }
                }
            }

            if (eventLogPanel) {
                std::string attackMsg;
                if (usedSkill) {
                    const RabbitSkill& skill = r.getRabbitSkill();
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
            
            // 宠物帮忙反击
            if (petManager) {
                Pet* pet = petManager->getCurrentPet();
                if (pet && !pet->isDead()) {
                    // 设置攻击目标为攻击玩家的兔子
                    pet->setAttackTarget(rabbitPos);
                    
                    if (eventLogPanel) {
                        eventLogPanel->addMessage(pet->getName() + " 帮你反击!", EventType::Combat);
                    }
                }
            }
            
            if (player->isDead()) {
                if (eventLogPanel) {
                    eventLogPanel->addMessage("你被击败了...", EventType::Combat);
                }
            }
        });
    }
    
    std::cout << "[Rabbits] Spawned " << rabbitManager->getRabbitCount() << " rabbits" << std::endl;
}

// ============================================================================
// 初始化石头建筑
// ============================================================================
void GameState::initStoneBuilds() {
    if (!stoneBuildManager || !tileMap) return;
    
    const auto& objects = tileMap->getObjects();
    float displayScale = (float)tileMap->getTileSize() / 32.0f;
    
    std::cout << "[StoneBuilds] Loading stone builds from " << objects.size() << " map objects..." << std::endl;
    
    int stoneCount = 0;
    for (const auto& obj : objects) {
        if (!obj.tileProperty) continue;
        
        // 检查是否是石头建筑类型
        // TSX中定义：base="build", type="stone_build"
        bool isStone = false;
        if (obj.tileProperty->type == "stone_build") {
            isStone = true;
        } else if (obj.tileProperty->name.find("stone_build") != std::string::npos) {
            isStone = true;
        }
        
        if (!isStone) continue;
        
        float x = obj.x * displayScale;
        float y = obj.y * displayScale;
        float width = obj.width * displayScale;
        float height = obj.height * displayScale;
        
        StoneBuild* stone = stoneBuildManager->addStoneFromProperty(x, y, obj.tileProperty);
        if (stone) {
            stone->setSize(width, height);
            stoneCount++;
            
            std::cout << "[StoneBuilds] Created: " << obj.tileProperty->name 
                      << " HP=" << obj.tileProperty->hp 
                      << " DEF=" << obj.tileProperty->defense << std::endl;
        }
    }
    
    std::cout << "[StoneBuilds] Loaded " << stoneCount << " stone builds" << std::endl;
}

// ============================================================================
// 初始化野生植物
// ============================================================================
void GameState::initWildPlants() {
    if (!wildPlantManager || !tileMap) return;
    
    const auto& objects = tileMap->getObjects();
    float displayScale = (float)tileMap->getTileSize() / 32.0f;
    
    std::cout << "[WildPlants] Loading wild plants from " << objects.size() << " map objects..." << std::endl;
    
    int plantCount = 0;
    for (const auto& obj : objects) {
        if (!obj.tileProperty) continue;
        
        // 检查是否是野生植物类型
        // TSX中定义：base="plants", type="wild_plants"
        bool isWildPlant = false;
        if (obj.tileProperty->type == "wild_plants") {
            isWildPlant = true;
        } else if (obj.tileProperty->name.find("carrot") != std::string::npos ||
                   obj.tileProperty->name.find("bean") != std::string::npos) {
            isWildPlant = true;
        }
        
        if (!isWildPlant) continue;
        
        float x = obj.x * displayScale;
        float y = obj.y * displayScale;
        float width = obj.width * displayScale;
        float height = obj.height * displayScale;
        
        WildPlant* plant = wildPlantManager->addPlantFromProperty(x, y, obj.tileProperty);
        if (plant) {
            plant->setSize(width, height);
            plantCount++;
            
            std::cout << "[WildPlants] Created: " << obj.tileProperty->name 
                      << " pickup=" << (obj.tileProperty->allowPickup ? "yes" : "no")
                      << " item=" << obj.tileProperty->pickupObject << std::endl;
        }
    }
    
    std::cout << "[WildPlants] Loaded " << plantCount << " wild plants" << std::endl;
}

// ============================================================================
// 处理野生植物拾取（V键）
// ============================================================================
void GameState::handlePlantPickup() {
    if (!player || !wildPlantManager) return;
    
    // 检查 V 键按下
    bool pickupPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::V);
    
    if (pickupPressed && !pickupKeyPressed) {
        // V 键刚按下
        sf::Vector2f playerPos = player->getPosition();
        
        std::cout << "[DEBUG] V key pressed, player at (" << playerPos.x << ", " << playerPos.y << ")" << std::endl;
        std::cout << "[DEBUG] Plant count: " << wildPlantManager->getPlantCount() << std::endl;
        
        // 查找范围内可拾取的植物
        WildPlant* plant = wildPlantManager->getPickablePlantInRange(
            playerPos, PLANT_PICKUP_RANGE);
        
        if (plant) {
            std::cout << "[DEBUG] Found plant: " << plant->getName() << " at (" 
                      << plant->getPosition().x << ", " << plant->getPosition().y << ")" << std::endl;
            
            // 播放拾取动画
            player->startPickup();
            
            // 执行拾取
            auto drops = wildPlantManager->pickupPlant(plant);
            
            std::cout << "[DEBUG] Plant pickup drops count: " << drops.size() << std::endl;
            
            // 添加物品到背包
            for (const auto& drop : drops) {
                std::cout << "[DEBUG] Trying to add item: '" << drop.first << "' x" << drop.second << std::endl;
                
                if (categoryInventory) {
                    int added = categoryInventory->addItem(drop.first, drop.second);
                    
                    std::cout << "[DEBUG] Actually added: " << added << std::endl;
                    
                    // 记录到事件日志
                    if (added > 0 && eventLogPanel) {
                        const ItemData* data = ItemDatabase::getInstance().getItemData(drop.first);
                        if (data) {
                            eventLogPanel->addItemObtained(data->name, added, drop.first);
                        }
                    }
                    
                    if (added < drop.second && eventLogPanel) {
                        eventLogPanel->addWarning("背包已满!");
                    }
                } else {
                    std::cout << "[DEBUG] categoryInventory is NULL!" << std::endl;
                }
                
                std::cout << "[Pickup] 获得 " << drop.first << " x" << drop.second << std::endl;
            }
        } else {
            std::cout << "[DEBUG] No pickable plant found in range " << PLANT_PICKUP_RANGE << std::endl;
        }
    }
    
    pickupKeyPressed = pickupPressed;
    
    // 清理已拾取的植物
    wildPlantManager->removePickedPlants();
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
        
        // 攻击石头建筑
        if (stoneBuildManager) {
            auto hitStones = stoneBuildManager->damageStonesInRange(attackCenter, attackRadius, damage);
            
            for (auto* stone : hitStones) {
                if (stone->isDead()) {
                    // 石头被摧毁，生成掉落物
                    auto drops = stone->generateDrops();
                    
                    if (!drops.empty() && droppedItemManager) {
                        sf::Vector2f stonePos = stone->getPosition();
                        droppedItemManager->spawnItems(drops, stonePos.x, stonePos.y - 20);
                        
                        // 添加到事件日志
                        if (eventLogPanel) {
                            eventLogPanel->addMessage("采集了 " + stone->getName(), EventType::System);
                            for (const auto& drop : drops) {
                                const ItemData* data = ItemDatabase::getInstance().getItemData(drop.first);
                                if (data) {
                                    eventLogPanel->addItemObtained(data->name, drop.second, drop.first);
                                }
                            }
                        }
                    }
                    
                    // 获得经验和金币
                    int exp = stone->getExpReward();
                    int gold = stone->getGoldReward();
                    
                    int oldLevel = player->getStats().getLevel();
                    player->getStats().addExp(exp);
                    int newLevel = player->getStats().getLevel();
                    player->getStats().addGold(gold);
                    
                    if (eventLogPanel) {
                        if (exp > 0) eventLogPanel->addExpObtained(exp, "采石");
                        if (gold > 0) eventLogPanel->addGoldObtained(gold);
                        if (newLevel > oldLevel) eventLogPanel->addLevelUp(newLevel);
                    }
                    
                    std::cout << "[Stone] Destroyed! +" << exp << " EXP, +" << gold << " Gold" << std::endl;
                }
            }
            
            if (!hitStones.empty()) {
                std::cout << "[Attack] Hit " << hitStones.size() << " stone(s) for " 
                          << damage << " damage" << std::endl;
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
        
        // 【修复】禁用变换功能
        newTree->setCanTransform(false);
        
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

// ============================================================================
// 初始化宠物系统
// ============================================================================
void GameState::initPetSystem() {
    std::cout << "[PetSystem] Initializing..." << std::endl;
    
    // 创建宠物管理器
    petManager = std::make_unique<PetManager>();
    petManager->init("../../assets");
    
    sf::Vector2u windowSize = game->getWindow().getSize();
    
    // 创建宠物栏面板
    petPanel = std::make_unique<PetPanel>();
    petPanel->init("../../assets/ui/pet_icon.png");
    petPanel->setInventoryManager(petManager.get());
    petPanel->setIconPosition(300.0f, windowSize.y - 80.0f);
    
    // 设置洗点回调
    petPanel->setWashCallback([this](int slotIndex, float luck) {
        return onWashPet(slotIndex, luck);
    });
    
    // 设置切换宠物回调
    petPanel->setSwitchPetCallback([this](int slotIndex) {
        return onSwitchPet(slotIndex);
    });
    
    // 创建孵化栏面板
    hatchPanel = std::make_unique<HatchPanel>();
    hatchPanel->init("../../assets/ui/hatch_icon.png");
    hatchPanel->setPetManager(petManager.get());
    hatchPanel->setIconPosition(370.0f, windowSize.y - 80.0f);
    
    // 设置孵化回调
    hatchPanel->setHatchCallback([this](int petTypeId, int enhancerCount) {
        return onHatchPet(petTypeId, enhancerCount);
    });
    
    // 初始化时更新物品数量
    updatePetPanelItemCounts();
    
    std::cout << "[PetSystem] Initialized successfully" << std::endl;
    std::cout << "  - P: 宠物栏" << std::endl;
    std::cout << "  - H: 孵化栏" << std::endl;
}

// ============================================================================
// 孵化宠物回调
// ============================================================================
bool GameState::onHatchPet(int petTypeId, int enhancerCount) {
    if (!petManager || !categoryInventory) return false;
    
    // 检查精元和获取对应的强化剂ID
    std::string essenceId;
    std::string enhancerId;  // 每种精元对应的特殊强化剂
    std::string enhancerName;
    
    if (petTypeId == 1) {
        essenceId = "rabbit_essence";
        enhancerId = "rabbit_fur";  // 兔子精元使用兔毛作为强化剂
        enhancerName = "兔毛";
    }
    // 未来可以添加更多宠物类型
    // else if (petTypeId == 2) {
    //     essenceId = "slime_essence";
    //     enhancerId = "slime_goo";
    //     enhancerName = "粘液";
    // }
    
    if (essenceId.empty()) {
        if (eventLogPanel) {
            eventLogPanel->addWarning("未知的宠物类型！");
        }
        return false;
    }
    
    // 检查是否有精元
    int essenceCount = categoryInventory->getItemCount(essenceId);
    if (essenceCount <= 0) {
        if (eventLogPanel) {
            eventLogPanel->addWarning("没有足够的精元！");
        }
        return false;
    }
    
    // 检查特定强化剂数量
    int availableEnhancers = categoryInventory->getItemCount(enhancerId);
    int usedEnhancers = std::min(enhancerCount, availableEnhancers);
    
    // 消耗精元
    categoryInventory->removeItem(essenceId, 1);
    
    // 消耗强化剂
    if (usedEnhancers > 0) {
        categoryInventory->removeItem(enhancerId, usedEnhancers);
        if (eventLogPanel) {
            eventLogPanel->addMessage("使用了 " + std::to_string(usedEnhancers) + " 个" + enhancerName + "作为强化剂", EventType::System);
        }
    }
    
    // 孵化宠物
    if (petManager->hatchPet(petTypeId, usedEnhancers)) {
        if (eventLogPanel) {
            Pet* newPet = petManager->getCurrentPet();
            if (newPet) {
                std::string qualityName = Pet::getQualityName(newPet->getQuality());
                eventLogPanel->addMessage("孵化成功！获得 " + qualityName + " 资质的 " + 
                                         newPet->getPetTypeName(), EventType::System);
            }
        }
        
        // 更新面板物品数量显示
        updatePetPanelItemCounts();
        return true;
    }
    
    return false;
}

// ============================================================================
// 洗点宠物回调
// ============================================================================
bool GameState::onWashPet(int slotIndex, float playerLuck) {
    if (!petManager || !categoryInventory) return false;
    
    // 检查洗涤剂
    int cleanserCount = categoryInventory->getItemCount("pet_cleanser");
    if (cleanserCount <= 0) {
        if (eventLogPanel) {
            eventLogPanel->addWarning("没有宠物洗涤剂！");
        }
        return false;
    }
    
    // 获取当前宠物资质
    Pet* pet = petManager->getPetAt(slotIndex);
    if (!pet) return false;
    
    PetQuality oldQuality = pet->getQuality();
    
    // 消耗洗涤剂
    categoryInventory->removeItem("pet_cleanser", 1);
    
    // 执行洗点
    if (petManager->washPet(slotIndex, playerLuck)) {
        PetQuality newQuality = pet->getQuality();
        
        if (eventLogPanel) {
            eventLogPanel->addMessage("洗点成功！" + Pet::getQualityName(oldQuality) + 
                                     " -> " + Pet::getQualityName(newQuality), EventType::System);
        }
        
        // 更新面板物品数量显示
        updatePetPanelItemCounts();
        return true;
    }
    
    return false;
}

// ============================================================================
// 切换宠物回调
// ============================================================================
bool GameState::onSwitchPet(int slotIndex) {
    if (!petManager) return false;
    
    if (petManager->switchPet(slotIndex)) {
        Pet* pet = petManager->getCurrentPet();
        if (pet && eventLogPanel) {
            eventLogPanel->addMessage("切换到宠物: " + pet->getName(), EventType::System);
        }
        return true;
    }
    
    return false;
}

// ============================================================================
// 更新宠物面板物品数量
// ============================================================================
void GameState::updatePetPanelItemCounts() {
    if (!categoryInventory) {
        std::cout << "[PetSystem] Error: categoryInventory is null!" << std::endl;
        return;
    }
    
    int rabbitEssence = categoryInventory->getItemCount("rabbit_essence");
    int rabbitFur = categoryInventory->getItemCount("rabbit_fur");  // 兔毛作为兔子精元的强化剂
    int cleansers = categoryInventory->getItemCount("pet_cleanser");
    
    std::cout << "[PetSystem] Updating counts - Essence: " << rabbitEssence 
              << ", RabbitFur(Enhancer): " << rabbitFur 
              << ", Cleansers: " << cleansers << std::endl;
    
    // 更新宠物栏面板
    if (petPanel) {
        petPanel->setCleanserCount(cleansers);
        if (player) {
            petPanel->setPlayerLuck(player->getStats().getLuck());
        }
    }
    
    // 更新孵化栏面板
    if (hatchPanel) {
        hatchPanel->setEssenceCount(1, rabbitEssence);
        hatchPanel->setEnhancerCount(rabbitFur);  // 使用兔毛数量
    }
}

void GameState::handlePetCommand(const sf::Event& event) {
    if (!player || !petManager || !camera || !rabbitManager) return;
    
    // 获取当前宠物
    Pet* pet = petManager->getCurrentPet();
    if (!pet) return;

    // 获取鼠标在世界坐标中的位置
    sf::Vector2i mouseScreenPos = sf::Mouse::getPosition(game->getWindow());
    sf::Vector2f mouseWorldPos = game->getWindow().mapPixelToCoords(mouseScreenPos, camera->getView());

    // 1. 检查鼠标是否点到了兔子
    Rabbit* targetRabbit = nullptr;
    for (auto& rabbit : rabbitManager->getRabbits()) {
        if (!rabbit->isDead() && rabbit->getBounds().contains(mouseWorldPos)) {
            targetRabbit = rabbit.get();
            break;
        }
    }

    if (targetRabbit) {
        // 获取兔子中心点
        sf::FloatRect bounds = targetRabbit->getBounds();
        sf::Vector2f targetCenter(bounds.left + bounds.width/2, bounds.top + bounds.height/2);
        
        // 发送指令
        pet->commandAttack(targetCenter);
        
        // 显示特效或日志
        if (eventLogPanel) {
            eventLogPanel->addMessage("指挥 " + pet->getName() + " 攻击目标!", EventType::System);
        }
    } else {
        // 如果点空地，也可以让宠物移动过去（可选）
        // pet->commandAttack(mouseWorldPos); 
    }
}

void GameState::handlePetAttack() {
    if (!petManager || !rabbitManager) return;

    Pet* pet = petManager->getCurrentPet();
    
    // 检查宠物是否在本帧挥出了攻击
    if (pet && pet->hasJustAttacked()) {
        float damage = pet->performAttack(); // 获取计算过暴击/技能的伤害
        sf::Vector2f petPos = pet->getPosition();
        float attackRange = pet->getAttackRange(); 
        
        // 关键：伤害判定圆心是宠物，不是玩家
        auto hitRabbits = rabbitManager->damageRabbitsInRange(petPos, attackRange, damage, false);
        
        if (hitRabbits.empty()) {
            // 如果没打到人（可能距离不够），尝试判定是否是“指定攻击”
            // 有时候动画播放了但距离差一点点，稍微放宽一点判定
            if (pet->hasCommandedTarget()) {
                 hitRabbits = rabbitManager->damageRabbitsInRange(pet->getCommandedTarget(), 32.0f, damage, false);
            }
        }
        
        for (auto* rabbit : hitRabbits) {
            if (rabbit->isDead()) {
                // 处理掉落
                auto drops = rabbit->generateDrops();
                if (!drops.empty() && droppedItemManager) {
                    sf::Vector2f rabbitPos = rabbit->getPosition();
                    droppedItemManager->spawnItems(drops, rabbitPos.x, rabbitPos.y);
                    
                    // 添加到事件日志
                    if (eventLogPanel) {
                        eventLogPanel->addMessage(pet->getName() + " 击杀了 " + rabbit->getName(), EventType::Combat);
                        for (const auto& drop : drops) {
                            const ItemData* data = ItemDatabase::getInstance().getItemData(drop.first);
                            if (data) {
                                eventLogPanel->addItemObtained(data->name, drop.second, drop.first);
                            }
                        }
                    }
                }
                
                // 处理经验
                int exp = rabbit->getExpReward();
                int gold = rabbit->getGoldReward();
                
                if (player) {
                    int oldLevel = player->getStats().getLevel();
                    player->getStats().addExp(exp);
                    int newLevel = player->getStats().getLevel();
                    player->getStats().addGold(gold);
                    pet->addExp(exp / 2); // 宠物分得经验
                    
                    // 添加经验和金币日志
                    if (eventLogPanel) {
                        if (exp > 0) eventLogPanel->addExpObtained(exp, "宠物击杀");
                        if (gold > 0) eventLogPanel->addGoldObtained(gold);
                        if (newLevel > oldLevel) eventLogPanel->addLevelUp(newLevel);
                    }
                }
                
                std::cout << "[Pet] " << pet->getName() << " 独立击杀了兔子! +"
                          << exp << " EXP, +" << gold << " Gold" << std::endl;
            }
        }
        
        // 重要：清除攻击标志，防止一刀多判
        pet->clearJustAttacked();
    }
}