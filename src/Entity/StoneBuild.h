#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>
#include <memory>

// ============================================================================
// 石头建筑系统 (Stone Build System)
// 
// 继承关系：Build (基类) -> StoneBuild
// 与 Tree 类似，但用于石头类型的可破坏建筑物
//
// 功能：
//   - 生命值、防御力
//   - 掉落物品（石头等）
//   - 交互：采矿、悬浮提示
// ============================================================================

// 掉落物品信息（与Tree共用结构）
struct StoneDropItem {
    std::string itemId;     // 物品ID
    std::string name;       // 物品名称
    int minCount;           // 最小数量
    int maxCount;           // 最大数量
    float dropChance;       // 掉落概率 (0-1)
    
    StoneDropItem(const std::string& id = "", const std::string& n = "", 
                  int min = 1, int max = 1, float chance = 1.0f)
        : itemId(id), name(n), minCount(min), maxCount(max), dropChance(chance) {}
};

class StoneBuild {
public:
    StoneBuild();
    StoneBuild(float x, float y, const std::string& stoneType = "stone_build");
    
    // ========================================
    // 初始化
    // ========================================
    void init(float x, float y, const std::string& stoneType);
    
    // 从地图对象的TileProperty动态初始化（推荐使用）
    void initFromTileProperty(float x, float y, const struct TileProperty* prop);
    
    // 设置贴图（用于collection of images类型的tileset）
    void setTextureFromProperty(const struct TileProperty* prop);
    
    bool loadTexture(const std::string& texturePath);
    
    // ========================================
    // 更新
    // ========================================
    void update(float dt);
    
    // ========================================
    // 渲染
    // ========================================
    void render(sf::RenderWindow& window);
    
    // ========================================
    // 交互
    // ========================================
    
    // 受到伤害，返回是否被摧毁
    bool takeDamage(float damage);
    
    // 检查点是否在石头范围内
    bool containsPoint(const sf::Vector2f& point) const;
    
    // 检查矩形是否与石头碰撞
    bool intersects(const sf::FloatRect& rect) const;
    
    // ========================================
    // 属性 Getters
    // ========================================
    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    float getHealthPercent() const { return health / maxHealth; }
    float getDefense() const { return defense; }
    const std::string& getStoneType() const { return stoneType; }
    const std::string& getName() const { return name; }
    bool isDead() const { return health <= 0; }
    
    // ========================================
    // 属性 Setters
    // ========================================
    void setHealth(float hp) { health = std::max(0.0f, std::min(hp, maxHealth)); }
    void setMaxHealth(float hp) { maxHealth = hp; health = std::min(health, maxHealth); }
    void setDefense(float def) { defense = def; }
    void setName(const std::string& n) { name = n; }
    
    // ========================================
    // 位置和碰撞
    // ========================================
    sf::Vector2f getPosition() const { return position; }
    void setPosition(float x, float y) { position = sf::Vector2f(x, y); updateSprite(); }
    void setSize(float w, float h) { size = sf::Vector2f(w, h); updateSprite(); }
    sf::Vector2f getSize() const { return size; }
    sf::FloatRect getBounds() const;
    sf::FloatRect getCollisionBox() const;  // 碰撞盒
    
    // ========================================
    // 掉落物品
    // ========================================
    void addDropItem(const StoneDropItem& item);
    void clearDropItems();
    const std::vector<StoneDropItem>& getDropItems() const { return dropItems; }
    
    // 生成掉落（返回实际掉落的物品列表）
    std::vector<std::pair<std::string, int>> generateDrops();
    
    // 获取击杀奖励（随机范围内）
    int getExpReward() const;
    int getGoldReward() const;
    int getExpMin() const { return expMin; }
    int getExpMax() const { return expMax; }
    int getGoldMin() const { return goldMin; }
    int getGoldMax() const { return goldMax; }
    int getDropMax() const { return dropMax; }
    
    // ========================================
    // 悬浮提示
    // ========================================
    void setHovered(bool hovered) { isHovered = hovered; }
    bool getHovered() const { return isHovered; }
    
    // ========================================
    // 回调
    // ========================================
    using StoneCallback = std::function<void(StoneBuild&)>;
    void setOnDestroyed(StoneCallback cb) { onDestroyed = cb; }

private:
    void updateSprite();
    
private:
    // === 基础属性 ===
    std::string stoneType;      // 石头类型
    std::string name;           // 显示名称
    float health;
    float maxHealth;
    float defense;
    
    // === 位置 ===
    sf::Vector2f position;
    sf::Vector2f size;          // 尺寸
    
    // === 渲染 ===
    sf::Sprite sprite;
    sf::Texture texture;
    bool textureLoaded;
    
    // === 掉落物品 ===
    std::vector<StoneDropItem> dropItems;
    
    // === 击杀奖励 ===
    int expMin;                 // 最小经验
    int expMax;                 // 最大经验
    int goldMin;                // 最小金币
    int goldMax;                // 最大金币
    int dropMax;                // 单个物品最大掉落数量
    
    // === 交互状态 ===
    bool isHovered;
    float shakeTimer;           // 被敲击时的震动
    float shakeIntensity;
    
    // === 回调 ===
    StoneCallback onDestroyed;
};

// ============================================================================
// 石头建筑管理器
// ============================================================================

class StoneBuildManager {
public:
    StoneBuildManager();
    
    // 初始化
    bool init(const std::string& assetsPath);
    
    // 更新所有石头
    void update(float dt);
    
    // 渲染所有石头
    void render(sf::RenderWindow& window, const sf::View& view);
    
    // 渲染悬浮提示UI（在默认视图下调用）
    void renderTooltips(sf::RenderWindow& window, const sf::Vector2f& mouseWorldPos);
    
    // ========================================
    // 石头管理
    // ========================================
    StoneBuild* addStone(float x, float y, const std::string& type = "stone_build");
    
    // 从TileProperty动态创建石头（推荐使用）
    StoneBuild* addStoneFromProperty(float x, float y, const struct TileProperty* prop);
    
    void removeStone(StoneBuild* stone);
    void clearAllStones();
    
    // 从地图对象加载石头
    void loadFromMapObjects(const std::vector<struct MapObject>& objects, 
                            float displayScale);
    
    // ========================================
    // 交互
    // ========================================
    
    // 获取指定位置的石头
    StoneBuild* getStoneAt(const sf::Vector2f& position);
    
    // 获取与矩形碰撞的石头
    StoneBuild* getStoneInRect(const sf::FloatRect& rect);
    
    // 对范围内的石头造成伤害
    std::vector<StoneBuild*> damageStonesInRange(const sf::Vector2f& center, 
                                                  float radius, float damage);
    
    // 更新悬浮状态
    void updateHover(const sf::Vector2f& mouseWorldPos);
    
    // ========================================
    // 碰撞检测
    // ========================================
    bool isCollidingWithAnyStone(const sf::FloatRect& rect) const;
    
    // ========================================
    // 获取器
    // ========================================
    size_t getStoneCount() const { return stones.size(); }
    const std::vector<std::unique_ptr<StoneBuild>>& getStones() const { return stones; }
    
    // ========================================
    // 字体设置（用于提示框）
    // ========================================
    bool loadFont(const std::string& fontPath);

private:
    void renderTooltip(sf::RenderWindow& window, StoneBuild* stone);
    
private:
    std::vector<std::unique_ptr<StoneBuild>> stones;
    std::string assetsBasePath;
    
    // 字体（用于悬浮提示）
    sf::Font font;
    bool fontLoaded;
    
    // 当前悬浮的石头
    StoneBuild* hoveredStone;
};
