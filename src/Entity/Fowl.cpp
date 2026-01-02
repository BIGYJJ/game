#include "Fowl.h"
#include "Chick.h"
#include "Hen.h"
#include <iostream>
#include <algorithm>
#include <cmath>

// ============================================================================
// Fowl 基类实现
// ============================================================================

Fowl::Fowl()
    : Monster()
    , fowlName("家禽")
    , fowlTypeName("家禽")
    , satiety(100.0f)
    , maxSatiety(100.0f)
    , dailyHunger(10.0f)
    , canEvolveFlag(false)
    , evolveTo("")
    , growthDays(0)
    , growthCycleDays(0)
    , hasProduct(false)
    , productCycleDays(0)
    , productTimer(0.0f)
    , canAttack(false)
    , wanderTimer(0.0f)
    , wanderDuration(0.0f)
    , idleTimer(0.0f)
    , animState(FowlAnimState::MoveDown)
    , lastAnimState(FowlAnimState::MoveDown)
    , currentFrame(0)
    , animTimer(0.0f)
    , frameTime(0.15f)
    , animationLocked(false)
    , animLockTimer(0.0f)
{
    // 设置默认尺寸
    size = sf::Vector2f(32.0f, 32.0f);
    
    // 设置默认AI参数（家禽比较温顺，移动较慢）
    moveSpeed = 30.0f;
    chaseSpeed = 50.0f;
    returnSpeed = 40.0f;
    aggroDuration = 5.0f;
    attackRange = 35.0f;
    attackCooldownTime = 1.5f;
    chaseRange = 150.0f;
    leashRange = 250.0f;
}

void Fowl::init(float x, float y) {
    Monster::init(x, y);
}

bool Fowl::loadTexture(const std::string& texturePath) {
    if (texture.loadFromFile(texturePath)) {
        sprite.setTexture(texture);
        sprite.setTextureRect(getFrameRect(ROW_MOVE_DOWN, 0));
        sprite.setScale(2.0f, 2.0f);
        updateSprite();
        textureLoaded = true;
        std::cout << "[Fowl] Texture loaded: " << texturePath << std::endl;
        return true;
    }
    
    std::cerr << "[Fowl] Failed to load texture: " << texturePath << std::endl;
    return false;
}

void Fowl::randomizeStats() {
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

void Fowl::update(float dt, const sf::Vector2f& playerPos) {
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

void Fowl::updateAI(float dt, const sf::Vector2f& playerPos) {
    // 如果不能攻击，简化AI（只游荡）
    if (!canAttack) {
        switch (aiState) {
            case MonsterAIState::Idle:
                velocity = sf::Vector2f(0, 0);
                idleTimer += dt;
                
                // 设置待机动画
                switch (direction) {
                    case MonsterDirection::Down: setAnimState(FowlAnimState::MoveDown); break;
                    case MonsterDirection::Up: setAnimState(FowlAnimState::MoveUp); break;
                    case MonsterDirection::Left: setAnimState(FowlAnimState::MoveLeft); break;
                    case MonsterDirection::Right: setAnimState(FowlAnimState::MoveRight); break;
                }
                
                // 随机开始游荡
                if (idleTimer > 2.0f) {
                    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
                    if (dist(rng) < 0.3f) {
                        aiState = MonsterAIState::Wandering;
                        idleTimer = 0;
                        
                        std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
                        float angle = angleDist(rng);
                        velocity.x = std::cos(angle) * moveSpeed;
                        velocity.y = std::sin(angle) * moveSpeed;
                        
                        std::uniform_real_distribution<float> durationDist(1.0f, 3.0f);
                        wanderDuration = durationDist(rng);
                        wanderTimer = 0;
                        
                        updateDirectionFromVelocityFowl();
                    }
                    idleTimer = 0;
                }
                break;
                
            case MonsterAIState::Wandering:
                wanderTimer += dt;
                updateDirectionFromVelocityFowl();
                
                if (wanderTimer >= wanderDuration) {
                    aiState = MonsterAIState::Idle;
                    velocity = sf::Vector2f(0, 0);
                }
                break;
                
            default:
                aiState = MonsterAIState::Idle;
                velocity = sf::Vector2f(0, 0);
                break;
        }
        return;
    }
    
    // 可以攻击的家禽，使用完整AI（和怪物一样）
    sf::Vector2f toPlayer = playerPos - position;
    float distToPlayer = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
    
    sf::Vector2f toHome = homePosition - position;
    float distToHome = std::sqrt(toHome.x * toHome.x + toHome.y * toHome.y);
    
    switch (aiState) {
        case MonsterAIState::Idle:
            velocity = sf::Vector2f(0, 0);
            idleTimer += dt;
            
            switch (direction) {
                case MonsterDirection::Down: setAnimState(FowlAnimState::MoveDown); break;
                case MonsterDirection::Up: setAnimState(FowlAnimState::MoveUp); break;
                case MonsterDirection::Left: setAnimState(FowlAnimState::MoveLeft); break;
                case MonsterDirection::Right: setAnimState(FowlAnimState::MoveRight); break;
            }
            
            if (idleTimer > 2.0f) {
                std::uniform_real_distribution<float> dist(0.0f, 1.0f);
                if (dist(rng) < 0.3f) {
                    aiState = MonsterAIState::Wandering;
                    idleTimer = 0;
                    
                    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
                    float angle = angleDist(rng);
                    velocity.x = std::cos(angle) * moveSpeed;
                    velocity.y = std::sin(angle) * moveSpeed;
                    
                    std::uniform_real_distribution<float> durationDist(1.0f, 3.0f);
                    wanderDuration = durationDist(rng);
                    wanderTimer = 0;
                    
                    updateDirectionFromVelocityFowl();
                }
                idleTimer = 0;
            }
            break;
            
        case MonsterAIState::Wandering:
            wanderTimer += dt;
            updateDirectionFromVelocityFowl();
            
            if (wanderTimer >= wanderDuration) {
                aiState = MonsterAIState::Idle;
                velocity = sf::Vector2f(0, 0);
            }
            break;
            
        case MonsterAIState::Chasing:
            if (distToHome > leashRange) {
                isAggroed = false;
                aiState = MonsterAIState::Returning;
                break;
            }
            
            if (distToPlayer > attackRange) {
                if (distToPlayer > 0) {
                    sf::Vector2f chaseDir = toPlayer / distToPlayer;
                    velocity = chaseDir * chaseSpeed;
                    updateDirectionFromVelocityFowl();
                }
            } else {
                aiState = MonsterAIState::Attacking;
                velocity = sf::Vector2f(0, 0);
            }
            break;
            
        case MonsterAIState::Attacking:
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
                case MonsterDirection::Down: setAnimState(FowlAnimState::AttackDown); break;
                case MonsterDirection::Up: setAnimState(FowlAnimState::AttackUp); break;
                case MonsterDirection::Left: setAnimState(FowlAnimState::AttackLeft); break;
                case MonsterDirection::Right: setAnimState(FowlAnimState::AttackRight); break;
            }
            
            // 执行攻击（和怪物兔子一样的逻辑）
            if (attackCooldown <= 0) {
                if (onAttack) {
                    onAttack(*this);
                }
                attackCooldown = attackCooldownTime;
            }
            
            velocity = sf::Vector2f(0, 0);
            break;
            
        case MonsterAIState::Returning:
            if (distToHome < 10.0f) {
                aiState = MonsterAIState::Idle;
                velocity = sf::Vector2f(0, 0);
                break;
            }
            
            if (distToHome > 0) {
                sf::Vector2f returnDir = toHome / distToHome;
                velocity = returnDir * returnSpeed;
                updateDirectionFromVelocityFowl();
            }
            break;
            
        default:
            break;
    }
}

void Fowl::updateAnimation(float dt) {
    animTimer += dt;
    
    if (animTimer >= frameTime) {
        animTimer -= frameTime;  // 保持动画平滑（和兔子一样）
        currentFrame = (currentFrame + 1) % FRAMES_PER_ROW;
        
        // 获取当前动画行
        int row = 0;
        switch (animState) {
            case FowlAnimState::MoveDown: row = ROW_MOVE_DOWN; break;
            case FowlAnimState::MoveUp: row = ROW_MOVE_UP; break;
            case FowlAnimState::MoveLeft: row = ROW_MOVE_LEFT; break;
            case FowlAnimState::MoveRight: row = ROW_MOVE_RIGHT; break;
            case FowlAnimState::AttackDown: row = ROW_ATTACK_DOWN; break;
            case FowlAnimState::AttackUp: row = ROW_ATTACK_UP; break;
            case FowlAnimState::AttackLeft: row = ROW_ATTACK_LEFT; break;
            case FowlAnimState::AttackRight: row = ROW_ATTACK_RIGHT; break;
        }
        
        sprite.setTextureRect(getFrameRect(row, currentFrame));
    }
}

void Fowl::setAnimState(FowlAnimState state) {
    // 如果动画锁定中，只允许攻击动画覆盖（和兔子怪物一样）
    if (animationLocked && 
        state != FowlAnimState::AttackDown &&
        state != FowlAnimState::AttackUp &&
        state != FowlAnimState::AttackLeft &&
        state != FowlAnimState::AttackRight) {
        return;
    }
    
    if (state != animState) {
        lastAnimState = animState;
        animState = state;
        currentFrame = 0;
        animTimer = 0;
        
        // 设置动画锁定
        animationLocked = true;
        animLockTimer = MIN_ANIM_DURATION;
    }
}

void Fowl::updateSprite() {
    sprite.setPosition(position);
}

sf::IntRect Fowl::getFrameRect(int row, int col) const {
    return sf::IntRect(col * FRAME_WIDTH, row * FRAME_HEIGHT, FRAME_WIDTH, FRAME_HEIGHT);
}

void Fowl::updateDirectionFromVelocityFowl() {
    if (std::abs(velocity.x) > std::abs(velocity.y)) {
        direction = (velocity.x > 0) ? MonsterDirection::Right : MonsterDirection::Left;
        setAnimState((velocity.x > 0) ? FowlAnimState::MoveRight : FowlAnimState::MoveLeft);
    } else if (std::abs(velocity.y) > 0.001f) {
        direction = (velocity.y > 0) ? MonsterDirection::Down : MonsterDirection::Up;
        setAnimState((velocity.y > 0) ? FowlAnimState::MoveDown : FowlAnimState::MoveUp);
    }
}

void Fowl::render(sf::RenderWindow& window) {
    if (textureLoaded) {
        window.draw(sprite);
    } else {
        // 绘制占位符
        sf::RectangleShape placeholder(size);
        placeholder.setPosition(position);
        placeholder.setFillColor(sf::Color(255, 200, 100, 200));
        placeholder.setOutlineThickness(1);
        placeholder.setOutlineColor(sf::Color::Black);
        window.draw(placeholder);
    }
    
    // 绘制生命条（如果受伤）
    if (health < maxHealth) {
        float barWidth = 40.0f;
        float barHeight = 4.0f;
        float barY = position.y - 8.0f;
        float barX = position.x + (size.x * 2 - barWidth) / 2;
        
        // 背景
        sf::RectangleShape bgBar(sf::Vector2f(barWidth, barHeight));
        bgBar.setPosition(barX, barY);
        bgBar.setFillColor(sf::Color(50, 50, 50, 200));
        window.draw(bgBar);
        
        // 生命值
        float healthPercent = getHealthPercent();
        sf::RectangleShape healthBar(sf::Vector2f(barWidth * healthPercent, barHeight));
        healthBar.setPosition(barX, barY);
        healthBar.setFillColor(sf::Color(50, 200, 50));
        window.draw(healthBar);
    }
    
    // 绘制饱腹值条
    {
        float barWidth = 40.0f;
        float barHeight = 3.0f;
        float barY = position.y - 4.0f;
        float barX = position.x + (size.x * 2 - barWidth) / 2;
        
        // 背景
        sf::RectangleShape bgBar(sf::Vector2f(barWidth, barHeight));
        bgBar.setPosition(barX, barY);
        bgBar.setFillColor(sf::Color(50, 50, 50, 150));
        window.draw(bgBar);
        
        // 饱腹值
        float satietyPercent = getSatietyPercent();
        sf::RectangleShape satietyBar(sf::Vector2f(barWidth * satietyPercent, barHeight));
        satietyBar.setPosition(barX, barY);
        satietyBar.setFillColor(sf::Color(255, 180, 50));
        window.draw(satietyBar);
    }
    
    // 悬浮高亮
    if (isHovered) {
        sf::RectangleShape highlight(sf::Vector2f(size.x * 2, size.y * 2));
        highlight.setPosition(position);
        highlight.setFillColor(sf::Color(255, 255, 255, 50));
        highlight.setOutlineThickness(2);
        highlight.setOutlineColor(sf::Color(255, 255, 100, 150));
        window.draw(highlight);
    }
}

sf::FloatRect Fowl::getCollisionBox() const {
    sf::FloatRect bounds = getBounds();
    float shrink = 0.2f;
    return sf::FloatRect(
        bounds.left + bounds.width * shrink / 2,
        bounds.top + bounds.height * shrink,
        bounds.width * (1 - shrink),
        bounds.height * (1 - shrink)
    );
}

// ========================================
// 饥饿系统
// ========================================

bool Fowl::feed(float amount) {
    if (satiety >= maxSatiety) {
        return false;
    }
    
    satiety = std::min(maxSatiety, satiety + amount);
    std::cout << "[" << fowlTypeName << "] 喂食成功，饱腹值: " << satiety << "/" << maxSatiety << std::endl;
    return true;
}

// ========================================
// 生长系统
// ========================================

float Fowl::getGrowthProgress() const {
    if (growthCycleDays <= 0) return 0.0f;
    return std::min(1.0f, static_cast<float>(growthDays) / static_cast<float>(growthCycleDays));
}

// ========================================
// 产出系统
// ========================================

float Fowl::getProductProgress() const {
    if (productCycleDays <= 0) return 0.0f;
    return std::min(1.0f, productTimer / static_cast<float>(productCycleDays));
}

std::vector<std::pair<std::string, int>> Fowl::harvest() {
    std::vector<std::pair<std::string, int>> result;
    
    if (!canProduce()) {
        return result;
    }
    
    std::uniform_int_distribution<int> countDist(product.minCount, product.maxCount);
    int count = countDist(rng);
    
    if (count > 0) {
        result.push_back({product.itemId, count});
        std::cout << "[" << fowlTypeName << "] 收获: " << product.name << " x" << count << std::endl;
    }
    
    // 重置产出计时器
    productTimer = 0.0f;
    
    if (onProduce) {
        onProduce(*this);
    }
    
    return result;
}

// ========================================
// 时间更新
// ========================================

void Fowl::onDayPass() {
    // 饥饿消耗
    if (satiety > 0) {
        satiety -= dailyHunger;
        satiety = std::max(0.0f, satiety);
        
        if (satiety <= 0 && onHungry) {
            onHungry(*this);
        }
    }
    
    // 如果饱腹值为0，停止生长和产出
    if (satiety <= 0) {
        std::cout << "[" << fowlTypeName << "] 饥饿状态，停止生长和产出" << std::endl;
        return;
    }
    
    // 生长
    if (canEvolveFlag && growthDays < growthCycleDays) {
        growthDays++;
        std::cout << "[" << fowlTypeName << "] 生长天数: " << growthDays << "/" << growthCycleDays << std::endl;
        
        if (growthDays >= growthCycleDays && onEvolve) {
            onEvolve(*this);
        }
    }
    
    // 产出
    if (hasProduct && productTimer < productCycleDays) {
        productTimer += 1.0f;
        std::cout << "[" << fowlTypeName << "] 产出进度: " << productTimer << "/" << productCycleDays << std::endl;
    }
}


// ============================================================================
// FowlManager 实现
// ============================================================================

FowlManager::FowlManager()
    : fontLoaded(false)
    , hoveredFowl(nullptr)
{
    rng.seed(std::random_device{}());
}

bool FowlManager::init(const std::string& chickPath, const std::string& henPath) {
    chickTexturePath = chickPath;
    henTexturePath = henPath;
    return true;
}

Fowl* FowlManager::addChick(float x, float y) {
    auto chick = std::make_unique<Chick>(x, y);
    chick->loadTexture(chickTexturePath);
    Fowl* ptr = chick.get();
    fowls.push_back(std::move(chick));
    std::cout << "[FowlManager] 添加小鸡于 (" << x << ", " << y << ")" << std::endl;
    return ptr;
}

Fowl* FowlManager::addHen(float x, float y) {
    auto hen = std::make_unique<Hen>(x, y);
    hen->loadTexture(henTexturePath);
    Fowl* ptr = hen.get();
    fowls.push_back(std::move(hen));
    std::cout << "[FowlManager] 添加母鸡于 (" << x << ", " << y << ")" << std::endl;
    return ptr;
}

void FowlManager::update(float dt, const sf::Vector2f& playerPos) {
    for (auto& fowl : fowls) {
        fowl->update(dt, playerPos);
    }
    
    // 移除死亡的家禽
    fowls.erase(
        std::remove_if(fowls.begin(), fowls.end(),
            [](const std::unique_ptr<Fowl>& f) { return f->isDead(); }),
        fowls.end()
    );
}

void FowlManager::render(sf::RenderWindow& window, const sf::View& view) {
    sf::FloatRect viewBounds(
        view.getCenter().x - view.getSize().x / 2.0f,
        view.getCenter().y - view.getSize().y / 2.0f,
        view.getSize().x,
        view.getSize().y
    );
    
    for (auto& fowl : fowls) {
        sf::FloatRect bounds = fowl->getBounds();
        if (viewBounds.intersects(bounds)) {
            fowl->render(window);
        }
    }
}

void FowlManager::renderTooltips(sf::RenderWindow& window, const sf::Vector2f& mouseWorldPos) {
    if (hoveredFowl && fontLoaded) {
        renderTooltip(window, hoveredFowl);
    }
}

void FowlManager::renderTooltip(sf::RenderWindow& window, Fowl* fowl) {
    if (!fontLoaded) return;
    
    sf::Vector2f pos = fowl->getPosition();
    
    // 构建提示信息
    std::vector<std::string> lines;
    lines.push_back(fowl->getTypeName());
    lines.push_back("HP: " + std::to_string(static_cast<int>(fowl->getHealth())) + "/" + 
                    std::to_string(static_cast<int>(fowl->getMaxHealth())));
    lines.push_back("饱腹: " + std::to_string(static_cast<int>(fowl->getSatiety())) + "/" + 
                    std::to_string(static_cast<int>(fowl->getMaxSatiety())));
    
    if (fowl->isGrowing()) {
        lines.push_back("生长: " + std::to_string(fowl->getGrowthDays()) + "/" + 
                        std::to_string(fowl->getGrowthCycleDays()) + "天");
    }
    
    if (fowl->hasProductAbility()) {
        lines.push_back("产出: " + std::to_string(static_cast<int>(fowl->getProductProgress() * 100)) + "%");
        if (fowl->canProduce()) {
            lines.push_back("[可收获!]");
        }
    }
    
    // 绘制提示框
    float padding = 8.0f;
    float lineHeight = 18.0f;
    float tooltipWidth = 120.0f;
    float tooltipHeight = padding * 2 + lines.size() * lineHeight;
    
    float tooltipX = pos.x + 70;
    float tooltipY = pos.y - tooltipHeight / 2;
    
    sf::RectangleShape background(sf::Vector2f(tooltipWidth, tooltipHeight));
    background.setPosition(tooltipX, tooltipY);
    background.setFillColor(sf::Color(20, 20, 30, 230));
    background.setOutlineThickness(2);
    background.setOutlineColor(sf::Color(80, 80, 100));
    window.draw(background);
    
    float y = tooltipY + padding;
    for (size_t i = 0; i < lines.size(); i++) {
        sf::Text text;
        text.setFont(font);
        text.setString(sf::String::fromUtf8(lines[i].begin(), lines[i].end()));
        text.setCharacterSize(i == 0 ? 14 : 12);
        text.setFillColor(sf::Color::White);
        text.setPosition(tooltipX + padding, y);
        window.draw(text);
        y += lineHeight;
    }
}

void FowlManager::removeFowl(Fowl* fowl) {
    fowls.erase(
        std::remove_if(fowls.begin(), fowls.end(),
            [fowl](const std::unique_ptr<Fowl>& f) { return f.get() == fowl; }),
        fowls.end()
    );
    
    if (hoveredFowl == fowl) {
        hoveredFowl = nullptr;
    }
}

void FowlManager::clearAllFowls() {
    fowls.clear();
    hoveredFowl = nullptr;
}

void FowlManager::onDayPass() {
    for (auto& fowl : fowls) {
        fowl->onDayPass();
    }
}

void FowlManager::processEvolutions() {
    // 进化处理需要在外部实现，因为需要创建新的家禽类型
    // 这里只提供获取可进化家禽的接口
}

Fowl* FowlManager::getFowlAt(const sf::Vector2f& position) {
    for (auto& fowl : fowls) {
        if (fowl->containsPoint(position)) {
            return fowl.get();
        }
    }
    return nullptr;
}

Fowl* FowlManager::getFowlInRect(const sf::FloatRect& rect) {
    for (auto& fowl : fowls) {
        if (fowl->intersects(rect)) {
            return fowl.get();
        }
    }
    return nullptr;
}

std::vector<Fowl*> FowlManager::damageFowlsInRange(const sf::Vector2f& center,
                                                    float radius, float damage,
                                                    bool ignoreDefense) {
    std::vector<Fowl*> hitFowls;
    
    for (auto& fowl : fowls) {
        sf::Vector2f fowlCenter = fowl->getPosition() + 
                                  sf::Vector2f(fowl->getSize().x, fowl->getSize().y);
        
        float dist = std::sqrt(
            std::pow(center.x - fowlCenter.x, 2) +
            std::pow(center.y - fowlCenter.y, 2)
        );
        
        if (dist <= radius) {
            fowl->takeDamage(damage, ignoreDefense);
            hitFowls.push_back(fowl.get());
        }
    }
    
    return hitFowls;
}

std::vector<Fowl*> FowlManager::getAttackingFowlsInRange(const sf::Vector2f& center, float radius) {
    std::vector<Fowl*> attackingFowls;
    
    for (auto& fowl : fowls) {
        if (!fowl->isAttacking()) continue;
        
        sf::Vector2f fowlCenter = fowl->getPosition() + 
                                  sf::Vector2f(fowl->getSize().x, fowl->getSize().y);
        
        float dist = std::sqrt(
            std::pow(center.x - fowlCenter.x, 2) +
            std::pow(center.y - fowlCenter.y, 2)
        );
        
        if (dist <= radius) {
            attackingFowls.push_back(fowl.get());
        }
    }
    
    return attackingFowls;
}

void FowlManager::updateHover(const sf::Vector2f& mouseWorldPos) {
    if (hoveredFowl) {
        hoveredFowl->setHovered(false);
        hoveredFowl = nullptr;
    }
    
    for (auto& fowl : fowls) {
        if (fowl->containsPoint(mouseWorldPos)) {
            hoveredFowl = fowl.get();
            hoveredFowl->setHovered(true);
            break;
        }
    }
}

bool FowlManager::isCollidingWithAnyFowl(const sf::FloatRect& rect) const {
    for (const auto& fowl : fowls) {
        if (fowl->getCollisionBox().intersects(rect)) {
            return true;
        }
    }
    return false;
}

std::vector<Fowl*> FowlManager::pushFowlsFromRect(const sf::FloatRect& moverBox, float pushStrength) {
    std::vector<Fowl*> pushedFowls;
    
    for (auto& fowl : fowls) {
        sf::FloatRect fowlBox = fowl->getCollisionBox();
        
        if (moverBox.intersects(fowlBox)) {
            sf::Vector2f moverCenter(
                moverBox.left + moverBox.width / 2.0f,
                moverBox.top + moverBox.height / 2.0f
            );
            sf::Vector2f fowlCenter(
                fowlBox.left + fowlBox.width / 2.0f,
                fowlBox.top + fowlBox.height / 2.0f
            );
            
            sf::Vector2f pushDir = fowlCenter - moverCenter;
            float length = std::sqrt(pushDir.x * pushDir.x + pushDir.y * pushDir.y);
            
            if (length < 0.001f) {
                pushDir = sf::Vector2f(1.0f, 0.0f);
                length = 1.0f;
            }
            
            pushDir /= length;
            
            float overlapX = (moverBox.width + fowlBox.width) / 2.0f - 
                            std::abs(moverCenter.x - fowlCenter.x);
            float overlapY = (moverBox.height + fowlBox.height) / 2.0f - 
                            std::abs(moverCenter.y - fowlCenter.y);
            
            float pushDistance = std::min(overlapX, overlapY) + 2.0f;
            
            sf::Vector2f pushVector = pushDir * pushDistance * pushStrength;
            fowl->applyPush(pushVector);
            
            pushedFowls.push_back(fowl.get());
        }
    }
    
    return pushedFowls;
}

std::vector<Fowl*> FowlManager::getFowlsCollidingWith(const sf::FloatRect& rect) const {
    std::vector<Fowl*> result;
    for (const auto& fowl : fowls) {
        if (fowl->getCollisionBox().intersects(rect)) {
            result.push_back(fowl.get());
        }
    }
    return result;
}

std::vector<Fowl*> FowlManager::getMovingFowlsCollidingWith(const sf::FloatRect& rect) const {
    std::vector<Fowl*> result;
    for (const auto& fowl : fowls) {
        if (fowl->isMoving() && fowl->getCollisionBox().intersects(rect)) {
            result.push_back(fowl.get());
        }
    }
    return result;
}

std::vector<Fowl*> FowlManager::getHarvestableFowls() {
    std::vector<Fowl*> result;
    for (const auto& fowl : fowls) {
        if (fowl->canProduce()) {
            result.push_back(fowl.get());
        }
    }
    return result;
}

std::vector<Fowl*> FowlManager::getEvolvableFowls() {
    std::vector<Fowl*> result;
    for (const auto& fowl : fowls) {
        if (fowl->canEvolve()) {
            result.push_back(fowl.get());
        }
    }
    return result;
}

bool FowlManager::loadFont(const std::string& fontPath) {
    if (font.loadFromFile(fontPath)) {
        fontLoaded = true;
        return true;
    }
    return false;
}
