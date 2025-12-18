#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>
#include <random>
#include <memory>
// ============================================================================
// 兔子怪物系统 (改进版)
// 
// 功能：
//   - 属性：生命值、防御、攻击力、闪避
//   - 动画：上下左右移动、攻击（4帧平滑动画）
//   - AI：被攻击后会在一定范围内追击玩家（类似饥荒）
//   - 掉落：兔毛、胡萝卜、兔肉
//   - 碰撞：有碰撞体积
// ============================================================================

// 兔子动画状态
enum class RabbitAnimState {
    MoveDown,
    MoveUp,
    MoveLeft,
    MoveRight,
    AttackDown,
    AttackUp,
    AttackLeft,
    AttackRight
};

// 兔子AI状态
enum class RabbitAIState {
    Idle,           // 站立
    Wandering,      // 随机游荡
    Chasing,        // 追击玩家（被攻击后）
    Attacking,      // 近身攻击
    Returning       // 返回原位（超出追击范围）
};

// 兔子朝向
enum class RabbitDirection {
    Down,
    Up,
    Left,
    Right
};

// 兔子掉落物品
struct RabbitDrop {
    std::string itemId;
    std::string name;
    int minCount;
    int maxCount;
    float dropChance;  // 0-1
    
    RabbitDrop(const std::string& id = "", const std::string& n = "",
               int min = 1, int max = 1, float chance = 1.0f)
        : itemId(id), name(n), minCount(min), maxCount(max), dropChance(chance) {}
};

class Rabbit {
public:
    Rabbit();
    Rabbit(float x, float y);
    
    // 初始化
    void init(float x, float y);
    bool loadTexture(const std::string& texturePath);
    
    // 设置随机属性（在给定范围内）
    void randomizeStats();
    
    // 更新
    void update(float dt, const sf::Vector2f& playerPos);
    
    // 渲染
    void render(sf::RenderWindow& window);
    
    // ========================================
    // 战斗交互
    // ========================================
    
    // 受到伤害，返回是否死亡
    bool takeDamage(float damage, bool ignoreDefense = false);
    
    // 执行攻击
    float performAttack();
    
    // 闪避判定（根据闪避值）
    bool rollDodge() const;
    
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
    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    float getHealthPercent() const { return health / maxHealth; }
    float getDefense() const { return defense; }
    float getAttack() const { return attack; }
    int getDodge() const { return dodge; }
    bool isDead() const { return health <= 0; }
    bool isAttacking() const { return aiState == RabbitAIState::Attacking; }
    bool isAggressive() const { return isAggroed; }
    RabbitAIState getAIState() const { return aiState; }
    
    sf::Vector2f getPosition() const { return position; }
    void setPosition(float x, float y);
    
    // ========================================
    // 掉落物品
    // ========================================
    std::vector<std::pair<std::string, int>> generateDrops() const;
    int getExpReward() const;
    int getGoldReward() const;
    
    // ========================================
    // 悬浮提示
    // ========================================
    void setHovered(bool hovered) { isHovered = hovered; }
    bool getHovered() const { return isHovered; }
    std::string getName() const { return "兔子"; }
    
    // ========================================
    // 回调
    // ========================================
    using RabbitCallback = std::function<void(Rabbit&)>;
    void setOnDeath(RabbitCallback cb) { onDeath = cb; }
    void setOnAttack(RabbitCallback cb) { onAttack = cb; }

private:
    void updateAnimation(float dt);
    void updateAI(float dt, const sf::Vector2f& playerPos);
    void setAnimState(RabbitAnimState state);
    void updateSprite();
    sf::IntRect getFrameRect(int row, int col) const;
    void updateDirectionFromVelocity();
    
private:
    // === 基础属性 ===
    float health;
    float maxHealth;
    float defense;
    float attack;
    int dodge;          // 闪避值 0-200, 1点 = 0.5%闪避几率
    int level;
    
    // === 掉落奖励范围 ===
    int expMin, expMax;
    int goldMin, goldMax;
    
    // === 掉落物品 ===
    std::vector<RabbitDrop> drops;
    
    // === 位置 ===
    sf::Vector2f position;
    sf::Vector2f size;
    sf::Vector2f velocity;
    sf::Vector2f homePosition;  // 原始位置（用于返回）
    
    // === AI状态 ===
    RabbitAIState aiState;
    RabbitDirection direction;
    bool isAggroed;          // 是否被激怒
    float aggroTimer;        // 激怒持续时间
    float wanderTimer;       // 游荡计时
    float wanderDuration;    // 当前游荡持续时间
    float idleTimer;         // 静止计时
    float attackCooldown;    // 攻击冷却
    float lastAttackTime;    // 上次攻击时间
    
    // === 渲染 ===
    sf::Sprite sprite;
    sf::Texture texture;
    bool textureLoaded;
    
    // === 动画 ===
    RabbitAnimState animState;
    RabbitAnimState lastAnimState;  // 用于检测状态变化
    int currentFrame;
    float animTimer;
    float frameTime;
    bool animationLocked;    // 动画锁定（防止闪烁）
    float animLockTimer;     // 动画锁定时间
    
    // 新的精灵表参数（128x256，每帧32x32，每行4帧，共8行）
    static constexpr int FRAME_WIDTH = 32;
    static constexpr int FRAME_HEIGHT = 32;
    static constexpr int FRAMES_PER_ROW = 4;
    
    // 行定义（新精灵表布局）
    static constexpr int ROW_MOVE_DOWN = 0;
    static constexpr int ROW_MOVE_UP = 1;
    static constexpr int ROW_MOVE_LEFT = 2;
    static constexpr int ROW_MOVE_RIGHT = 3;
    static constexpr int ROW_ATTACK_DOWN = 4;
    static constexpr int ROW_ATTACK_UP = 5;
    static constexpr int ROW_ATTACK_LEFT = 6;
    static constexpr int ROW_ATTACK_RIGHT = 7;
    
    // === 交互状态 ===
    bool isHovered;
    
    // === 回调 ===
    RabbitCallback onDeath;
    RabbitCallback onAttack;
    
    // === 随机数生成 ===
    mutable std::mt19937 rng;
    
    // === 常量 ===
    static constexpr float MOVE_SPEED = 40.0f;      // 普通移动速度
    static constexpr float CHASE_SPEED = 70.0f;     // 追击速度
    static constexpr float RETURN_SPEED = 50.0f;    // 返回速度
    static constexpr float AGGRO_DURATION = 8.0f;   // 激怒持续时间
    static constexpr float ATTACK_RANGE = 35.0f;    // 攻击范围
    static constexpr float ATTACK_COOLDOWN = 1.2f;  // 攻击冷却
    static constexpr float CHASE_RANGE = 200.0f;    // 追击范围（超出则返回）
    static constexpr float LEASH_RANGE = 300.0f;    // 牵引范围（超出则放弃追击）
    static constexpr float MIN_ANIM_DURATION = 0.2f; // 最小动画持续时间（防止闪烁）
};

// ============================================================================
// 兔子管理器
// ============================================================================

class RabbitManager {
public:
    RabbitManager();
    
    // 初始化
    bool init(const std::string& texturePath);
    
    // 更新所有兔子
    void update(float dt, const sf::Vector2f& playerPos);
    
    // 渲染所有兔子（需要传入视图用于裁剪）
    void render(sf::RenderWindow& window, const sf::View& view);
    
    // 渲染悬浮提示
    void renderTooltips(sf::RenderWindow& window, const sf::Vector2f& mouseWorldPos);
    
    // ========================================
    // 兔子管理
    // ========================================
    Rabbit* addRabbit(float x, float y);
    void removeRabbit(Rabbit* rabbit);
    void clearAllRabbits();
    
    // 在地图上随机生成兔子
    void spawnRandomRabbits(int count, const sf::Vector2i& mapSize, int tileSize);
    
    // ========================================
    // 交互
    // ========================================
    
    // 获取指定位置的兔子
    Rabbit* getRabbitAt(const sf::Vector2f& position);
    
    // 获取与矩形碰撞的兔子
    Rabbit* getRabbitInRect(const sf::FloatRect& rect);
    
    // 对范围内的兔子造成伤害
    std::vector<Rabbit*> damageRabbitsInRange(const sf::Vector2f& center,
                                              float radius, float damage,
                                              bool ignoreDefense = false);
    
    // 获取范围内正在攻击的兔子（用于检测玩家是否被攻击）
    std::vector<Rabbit*> getAttackingRabbitsInRange(const sf::Vector2f& center, float radius);
    
    // 更新悬浮状态
    void updateHover(const sf::Vector2f& mouseWorldPos);
    
    // ========================================
    // 碰撞检测
    // ========================================
    bool isCollidingWithAnyRabbit(const sf::FloatRect& rect) const;
    
    // ========================================
    // 获取器
    // ========================================
    size_t getRabbitCount() const { return rabbits.size(); }
    const std::vector<std::unique_ptr<Rabbit>>& getRabbits() const { return rabbits; }
    
    // ========================================
    // 字体设置
    // ========================================
    bool loadFont(const std::string& fontPath);

private:
    void renderTooltip(sf::RenderWindow& window, Rabbit* rabbit);
    
private:
    std::vector<std::unique_ptr<Rabbit>> rabbits;
    std::string texturePath;
    
    // 字体
    sf::Font font;
    bool fontLoaded;
    
    // 当前悬浮的兔子
    Rabbit* hoveredRabbit;
    
    // 随机数生成器
    std::mt19937 rng;
};
