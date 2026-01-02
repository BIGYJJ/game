#pragma once
#include "Fowl.h"

// ============================================================================
// 母鸡类 (Hen)
// 
// 属性（根据家禽表）：
//   - 生命值：50
//   - 防御值：3
//   - 速度：7（较快）
//   - 碰撞体积：有
//   - 进化：无
//   - 产出：鸡蛋
//   - 攻击能力：怪物类（被攻击后反击）
//   - 攻击力：10
//   - 饥饿值（每日消耗）：15
//   - 生长周期：无
//   - 产出速度：3天/鸡蛋
//   - 饱腹值：200/200
// ============================================================================

class Hen : public Fowl {
public:
    Hen();
    Hen(float x, float y);
    
    // ========================================
    // 重写虚函数
    // ========================================
    std::string getName() const override { return "母鸡"; }
    std::string getTypeName() const override { return "母鸡"; }
    std::string getEvolveTo() const override { return ""; }  // 母鸡不能进化
    
    // 初始化
    void init(float x, float y) override;
    
private:
    void setupHenStats();
    
    // === 母鸡常量 ===
    static constexpr float HEN_HEALTH = 50.0f;
    static constexpr float HEN_DEFENSE = 3.0f;
    static constexpr float HEN_SPEED = 35.0f;  // 速度值7转换为实际移动速度
    static constexpr float HEN_ATTACK = 10.0f;
    static constexpr float HEN_MAX_SATIETY = 200.0f;
    static constexpr float HEN_DAILY_HUNGER = 15.0f;
    static constexpr int HEN_PRODUCT_DAYS = 3;  // 3天产一个鸡蛋
    
    // === 母鸡AI参数（和怪物类似）===
    static constexpr float HEN_CHASE_SPEED = 60.0f;
    static constexpr float HEN_AGGRO_DURATION = 8.0f;
    static constexpr float HEN_ATTACK_RANGE = 35.0f;
    static constexpr float HEN_ATTACK_COOLDOWN = 1.2f;
    static constexpr float HEN_CHASE_RANGE = 200.0f;
    static constexpr float HEN_LEASH_RANGE = 300.0f;
};
