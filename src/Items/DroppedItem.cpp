#include "DroppedItem.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>

// 静态字体指针
sf::Font* DroppedItem::sharedFont = nullptr;

// ============================================================================
// DroppedItem 实现
// ============================================================================

DroppedItem::DroppedItem()
    : count(0)
    , position(0, 0)
    , velocity(0, 0)
    , lifetime(300.0f)
    , floatTimer(0)
    , floatOffset(0)
    , pickedUp(false)
    , texture(nullptr)
    , hasTexture(false)
{
}

DroppedItem::DroppedItem(const std::string& id, int cnt, float x, float y)
    : itemId(id)
    , count(cnt)
    , position(x, y)
    , groundY(y)                // 记录初始Y位置作为地面
    , onGround(false)
    , lifetime(300.0f)
    , floatTimer((float)(rand() % 100) / 100.0f * 3.14159f * 2)  // 随机初始相位
    , floatOffset(0)
    , pickedUp(false)
    , texture(nullptr)
    , hasTexture(false)
{
    // 随机初始速度（散开效果）
    float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
    float speed = 30.0f + (float)(rand() % 30);
    velocity.x = std::cos(angle) * speed;
    velocity.y = -80.0f - (float)(rand() % 40);  // 向上抛出
    
    // 设置数量文字
    if (sharedFont) {
        countText.setFont(*sharedFont);
        countText.setCharacterSize(14);
        countText.setFillColor(sf::Color::White);
        countText.setOutlineColor(sf::Color::Black);
        countText.setOutlineThickness(1.0f);
        if (count > 1) {
            countText.setString(std::to_string(count));
        }
    }
}

void DroppedItem::update(float dt) {
    if (pickedUp) return;
    
    // 更新生命周期
    lifetime -= dt;
    
    // ========================================
    // 物品掉落物理：
    //   初始有向上抛出的速度
    //   受重力影响下落
    //   落到初始位置后停止（模拟地面）
    // ========================================
    
    if (!onGround) {
        // 重力
        velocity.y += 400.0f * dt;
        
        // 更新位置
        position += velocity * dt;
        
        // 检查是否落到地面（初始Y位置）
        if (position.y >= groundY) {
            position.y = groundY;
            velocity = sf::Vector2f(0, 0);
            onGround = true;
        }
        
        // 水平阻力
        velocity.x *= 0.95f;
    }
    
    // 浮动动画（只有落地后才浮动）
    if (onGround) {
        floatTimer += dt * 3.0f;
        floatOffset = std::sin(floatTimer) * 3.0f;
    }
    
    // 更新精灵位置
    if (hasTexture) {
        sprite.setPosition(position.x, position.y + floatOffset);
    }
    
    // 更新数量文字位置
    if (count > 1 && sharedFont) {
        sf::FloatRect bounds = sprite.getGlobalBounds();
        countText.setPosition(
            bounds.left + bounds.width - countText.getLocalBounds().width - 2,
            bounds.top + bounds.height - countText.getLocalBounds().height - 4
        );
    }
}

void DroppedItem::render(sf::RenderWindow& window) {
    if (pickedUp) return;
    
    // 快过期时闪烁
    bool visible = true;
    if (lifetime < 30.0f) {
        visible = (int)(lifetime * 4) % 2 == 0;
    }
    
    if (!visible) return;
    
    if (hasTexture) {
        window.draw(sprite);
    } else {
        // 绘制占位符
        sf::RectangleShape placeholder(sf::Vector2f(32, 32));
        placeholder.setPosition(position.x - 16, position.y - 16 + floatOffset);
        placeholder.setFillColor(sf::Color(150, 100, 50, 200));
        placeholder.setOutlineThickness(1);
        placeholder.setOutlineColor(sf::Color::White);
        window.draw(placeholder);
    }
    
    // 绘制数量
    if (count > 1 && sharedFont) {
        window.draw(countText);
    }
}

sf::FloatRect DroppedItem::getBounds() const {
    if (hasTexture) {
        return sprite.getGlobalBounds();
    }
    return sf::FloatRect(position.x - 16, position.y - 16, 32, 32);
}

bool DroppedItem::isInPickupRange(const sf::Vector2f& point, float range) const {
    float dx = position.x - point.x;
    float dy = position.y - point.y;
    return (dx * dx + dy * dy) <= (range * range);
}

void DroppedItem::setTexture(const sf::Texture* tex) {
    texture = tex;
    if (texture) {
        sprite.setTexture(*texture);
        // 居中显示
        sf::Vector2u size = texture->getSize();
        sprite.setOrigin(size.x / 2.0f, size.y / 2.0f);
        sprite.setPosition(position);
        hasTexture = true;
    }
}

// ============================================================================
// DroppedItemManager 实现
// ============================================================================

DroppedItemManager::DroppedItemManager()
    : fontLoaded(false)
{
}

bool DroppedItemManager::init(const std::string& assetsPath) {
    assetsBasePath = assetsPath;
    
    // 尝试加载字体（优先使用系统中文字体）
    std::vector<std::string> fontPaths = {
        "C:/Windows/Fonts/msyh.ttc",        // 微软雅黑
        "C:/Windows/Fonts/simhei.ttf",      // 黑体
        "C:/Windows/Fonts/simsun.ttc",      // 宋体
        assetsPath + "/fonts/pixel.ttf",
        assetsPath + "/fonts/font.ttf",
    };
    
    for (const auto& path : fontPaths) {
        if (loadFont(path)) break;
    }
    
    std::cout << "[DroppedItemManager] Initialized" << std::endl;
    return true;
}

bool DroppedItemManager::loadFont(const std::string& fontPath) {
    if (font.loadFromFile(fontPath)) {
        fontLoaded = true;
        DroppedItem::setSharedFont(&font);
        std::cout << "[DroppedItemManager] Font loaded: " << fontPath << std::endl;
        return true;
    }
    return false;
}

void DroppedItemManager::update(float dt) {
    for (auto& item : droppedItems) {
        item->update(dt);
    }
    
    // 清理过期和已拾取的物品
    cleanup();
}

void DroppedItemManager::render(sf::RenderWindow& window, const sf::View& view) {
    // 简单的视口裁剪
    sf::FloatRect viewBounds(
        view.getCenter().x - view.getSize().x / 2 - 50,
        view.getCenter().y - view.getSize().y / 2 - 50,
        view.getSize().x + 100,
        view.getSize().y + 100
    );
    
    for (auto& item : droppedItems) {
        if (!item->isPickedUp() && !item->isExpired()) {
            sf::FloatRect itemBounds = item->getBounds();
            if (viewBounds.intersects(itemBounds)) {
                item->render(window);
            }
        }
    }
}

void DroppedItemManager::spawnItem(const std::string& itemId, int count, float x, float y) {
    if (itemId.empty() || count <= 0) return;
    
    auto item = std::make_unique<DroppedItem>(itemId, count, x, y);
    
    // 设置贴图
    const sf::Texture* tex = ItemDatabase::getInstance().getTexture(itemId);
    if (tex) {
        item->setTexture(tex);
    }
    
    droppedItems.push_back(std::move(item));
    
    const ItemData* data = ItemDatabase::getInstance().getItemData(itemId);
    std::string name = data ? data->name : itemId;
    std::cout << "[DroppedItemManager] Spawned " << count << "x " << name 
              << " at (" << x << ", " << y << ")" << std::endl;
}

void DroppedItemManager::spawnItems(const std::vector<std::pair<std::string, int>>& items, 
                                     float x, float y) {
    float spacing = 40.0f;  // 物品之间的间距
    
    // 计算总宽度，使物品居中
    float totalWidth = (items.size() - 1) * spacing;
    float startX = x - totalWidth / 2;
    
    for (size_t i = 0; i < items.size(); i++) {
        const auto& pair = items[i];
        spawnItem(pair.first, pair.second, startX + i * spacing, y);
    }
}

std::vector<std::pair<std::string, int>> DroppedItemManager::calculateDrops(
    const std::vector<std::string>& dropTypes,
    const std::vector<float>& dropProbabilities,
    int dropMax) {
    
    std::vector<std::pair<std::string, int>> result;
    
    for (size_t i = 0; i < dropTypes.size(); i++) {
        const std::string& itemId = dropTypes[i];
        float baseProbability = (i < dropProbabilities.size()) ? dropProbabilities[i] : 0.5f;
        
        // ========================================
        // 递减概率计算
        // 第1个: baseProbability
        // 第2个: baseProbability * baseProbability
        // 第3个: baseProbability ^ 3
        // ...
        // ========================================
        
        int count = 0;
        float currentProbability = baseProbability;
        
        for (int j = 0; j < dropMax; j++) {
            float roll = (float)(rand() % 1000) / 1000.0f;
            if (roll < currentProbability) {
                count++;
                currentProbability *= baseProbability;  // 递减概率
            } else {
                break;  // 一旦失败就停止
            }
        }
        
        if (count > 0) {
            result.push_back({itemId, count});
        }
    }
    
    return result;
}

std::vector<ItemStack> DroppedItemManager::pickupItemsInRange(const sf::Vector2f& position, 
                                                               float range) {
    std::vector<ItemStack> pickedUp;
    
    for (auto& item : droppedItems) {
        if (item->isPickedUp() || item->isExpired()) continue;
        
        if (item->isInPickupRange(position, range)) {
            ItemStack stack(item->getItemId(), item->getCount());
            pickedUp.push_back(stack);
            item->markPickedUp();
            
            if (onItemPickup) {
                onItemPickup(stack);
            }
        }
    }
    
    return pickedUp;
}

void DroppedItemManager::cleanup() {
    droppedItems.erase(
        std::remove_if(droppedItems.begin(), droppedItems.end(),
            [](const std::unique_ptr<DroppedItem>& item) {
                return item->isPickedUp() || item->isExpired();
            }),
        droppedItems.end()
    );
}

void DroppedItemManager::clearAll() {
    droppedItems.clear();
}
