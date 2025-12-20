#pragma once
#include "Monster.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>
#include <random>
#include <memory>

// ============================================================================
// 兔子怪物系统 (继承自Monster基类)
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

// 兔子掉落物品（为了向后兼容保留）
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

// 兔子技能（为了向后兼容保留）
struct RabbitSkill {
    std::string id;
    std::string name;
    std::string description;
    float damageMultiplier;  // 伤害倍率
    float triggerChance;     // 触发概率 0-1
    sf::Color iconColor;     // 技能图标颜色
    
    RabbitSkill(const std::string& skillId = "", const std::string& n = "",
                const std::string& desc = "", float mult = 1.0f, 
                float chance = 0.0f, sf::Color color = sf::Color::White)
        : id(skillId), name(n), description(desc), 
          damageMultiplier(mult), triggerChance(chance), iconColor(color) {}
};

// 为了向后兼容保留这些枚举别名
using RabbitAIState = MonsterAIState;
using RabbitDirection = MonsterDirection;

// ============================================================================
// 兔子类（继承自Monster）
// ============================================================================
class Rabbit : public Monster {
public:
    Rabbit();
    Rabbit(float x, float y);
    
    // ========================================
    // Monster接口实现
    // ========================================
    void update(float dt, const sf::Vector2f& playerPos) override;
    void render(sf::RenderWindow& window) override;
    std::string getName() const override { return "兔子"; }
    std::string getTypeName() const override { return "兔子"; }
    
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
    // 兔子特有功能（向后兼容）
    // ========================================
    
    // 获取兔子技能信息
    const RabbitSkill& getRabbitSkill() const { return rabbitSkill; }
    
    // 获取兔子掉落
    const std::vector<RabbitDrop>& getRabbitDrops() const { return rabbitDrops; }
    
    // 回调（使用父类类型但保留别名）
    using RabbitCallback = std::function<void(Rabbit&)>;
    void setOnDeathRabbit(RabbitCallback cb) { onDeathRabbit = cb; }
    void setOnAttackRabbit(RabbitCallback cb) { onAttackRabbit = cb; }

private:
    void updateAnimation(float dt);
    void updateAI(float dt, const sf::Vector2f& playerPos);
    void setAnimState(RabbitAnimState state);
    void updateSprite() override;
    sf::IntRect getFrameRect(int row, int col) const;
    void updateDirectionFromVelocityRabbit();
    
private:
    // === 兔子特有掉落 ===
    std::vector<RabbitDrop> rabbitDrops;
    
    // === 兔子特有技能 ===
    RabbitSkill rabbitSkill;
    
    // === 游荡计时 ===
    float wanderTimer;
    float wanderDuration;
    float idleTimer;
    
    // === 动画 ===
    RabbitAnimState animState;
    RabbitAnimState lastAnimState;
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
    
    // === 兔子特有回调 ===
    RabbitCallback onDeathRabbit;
    RabbitCallback onAttackRabbit;
    
    // === 常量（覆盖父类默认值）===
    static constexpr float RABBIT_MOVE_SPEED = 40.0f;
    static constexpr float RABBIT_CHASE_SPEED = 70.0f;
    static constexpr float RABBIT_RETURN_SPEED = 50.0f;
    static constexpr float RABBIT_AGGRO_DURATION = 8.0f;
    static constexpr float RABBIT_ATTACK_RANGE = 35.0f;
    static constexpr float RABBIT_ATTACK_COOLDOWN = 1.2f;
    static constexpr float RABBIT_CHASE_RANGE = 200.0f;
    static constexpr float RABBIT_LEASH_RANGE = 300.0f;
    static constexpr float MIN_ANIM_DURATION = 0.2f;
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
    
    // 碰撞推挤：当玩家主动移动碰到兔子时，推开兔子
    std::vector<Rabbit*> pushRabbitsFromRect(const sf::FloatRect& moverBox, float pushStrength = 1.0f);
    
    // 获取与矩形碰撞的所有兔子
    std::vector<Rabbit*> getRabbitsCollidingWith(const sf::FloatRect& rect) const;
    
    // 获取正在移动的兔子中碰撞到指定矩形的（用于判断兔子是否主动撞玩家）
    std::vector<Rabbit*> getMovingRabbitsCollidingWith(const sf::FloatRect& rect) const;
    
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
