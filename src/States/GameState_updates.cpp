// ============================================================================
// GameState.cpp 更新说明
// 
// 这个文件包含需要添加到 GameState.cpp 的代码片段
// ============================================================================

// ============================================================================
// 1. 在文件顶部添加 include
// ============================================================================
#include "../Entity/StoneBuild.h"
#include "../Entity/WildPlant.h"

// ============================================================================
// 2. 在 GameState 构造函数中初始化新的管理器
// ============================================================================
// 在 initTrees() 之后添加:
void GameState_InitNewSystems() {
    // 初始化石头建筑管理器
    stoneBuildManager = std::make_unique<StoneBuildManager>();
    stoneBuildManager->init("../../assets/");
    initStoneBuild();
    
    // 初始化野生植物管理器
    wildPlantManager = std::make_unique<WildPlantManager>();
    wildPlantManager->init("../../assets/");
    initWildPlants();
}

// ============================================================================
// 3. 新增初始化方法
// ============================================================================

void GameState::initStoneBuild() {
    if (!stoneBuildManager || !tileMap) return;
    
    const auto& objects = tileMap->getObjects();
    
    for (const auto& obj : objects) {
        if (!obj.tileProperty) continue;
        
        // 检查是否是石头建筑类型（base="build", type="stone_build"）
        if (obj.tileProperty->base == "build" && 
            obj.tileProperty->type == "stone_build") {
            
            float scale = 1.5f;  // 根据你的显示缩放调整
            float x = obj.x * scale;
            float y = obj.y * scale;
            
            StoneBuild* stone = stoneBuildManager->addStoneFromProperty(x, y, obj.tileProperty);
            if (stone) {
                stone->setSize(obj.width * scale, obj.height * scale);
            }
        }
    }
    
    std::cout << "[GameState] Loaded " << stoneBuildManager->getStoneCount() 
              << " stone buildings" << std::endl;
}

void GameState::initWildPlants() {
    if (!wildPlantManager || !tileMap) return;
    
    const auto& objects = tileMap->getObjects();
    
    for (const auto& obj : objects) {
        if (!obj.tileProperty) continue;
        
        // 检查是否是野生植物类型（base="plant", type="wild_plants"）
        if (obj.tileProperty->base == "plant" && 
            obj.tileProperty->type == "wild_plants") {
            
            float scale = 1.5f;  // 根据你的显示缩放调整
            float x = obj.x * scale;
            float y = obj.y * scale;
            
            WildPlant* plant = wildPlantManager->addPlantFromProperty(x, y, obj.tileProperty);
            if (plant) {
                plant->setSize(obj.width * scale, obj.height * scale);
                
                // 设置拾取物品信息
                if (obj.tileProperty->allowPickup) {
                    plant->setAllowPickup(true);
                    plant->setPickupItem(
                        obj.tileProperty->pickupObject,
                        obj.tileProperty->countMin,
                        obj.tileProperty->countMax
                    );
                }
            }
        }
    }
    
    std::cout << "[GameState] Loaded " << wildPlantManager->getPlantCount() 
              << " wild plants" << std::endl;
}

// ============================================================================
// 4. 处理植物拾取（按 E 键）
// ============================================================================

void GameState::handlePlantPickup() {
    if (!player || !wildPlantManager) return;
    
    // 检查 E 键按下
    bool pickupPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::E);
    
    if (pickupPressed && !pickupKeyPressed) {
        // E 键刚按下
        sf::Vector2f playerPos = player->getPosition();
        
        // 查找范围内可拾取的植物
        WildPlant* plant = wildPlantManager->getPickablePlantInRange(
            playerPos, PLANT_PICKUP_RANGE);
        
        if (plant) {
            // 执行拾取
            auto drops = wildPlantManager->pickupPlant(plant);
            
            // 添加物品到背包
            for (const auto& drop : drops) {
                if (categoryInventory) {
                    categoryInventory->addItem(drop.first, drop.second);
                }
                
                // 记录到事件日志
                if (eventLogPanel) {
                    const ItemData* data = ItemDatabase::getInstance().getItemData(drop.first);
                    if (data) {
                        eventLogPanel->addItemObtained(data->name, drop.second, drop.first);
                    }
                }
                
                std::cout << "[Pickup] 获得 " << drop.first << " x" << drop.second << std::endl;
            }
        }
    }
    
    pickupKeyPressed = pickupPressed;
    
    // 清理已拾取的植物
    wildPlantManager->removePickedPlants();
}

// ============================================================================
// 5. 处理宠物指挥攻击（右键点击目标）
// ============================================================================

void GameState::handlePetCommandAttack(const sf::Vector2f& targetWorldPos) {
    if (!petManager) return;
    
    Pet* pet = petManager->getCurrentPet();
    if (!pet) return;
    
    // 检查目标位置是否有敌人或可攻击的对象
    bool hasTarget = false;
    
    // 检查兔子
    if (rabbitManager) {
        Rabbit* rabbit = rabbitManager->getRabbitAt(targetWorldPos);
        if (rabbit && !rabbit->isDead()) {
            pet->commandAttack(rabbit->getPosition());
            hasTarget = true;
            
            if (eventLogPanel) {
                eventLogPanel->addMessage(pet->getName() + " 收到攻击指令!", EventType::Combat);
            }
        }
    }
    
    // 检查树木
    if (!hasTarget && treeManager) {
        Tree* tree = treeManager->getTreeAt(targetWorldPos);
        if (tree && !tree->isDead()) {
            pet->commandAttack(tree->getPosition());
            hasTarget = true;
            
            if (eventLogPanel) {
                eventLogPanel->addMessage(pet->getName() + " 收到攻击指令!", EventType::Combat);
            }
        }
    }
    
    // 检查石头
    if (!hasTarget && stoneBuildManager) {
        StoneBuild* stone = stoneBuildManager->getStoneAt(targetWorldPos);
        if (stone && !stone->isDead()) {
            pet->commandAttack(stone->getPosition());
            hasTarget = true;
            
            if (eventLogPanel) {
                eventLogPanel->addMessage(pet->getName() + " 收到攻击指令!", EventType::Combat);
            }
        }
    }
    
    if (!hasTarget) {
        std::cout << "[Pet Command] 没有找到可攻击的目标" << std::endl;
    }
}

// ============================================================================
// 6. 在 handleInput 中添加右键检测
// ============================================================================
// 在 handleInput 方法中添加:

void GameState_HandleInput_Addition(const sf::Event& event, sf::RenderWindow& window, 
                                     TileMap* tileMap, Camera* camera) {
    // 右键点击指挥宠物攻击
    if (event.type == sf::Event::MouseButtonPressed && 
        event.mouseButton.button == sf::Mouse::Right) {
        
        // 获取鼠标世界坐标
        sf::Vector2i mousePos(event.mouseButton.x, event.mouseButton.y);
        sf::Vector2f worldPos = window.mapPixelToCoords(mousePos, camera->getView());
        
        // 调用宠物指挥攻击
        // handlePetCommandAttack(worldPos);
    }
}

// ============================================================================
// 7. 在 update 中调用新的更新方法
// ============================================================================
// 在 update 方法的适当位置添加:

void GameState_Update_Addition(float dt) {
    // 更新石头建筑
    // if (stoneBuildManager) {
    //     stoneBuildManager->update(dt);
    // }
    
    // 更新野生植物
    // if (wildPlantManager) {
    //     wildPlantManager->update(dt);
    // }
    
    // 处理植物拾取
    // handlePlantPickup();
}

// ============================================================================
// 8. 在 render 中渲染新的对象
// ============================================================================
// 在 render 方法中添加（在树木渲染之后）:

void GameState_Render_Addition(sf::RenderWindow& window, Camera* camera) {
    // 渲染石头建筑
    // if (stoneBuildManager) {
    //     stoneBuildManager->render(window, camera->getView());
    // }
    
    // 渲染野生植物
    // if (wildPlantManager) {
    //     wildPlantManager->render(window, camera->getView());
    // }
}

// ============================================================================
// 9. 【重要】修复宠物攻击伤害 - 在 handlePlayerAttack 中扩展宠物攻击逻辑
// ============================================================================

void GameState_FixPetAttackDamage() {
    // 在原有的 "宠物协同攻击敌人" 代码块之后，添加以下代码：
    
    /*
    // 宠物也可以攻击树木
    if (petManager && treeManager) {
        Pet* pet = petManager->getCurrentPet();
        if (pet && pet->hasJustAttacked()) {
            float petDamage = pet->getAttack();
            sf::Vector2f petPos = pet->getPosition();
            float petAttackRange = pet->getAttackRange();
            
            // 宠物攻击范围内的树木
            auto hitTrees = treeManager->damageTreesInRange(petPos, petAttackRange, petDamage);
            
            for (auto* tree : hitTrees) {
                if (tree->isDead()) {
                    // 树木被摧毁，生成掉落物
                    auto drops = tree->generateDrops();
                    
                    if (!drops.empty() && droppedItemManager) {
                        sf::Vector2f treePos = tree->getPosition();
                        droppedItemManager->spawnItems(drops, treePos.x, treePos.y);
                        
                        if (eventLogPanel) {
                            eventLogPanel->addMessage(pet->getName() + " 砍倒了 " + tree->getName(), EventType::Combat);
                            for (const auto& drop : drops) {
                                const ItemData* data = ItemDatabase::getInstance().getItemData(drop.first);
                                if (data) {
                                    eventLogPanel->addItemObtained(data->name, drop.second, drop.first);
                                }
                            }
                        }
                    }
                    
                    // 获得经验和金币
                    int exp = tree->getExpReward();
                    int gold = tree->getGoldReward();
                    
                    player->getStats().addExp(exp);
                    player->getStats().addGold(gold);
                    pet->addExp(exp / 2);
                    
                    if (eventLogPanel) {
                        if (exp > 0) eventLogPanel->addExpObtained(exp, "宠物砍树");
                        if (gold > 0) eventLogPanel->addGoldObtained(gold);
                    }
                }
            }
            
            if (!hitTrees.empty()) {
                std::cout << "[Pet Attack] " << pet->getName() << " hit " << hitTrees.size() 
                          << " tree(s) for " << petDamage << " damage" << std::endl;
            }
            
            // 注意：不要在这里清除 justAttacked，让后面的代码处理
        }
    }
    
    // 宠物攻击石头
    if (petManager && stoneBuildManager) {
        Pet* pet = petManager->getCurrentPet();
        if (pet && pet->hasJustAttacked()) {
            float petDamage = pet->getAttack();
            sf::Vector2f petPos = pet->getPosition();
            float petAttackRange = pet->getAttackRange();
            
            // 宠物攻击范围内的石头
            auto hitStones = stoneBuildManager->damageStonesInRange(petPos, petAttackRange, petDamage);
            
            for (auto* stone : hitStones) {
                if (stone->isDead()) {
                    // 石头被摧毁，生成掉落物
                    auto drops = stone->generateDrops();
                    
                    if (!drops.empty() && droppedItemManager) {
                        sf::Vector2f stonePos = stone->getPosition();
                        droppedItemManager->spawnItems(drops, stonePos.x, stonePos.y);
                        
                        if (eventLogPanel) {
                            eventLogPanel->addMessage(pet->getName() + " 采集了 " + stone->getName(), EventType::Combat);
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
                    
                    player->getStats().addExp(exp);
                    player->getStats().addGold(gold);
                    pet->addExp(exp / 2);
                    
                    if (eventLogPanel) {
                        if (exp > 0) eventLogPanel->addExpObtained(exp, "宠物采石");
                        if (gold > 0) eventLogPanel->addGoldObtained(gold);
                    }
                }
            }
            
            if (!hitStones.empty()) {
                std::cout << "[Pet Attack] " << pet->getName() << " hit " << hitStones.size() 
                          << " stone(s) for " << petDamage << " damage" << std::endl;
            }
        }
    }
    */
}

// ============================================================================
// 10. 玩家攻击石头 - 在 handlePlayerAttack 中添加
// ============================================================================

void GameState_PlayerAttackStone() {
    /*
    // 在 "攻击树木" 代码块之后添加：
    
    // 攻击石头
    if (stoneBuildManager) {
        auto hitStones = stoneBuildManager->damageStonesInRange(attackCenter, attackRadius, damage);
        
        for (auto* stone : hitStones) {
            if (stone->isDead()) {
                // 石头被摧毁，生成掉落物
                auto drops = stone->generateDrops();
                
                if (!drops.empty() && droppedItemManager) {
                    sf::Vector2f stonePos = stone->getPosition();
                    droppedItemManager->spawnItems(drops, stonePos.x, stonePos.y);
                    
                    if (eventLogPanel) {
                        eventLogPanel->addMessage("采集了 " + stone->getName(), EventType::Resource);
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
            }
        }
        
        if (!hitStones.empty()) {
            std::cout << "[Attack] Hit " << hitStones.size() << " stone(s) for " 
                      << damage << " damage" << std::endl;
        }
    }
    */
}
