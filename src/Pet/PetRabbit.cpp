#include "PetRabbit.h"
#include <iostream>
#include <algorithm>

// ============================================================================
// 构造函数
// ============================================================================
PetRabbit::PetRabbit() 
    : Pet()
    , rabbitAnimState(PetRabbitAnimState::IdleDown)
    , currentFrame(0)
    , animTimer(0)
    , frameTime(0.15f)
    , sheddingTimer(0)
{
    name = "宠物兔";
    size = sf::Vector2f(32, 32);
    initQualityConfigs();
}

PetRabbit::PetRabbit(float x, float y) : PetRabbit() {
    init(x, y);
}

// ============================================================================
// 初始化资质配置
// ============================================================================
void PetRabbit::initQualityConfigs() {
    // 平庸资质配置
    {
        HatchConfig config;
        config.health = PetStatRange(10, 20);
        config.attack = PetStatRange(2, 3);
        config.defense = PetStatRange(1, 2);
        config.dodge = PetStatRange(0, 1);
        config.skillChances = {
            {SKILL_BITE, 0.20f},
            {SKILL_SHEDDING, 0.05f}
        };
        setHatchConfig(PetQuality::Mediocre, config);
        
        LevelUpConfig lvlConfig;
        lvlConfig.health = PetStatRange(3, 5);
        lvlConfig.attack = PetStatRange(1, 3);
        lvlConfig.defense = PetStatRange(1, 2);
        lvlConfig.dodge = PetStatRange(0, 1);
        setLevelUpConfig(PetQuality::Mediocre, lvlConfig);
    }
    
    // 良好资质配置
    {
        HatchConfig config;
        config.health = PetStatRange(15, 30);
        config.attack = PetStatRange(3, 5);
        config.defense = PetStatRange(1, 3);
        config.dodge = PetStatRange(0, 1);
        config.skillChances = {
            {SKILL_BITE, 0.30f},
            {SKILL_SHEDDING, 0.08f}
        };
        setHatchConfig(PetQuality::Good, config);
        
        LevelUpConfig lvlConfig;
        lvlConfig.health = PetStatRange(4, 7);
        lvlConfig.attack = PetStatRange(2, 4);
        lvlConfig.defense = PetStatRange(1, 3);
        lvlConfig.dodge = PetStatRange(0, 1);
        setLevelUpConfig(PetQuality::Good, lvlConfig);
    }
    
    // 优秀资质配置
    {
        HatchConfig config;
        config.health = PetStatRange(30, 60);
        config.attack = PetStatRange(5, 8);
        config.defense = PetStatRange(3, 6);
        config.dodge = PetStatRange(1, 3);
        config.skillChances = {
            {SKILL_BITE, 0.50f},
            {SKILL_SHEDDING, 0.20f}
        };
        setHatchConfig(PetQuality::Excellent, config);
        
        LevelUpConfig lvlConfig;
        lvlConfig.health = PetStatRange(8, 13);
        lvlConfig.attack = PetStatRange(4, 8);
        lvlConfig.defense = PetStatRange(2, 4);
        lvlConfig.dodge = PetStatRange(1, 2);
        setLevelUpConfig(PetQuality::Excellent, lvlConfig);
    }
    
    // 卓越资质配置
    {
        HatchConfig config;
        config.health = PetStatRange(50, 80);
        config.attack = PetStatRange(8, 15);
        config.defense = PetStatRange(6, 10);
        config.dodge = PetStatRange(2, 5);
        config.skillChances = {
            {SKILL_BITE, 0.70f},
            {SKILL_SHEDDING, 0.35f}
        };
        setHatchConfig(PetQuality::Outstanding, config);
        
        LevelUpConfig lvlConfig;
        lvlConfig.health = PetStatRange(12, 20);
        lvlConfig.attack = PetStatRange(8, 12);
        lvlConfig.defense = PetStatRange(5, 8);
        lvlConfig.dodge = PetStatRange(2, 5);
        setLevelUpConfig(PetQuality::Outstanding, lvlConfig);
    }
    
    // 稀有资质配置
    {
        HatchConfig config;
        config.health = PetStatRange(100, 150);
        config.attack = PetStatRange(15, 30);
        config.defense = PetStatRange(10, 17);
        config.dodge = PetStatRange(4, 8);
        config.skillChances = {
            {SKILL_BITE, 1.0f},      // 100%
            {SKILL_SHEDDING, 1.0f},  // 100%
            {SKILL_CHEER, 1.0f}      // 100%
        };
        setHatchConfig(PetQuality::Rare, config);
        
        LevelUpConfig lvlConfig;
        lvlConfig.health = PetStatRange(20, 35);
        lvlConfig.attack = PetStatRange(12, 20);
        lvlConfig.defense = PetStatRange(8, 12);
        lvlConfig.dodge = PetStatRange(4, 6);
        setLevelUpConfig(PetQuality::Rare, lvlConfig);
    }
}

// ============================================================================
// 初始化
// ============================================================================
void PetRabbit::init(float x, float y) {
    Pet::init(x, y);
    rabbitAnimState = PetRabbitAnimState::IdleDown;
    currentFrame = 0;
    animTimer = 0;
    sheddingTimer = 0;
}

bool PetRabbit::loadTexture(const std::string& texturePath) {
    if (!Pet::loadTexture(texturePath)) {
        return false;
    }
    
    // 设置初始帧
    sprite.setTextureRect(getFrameRect(ROW_MOVE_DOWN, 0));
    sprite.setScale(1.5f, 1.5f);  // 放大一点使宠物更明显
    
    return true;
}

void PetRabbit::hatch(PetQuality q, int enhancerCount) {
    Pet::hatch(q, enhancerCount);
    sheddingTimer = 0;
}

// ============================================================================
// 技能随机
// ============================================================================
void PetRabbit::rollSkills() {
    skills.clear();
    
    auto it = hatchConfigs.find(quality);
    if (it == hatchConfigs.end()) return;
    
    const HatchConfig& config = it->second;
    
    for (const auto& skillChance : config.skillChances) {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        if (dist(rng) < skillChance.second) {
            if (skillChance.first == SKILL_BITE) {
                skills.push_back(PetSkill(
                    SKILL_BITE, 
                    "撕咬",
                    "30%触发双倍普通攻击伤害",
                    0.30f,      // 触发概率
                    2.0f,       // 伤害倍率
                    0,
                    false       // 主动技能
                ));
                std::cout << "  获得技能: 撕咬" << std::endl;
            }
            else if (skillChance.first == SKILL_SHEDDING) {
                skills.push_back(PetSkill(
                    SKILL_SHEDDING,
                    "脱毛",
                    "每20分钟掉落一个兔毛",
                    1.0f,       // 100%触发（被动）
                    1.0f,
                    1,          // 掉落1个
                    true        // 被动技能
                ));
                std::cout << "  获得技能: 脱毛" << std::endl;
            }
            else if (skillChance.first == SKILL_CHEER) {
                skills.push_back(PetSkill(
                    SKILL_CHEER,
                    "兔子助威",
                    "增加主人3%最大攻击力",
                    1.0f,       // 100%触发（被动）
                    1.0f,
                    0.03f,      // 3%加成
                    true        // 被动技能
                ));
                std::cout << "  获得技能: 兔子助威" << std::endl;
            }
        }
    }
}

// ============================================================================
// 更新
// ============================================================================
void PetRabbit::update(float dt, const sf::Vector2f& ownerPos, bool ownerAttacking) {
    // 更新跟随AI
    updateFollowAI(dt, ownerPos);
    
    // 更新攻击AI
    updateAttackAI(dt, ownerAttacking);
    
    // 更新动画
    updateAnimation(dt);
    
    // 更新脱毛计时器
    if (hasSkill(SKILL_SHEDDING)) {
        sheddingTimer += dt;
    }
    
    // 更新精灵位置
    updateSprite();
}

void PetRabbit::updateAnimation(float dt) {
    animTimer += dt;
    
    if (animTimer >= frameTime) {
        animTimer = 0;
        currentFrame = (currentFrame + 1) % FRAMES_PER_ROW;
    }
    
    // 根据状态和方向设置动画行
    int row = ROW_MOVE_DOWN;
    
    if (animState == PetAnimState::Attack) {
        switch (direction) {
            case PetDirection::Down:  row = ROW_ATTACK_DOWN; break;
            case PetDirection::Up:    row = ROW_ATTACK_UP; break;
            case PetDirection::Left:  row = ROW_ATTACK_LEFT; break;
            case PetDirection::Right: row = ROW_ATTACK_RIGHT; break;
        }
    } else if (animState == PetAnimState::Follow) {
        switch (direction) {
            case PetDirection::Down:  row = ROW_MOVE_DOWN; break;
            case PetDirection::Up:    row = ROW_MOVE_UP; break;
            case PetDirection::Left:  row = ROW_MOVE_LEFT; break;
            case PetDirection::Right: row = ROW_MOVE_RIGHT; break;
        }
    } else {
        // Idle状态使用第一帧
        switch (direction) {
            case PetDirection::Down:  row = ROW_MOVE_DOWN; break;
            case PetDirection::Up:    row = ROW_MOVE_UP; break;
            case PetDirection::Left:  row = ROW_MOVE_LEFT; break;
            case PetDirection::Right: row = ROW_MOVE_RIGHT; break;
        }
    }
    
    sprite.setTextureRect(getFrameRect(row, animState == PetAnimState::Idle ? 0 : currentFrame));
}

sf::IntRect PetRabbit::getFrameRect(int row, int col) const {
    return sf::IntRect(
        col * FRAME_WIDTH,
        row * FRAME_HEIGHT,
        FRAME_WIDTH,
        FRAME_HEIGHT
    );
}

void PetRabbit::updateDirectionFromVelocity() {
    if (std::abs(velocity.x) > std::abs(velocity.y)) {
        direction = velocity.x > 0 ? PetDirection::Right : PetDirection::Left;
    } else if (velocity.y != 0) {
        direction = velocity.y > 0 ? PetDirection::Down : PetDirection::Up;
    }
}

void PetRabbit::setAnimState(PetRabbitAnimState state) {
    if (rabbitAnimState != state) {
        rabbitAnimState = state;
        currentFrame = 0;
        animTimer = 0;
    }
}

// ============================================================================
// 渲染
// ============================================================================
void PetRabbit::render(sf::RenderWindow& window) {
    if (!textureLoaded) return;
    
    // 绘制宠物精灵
    window.draw(sprite);
    
    // 可以在这里添加资质光环效果等
}

// ============================================================================
// 获取宠物图标矩形（精灵表第一帧 - 面朝下站立）
// ============================================================================
sf::IntRect PetRabbit::getIconRect() const {
    // 返回精灵表第一行第一帧（向下移动/站立）
    return sf::IntRect(0, 0, FRAME_WIDTH, FRAME_HEIGHT);
}

// ============================================================================
// 碰撞检测
// ============================================================================
sf::FloatRect PetRabbit::getCollisionBox() const {
    sf::FloatRect bounds = getBounds();
    float shrink = 0.3f;
    return sf::FloatRect(
        bounds.left + bounds.width * shrink / 2,
        bounds.top + bounds.height * shrink,
        bounds.width * (1 - shrink),
        bounds.height * (1 - shrink)
    );
}

// ============================================================================
// 兔子特有功能
// ============================================================================
bool PetRabbit::checkShedding(float dt) {
    if (!hasSkill(SKILL_SHEDDING)) return false;
    
    if (sheddingTimer >= SHEDDING_INTERVAL) {
        sheddingTimer = 0;
        std::cout << name << " 掉落了一个兔毛!" << std::endl;
        return true;
    }
    
    return false;
}

float PetRabbit::getOwnerAttackBonus() const {
    for (const auto& skill : skills) {
        if (skill.id == SKILL_CHEER) {
            return skill.effectValue;  // 返回3%
        }
    }
    return 0.0f;
}
