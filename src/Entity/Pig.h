#pragma once
#include "Fowl.h"

// ============================================================================
// 猪类家禽 (Pig Family)
// 
// 包含：小猪、母猪、臭猪
// ============================================================================

// ============================================================================
// 小猪类 (Piglet)
// 
// 属性（根据家禽表）：
//   - 生命值：100
//   - 防御值：10
//   - 速度：10
//   - 攻击力：15
//   - 进化：母猪(90%), 臭猪(10%)
//   - 产出：无
//   - 饱腹值：350/350
//   - 饥饿值：30
//   - 生长周期：20天
// ============================================================================

class Piglet : public Fowl {
public:
    Piglet();
    Piglet(float x, float y);
    
    std::string getName() const override { return "小猪"; }
    std::string getTypeName() const override { return "小猪"; }
    std::string getEvolveTo() const override;  // 动态决定进化目标
    
    void init(float x, float y) override;
    
private:
    void setupPigletStats();
    
    static constexpr float PIGLET_HEALTH = 100.0f;
    static constexpr float PIGLET_DEFENSE = 10.0f;
    static constexpr float PIGLET_SPEED = 50.0f;
    static constexpr float PIGLET_ATTACK = 15.0f;
    static constexpr float PIGLET_MAX_SATIETY = 350.0f;
    static constexpr float PIGLET_DAILY_HUNGER = 30.0f;
    static constexpr int PIGLET_GROWTH_DAYS = 20;
};

// ============================================================================
// 母猪类 (Sow)
// 
// 属性（根据家禽表）：
//   - 生命值：300
//   - 防御值：50
//   - 速度：30
//   - 攻击力：30
//   - 进化：无
//   - 产出：小猪，20天/只
//   - 饱腹值：600/600
//   - 饥饿值：50
//   - 技能：猪猪冲撞（20%概率，造成50点伤害）
// ============================================================================

class Sow : public Fowl {
public:
    Sow();
    Sow(float x, float y);
    
    std::string getName() const override { return "母猪"; }
    std::string getTypeName() const override { return "母猪"; }
    std::string getEvolveTo() const override { return ""; }
    
    void init(float x, float y) override;
    
    // 产出小猪
    std::vector<std::pair<std::string, int>> harvest() override;
    
private:
    void setupSowStats();
    
    static constexpr float SOW_HEALTH = 300.0f;
    static constexpr float SOW_DEFENSE = 50.0f;
    static constexpr float SOW_SPEED = 150.0f;
    static constexpr float SOW_ATTACK = 30.0f;
    static constexpr float SOW_MAX_SATIETY = 600.0f;
    static constexpr float SOW_DAILY_HUNGER = 50.0f;
    static constexpr int SOW_PRODUCT_DAYS = 20;
    
    // 冲撞技能伤害
    static constexpr float CHARGE_DAMAGE = 50.0f;
    static constexpr float CHARGE_PROB = 0.20f;
};

// ============================================================================
// 臭猪类 (StinkyPig)
// 
// 属性（根据家禽表）：
//   - 生命值：400
//   - 防御值：80
//   - 速度：20
//   - 攻击力：40
//   - 进化：无
//   - 产出：无
//   - 特性：会无差别攻击同类之后的任何生物
//   - 技能：撕咬（20%概率，造成250%普通攻击伤害）
// ============================================================================

class StinkyPig : public Fowl {
public:
    StinkyPig();
    StinkyPig(float x, float y);
    
    std::string getName() const override { return "臭猪"; }
    std::string getTypeName() const override { return "臭猪"; }
    std::string getEvolveTo() const override { return ""; }
    
    void init(float x, float y) override;
    
private:
    void setupStinkyPigStats();
    
    static constexpr float STINKY_HEALTH = 400.0f;
    static constexpr float STINKY_DEFENSE = 80.0f;
    static constexpr float STINKY_SPEED = 100.0f;
    static constexpr float STINKY_ATTACK = 40.0f;
    
    // 撕咬技能
    static constexpr float BITE_MULTIPLIER = 2.5f;
    static constexpr float BITE_PROB = 0.20f;
};
