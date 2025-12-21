#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <random>
#include <functional>
#include <memory>
#include <cmath>

// ============================================================================
// 宠物系统 (Pet System)
// 
// 设计规则：
// ============================================================================
// 
// 【宠物资质 PetQuality】
//   - Mediocre:   平庸 60%
//   - Good:       良好 20%
//   - Excellent:  优秀 10%
//   - Outstanding:卓越 8%
//   - Rare:       稀有 2%
//
// 【洗点资质概率（洗涤剂）】
//   - Mediocre:   平庸 50%
//   - Good:       良好 24%
//   - Excellent:  优秀 13%
//   - Outstanding:卓越 10%
//   - Rare:       稀有 3%
//
// 【宠物属性】
//   - petId:      宠物ID
//   - name:       宠物名称
//   - quality:    资质
//   - level:      等级
//   - exp:        经验值
//   - health:     生命值
//   - sp:         SP值（技能点）
//   - attack:     攻击力
//   - defense:    防御力
//   - dodge:      闪避
//
// 【宠物技能】
//   - 技能栏
//   - 触发概率
//   - 效果描述
//
// ============================================================================

// 宠物资质枚举
enum class PetQuality {
    Mediocre,       // 平庸 60%
    Good,           // 良好 20%
    Excellent,      // 优秀 10%
    Outstanding,    // 卓越 8%
    Rare            // 稀有 2%
};

// 宠物动画状态
enum class PetAnimState {
    Idle,
    Follow,
    Attack
};

// 宠物朝向
enum class PetDirection {
    Down,
    Up,
    Left,
    Right
};

// 宠物技能结构
struct PetSkill {
    std::string id;
    std::string name;
    std::string description;
    float triggerChance;        // 触发概率 0-1
    float damageMultiplier;     // 伤害倍率
    float effectValue;          // 效果数值
    bool isPassive;             // 是否被动技能
    
    PetSkill(const std::string& skillId = "", const std::string& n = "",
             const std::string& desc = "", float chance = 0.0f, 
             float mult = 1.0f, float val = 0.0f, bool passive = false)
        : id(skillId), name(n), description(desc), 
          triggerChance(chance), damageMultiplier(mult), 
          effectValue(val), isPassive(passive) {}
};

// 宠物属性范围结构
struct PetStatRange {
    float minValue;
    float maxValue;
    
    PetStatRange(float min = 0, float max = 0) : minValue(min), maxValue(max) {}
    
    float roll(std::mt19937& rng) const {
        std::uniform_real_distribution<float> dist(minValue, maxValue);
        return dist(rng);
    }
};

// 孵化属性配置（按资质）
struct HatchConfig {
    PetStatRange health;
    PetStatRange attack;
    PetStatRange defense;
    PetStatRange dodge;
    
    // 技能获取概率
    std::vector<std::pair<std::string, float>> skillChances;
};

// 升级属性增加配置（按资质）
struct LevelUpConfig {
    PetStatRange health;
    PetStatRange attack;
    PetStatRange defense;
    PetStatRange dodge;
};

// ============================================================================
// 宠物基类
// ============================================================================
class Pet {
public:
    using PetCallback = std::function<void(Pet&)>;
    
    Pet();
    virtual ~Pet() = default;
    
    // ========================================
    // 核心接口（子类必须实现）
    // ========================================
    virtual void update(float dt, const sf::Vector2f& ownerPos, bool ownerAttacking) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
    virtual std::string getPetTypeName() const = 0;  // 宠物类型名（如"兔子"）
    virtual int getPetTypeId() const = 0;            // 宠物类型ID
    
    // ========================================
    // 初始化
    // ========================================
    virtual void init(float x, float y);
    virtual bool loadTexture(const std::string& texturePath);
    
    // 孵化宠物（根据资质初始化属性）
    virtual void hatch(PetQuality quality, int enhancerCount = 0);
    
    // 设置资质配置（子类调用）
    void setHatchConfig(PetQuality quality, const HatchConfig& config);
    void setLevelUpConfig(PetQuality quality, const LevelUpConfig& config);
    
    // ========================================
    // 等级经验系统
    // ========================================
    int getLevel() const { return level; }
    int getExp() const { return exp; }
    int getExpToNextLevel() const;
    float getExpPercent() const;
    void addExp(int amount);
    void levelUp();
    
    // ========================================
    // 战斗系统
    // ========================================
    
    // 执行攻击（返回伤害值）
    virtual float performAttack();
    
    // 检查是否触发技能
    bool rollSkill(int skillIndex) const;
    
    // 受到伤害
    virtual void takeDamage(float damage);
    
    // 恢复生命
    void heal(float amount);
    
    // 设置/获取战斗目标
    void setAttackTarget(const sf::Vector2f& target) { attackTarget = target; hasTarget = true; }
    void clearAttackTarget() { hasTarget = false; }
    bool hasAttackTarget() const { return hasTarget; }
    sf::Vector2f getAttackTarget() const { return attackTarget; }
    
    // 指定攻击目标（用于玩家指挥宠物攻击）
    void commandAttack(const sf::Vector2f& targetPos);
    bool hasCommandedTarget() const { return hasCommandTarget; }
    sf::Vector2f getCommandedTarget() const { return commandTargetPos; }
    void clearCommandedTarget() { hasCommandTarget = false; }
    
    // 检查是否刚完成一次攻击（用于伤害判定）
    bool hasJustAttacked() const { return justAttacked; }
    void clearJustAttacked() { justAttacked = false; }
    
    // 获取攻击范围
    float getAttackRange() const { return PET_ATTACK_RANGE; }
    
    // 获取宠物图标贴图（用于UI显示，返回精灵表的第一帧）
    virtual sf::IntRect getIconRect() const;
    const sf::Texture& getTexture() const { return texture; }
    
    // ========================================
    // 属性 Getters
    // ========================================
    std::string getName() const { return name; }
    PetQuality getQuality() const { return quality; }
    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    float getHealthPercent() const { return maxHealth > 0 ? health / maxHealth : 0; }
    float getSP() const { return sp; }
    float getMaxSP() const { return maxSP; }
    float getAttack() const { return attack; }
    float getDefense() const { return defense; }
    float getDodge() const { return dodge; }
    bool isDead() const { return health <= 0; }
    bool isAttacking() const { return animState == PetAnimState::Attack; }
    
    // 技能
    const std::vector<PetSkill>& getSkills() const { return skills; }
    bool hasSkill(const std::string& skillId) const;
    
    // ========================================
    // 属性 Setters
    // ========================================
    void setName(const std::string& n) { name = n; }
    void setQuality(PetQuality q) { quality = q; }
    
    // ========================================
    // 位置和移动
    // ========================================
    sf::Vector2f getPosition() const { return position; }
    void setPosition(float x, float y);
    void setPosition(const sf::Vector2f& pos);
    
    // ========================================
    // 碰撞检测
    // ========================================
    sf::FloatRect getBounds() const;
    virtual sf::FloatRect getCollisionBox() const;
    bool containsPoint(const sf::Vector2f& point) const;
    
    // ========================================
    // 洗点系统
    // ========================================
    // 洗点（重置等级为1，重新随机资质和属性）
    // playerLuck: 玩家幸运值 0-300
    void wash(float playerLuck);
    
    // ========================================
    // 回调
    // ========================================
    void setOnLevelUp(PetCallback cb) { onLevelUp = cb; }
    void setOnSkillTrigger(PetCallback cb) { onSkillTrigger = cb; }
    
    // ========================================
    // 静态工具函数
    // ========================================
    
    // 获取资质名称
    static std::string getQualityName(PetQuality quality);
    
    // 获取资质颜色
    static sf::Color getQualityColor(PetQuality quality);
    
    // 随机孵化资质（基础概率）
    static PetQuality rollHatchQuality(std::mt19937& rng, int enhancerCount = 0);
    
    // 随机洗点资质（受幸运值影响）
    static PetQuality rollWashQuality(std::mt19937& rng, float playerLuck);
    
    // 计算升级所需经验（分段指数型）
    static int calculateExpForLevel(int level);
    
    // 计算强化剂影响的概率
    static void getEnhancerProbabilities(int enhancerCount, 
        float& mediocre, float& good, float& excellent, 
        float& outstanding, float& rare);
    
    // 计算幸运值影响的洗点概率
    static void getLuckWashProbabilities(float luck,
        float& mediocre, float& good, float& excellent,
        float& outstanding, float& rare);

protected:
    // 更新精灵位置
    virtual void updateSprite();
    
    // 更新跟随AI
    virtual void updateFollowAI(float dt, const sf::Vector2f& ownerPos);
    
    // 更新攻击AI
    virtual void updateAttackAI(float dt, bool ownerAttacking);
    
    // 应用升级属性增加
    virtual void applyLevelUpStats();
    
    // 随机获取技能
    virtual void rollSkills();

protected:
    // === 基础属性 ===
    std::string name;
    PetQuality quality;
    int level;
    int exp;
    
    float health;
    float maxHealth;
    float sp;
    float maxSP;
    float attack;
    float defense;
    float dodge;
    
    // === 技能 ===
    std::vector<PetSkill> skills;
    int lastTriggeredSkillIndex;
    
    // === 位置和移动 ===
    sf::Vector2f position;
    sf::Vector2f size;
    sf::Vector2f velocity;
    sf::Vector2f targetOffset;      // 相对主人的目标偏移
    
    // === 跟随参数 ===
    float followDistance;           // 跟随距离
    float followSpeed;              // 跟随速度
    
    // === 动画状态 ===
    PetAnimState animState;
    PetDirection direction;
    
    // === 渲染 ===
    sf::Sprite sprite;
    sf::Texture texture;
    bool textureLoaded;
    
    // === 攻击状态 ===
    float attackCooldown;
    float lastAttackTime;
    bool hasAttackedThisCycle;
    bool justAttacked = false;           // 刚完成一次攻击（用于伤害判定）
    bool hasTarget = false;              // 是否有攻击目标
    sf::Vector2f attackTarget;           // 攻击目标位置
    
    // === 玩家指挥攻击 ===
    bool hasCommandTarget = false;       // 是否有玩家指定的攻击目标
    sf::Vector2f commandTargetPos;       // 玩家指定的目标位置
    
    // === 配置 ===
    std::map<PetQuality, HatchConfig> hatchConfigs;
    std::map<PetQuality, LevelUpConfig> levelUpConfigs;
    
    // === 回调 ===
    PetCallback onLevelUp;
    PetCallback onSkillTrigger;
    
    // === 随机数生成 ===
    mutable std::mt19937 rng;
    
    // === 常量 ===
    static constexpr float DEFAULT_FOLLOW_DISTANCE = 50.0f;
    static constexpr float DEFAULT_FOLLOW_SPEED = 120.0f;
    static constexpr float ATTACK_COOLDOWN_TIME = 1.5f;
    static constexpr float PET_ATTACK_RANGE = 60.0f;  // 宠物攻击范围
};

// ============================================================================
// 宠物管理器基类
// ============================================================================
class PetManagerBase {
public:
    PetManagerBase() = default;
    virtual ~PetManagerBase() = default;
    
    // 更新当前宠物
    virtual void update(float dt, const sf::Vector2f& ownerPos, bool ownerAttacking) = 0;
    
    // 渲染当前宠物
    virtual void render(sf::RenderWindow& window) = 0;
    
    // 获取当前宠物攻击伤害（如果正在攻击）
    virtual float getCurrentPetDamage() = 0;
    
    // 获取当前宠物
    virtual Pet* getCurrentPet() = 0;
    
    // 获取宠物数量
    virtual size_t getPetCount() const = 0;
};
