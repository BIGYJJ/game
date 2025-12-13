#include "Tree.h"
#include "../World/TileMap.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <sstream>

// ============================================================================
// Tree 构造函数
// ============================================================================

Tree::Tree()
    : treeType("oak")
    , name("橡树")
    , health(30.0f)
    , maxHealth(30.0f)
    , defense(5.0f)
    , position(0, 0)
    , size(64, 64)
    , growthStage(TreeGrowthStage::Mature)
    , growthTimer(0.0f)
    , seedlingTime(60.0f)       // 1分钟
    , growingTime(120.0f)       // 2分钟
    , matureTime(180.0f)        // 3分钟结果
    , fruitRegrowTime(60.0f)    // 1分钟果实再生
    , currentTexture(nullptr)
    , texturesLoaded(false)
    , isHovered(false)
    , shakeTimer(0.0f)
    , shakeIntensity(0.0f)
    , canTransform(false)       // 新增：是否可以变换
    , hasTransformed(false)     // 新增：是否已经变换过
    , onDestroyed(nullptr)
    , onFruitHarvested(nullptr)
    , onGrowthStageChanged(nullptr)
{
    // 默认掉落物品
    dropItems.push_back(DropItem("wood", "木材", 2, 5, 1.0f));
    dropItems.push_back(DropItem("stick", "树枝", 1, 3, 0.8f));
    dropItems.push_back(DropItem("seed", "种子", 0, 2, 0.3f));
}

Tree::Tree(float x, float y, const std::string& type)
    : Tree()
{
    init(x, y, type);
}

void Tree::init(float x, float y, const std::string& type) {
    position = sf::Vector2f(x, y);
    treeType = type;
    
    // 根据类型设置属性
    if (type == "oak" || type == "tree1") {
        name = "橡树";
        maxHealth = 30.0f;
        defense = 5.0f;
        size = sf::Vector2f(64, 64);
        canTransform = true;  // 普通树可以变换成果树
        hasTransformed = false;
    } else if (type == "pine") {
        name = "松树";
        maxHealth = 40.0f;
        defense = 8.0f;
        size = sf::Vector2f(64, 96);
        canTransform = false;
    } else if (type == "apple_tree") {
        name = "苹果树";
        maxHealth = 25.0f;
        defense = 3.0f;
        size = sf::Vector2f(64, 64);
        canTransform = false;
        // 苹果树的果实掉落
        fruitDropItems.push_back(DropItem("apple", "苹果", 1, 3, 1.0f));
    } else if (type == "cherry_tree") {
        name = "樱桃树";
        maxHealth = 20.0f;
        defense = 2.0f;
        size = sf::Vector2f(64, 64);
        canTransform = false;
        fruitDropItems.push_back(DropItem("cherry", "樱桃", 2, 5, 1.0f));
    }
    
    health = maxHealth;
    updateSprite();
}

bool Tree::loadTextures(const std::string& basePath) {
    std::cout << "[Tree] loadTextures called for type: " << treeType 
              << " basePath: " << basePath << std::endl;
    
    bool loaded = false;
    
    // 根据树类型构建贴图路径
    std::vector<std::string> texturePaths;
    
    if (treeType == "apple") {
        texturePaths = {
            basePath + "/apple_tree.png",
            basePath + "/apple.png",
            "../../assets/game_source/tree/apple_tree.png",
            "assets/game_source/tree/apple_tree.png"
        };
    } else if (treeType == "cherry") {
        texturePaths = {
            basePath + "/cherry_tree.png",
            basePath + "/cherry.png",
            "../../assets/game_source/tree/cherry_tree.png",
            "assets/game_source/tree/cherry_tree.png"
        };
    } else {
        // tree1, oak 等普通树
        texturePaths = {
            basePath + "/tree.png",
            basePath + "/tree1.png",
            basePath + "/oak.png",
            "../../assets/game_source/tree/tree.png",
            "assets/game_source/tree/tree.png"
        };
    }
    
    // 尝试加载贴图
    for (const auto& path : texturePaths) {
        if (textureMature.loadFromFile(path)) {
            loaded = true;
            std::cout << "[Tree] Loaded texture: " << path << std::endl;
            break;
        }
    }
    
    if (!loaded) {
        // 创建占位贴图
        sf::Image placeholder;
        if (treeType == "apple_tree") {
            placeholder.create(64, 64, sf::Color(255, 100, 100));  // 红色代表苹果树
        } else if (treeType == "cherry_tree") {
            placeholder.create(64, 64, sf::Color(255, 150, 200));  // 粉色代表樱桃树
        } else {
            placeholder.create(64, 64, sf::Color(34, 139, 34));    // 绿色代表普通树
        }
        textureMature.loadFromImage(placeholder);
        std::cout << "[Tree] Using placeholder texture for: " << treeType << std::endl;
    }
    
    // 其他阶段贴图使用成熟贴图
    textureSeedling = textureMature;
    textureGrowing = textureMature;
    textureFruiting = textureMature;
    
    texturesLoaded = true;
    updateSprite();
    
    return loaded;
}

// ============================================================================
// 变换树类型（普通树变成果树）
// ============================================================================

void Tree::transformToFruitTree() {
    if (!canTransform || hasTransformed) return;
    
    // 随机选择变成苹果树或樱桃树
    int choice = rand() % 2;
    std::string newType = (choice == 0) ? "apple" : "cherry";
    
    std::cout << "[Tree] " << name << " transforms to " << newType << " tree!" << std::endl;
    
    // 保存位置和尺寸
    sf::Vector2f savedPos = position;
    sf::Vector2f savedSize = size;
    
    // 重新初始化为新类型
    treeType = newType;
    
    if (newType == "apple_tree") {
        name = "苹果树";
        maxHealth = 25.0f;
        defense = 3.0f;
        fruitDropItems.clear();
        fruitDropItems.push_back(DropItem("apple", "苹果", 1, 3, 1.0f));
    } else if (newType == "cherry_tree"){
        name = "樱桃树";
        maxHealth = 20.0f;
        defense = 2.0f;
        fruitDropItems.clear();
        fruitDropItems.push_back(DropItem("cherry", "樱桃", 2, 5, 1.0f));
    }
    
    health = maxHealth;
    hasTransformed = true;
    canTransform = false;
    
    // 恢复位置和尺寸
    position = savedPos;
    size = savedSize;
    
    // 重新加载贴图
    // 注意：这里需要知道 basePath，我们用一个常用路径
    loadTextures("../../assets/game_source/tree");
    
    // 设置为结果阶段
    growthStage = TreeGrowthStage::Fruiting;
    updateSprite();
}

// ============================================================================
// 更新
// ============================================================================

void Tree::update(float dt) {
    // 生长
    grow(dt);
    
    // 震动效果
    if (shakeTimer > 0) {
        shakeTimer -= dt;
        if (shakeTimer <= 0) {
            shakeTimer = 0;
            shakeIntensity = 0;
        }
    }
    
    // 更新掉落粒子
    updateDropParticles(dt);
}

void Tree::grow(float dt) {
    growthTimer += dt;
    
    TreeGrowthStage oldStage = growthStage;
    
    switch (growthStage) {
        case TreeGrowthStage::Seedling:
            if (growthTimer >= seedlingTime) {
                growthTimer = 0;
                growthStage = TreeGrowthStage::Growing;
            }
            break;
            
        case TreeGrowthStage::Growing:
            if (growthTimer >= growingTime) {
                growthTimer = 0;
                growthStage = TreeGrowthStage::Mature;
            }
            break;
            
        case TreeGrowthStage::Mature:
            // 普通树成熟后可以变换成果树
            if (canTransform && !hasTransformed && growthTimer >= matureTime) {
                growthTimer = 0;
                transformToFruitTree();  // 变换成苹果树或樱桃树
                return;  // 变换后直接返回，避免重复处理
            }
            // 如果是果树，检查是否应该结果
            else if (!fruitDropItems.empty() && growthTimer >= matureTime) {
                growthTimer = 0;
                growthStage = TreeGrowthStage::Fruiting;
            }
            break;
            
        case TreeGrowthStage::Fruiting:
            // 果实阶段不自动变化，等待采摘
            break;
    }
    
    // 阶段变化回调
    if (oldStage != growthStage) {
        updateSprite();
        if (onGrowthStageChanged) {
            onGrowthStageChanged(*this);
        }
    }
}

void Tree::setGrowthStage(TreeGrowthStage stage) {
    if (growthStage != stage) {
        growthStage = stage;
        growthTimer = 0;
        updateSprite();
        if (onGrowthStageChanged) {
            onGrowthStageChanged(*this);
        }
    }
}

float Tree::getGrowthProgress() const {
    switch (growthStage) {
        case TreeGrowthStage::Seedling:
            return seedlingTime > 0 ? growthTimer / seedlingTime : 1.0f;
        case TreeGrowthStage::Growing:
            return growingTime > 0 ? growthTimer / growingTime : 1.0f;
        case TreeGrowthStage::Mature:
            return matureTime > 0 ? growthTimer / matureTime : 1.0f;
        case TreeGrowthStage::Fruiting:
            return 1.0f;
    }
    return 0.0f;
}

std::string Tree::getGrowthStageName() const {
    switch (growthStage) {
        case TreeGrowthStage::Seedling: return "幼苗";
        case TreeGrowthStage::Growing:  return "成长中";
        case TreeGrowthStage::Mature:   return "成熟";
        case TreeGrowthStage::Fruiting: return "有果实";
    }
    return "未知";
}

// ============================================================================
// 渲染
// ============================================================================

void Tree::render(sf::RenderWindow& window) {
    if (!texturesLoaded) return;
    
    sf::Vector2f renderPos = position;
    
    // 震动效果
    if (shakeTimer > 0 && shakeIntensity > 0) {
        float shake = std::sin(shakeTimer * 30.0f) * shakeIntensity;
        renderPos.x += shake;
    }
    
    sprite.setPosition(renderPos.x, renderPos.y - size.y);
    
    // 悬浮高亮效果
    if (isHovered) {
        sprite.setColor(sf::Color(255, 255, 200));  // 轻微黄色高亮
    } else {
        sprite.setColor(sf::Color::White);
    }
    
    window.draw(sprite);
}

void Tree::renderDropParticles(sf::RenderWindow& window) {
    for (const auto& particle : dropParticles) {
        if (!particle.active) continue;
        
        // 简单的方块粒子
        sf::RectangleShape shape(sf::Vector2f(8, 8));
        shape.setPosition(particle.position);
        shape.setFillColor(sf::Color(139, 90, 43, 
            static_cast<sf::Uint8>(255 * (particle.lifetime / particle.maxLifetime))));
        window.draw(shape);
    }
}

void Tree::updateSprite() {
    sf::Texture* tex = nullptr;
    
    switch (growthStage) {
        case TreeGrowthStage::Seedling: tex = &textureSeedling; break;
        case TreeGrowthStage::Growing:  tex = &textureGrowing; break;
        case TreeGrowthStage::Mature:   tex = &textureMature; break;
        case TreeGrowthStage::Fruiting: tex = &textureFruiting; break;
    }
    
    if (tex && tex->getSize().x > 0) {
        currentTexture = tex;
        sprite.setTexture(*currentTexture);
        
        // 根据尺寸缩放
        sf::Vector2u texSize = currentTexture->getSize();
        sprite.setScale(size.x / texSize.x, size.y / texSize.y);
    }
}

// ============================================================================
// 交互
// ============================================================================

bool Tree::takeDamage(float damage) {
    // 计算实际伤害
    float actualDamage = std::max(1.0f, damage - defense);
    health -= actualDamage;
    
    // 触发震动效果
    shakeTimer = 0.3f;
    shakeIntensity = 3.0f;
    
    std::cout << "[Tree] " << name << " took " << actualDamage 
              << " damage, HP: " << health << "/" << maxHealth << std::endl;
    
    if (health <= 0) {
        health = 0;
        // 生成掉落粒子
        spawnDropParticles(position, 5);
        
        if (onDestroyed) {
            onDestroyed(*this);
        }
        return true;  // 树木被砍倒
    }
    
    return false;
}

bool Tree::harvestFruit() {
    if (growthStage != TreeGrowthStage::Fruiting) {
        return false;
    }
    
    // 生成果实掉落粒子
    spawnDropParticles(sf::Vector2f(position.x, position.y - size.y * 0.5f), 3);
    
    // 回到成熟阶段，开始重新结果
    growthStage = TreeGrowthStage::Mature;
    growthTimer = 0;
    updateSprite();
    
    if (onFruitHarvested) {
        onFruitHarvested(*this);
    }
    
    std::cout << "[Tree] Harvested fruit from " << name << std::endl;
    return true;
}

bool Tree::containsPoint(const sf::Vector2f& point) const {
    sf::FloatRect bounds = getBounds();
    return bounds.contains(point);
}

bool Tree::intersects(const sf::FloatRect& rect) const {
    return getCollisionBox().intersects(rect);
}

sf::FloatRect Tree::getBounds() const {
    return sf::FloatRect(position.x, position.y - size.y, size.x, size.y);
}

sf::FloatRect Tree::getCollisionBox() const {
    // 碰撞盒只取树干部分（下半部分，更窄）
    float collisionWidth = size.x * 0.5f;
    float collisionHeight = size.y * 0.4f;
    return sf::FloatRect(
        position.x + (size.x - collisionWidth) / 2.0f,
        position.y - collisionHeight,
        collisionWidth,
        collisionHeight
    );
}

// ============================================================================
// 掉落物品
// ============================================================================

void Tree::addDropItem(const DropItem& item) {
    dropItems.push_back(item);
}

void Tree::clearDropItems() {
    dropItems.clear();
}

void Tree::addFruitDropItem(const DropItem& item) {
    fruitDropItems.push_back(item);
}

std::vector<std::pair<std::string, int>> Tree::generateDrops() {
    std::vector<std::pair<std::string, int>> result;
    
    for (const auto& item : dropItems) {
        float roll = static_cast<float>(rand()) / RAND_MAX;
        if (roll <= item.dropChance) {
            int count = item.minCount + rand() % (item.maxCount - item.minCount + 1);
            if (count > 0) {
                result.push_back({item.itemId, count});
                std::cout << "[Tree] Dropped: " << item.name << " x" << count << std::endl;
            }
        }
    }
    
    return result;
}

std::vector<std::pair<std::string, int>> Tree::generateFruitDrops() {
    std::vector<std::pair<std::string, int>> result;
    
    for (const auto& item : fruitDropItems) {
        float roll = static_cast<float>(rand()) / RAND_MAX;
        if (roll <= item.dropChance) {
            int count = item.minCount + rand() % (item.maxCount - item.minCount + 1);
            if (count > 0) {
                result.push_back({item.itemId, count});
                std::cout << "[Tree] Fruit dropped: " << item.name << " x" << count << std::endl;
            }
        }
    }
    
    return result;
}

// ============================================================================
// 掉落粒子
// ============================================================================

void Tree::spawnDropParticles(const sf::Vector2f& pos, int count) {
    for (int i = 0; i < count; i++) {
        DropParticle particle;
        particle.position = pos;
        particle.velocity = sf::Vector2f(
            (rand() % 100 - 50) * 2.0f,  // -100 to 100
            -150.0f - rand() % 100       // -150 to -250 (向上)
        );
        particle.lifetime = 1.0f;
        particle.maxLifetime = 1.0f;
        particle.active = true;
        dropParticles.push_back(particle);
    }
}

void Tree::updateDropParticles(float dt) {
    for (auto& particle : dropParticles) {
        if (!particle.active) continue;
        
        particle.lifetime -= dt;
        if (particle.lifetime <= 0) {
            particle.active = false;
            continue;
        }
        
        // 重力
        particle.velocity.y += 500.0f * dt;
        particle.position += particle.velocity * dt;
    }
    
    // 移除非活动粒子
    dropParticles.erase(
        std::remove_if(dropParticles.begin(), dropParticles.end(),
            [](const DropParticle& p) { return !p.active; }),
        dropParticles.end()
    );
}

// ============================================================================
// TreeManager 实现
// ============================================================================

TreeManager::TreeManager()
    : fontLoaded(false)
    , hoveredTree(nullptr)
{
}

bool TreeManager::init(const std::string& assetsPath) {
    assetsBasePath = assetsPath;
    
    // 尝试加载字体
    std::vector<std::string> fontPaths = {
        assetsPath + "/fonts/pixel.ttf",
        "../../assets/fonts/font.ttf",
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/simhei.ttf"
    };
    
    for (const auto& path : fontPaths) {
        if (font.loadFromFile(path)) {
            fontLoaded = true;
            std::cout << "[TreeManager] Font loaded: " << path << std::endl;
            break;
        }
    }
    
    return true;
}

bool TreeManager::loadFont(const std::string& fontPath) {
    if (font.loadFromFile(fontPath)) {
        fontLoaded = true;
        return true;
    }
    return false;
}

void TreeManager::update(float dt) {
    // 更新所有树木
    for (auto it = trees.begin(); it != trees.end(); ) {
        (*it)->update(dt);
        ++it;
    }
}

void TreeManager::render(sf::RenderWindow& window, const sf::View& view) {
    // 简单的视口裁剪
    sf::FloatRect viewBounds(
        view.getCenter().x - view.getSize().x / 2 - 100,
        view.getCenter().y - view.getSize().y / 2 - 100,
        view.getSize().x + 200,
        view.getSize().y + 200
    );
    
    for (auto& tree : trees) {
        if (!tree->isDead()) {
            sf::FloatRect treeBounds = tree->getBounds();
            if (viewBounds.intersects(treeBounds)) {
                tree->render(window);
            }
        }
        // 渲染掉落粒子
        tree->renderDropParticles(window);
    }
}

void TreeManager::renderTooltips(sf::RenderWindow& window, const sf::Vector2f& mouseWorldPos) {
    // 更新悬浮状态
    updateHover(mouseWorldPos);
    
    // 渲染悬浮提示
    if (hoveredTree && !hoveredTree->isDead()) {
        renderTooltip(window, hoveredTree);
    }
}

void TreeManager::renderTooltip(sf::RenderWindow& window, Tree* tree) {
    if (!fontLoaded || !tree) return;
    
    // 获取鼠标屏幕位置
    sf::Vector2i mouseScreenPos = sf::Mouse::getPosition(window);
    float tooltipX = mouseScreenPos.x + 15.0f;
    float tooltipY = mouseScreenPos.y + 15.0f;
    
    // 构建提示内容
    std::vector<std::wstring> lines;
    lines.push_back(std::wstring(tree->getName().begin(), tree->getName().end()));
    
    // 生长状态
    std::wstring stageLine = L"状态: ";
    std::string stageName = tree->getGrowthStageName();
    stageLine += std::wstring(stageName.begin(), stageName.end());
    lines.push_back(stageLine);
    
    // 生命值
    std::wstringstream hpStream;
    hpStream << L"生命: " << (int)tree->getHealth() << L"/" << (int)tree->getMaxHealth();
    lines.push_back(hpStream.str());
    
    // 防御
    std::wstringstream defStream;
    defStream << L"防御: " << (int)tree->getDefense();
    lines.push_back(defStream.str());
    
    // 掉落物品
    lines.push_back(L"掉落:");
    for (const auto& item : tree->getDropItems()) {
        std::wstringstream itemStream;
        itemStream << L"  " << std::wstring(item.name.begin(), item.name.end());
        itemStream << L" x" << item.minCount << L"-" << item.maxCount;
        lines.push_back(itemStream.str());
    }
    
    // 果实
    if (tree->hasFruit()) {
        lines.push_back(L"果实:");
        for (const auto& item : tree->getFruitDropItems()) {
            std::wstringstream itemStream;
            itemStream << L"  " << std::wstring(item.name.begin(), item.name.end());
            lines.push_back(itemStream.str());
        }
    }
    
    // 计算提示框尺寸
    float lineHeight = 22.0f;
    float padding = 10.0f;
    float maxWidth = 0.0f;
    
    sf::Text measureText;
    measureText.setFont(font);
    measureText.setCharacterSize(16);
    
    for (const auto& line : lines) {
        measureText.setString(line);
        float width = measureText.getLocalBounds().width;
        if (width > maxWidth) maxWidth = width;
    }
    
    float tooltipWidth = maxWidth + padding * 2;
    float tooltipHeight = lines.size() * lineHeight + padding * 2;
    
    // 确保不超出屏幕
    sf::Vector2u windowSize = window.getSize();
    if (tooltipX + tooltipWidth > windowSize.x) {
        tooltipX = windowSize.x - tooltipWidth - 5;
    }
    if (tooltipY + tooltipHeight > windowSize.y) {
        tooltipY = windowSize.y - tooltipHeight - 5;
    }
    
    // 绘制背景
    sf::RectangleShape bg(sf::Vector2f(tooltipWidth, tooltipHeight));
    bg.setPosition(tooltipX, tooltipY);
    bg.setFillColor(sf::Color(20, 20, 30, 230));
    bg.setOutlineThickness(2.0f);
    bg.setOutlineColor(sf::Color(100, 80, 60));
    window.draw(bg);
    
    // 绘制文字
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(16);
    
    float y = tooltipY + padding;
    for (size_t i = 0; i < lines.size(); i++) {
        text.setString(lines[i]);
        text.setPosition(tooltipX + padding, y);
        
        // 标题用金色
        if (i == 0) {
            text.setFillColor(sf::Color(255, 215, 0));
            text.setCharacterSize(18);
        } else {
            text.setFillColor(sf::Color(220, 220, 220));
            text.setCharacterSize(16);
        }
        
        window.draw(text);
        y += lineHeight;
    }
    
    // 绘制血条
    float barX = tooltipX + padding;
    float barY = tooltipY + padding + lineHeight * 2.5f;
    float barWidth = tooltipWidth - padding * 2;
    float barHeight = 8.0f;
    
    // 背景
    sf::RectangleShape barBg(sf::Vector2f(barWidth, barHeight));
    barBg.setPosition(barX, barY);
    barBg.setFillColor(sf::Color(40, 40, 40));
    window.draw(barBg);
    
    // 血条
    sf::RectangleShape barFill(sf::Vector2f(barWidth * tree->getHealthPercent(), barHeight));
    barFill.setPosition(barX, barY);
    barFill.setFillColor(sf::Color(220, 60, 60));
    window.draw(barFill);
}

Tree* TreeManager::addTree(float x, float y, const std::string& type) {
    auto tree = std::make_unique<Tree>(x, y, type);
    tree->loadTextures(assetsBasePath + "/game_source/tree");
    
    Tree* ptr = tree.get();
    trees.push_back(std::move(tree));
    
    std::cout << "[TreeManager] Added " << type << " tree at (" << x << ", " << y << ")" << std::endl;
    return ptr;
}

void TreeManager::removeTree(Tree* tree) {
    trees.erase(
        std::remove_if(trees.begin(), trees.end(),
            [tree](const std::unique_ptr<Tree>& t) { return t.get() == tree; }),
        trees.end()
    );
}

void TreeManager::clearAllTrees() {
    trees.clear();
}

void TreeManager::loadFromMapObjects(const std::vector<MapObject>& objects, float displayScale) {
    for (const auto& obj : objects) {
        if (obj.gid > 0) {
            float x = obj.x * displayScale;
            float y = obj.y * displayScale;
            
            // 使用对象的name属性来确定树的类型
            std::string treeType = obj.name.empty() ? "tree1" : obj.name;
            
            Tree* tree = addTree(x, y, treeType);
            if (tree) {
                // 可以从tsx属性读取HP，这里先用默认值
                if (treeType == "apple_tree") {
                    tree->setMaxHealth(40.0f);
                    tree->setHealth(40.0f);
                } else {
                    tree->setMaxHealth(30.0f);
                    tree->setHealth(30.0f);
                }
            }
        }
    }
}

Tree* TreeManager::getTreeAt(const sf::Vector2f& position) {
    for (auto& tree : trees) {
        if (!tree->isDead() && tree->containsPoint(position)) {
            return tree.get();
        }
    }
    return nullptr;
}

Tree* TreeManager::getTreeInRect(const sf::FloatRect& rect) {
    for (auto& tree : trees) {
        if (!tree->isDead() && tree->intersects(rect)) {
            return tree.get();
        }
    }
    return nullptr;
}

std::vector<Tree*> TreeManager::damageTreesInRange(const sf::Vector2f& center, 
                                                    float radius, float damage) {
    std::vector<Tree*> hitTrees;
    
    for (auto& tree : trees) {
        if (tree->isDead()) continue;
        
        sf::Vector2f treeCenter = tree->getPosition();
        treeCenter.y -= tree->getBounds().height / 2;
        
        float dx = treeCenter.x - center.x;
        float dy = treeCenter.y - center.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        if (distance <= radius) {
            tree->takeDamage(damage);
            hitTrees.push_back(tree.get());
        }
    }
    
    return hitTrees;
}

void TreeManager::updateHover(const sf::Vector2f& mouseWorldPos) {
    Tree* newHovered = nullptr;
    
    for (auto& tree : trees) {
        if (!tree->isDead() && tree->containsPoint(mouseWorldPos)) {
            newHovered = tree.get();
            break;
        }
    }
    
    // 更新悬浮状态
    if (hoveredTree != newHovered) {
        if (hoveredTree) {
            hoveredTree->setHovered(false);
        }
        hoveredTree = newHovered;
        if (hoveredTree) {
            hoveredTree->setHovered(true);
        }
    }
}

bool TreeManager::isCollidingWithAnyTree(const sf::FloatRect& rect) const {
    for (const auto& tree : trees) {
        if (!tree->isDead() && tree->intersects(rect)) {
            return true;
        }
    }
    return false;
}