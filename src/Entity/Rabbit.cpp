#include "Rabbit.h"
#include <iostream>
#include <algorithm>
#include <cmath>

// ============================================================================
// Rabbit 实现（继承自Monster）
// ============================================================================

Rabbit::Rabbit()
    : Monster()
    , rabbitSkill("bite", "撕咬", "凶猛撕咬，造成双倍伤害", 2.0f, 0.10f, sf::Color(255, 80, 80))
    , wanderTimer(0.0f)
    , wanderDuration(0.0f)
    , idleTimer(0.0f)
    , animState(RabbitAnimState::MoveDown)
    , lastAnimState(RabbitAnimState::MoveDown)
    , currentFrame(0)
    , animTimer(0.0f)
    , frameTime(0.15f)
    , animationLocked(false)
    , animLockTimer(0.0f)
{
    // 设置兔子特有的属性范围
    healthMin = 30.0f; healthMax = 50.0f;
    defenseMin = 1.0f; defenseMax = 5.0f;
    attackMin = 3.0f; attackMax = 8.0f;
    dodgeMin = 0; dodgeMax = 2;
    
    // 设置掉落奖励范围
    expMin = 10; expMax = 20;
    goldMin = 10; goldMax = 30;
    
    // 设置兔子特有掉落
    rabbitDrops.push_back(RabbitDrop("rabbit_fur", "兔毛", 1, 2, 0.30f));
    rabbitDrops.push_back(RabbitDrop("carrot", "胡萝卜", 1, 2, 0.20f));
    rabbitDrops.push_back(RabbitDrop("rabbit_meat", "兔肉", 1, 1, 0.10f));
    
    // 同时添加到父类的掉落列表
    drops.push_back(MonsterDrop("rabbit_fur", "兔毛", 1, 2, 0.30f));
    drops.push_back(MonsterDrop("carrot", "胡萝卜", 1, 2, 0.20f));
    drops.push_back(MonsterDrop("rabbit_meat", "兔肉", 1, 1, 0.10f));
    
    // 设置父类技能
    skill = MonsterSkill("bite", "撕咬", "凶猛撕咬，造成双倍伤害", 2.0f, 0.10f, sf::Color(255, 80, 80));
    
    // 设置兔子的AI参数
    moveSpeed = RABBIT_MOVE_SPEED;
    chaseSpeed = RABBIT_CHASE_SPEED;
    returnSpeed = RABBIT_RETURN_SPEED;
    aggroDuration = RABBIT_AGGRO_DURATION;
    attackRange = RABBIT_ATTACK_RANGE;
    attackCooldownTime = RABBIT_ATTACK_COOLDOWN;
    chaseRange = RABBIT_CHASE_RANGE;
    leashRange = RABBIT_LEASH_RANGE;
    
    // 设置尺寸
    size = sf::Vector2f(32.0f, 32.0f);
}

Rabbit::Rabbit(float x, float y) : Rabbit() {
    init(x, y);
}

void Rabbit::init(float x, float y) {
    Monster::init(x, y);
}

bool Rabbit::loadTexture(const std::string& texturePath) {
    if (texture.loadFromFile(texturePath)) {
        sprite.setTexture(texture);
        sprite.setTextureRect(getFrameRect(ROW_MOVE_DOWN, 0));
        sprite.setScale(2.0f, 2.0f);
        updateSprite();
        textureLoaded = true;
        std::cout << "[Rabbit] Texture loaded: " << texturePath << std::endl;
        return true;
    }
    
    std::cerr << "[Rabbit] Failed to load texture: " << texturePath << std::endl;
    return false;
}

void Rabbit::randomizeStats() {
    std::uniform_int_distribution<int> healthDist(static_cast<int>(healthMin), static_cast<int>(healthMax));
    std::uniform_int_distribution<int> defenseDist(static_cast<int>(defenseMin), static_cast<int>(defenseMax));
    std::uniform_int_distribution<int> attackDist(static_cast<int>(attackMin), static_cast<int>(attackMax));
    std::uniform_int_distribution<int> dodgeDist(dodgeMin, dodgeMax);
    
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
            if (aiState == MonsterAIState::Chasing || aiState == MonsterAIState::Attacking) {
                aiState = MonsterAIState::Returning;
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
        case MonsterAIState::Idle:
            velocity = sf::Vector2f(0, 0);
            idleTimer += dt;
            
            // 设置待机动画（基于当前朝向）
            switch (direction) {
                case MonsterDirection::Down: setAnimState(RabbitAnimState::MoveDown); break;
                case MonsterDirection::Up: setAnimState(RabbitAnimState::MoveUp); break;
                case MonsterDirection::Left: setAnimState(RabbitAnimState::MoveLeft); break;
                case MonsterDirection::Right: setAnimState(RabbitAnimState::MoveRight); break;
            }
            
            // 随机开始游荡
            if (idleTimer > 2.0f) {
                std::uniform_real_distribution<float> dist(0.0f, 1.0f);
                if (dist(rng) < 0.3f) {
                    aiState = MonsterAIState::Wandering;
                    idleTimer = 0;
                    
                    // 随机选择方向
                    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
                    float angle = angleDist(rng);
                    velocity.x = std::cos(angle) * moveSpeed;
                    velocity.y = std::sin(angle) * moveSpeed;
                    
                    std::uniform_real_distribution<float> durationDist(1.0f, 3.0f);
                    wanderDuration = durationDist(rng);
                    wanderTimer = 0;
                    
                    updateDirectionFromVelocityRabbit();
                }
                idleTimer = 0;
            }
            break;
            
        case MonsterAIState::Wandering:
            wanderTimer += dt;
            
            // 更新朝向和动画
            updateDirectionFromVelocityRabbit();
            
            if (wanderTimer >= wanderDuration) {
                aiState = MonsterAIState::Idle;
                velocity = sf::Vector2f(0, 0);
            }
            break;
            
        case MonsterAIState::Chasing:
            // 检查是否超出牵引范围
            if (distToHome > leashRange) {
                isAggroed = false;
                aiState = MonsterAIState::Returning;
                break;
            }
            
            // 追击玩家
            if (distToPlayer > attackRange) {
                if (distToPlayer > 0) {
                    sf::Vector2f chaseDir = toPlayer / distToPlayer;
                    velocity = chaseDir * chaseSpeed;
                    updateDirectionFromVelocityRabbit();
                }
            } else {
                aiState = MonsterAIState::Attacking;
                velocity = sf::Vector2f(0, 0);
            }
            break;
            
        case MonsterAIState::Attacking:
            // 检查是否超出牵引范围
            if (distToHome > leashRange) {
                isAggroed = false;
                aiState = MonsterAIState::Returning;
                break;
            }
            
            // 更新朝向朝向玩家
            if (std::abs(toPlayer.x) > std::abs(toPlayer.y)) {
                direction = (toPlayer.x > 0) ? MonsterDirection::Right : MonsterDirection::Left;
            } else {
                direction = (toPlayer.y > 0) ? MonsterDirection::Down : MonsterDirection::Up;
            }
            
            // 如果玩家跑出攻击范围，继续追
            if (distToPlayer > attackRange * 1.5f) {
                aiState = MonsterAIState::Chasing;
                break;
            }
            
            // 设置攻击动画
            switch (direction) {
                case MonsterDirection::Down: setAnimState(RabbitAnimState::AttackDown); break;
                case MonsterDirection::Up: setAnimState(RabbitAnimState::AttackUp); break;
                case MonsterDirection::Left: setAnimState(RabbitAnimState::AttackLeft); break;
                case MonsterDirection::Right: setAnimState(RabbitAnimState::AttackRight); break;
            }
            
            // 执行攻击
            if (attackCooldown <= 0) {
                if (onAttack) {
                    onAttack(*this);
                }
                if (onAttackRabbit) {
                    onAttackRabbit(*this);
                }
                attackCooldown = attackCooldownTime;
            }
            
            velocity = sf::Vector2f(0, 0);
            break;
            
        case MonsterAIState::Returning:
            // 返回家的位置
            if (distToHome > 10.0f) {
                sf::Vector2f returnDir = toHome / distToHome;
                velocity = returnDir * returnSpeed;
                updateDirectionFromVelocityRabbit();
            } else {
                velocity = sf::Vector2f(0, 0);
                aiState = MonsterAIState::Idle;
            }
            break;
            
        case MonsterAIState::Fleeing:
            // 兔子不会逃跑，但保留这个状态以防将来需要
            break;
    }
}

void Rabbit::updateDirectionFromVelocityRabbit() {
    if (velocity.x == 0 && velocity.y == 0) return;
    
    // 根据速度方向更新朝向
    if (std::abs(velocity.x) > std::abs(velocity.y)) {
        direction = (velocity.x > 0) ? MonsterDirection::Right : MonsterDirection::Left;
    } else {
        direction = (velocity.y > 0) ? MonsterDirection::Down : MonsterDirection::Up;
    }
    
    // 设置移动动画
    switch (direction) {
        case MonsterDirection::Down: setAnimState(RabbitAnimState::MoveDown); break;
        case MonsterDirection::Up: setAnimState(RabbitAnimState::MoveUp); break;
        case MonsterDirection::Left: setAnimState(RabbitAnimState::MoveLeft); break;
        case MonsterDirection::Right: setAnimState(RabbitAnimState::MoveRight); break;
    }
}

void Rabbit::updateAnimation(float dt) {
    animTimer += dt;
    
    if (animTimer >= frameTime) {
        animTimer -= frameTime;
        currentFrame = (currentFrame + 1) % FRAMES_PER_ROW;
        
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
    }
}

sf::FloatRect Rabbit::getCollisionBox() const {
    float shrink = 0.2f;
    sf::FloatRect bounds = sf::FloatRect(position.x, position.y, size.x * 2, size.y * 2);
    return sf::FloatRect(
        bounds.left + bounds.width * shrink / 2,
        bounds.top + bounds.height * shrink / 2,
        bounds.width * (1 - shrink),
        bounds.height * (1 - shrink)
    );
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
        sf::FloatRect bounds = sf::FloatRect(
            rabbit->getPosition().x, 
            rabbit->getPosition().y, 
            rabbit->getSize().x * 2, 
            rabbit->getSize().y * 2
        );
        if (viewBounds.intersects(bounds)) {
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
    
    // 准备文字内容
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
    lines.push_back({"Lv." + std::to_string(rabbit->getLevel()) + " 普通怪物", sf::Color(180, 180, 180), false, false, sf::Color::White});
    lines.push_back({"", sf::Color::White, false, false, sf::Color::White});
    
    // 基础属性
    std::string hpStr = "HP: " + std::to_string(static_cast<int>(rabbit->getHealth())) + "/" +
                        std::to_string(static_cast<int>(rabbit->getMaxHealth()));
    lines.push_back({hpStr, sf::Color(220, 80, 80), false, false, sf::Color::White});
    
    std::string atkStr = "攻击力: " + std::to_string(static_cast<int>(rabbit->getAttack()));
    lines.push_back({atkStr, sf::Color(255, 180, 100), false, false, sf::Color::White});
    
    std::string defStr = "防御力: " + std::to_string(static_cast<int>(rabbit->getDefense()));
    lines.push_back({defStr, sf::Color(100, 180, 255), false, false, sf::Color::White});
    
    std::string dodgeStr = "闪避: " + std::to_string(rabbit->getDodge());
    lines.push_back({dodgeStr, sf::Color(180, 255, 180), false, false, sf::Color::White});
    
    lines.push_back({"", sf::Color::White, false, false, sf::Color::White});
    
    // 技能
    const RabbitSkill& skill = rabbit->getRabbitSkill();
    lines.push_back({skill.name, skill.iconColor, true, true, skill.iconColor});
    lines.push_back({skill.description, sf::Color(200, 200, 200), false, false, sf::Color::White});
    
    lines.push_back({"", sf::Color::White, false, false, sf::Color::White});
    
    // 掉落物品
    lines.push_back({"掉落物品:", sf::Color(200, 200, 100), true, false, sf::Color::White});
    for (const auto& drop : rabbit->getRabbitDrops()) {
        int chance = static_cast<int>(drop.dropChance * 100);
        std::string dropStr = "  " + drop.name + " x" + std::to_string(drop.minCount);
        if (drop.maxCount > drop.minCount) {
            dropStr += "-" + std::to_string(drop.maxCount);
        }
        dropStr += " (" + std::to_string(chance) + "%)";
        lines.push_back({dropStr, sf::Color(180, 180, 180), false, false, sf::Color::White});
    }
    
    // 计算提示框大小
    float maxWidth = 0;
    float lineHeight = 18.0f;
    float padding = 10.0f;
    
    for (const auto& line : lines) {
        sf::Text text;
        text.setFont(font);
        text.setString(sf::String::fromUtf8(line.text.begin(), line.text.end()));
        text.setCharacterSize(13);
        float width = text.getLocalBounds().width;
        if (line.isSkillIcon) width += 20;
        maxWidth = std::max(maxWidth, width);
    }
    
    float tooltipWidth = maxWidth + padding * 2;
    float tooltipHeight = lines.size() * lineHeight + padding * 2;
    
    // 确保不超出窗口
    sf::Vector2u windowSize = window.getSize();
    if (tooltipX + tooltipWidth > windowSize.x) {
        tooltipX = mousePos.x - tooltipWidth - 10;
    }
    if (tooltipY + tooltipHeight > windowSize.y) {
        tooltipY = mousePos.y - tooltipHeight - 10;
    }
    
    // 绘制背景
    sf::RectangleShape background(sf::Vector2f(tooltipWidth, tooltipHeight));
    background.setPosition(tooltipX, tooltipY);
    background.setFillColor(sf::Color(20, 20, 30, 230));
    background.setOutlineThickness(2);
    background.setOutlineColor(sf::Color(80, 80, 100));
    window.draw(background);
    
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
                                    sf::Vector2f(rabbit->getSize().x, rabbit->getSize().y);
        
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
                                    sf::Vector2f(rabbit->getSize().x, rabbit->getSize().y);
        
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
    if (hoveredRabbit) {
        hoveredRabbit->setHovered(false);
        hoveredRabbit = nullptr;
    }
    
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

std::vector<Rabbit*> RabbitManager::pushRabbitsFromRect(const sf::FloatRect& moverBox, float pushStrength) {
    std::vector<Rabbit*> pushedRabbits;
    
    for (auto& rabbit : rabbits) {
        sf::FloatRect rabbitBox = rabbit->getCollisionBox();
        
        if (moverBox.intersects(rabbitBox)) {
            sf::Vector2f moverCenter(
                moverBox.left + moverBox.width / 2.0f,
                moverBox.top + moverBox.height / 2.0f
            );
            sf::Vector2f rabbitCenter(
                rabbitBox.left + rabbitBox.width / 2.0f,
                rabbitBox.top + rabbitBox.height / 2.0f
            );
            
            sf::Vector2f pushDir = rabbitCenter - moverCenter;
            float length = std::sqrt(pushDir.x * pushDir.x + pushDir.y * pushDir.y);
            
            if (length < 0.001f) {
                pushDir = sf::Vector2f(1.0f, 0.0f);
                length = 1.0f;
            }
            
            pushDir /= length;
            
            float overlapX = (moverBox.width + rabbitBox.width) / 2.0f - 
                            std::abs(moverCenter.x - rabbitCenter.x);
            float overlapY = (moverBox.height + rabbitBox.height) / 2.0f - 
                            std::abs(moverCenter.y - rabbitCenter.y);
            
            float pushDistance = std::min(overlapX, overlapY) + 2.0f;
            
            sf::Vector2f pushVector = pushDir * pushDistance * pushStrength;
            rabbit->applyPush(pushVector);
            
            pushedRabbits.push_back(rabbit.get());
        }
    }
    
    return pushedRabbits;
}

std::vector<Rabbit*> RabbitManager::getRabbitsCollidingWith(const sf::FloatRect& rect) const {
    std::vector<Rabbit*> result;
    for (const auto& rabbit : rabbits) {
        if (rabbit->getCollisionBox().intersects(rect)) {
            result.push_back(rabbit.get());
        }
    }
    return result;
}

std::vector<Rabbit*> RabbitManager::getMovingRabbitsCollidingWith(const sf::FloatRect& rect) const {
    std::vector<Rabbit*> result;
    for (const auto& rabbit : rabbits) {
        if (rabbit->isMoving() && rabbit->getCollisionBox().intersects(rect)) {
            result.push_back(rabbit.get());
        }
    }
    return result;
}

bool RabbitManager::loadFont(const std::string& fontPath) {
    if (font.loadFromFile(fontPath)) {
        fontLoaded = true;
        return true;
    }
    return false;
}
