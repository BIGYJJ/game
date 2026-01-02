#include "Hen.h"
#include <iostream>

// ============================================================================
// Hen 实现
// ============================================================================

Hen::Hen() : Fowl() {
    setupHenStats();
}

Hen::Hen(float x, float y) : Hen() {
    init(x, y);
}

void Hen::init(float x, float y) {
    Fowl::init(x, y);
}

void Hen::setupHenStats() {
    // 设置名称
    fowlName = "母鸡";
    fowlTypeName = "母鸡";
    
    // 设置基础属性（根据家禽表）
    maxHealth = HEN_HEALTH;
    health = maxHealth;
    healthMin = HEN_HEALTH;
    healthMax = HEN_HEALTH;
    
    defense = HEN_DEFENSE;
    defenseMin = HEN_DEFENSE;
    defenseMax = HEN_DEFENSE;
    
    attack = HEN_ATTACK;
    attackMin = HEN_ATTACK;
    attackMax = HEN_ATTACK;
    
    // 有一定闪避
    dodge = 2;
    dodgeMin = 1;
    dodgeMax = 3;
    
    // 饥饿系统
    maxSatiety = HEN_MAX_SATIETY;
    satiety = maxSatiety;
    dailyHunger = HEN_DAILY_HUNGER;
    
    // 生长系统（母鸡不能进化）
    canEvolveFlag = false;
    evolveTo = "";
    growthDays = 0;
    growthCycleDays = 0;
    
    // 产出系统
    hasProduct = true;
    product = FowlProduct("egg", "鸡蛋", 1, 1);
    productCycleDays = HEN_PRODUCT_DAYS;
    productTimer = 0.0f;
    
    // 攻击能力（母鸡可以攻击，和怪物类一样）
    canAttack = true;
    
    // 移动速度
    moveSpeed = HEN_SPEED;
    chaseSpeed = HEN_CHASE_SPEED;
    returnSpeed = HEN_SPEED;
    
    // AI参数（和怪物类一样）
    aggroDuration = HEN_AGGRO_DURATION;
    attackRange = HEN_ATTACK_RANGE;
    attackCooldownTime = HEN_ATTACK_COOLDOWN;
    chaseRange = HEN_CHASE_RANGE;
    leashRange = HEN_LEASH_RANGE;
    
    // 设置技能（母鸡的啄击）
    skill = MonsterSkill("peck", "啄击", "凶猛啄击，造成1.5倍伤害", 1.5f, 0.15f, sf::Color(255, 150, 50));
    
    // 掉落奖励（根据家禽表）
    expMin = 20;
    expMax = 40;
    goldMin = 15;
    goldMax = 20;
    
    // 掉落物品（根据家禽表：鸡肉(4)(50%), 鸡蛋(3)(60%)）
    drops.clear();
    drops.push_back(MonsterDrop("chicken", "鸡肉", 1, 4, 0.50f));
    drops.push_back(MonsterDrop("egg", "鸡蛋", 1, 3, 0.60f));
    
    std::cout << "[Hen] 母鸡已创建" << std::endl;
}
