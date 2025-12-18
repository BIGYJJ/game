#pragma once
#include "Item.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <functional>

// ============================================================================
// 掉落物品系统 (Dropped Item System)
// 
// 设计规则：
// ============================================================================
// 
// 【掉落计算规则】
//   树木的 drop_type 定义掉落物品列表，如: "wood", "seed", "stick"
//   每个物品有独立的掉落概率: drop1_Probability, drop2_Probability, ...
//   drop_max 定义单个物品的最大掉落数量
//   
//   掉落数量计算（递减概率）:
//     假设 wood 的基础概率为 0.7, drop_max = 5
//     - 第1个: 概率 0.7
//     - 第2个: 概率 0.7 * 0.7 = 0.49
//     - 第3个: 概率 0.7 * 0.7 * 0.7 = 0.343
//     - 依此类推...
//     最终数量 = 连续成功的次数
//
// 【掉落物品显示规则】
//   - 每种物品显示为一个图标 + 数量
//   - 不同物品不重叠，水平排列
//   - 物品从树木位置向周围散开
//   - 物品有轻微的上下浮动动画
//
// 【拾取规则】
//   - 玩家碰撞到掉落物品时自动拾取
//   - 拾取范围: 玩家周围一定半径内
//   - 拾取后物品消失，添加到背包
//   - 背包满时无法拾取
//
// ============================================================================

// 单个掉落物品实体
class DroppedItem {
public:
    DroppedItem();
    DroppedItem(const std::string& itemId, int count, float x, float y);
    
    void update(float dt);
    void render(sf::RenderWindow& window);
    
    // 获取碰撞区域
    sf::FloatRect getBounds() const;
    
    // 检查点是否在拾取范围内
    bool isInPickupRange(const sf::Vector2f& point, float range) const;
    
    // Getters
    const std::string& getItemId() const { return itemId; }
    int getCount() const { return count; }
    sf::Vector2f getPosition() const { return position; }
    bool isPickedUp() const { return pickedUp; }
    bool isExpired() const { return lifetime <= 0; }
    
    // 标记为已拾取
    void markPickedUp() { pickedUp = true; }
    
    // 设置贴图
    void setTexture(const sf::Texture* tex);
    
    // 设置共享字体（供 DroppedItemManager 调用）
    static void setSharedFont(sf::Font* font) { sharedFont = font; }

private:
    std::string itemId;
    int count;
    sf::Vector2f position;
    sf::Vector2f velocity;          // 初始散开速度
    float groundY;                  // 地面Y坐标（初始位置）
    bool onGround;                  // 是否已落地
    float lifetime;                 // 存活时间（秒）
    float floatTimer;               // 浮动动画计时器
    float floatOffset;              // 浮动偏移
    bool pickedUp;
    
    sf::Sprite sprite;
    const sf::Texture* texture;
    bool hasTexture;
    
    // 显示数量文字
    static sf::Font* sharedFont;
    sf::Text countText;
};

// ============================================================================
// 掉落物品管理器
// ============================================================================

class DroppedItemManager {
public:
    DroppedItemManager();
    
    // 初始化
    bool init(const std::string& assetsPath);
    bool loadFont(const std::string& fontPath);
    
    // 更新所有掉落物品
    void update(float dt);
    
    // 渲染所有掉落物品
    void render(sf::RenderWindow& window, const sf::View& view);
    
    // ========================================
    // 生成掉落物品
    // ========================================
    
    // 在指定位置生成单个物品
    void spawnItem(const std::string& itemId, int count, float x, float y);
    
    // 在指定位置生成多个物品（散开显示）
    void spawnItems(const std::vector<std::pair<std::string, int>>& items, float x, float y);
    
    // ========================================
    // 掉落计算
    // ========================================
    
    // 根据掉落配置计算实际掉落物品
    // dropTypes: 物品ID列表
    // dropProbabilities: 对应的掉落概率
    // dropMax: 单个物品最大数量
    // 返回: (物品ID, 数量) 列表
    static std::vector<std::pair<std::string, int>> calculateDrops(
        const std::vector<std::string>& dropTypes,
        const std::vector<float>& dropProbabilities,
        int dropMax);
    
    // ========================================
    // 拾取系统
    // ========================================
    
    // 检查并拾取范围内的物品
    // 返回拾取的物品列表
    std::vector<ItemStack> pickupItemsInRange(const sf::Vector2f& position, float range);
    
    // 清理已拾取和过期的物品
    void cleanup();
    
    // 清除所有掉落物品
    void clearAll();
    
    // ========================================
    // 回调
    // ========================================
    using PickupCallback = std::function<void(const ItemStack&)>;
    void setOnItemPickup(PickupCallback cb) { onItemPickup = cb; }
    
    // 获取掉落物品数量
    size_t getDroppedItemCount() const { return droppedItems.size(); }

private:
    std::vector<std::unique_ptr<DroppedItem>> droppedItems;
    std::string assetsBasePath;
    
    sf::Font font;
    bool fontLoaded;
    
    PickupCallback onItemPickup;
    
    // 物品存活时间（秒）
    static constexpr float ITEM_LIFETIME = 300.0f;  // 5分钟
};
