#pragma once
#include <string>
#include <functional>

// ============================================================================
// 角色属性系统
// 
// 包含：
//   - 基础属性：生命、体力、饥饿度
//   - 等级经验系统
//   - 战斗属性：攻击、防御、速度、闪避、幸运、增伤%、减闪%
//   - 财产：金币
//   - 生活技能：种植、渔业、采矿
// ============================================================================

// 生活技能类型
enum class LifeSkill {
    Farming,    // 种植
    Fishing,    // 渔业
    Mining      // 采矿
};

// 技能信息结构
struct SkillInfo {
    int level;          // 当前等级
    int exp;            // 当前经验
    int expToNext;      // 升级所需经验
    
    SkillInfo() : level(1), exp(0), expToNext(100) {}
};

// 属性变化回调类型（用于UI更新等）
using StatsCallback = std::function<void()>;

class PlayerStats {
public:
    PlayerStats();
    
    // ========================================
    // 生命值系统
    // ========================================
    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    float getHealthPercent() const { return health / maxHealth; }
    void setHealth(float value);
    void setMaxHealth(float value);
    void modifyHealth(float delta);     // 正数回复，负数受伤
    void heal(float amount);            // 治疗（受幸运加成）
    void takeDamage(float rawDamage);   // 受伤（计算防御）
    bool isDead() const { return health <= 0; }
    
    // ========================================
    // 体力值系统
    // ========================================
    float getStamina() const { return stamina; }
    float getMaxStamina() const { return maxStamina; }
    float getStaminaPercent() const { return stamina / maxStamina; }
    void setStamina(float value);
    void setMaxStamina(float value);
    void modifyStamina(float delta);
    void consumeStamina(float amount);  // 消耗体力（劳作）
    void restoreStamina(float amount);  // 恢复体力
    bool isExhausted() const { return stamina <= 0; }
    bool hasStamina(float required) const { return stamina >= required; }
    
    // ========================================
    // 饥饿度系统
    // ========================================
    float getHunger() const { return hunger; }
    float getMaxHunger() const { return maxHunger; }
    float getHungerPercent() const { return hunger / maxHunger; }
    void setHunger(float value);
    void setMaxHunger(float value);
    void modifyHunger(float delta);
    void eat(float foodValue);          // 进食
    bool isStarving() const { return hunger <= maxHunger * 0.1f; }
    bool isHungry() const { return hunger <= maxHunger * 0.3f; }
    
    // ========================================
    // 等级经验系统
    // ========================================
    int getLevel() const { return level; }
    int getExp() const { return exp; }
    int getExpToNextLevel() const { return expToNextLevel; }
    float getExpPercent() const { return (float)exp / expToNextLevel; }
    void addExp(int amount);
    void setLevel(int newLevel);
    
    // ========================================
    // 战斗属性 - Getters（返回基础值+加成值）
    // ========================================
    float getAttack() const { return baseAttack + bonusAttack; }
    float getDefense() const { return baseDefense + bonusDefense; }
    float getSpeed() const { return baseSpeed + bonusSpeed; }
    float getDodge() const { return baseDodge + bonusDodge; }
    float getLuck() const { return luck; }
    float getDamageBonus() const { return damageBonus; }        // 增伤百分比
    float getDodgeReduction() const { return dodgeReduction; }  // 减闪百分比
    
    // 基础值 Getters
    float getBaseAttack() const { return baseAttack; }
    float getBaseDefense() const { return baseDefense; }
    float getBaseSpeed() const { return baseSpeed; }
    float getBaseDodge() const { return baseDodge; }
    
    // 加成值 Getters
    float getBonusAttack() const { return bonusAttack; }
    float getBonusDefense() const { return bonusDefense; }
    float getBonusSpeed() const { return bonusSpeed; }
    float getBonusDodge() const { return bonusDodge; }
    
    // ========================================
    // 战斗属性 - Setters
    // ========================================
    void setBaseAttack(float value) { baseAttack = value; }
    void setBaseDefense(float value) { baseDefense = value; }
    void setBaseSpeed(float value) { baseSpeed = value; }
    void setBaseDodge(float value) { baseDodge = value; }
    void setLuck(float value) { luck = value; }
    void setDamageBonus(float percent) { damageBonus = percent; }
    void setDodgeReduction(float percent) { dodgeReduction = percent; }
    
    // 加成值 Setters（装备/Buff使用）
    void setBonusAttack(float value) { bonusAttack = value; }
    void setBonusDefense(float value) { bonusDefense = value; }
    void setBonusSpeed(float value) { bonusSpeed = value; }
    void setBonusDodge(float value) { bonusDodge = value; }
    
    // 加成值修改（叠加）
    void addBonusAttack(float delta) { bonusAttack += delta; }
    void addBonusDefense(float delta) { bonusDefense += delta; }
    void addBonusSpeed(float delta) { bonusSpeed += delta; }
    void addBonusDodge(float delta) { bonusDodge += delta; }
    void addLuck(float delta) { luck += delta; }
    void addDamageBonus(float delta) { damageBonus += delta; }
    void addDodgeReduction(float delta) { dodgeReduction += delta; }
    
    // ========================================
    // 战斗计算
    // ========================================
    float calculateDamage(float baseDamage) const;      // 计算输出伤害
    float calculateDamageTaken(float incomingDamage) const;  // 计算承受伤害
    bool rollDodge(float enemyDodgeReduction = 0) const;     // 闪避判定
    bool rollCritical() const;                          // 暴击判定（基于幸运）
    float getCriticalMultiplier() const;                // 暴击倍率
    
    // ========================================
    // 财产系统
    // ========================================
    int getGold() const { return gold; }
    void setGold(int amount);
    void addGold(int amount);
    bool spendGold(int amount);         // 花费金币，返回是否成功
    bool canAfford(int cost) const { return gold >= cost; }
    
    // ========================================
    // 生活技能系统
    // ========================================
    const SkillInfo& getSkill(LifeSkill skill) const;
    int getSkillLevel(LifeSkill skill) const;
    int getSkillExp(LifeSkill skill) const;
    float getSkillExpPercent(LifeSkill skill) const;
    void addSkillExp(LifeSkill skill, int amount);
    void setSkillLevel(LifeSkill skill, int level);
    
    // 技能等级带来的加成
    float getFarmingYieldBonus() const;     // 种植产量加成
    float getFishingSuccessBonus() const;   // 钓鱼成功率加成
    float getMiningSpeedBonus() const;      // 采矿速度加成
    
    // ========================================
    // 每帧更新
    // ========================================
    void update(float dt);
    
    // ========================================
    // 状态重置
    // ========================================
    void fullRestore();             // 完全恢复（生命、体力、饥饿）
    void respawn();                 // 死亡重生
    void resetToDefault();          // 重置为初始状态
    
    // ========================================
    // 回调设置（用于UI更新等）
    // ========================================
    void setOnLevelUp(StatsCallback callback) { onLevelUp = callback; }
    void setOnHealthChange(StatsCallback callback) { onHealthChange = callback; }
    void setOnStaminaChange(StatsCallback callback) { onStaminaChange = callback; }
    void setOnGoldChange(StatsCallback callback) { onGoldChange = callback; }
    void setOnSkillLevelUp(StatsCallback callback) { onSkillLevelUp = callback; }
    
    // ========================================
    // 调试信息
    // ========================================
    std::string getDebugString() const;

private:
    // 等级计算
    void checkLevelUp();
    void applyLevelUpBonus();
    int calculateExpForLevel(int lvl) const;
    
    // 技能等级计算
    void checkSkillLevelUp(LifeSkill skill);
    int calculateSkillExpForLevel(int lvl) const;
    SkillInfo& getSkillRef(LifeSkill skill);
    
    // 数值限制
    float clamp(float value, float minVal, float maxVal) const;
    
private:
    // === 基础属性 ===
    float health;
    float maxHealth;
    float stamina;
    float maxStamina;
    float hunger;
    float maxHunger;
    
    // === 等级经验 ===
    int level;
    int exp;
    int expToNextLevel;
    
    // === 战斗属性 ===
    // 基础值（等级成长）
    float baseAttack;
    float baseDefense;
    float baseSpeed;
    float baseDodge;
    
    // 加成值（装备/Buff）
    float bonusAttack;
    float bonusDefense;
    float bonusSpeed;
    float bonusDodge;
    
    // 特殊属性
    float luck;             // 幸运值 (0-100)
    float damageBonus;      // 增伤百分比 (0-100+)
    float dodgeReduction;   // 减闪百分比 (0-100)
    
    // === 财产 ===
    int gold;
    
    // === 生活技能 ===
    SkillInfo farmingSkill;     // 种植
    SkillInfo fishingSkill;     // 渔业
    SkillInfo miningSkill;      // 采矿
    
    // === 计时器 ===
    float hungerDecayTimer;     // 饥饿度下降计时
    float staminaRegenTimer;    // 体力恢复计时
    float healthRegenTimer;     // 生命恢复计时
    
    // === 回调 ===
    StatsCallback onLevelUp;
    StatsCallback onHealthChange;
    StatsCallback onStaminaChange;
    StatsCallback onGoldChange;
    StatsCallback onSkillLevelUp;
    
    // === 常量配置 ===
    static constexpr float HUNGER_DECAY_RATE = 1.0f;        // 每秒饥饿度下降
    static constexpr float HUNGER_DECAY_INTERVAL = 60.0f;   // 饥饿下降间隔（秒）
    static constexpr float STAMINA_REGEN_RATE = 5.0f;       // 每秒体力恢复
    static constexpr float STAMINA_REGEN_INTERVAL = 1.0f;   // 体力恢复间隔
    static constexpr float HEALTH_REGEN_RATE = 1.0f;        // 每秒生命恢复
    static constexpr float HEALTH_REGEN_INTERVAL = 3.0f;    // 生命恢复间隔
    static constexpr float STARVING_DAMAGE = 5.0f;          // 饥饿时每次伤害
};