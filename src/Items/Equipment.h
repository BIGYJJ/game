#pragma once
#include "Item.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <functional>
#include <map>
#include <sstream>
#include <random>
#include <vector>

// ============================================================================
// 装备系统 (Equipment System)
// 
// 基于冒险岛战士职业设计
// ============================================================================
// 
// 【装备槽位】
//   - Weapon:     武器（剑、斧、锤等）
//   - SecondHand: 副手（盾牌）
//   - Helmet:     头盔
//   - Armor:      上衣/铠甲
//   - Pants:      裤子
//   - Gloves:     手套
//   - Boots:      鞋子
//   - Cape:       披风
//   - Necklace:   项链
//   - Ring1:      戒指1
//   - Ring2:      戒指2
//   - Earring:    耳环
//   - Belt:       腰带
//   - Shoulder:   肩甲
//
// 【装备属性】
//   - attack:     攻击力
//   - defense:    防御力
//   - strength:   力量
//   - dexterity:  敏捷
//   - intelligence: 智力
//   - luck:       运气
//   - hp:         生命值加成
//   - mp:         魔法值加成
//   - speed:      移动速度
//   - jump:       跳跃力
//
// 【特殊效果】
//   - ignoreDefense:  无视防御（如斧头无视树木防御）
//   - critRate:       暴击率加成
//   - critDamage:     暴击伤害加成
//   - lifeSteal:      吸血（攻击伤害*吸血率=恢复生命）
//
// 【武器资质系统】
//   - 资质等级：白、绿、蓝、橙、紫、黄、红（7个等级）
//   - 不同资质影响武器的攻击力范围
//   - 可通过锻造获得，锻造时可添加武器魂提升高资质概率
//
// 【武器强化系统】
//   - 强化需要强化石
//   - 强化消耗公式：消耗 = 基础值 + (当前等级 × 当前等级 × 系数)
//   - 每次强化增加固定攻击力
//   - 满级额外属性（如吸血）
//
// ============================================================================

// 装备槽位枚举
enum class EquipmentSlot {
    Weapon,         // 武器
    SecondHand,     // 副手（盾牌）
    Helmet,         // 头盔
    Armor,          // 铠甲
    Pants,          // 裤子
    Gloves,         // 手套
    Boots,          // 鞋子
    Cape,           // 披风
    Necklace,       // 项链
    Ring1,          // 戒指1
    Ring2,          // 戒指2
    Earring,        // 耳环
    Belt,           // 腰带
    Shoulder,       // 肩甲
    Count           // 槽位总数
};

// 武器类型枚举（战士系）
enum class WeaponType {
    None,
    Sword,          // 单手剑
    TwoHandSword,   // 双手剑
    Axe,            // 单手斧
    TwoHandAxe,     // 双手斧
    Mace,           // 单手锤
    TwoHandMace,    // 双手锤
    Spear,          // 枪
    Polearm,        // 矛
    Knife           // 小刀
};

// ============================================================================
// 武器资质系统
// ============================================================================

// 武器资质等级枚举
enum class WeaponQuality {
    White = 0,      // 白
    Green,          // 绿
    Blue,           // 蓝
    Orange,         // 橙
    Purple,         // 紫
    Yellow,         // 黄
    Red,            // 红
    Count
};

// 武器攻击力范围（根据资质）
struct WeaponAttackRange {
    int minAttack;
    int maxAttack;
    
    WeaponAttackRange(int min = 0, int max = 0) : minAttack(min), maxAttack(max) {}
};

// 武器资质配置
struct WeaponQualityConfig {
    std::string weaponId;                   // 武器ID
    bool hasQualitySystem;                  // 是否有资质系统
    bool canEnhance;                        // 是否可强化
    int maxEnhanceLevel;                    // 最大强化等级
    int enhanceAttackBonus;                 // 每级强化攻击力加成
    float maxLevelLifeSteal;                // 满级吸血率
    int fixedIgnoreDefense;                 // 固定无视防御值
    
    // 掉落资质概率（白、绿、蓝、橙、紫、黄、红）
    std::array<float, 7> dropQualityProb;
    // 锻造资质概率（白、绿、蓝、橙、紫、黄、红）
    std::array<float, 7> forgeQualityProb;
    // 各资质攻击力范围
    std::array<WeaponAttackRange, 7> attackRanges;
    
    WeaponQualityConfig()
        : hasQualitySystem(false)
        , canEnhance(false)
        , maxEnhanceLevel(0)
        , enhanceAttackBonus(0)
        , maxLevelLifeSteal(0.0f)
        , fixedIgnoreDefense(0)
    {
        dropQualityProb.fill(0.0f);
        forgeQualityProb.fill(0.0f);
    }
};

// ============================================================================
// 武器强化系统
// ============================================================================

// 强化结果
enum class EnhanceResult {
    Success,            // 成功
    NotEnough,          // 材料不足
    MaxLevel,           // 已满级
    NotEnhanceable      // 不可强化
};

// ============================================================================
// 装备属性结构
// ============================================================================

struct EquipmentStats {
    int attack = 0;         // 攻击力
    int defense = 0;        // 防御力
    int strength = 0;       // 力量
    int dexterity = 0;      // 敏捷
    int intelligence = 0;   // 智力
    int luck = 0;           // 运气
    int hp = 0;             // 生命值加成
    int mp = 0;             // 魔法值加成
    float speed = 0;        // 移动速度
    float jump = 0;         // 跳跃力
    
    // 特殊效果
    bool ignoreDefense = false;     // 无视目标防御（完全无视）
    float ignoreDefenseRate = 0;    // 无视防御比例 (0-1)
    int ignoreDefenseValue = 0;     // 固定无视防御值
    float critRate = 0;             // 暴击率加成
    float critDamage = 0;           // 暴击伤害加成
    float lifeSteal = 0;            // 吸血率
    
    // 运算符重载（用于计算总属性）
    EquipmentStats operator+(const EquipmentStats& other) const {
        EquipmentStats result;
        result.attack = attack + other.attack;
        result.defense = defense + other.defense;
        result.strength = strength + other.strength;
        result.dexterity = dexterity + other.dexterity;
        result.intelligence = intelligence + other.intelligence;
        result.luck = luck + other.luck;
        result.hp = hp + other.hp;
        result.mp = mp + other.mp;
        result.speed = speed + other.speed;
        result.jump = jump + other.jump;
        result.ignoreDefense = ignoreDefense || other.ignoreDefense;
        result.ignoreDefenseRate = std::max(ignoreDefenseRate, other.ignoreDefenseRate);
        result.ignoreDefenseValue = ignoreDefenseValue + other.ignoreDefenseValue;
        result.critRate = critRate + other.critRate;
        result.critDamage = critDamage + other.critDamage;
        result.lifeSteal = lifeSteal + other.lifeSteal;
        return result;
    }
};

// ============================================================================
// 装备数据结构
// ============================================================================

struct EquipmentData {
    std::string id;             // 唯一ID
    std::string name;           // 显示名称
    std::string description;    // 描述
    EquipmentSlot slot;         // 装备槽位
    WeaponType weaponType;      // 武器类型（仅武器有效）
    ItemRarity rarity;          // 稀有度
    int requiredLevel;          // 需求等级
    int requiredStrength;       // 需求力量
    int requiredDexterity;      // 需求敏捷
    EquipmentStats stats;       // 属性
    std::string texturePath;    // 贴图路径
    
    EquipmentData()
        : slot(EquipmentSlot::Weapon)
        , weaponType(WeaponType::None)
        , rarity(ItemRarity::Common)
        , requiredLevel(0)
        , requiredStrength(0)
        , requiredDexterity(0) {}
};

// ============================================================================
// 武器实例（带资质和强化等级的武器）
// ============================================================================

struct WeaponInstance {
    std::string baseWeaponId;       // 基础武器ID
    WeaponQuality quality;          // 资质等级
    int enhanceLevel;               // 强化等级
    int baseAttack;                 // 基础攻击力（由资质决定）
    int totalAttack;                // 总攻击力（基础+强化）
    float lifeSteal;                // 吸血率
    int ignoreDefenseValue;         // 固定无视防御值
    std::string uniqueId;           // 唯一标识符
    
    WeaponInstance()
        : quality(WeaponQuality::White)
        , enhanceLevel(0)
        , baseAttack(0)
        , totalAttack(0)
        , lifeSteal(0.0f)
        , ignoreDefenseValue(0)
    {}
    
    // 获取显示名称（包含资质和强化信息）
    std::string getDisplayName() const;
    
    // 获取资质颜色
    static sf::Color getQualityColor(WeaponQuality q);
    
    // 获取资质名称
    static std::string getQualityName(WeaponQuality q);
};

// ============================================================================
// 装备管理器（单例）
// ============================================================================

class EquipmentManager {
public:
    static EquipmentManager& getInstance();
    
    // 初始化装备定义
    void initialize();
    
    // 获取装备数据
    const EquipmentData* getEquipmentData(const std::string& equipId) const;
    
    // 注册新装备
    void registerEquipment(const EquipmentData& data);
    
    // 获取槽位名称
    static std::string getSlotName(EquipmentSlot slot);
    
    // 获取武器类型名称
    static std::string getWeaponTypeName(WeaponType type);
    
    // ========================================
    // 武器资质系统
    // ========================================
    
    // 注册武器资质配置
    void registerWeaponQualityConfig(const WeaponQualityConfig& config);
    
    // 获取武器资质配置
    const WeaponQualityConfig* getWeaponQualityConfig(const std::string& weaponId) const;
    
    // 生成武器实例（怪物掉落）
    WeaponInstance generateWeaponFromDrop(const std::string& weaponId);
    
    // 生成武器实例（锻造，可指定武器魂数量）
    WeaponInstance forgeWeapon(const std::string& weaponId, int weaponSoulCount = 0);
    
    // 计算锻造概率
    std::array<float, 7> calculateForgeProb(const std::string& weaponId, int weaponSoulCount) const;
    
    // ========================================
    // 武器强化系统
    // ========================================
    
    // 计算强化所需强化石数量
    int calculateEnhanceCost(const std::string& weaponId, int currentLevel) const;
    
    // 强化武器
    EnhanceResult enhanceWeapon(WeaponInstance& weapon, int& enhanceStoneCount);
    
    // 更新武器总攻击力
    void updateWeaponStats(WeaponInstance& weapon);

private:
    EquipmentManager() = default;
    std::map<std::string, EquipmentData> equipments;
    std::map<std::string, WeaponQualityConfig> qualityConfigs;
    bool initialized = false;
    
    // 随机数生成器
    std::mt19937 rng{std::random_device{}()};
    
    // 根据概率数组随机选择资质
    WeaponQuality randomQuality(const std::array<float, 7>& probs);
    
    // 在攻击力范围内随机
    int randomAttack(const WeaponAttackRange& range);
    
    // 生成唯一ID
    std::string generateUniqueId();
};

// ============================================================================
// 玩家装备栏
// ============================================================================

class PlayerEquipment {
public:
    using EquipCallback = std::function<void(EquipmentSlot, const std::string&)>;
    
    PlayerEquipment();
    
    // 装备物品（通过装备ID）
    // 返回被替换的装备ID（如果有）
    std::string equip(const std::string& equipId);
    
    // 装备物品（通过装备数据）
    // 返回被替换的装备物品（ItemStack，可能为空）
    ItemStack equip(const EquipmentData& equipData);
    
    // 装备武器实例
    WeaponInstance equipWeapon(const WeaponInstance& weapon);
    
    // 卸下装备
    // 返回被卸下的装备ID
    std::string unequip(EquipmentSlot slot);
    
    // 卸下装备并返回ItemStack
    ItemStack unequipToStack(EquipmentSlot slot);
    
    // 卸下武器并返回WeaponInstance
    WeaponInstance unequipWeapon();
    
    // 获取指定槽位的装备ID
    const std::string& getEquippedItem(EquipmentSlot slot) const;
    
    // 检查槽位是否有装备
    bool hasEquipment(EquipmentSlot slot) const;
    
    // 获取当前武器实例
    const WeaponInstance* getEquippedWeapon() const;
    WeaponInstance* getEquippedWeaponMutable();
    
    // 获取总属性加成
    EquipmentStats getTotalStats() const;
    
    // 检查是否有无视防御效果
    bool hasIgnoreDefense() const;
    float getIgnoreDefenseRate() const;
    int getIgnoreDefenseValue() const;
    
    // 获取当前武器类型
    WeaponType getCurrentWeaponType() const;
    
    // 回调设置
    void setOnEquip(EquipCallback cb) { onEquip = cb; }
    void setOnUnequip(EquipCallback cb) { onUnequip = cb; }

private:
    std::array<std::string, static_cast<size_t>(EquipmentSlot::Count)> equippedItems;
    WeaponInstance equippedWeapon;  // 当前装备的武器实例
    bool hasWeaponInstance;         // 是否有武器实例
    EquipCallback onEquip;
    EquipCallback onUnequip;
};

// ============================================================================
// 装备面板 UI
// ============================================================================

class EquipmentPanel {
public:
    using UnequipCallback = std::function<void(EquipmentSlot)>;
    
    EquipmentPanel();
    
    // 初始化
    bool init(const std::string& iconPath);
    bool loadFont(const std::string& fontPath);
    
    // 设置关联的装备栏
    void setEquipment(PlayerEquipment* eq) { equipment = eq; }
    void setPlayerEquipment(PlayerEquipment* eq) { equipment = eq; }  // Alias
    
    // 设置卸下装备回调
    void setOnUnequip(UnequipCallback cb) { onUnequipCallback = cb; }
    
    // 设置图标位置
    void setIconPosition(float x, float y);
    
    // 更新
    void update(float dt);
    
    // 处理事件
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    
    // 渲染
    void render(sf::RenderWindow& window);
    
    // 打开/关闭
    void open();
    void close();
    void toggle();
    bool isOpen() const { return panelOpen; }

private:
    void renderSlot(sf::RenderWindow& window, EquipmentSlot slot, 
                   const sf::Vector2f& pos, const sf::Vector2f& size);
    void renderTooltip(sf::RenderWindow& window);
    void renderWeaponTooltip(sf::RenderWindow& window);
    sf::Vector2f getSlotPosition(EquipmentSlot slot) const;
    EquipmentSlot getSlotAtPosition(const sf::Vector2f& pos) const;

private:
    PlayerEquipment* equipment;
    
    // 回调
    UnequipCallback onUnequipCallback;
    
    // UI状态
    bool panelOpen;
    EquipmentSlot hoveredSlot;
    EquipmentSlot selectedSlot;
    
    // 图标
    sf::Texture iconTexture;
    sf::Sprite iconSprite;
    sf::Vector2f iconPosition;
    bool iconLoaded;
    
    // 图标悬浮动画
    bool iconHovered;
    float iconHoverScale;
    float iconTargetScale;
    static constexpr float ICON_BASE_SCALE = 0.8f;
    
    // 面板
    sf::Vector2f panelPosition;
    sf::Vector2f panelSize;
    
    // 字体
    sf::Font font;
    bool fontLoaded;
    
    // 角色形象贴图（可选）
    sf::Texture characterTexture;
    bool characterLoaded;
    
    // 颜色
    static const sf::Color BG_COLOR;
    static const sf::Color SLOT_COLOR;
    static const sf::Color SLOT_HOVER_COLOR;
    static const sf::Color SLOT_EQUIPPED_COLOR;
    static const sf::Color BORDER_COLOR;
};
