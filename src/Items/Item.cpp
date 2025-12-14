#include "Item.h"
#include <iostream>

// ============================================================================
// ItemDatabase 单例实现
// ============================================================================

ItemDatabase& ItemDatabase::getInstance() {
    static ItemDatabase instance;
    return instance;
}

void ItemDatabase::initialize() {
    if (initialized) return;
    
    std::cout << "[ItemDatabase] Initializing item definitions..." << std::endl;
    
    // ========================================
    // 材料类物品 (Materials)
    // 存放目录: assets/materials/
    // ========================================
    
    {
        ItemData wood;
        wood.id = "wood";
        wood.name = "木材";
        wood.description = "从树木上砍伐获得的木材，可用于建造和合成";
        wood.type = ItemType::Material;
        wood.rarity = ItemRarity::Common;
        wood.maxStack = 99;
        wood.sellPrice = 2;
        wood.buyPrice = 10;
        wood.texturePath = "assets/materials/wood.png";
        registerItem(wood);
    }
    
    {
        ItemData stick;
        stick.id = "stick";
        stick.name = "树枝";
        stick.description = "细小的树枝，可用于制作简单工具";
        stick.type = ItemType::Material;
        stick.rarity = ItemRarity::Common;
        stick.maxStack = 99;
        stick.sellPrice = 1;
        stick.buyPrice = 5;
        stick.texturePath = "assets/materials/stick.png";
        registerItem(stick);
    }
    
    {
        ItemData seed;
        seed.id = "seed";
        seed.name = "种子";
        seed.description = "可以种植的树种子";
        seed.type = ItemType::Material;
        seed.rarity = ItemRarity::Common;
        seed.maxStack = 99;
        seed.sellPrice = 5;
        seed.buyPrice = 20;
        seed.texturePath = "assets/materials/seed.png";
        registerItem(seed);
    }
    
    {
        ItemData stone;
        stone.id = "stone";
        stone.name = "石头";
        stone.description = "普通的石头，可用于建造";
        stone.type = ItemType::Material;
        stone.rarity = ItemRarity::Common;
        stone.maxStack = 99;
        stone.sellPrice = 1;
        stone.buyPrice = 5;
        stone.texturePath = "assets/materials/stone.png";
        registerItem(stone);
    }
    
    // ========================================
    // 消耗品类物品 (Consumables)
    // 存放目录: assets/consumables/
    // ========================================
    
    {
        ItemData apple;
        apple.id = "apple";
        apple.name = "苹果";
        apple.description = "新鲜的苹果，食用后恢复少量生命值";
        apple.type = ItemType::Consumable;
        apple.rarity = ItemRarity::Common;
        apple.maxStack = 20;
        apple.sellPrice = 5;
        apple.buyPrice = 15;
        apple.texturePath = "assets/consumables/apple.png";
        apple.effects.push_back(ConsumableEffect(EffectType::RestoreHealth, 10));
        registerItem(apple);
    }
    
    {
        ItemData cherry;
        cherry.id = "cherry";
        cherry.name = "樱桃";
        cherry.description = "甜美的樱桃，食用后恢复生命值";
        cherry.type = ItemType::Consumable;
        cherry.rarity = ItemRarity::Uncommon;
        cherry.maxStack = 20;
        cherry.sellPrice = 8;
        cherry.buyPrice = 25;
        cherry.texturePath = "assets/consumables/cherry.png";
        cherry.effects.push_back(ConsumableEffect(EffectType::RestoreHealth, 20));
        registerItem(cherry);
    }
    
    {
        ItemData healthPotion;
        healthPotion.id = "health_potion";
        healthPotion.name = "生命药水";
        healthPotion.description = "红色的药水，能够快速恢复大量生命值";
        healthPotion.type = ItemType::Consumable;
        healthPotion.rarity = ItemRarity::Rare;
        healthPotion.maxStack = 10;
        healthPotion.sellPrice = 25;
        healthPotion.buyPrice = 100;
        healthPotion.texturePath = "assets/consumables/health_potion.png";
        healthPotion.effects.push_back(ConsumableEffect(EffectType::RestoreHealth, 50));
        registerItem(healthPotion);
    }
    
    initialized = true;
    std::cout << "[ItemDatabase] Registered " << items.size() << " items" << std::endl;
}

bool ItemDatabase::loadTextures(const std::string& basePath) {
    std::cout << "[ItemDatabase] Loading item textures from: " << basePath << std::endl;
    
    int loaded = 0;
    int failed = 0;
    
    for (auto& pair : items) {
        const std::string& itemId = pair.first;
        const ItemData& data = pair.second;
        
        if (data.texturePath.empty()) continue;
        
        sf::Texture texture;
        
        // 尝试多种路径
        std::vector<std::string> paths = {
            data.texturePath,
            basePath + "/" + data.texturePath,
            "../../" + data.texturePath
        };
        
        bool success = false;
        for (const auto& path : paths) {
            if (texture.loadFromFile(path)) {
                textures[itemId] = texture;
                loaded++;
                success = true;
                break;
            }
        }
        
        if (!success) {
            // 创建占位贴图
            sf::Image placeholder;
            placeholder.create(32, 32, sf::Color(100, 100, 100, 200));
            texture.loadFromImage(placeholder);
            textures[itemId] = texture;
            failed++;
            std::cout << "[ItemDatabase] Missing texture for: " << itemId << std::endl;
        }
    }
    
    std::cout << "[ItemDatabase] Loaded " << loaded << " textures, " 
              << failed << " placeholders" << std::endl;
    
    return loaded > 0;
}

const ItemData* ItemDatabase::getItemData(const std::string& itemId) const {
    auto it = items.find(itemId);
    if (it != items.end()) {
        return &it->second;
    }
    return nullptr;
}

const sf::Texture* ItemDatabase::getTexture(const std::string& itemId) const {
    auto it = textures.find(itemId);
    if (it != textures.end()) {
        return &it->second;
    }
    return nullptr;
}

void ItemDatabase::registerItem(const ItemData& data) {
    items[data.id] = data;
    std::cout << "[ItemDatabase] Registered: " << data.id << " (" << data.name << ")" << std::endl;
}

sf::Color ItemDatabase::getRarityColor(ItemRarity rarity) {
    switch (rarity) {
        case ItemRarity::Common:    return sf::Color(255, 255, 255);     // 白色
        case ItemRarity::Uncommon:  return sf::Color(30, 255, 30);       // 绿色
        case ItemRarity::Rare:      return sf::Color(30, 144, 255);      // 蓝色
        case ItemRarity::Epic:      return sf::Color(163, 53, 238);      // 紫色
        case ItemRarity::Legendary: return sf::Color(255, 165, 0);       // 橙色
        default:                    return sf::Color::White;
    }
}

std::string ItemDatabase::getTypeName(ItemType type) {
    switch (type) {
        case ItemType::Material:    return "材料";
        case ItemType::Consumable:  return "消耗品";
        case ItemType::Equipment:   return "装备";
        case ItemType::Quest:       return "任务物品";
        case ItemType::Misc:        return "杂物";
        default:                    return "未知";
    }
}

// ============================================================================
// 辅助函数实现
// ============================================================================

ItemStack createItemStack(const std::string& itemId, int count) {
    return ItemStack(itemId, count);
}

bool canStackItems(const ItemStack& a, const ItemStack& b) {
    if (a.isEmpty() || b.isEmpty()) return false;
    return a.itemId == b.itemId;
}

int mergeItemStacks(ItemStack& dest, ItemStack& src, int maxStack) {
    if (dest.isEmpty()) {
        dest = src;
        src.clear();
        return 0;
    }
    
    if (dest.itemId != src.itemId) {
        return src.count;
    }
    
    int canAdd = maxStack - dest.count;
    int toAdd = std::min(canAdd, src.count);
    
    dest.count += toAdd;
    src.count -= toAdd;
    
    if (src.count <= 0) {
        src.clear();
    }
    
    return src.count;
}
