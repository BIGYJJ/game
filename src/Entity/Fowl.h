#pragma once
#include "Monster.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>
#include <random>
#include <memory>

// ============================================================================
// 家禽基类 (Fowl Base Class)
// 
// 功能：
//   - 属性：生命值、防御值、速度、攻击力
//   - 饥饿系统：饱腹值、饥饿消耗
//   - 生长系统：生长周期、进化
//   - 产出系统：产出物品、产出周期
//   - 动画：和兔子相同的精灵表结构（128x256，每帧32x32，每行4帧，共8行）
//   - AI：和怪物相同的攻击行为
//   - 碰撞：有碰撞体积
// ============================================================================

// 家禽动画状态
enum class FowlAnimState {
    MoveDown,
    MoveUp,
    MoveLeft,
    MoveRight,
    AttackDown,
    AttackUp,
    AttackLeft,
    AttackRight
};

// 家禽产出物
struct FowlProduct {
    std::string itemId;
    std::string name;
    int minCount;
    int maxCount;
    
    FowlProduct(const std::string& id = "", const std::string& n = "",
                int min = 1, int max = 1)
        : itemId(id), name(n), minCount(min), maxCount(max) {}
};

// ============================================================================
// 家禽基类（继承自Monster）
// ============================================================================
class Fowl : public Monster {
public:
    using FowlCallback = std::function<void(Fowl&)>;
    
    Fowl();
    virtual ~Fowl() = default;
    
    // ========================================
    // Monster接口实现
    // ========================================
    void update(float dt, const sf::Vector2f& playerPos) override;
    void render(sf::RenderWindow& window) override;
    std::string getName() const override { return fowlName; }
    std::string getTypeName() const override { return fowlTypeName; }
    
    // ========================================
    // 初始化
    // ========================================
    void init(float x, float y) override;
    bool loadTexture(const std::string& texturePath) override;
    void randomizeStats() override;
    
    // ========================================
    // 碰撞检测
    // ========================================
    sf::FloatRect getCollisionBox() const override;
    
    // ========================================
    // 饥饿系统
    // ========================================
    
    // 获取饱腹值
    float getSatiety() const { return satiety; }
    float getMaxSatiety() const { return maxSatiety; }
    float getSatietyPercent() const { return maxSatiety > 0 ? satiety / maxSatiety : 0; }
    
    // 喂食（增加饱腹值）
    bool feed(float amount);
    
    // 检查是否饥饿（饱腹值为0）
    bool isHungry() const { return satiety <= 0; }
    
    // 获取每日饥饿消耗
    float getDailyHunger() const { return dailyHunger; }
    
    // ========================================
    // 生长系统
    // ========================================
    
    // 获取生长进度（0-1）
    float getGrowthProgress() const;
    
    // 检查是否可以进化
    bool canEvolve() const { return canEvolveFlag && growthDays >= growthCycleDays; }
    
    // 获取进化后的类型名称
    virtual std::string getEvolveTo() const { return evolveTo; }
    
    // 检查是否可以生长
    bool isGrowing() const { return canEvolveFlag; }
    
    // 获取生长天数
    int getGrowthDays() const { return growthDays; }
    int getGrowthCycleDays() const { return growthCycleDays; }
    
    // ========================================
    // 产出系统
    // ========================================
    
    // 检查是否可以产出
    bool canProduce() const { return hasProduct && productTimer >= productCycleDays; }
    
    // 获取产出物
    virtual std::vector<std::pair<std::string, int>> harvest();
    
    // 获取产出信息
    const FowlProduct& getProduct() const { return product; }
    float getProductProgress() const;
    int getProductCycleDays() const { return productCycleDays; }
    bool hasProductAbility() const { return hasProduct; }
    
    // ========================================
    // 时间更新（每游戏日调用）
    // ========================================
    virtual void onDayPass();
    
    // ========================================
    // 回调设置
    // ========================================
    void setOnEvolve(FowlCallback cb) { onEvolve = cb; }
    void setOnProduce(FowlCallback cb) { onProduce = cb; }
    void setOnHungry(FowlCallback cb) { onHungry = cb; }

protected:
    void updateAnimation(float dt);
    void updateAI(float dt, const sf::Vector2f& playerPos);
    void setAnimState(FowlAnimState state);
    void updateSprite() override;
    sf::IntRect getFrameRect(int row, int col) const;
    void updateDirectionFromVelocityFowl();
    
protected:
    // === 家禽名称 ===
    std::string fowlName;
    std::string fowlTypeName;
    
    // === 饥饿系统 ===
    float satiety;          // 当前饱腹值
    float maxSatiety;       // 最大饱腹值
    float dailyHunger;      // 每日消耗的饱腹值
    
    // === 生长系统 ===
    bool canEvolveFlag;     // 是否可以进化
    std::string evolveTo;   // 进化后的类型
    int growthDays;         // 已生长天数
    int growthCycleDays;    // 生长周期（天）
    
    // === 产出系统 ===
    bool hasProduct;        // 是否有产出能力
    FowlProduct product;    // 产出物
    int productCycleDays;   // 产出周期（天）
    float productTimer;     // 产出计时器（天）
    
    // === 是否可以攻击 ===
    bool canAttack;
    
    // === 游荡计时 ===
    float wanderTimer;
    float wanderDuration;
    float idleTimer;
    
    // === 动画 ===
    FowlAnimState animState;
    FowlAnimState lastAnimState;
    int currentFrame;
    float animTimer;
    float frameTime;
    bool animationLocked;
    float animLockTimer;
    
    // 精灵表参数（128x256，每帧32x32，每行4帧，共8行）
    static constexpr int FRAME_WIDTH = 32;
    static constexpr int FRAME_HEIGHT = 32;
    static constexpr int FRAMES_PER_ROW = 4;
    
    // 行定义（精灵表布局）
    static constexpr int ROW_MOVE_DOWN = 0;
    static constexpr int ROW_MOVE_UP = 1;
    static constexpr int ROW_MOVE_LEFT = 2;
    static constexpr int ROW_MOVE_RIGHT = 3;
    static constexpr int ROW_ATTACK_DOWN = 4;
    static constexpr int ROW_ATTACK_UP = 5;
    static constexpr int ROW_ATTACK_LEFT = 6;
    static constexpr int ROW_ATTACK_RIGHT = 7;
    
    // === 回调 ===
    FowlCallback onEvolve;
    FowlCallback onProduce;
    FowlCallback onHungry;
    
    // === 动画常量 ===
    static constexpr float MIN_ANIM_DURATION = 0.2f;
};

// ============================================================================
// 家禽管理器
// ============================================================================
class FowlManager {
public:
    FowlManager();
    
    // 初始化
    bool init(const std::string& chickTexturePath, const std::string& henTexturePath);
    
    // 更新所有家禽
    void update(float dt, const sf::Vector2f& playerPos);
    
    // 渲染所有家禽
    void render(sf::RenderWindow& window, const sf::View& view);
    
    // 渲染悬浮提示
    void renderTooltips(sf::RenderWindow& window, const sf::Vector2f& mouseWorldPos);
    
    // ========================================
    // 家禽管理
    // ========================================
    
    // 添加小鸡
    Fowl* addChick(float x, float y);
    
    // 添加母鸡
    Fowl* addHen(float x, float y);
    
    // 移除家禽
    void removeFowl(Fowl* fowl);
    
    // 清空所有家禽
    void clearAllFowls();
    
    // ========================================
    // 时间系统
    // ========================================
    
    // 每日更新（处理饥饿、生长、产出）
    void onDayPass();
    
    // 处理进化
    void processEvolutions();
    
    // ========================================
    // 交互
    // ========================================
    
    // 获取指定位置的家禽
    Fowl* getFowlAt(const sf::Vector2f& position);
    
    // 获取与矩形碰撞的家禽
    Fowl* getFowlInRect(const sf::FloatRect& rect);
    
    // 对范围内的家禽造成伤害
    std::vector<Fowl*> damageFowlsInRange(const sf::Vector2f& center,
                                           float radius, float damage,
                                           bool ignoreDefense = false);
    
    // 获取范围内正在攻击的家禽
    std::vector<Fowl*> getAttackingFowlsInRange(const sf::Vector2f& center, float radius);
    
    // 更新悬浮状态
    void updateHover(const sf::Vector2f& mouseWorldPos);
    
    // ========================================
    // 碰撞检测
    // ========================================
    bool isCollidingWithAnyFowl(const sf::FloatRect& rect) const;
    std::vector<Fowl*> pushFowlsFromRect(const sf::FloatRect& moverBox, float pushStrength = 1.0f);
    std::vector<Fowl*> getFowlsCollidingWith(const sf::FloatRect& rect) const;
    std::vector<Fowl*> getMovingFowlsCollidingWith(const sf::FloatRect& rect) const;
    
    // ========================================
    // 获取器
    // ========================================
    size_t getFowlCount() const { return fowls.size(); }
    const std::vector<std::unique_ptr<Fowl>>& getFowls() const { return fowls; }
    
    // 获取可收获的家禽
    std::vector<Fowl*> getHarvestableFowls();
    
    // 获取可进化的家禽
    std::vector<Fowl*> getEvolvableFowls();
    
    // ========================================
    // 字体设置
    // ========================================
    bool loadFont(const std::string& fontPath);

private:
    void renderTooltip(sf::RenderWindow& window, Fowl* fowl);
    
private:
    std::vector<std::unique_ptr<Fowl>> fowls;
    std::string chickTexturePath;
    std::string henTexturePath;
    
    // 字体
    sf::Font font;
    bool fontLoaded;
    
    // 当前悬浮的家禽
    Fowl* hoveredFowl;
    
    // 随机数生成器
    std::mt19937 rng;
};
