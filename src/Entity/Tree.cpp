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
    , expMin(5)                 // 默认经验范围
    , expMax(12)
    , goldMin(10)               // 默认金币范围
    , goldMax(30)
    , dropMax(3)                // 默认最大掉落数量
    , isHovered(false)
    , shakeTimer(0.0f)
    , shakeIntensity(0.0f)
    , canTransform(false)       // 是否可以变换
    , hasTransformed(false)     // 是否已经变换过
    , onDestroyed(nullptr)
    , onFruitHarvested(nullptr)
    , onGrowthStageChanged(nullptr)
{
    // 默认掉落物品
    dropItems.push_back(DropItem("wood", "木材", 1, 3, 0.75f));
    dropItems.push_back(DropItem("seed", "种子", 1, 2, 0.5f));
    dropItems.push_back(DropItem("stick", "树枝", 1, 2, 0.4f));
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

// ============================================================================
// 从 TileProperty 动态初始化（推荐使用）
// ============================================================================

void Tree::initFromTileProperty(float x, float y, const TileProperty* prop) {
    position = sf::Vector2f(x, y);
    
    if (!prop) {
        // 如果没有属性信息，使用默认值
        treeType = "tree1";
        name = "树木";
        maxHealth = 30.0f;
        defense = 5.0f;
        size = sf::Vector2f(64, 64);
        health = maxHealth;
        updateSprite();
        return;
    }
    
    // 从 TileProperty 设置属性
    treeType = prop->name;
    name = prop->name;  // 可以后续添加中文名映射
    maxHealth = (float)prop->hp;
    defense = (float)prop->defense;
    size = sf::Vector2f(64, 64);  // 默认尺寸，可从外部设置
    
    // 解析经验和金币奖励
    expMin = prop->expMin > 0 ? prop->expMin : 5;
    expMax = prop->expMax > 0 ? prop->expMax : 12;
    goldMin = prop->goldMin > 0 ? prop->goldMin : 10;
    goldMax = prop->goldMax > 0 ? prop->goldMax : 30;
    dropMax = prop->dropMax > 0 ? prop->dropMax : 3;
    
    // 清空并设置掉落物品
    dropItems.clear();
    fruitDropItems.clear();
    
    // ========================================
    // 掉落规则：
    //   如果 tsx 文件设置了 drop_type，使用配置的掉落
    //   否则使用默认掉落（wood, seed, stick）
    //   每个物品的掉落概率由 dropX_Probability 设置
    //   实际掉落数量使用递减概率计算（见 generateDrops）
    // ========================================
    
    if (!prop->dropTypes.empty()) {
        // 使用 TSX 配置的掉落
        for (size_t i = 0; i < prop->dropTypes.size(); i++) {
            const std::string& itemId = prop->dropTypes[i];
            float prob = (i < prop->dropProbabilities.size()) ? prop->dropProbabilities[i] : 0.5f;
            
            // 创建中文名映射
            std::string itemName = itemId;
            if (itemId == "wood") itemName = "木材";
            else if (itemId == "seed") itemName = "种子";
            else if (itemId == "stick") itemName = "树枝";
            else if (itemId == "apple") itemName = "苹果";
            else if (itemId == "cherry") itemName = "樱桃";
            else if (itemId == "stone") itemName = "石头";
            
            // 判断是砍伐掉落还是果实掉落
            if (itemId == "apple" || itemId == "cherry" || 
                itemId.find("fruit") != std::string::npos) {
                fruitDropItems.push_back(DropItem(itemId, itemName, 1, dropMax, prob));
            } else {
                dropItems.push_back(DropItem(itemId, itemName, 1, dropMax, prob));
            }
        }
    } else {
        // 使用默认掉落（当 tsx 没有设置 drop_type 时）
        float defaultProb1 = 0.75f;
        float defaultProb2 = 0.5f;
        float defaultProb3 = 0.4f;
        
        if (prop->dropProbabilities.size() >= 1) defaultProb1 = prop->dropProbabilities[0];
        if (prop->dropProbabilities.size() >= 2) defaultProb2 = prop->dropProbabilities[1];
        if (prop->dropProbabilities.size() >= 3) defaultProb3 = prop->dropProbabilities[2];
        
        dropItems.push_back(DropItem("wood", "木材", 1, dropMax, defaultProb1));
        dropItems.push_back(DropItem("seed", "种子", 1, dropMax, defaultProb2));
        dropItems.push_back(DropItem("stick", "树枝", 1, dropMax, defaultProb3));
    }
    
    // 设置中文显示名
    if (treeType == "tree1") {
        name = "橡树";
        // 不再默认设置 canTransform，由外部控制
    } else if (treeType == "apple_tree") {
        name = "苹果树";
    } else if (treeType == "cherry_tree") {
        name = "樱桃树";
    } else if (treeType == "cherry_blossom_tree" || treeType == "cherry_blossom_tree.png") {
        name = "樱花树";
    } else {
        // 未知类型，使用原始名称
    }
    canTransform = false;  // 默认不变换，由外部设置
    
    health = maxHealth;
    
    std::cout << "[Tree] Initialized from TileProperty: " << name 
              << " HP=" << maxHealth << " DEF=" << defense 
              << " drops=" << dropItems.size() 
              << " fruits=" << fruitDropItems.size() << std::endl;
    
    updateSprite();
}

void Tree::setTextureFromProperty(const TileProperty* prop) {
    if (!prop || !prop->hasTexture || !prop->texture) return;
    
    // 从 shared_ptr 复制贴图（这会创建独立的副本，避免引用失效）
    textureMature = *prop->texture;
    textureSeedling = textureMature;
    textureGrowing = textureMature;
    textureFruiting = textureMature;
    
    texturesLoaded = true;
    updateSprite();
    
    std::cout << "[Tree] Texture set from TileProperty for: " << treeType << std::endl;
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
    
    // ========================================
    // 递减概率掉落计算规则：
    //   每个物品独立计算掉落数量
    //   第1个: 概率 = baseProbability
    //   第2个: 概率 = baseProbability * baseProbability
    //   第3个: 概率 = baseProbability ^ 3
    //   ...以此类推，直到 dropMax 或随机失败
    // ========================================
    
    for (const auto& item : dropItems) {
        int count = 0;
        float currentProbability = item.dropChance;
        
        // 递减概率计算
        for (int i = 0; i < dropMax; i++) {
            float roll = static_cast<float>(rand()) / RAND_MAX;
            if (roll < currentProbability) {
                count++;
                currentProbability *= item.dropChance;  // 概率递减
            } else {
                break;  // 一旦失败就停止
            }
        }
        
        if (count > 0) {
            result.push_back({item.itemId, count});
            std::cout << "[Tree] Dropped: " << item.name << " x" << count 
                      << " (prob=" << (int)(item.dropChance * 100) << "%)" << std::endl;
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

int Tree::getExpReward() const {
    if (expMax <= expMin) return expMin;
    return expMin + rand() % (expMax - expMin + 1);
}

int Tree::getGoldReward() const {
    if (goldMax <= goldMin) return goldMin;
    return goldMin + rand() % (goldMax - goldMin + 1);
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
    // 优先使用系统中文字体，因为 pixel.ttf 可能不支持中文
    std::vector<std::string> fontPaths = {
        "C:/Windows/Fonts/msyh.ttc",        // 微软雅黑（优先）
        "C:/Windows/Fonts/simhei.ttf",      // 黑体
        "C:/Windows/Fonts/simsun.ttc",      // 宋体
        assetsPath + "/fonts/pixel.ttf",
        "../../assets/fonts/pixel.ttf",
        "../../assets/fonts/font.ttf",
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
    float tooltipX = mouseScreenPos.x + 20.0f;
    float tooltipY = mouseScreenPos.y + 20.0f;
    
    // ========================================
    // 使用 sf::String 代替 std::wstring
    // sf::String 可以正确处理 UTF-8 中文
    // ========================================
    
    std::vector<sf::String> lines;
    
    // 标题（树木名称）
    lines.push_back(sf::String::fromUtf8(tree->getName().begin(), tree->getName().end()));
    
    // 类型
    std::string typeStr = "类型: " + tree->getTreeType();
    lines.push_back(sf::String::fromUtf8(typeStr.begin(), typeStr.end()));
    
    // 生长状态
    std::string stageStr = "状态: " + tree->getGrowthStageName();
    lines.push_back(sf::String::fromUtf8(stageStr.begin(), stageStr.end()));
    
    // 空行用于分隔
    lines.push_back("");
    
    // 生命值
    std::ostringstream hpStream;
    hpStream << "生命值: " << (int)tree->getHealth() << " / " << (int)tree->getMaxHealth();
    lines.push_back(sf::String::fromUtf8(hpStream.str().begin(), hpStream.str().end()));
    
    // 防御
    std::ostringstream defStream;
    defStream << "防御力: " << (int)tree->getDefense();
    lines.push_back(sf::String::fromUtf8(defStream.str().begin(), defStream.str().end()));
    
    // 空行用于分隔
    lines.push_back("");
    
    // 掉落物品
    if (!tree->getDropItems().empty()) {
        lines.push_back(sf::String::fromUtf8(std::string("== 砍伐掉落 ==").begin(), std::string("== 砍伐掉落 ==").end()));
        for (const auto& item : tree->getDropItems()) {
            std::ostringstream itemStream;
            itemStream << "  - " << item.name;
            itemStream << " x1-" << tree->getDropMax();
            itemStream << " (" << (int)(item.dropChance * 100) << "%)";
            lines.push_back(sf::String::fromUtf8(itemStream.str().begin(), itemStream.str().end()));
        }
    }
    
    // 果实
    if (!tree->getFruitDropItems().empty()) {
        lines.push_back("");
        lines.push_back(sf::String::fromUtf8(std::string("== 果实 ==").begin(), std::string("== 果实 ==").end()));
        for (const auto& item : tree->getFruitDropItems()) {
            std::ostringstream itemStream;
            itemStream << "  - " << item.name;
            if (tree->hasFruit()) {
                itemStream << " [可采摘]";
            }
            lines.push_back(sf::String::fromUtf8(itemStream.str().begin(), itemStream.str().end()));
        }
    }
    
    // 计算提示框尺寸 - 增大尺寸
    float lineHeight = 28.0f;          // 增大行高
    float padding = 16.0f;             // 增大内边距
    float minWidth = 220.0f;           // 最小宽度
    float maxWidth = 0.0f;
    
    sf::Text measureText;
    measureText.setFont(font);
    measureText.setCharacterSize(18);  // 增大字体用于测量
    
    for (const auto& line : lines) {
        measureText.setString(line);
        float width = measureText.getLocalBounds().width;
        if (width > maxWidth) maxWidth = width;
    }
    
    float tooltipWidth = std::max(minWidth, maxWidth + padding * 2);
    float tooltipHeight = lines.size() * lineHeight + padding * 2 + 20;  // 额外空间给血条
    
    // 确保不超出屏幕
    sf::Vector2u windowSize = window.getSize();
    if (tooltipX + tooltipWidth > windowSize.x) {
        tooltipX = windowSize.x - tooltipWidth - 10;
    }
    if (tooltipY + tooltipHeight > windowSize.y) {
        tooltipY = windowSize.y - tooltipHeight - 10;
    }
    
    // 绘制背景 - 更好看的样式
    sf::RectangleShape bg(sf::Vector2f(tooltipWidth, tooltipHeight));
    bg.setPosition(tooltipX, tooltipY);
    bg.setFillColor(sf::Color(25, 25, 35, 240));
    bg.setOutlineThickness(3.0f);
    bg.setOutlineColor(sf::Color(139, 90, 43));  // 木质边框色
    window.draw(bg);
    
    // 绘制标题背景条
    sf::RectangleShape titleBg(sf::Vector2f(tooltipWidth - 6, lineHeight + 4));
    titleBg.setPosition(tooltipX + 3, tooltipY + 3);
    titleBg.setFillColor(sf::Color(60, 45, 30, 200));
    window.draw(titleBg);
    
    // 绘制文字
    sf::Text text;
    text.setFont(font);
    
    float y = tooltipY + padding;
    for (size_t i = 0; i < lines.size(); i++) {
        if (lines[i].isEmpty()) {
            y += lineHeight * 0.3f;  // 空行只占部分高度
            continue;
        }
        
        text.setString(lines[i]);
        text.setPosition(tooltipX + padding, y);
        
        // 标题用金色大字
        if (i == 0) {
            text.setFillColor(sf::Color(255, 215, 0));
            text.setCharacterSize(22);
            text.setStyle(sf::Text::Bold);
        }
        // 分隔符用特殊颜色
        else if (lines[i].find("==") != sf::String::InvalidPos) {
            text.setFillColor(sf::Color(180, 140, 100));
            text.setCharacterSize(16);
            text.setStyle(sf::Text::Bold);
        }
        // 普通文字
        else {
            text.setFillColor(sf::Color(230, 230, 230));
            text.setCharacterSize(18);
            text.setStyle(sf::Text::Regular);
        }
        
        window.draw(text);
        y += lineHeight;
    }
    
    // 绘制血条 - 增大尺寸
    float barX = tooltipX + padding;
    float barY = tooltipY + tooltipHeight - padding - 14;
    float barWidth = tooltipWidth - padding * 2;
    float barHeight = 12.0f;
    
    // 血条背景
    sf::RectangleShape barBg(sf::Vector2f(barWidth, barHeight));
    barBg.setPosition(barX, barY);
    barBg.setFillColor(sf::Color(50, 50, 50));
    barBg.setOutlineThickness(1.0f);
    barBg.setOutlineColor(sf::Color(80, 80, 80));
    window.draw(barBg);
    
    // 血条填充
    float healthPercent = tree->getHealthPercent();
    sf::Color barColor;
    if (healthPercent > 0.6f) {
        barColor = sf::Color(60, 180, 60);  // 绿色
    } else if (healthPercent > 0.3f) {
        barColor = sf::Color(220, 180, 50); // 黄色
    } else {
        barColor = sf::Color(220, 60, 60);  // 红色
    }
    
    sf::RectangleShape barFill(sf::Vector2f(barWidth * healthPercent, barHeight));
    barFill.setPosition(barX, barY);
    barFill.setFillColor(barColor);
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

Tree* TreeManager::addTreeFromProperty(float x, float y, const TileProperty* prop) {
    auto tree = std::make_unique<Tree>();
    tree->initFromTileProperty(x, y, prop);
    
    // 优先使用 TileProperty 中的独立贴图
    if (prop && prop->hasTexture) {
        tree->setTextureFromProperty(prop);
    } else {
        // 后备：从文件加载
        tree->loadTextures(assetsBasePath + "/game_source/tree");
    }
    
    Tree* ptr = tree.get();
    trees.push_back(std::move(tree));
    
    std::string treeName = prop ? prop->name : "unknown";
    std::cout << "[TreeManager] Added " << treeName << " tree from property at (" << x << ", " << y << ")" << std::endl;
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
            
            Tree* tree = nullptr;
            
            // 优先使用 TileProperty（动态属性）
            if (obj.tileProperty) {
                tree = addTreeFromProperty(x, y, obj.tileProperty);
            } else {
                // 后备：使用对象名称
                std::string treeType = obj.name.empty() ? "tree1" : obj.name;
                tree = addTree(x, y, treeType);
            }
            
            if (tree) {
                // 设置尺寸
                tree->setSize(obj.width * displayScale, obj.height * displayScale);
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