#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <random>

// ============================================================================
// 野生植物系统 (Wild Plant System)
// 
// 基类继承关系：
//   Plant (基类) -> WildPlant (野生植物)
//   具体类型：carrot_plant, bean_plant
//
// 功能：
//   - 可拾取的植物
//   - 随机生成概率
//   - 拾取后获得物品（count_min ~ count_max）
//   - 碰撞体积
// ============================================================================

// 野生植物类型
enum class WildPlantType {
    Carrot,     // 胡萝卜
    Bean,       // 豆子
    Unknown
};

// 掉落物品信息
struct PlantDropInfo {
    std::string itemId;     // 物品ID
    std::string name;       // 物品名称
    int countMin;           // 最小数量
    int countMax;           // 最大数量
    
    PlantDropInfo(const std::string& id = "", const std::string& n = "", 
                  int min = 1, int max = 1)
        : itemId(id), name(n), countMin(min), countMax(max) {}
};

class WildPlant {
public:
    WildPlant();
    WildPlant(float x, float y, const std::string& plantType = "carrot");
    virtual ~WildPlant() = default;
    
    // ========================================
    // 初始化
    // ========================================
    void init(float x, float y, const std::string& plantType);
    
    // 从地图对象的TileProperty动态初始化（推荐使用）
    void initFromTileProperty(float x, float y, const struct TileProperty* prop);
    
    // 设置贴图
    void setTextureFromProperty(const struct TileProperty* prop);
    bool loadTexture(const std::string& texturePath);
    
    // ========================================
    // 更新和渲染
    // ========================================
    void update(float dt);
    void render(sf::RenderWindow& window);
    
    // ========================================
    // 拾取系统
    // ========================================
    
    // 检查是否可以拾取
    bool canPickup() const { return allowPickup && !isPickedUp; }
    
    // 执行拾取，返回获得的物品列表 (itemId, count)
    std::vector<std::pair<std::string, int>> pickup();
    
    // 是否已被拾取
    bool isCollected() const { return isPickedUp; }
    
    // ========================================
    // 碰撞检测
    // ========================================
    bool containsPoint(const sf::Vector2f& point) const;
    bool intersects(const sf::FloatRect& rect) const;
    sf::FloatRect getBounds() const;
    sf::FloatRect getCollisionBox() const;
    
    // ========================================
    // 属性 Getters
    // ========================================
    sf::Vector2f getPosition() const { return position; }
    sf::Vector2f getSize() const { return size; }
    const std::string& getPlantType() const { return plantType; }
    const std::string& getName() const { return name; }
    float getProbability() const { return probability; }
    WildPlantType getType() const { return type; }
    const PlantDropInfo& getDropInfo() const { return dropInfo; }
    
    // ========================================
    // 属性 Setters
    // ========================================
    void setPosition(float x, float y);
    void setPosition(const sf::Vector2f& pos);
    void setSize(float w, float h) { size = sf::Vector2f(w, h); updateSprite(); }
    void setName(const std::string& n) { name = n; }
    void setAllowPickup(bool allow) { allowPickup = allow; }
    void setProbability(float prob) { probability = prob; }
    void setPickupItem(const std::string& itemId, int minCount, int maxCount);
    
    // ========================================
    // 悬浮提示
    // ========================================
    void setHovered(bool hovered) { isHovered = hovered; }
    bool getHovered() const { return isHovered; }
    
    // ========================================
    // 回调
    // ========================================
    using PlantCallback = std::function<void(WildPlant&)>;
    void setOnPickup(PlantCallback cb) { onPickup = cb; }

private:
    void updateSprite();
    WildPlantType parseType(const std::string& typeStr);

private:
    // === 基础属性 ===
    std::string plantType;      // 植物类型名 (carrot, bean)
    std::string name;           // 显示名称
    WildPlantType type;         // 植物类型枚举
    
    // === 拾取属性 ===
    bool allowPickup;           // 是否允许拾取
    PlantDropInfo dropInfo;     // 掉落物品信息
    float probability;          // 生成概率 (0-1)
    bool isPickedUp;            // 是否已被拾取
    
    // === 位置和大小 ===
    sf::Vector2f position;
    sf::Vector2f size;
    sf::FloatRect collisionBox; // 自定义碰撞盒（从tsx读取）
    bool hasCustomCollision;    // 是否有自定义碰撞盒
    
    // === 渲染 ===
    sf::Sprite sprite;
    sf::Texture texture;
    bool textureLoaded;
    
    // === 交互状态 ===
    bool isHovered;
    
    // === 随机数生成 ===
    mutable std::mt19937 rng;
    
    // === 回调 ===
    PlantCallback onPickup;
};

// ============================================================================
// 野生植物管理器
// ============================================================================

class WildPlantManager {
public:
    WildPlantManager();
    
    // 初始化
    bool init(const std::string& assetsPath);
    
    // 更新所有植物
    void update(float dt);
    
    // 渲染所有植物
    void render(sf::RenderWindow& window, const sf::View& view);
    
    // 渲染悬浮提示UI
    void renderTooltips(sf::RenderWindow& window, const sf::Vector2f& mouseWorldPos);
    
    // ========================================
    // 植物管理
    // ========================================
    WildPlant* addPlant(float x, float y, const std::string& type);
    
    // 从TileProperty动态创建植物（推荐使用）
    WildPlant* addPlantFromProperty(float x, float y, const struct TileProperty* prop);
    
    void removePlant(WildPlant* plant);
    void clearAllPlants();
    
    // 从地图对象加载植物
    void loadFromMapObjects(const std::vector<struct MapObject>& objects, 
                            float displayScale);
    
    // ========================================
    // 交互
    // ========================================
    
    // 获取指定位置的植物
    WildPlant* getPlantAt(const sf::Vector2f& position);
    
    // 获取与矩形碰撞的植物
    WildPlant* getPlantInRect(const sf::FloatRect& rect);
    
    // 获取可拾取范围内的植物
    WildPlant* getPickablePlantInRange(const sf::Vector2f& center, float range);
    
    // 拾取植物，返回获得的物品
    std::vector<std::pair<std::string, int>> pickupPlant(WildPlant* plant);
    
    // 更新悬浮状态
    void updateHover(const sf::Vector2f& mouseWorldPos);
    
    // ========================================
    // 碰撞检测
    // ========================================
    bool isCollidingWithAnyPlant(const sf::FloatRect& rect) const;
    
    // ========================================
    // 获取器
    // ========================================
    size_t getPlantCount() const { return plants.size(); }
    const std::vector<std::unique_ptr<WildPlant>>& getPlants() const { return plants; }
    
    // ========================================
    // 移除已拾取的植物
    // ========================================
    void removePickedPlants();
    
    // ========================================
    // 字体设置（用于提示框）
    // ========================================
    bool loadFont(const std::string& fontPath);

private:
    void renderTooltip(sf::RenderWindow& window, WildPlant* plant);
    
private:
    std::vector<std::unique_ptr<WildPlant>> plants;
    std::string assetsBasePath;
    
    // 字体（用于悬浮提示）
    sf::Font font;
    bool fontLoaded;
    
    // 当前悬浮的植物
    WildPlant* hoveredPlant;
};
