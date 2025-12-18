#pragma once
#include "State.h"
#include "../Entity/Player.h"
#include "../Entity/Tree.h"
#include "../Entity/Rabbit.h"
#include "../World/TileMap.h"
#include "../World/Camera.h"
#include "../Systems/TimeSystem.h"
#include "../UI/StatsPanel.h"
#include "../UI/InventoryPanel.h"
#include "../UI/CategoryInventoryPanel.h"
#include "../UI/EventLogPanel.h"
#include "../Items/Item.h"
#include "../Items/Inventory.h"
#include "../Items/CategoryInventory.h"
#include "../Items/Equipment.h"
#include "../Items/Crafting.h"
#include "../Items/DroppedItem.h"
#include <memory>
#include <string>
#include <random>

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
    
    // Initialize UI
    void initUI(sf::RenderWindow& window);
    
    // Initialize trees from map
    void initTrees();
    
    // Initialize rabbits
    void initRabbits();
    
    // Initialize item system
    void initItemSystem();
    
    // Handle player attack
    void handlePlayerAttack();
    
    // Handle item pickup
    void handleItemPickup();
    
    // Use consumable item (callback)
    bool onUseItem(const ItemStack& item, const ItemData* data);
    
    // Sell item callback
    void onSellItem(const ItemStack& item, int sellPrice);
    
    // Plant seed callback - returns true if planting was successful
    bool onPlantSeed(const ItemStack& seed);
    
    // Equip item callback
    void onEquipItem(const ItemStack& item);
    
    // Unequip item callback
    void onUnequipItem(EquipmentSlot slot);
    
    // Get map name string
    std::string getMapName(MapType mapType) const;

private:
    // Game objects
    std::unique_ptr<Player> player;
    std::unique_ptr<TileMap> tileMap;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<TimeSystem> timeSystem;
    std::unique_ptr<TreeManager> treeManager;
    
    // Rabbit system
    std::unique_ptr<RabbitManager> rabbitManager;
    
    // Item system - new categorized inventory
    std::unique_ptr<CategoryInventory> categoryInventory;
    std::unique_ptr<DroppedItemManager> droppedItemManager;
    
    // Equipment system
    std::unique_ptr<PlayerEquipment> playerEquipment;
    
    // UI panels
    std::unique_ptr<StatsPanel> statsPanel;
    std::unique_ptr<CategoryInventoryPanel> categoryInventoryPanel;
    std::unique_ptr<EquipmentPanel> equipmentPanel;
    std::unique_ptr<CraftingPanel> craftingPanel;
    std::unique_ptr<EventLogPanel> eventLogPanel;
    
    // Current map type
    MapType currentMap;
    
    // Attack state tracking
    bool wasAttacking;
    
    // Item pickup range
    static constexpr float PICKUP_RANGE = 50.0f;
    
    // Random number generator for seed planting
    std::mt19937 rng;
    
    // Tree types available for seed planting (from tree.tsx)
    std::vector<std::string> availableTreeTypes;
};