#include "Chick.h"
#include <iostream>

// ============================================================================
// Chick 实现
// ============================================================================

Chick::Chick() : Fowl() {
    setupChickStats();
}

Chick::Chick(float x, float y) : Chick() {
    init(x, y);
}

void Chick::init(float x, float y) {
    Fowl::init(x, y);
}

void Chick::setupChickStats() {
    // 设置名称
    fowlName = "小鸡";
    fowlTypeName = "小鸡";
    
    // 设置基础属性（根据家禽表）
    maxHealth = CHICK_HEALTH;
    health = maxHealth;
    healthMin = CHICK_HEALTH;
    healthMax = CHICK_HEALTH;
    
    defense = CHICK_DEFENSE;
    defenseMin = CHICK_DEFENSE;
    defenseMax = CHICK_DEFENSE;
    
    attack = CHICK_ATTACK;
    attackMin = CHICK_ATTACK;
    attackMax = CHICK_ATTACK;
    
    // 不能闪避
    dodge = 0;
    dodgeMin = 0;
    dodgeMax = 0;
    
    // 饥饿系统
    maxSatiety = CHICK_MAX_SATIETY;
    satiety = maxSatiety;
    dailyHunger = CHICK_DAILY_HUNGER;
    
    // 生长系统
    canEvolveFlag = true;
    evolveTo = "母鸡";
    growthDays = 0;
    growthCycleDays = CHICK_GROWTH_DAYS;
    
    // 产出系统（小鸡无产出）
    hasProduct = false;
    productCycleDays = 0;
    productTimer = 0.0f;
    
    // 攻击能力（小鸡无攻击能力）
    canAttack = false;
    
    // 移动速度（小鸡移动较慢）
    moveSpeed = CHICK_SPEED;
    chaseSpeed = CHICK_SPEED;
    returnSpeed = CHICK_SPEED;
    
    // 掉落奖励（根据家禽表）
    expMin = 10;
    expMax = 20;
    goldMin = 6;
    goldMax = 12;
    
    // 掉落物品（根据家禽表：鸡肉(2)(30%)）
    drops.clear();
    drops.push_back(MonsterDrop("chicken", "鸡肉", 1, 2, 0.30f));
    
    std::cout << "[Chick] 小鸡已创建" << std::endl;
}
