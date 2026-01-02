#pragma once
#include "Fowl.h"

// ============================================================================
// 牛类家禽 (Cattle Family)
// 
// 包含：小牛、耗牛
// ============================================================================

// ============================================================================
// 小牛类 (Calf)
// 
// 属性（根据家禽表）：
//   - 生命值：150
//   - 防御值：20
//   - 速度：10
//   - 攻击力：0（不会攻击人物）
//   - 进化：耗牛(80%), 奶牛(15%), 犀牛(5%)
//   - 产出：无
//   - 饱腹值：400/400
//   - 饥饿值：40
//   - 生长周期：20天
// ============================================================================

class Calf : public Fowl {
public:
    Calf();
    Calf(float x, float y);
    
    std::string getName() const override { return "小牛"; }
    std::string getTypeName() const override { return "小牛"; }
    std::string getEvolveTo() const override;  // 动态决定进化目标
    
    void init(float x, float y) override;
    
private:
    void setupCalfStats();
    
    static constexpr float CALF_HEALTH = 150.0f;
    static constexpr float CALF_DEFENSE = 20.0f;
    static constexpr float CALF_SPEED = 50.0f;
    static constexpr float CALF_ATTACK = 0.0f;  // 不攻击
    static constexpr float CALF_MAX_SATIETY = 400.0f;
    static constexpr float CALF_DAILY_HUNGER = 40.0f;
    static constexpr int CALF_GROWTH_DAYS = 20;
};

// ============================================================================
// 耗牛类 (Yak)
// 
// 属性（根据家禽表）：
//   - 生命值：400
//   - 防御值：60
//   - 速度：6
//   - 攻击力：0（不会攻击人物）
//   - 进化：无
//   - 产出：小牛，30天/只
//   - 饱腹值：600/600
//   - 饥饿值：80
// ============================================================================

class Yak : public Fowl {
public:
    Yak();
    Yak(float x, float y);
    
    std::string getName() const override { return "耗牛"; }
    std::string getTypeName() const override { return "耗牛"; }
    std::string getEvolveTo() const override { return ""; }
    
    void init(float x, float y) override;
    
    // 产出小牛
    std::vector<std::pair<std::string, int>> harvest() override;
    
private:
    void setupYakStats();
    
    static constexpr float YAK_HEALTH = 400.0f;
    static constexpr float YAK_DEFENSE = 60.0f;
    static constexpr float YAK_SPEED = 30.0f;  // 速度6转换
    static constexpr float YAK_ATTACK = 0.0f;  // 不攻击
    static constexpr float YAK_MAX_SATIETY = 600.0f;
    static constexpr float YAK_DAILY_HUNGER = 80.0f;
    static constexpr int YAK_PRODUCT_DAYS = 30;
};
