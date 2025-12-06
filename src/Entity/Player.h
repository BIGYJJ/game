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
        , frameTime(0.12f)  // 每帧持续时间（秒）
        , facing(Direction::Right)
    {
        // 1. 加载精灵表
        if (!texture.loadFromFile("../../assets/player.png")) {
            std::cerr << "错误：无法加载 player.jpg" << std::endl;
            texture.create(64, 64);
        }

        sprite.setTexture(texture);
        sprite.setPosition(x, y);

        // 2. 初始化动画帧数据（基于之前的分析结果）
        initAnimationFrames();

        // 3. 设置初始帧
        sprite.setTextureRect(idleFrames[0]);
        
        // 4. 设置缩放（可选，根据需要调整）
        sprite.setScale(1.0f, 1.0f);
        
        // 5. 设置原点为底部中心，方便定位
        updateOrigin();
    }
    
    void update(float dt) {
        sf::Vector2f movement(0.f, 0.f);
        bool isMoving = false;

        // 键盘控制移动
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            movement.y -= speed * dt;
            isMoving = true;
            std::cout<<"Up!"<<std::endl;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            movement.y += speed * dt;
            isMoving = true;
            std::cout<<"Down!"<<std::endl;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            movement.x -= speed * dt;
            isMoving = true;
            facing = Direction::Left;
            std::cout<<"Left!"<<std::endl;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            movement.x += speed * dt;
            isMoving = true;
            facing = Direction::Right;
            std::cout<<"Right!"<<std::endl;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            std::cout<<"attack!"<<std::endl;
            setState(AnimState::Attack);
        }

        // 更新位置
        sprite.move(movement);

        // 更新动画状态
        if (isMoving) {
            setState(AnimState::Walk);
        } else {
            setState(AnimState::Idle);
        }

        // 更新动画帧
        updateAnimation(dt);
        
        // 更新精灵朝向
        updateFacing();
    }
    
    void render(sf::RenderWindow& window) {
        window.draw(sprite);
    }
    
    sf::Vector2f getPosition() const { return sprite.getPosition(); }
    
    // 获取精灵边界（用于碰撞检测）
    sf::FloatRect getBounds() const { return sprite.getGlobalBounds(); }

private:
    // 初始化所有动画帧
    void initAnimationFrames() {
        // 站立动画帧 (第1行) - 4帧
        idleFrames = {
            sf::IntRect(58,  55, 105, 117),
            sf::IntRect(195, 55, 105, 117),
            sf::IntRect(327, 55, 105, 117),
            sf::IntRect(455, 55, 114, 117)
        };

        // 行走动画帧 (第2行) - 7帧
        walkFrames = {
            sf::IntRect(55,  216, 112, 118),
            sf::IntRect(196, 216, 112, 118),
            sf::IntRect(336, 216, 108, 118),
            sf::IntRect(468, 216, 83,  118),
            sf::IntRect(596, 216, 92,  118),
            sf::IntRect(724, 216, 113, 118),
            sf::IntRect(866, 216, 113, 118)
        };

        // 攻击动画帧 (第3行) - 5帧 (跳过最后一个不完整的)
        attackFrames = {
            sf::IntRect(54,  371, 114, 165),
            sf::IntRect(191, 371, 112, 165),
            sf::IntRect(327, 371, 97,  165),
            sf::IntRect(464, 371, 96,  165),
            sf::IntRect(600, 371, 130, 165)
        };
    }

    // 更新动画
    void updateAnimation(float dt) {
        animTimer += dt;

        if (animTimer >= frameTime) {
            animTimer = 0.0f;
            currentFrame++;

            // 获取当前动画的帧数组
            const std::vector<sf::IntRect>& frames = getCurrentFrames();
            
            // 循环播放
            if (currentFrame >= frames.size()) {
                currentFrame = 0;
            }

            // 更新精灵显示的帧
            sprite.setTextureRect(frames[currentFrame]);
            updateOrigin();
        }
    }

    // 获取当前状态对应的帧数组
    const std::vector<sf::IntRect>& getCurrentFrames() const {
        switch (currentState) {
            case AnimState::Walk:   return walkFrames;
            case AnimState::Attack: return attackFrames;
            case AnimState::Idle:
            default:                return idleFrames;
        }
    }

    // 设置动画状态
    void setState(AnimState newState) {
        if (currentState != newState) {
            currentState = newState;
            currentFrame = 0;
            animTimer = 0.0f;
            
            // 立即更新到新状态的第一帧
            sprite.setTextureRect(getCurrentFrames()[0]);
            updateOrigin();
        }
    }

    // 更新精灵朝向（通过水平翻转）
    void updateFacing() {
        sf::IntRect rect = sprite.getTextureRect();
        
        if (facing == Direction::Left) {
            // 水平翻转：使用负宽度
            if (rect.width > 0) {
                rect.left += rect.width;
                rect.width = -rect.width;
                sprite.setTextureRect(rect);
            }
        } else {
            // 正常朝右
            if (rect.width < 0) {
                rect.width = -rect.width;
                rect.left -= rect.width;
                sprite.setTextureRect(rect);
            }
        }
    }

    // 更新原点（保持角色脚底对齐）
    void updateOrigin() {
        sf::IntRect rect = sprite.getTextureRect();
        int w = rect.width > 0 ? rect.width : -rect.width;
        int h = rect.height > 0 ? rect.height : -rect.height;
        sprite.setOrigin(w / 2.0f, static_cast<float>(h));
    }

    // 触发攻击（可选，按空格键攻击）
    void attack() {
        setState(AnimState::Attack);
    }

private:
    sf::Sprite sprite;
    sf::Texture texture;
    float speed;

    // 动画系统
    AnimState currentState;
    Direction facing;
    size_t currentFrame;
    float animTimer;
    float frameTime;

    // 动画帧数据
    std::vector<sf::IntRect> idleFrames;
    std::vector<sf::IntRect> walkFrames;
    std::vector<sf::IntRect> attackFrames;
};