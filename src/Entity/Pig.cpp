#include "Pig.h"
#include <iostream>
#include <random>

// ============================================================================
// Piglet 实现
// ============================================================================

Piglet::Piglet() : Fowl() {
    setupPigletStats();
}

Piglet::Piglet(float x, float y) : Piglet() {
    init(x, y);
}

void Piglet::init(float x, float y) {
    Fowl::init(x, y);
}

std::string Piglet::getEvolveTo() const {
    // 动态决定进化目标：90%母猪，10%臭猪
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    if (dist(rng) < 0.10f) {
        return "臭猪";
    }
    return "母猪";
}

void Piglet::setupPigletStats() {
    fowlName = "小猪";
    fowlTypeName = "小猪";
    
    // 基础属性（根据家禽表）
    maxHealth = PIGLET_HEALTH;
    health = maxHealth;
    healthMin = PIGLET_HEALTH;
    healthMax = PIGLET_HEALTH;
    
    defense = PIGLET_DEFENSE;
    defenseMin = PIGLET_DEFENSE;
    defenseMax = PIGLET_DEFENSE;
    
    attack = PIGLET_ATTACK;
    attackMin = PIGLET_ATTACK;
    attackMax = PIGLET_ATTACK;
    
    dodge = 0;
    dodgeMin = 0;
    dodgeMax = 0;
    
    // 饥饿系统
    maxSatiety = PIGLET_MAX_SATIETY;
    satiety = maxSatiety;
    dailyHunger = PIGLET_DAILY_HUNGER;
    
    // 生长系统
    canEvolveFlag = true;
    evolveTo = "母猪";  // 默认，实际由getEvolveTo()动态决定
    growthDays = 0;
    growthCycleDays = PIGLET_GROWTH_DAYS;
    
    // 产出系统（小猪无产出）
    hasProduct = false;
    productCycleDays = 0;
    productTimer = 0.0f;
    
    // 攻击能力（小猪可以攻击）
    canAttack = true;
    
    // 移动速度
    moveSpeed = PIGLET_SPEED;
    chaseSpeed = PIGLET_SPEED * 1.2f;
    returnSpeed = PIGLET_SPEED;
    
    // 掉落奖励（根据家禽表）
    expMin = 60;
    expMax = 100;
    goldMin = 40;
    goldMax = 80;
    
    // 掉落物品：猪肉(3)(70%), 猪皮(2)(30%)
    drops.clear();
    drops.push_back(MonsterDrop("pork", "猪肉", 1, 3, 0.70f));
    drops.push_back(MonsterDrop("pigskin", "猪皮", 1, 2, 0.30f));
    
    std::cout << "[Piglet] 小猪已创建" << std::endl;
}

// ============================================================================
// Sow 实现
// ============================================================================

Sow::Sow() : Fowl() {
    setupSowStats();
}

Sow::Sow(float x, float y) : Sow() {
    init(x, y);
}

void Sow::init(float x, float y) {
    Fowl::init(x, y);
}

void Sow::setupSowStats() {
    fowlName = "母猪";
    fowlTypeName = "母猪";
    
    // 基础属性（根据家禽表）
    maxHealth = SOW_HEALTH;
    health = maxHealth;
    healthMin = SOW_HEALTH;
    healthMax = SOW_HEALTH;
    
    defense = SOW_DEFENSE;
    defenseMin = SOW_DEFENSE;
    defenseMax = SOW_DEFENSE;
    
    attack = SOW_ATTACK;
    attackMin = SOW_ATTACK;
    attackMax = SOW_ATTACK;
    
    dodge = 3;
    dodgeMin = 2;
    dodgeMax = 5;
    
    // 饥饿系统
    maxSatiety = SOW_MAX_SATIETY;
    satiety = maxSatiety;
    dailyHunger = SOW_DAILY_HUNGER;
    
    // 生长系统（母猪不进化）
    canEvolveFlag = false;
    evolveTo = "";
    growthDays = 0;
    growthCycleDays = 0;
    
    // 产出系统（产出小猪）
    hasProduct = true;
    product = FowlProduct("piglet_spawn", "小猪", 1, 1);  // 特殊产出：生成小猪
    productCycleDays = SOW_PRODUCT_DAYS;
    productTimer = 0.0f;
    
    // 攻击能力
    canAttack = true;
    
    // 移动速度
    moveSpeed = SOW_SPEED;
    chaseSpeed = SOW_SPEED * 1.5f;
    returnSpeed = SOW_SPEED;
    
    // AI参数
    aggroDuration = 8.0f;
    attackRange = 50.0f;
    attackCooldownTime = 1.2f;
    chaseRange = 200.0f;
    leashRange = 350.0f;
    
    // 技能：猪猪冲撞
    skill = MonsterSkill("charge", "猪猪冲撞", "冲向一段距离造成50点伤害", 
                         CHARGE_DAMAGE / SOW_ATTACK, CHARGE_PROB, sf::Color(200, 100, 50));
    
    // 掉落奖励（根据家禽表）
    expMin = 150;
    expMax = 250;
    goldMin = 150;
    goldMax = 230;
    
    // 掉落物品：猪肉(5)(80%), 猪皮(3)(50%), 猪后腿(1)(30%), 骨头(4)(50%), 猪心(1)(10%)
    drops.clear();
    drops.push_back(MonsterDrop("pork", "猪肉", 1, 5, 0.80f));
    drops.push_back(MonsterDrop("pigskin", "猪皮", 1, 3, 0.50f));
    drops.push_back(MonsterDrop("pig_hind_leg", "猪后腿", 1, 1, 0.30f));
    drops.push_back(MonsterDrop("bone", "骨头", 1, 4, 0.50f));
    drops.push_back(MonsterDrop("pork_heart", "猪心", 1, 1, 0.10f));
    
    std::cout << "[Sow] 母猪已创建" << std::endl;
}

std::vector<std::pair<std::string, int>> Sow::harvest() {
    std::vector<std::pair<std::string, int>> result;
    
    if (canProduce()) {
        // 母猪产出小猪（特殊标记，由管理器处理生成）
        result.push_back(std::make_pair("piglet_spawn", 1));
        productTimer = 0.0f;
        
        if (onProduce) {
            onProduce(*this);
        }
    }
    
    return result;
}

// ============================================================================
// StinkyPig 实现
// ============================================================================

StinkyPig::StinkyPig() : Fowl() {
    setupStinkyPigStats();
}

StinkyPig::StinkyPig(float x, float y) : StinkyPig() {
    init(x, y);
}

void StinkyPig::init(float x, float y) {
    Fowl::init(x, y);
}

void StinkyPig::setupStinkyPigStats() {
    fowlName = "臭猪";
    fowlTypeName = "臭猪";
    
    // 基础属性（根据家禽表）
    maxHealth = STINKY_HEALTH;
    health = maxHealth;
    healthMin = STINKY_HEALTH;
    healthMax = STINKY_HEALTH;
    
    defense = STINKY_DEFENSE;
    defenseMin = STINKY_DEFENSE;
    defenseMax = STINKY_DEFENSE;
    
    attack = STINKY_ATTACK;
    attackMin = STINKY_ATTACK;
    attackMax = STINKY_ATTACK;
    
    dodge = 5;
    dodgeMin = 3;
    dodgeMax = 8;
    
    // 饥饿系统（臭猪不需要喂食）
    maxSatiety = 0.0f;
    satiety = 0.0f;
    dailyHunger = 0.0f;
    
    // 生长系统（臭猪不进化）
    canEvolveFlag = false;
    evolveTo = "";
    growthDays = 0;
    growthCycleDays = 0;
    
    // 产出系统（臭猪无产出）
    hasProduct = false;
    productCycleDays = 0;
    productTimer = 0.0f;
    
    // 攻击能力（臭猪会无差别攻击）
    canAttack = true;
    
    // 移动速度
    moveSpeed = STINKY_SPEED;
    chaseSpeed = STINKY_SPEED * 1.5f;
    returnSpeed = STINKY_SPEED;
    
    // AI参数（更激进）
    aggroDuration = 15.0f;  // 更长的激怒时间
    attackRange = 60.0f;
    attackCooldownTime = 0.8f;  // 更快的攻击速度
    chaseRange = 300.0f;  // 更大的追击范围
    leashRange = 500.0f;
    
    // 技能：撕咬（250%伤害）
    skill = MonsterSkill("bite", "撕咬", "造成250%普通攻击伤害", 
                         BITE_MULTIPLIER, BITE_PROB, sf::Color(150, 50, 50));
    
    // 掉落奖励（根据家禽表）
    expMin = 200;
    expMax = 300;
    goldMin = 300;
    goldMax = 500;
    
    // 掉落物品：臭猪肉(3)(50%), 猪皮(6)(60%), 长矛(1)(30%), 臭猪精元(1)(2%)
    drops.clear();
    drops.push_back(MonsterDrop("stinky_pork", "臭猪肉", 1, 3, 0.50f));
    drops.push_back(MonsterDrop("pigskin", "猪皮", 1, 6, 0.60f));
    drops.push_back(MonsterDrop("spear", "长矛", 1, 1, 0.30f));
    drops.push_back(MonsterDrop("stinky_pork_essence", "臭猪精元", 1, 1, 0.02f));
    
    std::cout << "[StinkyPig] 臭猪已创建" << std::endl;
}
