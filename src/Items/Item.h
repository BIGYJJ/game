#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <map>
#include <memory>

// ============================================================================
// 物品系统 (Item System)
// 
// 设计规则：
// ============================================================================
// 
// 【物品类型 ItemType】
//   - Material:    材料类（木材、树枝、石头等），不可使用，用于合成
//   - Consumable:  消耗品（苹果、樱桃、药水等），可使用，有效果
//   - Equipment:   装备类（武器、防具等），可装备
//   - Quest:       任务物品，不可丢弃
//   - Misc:        杂物
//
// 【物品属性】
//   - id:          唯一标识符（如 "wood", "apple"）
//   - name:        显示名称（如 "木材", "苹果"）
//   - type:        物品类型
//   - maxStack:    最大堆叠数量（材料通常99，消耗品通常20）
//   - sellPrice:   出售价格（金币）
//   - buyPrice:    购买价格（金币）
//   - rarity:      稀有度（影响名称颜色）
//
// 【消耗品效果 ConsumableEffect】
//   - effectType:  效果类型（恢复生命、恢复体力、增加属性等）
//   - effectValue: 效果数值
//   - duration:    持续时间（0表示即时效果）
//
// 【资源目录结构】
//   - assets/materials/    材料贴图（wood.png, stick.png, stone.png...）
//   - assets/consumables/  消耗品贴图（apple.png, cherry.png, potion.png...）
//   - assets/equipment/    装备贴图
//   - assets/items/        通用物品贴图
//
// ============================================================================

// 物品类型枚举
enum class ItemType {
    Material,       // 材料
    Consumable,     // 消耗品
    Equipment,      // 装备
    Quest,          // 任务物品
    Misc            // 杂物
};

// 物品稀有度（影响名称显示颜色）
enum class ItemRarity {
    Common,         // 普通 - 白色
    Uncommon,       // 优秀 - 绿色
    Rare,           // 稀有 - 蓝色
    Epic,           // 史诗 - 紫色
    Legendary       // 传说 - 橙色
};

// 消耗品效果类型
enum class EffectType {
    None,
    RestoreHealth,      // 恢复生命
    RestoreStamina,     // 恢复体力
    RestoreMana,        // 恢复魔力
    BuffAttack,         // 增加攻击
    BuffDefense,        // 增加防御
    BuffSpeed,          // 增加速度
    BuffExp             // 增加经验获取
};

// 消耗品效果结构
struct ConsumableEffect {
    EffectType type;
    float value;            // 效果数值
    float duration;         // 持续时间（秒），0表示即时效果
    
    ConsumableEffect() : type(EffectType::None), value(0), duration(0) {}
    ConsumableEffect(EffectType t, float v, float d = 0) 
        : type(t), value(v), duration(d) {}
};

// ============================================================================
// 物品数据定义（静态数据，定义物品的基本属性）
// ============================================================================

struct ItemData {
    std::string id;             // 唯一ID
    std::string name;           // 显示名称
    std::string description;    // 物品描述
    ItemType type;              // 物品类型
    ItemRarity rarity;          // 稀有度
    int maxStack;               // 最大堆叠数
    int sellPrice;              // 出售价格
    int buyPrice;               // 购买价格
    std::string texturePath;    // 贴图路径
    
    // 消耗品专用
    std::vector<ConsumableEffect> effects;
    
    ItemData() 
        : type(ItemType::Misc)
        , rarity(ItemRarity::Common)
        , maxStack(99)
        , sellPrice(1)
        , buyPrice(10) {}
};

// ============================================================================
// 物品实例（背包中的实际物品，包含数量等运行时数据）
// ============================================================================

struct ItemStack {
    std::string itemId;         // 物品ID
    int count;                  // 数量
    
    ItemStack() : count(0) {}
    ItemStack(const std::string& id, int c) : itemId(id), count(c) {}
    
    bool isEmpty() const { return itemId.empty() || count <= 0; }
    void clear() { itemId.clear(); count = 0; }
};

// ============================================================================
// 物品数据库（单例模式，存储所有物品定义）
// ============================================================================

class ItemDatabase {
public:
    static ItemDatabase& getInstance();
    
    // 初始化所有物品定义
    void initialize();
    
    // 加载物品贴图
    bool loadTextures(const std::string& basePath);
    
    // 获取物品数据
    const ItemData* getItemData(const std::string& itemId) const;
    
    // 获取物品贴图
    const sf::Texture* getTexture(const std::string& itemId) const;
    
    // 注册新物品
    void registerItem(const ItemData& data);
    
    // 获取稀有度对应的颜色
    static sf::Color getRarityColor(ItemRarity rarity);
    
    // 获取物品类型的中文名
    static std::string getTypeName(ItemType type);

private:
    ItemDatabase() = default;
    ItemDatabase(const ItemDatabase&) = delete;
    ItemDatabase& operator=(const ItemDatabase&) = delete;
    
    std::map<std::string, ItemData> items;
    std::map<std::string, sf::Texture> textures;
    bool initialized = false;
};

// ============================================================================
// 辅助函数
// ============================================================================

// 创建物品堆叠
ItemStack createItemStack(const std::string& itemId, int count = 1);

// 检查物品是否可以堆叠
bool canStackItems(const ItemStack& a, const ItemStack& b);

// 合并物品堆叠（返回剩余数量）
int mergeItemStacks(ItemStack& dest, ItemStack& src, int maxStack);
