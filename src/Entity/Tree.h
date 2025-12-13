#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>
#include <memory>

// ============================================================================
// 树木系统
// 
// 功能：
//   - 生长阶段：幼苗 -> 成长 -> 成熟 -> 有果实
//   - 属性：生命值、防御、掉落物品
//   - 交互：砍伐、采摘、悬浮提示
//   - 变换：普通树成熟后可随机变成苹果树或樱桃树
// ============================================================================

// 生长阶段
enum class TreeGrowthStage {
    Seedling,       // 幼苗
    Growing,        // 成长中
    Mature,         // 成熟（无果实）
    Fruiting        // 有果实
};

// 掉落物品信息
struct DropItem {
    std::string itemId;     // 物品ID
    std::string name;       // 物品名称
    int minCount;           // 最小数量
    int maxCount;           // 最大数量
    float dropChance;       // 掉落概率 (0-1)
    
    DropItem(const std::string& id = "", const std::string& n = "", 
             int min = 1, int max = 1, float chance = 1.0f)
        : itemId(id), name(n), minCount(min), maxCount(max), dropChance(chance) {}
};

// 掉落动画粒子
struct DropParticle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float lifetime;
    float maxLifetime;
    std::string itemId;
    bool active;
    
    DropParticle() : lifetime(0), maxLifetime(1.0f), active(false) {}
};

class Tree {
public:
    Tree();
    Tree(float x, float y, const std::string& treeType = "oak");
    
    // ========================================
    // 初始化
    // ========================================
    void init(float x, float y, const std::string& treeType);
    bool loadTextures(const std::string& basePath);
    
    // ========================================
    // 更新
    // ========================================
    void update(float dt);
    
    // ========================================
    // 渲染
    // ========================================
    void render(sf::RenderWindow& window);
    void renderDropParticles(sf::RenderWindow& window);
    
    // ========================================
    // 交互
    // ========================================
    
    // 受到伤害，返回是否被砍倒
    bool takeDamage(float damage);
    
    // 采摘果实，返回是否成功
    bool harvestFruit();
    
    // 检查点是否在树木范围内
    bool containsPoint(const sf::Vector2f& point) const;
    
    // 检查矩形是否与树木碰撞
    bool intersects(const sf::FloatRect& rect) const;
    
    // ========================================
    // 生长系统
    // ========================================
    void grow(float dt);
    void setGrowthStage(TreeGrowthStage stage);
    TreeGrowthStage getGrowthStage() const { return growthStage; }
    float getGrowthProgress() const;  // 0-1 当前阶段进度
    std::string getGrowthStageName() const;
    
    // ========================================
    // 变换系统（普通树变成果树）
    // ========================================
    void transformToFruitTree();
    bool canBeTransformed() const { return canTransform && !hasTransformed; }
    void setCanTransform(bool can) { canTransform = can; }
    
    // ========================================
    // 属性 Getters
    // ========================================
    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    float getHealthPercent() const { return health / maxHealth; }
    float getDefense() const { return defense; }
    const std::string& getTreeType() const { return treeType; }
    const std::string& getName() const { return name; }
    bool isDead() const { return health <= 0; }
    bool hasFruit() const { return growthStage == TreeGrowthStage::Fruiting; }
    
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
    sf::FloatRect getCollisionBox() const;  // 碰撞盒（通常比视觉范围小）
    
    // ========================================
    // 掉落物品
    // ========================================
    void addDropItem(const DropItem& item);
    void clearDropItems();
    const std::vector<DropItem>& getDropItems() const { return dropItems; }
    const std::vector<DropItem>& getFruitDropItems() const { return fruitDropItems; }
    void addFruitDropItem(const DropItem& item);
    
    // 生成掉落（返回实际掉落的物品列表）
    std::vector<std::pair<std::string, int>> generateDrops();
    std::vector<std::pair<std::string, int>> generateFruitDrops();
    
    // ========================================
    // 悬浮提示
    // ========================================
    void setHovered(bool hovered) { isHovered = hovered; }
    bool getHovered() const { return isHovered; }
    
    // ========================================
    // 回调
    // ========================================
    using TreeCallback = std::function<void(Tree&)>;
    void setOnDestroyed(TreeCallback cb) { onDestroyed = cb; }
    void setOnFruitHarvested(TreeCallback cb) { onFruitHarvested = cb; }
    void setOnGrowthStageChanged(TreeCallback cb) { onGrowthStageChanged = cb; }

private:
    void updateSprite();
    void spawnDropParticles(const sf::Vector2f& pos, int count);
    void updateDropParticles(float dt);
    
private:
    // === 基础属性 ===
    std::string treeType;       // 树木类型 (oak, pine, apple, etc.)
    std::string name;           // 显示名称
    float health;
    float maxHealth;
    float defense;
    
    // === 位置 ===
    sf::Vector2f position;
    sf::Vector2f size;          // 树木尺寸
    
    // === 生长 ===
    TreeGrowthStage growthStage;
    float growthTimer;          // 当前阶段已生长时间
    float seedlingTime;         // 幼苗阶段所需时间
    float growingTime;          // 成长阶段所需时间
    float matureTime;           // 成熟到结果所需时间
    float fruitRegrowTime;      // 果实再生时间
    
    // === 变换 ===
    bool canTransform;          // 是否可以变换成果树
    bool hasTransformed;        // 是否已经变换过
    
    // === 渲染 ===
    sf::Sprite sprite;
    sf::Texture textureSeedling;
    sf::Texture textureGrowing;
    sf::Texture textureMature;
    sf::Texture textureFruiting;
    sf::Texture* currentTexture;
    bool texturesLoaded;
    
    // === 掉落物品 ===
    std::vector<DropItem> dropItems;        // 砍伐掉落
    std::vector<DropItem> fruitDropItems;   // 果实掉落
    std::vector<DropParticle> dropParticles;
    
    // === 交互状态 ===
    bool isHovered;
    float shakeTimer;           // 被砍时的震动
    float shakeIntensity;
    
    // === 回调 ===
    TreeCallback onDestroyed;
    TreeCallback onFruitHarvested;
    TreeCallback onGrowthStageChanged;
};

// ============================================================================
// 树木管理器
// ============================================================================

class TreeManager {
public:
    TreeManager();
    
    // 初始化
    bool init(const std::string& assetsPath);
    
    // 更新所有树木
    void update(float dt);
    
    // 渲染所有树木
    void render(sf::RenderWindow& window, const sf::View& view);
    
    // 渲染悬浮提示UI（在默认视图下调用）
    void renderTooltips(sf::RenderWindow& window, const sf::Vector2f& mouseWorldPos);
    
    // ========================================
    // 树木管理
    // ========================================
    Tree* addTree(float x, float y, const std::string& type = "oak");
    void removeTree(Tree* tree);
    void clearAllTrees();
    
    // 从地图对象加载树木
    void loadFromMapObjects(const std::vector<struct MapObject>& objects, 
                           float displayScale);
    
    // ========================================
    // 交互
    // ========================================
    
    // 获取指定位置的树木
    Tree* getTreeAt(const sf::Vector2f& position);
    
    // 获取与矩形碰撞的树木
    Tree* getTreeInRect(const sf::FloatRect& rect);
    
    // 对范围内的树木造成伤害
    std::vector<Tree*> damageTreesInRange(const sf::Vector2f& center, 
                                          float radius, float damage);
    
    // 更新悬浮状态
    void updateHover(const sf::Vector2f& mouseWorldPos);
    
    // ========================================
    // 碰撞检测
    // ========================================
    bool isCollidingWithAnyTree(const sf::FloatRect& rect) const;
    
    // ========================================
    // 获取器
    // ========================================
    size_t getTreeCount() const { return trees.size(); }
    const std::vector<std::unique_ptr<Tree>>& getTrees() const { return trees; }
    
    // ========================================
    // 字体设置（用于提示框）
    // ========================================
    bool loadFont(const std::string& fontPath);

private:
    void renderTooltip(sf::RenderWindow& window, Tree* tree);
    
private:
    std::vector<std::unique_ptr<Tree>> trees;
    std::string assetsBasePath;
    
    // 字体（用于悬浮提示）
    sf::Font font;
    bool fontLoaded;
    
    // 当前悬浮的树木
    Tree* hoveredTree;
};