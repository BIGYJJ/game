#include "Equipment.h"
#include <iostream>
#include <algorithm>
#include <ctime>
#include <sstream>

// ============================================================================
// 颜色常量
// ============================================================================

const sf::Color EquipmentPanel::BG_COLOR(30, 30, 40, 240);
const sf::Color EquipmentPanel::SLOT_COLOR(50, 50, 60, 200);
const sf::Color EquipmentPanel::SLOT_HOVER_COLOR(70, 70, 90, 220);
const sf::Color EquipmentPanel::SLOT_EQUIPPED_COLOR(60, 80, 60, 220);
const sf::Color EquipmentPanel::BORDER_COLOR(139, 90, 43);

// ============================================================================
// EquipmentManager 实现
// ============================================================================

EquipmentManager& EquipmentManager::getInstance() {
    static EquipmentManager instance;
    return instance;
}

void EquipmentManager::initialize() {
    if (initialized) return;
    
    std::cout << "[EquipmentManager] Initializing equipment definitions..." << std::endl;
    
    // ========================================
    // 武器 - 斧头（无视树木类防御）
    // 资质：白色100%，无攻击力
    // ========================================
    {
        EquipmentData axe;
        axe.id = "axe";
        axe.name = "斧头";
        axe.description = "用于砍伐的工具，可以无视树木类的防御值";
        axe.slot = EquipmentSlot::Weapon;
        axe.weaponType = WeaponType::Axe;
        axe.rarity = ItemRarity::Common;  // 白色100%
        axe.requiredLevel = 1;
        axe.stats.attack = 0;  // 无攻击力
        axe.stats.ignoreDefense = true;  // 无视树木类防御
        axe.stats.ignoreDefenseRate = 1.0f;
        axe.texturePath = "assets/weapon/axe.png";
        registerEquipment(axe);
    }
    
    // ========================================
    // 武器 - 小刀（有资质系统，可强化）
    // 资质概率（怪物掉落）：白70%，绿20%，蓝10%
    // 资质概率（锻造）：白60%，绿30%，蓝10%
    // 攻击力：白(1-3)，绿(2-5)，蓝(3-7)，橙(5-10)，紫(7-12)，黄(10-15)，红(18-22)
    // 强化最高等级：3，每次强化+3攻击力
    // ========================================
    {
        EquipmentData knife;
        knife.id = "knife";
        knife.name = "小刀";
        knife.description = "锋利的小刀，可以通过锻造获得不同资质";
        knife.slot = EquipmentSlot::Weapon;
        knife.weaponType = WeaponType::Sword;  // 类似单手剑
        knife.rarity = ItemRarity::Common;  // 基础白色
        knife.requiredLevel = 1;
        // 基础攻击力（白色资质中间值）
        knife.stats.attack = 2;
        knife.texturePath = "assets/weapon/knife.png";
        registerEquipment(knife);
    }
    
    // ========================================
    // 武器 - 长矛（有资质系统，无视防御值2）
    // 资质概率（怪物掉落）：白70%，绿20%，蓝10%
    // 资质概率（锻造）：白50%，绿35%，蓝15%
    // 攻击力：白(3-5)，绿(4-8)，蓝(6-10)，橙(8-13)，紫(13-18)，黄(18-20)，红(22-25)
    // 无视防御值：2
    // 强化最高等级：8，每次强化+3攻击力，满级额外吸血+2%
    // ========================================
    {
        EquipmentData spear;
        spear.id = "spear";
        spear.name = "长矛";
        spear.description = "长杆武器，攻击距离较远，可以无视部分防御值";
        spear.slot = EquipmentSlot::Weapon;
        spear.weaponType = WeaponType::Spear;
        spear.rarity = ItemRarity::Common;  // 基础白色
        spear.requiredLevel = 1;
        // 基础攻击力（白色资质中间值）
        spear.stats.attack = 4;
        // 无视防御值2（特殊属性）
        spear.stats.ignoreDefense = true;
        spear.stats.ignoreDefenseRate = 0.0f;  // 不是完全无视，而是固定减2
        spear.texturePath = "assets/weapon/spear.png";
        registerEquipment(spear);
    }
    
    // ========================================
    // 防具 - 头盔
    // ========================================
    {
        EquipmentData helmet;
        helmet.id = "leather_cap";
        helmet.name = "皮帽";
        helmet.description = "简单的皮制帽子";
        helmet.slot = EquipmentSlot::Helmet;
        helmet.rarity = ItemRarity::Common;
        helmet.requiredLevel = 1;
        helmet.stats.defense = 2;
        helmet.stats.hp = 10;
        helmet.texturePath = "assets/equipment/leather_cap.png";
        registerEquipment(helmet);
    }
    
    {
        EquipmentData helmet;
        helmet.id = "iron_helmet";
        helmet.name = "铁盔";
        helmet.description = "坚固的铁制头盔";
        helmet.slot = EquipmentSlot::Helmet;
        helmet.rarity = ItemRarity::Uncommon;
        helmet.requiredLevel = 10;
        helmet.requiredStrength = 5;
        helmet.stats.defense = 8;
        helmet.stats.hp = 30;
        helmet.stats.strength = 1;
        helmet.texturePath = "assets/equipment/iron_helmet.png";
        registerEquipment(helmet);
    }
    
    // ========================================
    // 防具 - 铠甲
    // ========================================
    {
        EquipmentData armor;
        armor.id = "leather_armor";
        armor.name = "皮甲";
        armor.description = "轻便的皮制护甲";
        armor.slot = EquipmentSlot::Armor;
        armor.rarity = ItemRarity::Common;
        armor.requiredLevel = 1;
        armor.stats.defense = 5;
        armor.stats.hp = 20;
        armor.texturePath = "assets/equipment/leather_armor.png";
        registerEquipment(armor);
    }
    
    {
        EquipmentData armor;
        armor.id = "iron_armor";
        armor.name = "铁甲";
        armor.description = "厚重的铁制铠甲";
        armor.slot = EquipmentSlot::Armor;
        armor.rarity = ItemRarity::Uncommon;
        armor.requiredLevel = 10;
        armor.requiredStrength = 20;
        armor.stats.defense = 15;
        armor.stats.hp = 50;
        armor.stats.strength = 2;
        armor.texturePath = "assets/equipment/iron_armor.png";
        registerEquipment(armor);
    }
    
    // ========================================
    // 防具 - 裤子
    // ========================================
    {
        EquipmentData pants;
        pants.id = "leather_pants";
        pants.name = "皮裤";
        pants.description = "简单的皮制裤子";
        pants.slot = EquipmentSlot::Pants;
        pants.rarity = ItemRarity::Common;
        pants.requiredLevel = 1;
        pants.stats.defense = 3;
        pants.stats.hp = 10;
        pants.texturePath = "assets/equipment/leather_pants.png";
        registerEquipment(pants);
    }
    
    // ========================================
    // 防具 - 手套
    // ========================================
    {
        EquipmentData gloves;
        gloves.id = "leather_gloves";
        gloves.name = "皮手套";
        gloves.description = "简单的皮制手套";
        gloves.slot = EquipmentSlot::Gloves;
        gloves.rarity = ItemRarity::Common;
        gloves.requiredLevel = 1;
        gloves.stats.defense = 1;
        gloves.stats.attack = 1;
        gloves.texturePath = "assets/equipment/leather_gloves.png";
        registerEquipment(gloves);
    }
    
    {
        EquipmentData gloves;
        gloves.id = "warrior_gloves";
        gloves.name = "战士手套";
        gloves.description = "战士专用的强化手套";
        gloves.slot = EquipmentSlot::Gloves;
        gloves.rarity = ItemRarity::Uncommon;
        gloves.requiredLevel = 10;
        gloves.requiredStrength = 8;
        gloves.stats.defense = 3;
        gloves.stats.attack = 3;
        gloves.stats.strength = 1;
        gloves.texturePath = "assets/equipment/warrior_gloves.png";
        registerEquipment(gloves);
    }
    
    // ========================================
    // 防具 - 鞋子
    // ========================================
    {
        EquipmentData boots;
        boots.id = "leather_boots";
        boots.name = "皮靴";
        boots.description = "简单的皮制靴子";
        boots.slot = EquipmentSlot::Boots;
        boots.rarity = ItemRarity::Common;
        boots.requiredLevel = 1;
        boots.stats.defense = 2;
        boots.stats.speed = 5;
        boots.texturePath = "assets/equipment/leather_boots.png";
        registerEquipment(boots);
    }
    
    // ========================================
    // 饰品 - 披风
    // ========================================
    {
        EquipmentData cape;
        cape.id = "simple_cape";
        cape.name = "简易披风";
        cape.description = "一件普通的披风";
        cape.slot = EquipmentSlot::Cape;
        cape.rarity = ItemRarity::Common;
        cape.requiredLevel = 1;
        cape.stats.defense = 1;
        cape.stats.hp = 5;
        cape.texturePath = "assets/equipment/simple_cape.png";
        registerEquipment(cape);
    }
    
    // ========================================
    // 饰品 - 副手盾牌
    // ========================================
    {
        EquipmentData shield;
        shield.id = "wooden_shield";
        shield.name = "木盾";
        shield.description = "简单的木制盾牌";
        shield.slot = EquipmentSlot::SecondHand;
        shield.rarity = ItemRarity::Common;
        shield.requiredLevel = 1;
        shield.stats.defense = 5;
        shield.texturePath = "assets/equipment/wooden_shield.png";
        registerEquipment(shield);
    }
    
    {
        EquipmentData shield;
        shield.id = "iron_shield";
        shield.name = "铁盾";
        shield.description = "坚固的铁制盾牌";
        shield.slot = EquipmentSlot::SecondHand;
        shield.rarity = ItemRarity::Uncommon;
        shield.requiredLevel = 10;
        shield.requiredStrength = 15;
        shield.stats.defense = 12;
        shield.stats.hp = 20;
        shield.texturePath = "assets/equipment/iron_shield.png";
        registerEquipment(shield);
    }
    
    initialized = true;
    std::cout << "[EquipmentManager] Registered " << equipments.size() << " equipments" << std::endl;
    
    // ========================================
    // 初始化武器资质配置
    // ========================================
    
    // 小刀资质配置
    {
        WeaponQualityConfig config;
        config.weaponId = "knife";
        config.hasQualitySystem = true;
        config.canEnhance = true;
        config.maxEnhanceLevel = 3;
        config.enhanceAttackBonus = 3;
        config.maxLevelLifeSteal = 0.0f;  // 小刀满级无吸血
        config.fixedIgnoreDefense = 0;
        
        // 掉落资质概率：白70%，绿20%，蓝10%
        config.dropQualityProb = {0.70f, 0.20f, 0.10f, 0.0f, 0.0f, 0.0f, 0.0f};
        // 锻造资质概率：白60%，绿30%，蓝10%
        config.forgeQualityProb = {0.60f, 0.30f, 0.10f, 0.0f, 0.0f, 0.0f, 0.0f};
        
        // 各资质攻击力范围
        config.attackRanges[0] = WeaponAttackRange(1, 3);   // 白
        config.attackRanges[1] = WeaponAttackRange(2, 5);   // 绿
        config.attackRanges[2] = WeaponAttackRange(3, 7);   // 蓝
        config.attackRanges[3] = WeaponAttackRange(5, 10);  // 橙
        config.attackRanges[4] = WeaponAttackRange(7, 12);  // 紫
        config.attackRanges[5] = WeaponAttackRange(10, 15); // 黄
        config.attackRanges[6] = WeaponAttackRange(18, 22); // 红
        
        registerWeaponQualityConfig(config);
    }
    
    // 长矛资质配置
    {
        WeaponQualityConfig config;
        config.weaponId = "spear";
        config.hasQualitySystem = true;
        config.canEnhance = true;
        config.maxEnhanceLevel = 8;
        config.enhanceAttackBonus = 3;
        config.maxLevelLifeSteal = 0.02f;  // 满级吸血+2%
        config.fixedIgnoreDefense = 2;     // 无视防御值2
        
        // 掉落资质概率：白70%，绿20%，蓝10%
        config.dropQualityProb = {0.70f, 0.20f, 0.10f, 0.0f, 0.0f, 0.0f, 0.0f};
        // 锻造资质概率：白50%，绿35%，蓝15%
        config.forgeQualityProb = {0.50f, 0.35f, 0.15f, 0.0f, 0.0f, 0.0f, 0.0f};
        
        // 各资质攻击力范围
        config.attackRanges[0] = WeaponAttackRange(3, 5);   // 白
        config.attackRanges[1] = WeaponAttackRange(4, 8);   // 绿
        config.attackRanges[2] = WeaponAttackRange(6, 10);  // 蓝
        config.attackRanges[3] = WeaponAttackRange(8, 13);  // 橙
        config.attackRanges[4] = WeaponAttackRange(13, 18); // 紫
        config.attackRanges[5] = WeaponAttackRange(18, 20); // 黄
        config.attackRanges[6] = WeaponAttackRange(22, 25); // 红
        
        registerWeaponQualityConfig(config);
    }
    
    // 斧头资质配置（无资质系统，不可强化）
    {
        WeaponQualityConfig config;
        config.weaponId = "axe";
        config.hasQualitySystem = false;
        config.canEnhance = false;
        config.maxEnhanceLevel = 0;
        config.enhanceAttackBonus = 0;
        config.maxLevelLifeSteal = 0.0f;
        config.fixedIgnoreDefense = 0;  // 斧头是完全无视树木类防御，在其他地方处理
        
        registerWeaponQualityConfig(config);
    }
    
    std::cout << "[EquipmentManager] Registered " << qualityConfigs.size() << " weapon quality configs" << std::endl;
}

const EquipmentData* EquipmentManager::getEquipmentData(const std::string& equipId) const {
    auto it = equipments.find(equipId);
    if (it != equipments.end()) {
        return &it->second;
    }
    return nullptr;
}

void EquipmentManager::registerEquipment(const EquipmentData& data) {
    equipments[data.id] = data;
    std::cout << "[EquipmentManager] Registered: " << data.id << " (" << data.name << ")" << std::endl;
}

std::string EquipmentManager::getSlotName(EquipmentSlot slot) {
    switch (slot) {
        case EquipmentSlot::Weapon:     return "武器";
        case EquipmentSlot::SecondHand: return "副手";
        case EquipmentSlot::Helmet:     return "头盔";
        case EquipmentSlot::Armor:      return "铠甲";
        case EquipmentSlot::Pants:      return "裤子";
        case EquipmentSlot::Gloves:     return "手套";
        case EquipmentSlot::Boots:      return "鞋子";
        case EquipmentSlot::Cape:       return "披风";
        case EquipmentSlot::Necklace:   return "项链";
        case EquipmentSlot::Ring1:      return "戒指";
        case EquipmentSlot::Ring2:      return "戒指";
        case EquipmentSlot::Earring:    return "耳环";
        case EquipmentSlot::Belt:       return "腰带";
        case EquipmentSlot::Shoulder:   return "肩甲";
        default:                        return "未知";
    }
}

std::string EquipmentManager::getWeaponTypeName(WeaponType type) {
    switch (type) {
        case WeaponType::Sword:       return "单手剑";
        case WeaponType::TwoHandSword:return "双手剑";
        case WeaponType::Axe:         return "单手斧";
        case WeaponType::TwoHandAxe:  return "双手斧";
        case WeaponType::Mace:        return "单手锤";
        case WeaponType::TwoHandMace: return "双手锤";
        case WeaponType::Spear:       return "枪";
        case WeaponType::Polearm:     return "矛";
        case WeaponType::Knife:       return "小刀";
        default:                      return "无";
    }
}

// ============================================================================
// 武器资质系统实现
// ============================================================================

void EquipmentManager::registerWeaponQualityConfig(const WeaponQualityConfig& config) {
    qualityConfigs[config.weaponId] = config;
    std::cout << "[EquipmentManager] Registered quality config for: " << config.weaponId << std::endl;
}

const WeaponQualityConfig* EquipmentManager::getWeaponQualityConfig(const std::string& weaponId) const {
    auto it = qualityConfigs.find(weaponId);
    if (it != qualityConfigs.end()) {
        return &it->second;
    }
    return nullptr;
}

WeaponQuality EquipmentManager::randomQuality(const std::array<float, 7>& probs) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float roll = dist(rng);
    
    float cumulative = 0.0f;
    for (int i = 0; i < 7; i++) {
        cumulative += probs[i];
        if (roll <= cumulative) {
            return static_cast<WeaponQuality>(i);
        }
    }
    return WeaponQuality::White;
}

int EquipmentManager::randomAttack(const WeaponAttackRange& range) {
    if (range.minAttack >= range.maxAttack) {
        return range.minAttack;
    }
    std::uniform_int_distribution<int> dist(range.minAttack, range.maxAttack);
    return dist(rng);
}

std::string EquipmentManager::generateUniqueId() {
    static int counter = 0;
    std::stringstream ss;
    ss << "weapon_" << std::time(nullptr) << "_" << (counter++);
    return ss.str();
}

WeaponInstance EquipmentManager::generateWeaponFromDrop(const std::string& weaponId) {
    WeaponInstance weapon;
    weapon.baseWeaponId = weaponId;
    weapon.uniqueId = generateUniqueId();
    
    const WeaponQualityConfig* config = getWeaponQualityConfig(weaponId);
    if (!config || !config->hasQualitySystem) {
        // 无资质系统，使用基础属性
        weapon.quality = WeaponQuality::White;
        weapon.baseAttack = 0;
        weapon.enhanceLevel = 0;
        
        const EquipmentData* equipData = getEquipmentData(weaponId);
        if (equipData) {
            weapon.baseAttack = equipData->stats.attack;
        }
        weapon.ignoreDefenseValue = config ? config->fixedIgnoreDefense : 0;
    } else {
        // 使用掉落概率生成资质
        weapon.quality = randomQuality(config->dropQualityProb);
        int qualityIndex = static_cast<int>(weapon.quality);
        weapon.baseAttack = randomAttack(config->attackRanges[qualityIndex]);
        weapon.enhanceLevel = 0;
        weapon.ignoreDefenseValue = config->fixedIgnoreDefense;
    }
    
    updateWeaponStats(weapon);
    
    std::cout << "[EquipmentManager] Generated weapon from drop: " << weaponId 
              << " Quality: " << WeaponInstance::getQualityName(weapon.quality)
              << " Attack: " << weapon.totalAttack << std::endl;
    
    return weapon;
}

WeaponInstance EquipmentManager::forgeWeapon(const std::string& weaponId, int weaponSoulCount) {
    WeaponInstance weapon;
    weapon.baseWeaponId = weaponId;
    weapon.uniqueId = generateUniqueId();
    
    const WeaponQualityConfig* config = getWeaponQualityConfig(weaponId);
    if (!config || !config->hasQualitySystem) {
        // 无资质系统
        weapon.quality = WeaponQuality::White;
        weapon.baseAttack = 0;
        weapon.enhanceLevel = 0;
        
        const EquipmentData* equipData = getEquipmentData(weaponId);
        if (equipData) {
            weapon.baseAttack = equipData->stats.attack;
        }
        weapon.ignoreDefenseValue = config ? config->fixedIgnoreDefense : 0;
    } else {
        // 计算锻造概率（加入武器魂影响）
        std::array<float, 7> probs = calculateForgeProb(weaponId, weaponSoulCount);
        weapon.quality = randomQuality(probs);
        int qualityIndex = static_cast<int>(weapon.quality);
        weapon.baseAttack = randomAttack(config->attackRanges[qualityIndex]);
        weapon.enhanceLevel = 0;
        weapon.ignoreDefenseValue = config->fixedIgnoreDefense;
    }
    
    updateWeaponStats(weapon);
    
    std::cout << "[EquipmentManager] Forged weapon: " << weaponId 
              << " with " << weaponSoulCount << " souls"
              << " Quality: " << WeaponInstance::getQualityName(weapon.quality)
              << " Attack: " << weapon.totalAttack << std::endl;
    
    return weapon;
}

std::array<float, 7> EquipmentManager::calculateForgeProb(const std::string& weaponId, int weaponSoulCount) const {
    const WeaponQualityConfig* config = getWeaponQualityConfig(weaponId);
    if (!config) {
        return {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    }
    
    // 限制武器魂数量为0-30
    weaponSoulCount = std::max(0, std::min(30, weaponSoulCount));
    
    if (weaponSoulCount == 0) {
        return config->forgeQualityProb;
    }
    
    // 锻造概率公式参数
    const float decayLow = 0.08f;   // 低资质衰减系数
    const float decayMid = 0.03f;   // 中资质调整系数
    const float boostHigh = 1.5f;   // 高资质增益系数
    
    std::array<float, 7> result;
    float total = 0.0f;
    
    for (int i = 0; i < 7; i++) {
        float base = config->forgeQualityProb[i];
        
        if (i < 3) {
            // 低资质（白、绿、蓝）衰减
            result[i] = base * std::max(0.0f, 1.0f - decayLow * weaponSoulCount);
        } else if (i < 5) {
            // 中资质（橙、紫）增益
            result[i] = base + (weaponSoulCount * decayMid * 0.1f);
        } else {
            // 高资质（黄、红）增益
            result[i] = base + (weaponSoulCount * boostHigh * 0.02f);
        }
        
        total += result[i];
    }
    
    // 归一化
    if (total > 0) {
        for (int i = 0; i < 7; i++) {
            result[i] /= total;
        }
    }
    
    return result;
}

// ============================================================================
// 武器强化系统实现
// ============================================================================

int EquipmentManager::calculateEnhanceCost(const std::string& weaponId, int currentLevel) const {
    // 公式：消耗 = 基础值 + (当前等级 × 当前等级 × 系数)
    const int baseValue = 10;
    const int coefficient = 2;
    
    return baseValue + (currentLevel * currentLevel * coefficient);
}

EnhanceResult EquipmentManager::enhanceWeapon(WeaponInstance& weapon, int& enhanceStoneCount) {
    const WeaponQualityConfig* config = getWeaponQualityConfig(weapon.baseWeaponId);
    
    if (!config || !config->canEnhance) {
        return EnhanceResult::NotEnhanceable;
    }
    
    if (weapon.enhanceLevel >= config->maxEnhanceLevel) {
        return EnhanceResult::MaxLevel;
    }
    
    int cost = calculateEnhanceCost(weapon.baseWeaponId, weapon.enhanceLevel);
    
    if (enhanceStoneCount < cost) {
        return EnhanceResult::NotEnough;
    }
    
    // 扣除强化石
    enhanceStoneCount -= cost;
    
    // 提升强化等级
    weapon.enhanceLevel++;
    
    // 更新武器属性
    updateWeaponStats(weapon);
    
    std::cout << "[EquipmentManager] Enhanced weapon: " << weapon.baseWeaponId
              << " to level " << weapon.enhanceLevel
              << " Cost: " << cost << " stones"
              << " New Attack: " << weapon.totalAttack << std::endl;
    
    return EnhanceResult::Success;
}

void EquipmentManager::updateWeaponStats(WeaponInstance& weapon) {
    const WeaponQualityConfig* config = getWeaponQualityConfig(weapon.baseWeaponId);
    
    // 计算总攻击力
    weapon.totalAttack = weapon.baseAttack;
    
    if (config && config->canEnhance) {
        weapon.totalAttack += weapon.enhanceLevel * config->enhanceAttackBonus;
        
        // 满级额外属性
        if (weapon.enhanceLevel >= config->maxEnhanceLevel) {
            weapon.lifeSteal = config->maxLevelLifeSteal;
        } else {
            weapon.lifeSteal = 0.0f;
        }
    }
    
    if (config) {
        weapon.ignoreDefenseValue = config->fixedIgnoreDefense;
    }
}

// ============================================================================
// WeaponInstance 实现
// ============================================================================

std::string WeaponInstance::getDisplayName() const {
    const EquipmentData* data = EquipmentManager::getInstance().getEquipmentData(baseWeaponId);
    std::string baseName = data ? data->name : baseWeaponId;
    
    std::stringstream ss;
    ss << "[" << getQualityName(quality) << "] " << baseName;
    
    if (enhanceLevel > 0) {
        ss << " +" << enhanceLevel;
    }
    
    return ss.str();
}

sf::Color WeaponInstance::getQualityColor(WeaponQuality q) {
    switch (q) {
        case WeaponQuality::White:  return sf::Color(255, 255, 255);     // 白
        case WeaponQuality::Green:  return sf::Color(0, 255, 0);         // 绿
        case WeaponQuality::Blue:   return sf::Color(0, 0, 255);         // 蓝
        case WeaponQuality::Orange: return sf::Color(255, 165, 0);       // 橙
        case WeaponQuality::Purple: return sf::Color(128, 0, 128);       // 紫
        case WeaponQuality::Yellow: return sf::Color(255, 215, 0);       // 黄
        case WeaponQuality::Red:    return sf::Color(255, 0, 0);         // 红
        default:                    return sf::Color::White;
    }
}

std::string WeaponInstance::getQualityName(WeaponQuality q) {
    switch (q) {
        case WeaponQuality::White:  return "白";
        case WeaponQuality::Green:  return "绿";
        case WeaponQuality::Blue:   return "蓝";
        case WeaponQuality::Orange: return "橙";
        case WeaponQuality::Purple: return "紫";
        case WeaponQuality::Yellow: return "黄";
        case WeaponQuality::Red:    return "红";
        default:                    return "未知";
    }
}

// ============================================================================
// PlayerEquipment 实现
// ============================================================================

PlayerEquipment::PlayerEquipment()
    : hasWeaponInstance(false)
{
    for (auto& item : equippedItems) {
        item.clear();
    }
}

std::string PlayerEquipment::equip(const std::string& equipId) {
    const EquipmentData* data = EquipmentManager::getInstance().getEquipmentData(equipId);
    if (!data) {
        std::cout << "[PlayerEquipment] Unknown equipment: " << equipId << std::endl;
        return "";
    }
    
    size_t slotIndex = static_cast<size_t>(data->slot);
    std::string replaced = equippedItems[slotIndex];
    
    equippedItems[slotIndex] = equipId;
    
    // 如果装备武器，清除武器实例
    if (data->slot == EquipmentSlot::Weapon) {
        hasWeaponInstance = false;
        equippedWeapon = WeaponInstance();
    }
    
    if (onEquip) {
        onEquip(data->slot, equipId);
    }
    
    std::cout << "[PlayerEquipment] Equipped: " << data->name << " -> " 
              << EquipmentManager::getSlotName(data->slot) << std::endl;
    
    return replaced;
}

ItemStack PlayerEquipment::equip(const EquipmentData& equipData) {
    size_t slotIndex = static_cast<size_t>(equipData.slot);
    std::string replaced = equippedItems[slotIndex];
    
    equippedItems[slotIndex] = equipData.id;
    
    // 如果装备武器，清除武器实例
    if (equipData.slot == EquipmentSlot::Weapon) {
        hasWeaponInstance = false;
        equippedWeapon = WeaponInstance();
    }
    
    if (onEquip) {
        onEquip(equipData.slot, equipData.id);
    }
    
    std::cout << "[PlayerEquipment] Equipped: " << equipData.name << " -> " 
              << EquipmentManager::getSlotName(equipData.slot) << std::endl;
    
    // 返回被替换的装备作为ItemStack
    if (!replaced.empty()) {
        return ItemStack(replaced, 1);
    }
    return ItemStack();
}

WeaponInstance PlayerEquipment::equipWeapon(const WeaponInstance& weapon) {
    WeaponInstance oldWeapon;
    
    // 保存旧武器
    if (hasWeaponInstance) {
        oldWeapon = equippedWeapon;
    }
    
    // 装备新武器
    equippedWeapon = weapon;
    hasWeaponInstance = true;
    equippedItems[static_cast<size_t>(EquipmentSlot::Weapon)] = weapon.baseWeaponId;
    
    if (onEquip) {
        onEquip(EquipmentSlot::Weapon, weapon.baseWeaponId);
    }
    
    std::cout << "[PlayerEquipment] Equipped weapon instance: " << weapon.getDisplayName()
              << " Attack: " << weapon.totalAttack << std::endl;
    
    return oldWeapon;
}

std::string PlayerEquipment::unequip(EquipmentSlot slot) {
    size_t slotIndex = static_cast<size_t>(slot);
    std::string removed = equippedItems[slotIndex];
    
    if (!removed.empty()) {
        equippedItems[slotIndex].clear();
        
        // 如果卸下武器，也清除武器实例
        if (slot == EquipmentSlot::Weapon) {
            hasWeaponInstance = false;
            equippedWeapon = WeaponInstance();
        }
        
        if (onUnequip) {
            onUnequip(slot, removed);
        }
        
        const EquipmentData* data = EquipmentManager::getInstance().getEquipmentData(removed);
        std::cout << "[PlayerEquipment] Unequipped: " 
                  << (data ? data->name : removed) << std::endl;
    }
    
    return removed;
}

WeaponInstance PlayerEquipment::unequipWeapon() {
    WeaponInstance removed;
    
    if (hasWeaponInstance) {
        removed = equippedWeapon;
        hasWeaponInstance = false;
        equippedWeapon = WeaponInstance();
        equippedItems[static_cast<size_t>(EquipmentSlot::Weapon)].clear();
        
        if (onUnequip) {
            onUnequip(EquipmentSlot::Weapon, removed.baseWeaponId);
        }
        
        std::cout << "[PlayerEquipment] Unequipped weapon instance: " << removed.getDisplayName() << std::endl;
    }
    
    return removed;
}

ItemStack PlayerEquipment::unequipToStack(EquipmentSlot slot) {
    std::string removed = unequip(slot);
    if (!removed.empty()) {
        return ItemStack(removed, 1);
    }
    return ItemStack();
}

const std::string& PlayerEquipment::getEquippedItem(EquipmentSlot slot) const {
    static std::string empty;
    size_t slotIndex = static_cast<size_t>(slot);
    if (slotIndex < equippedItems.size()) {
        return equippedItems[slotIndex];
    }
    return empty;
}

bool PlayerEquipment::hasEquipment(EquipmentSlot slot) const {
    return !getEquippedItem(slot).empty();
}

const WeaponInstance* PlayerEquipment::getEquippedWeapon() const {
    if (hasWeaponInstance) {
        return &equippedWeapon;
    }
    return nullptr;
}

WeaponInstance* PlayerEquipment::getEquippedWeaponMutable() {
    if (hasWeaponInstance) {
        return &equippedWeapon;
    }
    return nullptr;
}

EquipmentStats PlayerEquipment::getTotalStats() const {
    EquipmentStats total;
    
    for (size_t i = 0; i < equippedItems.size(); i++) {
        if (!equippedItems[i].empty()) {
            // 武器槽位特殊处理
            if (i == static_cast<size_t>(EquipmentSlot::Weapon) && hasWeaponInstance) {
                // 使用武器实例的属性
                total.attack += equippedWeapon.totalAttack;
                total.lifeSteal += equippedWeapon.lifeSteal;
                total.ignoreDefenseValue += equippedWeapon.ignoreDefenseValue;
                
                // 获取基础武器的其他属性
                const EquipmentData* data = EquipmentManager::getInstance().getEquipmentData(equippedWeapon.baseWeaponId);
                if (data) {
                    // 添加除攻击力外的其他属性
                    total.defense += data->stats.defense;
                    total.strength += data->stats.strength;
                    total.dexterity += data->stats.dexterity;
                    total.intelligence += data->stats.intelligence;
                    total.luck += data->stats.luck;
                    total.hp += data->stats.hp;
                    total.mp += data->stats.mp;
                    total.speed += data->stats.speed;
                    total.jump += data->stats.jump;
                    total.ignoreDefense = total.ignoreDefense || data->stats.ignoreDefense;
                    total.ignoreDefenseRate = std::max(total.ignoreDefenseRate, data->stats.ignoreDefenseRate);
                    total.critRate += data->stats.critRate;
                    total.critDamage += data->stats.critDamage;
                }
            } else {
                const EquipmentData* data = EquipmentManager::getInstance().getEquipmentData(equippedItems[i]);
                if (data) {
                    total = total + data->stats;
                }
            }
        }
    }
    
    return total;
}

bool PlayerEquipment::hasIgnoreDefense() const {
    return getTotalStats().ignoreDefense;
}

float PlayerEquipment::getIgnoreDefenseRate() const {
    return getTotalStats().ignoreDefenseRate;
}

int PlayerEquipment::getIgnoreDefenseValue() const {
    return getTotalStats().ignoreDefenseValue;
}

WeaponType PlayerEquipment::getCurrentWeaponType() const {
    const std::string& weaponId = getEquippedItem(EquipmentSlot::Weapon);
    if (weaponId.empty()) return WeaponType::None;
    
    const EquipmentData* data = EquipmentManager::getInstance().getEquipmentData(weaponId);
    return data ? data->weaponType : WeaponType::None;
}

// ============================================================================
// EquipmentPanel 实现
// ============================================================================

EquipmentPanel::EquipmentPanel()
    : equipment(nullptr)
    , onUnequipCallback(nullptr)
    , panelOpen(false)
    , hoveredSlot(EquipmentSlot::Count)
    , selectedSlot(EquipmentSlot::Count)
    , iconLoaded(false)
    , iconHovered(false)
    , iconHoverScale(1.0f)
    , iconTargetScale(1.0f)
    , fontLoaded(false)
    , characterLoaded(false)
{
    panelSize = sf::Vector2f(350, 450);
}

bool EquipmentPanel::init(const std::string& iconPath) {
    if (iconTexture.loadFromFile(iconPath)) {
        iconSprite.setTexture(iconTexture);
        iconSprite.setScale(0.8f, 0.8f);
        iconLoaded = true;
        std::cout << "[EquipmentPanel] Icon loaded: " << iconPath << std::endl;
    } else {
        sf::Image placeholder;
        placeholder.create(64, 64, sf::Color(80, 60, 100, 200));
        iconTexture.loadFromImage(placeholder);
        iconSprite.setTexture(iconTexture);
        iconLoaded = true;
        std::cout << "[EquipmentPanel] Using placeholder icon" << std::endl;
    }
    
    std::vector<std::string> fontPaths = {
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/simhei.ttf",
        "C:/Windows/Fonts/simsun.ttc",
        "../../assets/fonts/pixel.ttf",
    };
    
    for (const auto& path : fontPaths) {
        if (loadFont(path)) break;
    }
    
    return true;
}

bool EquipmentPanel::loadFont(const std::string& fontPath) {
    if (font.loadFromFile(fontPath)) {
        fontLoaded = true;
        std::cout << "[EquipmentPanel] Font loaded: " << fontPath << std::endl;
        return true;
    }
    return false;
}

void EquipmentPanel::setIconPosition(float x, float y) {
    iconPosition = sf::Vector2f(x, y);
    iconSprite.setPosition(iconPosition);
}

void EquipmentPanel::update(float dt) {
    // 图标悬浮动画
    float scaleSpeed = 8.0f;
    iconHoverScale += (iconTargetScale - iconHoverScale) * scaleSpeed * dt;
    
    // 更新图标缩放（保持中心点）
    sf::Vector2f iconCenter = iconPosition + sf::Vector2f(
        iconTexture.getSize().x * ICON_BASE_SCALE / 2.0f,
        iconTexture.getSize().y * ICON_BASE_SCALE / 2.0f
    );
    iconSprite.setScale(ICON_BASE_SCALE * iconHoverScale, ICON_BASE_SCALE * iconHoverScale);
    iconSprite.setPosition(
        iconCenter.x - iconTexture.getSize().x * ICON_BASE_SCALE * iconHoverScale / 2.0f,
        iconCenter.y - iconTexture.getSize().y * ICON_BASE_SCALE * iconHoverScale / 2.0f
    );
}

void EquipmentPanel::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    
    // 检查图标悬浮
    sf::FloatRect iconBounds(iconPosition.x - 5, iconPosition.y - 5, 60, 60);
    if (iconBounds.contains(mousePosF)) {
        iconHovered = true;
        iconTargetScale = 1.15f;
    } else {
        iconHovered = false;
        iconTargetScale = 1.0f;
    }
    
    // 检查图标点击
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        if (!panelOpen && iconBounds.contains(mousePosF)) {
            open();
            return;
        }
    }
    
    // 键盘快捷键
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::E) {
            toggle();
            return;
        }
        if (event.key.code == sf::Keyboard::Escape && panelOpen) {
            close();
            return;
        }
    }
    
    if (!panelOpen) return;
    
    sf::FloatRect panelBounds(panelPosition, panelSize);
    
    // 鼠标移动
    if (event.type == sf::Event::MouseMoved) {
        if (panelBounds.contains(mousePosF)) {
            hoveredSlot = getSlotAtPosition(mousePosF);
        } else {
            hoveredSlot = EquipmentSlot::Count;
        }
    }
    
    // 鼠标点击
    if (event.type == sf::Event::MouseButtonPressed) {
        sf::FloatRect closeButton(
            panelPosition.x + panelSize.x - 30,
            panelPosition.y + 5,
            25, 25
        );
        
        if (event.mouseButton.button == sf::Mouse::Left) {
            if (closeButton.contains(mousePosF)) {
                close();
                return;
            }
            
            if (!panelBounds.contains(mousePosF)) {
                close();
                return;
            }
            
            // 点击槽位
            EquipmentSlot clickedSlot = getSlotAtPosition(mousePosF);
            if (clickedSlot != EquipmentSlot::Count) {
                selectedSlot = clickedSlot;
            }
        }
        else if (event.mouseButton.button == sf::Mouse::Right) {
            // 右键卸下装备
            EquipmentSlot clickedSlot = getSlotAtPosition(mousePosF);
            if (clickedSlot != EquipmentSlot::Count && equipment && 
                equipment->hasEquipment(clickedSlot)) {
                if (onUnequipCallback) {
                    onUnequipCallback(clickedSlot);
                } else {
                    equipment->unequip(clickedSlot);
                }
            }
        }
    }
}

void EquipmentPanel::render(sf::RenderWindow& window) {
    // 渲染图标
    if (iconLoaded) {
        sf::RectangleShape iconBg(sf::Vector2f(60, 60));
        iconBg.setPosition(iconPosition.x - 5, iconPosition.y - 5);
        iconBg.setFillColor(sf::Color(30, 30, 40, 200));
        iconBg.setOutlineThickness(2);
        iconBg.setOutlineColor(BORDER_COLOR);
        window.draw(iconBg);
        window.draw(iconSprite);
    }
    
    if (!panelOpen || !equipment) return;
    
    // 计算面板位置
    sf::Vector2u windowSize = window.getSize();
    panelPosition.x = (windowSize.x - panelSize.x) / 2;
    panelPosition.y = (windowSize.y - panelSize.y) / 2;
    
    // 绘制背景
    sf::RectangleShape bg(panelSize);
    bg.setPosition(panelPosition);
    bg.setFillColor(BG_COLOR);
    bg.setOutlineThickness(3);
    bg.setOutlineColor(BORDER_COLOR);
    window.draw(bg);
    
    // 标题栏
    sf::RectangleShape titleBar(sf::Vector2f(panelSize.x - 6, 30));
    titleBar.setPosition(panelPosition.x + 3, panelPosition.y + 3);
    titleBar.setFillColor(sf::Color(60, 45, 30, 200));
    window.draw(titleBar);
    
    if (fontLoaded) {
        sf::Text title;
        title.setFont(font);
        std::string titleStr = "装备栏";
        title.setString(sf::String::fromUtf8(titleStr.begin(), titleStr.end()));
        title.setCharacterSize(20);
        title.setFillColor(sf::Color(255, 220, 150));
        title.setPosition(panelPosition.x + 15, panelPosition.y + 5);
        window.draw(title);
    }
    
    // 关闭按钮
    sf::RectangleShape closeBtn(sf::Vector2f(20, 20));
    closeBtn.setPosition(panelPosition.x + panelSize.x - 25, panelPosition.y + 8);
    closeBtn.setFillColor(sf::Color(150, 50, 50, 200));
    closeBtn.setOutlineThickness(1);
    closeBtn.setOutlineColor(sf::Color::White);
    window.draw(closeBtn);
    
    if (fontLoaded) {
        sf::Text closeText;
        closeText.setFont(font);
        closeText.setString("X");
        closeText.setCharacterSize(14);
        closeText.setFillColor(sf::Color::White);
        closeText.setPosition(panelPosition.x + panelSize.x - 21, panelPosition.y + 7);
        window.draw(closeText);
    }
    
    // 绘制角色轮廓（简化版）
    float centerX = panelPosition.x + panelSize.x / 2;
    float centerY = panelPosition.y + 200;
    
    // 角色身体轮廓
    sf::CircleShape head(25);
    head.setFillColor(sf::Color(80, 80, 90, 150));
    head.setOutlineThickness(2);
    head.setOutlineColor(sf::Color(100, 100, 110));
    head.setPosition(centerX - 25, centerY - 80);
    window.draw(head);
    
    sf::RectangleShape body(sf::Vector2f(60, 80));
    body.setFillColor(sf::Color(80, 80, 90, 150));
    body.setOutlineThickness(2);
    body.setOutlineColor(sf::Color(100, 100, 110));
    body.setPosition(centerX - 30, centerY - 30);
    window.draw(body);
    
    // 槽位尺寸
    sf::Vector2f slotSize(50, 50);
    
    // 绘制各装备槽位
    // 头盔 - 头顶
    renderSlot(window, EquipmentSlot::Helmet, 
               sf::Vector2f(centerX - 25, centerY - 140), slotSize);
    
    // 武器 - 左侧
    renderSlot(window, EquipmentSlot::Weapon, 
               sf::Vector2f(centerX - 100, centerY - 40), slotSize);
    
    // 副手 - 右侧
    renderSlot(window, EquipmentSlot::SecondHand, 
               sf::Vector2f(centerX + 50, centerY - 40), slotSize);
    
    // 铠甲 - 身体
    renderSlot(window, EquipmentSlot::Armor, 
               sf::Vector2f(centerX - 25, centerY - 20), slotSize);
    
    // 手套 - 两侧偏下
    renderSlot(window, EquipmentSlot::Gloves, 
               sf::Vector2f(centerX - 100, centerY + 40), slotSize);
    
    // 披风 - 右侧偏上
    renderSlot(window, EquipmentSlot::Cape, 
               sf::Vector2f(centerX + 50, centerY + 40), slotSize);
    
    // 裤子 - 身体下方
    renderSlot(window, EquipmentSlot::Pants, 
               sf::Vector2f(centerX - 25, centerY + 60), slotSize);
    
    // 鞋子 - 最底部
    renderSlot(window, EquipmentSlot::Boots, 
               sf::Vector2f(centerX - 25, centerY + 120), slotSize);
    
    // 饰品区域（右侧）
    float accessoryX = panelPosition.x + panelSize.x - 70;
    float accessoryY = panelPosition.y + 60;
    
    renderSlot(window, EquipmentSlot::Necklace, 
               sf::Vector2f(accessoryX, accessoryY), sf::Vector2f(40, 40));
    renderSlot(window, EquipmentSlot::Earring, 
               sf::Vector2f(accessoryX, accessoryY + 50), sf::Vector2f(40, 40));
    renderSlot(window, EquipmentSlot::Ring1, 
               sf::Vector2f(accessoryX, accessoryY + 100), sf::Vector2f(40, 40));
    renderSlot(window, EquipmentSlot::Ring2, 
               sf::Vector2f(accessoryX, accessoryY + 150), sf::Vector2f(40, 40));
    renderSlot(window, EquipmentSlot::Belt, 
               sf::Vector2f(accessoryX, accessoryY + 200), sf::Vector2f(40, 40));
    renderSlot(window, EquipmentSlot::Shoulder, 
               sf::Vector2f(accessoryX, accessoryY + 250), sf::Vector2f(40, 40));
    
    // 显示总属性
    if (fontLoaded) {
        EquipmentStats stats = equipment->getTotalStats();
        
        float statsX = panelPosition.x + 15;
        float statsY = panelPosition.y + panelSize.y - 80;
        
        sf::Text statsTitle;
        statsTitle.setFont(font);
        std::string statsTitleStr = "属性加成";
        statsTitle.setString(sf::String::fromUtf8(statsTitleStr.begin(), statsTitleStr.end()));
        statsTitle.setCharacterSize(14);
        statsTitle.setFillColor(sf::Color(200, 200, 200));
        statsTitle.setPosition(statsX, statsY);
        window.draw(statsTitle);
        
        std::stringstream ss;
        ss << "ATK+" << stats.attack << "  DEF+" << stats.defense 
           << "  HP+" << stats.hp;
        
        sf::Text statsText;
        statsText.setFont(font);
        statsText.setString(ss.str());
        statsText.setCharacterSize(12);
        statsText.setFillColor(sf::Color(150, 255, 150));
        statsText.setPosition(statsX, statsY + 20);
        window.draw(statsText);
        
        if (stats.ignoreDefense) {
            std::string ignoreStr = "特效: 无视防御";
            sf::Text ignoreText;
            ignoreText.setFont(font);
            ignoreText.setString(sf::String::fromUtf8(ignoreStr.begin(), ignoreStr.end()));
            ignoreText.setCharacterSize(12);
            ignoreText.setFillColor(sf::Color(255, 200, 100));
            ignoreText.setPosition(statsX, statsY + 40);
            window.draw(ignoreText);
        }
    }
    
    // 渲染提示框
    renderTooltip(window);
}

void EquipmentPanel::renderSlot(sf::RenderWindow& window, EquipmentSlot slot, 
                                const sf::Vector2f& pos, const sf::Vector2f& size) {
    bool isHovered = (hoveredSlot == slot);
    bool hasEquip = equipment && equipment->hasEquipment(slot);
    
    sf::RectangleShape slotBg(size);
    slotBg.setPosition(pos);
    
    if (isHovered) {
        slotBg.setFillColor(SLOT_HOVER_COLOR);
    } else if (hasEquip) {
        slotBg.setFillColor(SLOT_EQUIPPED_COLOR);
    } else {
        slotBg.setFillColor(SLOT_COLOR);
    }
    
    slotBg.setOutlineThickness(2);
    slotBg.setOutlineColor(isHovered ? sf::Color::White : sf::Color(80, 80, 80));
    window.draw(slotBg);
    
    // 绘制槽位名称
    if (fontLoaded && !hasEquip) {
        std::string slotName = EquipmentManager::getSlotName(slot);
        sf::Text nameText;
        nameText.setFont(font);
        nameText.setString(sf::String::fromUtf8(slotName.begin(), slotName.end()));
        nameText.setCharacterSize(10);
        nameText.setFillColor(sf::Color(120, 120, 120));
        
        sf::FloatRect bounds = nameText.getLocalBounds();
        nameText.setPosition(
            pos.x + (size.x - bounds.width) / 2,
            pos.y + (size.y - bounds.height) / 2 - 2
        );
        window.draw(nameText);
    }
    
    // TODO: 绘制装备图标
}

void EquipmentPanel::renderTooltip(sf::RenderWindow& window) {
    if (hoveredSlot == EquipmentSlot::Count || !equipment || !fontLoaded) return;
    
    const std::string& equipId = equipment->getEquippedItem(hoveredSlot);
    if (equipId.empty()) return;
    
    const EquipmentData* data = EquipmentManager::getInstance().getEquipmentData(equipId);
    if (!data) return;
    
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    float tooltipX = mousePos.x + 15.0f;
    float tooltipY = mousePos.y + 15.0f;
    
    // 构建提示内容
    std::vector<std::pair<sf::String, sf::Color>> lines;
    
    lines.push_back({sf::String::fromUtf8(data->name.begin(), data->name.end()), 
                     ItemDatabase::getRarityColor(data->rarity)});
    
    std::string slotName = EquipmentManager::getSlotName(data->slot);
    lines.push_back({sf::String::fromUtf8(slotName.begin(), slotName.end()), 
                     sf::Color(150, 150, 150)});
    
    if (!data->description.empty()) {
        lines.push_back({"", sf::Color::White});
        lines.push_back({sf::String::fromUtf8(data->description.begin(), data->description.end()), 
                        sf::Color(200, 200, 200)});
    }
    
    // 属性
    lines.push_back({"", sf::Color::White});
    if (data->stats.attack > 0) {
        std::string str = "攻击力 +" + std::to_string(data->stats.attack);
        lines.push_back({sf::String::fromUtf8(str.begin(), str.end()), sf::Color(255, 100, 100)});
    }
    if (data->stats.defense > 0) {
        std::string str = "防御力 +" + std::to_string(data->stats.defense);
        lines.push_back({sf::String::fromUtf8(str.begin(), str.end()), sf::Color(100, 100, 255)});
    }
    if (data->stats.hp > 0) {
        std::string str = "生命值 +" + std::to_string(data->stats.hp);
        lines.push_back({sf::String::fromUtf8(str.begin(), str.end()), sf::Color(100, 255, 100)});
    }
    if (data->stats.ignoreDefense) {
        std::string str = "无视目标防御";
        lines.push_back({sf::String::fromUtf8(str.begin(), str.end()), sf::Color(255, 200, 100)});
    }
    
    // 计算尺寸
    float lineHeight = 20.0f;
    float padding = 10.0f;
    float maxWidth = 0.0f;
    
    sf::Text measureText;
    measureText.setFont(font);
    measureText.setCharacterSize(14);
    
    for (const auto& line : lines) {
        measureText.setString(line.first);
        float width = measureText.getLocalBounds().width;
        if (width > maxWidth) maxWidth = width;
    }
    
    float tooltipWidth = std::max(160.0f, maxWidth + padding * 2);
    float tooltipHeight = lines.size() * lineHeight + padding * 2;
    
    // 边界检查
    sf::Vector2u windowSize = window.getSize();
    if (tooltipX + tooltipWidth > windowSize.x) tooltipX = windowSize.x - tooltipWidth - 10;
    if (tooltipY + tooltipHeight > windowSize.y) tooltipY = windowSize.y - tooltipHeight - 10;
    
    // 绘制背景
    sf::RectangleShape bg(sf::Vector2f(tooltipWidth, tooltipHeight));
    bg.setPosition(tooltipX, tooltipY);
    bg.setFillColor(sf::Color(20, 20, 30, 245));
    bg.setOutlineThickness(2);
    bg.setOutlineColor(ItemDatabase::getRarityColor(data->rarity));
    window.draw(bg);
    
    // 绘制文字
    float y = tooltipY + padding;
    for (size_t i = 0; i < lines.size(); i++) {
        if (lines[i].first.isEmpty()) {
            y += lineHeight * 0.3f;
            continue;
        }
        
        sf::Text text;
        text.setFont(font);
        text.setString(lines[i].first);
        text.setCharacterSize(i == 0 ? 16 : 14);
        text.setFillColor(lines[i].second);
        text.setPosition(tooltipX + padding, y);
        window.draw(text);
        
        y += lineHeight;
    }
}

sf::Vector2f EquipmentPanel::getSlotPosition(EquipmentSlot slot) const {
    float centerX = panelPosition.x + panelSize.x / 2;
    float centerY = panelPosition.y + 200;
    float accessoryX = panelPosition.x + panelSize.x - 70;
    float accessoryY = panelPosition.y + 60;
    
    switch (slot) {
        case EquipmentSlot::Helmet:     return sf::Vector2f(centerX - 25, centerY - 140);
        case EquipmentSlot::Weapon:     return sf::Vector2f(centerX - 100, centerY - 40);
        case EquipmentSlot::SecondHand: return sf::Vector2f(centerX + 50, centerY - 40);
        case EquipmentSlot::Armor:      return sf::Vector2f(centerX - 25, centerY - 20);
        case EquipmentSlot::Gloves:     return sf::Vector2f(centerX - 100, centerY + 40);
        case EquipmentSlot::Cape:       return sf::Vector2f(centerX + 50, centerY + 40);
        case EquipmentSlot::Pants:      return sf::Vector2f(centerX - 25, centerY + 60);
        case EquipmentSlot::Boots:      return sf::Vector2f(centerX - 25, centerY + 120);
        case EquipmentSlot::Necklace:   return sf::Vector2f(accessoryX, accessoryY);
        case EquipmentSlot::Earring:    return sf::Vector2f(accessoryX, accessoryY + 50);
        case EquipmentSlot::Ring1:      return sf::Vector2f(accessoryX, accessoryY + 100);
        case EquipmentSlot::Ring2:      return sf::Vector2f(accessoryX, accessoryY + 150);
        case EquipmentSlot::Belt:       return sf::Vector2f(accessoryX, accessoryY + 200);
        case EquipmentSlot::Shoulder:   return sf::Vector2f(accessoryX, accessoryY + 250);
        default:                        return sf::Vector2f(0, 0);
    }
}

EquipmentSlot EquipmentPanel::getSlotAtPosition(const sf::Vector2f& pos) const {
    // 检查每个槽位
    for (int i = 0; i < static_cast<int>(EquipmentSlot::Count); i++) {
        EquipmentSlot slot = static_cast<EquipmentSlot>(i);
        sf::Vector2f slotPos = getSlotPosition(slot);
        
        // 确定槽位尺寸
        sf::Vector2f size(50, 50);
        if (slot >= EquipmentSlot::Necklace) {
            size = sf::Vector2f(40, 40);
        }
        
        sf::FloatRect bounds(slotPos.x, slotPos.y, size.x, size.y);
        if (bounds.contains(pos)) {
            return slot;
        }
    }
    
    return EquipmentSlot::Count;
}

void EquipmentPanel::open() {
    panelOpen = true;
    hoveredSlot = EquipmentSlot::Count;
    selectedSlot = EquipmentSlot::Count;
    std::cout << "[EquipmentPanel] Opened" << std::endl;
}

void EquipmentPanel::close() {
    panelOpen = false;
    std::cout << "[EquipmentPanel] Closed" << std::endl;
}

void EquipmentPanel::toggle() {
    if (panelOpen) close();
    else open();
}
