#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include "PlayerStats.h"

// 动画状态枚举
enum class AnimState {
    Idle,
    Walk,
    Attack
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
        , currentFrame(0)
        , animTimer(0.0f)
        , frameTime(0.12f)
        , facing(Direction::Right)
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
        
        // 如果已死亡或体力耗尽，不能移动
        if (stats.isDead()) {
            setState(AnimState::Idle);
            return;
        }
        
        sf::Vector2f movement(0.f, 0.f);
        bool isMoving = false;

        // 获取当前速度（受属性影响）
        float currentSpeed = stats.getSpeed();
        
        // 体力不足时速度减半
        if (stats.isExhausted()) {
            currentSpeed *= 0.5f;
        }

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

        // 状态切换
        if (currentState != AnimState::Attack) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                // 攻击消耗体力
                if (stats.hasStamina(5.0f)) {
                    stats.consumeStamina(5.0f);
                    setState(AnimState::Attack);
                }
            } else if (isMoving) {
                setState(AnimState::Walk);
            } else {
                setState(AnimState::Idle);
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
    
    // 受到伤害
    void receiveDamage(float damage, float attackerDodgeReduction = 0.0f) {
        // 闪避判定
        if (stats.rollDodge(attackerDodgeReduction)) {
            std::cout << "闪避成功!" << std::endl;
            return;
        }
        
        stats.takeDamage(damage);
        
        if (stats.isDead()) {
            std::cout << "玩家死亡!" << std::endl;
            // 可以在这里触发死亡动画
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
    
    // 属性系统
    PlayerStats stats;
};