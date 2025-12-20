#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>
#include <random>
#include <memory>
#include <cmath>

// ============================================================================
// 怪物基类 (Monster Base Class)
// 
// 所有怪物的共有属性和行为，子类继承并扩展特定功能
// ============================================================================

// 怪物掉落物品结构
struct MonsterDrop {
    std::string itemId;
    std::string name;
    int minCount;
    int maxCount;
    float dropChance;  // 0-1
    
    MonsterDrop(const std::string& id = "", const std::string& n = "",
                int min = 1, int max = 1, float chance = 1.0f)
        : itemId(id), name(n), minCount(min), maxCount(max), dropChance(chance) {}
};

// 怪物技能结构
struct MonsterSkill {
    std::string id;
    std::string name;
    std::string description;
    float damageMultiplier;  // 伤害倍率
    float triggerChance;     // 触发概率 0-1
    sf::Color iconColor;     // 技能图标颜色
    
    MonsterSkill(const std::string& skillId = "", const std::string& n = "",
                 const std::string& desc = "", float mult = 1.0f, 
                 float chance = 0.0f, sf::Color color = sf::Color::White)
        : id(skillId), name(n), description(desc), 
          damageMultiplier(mult), triggerChance(chance), iconColor(color) {}
};

// 怪物AI状态基础枚举
enum class MonsterAIState {
    Idle,           // 站立
    Wandering,      // 随机游荡
    Chasing,        // 追击玩家
    Attacking,      // 近身攻击
    Returning,      // 返回原位
    Fleeing         // 逃跑（胆小怪物）
};

// 怪物朝向
enum class MonsterDirection {
    Down,
    Up,
    Left,
    Right
};

// ============================================================================
// 怪物基类
// ============================================================================
class Monster {
public:
    using MonsterCallback = std::function<void(Monster&)>;

    Monster();
    virtual ~Monster() = default;
    
    // ========================================
    // 核心接口（子类必须实现）
    // ========================================
    virtual void update(float dt, const sf::Vector2f& playerPos) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
    virtual std::string getName() const = 0;
    virtual std::string getTypeName() const = 0;  // 怪物类型名称（如"兔子"、"史莱姆"）
    
    // ========================================
    // 初始化
    // ========================================
    virtual void init(float x, float y);
    virtual bool loadTexture(const std::string& texturePath);
    virtual void randomizeStats();  // 随机化属性（在给定范围内）
    
    // ========================================
    // 战斗系统
    // ========================================
    
    // 受到伤害，返回是否死亡
    virtual bool takeDamage(float damage, bool ignoreDefense = false);
    
    // 执行攻击（返回伤害值）
    virtual float performAttack();
    
    // 检查是否触发技能
    bool rollSkill() const;
    
    // 获取当前技能伤害倍率
    float getSkillMultiplier() const;
    
    // 是否最近触发了技能
    bool hasTriggeredSkill() const { return lastAttackUsedSkill; }
    
    // 获取技能信息
    const MonsterSkill& getSkill() const { return skill; }
    
    // 闪避判定
    bool rollDodge() const;
    
    // 激怒怪物（被攻击后触发）
    virtual void aggro(float duration);
    
    // ========================================
    // 碰撞检测
    // ========================================
    bool containsPoint(const sf::Vector2f& point) const;
    bool intersects(const sf::FloatRect& rect) const;
    sf::FloatRect getBounds() const;
    virtual sf::FloatRect getCollisionBox() const;
    
    // ========================================
    // 属性 Getters
    // ========================================
    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    float getHealthPercent() const { return maxHealth > 0 ? health / maxHealth : 0; }
    float getDefense() const { return defense; }
    float getAttack() const { return attack; }
    int getDodge() const { return dodge; }
    int getLevel() const { return level; }
    bool isDead() const { return health <= 0; }
    bool isAttacking() const { return aiState == MonsterAIState::Attacking; }
    bool isAggressive() const { return isAggroed; }
    MonsterAIState getAIState() const { return aiState; }
    MonsterDirection getDirection() const { return direction; }
    
    // ========================================
    // 位置和移动
    // ========================================
    sf::Vector2f getPosition() const { return position; }
    sf::Vector2f getVelocity() const { return velocity; }
    sf::Vector2f getSize() const { return size; }
    sf::Vector2f getHomePosition() const { return homePosition; }
    
    void setPosition(float x, float y);
    void setPosition(const sf::Vector2f& pos);
    
    // 判断是否正在主动移动（用于碰撞响应）
    bool isMoving() const { 
        return velocity.x != 0.0f || velocity.y != 0.0f; 
    }
    
    // 被推挤时调用（移动位置）
    virtual void applyPush(const sf::Vector2f& pushVector);
    
    // ========================================
    // 掉落奖励
    // ========================================
    virtual std::vector<std::pair<std::string, int>> generateDrops() const;
    virtual int getExpReward() const;
    virtual int getGoldReward() const;
    const std::vector<MonsterDrop>& getDrops() const { return drops; }
    std::pair<int, int> getExpRange() const { return {expMin, expMax}; }
    std::pair<int, int> getGoldRange() const { return {goldMin, goldMax}; }
    
    // ========================================
    // 悬浮提示
    // ========================================
    void setHovered(bool hovered) { isHovered = hovered; }
    bool getHovered() const { return isHovered; }
    
    // ========================================
    // 回调设置
    // ========================================
    void setOnDeath(MonsterCallback cb) { onDeath = cb; }
    void setOnAttack(MonsterCallback cb) { onAttack = cb; }

protected:
    // 更新方向（根据速度）
    void updateDirectionFromVelocity();
    
    // 更新精灵位置
    virtual void updateSprite();
    
protected:
    // === 基础属性 ===
    float health;
    float maxHealth;
    float defense;
    float attack;
    int dodge;          // 闪避值 0-200, 1点 = 0.5%闪避几率
    int level;
    
    // === 属性范围（用于随机化）===
    float healthMin, healthMax;
    float defenseMin, defenseMax;
    float attackMin, attackMax;
    int dodgeMin, dodgeMax;
    
    // === 掉落奖励范围 ===
    int expMin, expMax;
    int goldMin, goldMax;
    
    // === 掉落物品 ===
    std::vector<MonsterDrop> drops;
    
    // === 技能 ===
    MonsterSkill skill;
    bool lastAttackUsedSkill;
    
    // === 位置和移动 ===
    sf::Vector2f position;
    sf::Vector2f size;
    sf::Vector2f velocity;
    sf::Vector2f homePosition;
    
    // === AI状态 ===
    MonsterAIState aiState;
    MonsterDirection direction;
    bool isAggroed;
    float aggroTimer;
    float attackCooldown;
    float lastAttackTime;
    
    // === 渲染 ===
    sf::Sprite sprite;
    sf::Texture texture;
    bool textureLoaded;
    
    // === 交互状态 ===
    bool isHovered;
    
    // === 回调 ===
    MonsterCallback onDeath;
    MonsterCallback onAttack;
    
    // === 随机数生成 ===
    mutable std::mt19937 rng;
    
    // === AI常量（可在子类中覆盖）===
    float moveSpeed;
    float chaseSpeed;
    float returnSpeed;
    float aggroDuration;
    float attackRange;
    float attackCooldownTime;
    float chaseRange;
    float leashRange;
};

// ============================================================================
// 怪物管理器基类模板
// ============================================================================
template<typename T>
class MonsterManager {
public:
    MonsterManager() : hoveredMonster(nullptr) {
        rng.seed(std::random_device{}());
    }
    
    virtual ~MonsterManager() = default;
    
    // 初始化
    virtual bool init(const std::string& resourcePath) {
        texturePath = resourcePath;
        return true;
    }
    
    // 更新所有怪物
    virtual void update(float dt, const sf::Vector2f& playerPos) {
        for (auto& monster : monsters) {
            monster->update(dt, playerPos);
        }
        
        // 移除死亡的怪物
        monsters.erase(
            std::remove_if(monsters.begin(), monsters.end(),
                [](const std::unique_ptr<T>& m) { return m->isDead(); }),
            monsters.end()
        );
    }
    
    // 渲染所有怪物
    virtual void render(sf::RenderWindow& window, const sf::View& view) {
        sf::FloatRect viewBounds(
            view.getCenter().x - view.getSize().x / 2.0f,
            view.getCenter().y - view.getSize().y / 2.0f,
            view.getSize().x,
            view.getSize().y
        );
        
        for (auto& monster : monsters) {
            sf::FloatRect bounds = monster->getBounds();
            if (viewBounds.intersects(bounds)) {
                monster->render(window);
            }
        }
    }
    
    // 添加怪物
    T* addMonster(float x, float y) {
        auto monster = std::make_unique<T>(x, y);
        monster->loadTexture(texturePath);
        T* ptr = monster.get();
        monsters.push_back(std::move(monster));
        return ptr;
    }
    
    // 移除怪物
    void removeMonster(T* monster) {
        monsters.erase(
            std::remove_if(monsters.begin(), monsters.end(),
                [monster](const std::unique_ptr<T>& m) { return m.get() == monster; }),
            monsters.end()
        );
    }
    
    // 清空所有怪物
    void clearAllMonsters() {
        monsters.clear();
        hoveredMonster = nullptr;
    }
    
    // 获取指定位置的怪物
    T* getMonsterAt(const sf::Vector2f& position) {
        for (auto& monster : monsters) {
            if (monster->containsPoint(position)) {
                return monster.get();
            }
        }
        return nullptr;
    }
    
    // 获取与矩形碰撞的怪物
    T* getMonsterInRect(const sf::FloatRect& rect) {
        for (auto& monster : monsters) {
            if (monster->intersects(rect)) {
                return monster.get();
            }
        }
        return nullptr;
    }
    
    // 检查是否与任何怪物碰撞
    bool isCollidingWithAnyMonster(const sf::FloatRect& rect) const {
        for (const auto& monster : monsters) {
            if (monster->getCollisionBox().intersects(rect)) {
                return true;
            }
        }
        return false;
    }
    
    // 推挤怪物（当玩家主动碰到怪物时）
    std::vector<T*> pushMonstersFromRect(const sf::FloatRect& moverBox, float pushStrength = 1.0f) {
        std::vector<T*> pushedMonsters;
        
        for (auto& monster : monsters) {
            sf::FloatRect monsterBox = monster->getCollisionBox();
            
            if (moverBox.intersects(monsterBox)) {
                sf::Vector2f moverCenter(
                    moverBox.left + moverBox.width / 2.0f,
                    moverBox.top + moverBox.height / 2.0f
                );
                sf::Vector2f monsterCenter(
                    monsterBox.left + monsterBox.width / 2.0f,
                    monsterBox.top + monsterBox.height / 2.0f
                );
                
                sf::Vector2f pushDir = monsterCenter - moverCenter;
                float length = std::sqrt(pushDir.x * pushDir.x + pushDir.y * pushDir.y);
                
                if (length < 0.001f) {
                    pushDir = sf::Vector2f(1.0f, 0.0f);
                    length = 1.0f;
                }
                
                pushDir /= length;
                
                float overlapX = (moverBox.width + monsterBox.width) / 2.0f - 
                                std::abs(moverCenter.x - monsterCenter.x);
                float overlapY = (moverBox.height + monsterBox.height) / 2.0f - 
                                std::abs(moverCenter.y - monsterCenter.y);
                
                float pushDistance = std::min(overlapX, overlapY) + 2.0f;
                
                sf::Vector2f pushVector = pushDir * pushDistance * pushStrength;
                monster->applyPush(pushVector);
                
                pushedMonsters.push_back(monster.get());
            }
        }
        
        return pushedMonsters;
    }
    
    // 获取与矩形碰撞的所有怪物
    std::vector<T*> getMonstersCollidingWith(const sf::FloatRect& rect) const {
        std::vector<T*> result;
        for (const auto& monster : monsters) {
            if (monster->getCollisionBox().intersects(rect)) {
                result.push_back(monster.get());
            }
        }
        return result;
    }
    
    // 获取正在移动的怪物中碰撞到指定矩形的
    std::vector<T*> getMovingMonstersCollidingWith(const sf::FloatRect& rect) const {
        std::vector<T*> result;
        for (const auto& monster : monsters) {
            if (monster->isMoving() && monster->getCollisionBox().intersects(rect)) {
                result.push_back(monster.get());
            }
        }
        return result;
    }
    
    // 对范围内的怪物造成伤害
    std::vector<T*> damageMonstersInRange(const sf::Vector2f& center,
                                           float radius, float damage,
                                           bool ignoreDefense = false) {
        std::vector<T*> hitMonsters;
        
        for (auto& monster : monsters) {
            sf::Vector2f monsterCenter = monster->getPosition() + 
                                         sf::Vector2f(monster->getBounds().width / 2,
                                                      monster->getBounds().height / 2);
            
            float dist = std::sqrt(
                std::pow(center.x - monsterCenter.x, 2) +
                std::pow(center.y - monsterCenter.y, 2)
            );
            
            if (dist <= radius) {
                monster->takeDamage(damage, ignoreDefense);
                hitMonsters.push_back(monster.get());
            }
        }
        
        return hitMonsters;
    }
    
    // 获取范围内正在攻击的怪物
    std::vector<T*> getAttackingMonstersInRange(const sf::Vector2f& center, float radius) {
        std::vector<T*> attackingMonsters;
        
        for (auto& monster : monsters) {
            if (!monster->isAttacking()) continue;
            
            sf::Vector2f monsterCenter = monster->getPosition() + 
                                         sf::Vector2f(monster->getBounds().width / 2,
                                                      monster->getBounds().height / 2);
            
            float dist = std::sqrt(
                std::pow(center.x - monsterCenter.x, 2) +
                std::pow(center.y - monsterCenter.y, 2)
            );
            
            if (dist <= radius) {
                attackingMonsters.push_back(monster.get());
            }
        }
        
        return attackingMonsters;
    }
    
    // 更新悬浮状态
    void updateHover(const sf::Vector2f& mouseWorldPos) {
        if (hoveredMonster) {
            hoveredMonster->setHovered(false);
            hoveredMonster = nullptr;
        }
        
        for (auto& monster : monsters) {
            if (monster->containsPoint(mouseWorldPos)) {
                hoveredMonster = monster.get();
                hoveredMonster->setHovered(true);
                break;
            }
        }
    }
    
    // 获取器
    size_t getMonsterCount() const { return monsters.size(); }
    const std::vector<std::unique_ptr<T>>& getMonsters() const { return monsters; }
    
protected:
    std::vector<std::unique_ptr<T>> monsters;
    std::string texturePath;
    T* hoveredMonster;
    std::mt19937 rng;
};
