#include "Monster.h"
#include <iostream>
#include <algorithm>

// ============================================================================
// Monster 基类实现
// ============================================================================

Monster::Monster()
    : health(50.0f)
    , maxHealth(50.0f)
    , defense(3.0f)
    , attack(5.0f)
    , dodge(1)
    , level(1)
    , healthMin(30.0f), healthMax(70.0f)
    , defenseMin(1.0f), defenseMax(5.0f)
    , attackMin(3.0f), attackMax(8.0f)
    , dodgeMin(0), dodgeMax(5)
    , expMin(10), expMax(20)
    , goldMin(5), goldMax(15)
    , lastAttackUsedSkill(false)
    , position(0.0f, 0.0f)
    , size(32.0f, 32.0f)
    , velocity(0.0f, 0.0f)
    , homePosition(0.0f, 0.0f)
    , aiState(MonsterAIState::Idle)
    , direction(MonsterDirection::Down)
    , isAggroed(false)
    , aggroTimer(0.0f)
    , attackCooldown(0.0f)
    , lastAttackTime(0.0f)
    , textureLoaded(false)
    , isHovered(false)
    , moveSpeed(40.0f)
    , chaseSpeed(70.0f)
    , returnSpeed(50.0f)
    , aggroDuration(8.0f)
    , attackRange(35.0f)
    , attackCooldownTime(1.2f)
    , chaseRange(200.0f)
    , leashRange(300.0f)
    , rng(std::random_device{}())
{
}

void Monster::init(float x, float y) {
    position = sf::Vector2f(x, y);
    homePosition = position;
    randomizeStats();
}

bool Monster::loadTexture(const std::string& texturePath) {
    if (texture.loadFromFile(texturePath)) {
        sprite.setTexture(texture);
        textureLoaded = true;
        return true;
    }
    
    std::cerr << "[Monster] Failed to load texture: " << texturePath << std::endl;
    return false;
}

void Monster::randomizeStats() {
    std::uniform_real_distribution<float> healthDist(healthMin, healthMax);
    std::uniform_real_distribution<float> defenseDist(defenseMin, defenseMax);
    std::uniform_real_distribution<float> attackDist(attackMin, attackMax);
    std::uniform_int_distribution<int> dodgeDist(dodgeMin, dodgeMax);
    
    maxHealth = healthDist(rng);
    health = maxHealth;
    defense = defenseDist(rng);
    attack = attackDist(rng);
    dodge = dodgeDist(rng);
}

// ========================================
// 战斗系统
// ========================================

bool Monster::takeDamage(float damage, bool ignoreDefense) {
    // 闪避判定
    if (rollDodge()) {
        std::cout << "[" << getTypeName() << "] 闪避了攻击!" << std::endl;
        return false;
    }
    
    // 计算实际伤害
    float actualDamage = damage;
    if (!ignoreDefense) {
        actualDamage = std::max(1.0f, damage - defense);
    }
    
    health -= actualDamage;
    health = std::max(0.0f, health);
    
    std::cout << "[" << getTypeName() << "] 受到 " << actualDamage << " 点伤害, 剩余HP: " 
              << health << "/" << maxHealth << std::endl;
    
    // 被攻击后激怒
    if (!isDead()) {
        aggro(aggroDuration);
    } else {
        if (onDeath) {
            onDeath(*this);
        }
    }
    
    return isDead();
}

float Monster::performAttack() {
    float damage = attack;
    
    // 检查是否触发技能
    if (rollSkill()) {
        damage *= skill.damageMultiplier;
        lastAttackUsedSkill = true;
        std::cout << "[" << getTypeName() << "] 使用技能 [" << skill.name << "]!" << std::endl;
    } else {
        lastAttackUsedSkill = false;
    }
    
    attackCooldown = attackCooldownTime;
    
    if (onAttack) {
        onAttack(*this);
    }
    
    return damage;
}

bool Monster::rollSkill() const {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(rng) < skill.triggerChance;
}

float Monster::getSkillMultiplier() const {
    return skill.damageMultiplier;
}

bool Monster::rollDodge() const {
    // 闪避几率 = 闪避值 * 0.5%
    float dodgeChance = dodge * 0.005f;
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(rng) < dodgeChance;
}

void Monster::aggro(float duration) {
    isAggroed = true;
    aggroTimer = duration;
    
    if (aiState == MonsterAIState::Idle || aiState == MonsterAIState::Wandering) {
        aiState = MonsterAIState::Chasing;
    }
}

// ========================================
// 碰撞检测
// ========================================

bool Monster::containsPoint(const sf::Vector2f& point) const {
    return getBounds().contains(point);
}

bool Monster::intersects(const sf::FloatRect& rect) const {
    return getBounds().intersects(rect);
}

sf::FloatRect Monster::getBounds() const {
    sf::FloatRect spriteBounds = sprite.getGlobalBounds();
    if (spriteBounds.width > 0 && spriteBounds.height > 0) {
        return spriteBounds;
    }
    return sf::FloatRect(position.x, position.y, size.x, size.y);
}

sf::FloatRect Monster::getCollisionBox() const {
    sf::FloatRect bounds = getBounds();
    // 碰撞盒比视觉边界稍小
    float shrink = 0.2f;
    return sf::FloatRect(
        bounds.left + bounds.width * shrink / 2,
        bounds.top + bounds.height * shrink,
        bounds.width * (1 - shrink),
        bounds.height * (1 - shrink)
    );
}

// ========================================
// 位置和移动
// ========================================

void Monster::setPosition(float x, float y) {
    position = sf::Vector2f(x, y);
    updateSprite();
}

void Monster::setPosition(const sf::Vector2f& pos) {
    position = pos;
    updateSprite();
}

void Monster::applyPush(const sf::Vector2f& pushVector) {
    position += pushVector;
    updateSprite();
}

void Monster::updateDirectionFromVelocity() {
    if (std::abs(velocity.x) > std::abs(velocity.y)) {
        direction = (velocity.x > 0) ? MonsterDirection::Right : MonsterDirection::Left;
    } else if (std::abs(velocity.y) > 0.001f) {
        direction = (velocity.y > 0) ? MonsterDirection::Down : MonsterDirection::Up;
    }
}

void Monster::updateSprite() {
    sprite.setPosition(position);
}

// ========================================
// 掉落奖励
// ========================================

std::vector<std::pair<std::string, int>> Monster::generateDrops() const {
    std::vector<std::pair<std::string, int>> result;
    
    for (const auto& drop : drops) {
        std::uniform_real_distribution<float> chanceDist(0.0f, 1.0f);
        
        if (chanceDist(rng) <= drop.dropChance) {
            std::uniform_int_distribution<int> countDist(drop.minCount, drop.maxCount);
            int count = countDist(rng);
            
            if (count > 0) {
                result.push_back({drop.itemId, count});
            }
        }
    }
    
    return result;
}

int Monster::getExpReward() const {
    std::uniform_int_distribution<int> dist(expMin, expMax);
    return dist(rng);
}

int Monster::getGoldReward() const {
    std::uniform_int_distribution<int> dist(goldMin, goldMax);
    return dist(rng);
}
