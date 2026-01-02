#pragma once
#include "Fowl.h"

// ============================================================================
// 小鸡类 (Chick)
// 
// 属性（根据家禽表）：
//   - 生命值：20
//   - 防御值：0
//   - 速度：5（较慢）
//   - 碰撞体积：有
//   - 进化：母鸡
//   - 产出：无
//   - 攻击能力：无
//   - 攻击力：0
//   - 饥饿值（每日消耗）：10
//   - 生长周期：10天
//   - 饱腹值：100/100
// ============================================================================

class Chick : public Fowl {
public:
    Chick();
    Chick(float x, float y);
    
    // ========================================
    // 重写虚函数
    // ========================================
    std::string getName() const override { return "小鸡"; }
    std::string getTypeName() const override { return "小鸡"; }
    std::string getEvolveTo() const override { return "母鸡"; }
    
    // 初始化
    void init(float x, float y) override;
    
private:
    void setupChickStats();
    
    // === 小鸡常量 ===
    static constexpr float CHICK_HEALTH = 20.0f;
    static constexpr float CHICK_DEFENSE = 0.0f;
    static constexpr float CHICK_SPEED = 25.0f;  // 速度值5转换为实际移动速度
    static constexpr float CHICK_ATTACK = 0.0f;
    static constexpr float CHICK_MAX_SATIETY = 100.0f;
    static constexpr float CHICK_DAILY_HUNGER = 10.0f;
    static constexpr int CHICK_GROWTH_DAYS = 10;
};
