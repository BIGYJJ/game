#include "PlayerStats.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cstdlib>

// ============================================================================
// 构造函数 - 初始化默认属性
// ============================================================================

PlayerStats::PlayerStats()
    // 基础属性
    : health(100.0f)
    , maxHealth(100.0f)
    , stamina(100.0f)
    , maxStamina(100.0f)
    , hunger(100.0f)
    , maxHunger(100.0f)
    // 等级经验
    , level(1)
    , exp(0)
    , expToNextLevel(100)
    // 战斗基础值
    , baseAttack(10.0f)
    , baseDefense(5.0f)
    , baseSpeed(100.0f)
    , baseDodge(5.0f)
    // 战斗加成值
    , bonusAttack(0.0f)
    , bonusDefense(0.0f)
    , bonusSpeed(0.0f)
    , bonusDodge(0.0f)
    // 特殊属性
    , luck(10.0f)
    , damageBonus(0.0f)
    , dodgeReduction(0.0f)
    // 财产
    , gold(100)
    // 计时器
    , hungerDecayTimer(0.0f)
    , staminaRegenTimer(0.0f)
    , healthRegenTimer(0.0f)
    // 回调
    , onLevelUp(nullptr)
    , onHealthChange(nullptr)
    , onStaminaChange(nullptr)
    , onGoldChange(nullptr)
    , onSkillLevelUp(nullptr)
{
    // 初始化生活技能
    farmingSkill = SkillInfo();
    fishingSkill = SkillInfo();
    miningSkill = SkillInfo();
}

// ============================================================================
// 生命值系统
// ============================================================================

void PlayerStats::setHealth(float value) {
    float oldHealth = health;
    health = clamp(value, 0.0f, maxHealth);
    if (health != oldHealth && onHealthChange) {
        onHealthChange();
    }
}

void PlayerStats::setMaxHealth(float value) {
    maxHealth = std::max(1.0f, value);
    if (health > maxHealth) {
        setHealth(maxHealth);
    }
}

void PlayerStats::modifyHealth(float delta) {
    setHealth(health + delta);
}

void PlayerStats::heal(float amount) {
    // 幸运值影响治疗效果 (0-20% 额外治疗)
    float luckBonus = 1.0f + (luck / 500.0f);
    modifyHealth(amount * luckBonus);
}

void PlayerStats::takeDamage(float rawDamage) {
    float actualDamage = calculateDamageTaken(rawDamage);
    modifyHealth(-actualDamage);
}

// ============================================================================
// 体力值系统
// ============================================================================

void PlayerStats::setStamina(float value) {
    float oldStamina = stamina;
    stamina = clamp(value, 0.0f, maxStamina);
    if (stamina != oldStamina && onStaminaChange) {
        onStaminaChange();
    }
}

void PlayerStats::setMaxStamina(float value) {
    maxStamina = std::max(1.0f, value);
    if (stamina > maxStamina) {
        setStamina(maxStamina);
    }
}

void PlayerStats::modifyStamina(float delta) {
    setStamina(stamina + delta);
}

void PlayerStats::consumeStamina(float amount) {
    modifyStamina(-amount);
}

void PlayerStats::restoreStamina(float amount) {
    modifyStamina(amount);
}

// ============================================================================
// 饥饿度系统
// ============================================================================

void PlayerStats::setHunger(float value) {
    hunger = clamp(value, 0.0f, maxHunger);
}

void PlayerStats::setMaxHunger(float value) {
    maxHunger = std::max(1.0f, value);
    if (hunger > maxHunger) {
        hunger = maxHunger;
    }
}

void PlayerStats::modifyHunger(float delta) {
    setHunger(hunger + delta);
}

void PlayerStats::eat(float foodValue) {
    modifyHunger(foodValue);
    // 吃饱后小幅恢复体力
    if (hunger > maxHunger * 0.5f) {
        restoreStamina(foodValue * 0.1f);
    }
}

// ============================================================================
// 等级经验系统
// ============================================================================

void PlayerStats::addExp(int amount) {
    if (amount <= 0) return;
    
    // 幸运值影响经验获取 (0-10% 额外经验)
    float luckBonus = 1.0f + (luck / 1000.0f);
    exp += static_cast<int>(amount * luckBonus);
    
    checkLevelUp();
}

void PlayerStats::setLevel(int newLevel) {
    if (newLevel < 1) newLevel = 1;
    if (newLevel > 99) newLevel = 99;
    
    level = newLevel;
    exp = 0;
    expToNextLevel = calculateExpForLevel(level);
    
    // 重新计算基础属性
    baseAttack = 10.0f + (level - 1) * 2.0f;
    baseDefense = 5.0f + (level - 1) * 1.5f;
    baseSpeed = 100.0f + (level - 1) * 1.0f;
    baseDodge = 5.0f + (level - 1) * 0.5f;
    
    // 更新最大生命/体力
    maxHealth = 100.0f + (level - 1) * 10.0f;
    maxStamina = 100.0f + (level - 1) * 5.0f;
}

void PlayerStats::checkLevelUp() {
    while (exp >= expToNextLevel && level < 99) {
        exp -= expToNextLevel;
        level++;
        expToNextLevel = calculateExpForLevel(level);
        applyLevelUpBonus();
        
        if (onLevelUp) {
            onLevelUp();
        }
    }
}

void PlayerStats::applyLevelUpBonus() {
    // 每级属性成长
    baseAttack += 2.0f;
    baseDefense += 1.5f;
    baseSpeed += 1.0f;
    baseDodge += 0.5f;
    
    // 最大值成长
    maxHealth += 10.0f;
    maxStamina += 5.0f;
    
    // 升级时完全恢复
    health = maxHealth;
    stamina = maxStamina;
}

int PlayerStats::calculateExpForLevel(int lvl) const {
    // 经验公式: 100 * level^1.5
    return static_cast<int>(100 * std::pow(lvl, 1.5f));
}

// ============================================================================
// 战斗计算
// ============================================================================

float PlayerStats::calculateDamage(float baseDamage) const {
    // 最终伤害 = (基础伤害 + 攻击力) * (1 + 增伤百分比/100)
    float totalAttack = baseDamage + getAttack();
    float multiplier = 1.0f + (damageBonus / 100.0f);
    return totalAttack * multiplier;
}

float PlayerStats::calculateDamageTaken(float incomingDamage) const {
    // 伤害减免公式: 防御 / (防御 + 100)
    // 例如: 50防御 = 33%减伤, 100防御 = 50%减伤
    float defense = getDefense();
    float reduction = defense / (defense + 100.0f);
    float actualDamage = incomingDamage * (1.0f - reduction);
    return std::max(1.0f, actualDamage);  // 至少受到1点伤害
}

bool PlayerStats::rollDodge(float enemyDodgeReduction) const {
    // 实际闪避率 = 自身闪避 - 敌人减闪
    float effectiveDodge = getDodge() - enemyDodgeReduction;
    effectiveDodge = clamp(effectiveDodge, 0.0f, 75.0f);  // 最高75%闪避
    
    float roll = static_cast<float>(rand() % 100);
    return roll < effectiveDodge;
}

bool PlayerStats::rollCritical() const {
    // 暴击率 = 幸运值 / 2 (最高50%)
    float critChance = std::min(luck / 2.0f, 50.0f);
    float roll = static_cast<float>(rand() % 100);
    return roll < critChance;
}

float PlayerStats::getCriticalMultiplier() const {
    // 暴击倍率 = 1.5 + 幸运/100 (幸运100时为2.5倍)
    return 1.5f + (luck / 100.0f);
}

// ============================================================================
// 财产系统
// ============================================================================

void PlayerStats::setGold(int amount) {
    int oldGold = gold;
    gold = std::max(0, amount);
    if (gold != oldGold && onGoldChange) {
        onGoldChange();
    }
}

void PlayerStats::addGold(int amount) {
    setGold(gold + amount);
}

bool PlayerStats::spendGold(int amount) {
    if (amount <= 0) return true;
    if (gold < amount) return false;
    
    setGold(gold - amount);
    return true;
}

// ============================================================================
// 生活技能系统
// ============================================================================

const SkillInfo& PlayerStats::getSkill(LifeSkill skill) const {
    switch (skill) {
        case LifeSkill::Farming: return farmingSkill;
        case LifeSkill::Fishing: return fishingSkill;
        case LifeSkill::Mining:  return miningSkill;
        default: return farmingSkill;
    }
}

SkillInfo& PlayerStats::getSkillRef(LifeSkill skill) {
    switch (skill) {
        case LifeSkill::Farming: return farmingSkill;
        case LifeSkill::Fishing: return fishingSkill;
        case LifeSkill::Mining:  return miningSkill;
        default: return farmingSkill;
    }
}

int PlayerStats::getSkillLevel(LifeSkill skill) const {
    return getSkill(skill).level;
}

int PlayerStats::getSkillExp(LifeSkill skill) const {
    return getSkill(skill).exp;
}

float PlayerStats::getSkillExpPercent(LifeSkill skill) const {
    const SkillInfo& info = getSkill(skill);
    return static_cast<float>(info.exp) / info.expToNext;
}

void PlayerStats::addSkillExp(LifeSkill skill, int amount) {
    if (amount <= 0) return;
    
    SkillInfo& info = getSkillRef(skill);
    info.exp += amount;
    
    checkSkillLevelUp(skill);
}

void PlayerStats::setSkillLevel(LifeSkill skill, int newLevel) {
    if (newLevel < 1) newLevel = 1;
    if (newLevel > 10) newLevel = 10;  // 技能最高10级
    
    SkillInfo& info = getSkillRef(skill);
    info.level = newLevel;
    info.exp = 0;
    info.expToNext = calculateSkillExpForLevel(newLevel);
}

void PlayerStats::checkSkillLevelUp(LifeSkill skill) {
    SkillInfo& info = getSkillRef(skill);
    
    while (info.exp >= info.expToNext && info.level < 10) {
        info.exp -= info.expToNext;
        info.level++;
        info.expToNext = calculateSkillExpForLevel(info.level);
        
        if (onSkillLevelUp) {
            onSkillLevelUp();
        }
    }
}

int PlayerStats::calculateSkillExpForLevel(int lvl) const {
    // 技能经验公式: 100 * level^2
    return 100 * lvl * lvl;
}

float PlayerStats::getFarmingYieldBonus() const {
    // 每级增加10%产量
    return (farmingSkill.level - 1) * 0.10f;
}

float PlayerStats::getFishingSuccessBonus() const {
    // 每级增加8%成功率
    return (fishingSkill.level - 1) * 0.08f;
}

float PlayerStats::getMiningSpeedBonus() const {
    // 每级增加12%采矿速度
    return (miningSkill.level - 1) * 0.12f;
}

// ============================================================================
// 每帧更新
// ============================================================================

void PlayerStats::update(float dt) {
    // === 饥饿度下降 ===
    hungerDecayTimer += dt;
    if (hungerDecayTimer >= HUNGER_DECAY_INTERVAL) {
        hungerDecayTimer = 0.0f;
        modifyHunger(-HUNGER_DECAY_RATE);
        
        // 饥饿时受到伤害
        if (isStarving()) {
            takeDamage(STARVING_DAMAGE);
        }
    }
    
    // === 体力自然恢复 ===
    // 只有在不饥饿时才恢复体力
    if (!isHungry() && stamina < maxStamina) {
        staminaRegenTimer += dt;
        if (staminaRegenTimer >= STAMINA_REGEN_INTERVAL) {
            staminaRegenTimer = 0.0f;
            restoreStamina(STAMINA_REGEN_RATE);
        }
    }
    
    // === 生命自然恢复 ===
    // 只有在饥饿度>50%且体力>30%时恢复生命
    if (hunger > maxHunger * 0.5f && stamina > maxStamina * 0.3f && health < maxHealth) {
        healthRegenTimer += dt;
        if (healthRegenTimer >= HEALTH_REGEN_INTERVAL) {
            healthRegenTimer = 0.0f;
            heal(HEALTH_REGEN_RATE);
        }
    }
}

// ============================================================================
// 状态重置
// ============================================================================

void PlayerStats::fullRestore() {
    health = maxHealth;
    stamina = maxStamina;
    hunger = maxHunger;
}

void PlayerStats::respawn() {
    // 死亡重生：恢复50%生命和体力，保留等级和金币
    health = maxHealth * 0.5f;
    stamina = maxStamina * 0.5f;
    hunger = maxHunger * 0.5f;
}

void PlayerStats::resetToDefault() {
    // 完全重置
    *this = PlayerStats();
}

// ============================================================================
// 工具函数
// ============================================================================

float PlayerStats::clamp(float value, float minVal, float maxVal) const {
    return std::max(minVal, std::min(value, maxVal));
}

// ============================================================================
// 调试信息
// ============================================================================

std::string PlayerStats::getDebugString() const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    
    ss << "=== Player Stats ===" << std::endl;
    ss << "Lv." << level << " (" << exp << "/" << expToNextLevel << " EXP)" << std::endl;
    ss << std::endl;
    
    ss << "[Vitals]" << std::endl;
    ss << "  HP: " << health << "/" << maxHealth << std::endl;
    ss << "  SP: " << stamina << "/" << maxStamina << std::endl;
    ss << "  Hunger: " << hunger << "/" << maxHunger << std::endl;
    ss << std::endl;
    
    ss << "[Combat]" << std::endl;
    ss << "  ATK: " << getAttack() << " (" << baseAttack << "+" << bonusAttack << ")" << std::endl;
    ss << "  DEF: " << getDefense() << " (" << baseDefense << "+" << bonusDefense << ")" << std::endl;
    ss << "  SPD: " << getSpeed() << std::endl;
    ss << "  Dodge: " << getDodge() << "%" << std::endl;
    ss << "  Luck: " << luck << std::endl;
    ss << "  Dmg Bonus: +" << damageBonus << "%" << std::endl;
    ss << "  Dodge Red: " << dodgeReduction << "%" << std::endl;
    ss << std::endl;
    
    ss << "[Wealth]" << std::endl;
    ss << "  Gold: " << gold << std::endl;
    ss << std::endl;
    
    ss << "[Life Skills]" << std::endl;
    ss << "  Farming: Lv." << farmingSkill.level << " (" << farmingSkill.exp << "/" << farmingSkill.expToNext << ")" << std::endl;
    ss << "  Fishing: Lv." << fishingSkill.level << " (" << fishingSkill.exp << "/" << fishingSkill.expToNext << ")" << std::endl;
    ss << "  Mining:  Lv." << miningSkill.level << " (" << miningSkill.exp << "/" << miningSkill.expToNext << ")" << std::endl;
    
    return ss.str();
}