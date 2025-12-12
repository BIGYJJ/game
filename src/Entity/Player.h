#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>

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
        : speed(200.0f)
        , currentState(AnimState::Idle)
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
        sf::Vector2f movement(0.f, 0.f);
        bool isMoving = false;

        // 键盘控制
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            movement.y -= speed * dt;
            isMoving = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            movement.y += speed * dt;
            isMoving = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            movement.x -= speed * dt;
            isMoving = true;
            facing = Direction::Left;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            movement.x += speed * dt;
            isMoving = true;
            facing = Direction::Right;
        }

        // 更新位置
        sprite.move(movement);

        // 状态切换
        if (currentState != AnimState::Attack) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                setState(AnimState::Attack);
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
    
    // 获取位置
    sf::Vector2f getPosition() const { 
        return sprite.getPosition(); 
    }
    
    // 设置位置（用于碰撞修正）
    void setPosition(float x, float y) {
        sprite.setPosition(x, y);
    }
    
    void setPosition(const sf::Vector2f& pos) {
        sprite.setPosition(pos);
    }
    
    // 获取边界（用于碰撞检测）
    sf::FloatRect getBounds() const { 
        return sprite.getGlobalBounds(); 
    }
    
    // 获取底部中心点（更精确的碰撞点）
    sf::Vector2f getFootPosition() const {
        sf::FloatRect bounds = sprite.getGlobalBounds();
        return sf::Vector2f(
            bounds.left + bounds.width / 2.0f,
            bounds.top + bounds.height
        );
    }
    
    // 获取碰撞盒（可以比视觉边界小一些）
    sf::FloatRect getCollisionBox() const {
        sf::FloatRect bounds = sprite.getGlobalBounds();
        // 碰撞盒比实际显示小30%，更合理
        float shrink = 0.35f;
        return sf::FloatRect(
            bounds.left + bounds.width * shrink / 2,
            bounds.top + bounds.height * shrink,
            bounds.width * (1 - shrink),
            bounds.height * (1 - shrink)
        );
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
    sf::Sprite sprite;
    sf::Texture texture;
    float speed;

    AnimState currentState;
    Direction facing;
    size_t currentFrame;
    float animTimer;
    float frameTime;

    std::vector<sf::IntRect> idleFrames;
    std::vector<sf::IntRect> walkFrames;
    std::vector<sf::IntRect> attackFrames;
};