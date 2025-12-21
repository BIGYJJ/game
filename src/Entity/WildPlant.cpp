#include "WildPlant.h"
#include "../World/TileMap.h"
#include <iostream>
#include <algorithm>
#include <sstream>

#define U8(str) (const char*)u8##str

// ============================================================================
// WildPlant 构造函数
// ============================================================================

WildPlant::WildPlant()
    : plantType("carrot")
    , name("胡萝卜")
    , type(WildPlantType::Carrot)
    , allowPickup(true)
    , dropInfo("carrot", "胡萝卜", 1, 2)
    , probability(0.4f)
    , isPickedUp(false)
    , position(0, 0)
    , size(32, 32)
    , hasCustomCollision(false)
    , textureLoaded(false)
    , isHovered(false)
    , onPickup(nullptr)
{
    rng.seed(std::random_device{}());
}

WildPlant::WildPlant(float x, float y, const std::string& plantTypeStr)
    : WildPlant()
{
    init(x, y, plantTypeStr);
}

void WildPlant::init(float x, float y, const std::string& plantTypeStr) {
    position = sf::Vector2f(x, y);
    plantType = plantTypeStr;
    type = parseType(plantTypeStr);
    
    // 根据类型设置属性
    if (plantTypeStr == "carrot" || plantTypeStr == "carrot_plant") {
        name = "胡萝卜";
        type = WildPlantType::Carrot;
        dropInfo = PlantDropInfo("carrot", "胡萝卜", 1, 2);
        size = sf::Vector2f(32, 32);
    } else if (plantTypeStr == "bean" || plantTypeStr == "bean_plant") {
        name = "豆子";
        type = WildPlantType::Bean;
        dropInfo = PlantDropInfo("bean", "豆子", 1, 3);
        size = sf::Vector2f(32, 32);
    }
    
    updateSprite();
}

WildPlantType WildPlant::parseType(const std::string& typeStr) {
    if (typeStr == "carrot" || typeStr == "carrot_plant") {
        return WildPlantType::Carrot;
    } else if (typeStr == "bean" || typeStr == "bean_plant") {
        return WildPlantType::Bean;
    }
    return WildPlantType::Unknown;
}

// ============================================================================
// 从 TileProperty 动态初始化（推荐使用）
// ============================================================================

void WildPlant::initFromTileProperty(float x, float y, const TileProperty* prop) {
    position = sf::Vector2f(x, y);
    
    if (!prop) {
        plantType = "carrot";
        name = "野生植物";
        type = WildPlantType::Unknown;
        size = sf::Vector2f(32, 32);
        allowPickup = true;  // 默认允许拾取
        updateSprite();
        return;
    }
    
    // 从 TileProperty 设置属性
    plantType = prop->name;
    name = prop->name;
    
    // 解析植物类型名以确定类型
    if (prop->name.find("carrot") != std::string::npos) {
        type = WildPlantType::Carrot;
        name = "胡萝卜";
    } else if (prop->name.find("bean") != std::string::npos) {
        type = WildPlantType::Bean;
        name = "豆子";
    } else {
        type = WildPlantType::Unknown;
    }
    
    // 设置掉落物品 - 优先使用 pickupObject
    std::string itemId = prop->pickupObject;
    if (itemId.empty() && !prop->dropTypes.empty()) {
        itemId = prop->dropTypes[0];
    }
    
    // 移除引号
    if (itemId.size() >= 2 && itemId.front() == '"' && itemId.back() == '"') {
        itemId = itemId.substr(1, itemId.size() - 2);
    }
    
    // 如果没有设置itemId，根据植物类型自动设置
    if (itemId.empty()) {
        if (type == WildPlantType::Carrot) {
            itemId = "carrot";
        } else if (type == WildPlantType::Bean) {
            itemId = "bean";
        }
    }
    
    if (!itemId.empty()) {
        std::string itemName = itemId;
        if (itemId == "carrot") itemName = "胡萝卜";
        else if (itemId == "bean") itemName = "豆子";
        
        // 从 prop 读取 count_min 和 count_max
        int minCount = prop->countMin > 0 ? prop->countMin : 1;
        int maxCount = prop->countMax > 0 ? prop->countMax : 2;
        
        dropInfo = PlantDropInfo(itemId, itemName, minCount, maxCount);
        
        std::cout << "[WildPlant] Drop info set: " << itemId << " x" << minCount << "-" << maxCount << std::endl;
    }
    
    // 设置允许拾取 - 如果有掉落物品或显式设置了allow_pick，就允许拾取
    allowPickup = prop->allowPickup || !dropInfo.itemId.empty();
    
    std::cout << "[WildPlant] " << name << " allowPickup=" << (allowPickup ? "true" : "false") 
              << " itemId=" << dropInfo.itemId << std::endl;
    
    // 设置生成概率
    probability = prop->probability;
    
    // 设置大小
    size = sf::Vector2f(32, 32);  // 默认32x32
    
    updateSprite();
}

void WildPlant::setTextureFromProperty(const TileProperty* prop) {
    if (!prop || !prop->hasTexture || !prop->texture) {
        return;
    }
    
    texture = *prop->texture;
    sprite.setTexture(texture);
    textureLoaded = true;
    std::cout << "[WildPlant] Loaded texture: " << prop->name << std::endl;
    updateSprite();
}

bool WildPlant::loadTexture(const std::string& texturePath) {
    if (!texture.loadFromFile(texturePath)) {
        std::cerr << "WildPlant: 无法加载贴图 " << texturePath << std::endl;
        textureLoaded = false;
        return false;
    }
    sprite.setTexture(texture);
    textureLoaded = true;
    updateSprite();
    return true;
}

// ============================================================================
// 更新和渲染
// ============================================================================

void WildPlant::update(float dt) {
    (void)dt;  // 植物目前不需要更新逻辑
}

void WildPlant::render(sf::RenderWindow& window) {
    if (isPickedUp) return;  // 已被拾取则不渲染
    
    if (textureLoaded) {
        window.draw(sprite);
    } else {
        // 没有贴图时画一个颜色块
        sf::RectangleShape placeholder(size);
        placeholder.setPosition(position.x, position.y - size.y);
        
        // 根据类型设置颜色
        switch (type) {
            case WildPlantType::Carrot:
                placeholder.setFillColor(sf::Color(255, 140, 0));  // 橙色
                break;
            case WildPlantType::Bean:
                placeholder.setFillColor(sf::Color(50, 180, 50));  // 绿色
                break;
            default:
                placeholder.setFillColor(sf::Color(100, 200, 100));
                break;
        }
        
        // 悬浮高亮
        if (isHovered) {
            placeholder.setOutlineThickness(2);
            placeholder.setOutlineColor(sf::Color::Yellow);
        }
        
        window.draw(placeholder);
    }
}

void WildPlant::updateSprite() {
    if (!textureLoaded) return;
    
    // 设置精灵位置（底部对齐）
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
// 拾取系统
// ============================================================================

std::vector<std::pair<std::string, int>> WildPlant::pickup() {
    std::vector<std::pair<std::string, int>> drops;
    
    std::cout << "[WildPlant::pickup] Attempting pickup: " << name 
              << " canPickup=" << (canPickup() ? "true" : "false")
              << " allowPickup=" << (allowPickup ? "true" : "false")
              << " isPickedUp=" << (isPickedUp ? "true" : "false")
              << " itemId=" << dropInfo.itemId << std::endl;
    
    if (!canPickup()) {
        std::cout << "[WildPlant::pickup] Cannot pickup!" << std::endl;
        return drops;
    }
    
    // 随机数量
    std::uniform_int_distribution<int> dist(dropInfo.countMin, dropInfo.countMax);
    int count = dist(rng);
    
    if (count > 0 && !dropInfo.itemId.empty()) {
        drops.push_back({dropInfo.itemId, count});
        std::cout << "[WildPlant::pickup] Generated drop: " << dropInfo.itemId << " x" << count << std::endl;
    } else {
        std::cout << "[WildPlant::pickup] No drops generated! count=" << count << " itemId=" << dropInfo.itemId << std::endl;
    }
    
    // 标记为已拾取
    isPickedUp = true;
    
    // 触发回调
    if (onPickup) {
        onPickup(*this);
    }
    
    std::cout << "拾取了 " << name << " x" << count << std::endl;
    
    return drops;
}

void WildPlant::setPickupItem(const std::string& itemId, int minCount, int maxCount) {
    dropInfo.itemId = itemId;
    dropInfo.countMin = minCount;
    dropInfo.countMax = maxCount;
}

// ============================================================================
// 位置和碰撞
// ============================================================================

void WildPlant::setPosition(float x, float y) {
    position = sf::Vector2f(x, y);
    updateSprite();
}

void WildPlant::setPosition(const sf::Vector2f& pos) {
    position = pos;
    updateSprite();
}

sf::FloatRect WildPlant::getBounds() const {
    return sf::FloatRect(position.x, position.y - size.y, size.x, size.y);
}

sf::FloatRect WildPlant::getCollisionBox() const {
    if (hasCustomCollision) {
        return sf::FloatRect(
            position.x + collisionBox.left,
            position.y - size.y + collisionBox.top,
            collisionBox.width,
            collisionBox.height
        );
    }
    
    // 默认碰撞盒（比视觉范围小一些）
    sf::FloatRect bounds = getBounds();
    float shrink = 0.2f;
    return sf::FloatRect(
        bounds.left + bounds.width * shrink / 2,
        bounds.top + bounds.height * shrink,
        bounds.width * (1 - shrink),
        bounds.height * (1 - shrink)
    );
}

bool WildPlant::containsPoint(const sf::Vector2f& point) const {
    return getBounds().contains(point);
}

bool WildPlant::intersects(const sf::FloatRect& rect) const {
    return getCollisionBox().intersects(rect);
}

// ============================================================================
// WildPlantManager 实现
// ============================================================================

WildPlantManager::WildPlantManager()
    : fontLoaded(false)
    , hoveredPlant(nullptr)
{
}

bool WildPlantManager::init(const std::string& assetsPath) {
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

bool WildPlantManager::loadFont(const std::string& fontPath) {
    if (font.loadFromFile(fontPath)) {
        fontLoaded = true;
        std::cout << "[WildPlantManager] Font loaded: " << fontPath << std::endl;
        return true;
    }
    return false;
}

void WildPlantManager::update(float dt) {
    for (auto& plant : plants) {
        if (plant && !plant->isCollected()) {
            plant->update(dt);
        }
    }
}

void WildPlantManager::render(sf::RenderWindow& window, const sf::View& view) {
    sf::FloatRect viewBounds(
        view.getCenter().x - view.getSize().x / 2 - 100,
        view.getCenter().y - view.getSize().y / 2 - 100,
        view.getSize().x + 200,
        view.getSize().y + 200
    );
    
    for (auto& plant : plants) {
        if (plant && !plant->isCollected()) {
            sf::FloatRect bounds = plant->getBounds();
            if (viewBounds.intersects(bounds)) {
                plant->render(window);
            }
        }
    }
}

void WildPlantManager::renderTooltips(sf::RenderWindow& window, const sf::Vector2f& mouseWorldPos) {
    updateHover(mouseWorldPos);
    
    if (hoveredPlant && fontLoaded) {
        renderTooltip(window, hoveredPlant);
    }
}

void WildPlantManager::renderTooltip(sf::RenderWindow& window, WildPlant* plant) {
    if (!plant || !fontLoaded) return;
    
    // 获取鼠标屏幕位置
    sf::Vector2i mouseScreenPos = sf::Mouse::getPosition(window);
    float tooltipX = mouseScreenPos.x + 20.0f;
    float tooltipY = mouseScreenPos.y + 20.0f;
    
    std::vector<sf::String> lines;
    
    // 标题（名称）
    lines.push_back(sf::String::fromUtf8(plant->getName().begin(), plant->getName().end()));
    
    // 类型
    std::string typeStr = std::string(u8"类型: ") + plant->getPlantType();
    lines.push_back(sf::String::fromUtf8(typeStr.begin(), typeStr.end()));
    
    // 空行
    lines.push_back("");
    
    // 掉落物品
    std::string dropTitleStr = std::string(u8"== 可拾取 ==");
    lines.push_back(sf::String::fromUtf8(dropTitleStr.begin(), dropTitleStr.end()));
    
    const auto& dropInfo = plant->getDropInfo();
    if (!dropInfo.itemId.empty()) {
        std::ostringstream itemStream;
        itemStream << "  - " << dropInfo.name;
        itemStream << " x" << dropInfo.countMin << "-" << dropInfo.countMax;
        std::string itemStr = itemStream.str();
        lines.push_back(sf::String::fromUtf8(itemStr.begin(), itemStr.end()));
    }
    
    // 空行
    lines.push_back("");
    
    // 操作提示
    std::string tipStr = std::string(u8"[V] 拾取");
    lines.push_back(sf::String::fromUtf8(tipStr.begin(), tipStr.end()));
    
    // 计算提示框尺寸
    float lineHeight = 28.0f;
    float padding = 16.0f;
    float minWidth = 200.0f;
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
    bg.setFillColor(sf::Color(20, 30, 20, 240));
    bg.setOutlineThickness(3);
    bg.setOutlineColor(sf::Color(80, 140, 80));
    window.draw(bg);
    
    // 绘制标题栏
    sf::RectangleShape titleBar(sf::Vector2f(tooltipWidth, lineHeight + 8));
    titleBar.setPosition(tooltipX, tooltipY);
    titleBar.setFillColor(sf::Color(40, 70, 40, 200));
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
            text.setFillColor(sf::Color(150, 255, 150));  // 标题绿色
            text.setCharacterSize(20);
        } else if (lines[i].toAnsiString().find("==") != std::string::npos) {
            text.setFillColor(sf::Color(200, 255, 150));  // 分隔线浅绿
        } else if (lines[i].toAnsiString().find("[V]") != std::string::npos) {
            text.setFillColor(sf::Color(255, 255, 100));  // 操作提示黄色
        } else if (lines[i].toAnsiString().find("  -") != std::string::npos) {
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

WildPlant* WildPlantManager::addPlant(float x, float y, const std::string& type) {
    auto plant = std::make_unique<WildPlant>(x, y, type);
    WildPlant* ptr = plant.get();
    plants.push_back(std::move(plant));
    return ptr;
}

WildPlant* WildPlantManager::addPlantFromProperty(float x, float y, const TileProperty* prop) {
    auto plant = std::make_unique<WildPlant>();
    plant->initFromTileProperty(x, y, prop);
    
    if (prop) {
        plant->setTextureFromProperty(prop);
    }
    
    WildPlant* ptr = plant.get();
    plants.push_back(std::move(plant));
    return ptr;
}

void WildPlantManager::removePlant(WildPlant* plant) {
    auto it = std::find_if(plants.begin(), plants.end(),
        [plant](const std::unique_ptr<WildPlant>& p) { return p.get() == plant; });
    if (it != plants.end()) {
        plants.erase(it);
    }
}

void WildPlantManager::clearAllPlants() {
    plants.clear();
    hoveredPlant = nullptr;
}

void WildPlantManager::loadFromMapObjects(const std::vector<MapObject>& objects, float displayScale) {
    for (const auto& obj : objects) {
        if (obj.tileProperty) {
            // 检查是否是野生植物类型
            if (obj.tileProperty->type == "wild_plants" || 
                obj.tileProperty->type == "plant" ||
                obj.tileProperty->name.find("carrot") != std::string::npos ||
                obj.tileProperty->name.find("bean") != std::string::npos) {
                
                float x = obj.x * displayScale;
                float y = obj.y * displayScale;
                
                WildPlant* plant = addPlantFromProperty(x, y, obj.tileProperty);
                if (plant) {
                    plant->setSize(obj.width * displayScale, obj.height * displayScale);
                }
            }
        }
    }
    
    std::cout << "[WildPlantManager] Loaded " << plants.size() << " wild plants from map" << std::endl;
}

WildPlant* WildPlantManager::getPlantAt(const sf::Vector2f& position) {
    for (auto& plant : plants) {
        if (plant && !plant->isCollected() && plant->containsPoint(position)) {
            return plant.get();
        }
    }
    return nullptr;
}

WildPlant* WildPlantManager::getPlantInRect(const sf::FloatRect& rect) {
    for (auto& plant : plants) {
        if (plant && !plant->isCollected() && plant->intersects(rect)) {
            return plant.get();
        }
    }
    return nullptr;
}

WildPlant* WildPlantManager::getPickablePlantInRange(const sf::Vector2f& center, float range) {
    WildPlant* closest = nullptr;
    float closestDist = range * range;
    
    std::cout << "[DEBUG] getPickablePlantInRange: checking " << plants.size() << " plants" << std::endl;
    
    for (auto& plant : plants) {
        if (plant) {
            sf::Vector2f plantPos = plant->getPosition();
            float dx = plantPos.x - center.x;
            float dy = plantPos.y - center.y;
            float distSq = dx * dx + dy * dy;
            
            std::cout << "[DEBUG] Plant '" << plant->getName() 
                      << "' collected=" << (plant->isCollected() ? "true" : "false")
                      << " canPickup=" << (plant->canPickup() ? "true" : "false")
                      << " dist=" << std::sqrt(distSq) << std::endl;
            
            if (!plant->isCollected() && plant->canPickup()) {
                if (distSq < closestDist) {
                    closestDist = distSq;
                    closest = plant.get();
                }
            }
        }
    }
    
    return closest;
}

std::vector<std::pair<std::string, int>> WildPlantManager::pickupPlant(WildPlant* plant) {
    if (!plant || plant->isCollected()) {
        return {};
    }
    return plant->pickup();
}

void WildPlantManager::updateHover(const sf::Vector2f& mouseWorldPos) {
    hoveredPlant = nullptr;
    
    for (auto& plant : plants) {
        if (plant && !plant->isCollected()) {
            bool hovered = plant->containsPoint(mouseWorldPos);
            plant->setHovered(hovered);
            if (hovered) {
                hoveredPlant = plant.get();
            }
        }
    }
}

bool WildPlantManager::isCollidingWithAnyPlant(const sf::FloatRect& rect) const {
    for (const auto& plant : plants) {
        if (plant && !plant->isCollected()) {
            if (plant->intersects(rect)) {
                return true;
            }
        }
    }
    return false;
}

void WildPlantManager::removePickedPlants() {
    plants.erase(
        std::remove_if(plants.begin(), plants.end(),
            [](const std::unique_ptr<WildPlant>& p) { 
                return p && p->isCollected(); 
            }),
        plants.end()
    );
    
    if (hoveredPlant && hoveredPlant->isCollected()) {
        hoveredPlant = nullptr;
    }
}
