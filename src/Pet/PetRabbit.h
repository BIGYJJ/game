#pragma once
#include "Pet.h"

// ============================================================================
// 兔子宠物 (Pet Rabbit)
// 
// 继承自Pet基类，实现兔子特有的属性和技能
// 
// 【兔子技能】
//   - 撕咬: 30%触发双倍普通攻击伤害
//   - 脱毛: 每20分钟掉落一个兔毛（被动）
//   - 兔子助威: 增加主人3%最大攻击力（稀有资质专属）
//
// 【资质属性配置】详见附表
// ============================================================================

// 兔子宠物动画状态
enum class PetRabbitAnimState {
    IdleDown,
    IdleUp,
    IdleLeft,
    IdleRight,
    MoveDown,
    MoveUp,
    MoveLeft,
    MoveRight,
    AttackDown,
    AttackUp,
    AttackLeft,
    AttackRight
};

class PetRabbit : public Pet {
public:
    PetRabbit();
    PetRabbit(float x, float y);
    
    // ========================================
    // Pet接口实现
    // ========================================
    void update(float dt, const sf::Vector2f& ownerPos, bool ownerAttacking) override;
    void render(sf::RenderWindow& window) override;
    std::string getPetTypeName() const override { return "兔子"; }
    int getPetTypeId() const override { return 1; }
    
    // 获取宠物图标矩形（精灵表第一帧）
    sf::IntRect getIconRect() const override;
    
    // ========================================
    // 初始化
    // ========================================
    void init(float x, float y) override;
    bool loadTexture(const std::string& texturePath) override;
    void hatch(PetQuality quality, int enhancerCount = 0) override;
    
    // ========================================
    // 碰撞检测
    // ========================================
    sf::FloatRect getCollisionBox() const override;
    
    // ========================================
    // 兔子特有功能
    // ========================================
    
    // 检查脱毛（被动技能），返回是否掉落兔毛
    bool checkShedding(float dt);
    
    // 获取对主人的攻击力加成（兔子助威技能）
    float getOwnerAttackBonus() const;
    
    // 获取兔毛掉落计时器
    float getSheddingTimer() const { return sheddingTimer; }
    float getSheddingInterval() const { return SHEDDING_INTERVAL; }

private:
    void initQualityConfigs();
    void updateAnimation(float dt);
    void setAnimState(PetRabbitAnimState state);
    sf::IntRect getFrameRect(int row, int col) const;
    void updateDirectionFromVelocity();

protected:
    void rollSkills() override;

private:
    // === 动画 ===
    PetRabbitAnimState rabbitAnimState;
    int currentFrame;
    float animTimer;
    float frameTime;
    
    // 精灵表参数（使用与兔子怪物相同的精灵表）
    static constexpr int FRAME_WIDTH = 32;
    static constexpr int FRAME_HEIGHT = 32;
    static constexpr int FRAMES_PER_ROW = 4;
    
    // 行定义
    static constexpr int ROW_MOVE_DOWN = 0;
    static constexpr int ROW_MOVE_UP = 1;
    static constexpr int ROW_MOVE_LEFT = 2;
    static constexpr int ROW_MOVE_RIGHT = 3;
    static constexpr int ROW_ATTACK_DOWN = 4;
    static constexpr int ROW_ATTACK_UP = 5;
    static constexpr int ROW_ATTACK_LEFT = 6;
    static constexpr int ROW_ATTACK_RIGHT = 7;
    
    // === 脱毛计时器 ===
    float sheddingTimer;
    static constexpr float SHEDDING_INTERVAL = 1200.0f;  // 20分钟 = 1200秒
    
    // === 技能ID ===
    static constexpr const char* SKILL_BITE = "rabbit_bite";
    static constexpr const char* SKILL_SHEDDING = "rabbit_shedding";
    static constexpr const char* SKILL_CHEER = "rabbit_cheer";
};
