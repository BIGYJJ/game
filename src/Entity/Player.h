#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include "PlayerStats.h"

// 动画状态枚举
enum class AnimState {
    Idle,
    Walk,
    Attack,
    Hurt,    // 受伤状态
    Pickup   // 拾取状态
};

// 朝向枚举
enum class Direction {
    Left,
    Right
};

class Player {
public:
    Player(float x, float y) 
        : currentState(AnimState::Idle)
        , facing(Direction::Right)
        , currentFrame(0)
        , animTimer(0.0f)
        , frameTime(0.12f)
        , hurtTimer(0.0f)
        , hurtDuration(0.4f)
        , isHurt(false)
        , knockbackVelocity(0.0f, 0.0f)
        , knockbackTimer(0.0f)
        , knockbackDuration(0.15f)
        , isKnockedBack(false)
        , isPickingUp(false)
        , pickupTimer(0.0f)
        , pickupDuration(0.5f)
    {
        // 加载精灵表
        if (!texture.loadFromFile("../../assets/player.png")) {
            std::cerr << "error：无法加载 player.png" << std::endl;
            texture.create(64, 64);
        }

        sprite.setTexture(texture);
        sprite.setPosition(x, y);

        // 初始化动画帧
        initAnimationFrames();
        sprite.setTextureRect(idleFrames[0]);
        sprite.setScale(0.5f, 0.5f);
        updateOrigin();
    }
    
    void update(float dt) {
        // 更新属性系统
        stats.update(dt);
        
        // 如果已死亡，不能移动
        if (stats.isDead()) {
            setState(AnimState::Idle);
            return;
        }
        
        // 更新受伤计时器（在这里更新，不阻止移动）
        if (isHurt) {
            hurtTimer += dt;
            if (hurtTimer >= hurtDuration) {
                isHurt = false;
            }
        }
        
        // 更新拾取计时器
        if (isPickingUp) {
            pickupTimer += dt;
            if (pickupTimer >= pickupDuration) {
                isPickingUp = false;
            }
        }
        
        // 更新击退效果
        if (isKnockedBack) {
            knockbackTimer += dt;
            if (knockbackTimer >= knockbackDuration) {
                isKnockedBack = false;
                knockbackVelocity = sf::Vector2f(0.0f, 0.0f);
            } else {
                // 应用击退移动（带衰减）
                float progress = knockbackTimer / knockbackDuration;
                float decay = 1.0f - progress;  // 线性衰减
                sprite.move(knockbackVelocity * decay * dt);
            }
        }
        
        sf::Vector2f movement(0.f, 0.f);
        bool isMoving = false;

        // 获取当前速度（受属性影响）
        float currentSpeed = stats.getSpeed();
        
        // 体力不足时速度减半
        if (stats.isExhausted()) {
            currentSpeed *= 0.5f;
        }
        
        // 击退时减速移动
        if (isKnockedBack) {
            currentSpeed *= 0.3f;
        }
        
        // 拾取时不能移动
        if (!isPickingUp) {
            // 键盘控制
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
                movement.y -= currentSpeed * dt;
                isMoving = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
                movement.y += currentSpeed * dt;
                isMoving = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                movement.x -= currentSpeed * dt;
                isMoving = true;
                facing = Direction::Left;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                movement.x += currentSpeed * dt;
                isMoving = true;
                facing = Direction::Right;
            }

            // 更新位置
            sprite.move(movement);
        }

        // 状态切换（受伤动画优先级低于攻击）
        if (currentState != AnimState::Attack && currentState != AnimState::Pickup) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                // 攻击消耗体力
                if (stats.hasStamina(5.0f)) {
                    stats.consumeStamina(5.0f);
                    setState(AnimState::Attack);
                    isHurt = false;  // 攻击时取消受伤状态
                }
            } else if (isHurt && currentState != AnimState::Hurt) {
                // 被攻击但还没播放受伤动画
                setState(AnimState::Hurt);
            } else if (!isHurt && !isPickingUp) {
                // 正常移动/站立
                if (isMoving) {
                    setState(AnimState::Walk);
                } else {
                    setState(AnimState::Idle);
                }
            } else if (isMoving) {
                // 受伤时仍在移动，播放行走动画
                setState(AnimState::Walk);
            }
        }

        // 更新动画
        updateAnimation(dt);
        updateFacing();
    }
    
    void render(sf::RenderWindow& window) {
        window.draw(sprite);
    }
    
    // ========================================
    // 拾取动画相关
    // ========================================
    
    // 开始拾取动画
    void startPickup() {
        if (!isPickingUp && currentState != AnimState::Attack) {
            isPickingUp = true;
            pickupTimer = 0.0f;
            setState(AnimState::Pickup);
        }
    }
    
    // 是否正在拾取
    bool isInPickupAnimation() const {
        return isPickingUp || currentState == AnimState::Pickup;
    }
    
    // 拾取动画是否刚刚开始（用于触发拾取逻辑）
    bool isPickupJustStarted() const {
        return isPickingUp && pickupTimer < 0.1f;
    }
    
    // ========================================
    // 位置相关
    // ========================================
    sf::Vector2f getPosition() const { 
        return sprite.getPosition(); 
    }
    
    void setPosition(float x, float y) {
        sprite.setPosition(x, y);
    }
    
    void setPosition(const sf::Vector2f& pos) {
        sprite.setPosition(pos);
    }
    
    // 判断玩家是否正在主动移动（用于碰撞响应）
    bool isMoving() const {
        return sf::Keyboard::isKeyPressed(sf::Keyboard::W) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::S) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
    }
    
    // 被推挤时调用（移动位置）
    void applyPush(const sf::Vector2f& pushVector) {
        sprite.move(pushVector);
    }
    
    // ========================================
    // 碰撞检测
    // ========================================
    sf::FloatRect getBounds() const { 
        return sprite.getGlobalBounds(); 
    }
    
    sf::Vector2f getFootPosition() const {
        sf::FloatRect bounds = sprite.getGlobalBounds();
        return sf::Vector2f(
            bounds.left + bounds.width / 2.0f,
            bounds.top + bounds.height
        );
    }
    
    sf::FloatRect getCollisionBox() const {
        sf::FloatRect bounds = sprite.getGlobalBounds();
        float shrink = 0.35f;
        return sf::FloatRect(
            bounds.left + bounds.width * shrink / 2,
            bounds.top + bounds.height * shrink,
            bounds.width * (1 - shrink),
            bounds.height * (1 - shrink)
        );
    }
    
    // ========================================
    // 属性系统访问
    // ========================================
    PlayerStats& getStats() { return stats; }
    const PlayerStats& getStats() const { return stats; }
    
    // 便捷属性访问
    float getHealth() const { return stats.getHealth(); }
    float getMaxHealth() const { return stats.getMaxHealth(); }
    float getStamina() const { return stats.getStamina(); }
    float getMaxStamina() const { return stats.getMaxStamina(); }
    float getHunger() const { return stats.getHunger(); }
    float getMaxHunger() const { return stats.getMaxHunger(); }
    int getLevel() const { return stats.getLevel(); }
    int getGold() const { return stats.getGold(); }
    bool isDead() const { return stats.isDead(); }
    
    // ========================================
    // 战斗相关
    // ========================================
    
    // 执行攻击（返回伤害值）
    float performAttack(float baseDamage = 10.0f) {
        bool crit = stats.rollCritical();
        float damage = stats.calculateDamage(baseDamage);
        
        if (crit) {
            damage *= stats.getCriticalMultiplier();
            std::cout << "暴击! ";
        }
        
        return damage;
    }
    
    // 获取攻击范围（用于检测是否砍到树木等）
    sf::FloatRect getAttackRange() const {
        sf::FloatRect bounds = sprite.getGlobalBounds();
        float attackWidth = 60.0f;
        float attackHeight = 50.0f;
        float attackX;
        
        // 根据朝向确定攻击范围
        if (facing == Direction::Right) {
            attackX = bounds.left + bounds.width;
        } else {
            attackX = bounds.left - attackWidth;
        }
        
        return sf::FloatRect(
            attackX,
            bounds.top + bounds.height * 0.3f,
            attackWidth,
            attackHeight
        );
    }
    
    // 获取攻击中心点和半径（用于圆形范围检测）
    sf::Vector2f getAttackCenter() const {
        sf::FloatRect bounds = sprite.getGlobalBounds();
        float offsetX = (facing == Direction::Right) ? 40.0f : -40.0f;
        return sf::Vector2f(
            bounds.left + bounds.width / 2.0f + offsetX,
            bounds.top + bounds.height * 0.5f
        );
    }
    
    float getAttackRadius() const {
        return 50.0f;  // 攻击半径
    }
    
    // 是否正在攻击
    bool isAttacking() const {
        return currentState == AnimState::Attack;
    }
    
    // 获取朝向
    Direction getFacing() const { return facing; }
    
    // 受到伤害（带击退效果）
    void receiveDamage(float damage, const sf::Vector2f& attackerPos, float attackerDodgeReduction = 0.0f) {
        // 闪避判定
        if (stats.rollDodge(attackerDodgeReduction)) {
            std::cout << "闪避成功!" << std::endl;
            return;
        }
        
        stats.takeDamage(damage);
        
        // 触发受伤动画
        isHurt = true;
        hurtTimer = 0;
        setState(AnimState::Hurt);
        
        // 计算击退方向（从攻击者指向玩家）
        sf::Vector2f playerPos = getPosition();
        sf::Vector2f knockbackDir = playerPos - attackerPos;
        
        // 归一化方向
        float length = std::sqrt(knockbackDir.x * knockbackDir.x + knockbackDir.y * knockbackDir.y);
        if (length > 0.0f) {
            knockbackDir /= length;
        } else {
            // 如果位置重叠，随机方向
            knockbackDir = sf::Vector2f(1.0f, 0.0f);
        }
        
        // 设置击退速度（可调整击退力度）
        float knockbackForce = 400.0f;  // 击退力度
        knockbackVelocity = knockbackDir * knockbackForce;
        knockbackTimer = 0.0f;
        isKnockedBack = true;
        
        if (stats.isDead()) {
            std::cout << "玩家死亡!" << std::endl;
        }
    }
    
    // 受到伤害（无击退，向后兼容）
    void receiveDamage(float damage, float attackerDodgeReduction = 0.0f) {
        // 闪避判定
        if (stats.rollDodge(attackerDodgeReduction)) {
            std::cout << "闪避成功!" << std::endl;
            return;
        }
        
        stats.takeDamage(damage);
        
        // 触发受伤动画
        isHurt = true;
        hurtTimer = 0;
        setState(AnimState::Hurt);
        
        if (stats.isDead()) {
            std::cout << "玩家死亡!" << std::endl;
        }
    }
    
    // 重生
    void respawn(float x, float y) {
        stats.respawn();
        setPosition(x, y);
        setState(AnimState::Idle);
    }

private:
    void initAnimationFrames() {
        // 站立动画 (4帧)
        idleFrames = {
            sf::IntRect(58,  55, 105, 117),
            sf::IntRect(195, 55, 105, 117),
            sf::IntRect(327, 55, 105, 117),
            sf::IntRect(455, 55, 114, 117)
        };

        // 行走动画 (7帧)
        walkFrames = {
            sf::IntRect(55,  216, 112, 118),
            sf::IntRect(196, 216, 112, 118),
            sf::IntRect(336, 216, 108, 118),
            sf::IntRect(468, 216, 83,  118),
            sf::IntRect(596, 216, 92,  118),
            sf::IntRect(724, 216, 113, 118),
            sf::IntRect(866, 216, 113, 118)
        };

        // 攻击动画 (5帧)
        attackFrames = {
            sf::IntRect(54,  371, 114, 165),
            sf::IntRect(191, 371, 112, 165),
            sf::IntRect(327, 371, 97,  165),
            sf::IntRect(464, 371, 96,  165),
            sf::IntRect(600, 371, 130, 165)
        };
        
        // 受伤动画 (4帧) - 第四行，带红色闪烁和后仰效果
        hurtFrames = {
            sf::IntRect(68,  559, 105, 117),
            sf::IntRect(201, 559, 105, 117),
            sf::IntRect(330, 559, 105, 117),
            sf::IntRect(455, 559, 114, 117)
        };
        
        // 拾取动画 (5帧) - 第五行
        pickupFrames = {
            sf::IntRect(50,  759, 120, 150),  // Frame 1: 站立准备
            sf::IntRect(180, 759, 120, 150),  // Frame 2: 开始弯腰
            sf::IntRect(310, 759, 120, 150),  // Frame 3: 蹲下拾取
            sf::IntRect(440, 759, 120, 150),  // Frame 4: 起身
            sf::IntRect(570, 759, 120, 150)   // Frame 5: 恢复站立
        };
    }

    void updateAnimation(float dt) {
        animTimer += dt;

        if (animTimer >= frameTime) {
            animTimer = 0.0f;
            currentFrame++;

            const std::vector<sf::IntRect>& frames = getCurrentFrames();
            
            if (currentFrame >= frames.size()) {
                if (currentState == AnimState::Attack) {
                    setState(AnimState::Idle);
                    return;
                }
                if (currentState == AnimState::Pickup) {
                    // 拾取动画播放完后恢复站立
                    isPickingUp = false;
                    setState(AnimState::Idle);
                    return;
                }
                if (currentState == AnimState::Hurt) {
                    // 受伤动画播放完一遍后，根据是否移动切换状态
                    currentFrame = 0;
                }
                currentFrame = 0;
            }

            sprite.setTextureRect(frames[currentFrame]);
            updateOrigin();
        }
    }

    const std::vector<sf::IntRect>& getCurrentFrames() const {
        switch (currentState) {
            case AnimState::Walk:   return walkFrames;
            case AnimState::Attack: return attackFrames;
            case AnimState::Hurt:   return hurtFrames;
            case AnimState::Pickup: return pickupFrames;
            case AnimState::Idle:
            default:                return idleFrames;
        }
    }

    void setState(AnimState newState) {
        if (currentState != newState) {
            currentState = newState;
            currentFrame = 0;
            animTimer = 0.0f;
            sprite.setTextureRect(getCurrentFrames()[0]);
            updateOrigin();
        }
    }

    void updateFacing() {
        sf::IntRect rect = sprite.getTextureRect();
        
        if (facing == Direction::Left) {
            if (rect.width > 0) {
                rect.left += rect.width;
                rect.width = -rect.width;
                sprite.setTextureRect(rect);
            }
        } else {
            if (rect.width < 0) {
                rect.width = -rect.width;
                rect.left -= rect.width;
                sprite.setTextureRect(rect);
            }
        }
    }

    void updateOrigin() {
        sf::IntRect rect = sprite.getTextureRect();
        int w = rect.width > 0 ? rect.width : -rect.width;
        int h = rect.height > 0 ? rect.height : -rect.height;
        sprite.setOrigin(w / 2.0f, static_cast<float>(h));
    }

private:
    // 渲染相关
    sf::Sprite sprite;
    sf::Texture texture;

    // 动画相关
    AnimState currentState;
    Direction facing;
    size_t currentFrame;
    float animTimer;
    float frameTime;

    std::vector<sf::IntRect> idleFrames;
    std::vector<sf::IntRect> walkFrames;
    std::vector<sf::IntRect> attackFrames;
    std::vector<sf::IntRect> hurtFrames;   // 受伤动画帧
    std::vector<sf::IntRect> pickupFrames; // 拾取动画帧
    
    // 受伤状态
    float hurtTimer;           // 受伤动画计时
    float hurtDuration;        // 受伤动画持续时间
    bool isHurt;               // 是否正在受伤
    
    // 击退状态
    sf::Vector2f knockbackVelocity;  // 击退速度
    float knockbackTimer;            // 击退计时器
    float knockbackDuration;         // 击退持续时间
    bool isKnockedBack;              // 是否正在被击退
    
    // 拾取状态
    bool isPickingUp;          // 是否正在拾取
    float pickupTimer;         // 拾取计时器
    float pickupDuration;      // 拾取动画持续时间
    
    // 属性系统
    PlayerStats stats;
};
