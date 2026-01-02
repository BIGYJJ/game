#include "Pet.h"
#include <algorithm>
#include <iostream>

// ============================================================================
// 构造函数
// ============================================================================
Pet::Pet()
    : name("宠物")
    , quality(PetQuality::Mediocre)
    , level(1)
    , exp(0)
    , health(10)
    , maxHealth(10)
    , sp(10)
    , maxSP(10)
    , attack(1)
    , defense(1)
    , dodge(0)
    , lastTriggeredSkillIndex(-1)
    , position(0, 0)
    , size(32, 32)
    , velocity(0, 0)
    , targetOffset(-40, 0)
    , followDistance(DEFAULT_FOLLOW_DISTANCE)
    , followSpeed(DEFAULT_FOLLOW_SPEED)
    , animState(PetAnimState::Idle)
    , direction(PetDirection::Right)
    , textureLoaded(false)
    , attackCooldown(0)
    , lastAttackTime(0)
    , hasAttackedThisCycle(false)
{
    rng.seed(std::random_device{}());
}

// ============================================================================
// 初始化
// ============================================================================
void Pet::init(float x, float y) {
    position = sf::Vector2f(x, y);
    sprite.setPosition(position);
}

bool Pet::loadTexture(const std::string& texturePath) {
    if (!texture.loadFromFile(texturePath)) {
        std::cerr << "Pet: 无法加载贴图 " << texturePath << std::endl;
        texture.create(32, 32);
        textureLoaded = false;
        return false;
    }
    sprite.setTexture(texture);
    textureLoaded = true;
    return true;
}

void Pet::setHatchConfig(PetQuality q, const HatchConfig& config) {
    hatchConfigs[q] = config;
}

void Pet::setLevelUpConfig(PetQuality q, const LevelUpConfig& config) {
    levelUpConfigs[q] = config;
}

// ============================================================================
// 孵化
// ============================================================================
void Pet::hatch(PetQuality q, int enhancerCount) {
    quality = q;
    level = 1;
    exp = 0;
    
    auto it = hatchConfigs.find(quality);
    if (it != hatchConfigs.end()) {
        const HatchConfig& config = it->second;
        
        // 随机属性
        maxHealth = config.health.roll(rng);
        health = maxHealth;
        attack = config.attack.roll(rng);
        defense = config.defense.roll(rng);
        dodge = config.dodge.roll(rng);
        
        // 随机技能
        skills.clear();
        rollSkills();
    }
    
    std::cout << "宠物孵化成功! 资质: " << getQualityName(quality) << std::endl;
    std::cout << "  生命: " << maxHealth << " 攻击: " << attack 
              << " 防御: " << defense << " 闪避: " << dodge << std::endl;
}

void Pet::rollSkills() {
    auto it = hatchConfigs.find(quality);
    if (it == hatchConfigs.end()) return;
    
    const HatchConfig& config = it->second;
    
    for (const auto& skillChance : config.skillChances) {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        if (dist(rng) < skillChance.second) {
            // 子类应该重写此方法以添加具体技能
        }
    }
}

// ============================================================================
// 等级经验系统
// ============================================================================
int Pet::getExpToNextLevel() const {
    return calculateExpForLevel(level);
}

float Pet::getExpPercent() const {
    int needed = getExpToNextLevel();
    return needed > 0 ? (float)exp / needed : 0;
}

void Pet::addExp(int amount) {
    exp += amount;
    
    while (exp >= getExpToNextLevel()) {
        exp -= getExpToNextLevel();
        levelUp();
    }
}

void Pet::levelUp() {
    level++;
    applyLevelUpStats();
    
    std::cout << name << " 升级了! 当前等级: " << level << std::endl;
    
    if (onLevelUp) {
        onLevelUp(*this);
    }
}

void Pet::applyLevelUpStats() {
    auto it = levelUpConfigs.find(quality);
    if (it != levelUpConfigs.end()) {
        const LevelUpConfig& config = it->second;
        
        float healthIncrease = config.health.roll(rng);
        float attackIncrease = config.attack.roll(rng);
        float defenseIncrease = config.defense.roll(rng);
        float dodgeIncrease = config.dodge.roll(rng);
        
        maxHealth += healthIncrease;
        health += healthIncrease;
        attack += attackIncrease;
        defense += defenseIncrease;
        dodge += dodgeIncrease;
        
        std::cout << "  属性增加 - 生命: +" << healthIncrease 
                  << " 攻击: +" << attackIncrease
                  << " 防御: +" << defenseIncrease
                  << " 闪避: +" << dodgeIncrease << std::endl;
    }
}

// ============================================================================
// 战斗系统
// ============================================================================
float Pet::performAttack() {
    float damage = attack;
    
    // 检查技能触发
    for (size_t i = 0; i < skills.size(); i++) {
        if (!skills[i].isPassive && rollSkill(i)) {
            damage *= skills[i].damageMultiplier;
            lastTriggeredSkillIndex = i;
            
            std::cout << name << " 触发技能: " << skills[i].name << std::endl;
            
            if (onSkillTrigger) {
                onSkillTrigger(*this);
            }
            break;
        }
    }
    
    return damage;
}

bool Pet::rollSkill(int skillIndex) const {
    if (skillIndex < 0 || skillIndex >= (int)skills.size()) return false;
    
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(rng) < skills[skillIndex].triggerChance;
}

void Pet::takeDamage(float damage) {
    float actualDamage = std::max(0.0f, damage - defense * 0.5f);
    health = std::max(0.0f, health - actualDamage);
    
    if (isDead()) {
        std::cout << name << " 倒下了!" << std::endl;
    }
}

void Pet::heal(float amount) {
    health = std::min(maxHealth, health + amount);
}

bool Pet::hasSkill(const std::string& skillId) const {
    for (const auto& skill : skills) {
        if (skill.id == skillId) return true;
    }
    return false;
}

// ============================================================================
// 位置和碰撞
// ============================================================================
void Pet::setPosition(float x, float y) {
    position = sf::Vector2f(x, y);
    updateSprite();
}

void Pet::setPosition(const sf::Vector2f& pos) {
    position = pos;
    updateSprite();
}

sf::FloatRect Pet::getBounds() const {
    return sprite.getGlobalBounds();
}

sf::FloatRect Pet::getCollisionBox() const {
    sf::FloatRect bounds = getBounds();
    float shrink = 0.3f;
    return sf::FloatRect(
        bounds.left + bounds.width * shrink / 2,
        bounds.top + bounds.height * shrink,
        bounds.width * (1 - shrink),
        bounds.height * (1 - shrink)
    );
}

bool Pet::containsPoint(const sf::Vector2f& point) const {
    return getBounds().contains(point);
}

void Pet::updateSprite() {
    sprite.setPosition(position);
}

// ============================================================================
// AI更新
// ============================================================================
void Pet::updateFollowAI(float dt, const sf::Vector2f& ownerPos) {
    // 计算目标位置（主人位置 + 偏移）
    sf::Vector2f targetPos = ownerPos + targetOffset;
    
    // 计算到目标的距离
    sf::Vector2f diff = targetPos - position;
    float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    
    if (distance > followDistance * 0.5f) {
        // 需要移动
        animState = PetAnimState::Follow;
        
        // 归一化方向
        sf::Vector2f dir = diff / distance;
        
        // 计算速度（距离越远速度越快）
        float speedMult = std::min(2.0f, distance / followDistance);
        velocity = dir * followSpeed * speedMult;
        
        // 更新位置
        position += velocity * dt;
        
        // 更新朝向
        if (std::abs(diff.x) > std::abs(diff.y)) {
            direction = diff.x > 0 ? PetDirection::Right : PetDirection::Left;
        } else {
            direction = diff.y > 0 ? PetDirection::Down : PetDirection::Up;
        }
    } else {
        // 在跟随范围内，保持空闲
        if (animState != PetAnimState::Attack) {
            animState = PetAnimState::Idle;
        }
        velocity = sf::Vector2f(0, 0);
    }
}

void Pet::updateAttackAI(float dt, bool ownerAttacking) {
    
    

    // 更新攻击冷却
    if (attackCooldown > 0) {
        attackCooldown -= dt;
    }
    
    // 重置justAttacked标志（每帧开始时）
    justAttacked = false;
    
    // 判定是否应该攻击
    // 条件：(主人在攻击 OR 有目标 OR 处于战斗模式) AND 冷却完毕 AND 本帧还没打
    // 注意：我们将 "combatTimer > 0" 加入判定，让它更主动
    bool shouldAttack = (ownerAttacking || hasTarget || combatTimer > 0) 
                        && attackCooldown <= 0 
                        && !hasAttackedThisCycle;
    // 如果有玩家指挥的目标，优先处理
    if (hasCommandTarget) {
        // 移动到目标位置附近
        sf::Vector2f diff = commandTargetPos - position;
        float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        
        if (distance <= PET_ATTACK_RANGE && attackCooldown <= 0) {
            animState = PetAnimState::Attack;
            justAttacked = true;
            attackCooldown = ATTACK_COOLDOWN_TIME;
            
            performAttack(); // 播放动画，伤害在 GameState 处理
            
            // [修改] 不要立即清除 hasCommandTarget，让它持续攻击直到玩家取消或怪物死掉
            // hasCommandTarget = false; <--- 删除这行
            
            // 攻击后激活战斗模式，这样就算 commandTarget 清除了，它也会继续打附近的怪
            enterCombatMode(3.0f); 
        }
        return;
    }
    
    if (shouldAttack) {
        // 只有当真正有目标（由 GameState 设置 setAttackTarget）或者主人在攻击时才挥动
        // 或者是为了配合动画
        animState = PetAnimState::Attack;
        hasAttackedThisCycle = true;
        justAttacked = true;  
        attackCooldown = ATTACK_COOLDOWN_TIME;
        
        // 执行攻击
        performAttack();
        // std::cout << name << " 攻击!" << std::endl; // 调试用
        
        // 攻击一次，续杯战斗时间
        enterCombatMode(3.0f);
    }
    
    // 主人不再攻击且没有目标时重置 -> 改为：战斗时间结束才重置
    if (!ownerAttacking && !hasTarget && combatTimer <= 0) {
        hasAttackedThisCycle = false;
        if (animState == PetAnimState::Attack) {
            animState = PetAnimState::Idle;
        }
    }
    
}

// 指挥宠物攻击指定目标
void Pet::commandAttack(const sf::Vector2f& targetPos) {
    hasCommandTarget = true;
    commandTargetPos = targetPos;
    std::cout << name << " 收到攻击指令，目标位置: (" << targetPos.x << ", " << targetPos.y << ")" << std::endl;
}

// 获取宠物图标矩形（精灵表第一帧）
sf::IntRect Pet::getIconRect() const {
    // 默认返回32x32的第一帧
    return sf::IntRect(0, 0, 32, 32);
}

// ============================================================================
// 洗点系统
// ============================================================================
void Pet::wash(float playerLuck) {
    // 重置等级
    level = 1;
    exp = 0;
    
    // 根据幸运值随机新资质
    PetQuality newQuality = rollWashQuality(rng, playerLuck);
    
    std::cout << "宠物洗点! 原资质: " << getQualityName(quality) 
              << " -> 新资质: " << getQualityName(newQuality) << std::endl;
    
    // 重新孵化（使用新资质）
    hatch(newQuality, 0);
}

// ============================================================================
// 静态工具函数
// ============================================================================
std::string Pet::getQualityName(PetQuality quality) {
    switch (quality) {
        case PetQuality::Mediocre:    return "平庸";
        case PetQuality::Good:        return "良好";
        case PetQuality::Excellent:   return "优秀";
        case PetQuality::Outstanding: return "卓越";
        case PetQuality::Rare:        return "稀有";
        default:                      return "未知";
    }
}

sf::Color Pet::getQualityColor(PetQuality quality) {
    switch (quality) {
        case PetQuality::Mediocre:    return sf::Color(180, 180, 180);     // 灰色
        case PetQuality::Good:        return sf::Color(100, 200, 100);     // 绿色
        case PetQuality::Excellent:   return sf::Color(100, 150, 255);     // 蓝色
        case PetQuality::Outstanding: return sf::Color(200, 100, 255);     // 紫色
        case PetQuality::Rare:        return sf::Color(255, 180, 50);      // 橙色
        default:                      return sf::Color::White;
    }
}

void Pet::getEnhancerProbabilities(int enhancerCount,
    float& mediocre, float& good, float& excellent,
    float& outstanding, float& rare) {
    
    // 限制强化剂数量 0-100
    int n = std::max(0, std::min(100, enhancerCount));
    
    // 根据附表的线性公式计算
    // 平庸: 60% -> 0%
    mediocre = 0.60f - (0.60f * n / 100.0f);
    // 良好: 20% -> 30%
    good = 0.20f + (0.10f * n / 100.0f);
    // 优秀: 10% -> 25%
    excellent = 0.10f + (0.15f * n / 100.0f);
    // 卓越: 8% -> 25%
    outstanding = 0.08f + (0.17f * n / 100.0f);
    // 稀有: 2% -> 20%
    rare = 0.02f + (0.18f * n / 100.0f);
}

void Pet::getLuckWashProbabilities(float luck,
    float& mediocre, float& good, float& excellent,
    float& outstanding, float& rare) {
    
    // 限制幸运值 0-300
    float L = std::max(0.0f, std::min(300.0f, luck));
    
    // 根据附表的线性公式计算
    // 平庸: 50% -> 5%
    mediocre = 0.50f - (0.45f * L / 300.0f);
    // 良好: 24% -> 30%
    good = 0.24f + (0.06f * L / 300.0f);
    // 优秀: 13% -> 25%
    excellent = 0.13f + (0.12f * L / 300.0f);
    // 卓越: 10% -> 25%
    outstanding = 0.10f + (0.15f * L / 300.0f);
    // 稀有: 3% -> 15%
    rare = 0.03f + (0.12f * L / 300.0f);
}

PetQuality Pet::rollHatchQuality(std::mt19937& rng, int enhancerCount) {
    float mediocre, good, excellent, outstanding, rare;
    getEnhancerProbabilities(enhancerCount, mediocre, good, excellent, outstanding, rare);
    
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float roll = dist(rng);
    
    if (roll < rare) return PetQuality::Rare;
    roll -= rare;
    if (roll < outstanding) return PetQuality::Outstanding;
    roll -= outstanding;
    if (roll < excellent) return PetQuality::Excellent;
    roll -= excellent;
    if (roll < good) return PetQuality::Good;
    return PetQuality::Mediocre;
}

PetQuality Pet::rollWashQuality(std::mt19937& rng, float playerLuck) {
    float mediocre, good, excellent, outstanding, rare;
    getLuckWashProbabilities(playerLuck, mediocre, good, excellent, outstanding, rare);
    
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float roll = dist(rng);
    
    if (roll < rare) return PetQuality::Rare;
    roll -= rare;
    if (roll < outstanding) return PetQuality::Outstanding;
    roll -= outstanding;
    if (roll < excellent) return PetQuality::Excellent;
    roll -= excellent;
    if (roll < good) return PetQuality::Good;
    return PetQuality::Mediocre;
}

int Pet::calculateExpForLevel(int lvl) {
    // 分段指数型公式
    if (lvl < 1) return 50;
    
    if (lvl <= 20) {
        // 1-20级: E_n = 50 × 1.12^(n-1)
        return static_cast<int>(50 * std::pow(1.12, lvl - 1));
    } else if (lvl <= 50) {
        // 21-50级: E_n = E_20 × 1.25^(n-20)
        int e20 = static_cast<int>(50 * std::pow(1.12, 19));
        return static_cast<int>(e20 * std::pow(1.25, lvl - 20));
    } else {
        // 51级及以上: E_n = E_50 × 1.4^(n-50)
        int e20 = static_cast<int>(50 * std::pow(1.12, 19));
        int e50 = static_cast<int>(e20 * std::pow(1.25, 30));
        return static_cast<int>(e50 * std::pow(1.4, lvl - 50));
    }
}
