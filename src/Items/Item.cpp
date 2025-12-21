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
        seed.description = "可以种植的树种子。";
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
    
    // 兔子掉落物品
    {
        ItemData rabbitFur;
        rabbitFur.id = "rabbit_fur";
        rabbitFur.name = "兔毛";
        rabbitFur.description = "柔软的兔毛。可作为孵化兔子精元时的强化剂，提升稀有资质概率！";
        rabbitFur.type = ItemType::Material;
        rabbitFur.rarity = ItemRarity::Common;
        rabbitFur.maxStack = 99;
        rabbitFur.sellPrice = 5;
        rabbitFur.buyPrice = 15;
        rabbitFur.texturePath = "assets/materials/rabbit_fur.png";
        registerItem(rabbitFur);
    }
    
    {
        ItemData rabbitMeat;
        rabbitMeat.id = "rabbit_meat";
        rabbitMeat.name = "兔肉";
        rabbitMeat.description = "新鲜的兔肉，可以烹饪食用";
        rabbitMeat.type = ItemType::Consumable;
        rabbitMeat.rarity = ItemRarity::Uncommon;
        rabbitMeat.maxStack = 20;
        rabbitMeat.sellPrice = 15;
        rabbitMeat.buyPrice = 40;
        rabbitMeat.texturePath = "assets/consumables/rabbit_meat.png";
        rabbitMeat.effects.push_back(ConsumableEffect(EffectType::RestoreHealth, 30));
        registerItem(rabbitMeat);
    }
    
    {
        ItemData carrot;
        carrot.id = "carrot";
        carrot.name = "胡萝卜";
        carrot.description = "新鲜的胡萝卜，兔子的最爱";
        carrot.type = ItemType::Consumable;
        carrot.rarity = ItemRarity::Common;
        carrot.maxStack = 20;
        carrot.sellPrice = 3;
        carrot.buyPrice = 10;
        carrot.texturePath = "assets/consumables/carrot.png";
        carrot.effects.push_back(ConsumableEffect(EffectType::RestoreHealth, 8));
        registerItem(carrot);
    }
    
    {
        ItemData bean;
        bean.id = "bean";
        bean.name = "豆子";
        bean.description = "新鲜的豆子，可以食用或种植";
        bean.type = ItemType::Consumable;
        bean.rarity = ItemRarity::Common;
        bean.maxStack = 20;
        bean.sellPrice = 2;
        bean.buyPrice = 8;
        bean.texturePath = "assets/consumables/bean.png";
        bean.effects.push_back(ConsumableEffect(EffectType::RestoreHealth, 5));
        registerItem(bean);
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
    
    // ========================================
    // 装备类物品 (Equipment) - 武器
    // 存放目录: assets/equipment/
    // ========================================
    
    // 斧头系列 (无视防御)
    {
        ItemData woodenAxe;
        woodenAxe.id = "wooden_axe";
        woodenAxe.name = "木斧";
        woodenAxe.description = "简单的木制斧头，可以无视目标的防御";
        woodenAxe.type = ItemType::Equipment;
        woodenAxe.rarity = ItemRarity::Common;
        woodenAxe.maxStack = 1;
        woodenAxe.sellPrice = 15;
        woodenAxe.buyPrice = 50;
        woodenAxe.texturePath = "assets/equipment/wooden_axe.png";
        registerItem(woodenAxe);
    }
    
    {
        ItemData ironAxe;
        ironAxe.id = "iron_axe";
        ironAxe.name = "铁斧";
        ironAxe.description = "坚固的铁制斧头，可以无视目标的防御";
        ironAxe.type = ItemType::Equipment;
        ironAxe.rarity = ItemRarity::Uncommon;
        ironAxe.maxStack = 1;
        ironAxe.sellPrice = 50;
        ironAxe.buyPrice = 200;
        ironAxe.texturePath = "assets/equipment/iron_axe.png";
        registerItem(ironAxe);
    }
    
    // 剑系列
    {
        ItemData woodenSword;
        woodenSword.id = "wooden_sword";
        woodenSword.name = "木剑";
        woodenSword.description = "新手战士的第一把武器";
        woodenSword.type = ItemType::Equipment;
        woodenSword.rarity = ItemRarity::Common;
        woodenSword.maxStack = 1;
        woodenSword.sellPrice = 10;
        woodenSword.buyPrice = 40;
        woodenSword.texturePath = "assets/equipment/wooden_sword.png";
        registerItem(woodenSword);
    }
    
    {
        ItemData ironSword;
        ironSword.id = "iron_sword";
        ironSword.name = "铁剑";
        ironSword.description = "标准的铁制长剑";
        ironSword.type = ItemType::Equipment;
        ironSword.rarity = ItemRarity::Uncommon;
        ironSword.maxStack = 1;
        ironSword.sellPrice = 45;
        ironSword.buyPrice = 180;
        ironSword.texturePath = "assets/equipment/iron_sword.png";
        registerItem(ironSword);
    }
    
    // ========================================
    // 装备类物品 (Equipment) - 防具
    // ========================================
    
    // 头盔
    {
        ItemData leatherCap;
        leatherCap.id = "leather_cap";
        leatherCap.name = "皮帽";
        leatherCap.description = "简单的皮制帽子";
        leatherCap.type = ItemType::Equipment;
        leatherCap.rarity = ItemRarity::Common;
        leatherCap.maxStack = 1;
        leatherCap.sellPrice = 8;
        leatherCap.buyPrice = 30;
        leatherCap.texturePath = "assets/equipment/leather_cap.png";
        registerItem(leatherCap);
    }
    
    {
        ItemData ironHelmet;
        ironHelmet.id = "iron_helmet";
        ironHelmet.name = "铁盔";
        ironHelmet.description = "坚固的铁制头盔";
        ironHelmet.type = ItemType::Equipment;
        ironHelmet.rarity = ItemRarity::Uncommon;
        ironHelmet.maxStack = 1;
        ironHelmet.sellPrice = 35;
        ironHelmet.buyPrice = 150;
        ironHelmet.texturePath = "assets/equipment/iron_helmet.png";
        registerItem(ironHelmet);
    }
    
    // 铠甲
    {
        ItemData leatherArmor;
        leatherArmor.id = "leather_armor";
        leatherArmor.name = "皮甲";
        leatherArmor.description = "轻便的皮制护甲";
        leatherArmor.type = ItemType::Equipment;
        leatherArmor.rarity = ItemRarity::Common;
        leatherArmor.maxStack = 1;
        leatherArmor.sellPrice = 15;
        leatherArmor.buyPrice = 60;
        leatherArmor.texturePath = "assets/equipment/leather_armor.png";
        registerItem(leatherArmor);
    }
    
    {
        ItemData ironArmor;
        ironArmor.id = "iron_armor";
        ironArmor.name = "铁甲";
        ironArmor.description = "厚重的铁制铠甲";
        ironArmor.type = ItemType::Equipment;
        ironArmor.rarity = ItemRarity::Uncommon;
        ironArmor.maxStack = 1;
        ironArmor.sellPrice = 60;
        ironArmor.buyPrice = 250;
        ironArmor.texturePath = "assets/equipment/iron_armor.png";
        registerItem(ironArmor);
    }
    
    // 裤子
    {
        ItemData leatherPants;
        leatherPants.id = "leather_pants";
        leatherPants.name = "皮裤";
        leatherPants.description = "简单的皮制裤子";
        leatherPants.type = ItemType::Equipment;
        leatherPants.rarity = ItemRarity::Common;
        leatherPants.maxStack = 1;
        leatherPants.sellPrice = 10;
        leatherPants.buyPrice = 40;
        leatherPants.texturePath = "assets/equipment/leather_pants.png";
        registerItem(leatherPants);
    }
    
    // 手套
    {
        ItemData leatherGloves;
        leatherGloves.id = "leather_gloves";
        leatherGloves.name = "皮手套";
        leatherGloves.description = "简单的皮制手套";
        leatherGloves.type = ItemType::Equipment;
        leatherGloves.rarity = ItemRarity::Common;
        leatherGloves.maxStack = 1;
        leatherGloves.sellPrice = 6;
        leatherGloves.buyPrice = 25;
        leatherGloves.texturePath = "assets/equipment/leather_gloves.png";
        registerItem(leatherGloves);
    }
    
    {
        ItemData warriorGloves;
        warriorGloves.id = "warrior_gloves";
        warriorGloves.name = "战士手套";
        warriorGloves.description = "战士专用的强化手套";
        warriorGloves.type = ItemType::Equipment;
        warriorGloves.rarity = ItemRarity::Uncommon;
        warriorGloves.maxStack = 1;
        warriorGloves.sellPrice = 25;
        warriorGloves.buyPrice = 100;
        warriorGloves.texturePath = "assets/equipment/warrior_gloves.png";
        registerItem(warriorGloves);
    }
    
    // 鞋子
    {
        ItemData leatherBoots;
        leatherBoots.id = "leather_boots";
        leatherBoots.name = "皮靴";
        leatherBoots.description = "简单的皮制靴子";
        leatherBoots.type = ItemType::Equipment;
        leatherBoots.rarity = ItemRarity::Common;
        leatherBoots.maxStack = 1;
        leatherBoots.sellPrice = 8;
        leatherBoots.buyPrice = 35;
        leatherBoots.texturePath = "assets/equipment/leather_boots.png";
        registerItem(leatherBoots);
    }
    
    // 披风
    {
        ItemData simpleCape;
        simpleCape.id = "simple_cape";
        simpleCape.name = "简易披风";
        simpleCape.description = "一件普通的披风";
        simpleCape.type = ItemType::Equipment;
        simpleCape.rarity = ItemRarity::Common;
        simpleCape.maxStack = 1;
        simpleCape.sellPrice = 10;
        simpleCape.buyPrice = 45;
        simpleCape.texturePath = "assets/equipment/simple_cape.png";
        registerItem(simpleCape);
    }
    
    // 盾牌
    {
        ItemData woodenShield;
        woodenShield.id = "wooden_shield";
        woodenShield.name = "木盾";
        woodenShield.description = "简单的木制盾牌";
        woodenShield.type = ItemType::Equipment;
        woodenShield.rarity = ItemRarity::Common;
        woodenShield.maxStack = 1;
        woodenShield.sellPrice = 12;
        woodenShield.buyPrice = 50;
        woodenShield.texturePath = "assets/equipment/wooden_shield.png";
        registerItem(woodenShield);
    }
    
    {
        ItemData ironShield;
        ironShield.id = "iron_shield";
        ironShield.name = "铁盾";
        ironShield.description = "坚固的铁制盾牌";
        ironShield.type = ItemType::Equipment;
        ironShield.rarity = ItemRarity::Uncommon;
        ironShield.maxStack = 1;
        ironShield.sellPrice = 40;
        ironShield.buyPrice = 170;
        ironShield.texturePath = "assets/equipment/iron_shield.png";
        registerItem(ironShield);
    }
    
    // ========================================
    // 宠物相关物品 (Pet Items)
    // 存放目录: assets/pet/
    // ========================================
    
    // 兔子精元
    {
        ItemData rabbitEssence;
        rabbitEssence.id = "rabbit_essence";
        rabbitEssence.name = "兔子精元";
        rabbitEssence.description = "蕴含兔子灵魂的神秘精元，可用于孵化宠物兔。掉落概率：2%";
        rabbitEssence.type = ItemType::Material;
        rabbitEssence.rarity = ItemRarity::Rare;
        rabbitEssence.maxStack = 99;
        rabbitEssence.sellPrice = 50;
        rabbitEssence.buyPrice = 200;
        rabbitEssence.texturePath = "assets/pet/rabbit_essence.png";
        registerItem(rabbitEssence);
    }
    
    // 孵化强化剂 - 通用（已废弃，保留兼容）
    {
        ItemData hatchEnhancer;
        hatchEnhancer.id = "hatch_enhancer";
        hatchEnhancer.name = "通用强化剂";
        hatchEnhancer.description = "通用的孵化强化剂。注意：每种精元有对应的特殊强化剂材料！";
        hatchEnhancer.type = ItemType::Consumable;
        hatchEnhancer.rarity = ItemRarity::Uncommon;
        hatchEnhancer.maxStack = 99;
        hatchEnhancer.sellPrice = 20;
        hatchEnhancer.buyPrice = 80;
        hatchEnhancer.texturePath = "assets/pet/hatch_enhancer.png";
        registerItem(hatchEnhancer);
    }
    
    // 注：兔毛(rabbit_fur)已在上面定义，它同时作为兔子精元的专用强化剂
    // 每种精元对应的强化剂材料：
    //   - 兔子精元 -> 兔毛 (rabbit_fur)
    //   - 史莱姆精元 -> 粘液 (slime_goo) [未来添加]
    //   - 小鸡精元 -> 羽毛 (feather) [未来添加]
    
    // 宠物洗涤剂
    {
        ItemData petCleanser;
        petCleanser.id = "pet_cleanser";
        petCleanser.name = "宠物洗涤剂";
        petCleanser.description = "可重置宠物资质和属性。将宠物变回1级，重新随机资质（受幸运值影响）";
        petCleanser.type = ItemType::Consumable;
        petCleanser.rarity = ItemRarity::Rare;
        petCleanser.maxStack = 20;
        petCleanser.sellPrice = 100;
        petCleanser.buyPrice = 500;
        petCleanser.texturePath = "assets/pet/pet_cleanser.png";
        registerItem(petCleanser);
    }
    
    // 宠物经验药水
    {
        ItemData petExpPotion;
        petExpPotion.id = "pet_exp_potion";
        petExpPotion.name = "宠物经验药水";
        petExpPotion.description = "给宠物服用可获得500点经验值";
        petExpPotion.type = ItemType::Consumable;
        petExpPotion.rarity = ItemRarity::Uncommon;
        petExpPotion.maxStack = 20;
        petExpPotion.sellPrice = 30;
        petExpPotion.buyPrice = 120;
        petExpPotion.texturePath = "assets/pet/pet_exp_potion.png";
        petExpPotion.effects.push_back(ConsumableEffect(EffectType::BuffExp, 500));
        registerItem(petExpPotion);
    }
    
    // 宠物生命药水
    {
        ItemData petHealthPotion;
        petHealthPotion.id = "pet_health_potion";
        petHealthPotion.name = "宠物生命药水";
        petHealthPotion.description = "给宠物服用可恢复50点生命值";
        petHealthPotion.type = ItemType::Consumable;
        petHealthPotion.rarity = ItemRarity::Common;
        petHealthPotion.maxStack = 20;
        petHealthPotion.sellPrice = 15;
        petHealthPotion.buyPrice = 60;
        petHealthPotion.texturePath = "assets/pet/pet_health_potion.png";
        petHealthPotion.effects.push_back(ConsumableEffect(EffectType::RestoreHealth, 50));
        registerItem(petHealthPotion);
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
