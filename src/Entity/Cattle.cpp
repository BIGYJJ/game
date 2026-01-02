#include "Cattle.h"
#include <iostream>
#include <random>

// ============================================================================
// Calf 实现
// ============================================================================

Calf::Calf() : Fowl() {
    setupCalfStats();
}

Calf::Calf(float x, float y) : Calf() {
    init(x, y);
}

void Calf::init(float x, float y) {
    Fowl::init(x, y);
}

std::string Calf::getEvolveTo() const {
    // 动态决定进化目标：80%耗牛，15%奶牛，5%犀牛
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    float roll = dist(rng);
    if (roll < 0.05f) {
        return "犀牛";  // 5%
    } else if (roll < 0.20f) {
        return "奶牛";  // 15%
    }
    return "耗牛";  // 80%
}

void Calf::setupCalfStats() {
    fowlName = "小牛";
    fowlTypeName = "小牛";
    
    // 基础属性（根据家禽表）
    maxHealth = CALF_HEALTH;
    health = maxHealth;
    healthMin = CALF_HEALTH;
    healthMax = CALF_HEALTH;
    
    defense = CALF_DEFENSE;
    defenseMin = CALF_DEFENSE;
    defenseMax = CALF_DEFENSE;
    
    attack = CALF_ATTACK;
    attackMin = CALF_ATTACK;
    attackMax = CALF_ATTACK;
    
    dodge = 0;
    dodgeMin = 0;
    dodgeMax = 0;
    
    // 饥饿系统
    maxSatiety = CALF_MAX_SATIETY;
    satiety = maxSatiety;
    dailyHunger = CALF_DAILY_HUNGER;
    
    // 生长系统
    canEvolveFlag = true;
    evolveTo = "耗牛";  // 默认，实际由getEvolveTo()动态决定
    growthDays = 0;
    growthCycleDays = CALF_GROWTH_DAYS;
    
    // 产出系统（小牛无产出）
    hasProduct = false;
    productCycleDays = 0;
    productTimer = 0.0f;
    
    // 攻击能力（小牛不攻击）
    canAttack = false;
    
    // 移动速度
    moveSpeed = CALF_SPEED;
    chaseSpeed = CALF_SPEED;
    returnSpeed = CALF_SPEED;
    
    // 掉落奖励（根据家禽表）
    expMin = 60;
    expMax = 120;
    goldMin = 60;
    goldMax = 120;
    
    // 掉落物品：牛肉(3)(70%), 骨头(3)(30%)
    drops.clear();
    drops.push_back(MonsterDrop("beef", "牛肉", 1, 3, 0.70f));
    drops.push_back(MonsterDrop("bone", "骨头", 1, 3, 0.30f));
    
    std::cout << "[Calf] 小牛已创建" << std::endl;
}

// ============================================================================
// Yak 实现
// ============================================================================

Yak::Yak() : Fowl() {
    setupYakStats();
}

Yak::Yak(float x, float y) : Yak() {
    init(x, y);
}

void Yak::init(float x, float y) {
    Fowl::init(x, y);
}

void Yak::setupYakStats() {
    fowlName = "耗牛";
    fowlTypeName = "耗牛";
    
    // 基础属性（根据家禽表）
    maxHealth = YAK_HEALTH;
    health = maxHealth;
    healthMin = YAK_HEALTH;
    healthMax = YAK_HEALTH;
    
    defense = YAK_DEFENSE;
    defenseMin = YAK_DEFENSE;
    defenseMax = YAK_DEFENSE;
    
    attack = YAK_ATTACK;
    attackMin = YAK_ATTACK;
    attackMax = YAK_ATTACK;
    
    dodge = 0;
    dodgeMin = 0;
    dodgeMax = 0;
    
    // 饥饿系统
    maxSatiety = YAK_MAX_SATIETY;
    satiety = maxSatiety;
    dailyHunger = YAK_DAILY_HUNGER;
    
    // 生长系统（耗牛不进化）
    canEvolveFlag = false;
    evolveTo = "";
    growthDays = 0;
    growthCycleDays = 0;
    
    // 产出系统（产出小牛）
    hasProduct = true;
    product = FowlProduct("calf_spawn", "小牛", 1, 1);  // 特殊产出：生成小牛
    productCycleDays = YAK_PRODUCT_DAYS;
    productTimer = 0.0f;
    
    // 攻击能力（耗牛不攻击）
    canAttack = false;
    
    // 移动速度（较慢）
    moveSpeed = YAK_SPEED;
    chaseSpeed = YAK_SPEED;
    returnSpeed = YAK_SPEED;
    
    // 掉落奖励（根据家禽表）
    expMin = 150;
    expMax = 300;
    goldMin = 150;
    goldMax = 300;
    
    // 掉落物品：牛肉(6)(70%), 骨头(6)(50%), 耗牛毛(3)(50%)
    drops.clear();
    drops.push_back(MonsterDrop("beef", "牛肉", 1, 6, 0.70f));
    drops.push_back(MonsterDrop("bone", "骨头", 1, 6, 0.50f));
    drops.push_back(MonsterDrop("yak_fur", "耗牛毛", 1, 3, 0.50f));
    
    std::cout << "[Yak] 耗牛已创建" << std::endl;
}

std::vector<std::pair<std::string, int>> Yak::harvest() {
    std::vector<std::pair<std::string, int>> result;
    
    if (canProduce()) {
        // 耗牛产出小牛（特殊标记，由管理器处理生成）
        result.push_back(std::make_pair("calf_spawn", 1));
        productTimer = 0.0f;
        
        if (onProduce) {
            onProduce(*this);
        }
    }
    
    return result;
}
