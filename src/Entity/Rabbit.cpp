#include "Rabbit.h"
#include <iostream>
#include <algorithm>
#include <cmath>

// ============================================================================
// Rabbit 实现
// ============================================================================

Rabbit::Rabbit()
    : health(40.0f)
    , maxHealth(40.0f)
    , defense(3.0f)
    , attack(5.0f)
    , dodge(1)
    , level(1)
    , expMin(10)
    , expMax(20)
    , goldMin(10)
    , goldMax(30)
    , skill("bite", "撕咬", "凶猛撕咬，造成双倍伤害", 2.0f, 0.10f, sf::Color(255, 80, 80))
    , lastAttackUsedSkill(false)
    , position(0.0f, 0.0f)
    , size(32.0f, 32.0f)
    , velocity(0.0f, 0.0f)
    , homePosition(0.0f, 0.0f)
    , aiState(RabbitAIState::Idle)
    , direction(RabbitDirection::Down)
    , isAggroed(false)
    , aggroTimer(0.0f)
    , wanderTimer(0.0f)
    , wanderDuration(0.0f)
    , idleTimer(0.0f)
    , attackCooldown(0.0f)
    , lastAttackTime(0.0f)
    , textureLoaded(false)
    , animState(RabbitAnimState::MoveDown)
    , lastAnimState(RabbitAnimState::MoveDown)
    , currentFrame(0)
    , animTimer(0.0f)
    , frameTime(0.15f)
    , animationLocked(false)
    , animLockTimer(0.0f)
    , isHovered(false)
    , rng(std::random_device{}())
{
    // 设置掉落物品
    drops.push_back(RabbitDrop("rabbit_fur", "兔毛", 1, 2, 0.30f));
    drops.push_back(RabbitDrop("carrot", "胡萝卜", 1, 2, 0.20f));
    drops.push_back(RabbitDrop("rabbit_meat", "兔肉", 1, 1, 0.10f));
}

Rabbit::Rabbit(float x, float y) : Rabbit() {
    init(x, y);
}

void Rabbit::init(float x, float y) {
    position = sf::Vector2f(x, y);
    homePosition = position;  // 记录初始位置
    randomizeStats();
}

bool Rabbit::loadTexture(const std::string& texturePath) {
    if (texture.loadFromFile(texturePath)) {
        sprite.setTexture(texture);
        sprite.setTextureRect(getFrameRect(ROW_MOVE_DOWN, 0));
        sprite.setScale(2.0f, 2.0f);  // 放大精灵
        updateSprite();
        textureLoaded = true;
        std::cout << "[Rabbit] Texture loaded: " << texturePath << std::endl;
        return true;
    }
    
    std::cerr << "[Rabbit] Failed to load texture: " << texturePath << std::endl;
    return false;
}

void Rabbit::randomizeStats() {
    std::uniform_int_distribution<int> healthDist(30, 50);
    std::uniform_int_distribution<int> defenseDist(1, 5);
    std::uniform_int_distribution<int> attackDist(3, 8);
    std::uniform_int_distribution<int> dodgeDist(0, 2);
    
    maxHealth = static_cast<float>(healthDist(rng));
    health = maxHealth;
    defense = static_cast<float>(defenseDist(rng));
    attack = static_cast<float>(attackDist(rng));
    dodge = dodgeDist(rng);
}

void Rabbit::update(float dt, const sf::Vector2f& playerPos) {
    // 更新攻击冷却
    if (attackCooldown > 0) {
        attackCooldown -= dt;
    }
    
    // 更新动画锁定计时器
    if (animationLocked) {
        animLockTimer -= dt;
        if (animLockTimer <= 0) {
            animationLocked = false;
        }
    }
    
    // 更新AI
    updateAI(dt, playerPos);
    
    // 更新位置
    position += velocity * dt;
    
    // 更新动画
    updateAnimation(dt);
    
    // 更新精灵位置
    updateSprite();
    
    // 更新激怒状态
    if (isAggroed) {
        aggroTimer -= dt;
        if (aggroTimer <= 0) {
            isAggroed = false;
            // 返回原位
            if (aiState == RabbitAIState::Chasing || aiState == RabbitAIState::Attacking) {
                aiState = RabbitAIState::Returning;
            }
        }
    }
}

void Rabbit::updateAI(float dt, const sf::Vector2f& playerPos) {
    // 计算到玩家的距离
    sf::Vector2f toPlayer = playerPos - position;
    float distToPlayer = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
    
    // 计算到家的距离
    sf::Vector2f toHome = homePosition - position;
    float distToHome = std::sqrt(toHome.x * toHome.x + toHome.y * toHome.y);
    
    switch (aiState) {
        case RabbitAIState::Idle:
            velocity = sf::Vector2f(0, 0);
            idleTimer += dt;
            
            // 设置待机动画（基于当前朝向）
            switch (direction) {
                case RabbitDirection::Down: setAnimState(RabbitAnimState::MoveDown); break;
                case RabbitDirection::Up: setAnimState(RabbitAnimState::MoveUp); break;
                case RabbitDirection::Left: setAnimState(RabbitAnimState::MoveLeft); break;
                case RabbitDirection::Right: setAnimState(RabbitAnimState::MoveRight); break;
            }
            
            // 随机开始游荡
            if (idleTimer > 2.0f) {
                std::uniform_real_distribution<float> dist(0.0f, 1.0f);
                if (dist(rng) < 0.3f) {
                    aiState = RabbitAIState::Wandering;
                    idleTimer = 0;
                    
                    // 随机选择方向
                    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
                    float angle = angleDist(rng);
                    velocity.x = std::cos(angle) * MOVE_SPEED;
                    velocity.y = std::sin(angle) * MOVE_SPEED;
                    
                    std::uniform_real_distribution<float> durationDist(1.0f, 3.0f);
                    wanderDuration = durationDist(rng);
                    wanderTimer = 0;
                    
                    updateDirectionFromVelocity();
                }
                idleTimer = 0;
            }
            break;
            
        case RabbitAIState::Wandering:
            wanderTimer += dt;
            
            // 更新朝向和动画
            updateDirectionFromVelocity();
            
            if (wanderTimer >= wanderDuration) {
                aiState = RabbitAIState::Idle;
                velocity = sf::Vector2f(0, 0);
            }
            break;
            
        case RabbitAIState::Chasing:
            // 检查是否超出牵引范围
            if (distToHome > LEASH_RANGE) {
                isAggroed = false;
                aiState = RabbitAIState::Returning;
                break;
            }
            
            // 追击玩家
            if (distToPlayer > ATTACK_RANGE) {
                // 还没到攻击范围，继续追
                if (distToPlayer > 0) {
                    sf::Vector2f chaseDir = toPlayer / distToPlayer;
                    velocity = chaseDir * CHASE_SPEED;
                    updateDirectionFromVelocity();
                }
            } else {
                // 进入攻击范围
                aiState = RabbitAIState::Attacking;
                velocity = sf::Vector2f(0, 0);
            }
            break;
            
        case RabbitAIState::Attacking:
            // 检查是否超出牵引范围
            if (distToHome > LEASH_RANGE) {
                isAggroed = false;
                aiState = RabbitAIState::Returning;
                break;
            }
            
            // 更新朝向朝向玩家
            if (std::abs(toPlayer.x) > std::abs(toPlayer.y)) {
                direction = (toPlayer.x > 0) ? RabbitDirection::Right : RabbitDirection::Left;
            } else {
                direction = (toPlayer.y > 0) ? RabbitDirection::Down : RabbitDirection::Up;
            }
            
            // 如果玩家跑出攻击范围，继续追
            if (distToPlayer > ATTACK_RANGE * 1.5f) {
                aiState = RabbitAIState::Chasing;
                break;
            }
            
            // 设置攻击动画
            switch (direction) {
                case RabbitDirection::Down: setAnimState(RabbitAnimState::AttackDown); break;
                case RabbitDirection::Up: setAnimState(RabbitAnimState::AttackUp); break;
                case RabbitDirection::Left: setAnimState(RabbitAnimState::AttackLeft); break;
                case RabbitDirection::Right: setAnimState(RabbitAnimState::AttackRight); break;
            }
            
            // 执行攻击
            if (attackCooldown <= 0) {
                if (onAttack) {
                    onAttack(*this);
                }
                attackCooldown = ATTACK_COOLDOWN;
            }
            
            velocity = sf::Vector2f(0, 0);
            break;
            
        case RabbitAIState::Returning:
            // 返回家的位置
            if (distToHome > 10.0f) {
                sf::Vector2f returnDir = toHome / distToHome;
                velocity = returnDir * RETURN_SPEED;
                updateDirectionFromVelocity();
            } else {
                // 到家了
                velocity = sf::Vector2f(0, 0);
                aiState = RabbitAIState::Idle;
            }
            break;
    }
}

void Rabbit::updateDirectionFromVelocity() {
    if (velocity.x == 0 && velocity.y == 0) return;
    
    // 根据速度方向更新朝向
    if (std::abs(velocity.x) > std::abs(velocity.y)) {
        direction = (velocity.x > 0) ? RabbitDirection::Right : RabbitDirection::Left;
    } else {
        direction = (velocity.y > 0) ? RabbitDirection::Down : RabbitDirection::Up;
    }
    
    // 设置移动动画
    switch (direction) {
        case RabbitDirection::Down: setAnimState(RabbitAnimState::MoveDown); break;
        case RabbitDirection::Up: setAnimState(RabbitAnimState::MoveUp); break;
        case RabbitDirection::Left: setAnimState(RabbitAnimState::MoveLeft); break;
        case RabbitDirection::Right: setAnimState(RabbitAnimState::MoveRight); break;
    }
}

void Rabbit::updateAnimation(float dt) {
    animTimer += dt;
    
    if (animTimer >= frameTime) {
        animTimer -= frameTime;  // 减去而不是清零，保持平滑
        currentFrame = (currentFrame + 1) % FRAMES_PER_ROW;
        
        // 更新精灵贴图
        int row = 0;
        switch (animState) {
            case RabbitAnimState::MoveDown: row = ROW_MOVE_DOWN; break;
            case RabbitAnimState::MoveUp: row = ROW_MOVE_UP; break;
            case RabbitAnimState::MoveLeft: row = ROW_MOVE_LEFT; break;
            case RabbitAnimState::MoveRight: row = ROW_MOVE_RIGHT; break;
            case RabbitAnimState::AttackDown: row = ROW_ATTACK_DOWN; break;
            case RabbitAnimState::AttackUp: row = ROW_ATTACK_UP; break;
            case RabbitAnimState::AttackLeft: row = ROW_ATTACK_LEFT; break;
            case RabbitAnimState::AttackRight: row = ROW_ATTACK_RIGHT; break;
        }
        
        sprite.setTextureRect(getFrameRect(row, currentFrame));
    }
}

void Rabbit::setAnimState(RabbitAnimState state) {
    // 如果动画被锁定，不允许切换（除非是攻击动画）
    if (animationLocked && 
        state != RabbitAnimState::AttackDown &&
        state != RabbitAnimState::AttackUp &&
        state != RabbitAnimState::AttackLeft &&
        state != RabbitAnimState::AttackRight) {
        return;
    }
    
    if (animState != state) {
        lastAnimState = animState;
        animState = state;
        currentFrame = 0;
        animTimer = 0;
        
        // 设置动画锁定，防止快速切换导致闪烁
        animationLocked = true;
        animLockTimer = MIN_ANIM_DURATION;
    }
}

void Rabbit::updateSprite() {
    sprite.setPosition(position);
}

sf::IntRect Rabbit::getFrameRect(int row, int col) const {
    return sf::IntRect(
        col * FRAME_WIDTH,
        row * FRAME_HEIGHT,
        FRAME_WIDTH,
        FRAME_HEIGHT
    );
}

void Rabbit::render(sf::RenderWindow& window) {
    if (textureLoaded && !isDead()) {
        window.draw(sprite);
        
        // 绘制生命条
        float barWidth = 40.0f;
        float barHeight = 4.0f;
        float barX = position.x + (size.x * 2 - barWidth) / 2;
        float barY = position.y - 10;
        
        // 背景
        sf::RectangleShape bgBar(sf::Vector2f(barWidth, barHeight));
        bgBar.setPosition(barX, barY);
        bgBar.setFillColor(sf::Color(40, 40, 40, 180));
        window.draw(bgBar);
        
        // 生命值
        float healthWidth = barWidth * getHealthPercent();
        sf::RectangleShape healthBar(sf::Vector2f(healthWidth, barHeight));
        healthBar.setPosition(barX, barY);
        healthBar.setFillColor(sf::Color(220, 60, 60));
        window.draw(healthBar);
        
        // 如果被激怒，显示愤怒标记
        if (isAggroed) {
            sf::CircleShape aggroIndicator(5);
            aggroIndicator.setFillColor(sf::Color(255, 100, 100));
            aggroIndicator.setPosition(barX + barWidth + 5, barY);
            window.draw(aggroIndicator);
        }
        
        // 调试：显示AI状态
        /*
        sf::Text stateText;
        stateText.setString(std::to_string(static_cast<int>(aiState)));
        stateText.setPosition(position.x, position.y - 20);
        stateText.setCharacterSize(10);
        window.draw(stateText);
        */
    }
}

bool Rabbit::takeDamage(float damage, bool ignoreDefense) {
    // 闪避判定
    if (rollDodge()) {
        std::cout << "[Rabbit] Dodged attack!" << std::endl;
        return false;
    }
    
    // 计算实际伤害
    float actualDamage = damage;
    if (!ignoreDefense) {
        float reduction = defense / (defense + 50.0f);
        actualDamage = damage * (1.0f - reduction);
    }
    
    actualDamage = std::max(1.0f, actualDamage);
    health -= actualDamage;
    
    std::cout << "[Rabbit] Took " << actualDamage << " damage, HP: " 
              << health << "/" << maxHealth << std::endl;
    
    // 被攻击后激怒并追击！（类似饥荒的怪物行为）
    if (!isDead()) {
        isAggroed = true;
        aggroTimer = AGGRO_DURATION;
        
        // 进入追击状态
        aiState = RabbitAIState::Chasing;
        
        std::cout << "[Rabbit] Aggroed! Chasing player for " << AGGRO_DURATION << " seconds" << std::endl;
    } else {
        if (onDeath) {
            onDeath(*this);
        }
    }
    
    return isDead();
}

float Rabbit::performAttack() {
    // 检查是否触发技能（10%概率）
    lastAttackUsedSkill = rollSkill();
    
    if (lastAttackUsedSkill) {
        std::cout << "[Rabbit] Used skill: " << skill.name 
                  << " (x" << skill.damageMultiplier << " damage)" << std::endl;
        return attack * skill.damageMultiplier;
    }
    
    return attack;
}

bool Rabbit::rollSkill() const {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(rng) < skill.triggerChance;
}

float Rabbit::getSkillMultiplier() const {
    return skill.damageMultiplier;
}

bool Rabbit::rollDodge() const {
    float dodgeChance = std::min(dodge * 0.5f, 100.0f);
    std::uniform_real_distribution<float> dist(0.0f, 100.0f);
    return dist(rng) < dodgeChance;
}

bool Rabbit::containsPoint(const sf::Vector2f& point) const {
    return getBounds().contains(point);
}

bool Rabbit::intersects(const sf::FloatRect& rect) const {
    return getCollisionBox().intersects(rect);
}

sf::FloatRect Rabbit::getBounds() const {
    return sf::FloatRect(position.x, position.y, size.x * 2, size.y * 2);
}

sf::FloatRect Rabbit::getCollisionBox() const {
    float shrink = 0.2f;
    sf::FloatRect bounds = getBounds();
    return sf::FloatRect(
        bounds.left + bounds.width * shrink / 2,
        bounds.top + bounds.height * shrink / 2,
        bounds.width * (1 - shrink),
        bounds.height * (1 - shrink)
    );
}

void Rabbit::setPosition(float x, float y) {
    position = sf::Vector2f(x, y);
    updateSprite();
}

std::vector<std::pair<std::string, int>> Rabbit::generateDrops() const {
    std::vector<std::pair<std::string, int>> result;
    
    for (const auto& drop : drops) {
        std::uniform_real_distribution<float> chanceDist(0.0f, 1.0f);
        if (chanceDist(rng) <= drop.dropChance) {
            std::uniform_int_distribution<int> countDist(drop.minCount, drop.maxCount);
            int count = countDist(rng);
            result.push_back({drop.itemId, count});
        }
    }
    
    return result;
}

int Rabbit::getExpReward() const {
    std::uniform_int_distribution<int> dist(expMin, expMax);
    return dist(rng);
}

int Rabbit::getGoldReward() const {
    std::uniform_int_distribution<int> dist(goldMin, goldMax);
    return dist(rng);
}

// ============================================================================
// RabbitManager 实现
// ============================================================================

RabbitManager::RabbitManager()
    : fontLoaded(false)
    , hoveredRabbit(nullptr)
    , rng(std::random_device{}())
{
}

bool RabbitManager::init(const std::string& texturePath_) {
    texturePath = texturePath_;
    
    // 尝试加载字体
    std::vector<std::string> fontPaths = {
        "../../assets/fonts/NotoSansSC-Regular.ttf",
        "/usr/share/fonts/truetype/noto/NotoSansSC-Regular.ttf",
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "assets/fonts/NotoSansSC-Regular.ttf"
    };
    
    for (const auto& fontPath : fontPaths) {
        if (font.loadFromFile(fontPath)) {
            fontLoaded = true;
            break;
        }
    }
    
    std::cout << "[RabbitManager] Initialized with texture: " << texturePath << std::endl;
    return true;
}

void RabbitManager::update(float dt, const sf::Vector2f& playerPos) {
    // 移除死亡的兔子
    rabbits.erase(
        std::remove_if(rabbits.begin(), rabbits.end(),
            [](const std::unique_ptr<Rabbit>& r) { return r->isDead(); }),
        rabbits.end()
    );
    
    // 更新所有兔子
    for (auto& rabbit : rabbits) {
        rabbit->update(dt, playerPos);
    }
}

void RabbitManager::render(sf::RenderWindow& window, const sf::View& view) {
    sf::FloatRect viewBounds(
        view.getCenter().x - view.getSize().x / 2,
        view.getCenter().y - view.getSize().y / 2,
        view.getSize().x,
        view.getSize().y
    );
    
    for (auto& rabbit : rabbits) {
        if (viewBounds.intersects(rabbit->getBounds())) {
            rabbit->render(window);
        }
    }
}

void RabbitManager::renderTooltips(sf::RenderWindow& window, const sf::Vector2f& mouseWorldPos) {
    updateHover(mouseWorldPos);
    
    if (hoveredRabbit && fontLoaded) {
        renderTooltip(window, hoveredRabbit);
    }
}

void RabbitManager::renderTooltip(sf::RenderWindow& window, Rabbit* rabbit) {
    if (!rabbit || !fontLoaded) return;
    
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    float tooltipX = mousePos.x + 15.0f;
    float tooltipY = mousePos.y + 15.0f;
    
    // 准备文字内容 - 使用三元组：文字、颜色、是否加粗
    struct TooltipLine {
        std::string text;
        sf::Color color;
        bool bold;
        bool isSkillIcon;
        sf::Color iconColor;
    };
    std::vector<TooltipLine> lines;
    
    // 标题
    lines.push_back({rabbit->getName(), sf::Color(255, 200, 100), true, false, sf::Color::White});
    lines.push_back({"Lv.1 普通怪物", sf::Color(180, 180, 180), false, false, sf::Color::White});
    lines.push_back({"", sf::Color::White, false, false, sf::Color::White});  // 分隔线
    
    // 基础属性
    std::string hpStr = "HP: " + std::to_string(static_cast<int>(rabbit->getHealth())) + "/" +
                        std::to_string(static_cast<int>(rabbit->getMaxHealth()));
    lines.push_back({hpStr, sf::Color(220, 80, 80), false, false, sf::Color::White});
    
    std::string atkStr = "攻击: " + std::to_string(static_cast<int>(rabbit->getAttack()));
    lines.push_back({atkStr, sf::Color(255, 150, 150), false, false, sf::Color::White});
    
    std::string defStr = "防御: " + std::to_string(static_cast<int>(rabbit->getDefense()));
    lines.push_back({defStr, sf::Color(150, 150, 255), false, false, sf::Color::White});
    
    std::string dodgeStr = "闪避: " + std::to_string(rabbit->getDodge()) + 
                           " (" + std::to_string(static_cast<int>(rabbit->getDodge() * 0.5f)) + "%)";
    lines.push_back({dodgeStr, sf::Color(150, 255, 150), false, false, sf::Color::White});
    
    // 分隔线
    lines.push_back({"", sf::Color::White, false, false, sf::Color::White});
    lines.push_back({"═══ 技能 ═══", sf::Color(200, 180, 140), true, false, sf::Color::White});
    
    // 技能信息
    const RabbitSkill& skill = rabbit->getSkill();
    std::string skillName = skill.name + " (" + 
                           std::to_string(static_cast<int>(skill.triggerChance * 100)) + "%)";
    lines.push_back({skillName, skill.iconColor, true, true, skill.iconColor});
    lines.push_back({skill.description, sf::Color(200, 200, 200), false, false, sf::Color::White});
    
    // 掉落物品信息
    lines.push_back({"", sf::Color::White, false, false, sf::Color::White});
    lines.push_back({"═══ 掉落 ═══", sf::Color(200, 180, 140), true, false, sf::Color::White});
    
    const auto& drops = rabbit->getDrops();
    for (const auto& drop : drops) {
        std::string dropStr = drop.name + " x" + std::to_string(drop.minCount);
        if (drop.maxCount > drop.minCount) {
            dropStr += "-" + std::to_string(drop.maxCount);
        }
        dropStr += " (" + std::to_string(static_cast<int>(drop.dropChance * 100)) + "%)";
        lines.push_back({dropStr, sf::Color(180, 220, 180), false, false, sf::Color::White});
    }
    
    // 经验和金币奖励
    auto expRange = rabbit->getExpRange();
    auto goldRange = rabbit->getGoldRange();
    
    std::string expStr = "经验: " + std::to_string(expRange.first) + "-" + std::to_string(expRange.second);
    lines.push_back({expStr, sf::Color(150, 200, 255), false, false, sf::Color::White});
    
    std::string goldStr = "金币: " + std::to_string(goldRange.first) + "-" + std::to_string(goldRange.second);
    lines.push_back({goldStr, sf::Color(255, 215, 0), false, false, sf::Color::White});
    
    // 状态信息
    if (rabbit->isAggressive()) {
        lines.push_back({"", sf::Color::White, false, false, sf::Color::White});
        lines.push_back({"[激怒状态]", sf::Color(255, 100, 100), true, false, sf::Color::White});
        
        std::string stateStr = "";
        switch (rabbit->getAIState()) {
            case RabbitAIState::Chasing: stateStr = "追击中"; break;
            case RabbitAIState::Attacking: stateStr = "攻击中"; break;
            case RabbitAIState::Returning: stateStr = "返回中"; break;
            default: break;
        }
        if (!stateStr.empty()) {
            lines.push_back({stateStr, sf::Color(255, 150, 100), false, false, sf::Color::White});
        }
    }
    
    // 计算尺寸
    float lineHeight = 20.0f;
    float padding = 10.0f;
    float maxWidth = 0.0f;
    
    sf::Text measureText;
    measureText.setFont(font);
    measureText.setCharacterSize(13);
    
    for (const auto& line : lines) {
        if (line.text.empty()) continue;
        measureText.setString(sf::String::fromUtf8(line.text.begin(), line.text.end()));
        float width = measureText.getLocalBounds().width;
        if (line.isSkillIcon) width += 20;  // 为技能图标预留空间
        if (width > maxWidth) maxWidth = width;
    }
    
    float tooltipWidth = std::max(180.0f, maxWidth + padding * 2);
    float tooltipHeight = 0;
    for (const auto& line : lines) {
        tooltipHeight += line.text.empty() ? lineHeight * 0.4f : lineHeight;
    }
    tooltipHeight += padding * 2;
    
    // 边界检查
    sf::Vector2u windowSize = window.getSize();
    if (tooltipX + tooltipWidth > windowSize.x) tooltipX = windowSize.x - tooltipWidth - 10;
    if (tooltipY + tooltipHeight > windowSize.y) tooltipY = windowSize.y - tooltipHeight - 10;
    
    // 绘制背景
    sf::RectangleShape bg(sf::Vector2f(tooltipWidth, tooltipHeight));
    bg.setPosition(tooltipX, tooltipY);
    bg.setFillColor(sf::Color(20, 20, 30, 245));
    bg.setOutlineThickness(2);
    bg.setOutlineColor(sf::Color(139, 90, 43));
    window.draw(bg);
    
    // 绘制标题背景条
    sf::RectangleShape titleBg(sf::Vector2f(tooltipWidth - 4, lineHeight + 2));
    titleBg.setPosition(tooltipX + 2, tooltipY + 2);
    titleBg.setFillColor(sf::Color(50, 40, 30, 200));
    window.draw(titleBg);
    
    // 绘制文字
    float y = tooltipY + padding;
    for (size_t i = 0; i < lines.size(); i++) {
        const auto& line = lines[i];
        
        if (line.text.empty()) {
            y += lineHeight * 0.4f;
            continue;
        }
        
        float textX = tooltipX + padding;
        
        // 绘制技能图标
        if (line.isSkillIcon) {
            sf::CircleShape icon(7);
            icon.setFillColor(line.iconColor);
            icon.setOutlineThickness(1);
            icon.setOutlineColor(sf::Color(255, 255, 255, 150));
            icon.setPosition(textX, y + 2);
            window.draw(icon);
            
            // 在图标中心画一个小牙齿形状
            sf::ConvexShape tooth;
            tooth.setPointCount(3);
            tooth.setPoint(0, sf::Vector2f(0, 0));
            tooth.setPoint(1, sf::Vector2f(4, 8));
            tooth.setPoint(2, sf::Vector2f(8, 0));
            tooth.setFillColor(sf::Color::White);
            tooth.setPosition(textX + 3, y + 3);
            window.draw(tooth);
            
            textX += 20;
        }
        
        sf::Text text;
        text.setFont(font);
        text.setString(sf::String::fromUtf8(line.text.begin(), line.text.end()));
        text.setCharacterSize(i == 0 ? 15 : 13);
        text.setFillColor(line.color);
        text.setPosition(textX, y);
        if (line.bold) {
            text.setStyle(sf::Text::Bold);
        }
        window.draw(text);
        
        y += lineHeight;
    }
}

Rabbit* RabbitManager::addRabbit(float x, float y) {
    auto rabbit = std::make_unique<Rabbit>(x, y);
    rabbit->loadTexture(texturePath);
    Rabbit* ptr = rabbit.get();
    rabbits.push_back(std::move(rabbit));
    return ptr;
}

void RabbitManager::removeRabbit(Rabbit* rabbit) {
    rabbits.erase(
        std::remove_if(rabbits.begin(), rabbits.end(),
            [rabbit](const std::unique_ptr<Rabbit>& r) { return r.get() == rabbit; }),
        rabbits.end()
    );
}

void RabbitManager::clearAllRabbits() {
    rabbits.clear();
    hoveredRabbit = nullptr;
}

void RabbitManager::spawnRandomRabbits(int count, const sf::Vector2i& mapSize, int tileSize) {
    (void)tileSize;
    
    std::uniform_int_distribution<int> xDist(100, mapSize.x - 100);
    std::uniform_int_distribution<int> yDist(100, mapSize.y - 100);
    
    for (int i = 0; i < count; i++) {
        float x = static_cast<float>(xDist(rng));
        float y = static_cast<float>(yDist(rng));
        addRabbit(x, y);
    }
    
    std::cout << "[RabbitManager] Spawned " << count << " rabbits in area " 
              << mapSize.x << "x" << mapSize.y << std::endl;
}

Rabbit* RabbitManager::getRabbitAt(const sf::Vector2f& position) {
    for (auto& rabbit : rabbits) {
        if (rabbit->containsPoint(position)) {
            return rabbit.get();
        }
    }
    return nullptr;
}

Rabbit* RabbitManager::getRabbitInRect(const sf::FloatRect& rect) {
    for (auto& rabbit : rabbits) {
        if (rabbit->intersects(rect)) {
            return rabbit.get();
        }
    }
    return nullptr;
}

std::vector<Rabbit*> RabbitManager::damageRabbitsInRange(const sf::Vector2f& center,
                                                         float radius, float damage,
                                                         bool ignoreDefense) {
    std::vector<Rabbit*> hitRabbits;
    
    for (auto& rabbit : rabbits) {
        sf::Vector2f rabbitCenter = rabbit->getPosition() + 
                                    sf::Vector2f(rabbit->getBounds().width / 2,
                                                 rabbit->getBounds().height / 2);
        
        float dist = std::sqrt(
            std::pow(center.x - rabbitCenter.x, 2) +
            std::pow(center.y - rabbitCenter.y, 2)
        );
        
        if (dist <= radius) {
            rabbit->takeDamage(damage, ignoreDefense);
            hitRabbits.push_back(rabbit.get());
        }
    }
    
    return hitRabbits;
}

std::vector<Rabbit*> RabbitManager::getAttackingRabbitsInRange(const sf::Vector2f& center, float radius) {
    std::vector<Rabbit*> attackingRabbits;
    
    for (auto& rabbit : rabbits) {
        if (!rabbit->isAttacking()) continue;
        
        sf::Vector2f rabbitCenter = rabbit->getPosition() + 
                                    sf::Vector2f(rabbit->getBounds().width / 2,
                                                 rabbit->getBounds().height / 2);
        
        float dist = std::sqrt(
            std::pow(center.x - rabbitCenter.x, 2) +
            std::pow(center.y - rabbitCenter.y, 2)
        );
        
        if (dist <= radius) {
            attackingRabbits.push_back(rabbit.get());
        }
    }
    
    return attackingRabbits;
}

void RabbitManager::updateHover(const sf::Vector2f& mouseWorldPos) {
    // 清除之前的悬浮状态
    if (hoveredRabbit) {
        hoveredRabbit->setHovered(false);
        hoveredRabbit = nullptr;
    }
    
    // 查找新的悬浮目标
    for (auto& rabbit : rabbits) {
        if (rabbit->containsPoint(mouseWorldPos)) {
            hoveredRabbit = rabbit.get();
            hoveredRabbit->setHovered(true);
            break;
        }
    }
}

bool RabbitManager::isCollidingWithAnyRabbit(const sf::FloatRect& rect) const {
    for (const auto& rabbit : rabbits) {
        if (rabbit->getCollisionBox().intersects(rect)) {
            return true;
        }
    }
    return false;
}

bool RabbitManager::loadFont(const std::string& fontPath) {
    if (font.loadFromFile(fontPath)) {
        fontLoaded = true;
        return true;
    }
    return false;
}