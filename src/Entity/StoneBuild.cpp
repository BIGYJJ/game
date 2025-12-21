#include "StoneBuild.h"
#include "../World/TileMap.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <sstream>

#define U8(str) (const char*)u8##str

// ============================================================================
// StoneBuild 构造函数
// ============================================================================

StoneBuild::StoneBuild()
    : stoneType("stone_build")
    , name("石头")
    , health(30.0f)
    , maxHealth(30.0f)
    , defense(5.0f)
    , position(0, 0)
    , size(32, 32)
    , textureLoaded(false)
    , expMin(5)
    , expMax(12)
    , goldMin(10)
    , goldMax(30)
    , dropMax(3)
    , isHovered(false)
    , shakeTimer(0.0f)
    , shakeIntensity(0.0f)
    , onDestroyed(nullptr)
{
    // 默认掉落物品
    dropItems.push_back(StoneDropItem("stone", "石头", 1, 3, 0.75f));
}

StoneBuild::StoneBuild(float x, float y, const std::string& type)
    : StoneBuild()
{
    init(x, y, type);
}

void StoneBuild::init(float x, float y, const std::string& type) {
    position = sf::Vector2f(x, y);
    stoneType = type;
    
    // 根据类型设置属性
    if (type == "stone_build" || type == "stone_build2") {
        name = "石头";
        maxHealth = 30.0f;
        defense = 5.0f;
        size = sf::Vector2f(32, 32);
    } else if (type == "ore_stone") {
        name = "矿石";
        maxHealth = 50.0f;
        defense = 10.0f;
        size = sf::Vector2f(32, 32);
        dropItems.clear();
        dropItems.push_back(StoneDropItem("ore", "矿石", 1, 2, 0.8f));
        dropItems.push_back(StoneDropItem("stone", "石头", 1, 2, 0.5f));
    }
    
    health = maxHealth;
    updateSprite();
}

// ============================================================================
// 从 TileProperty 动态初始化（推荐使用）
// ============================================================================

void StoneBuild::initFromTileProperty(float x, float y, const TileProperty* prop) {
    position = sf::Vector2f(x, y);
    
    if (!prop) {
        stoneType = "stone_build";
        name = "石头";
        maxHealth = 30.0f;
        defense = 5.0f;
        size = sf::Vector2f(32, 32);
        health = maxHealth;
        updateSprite();
        return;
    }
    
    // 从 TileProperty 设置属性
    stoneType = prop->name;
    name = prop->name;
    
    // 设置中文名
    if (prop->name.find("stone") != std::string::npos) {
        name = "石头";
    }
    
    maxHealth = (float)prop->hp;
    defense = (float)prop->defense;
    size = sf::Vector2f(32, 32);  // 默认尺寸
    
    // 解析经验和金币奖励
    expMin = prop->expMin > 0 ? prop->expMin : 5;
    expMax = prop->expMax > 0 ? prop->expMax : 12;
    goldMin = prop->goldMin > 0 ? prop->goldMin : 10;
    goldMax = prop->goldMax > 0 ? prop->goldMax : 30;
    dropMax = prop->dropMax > 0 ? prop->dropMax : 3;
    
    // 清空并设置掉落物品
    dropItems.clear();
    
    if (!prop->dropTypes.empty()) {
        // 使用 TSX 配置的掉落
        for (size_t i = 0; i < prop->dropTypes.size(); i++) {
            std::string itemId = prop->dropTypes[i];
            float prob = (i < prop->dropProbabilities.size()) ? prop->dropProbabilities[i] : 0.5f;
            
            // 移除引号
            if (itemId.size() >= 2 && itemId.front() == '"' && itemId.back() == '"') {
                itemId = itemId.substr(1, itemId.size() - 2);
            }
            
            // 创建中文名映射
            std::string itemName = itemId;
            if (itemId == "stone") itemName = "石头";
            else if (itemId == "ore") itemName = "矿石";
            else if (itemId == "iron_ore") itemName = "铁矿";
            else if (itemId == "coal") itemName = "�ite煤炭";
            
            dropItems.push_back(StoneDropItem(itemId, itemName, 1, dropMax, prob));
        }
    } else {
        // 默认掉落
        dropItems.push_back(StoneDropItem("stone", "石头", 1, 3, 0.75f));
    }
    
    health = maxHealth;
    updateSprite();
}

void StoneBuild::setTextureFromProperty(const TileProperty* prop) {
    if (!prop || !prop->hasTexture || !prop->texture) {
        return;
    }
    
    texture = *prop->texture;
    sprite.setTexture(texture);
    textureLoaded = true;
    std::cout << "[StoneBuild] Loaded texture: " << prop->name << std::endl;
    updateSprite();
}

bool StoneBuild::loadTexture(const std::string& texturePath) {
    if (!texture.loadFromFile(texturePath)) {
        std::cerr << "StoneBuild: 无法加载贴图 " << texturePath << std::endl;
        textureLoaded = false;
        return false;
    }
    sprite.setTexture(texture);
    textureLoaded = true;
    updateSprite();
    return true;
}

// ============================================================================
// 更新
// ============================================================================

void StoneBuild::update(float dt) {
    // 更新震动效果
    if (shakeTimer > 0) {
        shakeTimer -= dt;
        if (shakeTimer <= 0) {
            shakeTimer = 0;
            shakeIntensity = 0;
        }
    }
}

// ============================================================================
// 渲染
// ============================================================================

void StoneBuild::render(sf::RenderWindow& window) {
    sf::Vector2f renderPos = position;
    
    // 应用震动
    if (shakeTimer > 0) {
        float offset = std::sin(shakeTimer * 50.0f) * shakeIntensity;
        renderPos.x += offset;
    }
    
    if (textureLoaded) {
        sprite.setPosition(renderPos.x, renderPos.y - size.y);
        window.draw(sprite);
    } else {
        // 没有贴图时画一个灰色块
        sf::RectangleShape placeholder(size);
        placeholder.setPosition(renderPos.x, renderPos.y - size.y);
        placeholder.setFillColor(sf::Color(120, 120, 120));
        
        // 悬浮高亮
        if (isHovered) {
            placeholder.setOutlineThickness(2);
            placeholder.setOutlineColor(sf::Color::Yellow);
        }
        
        window.draw(placeholder);
    }
    
    // 显示血条（受损时）
    if (health < maxHealth && health > 0) {
        float barWidth = size.x * 0.8f;
        float barHeight = 4.0f;
        float barX = renderPos.x + (size.x - barWidth) / 2;
        float barY = renderPos.y - size.y - 8;
        
        // 背景
        sf::RectangleShape bgBar(sf::Vector2f(barWidth, barHeight));
        bgBar.setPosition(barX, barY);
        bgBar.setFillColor(sf::Color(50, 50, 50));
        window.draw(bgBar);
        
        // 血条
        float healthRatio = health / maxHealth;
        sf::RectangleShape hpBar(sf::Vector2f(barWidth * healthRatio, barHeight));
        hpBar.setPosition(barX, barY);
        hpBar.setFillColor(sf::Color(100, 180, 100));
        window.draw(hpBar);
    }
}

void StoneBuild::updateSprite() {
    if (!textureLoaded) return;
    
    sprite.setPosition(position.x, position.y - size.y);
    
    // 根据贴图大小调整缩放
    sf::Vector2u texSize = texture.getSize();
    if (texSize.x > 0 && texSize.y > 0) {
        float scaleX = size.x / texSize.x;
        float scaleY = size.y / texSize.y;
        sprite.setScale(scaleX, scaleY);
    }
}

// ============================================================================
// 交互
// ============================================================================

bool StoneBuild::takeDamage(float damage) {
    float actualDamage = std::max(1.0f, damage - defense);
    health -= actualDamage;
    
    // 触发震动
    shakeTimer = 0.3f;
    shakeIntensity = 3.0f;
    
    std::cout << name << " 受到 " << actualDamage << " 点伤害，剩余 " << health << "/" << maxHealth << std::endl;
    
    if (health <= 0) {
        health = 0;
        if (onDestroyed) {
            onDestroyed(*this);
        }
        return true;  // 被摧毁
    }
    return false;
}

bool StoneBuild::containsPoint(const sf::Vector2f& point) const {
    return getBounds().contains(point);
}

bool StoneBuild::intersects(const sf::FloatRect& rect) const {
    return getCollisionBox().intersects(rect);
}

sf::FloatRect StoneBuild::getBounds() const {
    return sf::FloatRect(position.x, position.y - size.y, size.x, size.y);
}

sf::FloatRect StoneBuild::getCollisionBox() const {
    // 碰撞盒通常比视觉范围小一些
    sf::FloatRect bounds = getBounds();
    float shrink = 0.15f;
    return sf::FloatRect(
        bounds.left + bounds.width * shrink / 2,
        bounds.top + bounds.height * shrink,
        bounds.width * (1 - shrink),
        bounds.height * (1 - shrink)
    );
}

// ============================================================================
// 掉落物品
// ============================================================================

void StoneBuild::addDropItem(const StoneDropItem& item) {
    dropItems.push_back(item);
}

void StoneBuild::clearDropItems() {
    dropItems.clear();
}

std::vector<std::pair<std::string, int>> StoneBuild::generateDrops() {
    std::vector<std::pair<std::string, int>> drops;
    
    for (const auto& item : dropItems) {
        float roll = static_cast<float>(rand()) / RAND_MAX;
        if (roll <= item.dropChance) {
            int count = item.minCount + rand() % (item.maxCount - item.minCount + 1);
            if (count > 0) {
                drops.push_back({item.itemId, count});
            }
        }
    }
    
    return drops;
}

int StoneBuild::getExpReward() const {
    if (expMax <= expMin) return expMin;
    return expMin + rand() % (expMax - expMin + 1);
}

int StoneBuild::getGoldReward() const {
    if (goldMax <= goldMin) return goldMin;
    return goldMin + rand() % (goldMax - goldMin + 1);
}

// ============================================================================
// StoneBuildManager 实现
// ============================================================================

StoneBuildManager::StoneBuildManager()
    : fontLoaded(false)
    , hoveredStone(nullptr)
{
}

bool StoneBuildManager::init(const std::string& assetsPath) {
    assetsBasePath = assetsPath;
    
    // 尝试加载字体
    std::vector<std::string> fontPaths = {
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/simhei.ttf",
        "C:/Windows/Fonts/simsun.ttc",
        "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
        "/System/Library/Fonts/PingFang.ttc"
    };
    
    for (const auto& path : fontPaths) {
        if (loadFont(path)) break;
    }
    
    return true;
}

bool StoneBuildManager::loadFont(const std::string& fontPath) {
    if (font.loadFromFile(fontPath)) {
        fontLoaded = true;
        std::cout << "[StoneBuildManager] Font loaded: " << fontPath << std::endl;
        return true;
    }
    return false;
}

void StoneBuildManager::update(float dt) {
    for (auto& stone : stones) {
        if (stone && !stone->isDead()) {
            stone->update(dt);
        }
    }
    
    // 移除已摧毁的石头
    stones.erase(
        std::remove_if(stones.begin(), stones.end(),
            [](const std::unique_ptr<StoneBuild>& s) { return s && s->isDead(); }),
        stones.end()
    );
}

void StoneBuildManager::render(sf::RenderWindow& window, const sf::View& view) {
    sf::FloatRect viewBounds(
        view.getCenter().x - view.getSize().x / 2 - 100,
        view.getCenter().y - view.getSize().y / 2 - 100,
        view.getSize().x + 200,
        view.getSize().y + 200
    );
    
    for (auto& stone : stones) {
        if (stone && !stone->isDead()) {
            sf::FloatRect bounds = stone->getBounds();
            if (viewBounds.intersects(bounds)) {
                stone->render(window);
            }
        }
    }
}

void StoneBuildManager::renderTooltips(sf::RenderWindow& window, const sf::Vector2f& mouseWorldPos) {
    updateHover(mouseWorldPos);
    
    if (hoveredStone && fontLoaded) {
        renderTooltip(window, hoveredStone);
    }
}

void StoneBuildManager::renderTooltip(sf::RenderWindow& window, StoneBuild* stone) {
    if (!stone || !fontLoaded) return;
    
    // 获取鼠标屏幕位置
    sf::Vector2i mouseScreenPos = sf::Mouse::getPosition(window);
    float tooltipX = mouseScreenPos.x + 20.0f;
    float tooltipY = mouseScreenPos.y + 20.0f;
    
    std::vector<sf::String> lines;
    
    // 标题（名称）
    lines.push_back(sf::String::fromUtf8(stone->getName().begin(), stone->getName().end()));
    
    // 类型
    std::string typeStr = std::string(u8"类型: ") + stone->getStoneType();
    lines.push_back(sf::String::fromUtf8(typeStr.begin(), typeStr.end()));
    
    // 空行
    lines.push_back("");
    
    // 生命值
    std::ostringstream hpStream;
    hpStream << u8"生命值: " << (int)stone->getHealth() << " / " << (int)stone->getMaxHealth();
    std::string hpStr = hpStream.str();
    lines.push_back(sf::String::fromUtf8(hpStr.begin(), hpStr.end()));
    
    // 防御
    std::ostringstream defStream;
    defStream << u8"防御力: " << (int)stone->getDefense();
    std::string defStr = defStream.str();
    lines.push_back(sf::String::fromUtf8(defStr.begin(), defStr.end()));
    
    // 空行
    lines.push_back("");
    
    // 掉落物品
    std::string dropTitleStr = std::string(u8"== 砍伐掉落 ==");
    lines.push_back(sf::String::fromUtf8(dropTitleStr.begin(), dropTitleStr.end()));
    
    for (const auto& item : stone->getDropItems()) {
        std::ostringstream itemStream;
        itemStream << "  - " << item.name;
        itemStream << " x1-" << stone->getDropMax();
        itemStream << " (" << (int)(item.dropChance * 100) << "%)";
        std::string itemStr = itemStream.str();
        lines.push_back(sf::String::fromUtf8(itemStr.begin(), itemStr.end()));
    }
    
    // 计算提示框尺寸
    float lineHeight = 28.0f;
    float padding = 16.0f;
    float minWidth = 220.0f;
    float maxWidth = 0.0f;
    
    sf::Text measureText;
    measureText.setFont(font);
    measureText.setCharacterSize(18);
    
    for (const auto& line : lines) {
        measureText.setString(line);
        float width = measureText.getLocalBounds().width;
        if (width > maxWidth) maxWidth = width;
    }
    
    float tooltipWidth = std::max(minWidth, maxWidth + padding * 2);
    float tooltipHeight = lines.size() * lineHeight + padding * 2 + 20;
    
    // 确保不超出屏幕
    sf::Vector2u windowSize = window.getSize();
    if (tooltipX + tooltipWidth > windowSize.x) {
        tooltipX = mouseScreenPos.x - tooltipWidth - 10;
    }
    if (tooltipY + tooltipHeight > windowSize.y) {
        tooltipY = mouseScreenPos.y - tooltipHeight - 10;
    }
    
    // 使用屏幕坐标绘制
    sf::View currentView = window.getView();
    window.setView(window.getDefaultView());
    
    // 绘制背景
    sf::RectangleShape bg(sf::Vector2f(tooltipWidth, tooltipHeight));
    bg.setPosition(tooltipX, tooltipY);
    bg.setFillColor(sf::Color(20, 20, 30, 240));
    bg.setOutlineThickness(3);
    bg.setOutlineColor(sf::Color(100, 80, 60));
    window.draw(bg);
    
    // 绘制标题栏
    sf::RectangleShape titleBar(sf::Vector2f(tooltipWidth, lineHeight + 8));
    titleBar.setPosition(tooltipX, tooltipY);
    titleBar.setFillColor(sf::Color(60, 50, 40, 200));
    window.draw(titleBar);
    
    // 绘制文本
    float currentY = tooltipY + padding;
    for (size_t i = 0; i < lines.size(); ++i) {
        if (lines[i].getSize() == 0) {
            currentY += lineHeight * 0.5f;
            continue;
        }
        
        sf::Text text;
        text.setFont(font);
        text.setString(lines[i]);
        text.setCharacterSize(18);
        text.setPosition(tooltipX + padding, currentY);
        
        // 根据行内容设置颜色
        if (i == 0) {
            text.setFillColor(sf::Color(255, 220, 150));  // 标题金色
            text.setCharacterSize(20);
        } else if (lines[i].find("HP") != sf::String::InvalidPos || 
                   lines[i].toAnsiString().find(u8"生命值") != std::string::npos) {
            text.setFillColor(sf::Color(100, 220, 100));  // 生命绿色
        } else if (lines[i].toAnsiString().find(u8"防御") != std::string::npos) {
            text.setFillColor(sf::Color(100, 150, 255));  // 防御蓝色
        } else if (lines[i].toAnsiString().find("==") != std::string::npos) {
            text.setFillColor(sf::Color(255, 200, 100));  // 分隔线橙色
        } else if (lines[i].toAnsiString().find("-") == 0 || 
                   lines[i].toAnsiString().find("  -") != std::string::npos) {
            text.setFillColor(sf::Color(200, 200, 200));  // 掉落物白色
        } else {
            text.setFillColor(sf::Color(180, 180, 180));  // 默认灰色
        }
        
        window.draw(text);
        currentY += lineHeight;
    }
    
    // 恢复视图
    window.setView(currentView);
}

StoneBuild* StoneBuildManager::addStone(float x, float y, const std::string& type) {
    auto stone = std::make_unique<StoneBuild>(x, y, type);
    StoneBuild* ptr = stone.get();
    stones.push_back(std::move(stone));
    return ptr;
}

StoneBuild* StoneBuildManager::addStoneFromProperty(float x, float y, const TileProperty* prop) {
    auto stone = std::make_unique<StoneBuild>();
    stone->initFromTileProperty(x, y, prop);
    
    if (prop) {
        stone->setTextureFromProperty(prop);
    }
    
    StoneBuild* ptr = stone.get();
    stones.push_back(std::move(stone));
    return ptr;
}

void StoneBuildManager::removeStone(StoneBuild* stone) {
    auto it = std::find_if(stones.begin(), stones.end(),
        [stone](const std::unique_ptr<StoneBuild>& s) { return s.get() == stone; });
    if (it != stones.end()) {
        stones.erase(it);
    }
}

void StoneBuildManager::clearAllStones() {
    stones.clear();
    hoveredStone = nullptr;
}

void StoneBuildManager::loadFromMapObjects(const std::vector<MapObject>& objects, float displayScale) {
    for (const auto& obj : objects) {
        if (obj.tileProperty) {
            // 检查是否是石头建筑类型
            if (obj.tileProperty->type == "stone_build" ||
                obj.tileProperty->name.find("stone_build") != std::string::npos) {
                
                float x = obj.x * displayScale;
                float y = obj.y * displayScale;
                
                StoneBuild* stone = addStoneFromProperty(x, y, obj.tileProperty);
                if (stone) {
                    stone->setSize(obj.width * displayScale, obj.height * displayScale);
                }
            }
        }
    }
    
    std::cout << "[StoneBuildManager] Loaded " << stones.size() << " stones from map" << std::endl;
}

StoneBuild* StoneBuildManager::getStoneAt(const sf::Vector2f& position) {
    for (auto& stone : stones) {
        if (stone && !stone->isDead() && stone->containsPoint(position)) {
            return stone.get();
        }
    }
    return nullptr;
}

StoneBuild* StoneBuildManager::getStoneInRect(const sf::FloatRect& rect) {
    for (auto& stone : stones) {
        if (stone && !stone->isDead() && stone->intersects(rect)) {
            return stone.get();
        }
    }
    return nullptr;
}

std::vector<StoneBuild*> StoneBuildManager::damageStonesInRange(const sf::Vector2f& center, 
                                                                 float radius, float damage) {
    std::vector<StoneBuild*> hitStones;
    
    for (auto& stone : stones) {
        if (!stone || stone->isDead()) continue;
        
        sf::Vector2f stoneCenter = stone->getPosition();
        stoneCenter.y -= stone->getSize().y / 2;
        
        float dx = stoneCenter.x - center.x;
        float dy = stoneCenter.y - center.y;
        float distSq = dx * dx + dy * dy;
        
        if (distSq <= radius * radius) {
            stone->takeDamage(damage);
            hitStones.push_back(stone.get());
        }
    }
    
    return hitStones;
}

void StoneBuildManager::updateHover(const sf::Vector2f& mouseWorldPos) {
    hoveredStone = nullptr;
    
    for (auto& stone : stones) {
        if (stone && !stone->isDead()) {
            bool hovered = stone->containsPoint(mouseWorldPos);
            stone->setHovered(hovered);
            if (hovered) {
                hoveredStone = stone.get();
            }
        }
    }
}

bool StoneBuildManager::isCollidingWithAnyStone(const sf::FloatRect& rect) const {
    for (const auto& stone : stones) {
        if (stone && !stone->isDead()) {
            if (stone->intersects(rect)) {
                return true;
            }
        }
    }
    return false;
}
