#include "Crafting.h"
#include "Equipment.h"
#include <iostream>
#include <sstream>

// ============================================================================
// 颜色常量
// ============================================================================

const sf::Color CraftingPanel::BG_COLOR(30, 30, 40, 240);
const sf::Color CraftingPanel::SLOT_COLOR(50, 50, 60, 200);
const sf::Color CraftingPanel::SLOT_HOVER_COLOR(70, 70, 90, 220);
const sf::Color CraftingPanel::SLOT_SELECTED_COLOR(80, 100, 80, 240);
const sf::Color CraftingPanel::CRAFTABLE_COLOR(50, 120, 50, 200);
const sf::Color CraftingPanel::NOT_CRAFTABLE_COLOR(120, 50, 50, 200);
const sf::Color CraftingPanel::BORDER_COLOR(139, 90, 43);

// ============================================================================
// CraftingManager 实现
// ============================================================================

CraftingManager& CraftingManager::getInstance() {
    static CraftingManager instance;
    return instance;
}

void CraftingManager::initialize() {
    if (initialized) return;
    
    std::cout << "[CraftingManager] Initializing crafting recipes..." << std::endl;
    
    // ========================================
    // 工具配方
    // ========================================
    
    // 木斧 - 1石头 + 2树枝 = 1木斧（无视防御）
    {
        CraftingRecipe recipe;
        recipe.id = "craft_wooden_axe";
        recipe.name = "木斧";
        recipe.description = "合成一把木斧，可以无视树木的防御值";
        recipe.ingredients.push_back(RecipeIngredient("stone", 1));
        recipe.ingredients.push_back(RecipeIngredient("stick", 2));
        recipe.resultItemId = "wooden_axe";
        recipe.resultCount = 1;
        recipe.isEquipment = true;
        registerRecipe(recipe);
    }
    
    // 铁斧 - 3石头 + 2树枝 + 2木材 = 1铁斧
    {
        CraftingRecipe recipe;
        recipe.id = "craft_iron_axe";
        recipe.name = "铁斧";
        recipe.description = "合成一把更强力的铁斧";
        recipe.ingredients.push_back(RecipeIngredient("stone", 3));
        recipe.ingredients.push_back(RecipeIngredient("stick", 2));
        recipe.ingredients.push_back(RecipeIngredient("wood", 2));
        recipe.resultItemId = "iron_axe";
        recipe.resultCount = 1;
        recipe.isEquipment = true;
        registerRecipe(recipe);
    }
    
    // ========================================
    // 武器配方
    // ========================================
    
    // 木剑 - 2木材 + 1树枝 = 1木剑
    {
        CraftingRecipe recipe;
        recipe.id = "craft_wooden_sword";
        recipe.name = "木剑";
        recipe.description = "合成一把简单的木剑";
        recipe.ingredients.push_back(RecipeIngredient("wood", 2));
        recipe.ingredients.push_back(RecipeIngredient("stick", 1));
        recipe.resultItemId = "wooden_sword";
        recipe.resultCount = 1;
        recipe.isEquipment = true;
        registerRecipe(recipe);
    }
    
    // 铁剑 - 3石头 + 2木材 = 1铁剑
    {
        CraftingRecipe recipe;
        recipe.id = "craft_iron_sword";
        recipe.name = "铁剑";
        recipe.description = "合成一把锋利的铁剑";
        recipe.ingredients.push_back(RecipeIngredient("stone", 3));
        recipe.ingredients.push_back(RecipeIngredient("wood", 2));
        recipe.resultItemId = "iron_sword";
        recipe.resultCount = 1;
        recipe.isEquipment = true;
        registerRecipe(recipe);
    }
    
    // ========================================
    // 防具配方
    // ========================================
    
    // 木盾 - 3木材 = 1木盾
    {
        CraftingRecipe recipe;
        recipe.id = "craft_wooden_shield";
        recipe.name = "木盾";
        recipe.description = "合成一面简单的木盾";
        recipe.ingredients.push_back(RecipeIngredient("wood", 3));
        recipe.resultItemId = "wooden_shield";
        recipe.resultCount = 1;
        recipe.isEquipment = true;
        registerRecipe(recipe);
    }
    
    // 皮帽 - 2木材 = 1皮帽
    {
        CraftingRecipe recipe;
        recipe.id = "craft_leather_cap";
        recipe.name = "皮帽";
        recipe.description = "合成一顶简单的帽子";
        recipe.ingredients.push_back(RecipeIngredient("wood", 2));
        recipe.resultItemId = "leather_cap";
        recipe.resultCount = 1;
        recipe.isEquipment = true;
        registerRecipe(recipe);
    }
    
    // 皮甲 - 4木材 + 1石头 = 1皮甲
    {
        CraftingRecipe recipe;
        recipe.id = "craft_leather_armor";
        recipe.name = "皮甲";
        recipe.description = "合成一件简单的护甲";
        recipe.ingredients.push_back(RecipeIngredient("wood", 4));
        recipe.ingredients.push_back(RecipeIngredient("stone", 1));
        recipe.resultItemId = "leather_armor";
        recipe.resultCount = 1;
        recipe.isEquipment = true;
        registerRecipe(recipe);
    }
    
    // 皮裤 - 3木材 = 1皮裤
    {
        CraftingRecipe recipe;
        recipe.id = "craft_leather_pants";
        recipe.name = "皮裤";
        recipe.description = "合成一条简单的裤子";
        recipe.ingredients.push_back(RecipeIngredient("wood", 3));
        recipe.resultItemId = "leather_pants";
        recipe.resultCount = 1;
        recipe.isEquipment = true;
        registerRecipe(recipe);
    }
    
    // 皮手套 - 2木材 = 1皮手套
    {
        CraftingRecipe recipe;
        recipe.id = "craft_leather_gloves";
        recipe.name = "皮手套";
        recipe.description = "合成一双简单的手套";
        recipe.ingredients.push_back(RecipeIngredient("wood", 2));
        recipe.resultItemId = "leather_gloves";
        recipe.resultCount = 1;
        recipe.isEquipment = true;
        registerRecipe(recipe);
    }
    
    // 皮靴 - 2木材 + 1树枝 = 1皮靴
    {
        CraftingRecipe recipe;
        recipe.id = "craft_leather_boots";
        recipe.name = "皮靴";
        recipe.description = "合成一双简单的靴子";
        recipe.ingredients.push_back(RecipeIngredient("wood", 2));
        recipe.ingredients.push_back(RecipeIngredient("stick", 1));
        recipe.resultItemId = "leather_boots";
        recipe.resultCount = 1;
        recipe.isEquipment = true;
        registerRecipe(recipe);
    }
    
    // 简易披风 - 3木材 = 1简易披风
    {
        CraftingRecipe recipe;
        recipe.id = "craft_simple_cape";
        recipe.name = "简易披风";
        recipe.description = "合成一件简单的披风";
        recipe.ingredients.push_back(RecipeIngredient("wood", 3));
        recipe.resultItemId = "simple_cape";
        recipe.resultCount = 1;
        recipe.isEquipment = true;
        registerRecipe(recipe);
    }
    
    // ========================================
    // 消耗品配方
    // ========================================
    
    // 生命药水 - 3苹果 + 2樱桃 = 1生命药水
    {
        CraftingRecipe recipe;
        recipe.id = "craft_health_potion";
        recipe.name = "生命药水";
        recipe.description = "将水果酿成药水";
        recipe.ingredients.push_back(RecipeIngredient("apple", 3));
        recipe.ingredients.push_back(RecipeIngredient("cherry", 2));
        recipe.resultItemId = "health_potion";
        recipe.resultCount = 1;
        recipe.isEquipment = false;
        registerRecipe(recipe);
    }
    
    initialized = true;
    std::cout << "[CraftingManager] Registered " << recipes.size() << " recipes" << std::endl;
}

const CraftingRecipe* CraftingManager::getRecipe(const std::string& recipeId) const {
    for (const auto& recipe : recipes) {
        if (recipe.id == recipeId) {
            return &recipe;
        }
    }
    return nullptr;
}

void CraftingManager::registerRecipe(const CraftingRecipe& recipe) {
    recipes.push_back(recipe);
    std::cout << "[CraftingManager] Registered: " << recipe.id << " (" << recipe.name << ")" << std::endl;
}

bool CraftingManager::canCraft(const CraftingRecipe& recipe, CategoryInventory* inventory) const {
    if (!inventory) return false;
    
    for (const auto& ing : recipe.ingredients) {
        if (!inventory->hasItem(ing.itemId, ing.count)) {
            return false;
        }
    }
    return true;
}

bool CraftingManager::craft(const CraftingRecipe& recipe, CategoryInventory* inventory) {
    if (!inventory) return false;
    
    // 检查材料
    if (!canCraft(recipe, inventory)) {
        std::cout << "[CraftingManager] Cannot craft " << recipe.name << " - insufficient materials" << std::endl;
        return false;
    }
    
    // 消耗材料
    for (const auto& ing : recipe.ingredients) {
        inventory->removeItem(ing.itemId, ing.count);
    }
    
    // 添加产物
    int added = inventory->addItem(recipe.resultItemId, recipe.resultCount);
    
    if (added > 0) {
        std::cout << "[CraftingManager] Crafted " << added << "x " << recipe.name << std::endl;
        return true;
    } else {
        std::cout << "[CraftingManager] Failed to add crafted item - inventory full?" << std::endl;
        // TODO: 返还材料
        return false;
    }
}

// ============================================================================
// CraftingPanel 实现
// ============================================================================

CraftingPanel::CraftingPanel()
    : inventory(nullptr)
    , panelOpen(false)
    , selectedRecipe(-1)
    , hoveredRecipe(-1)
    , scrollOffset(0)
    , iconLoaded(false)
    , fontLoaded(false)
    , onCraft(nullptr)
    , onCraftSuccess(nullptr)
{
    panelSize = sf::Vector2f(500, 400);
}

bool CraftingPanel::init(const std::string& iconPath) {
    if (iconTexture.loadFromFile(iconPath)) {
        iconSprite.setTexture(iconTexture);
        iconSprite.setScale(0.8f, 0.8f);
        iconLoaded = true;
        std::cout << "[CraftingPanel] Icon loaded: " << iconPath << std::endl;
    } else {
        sf::Image placeholder;
        placeholder.create(64, 64, sf::Color(100, 80, 40, 200));
        iconTexture.loadFromImage(placeholder);
        iconSprite.setTexture(iconTexture);
        iconLoaded = true;
        std::cout << "[CraftingPanel] Using placeholder icon" << std::endl;
    }
    
    std::vector<std::string> fontPaths = {
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/simhei.ttf",
        "C:/Windows/Fonts/simsun.ttc",
        "../../assets/fonts/pixel.ttf",
    };
    
    for (const auto& path : fontPaths) {
        if (loadFont(path)) break;
    }
    
    return true;
}

bool CraftingPanel::loadFont(const std::string& fontPath) {
    if (font.loadFromFile(fontPath)) {
        fontLoaded = true;
        std::cout << "[CraftingPanel] Font loaded: " << fontPath << std::endl;
        return true;
    }
    return false;
}

void CraftingPanel::setIconPosition(float x, float y) {
    iconPosition = sf::Vector2f(x, y);
    iconSprite.setPosition(iconPosition);
}

void CraftingPanel::update(float /*dt*/) {
    // 更新动画等
}

void CraftingPanel::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    
    // 图标点击
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        if (!panelOpen && iconSprite.getGlobalBounds().contains(mousePosF)) {
            open();
            return;
        }
    }
    
    // 键盘快捷键
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::C) {
            toggle();
            return;
        }
        if (event.key.code == sf::Keyboard::Escape && panelOpen) {
            close();
            return;
        }
    }
    
    if (!panelOpen) return;
    
    sf::FloatRect panelBounds(panelPosition, panelSize);
    
    // 鼠标移动
    if (event.type == sf::Event::MouseMoved) {
        if (panelBounds.contains(mousePosF)) {
            hoveredRecipe = getRecipeAtPosition(mousePosF);
        } else {
            hoveredRecipe = -1;
        }
    }
    
    // 鼠标滚轮
    if (event.type == sf::Event::MouseWheelScrolled) {
        if (panelBounds.contains(mousePosF)) {
            scrollOffset -= static_cast<int>(event.mouseWheelScroll.delta * 2);
            scrollOffset = std::max(0, scrollOffset);
            int maxScroll = static_cast<int>(CraftingManager::getInstance().getAllRecipes().size()) - 5;
            scrollOffset = std::min(scrollOffset, std::max(0, maxScroll));
        }
    }
    
    // 鼠标点击
    if (event.type == sf::Event::MouseButtonPressed) {
        sf::FloatRect closeButton(
            panelPosition.x + panelSize.x - 30,
            panelPosition.y + 5,
            25, 25
        );
        
        if (event.mouseButton.button == sf::Mouse::Left) {
            if (closeButton.contains(mousePosF)) {
                close();
                return;
            }
            
            if (!panelBounds.contains(mousePosF)) {
                close();
                return;
            }
            
            // 点击配方列表
            int clickedRecipe = getRecipeAtPosition(mousePosF);
            if (clickedRecipe >= 0) {
                selectedRecipe = clickedRecipe;
            }
            
            // 点击合成按钮
            if (selectedRecipe >= 0) {
                sf::FloatRect craftButton(
                    panelPosition.x + LIST_WIDTH + 30,
                    panelPosition.y + panelSize.y - 50,
                    100, 35
                );
                
                if (craftButton.contains(mousePosF)) {
                    const auto& recipes = CraftingManager::getInstance().getAllRecipes();
                    if (selectedRecipe < static_cast<int>(recipes.size())) {
                        const CraftingRecipe& recipe = recipes[selectedRecipe];
                        if (CraftingManager::getInstance().craft(recipe, inventory)) {
                            if (onCraft) {
                                onCraft(recipe);
                            }
                            if (onCraftSuccess) {
                                onCraftSuccess(recipe.resultItemId, recipe.resultCount);
                            }
                        }
                    }
                }
            }
        }
    }
}

void CraftingPanel::render(sf::RenderWindow& window) {
    // 渲染图标
    if (iconLoaded) {
        sf::RectangleShape iconBg(sf::Vector2f(60, 60));
        iconBg.setPosition(iconPosition.x - 5, iconPosition.y - 5);
        iconBg.setFillColor(sf::Color(30, 30, 40, 200));
        iconBg.setOutlineThickness(2);
        iconBg.setOutlineColor(BORDER_COLOR);
        window.draw(iconBg);
        window.draw(iconSprite);
    }
    
    if (!panelOpen) return;
    
    // 计算面板位置
    sf::Vector2u windowSize = window.getSize();
    panelPosition.x = (windowSize.x - panelSize.x) / 2;
    panelPosition.y = (windowSize.y - panelSize.y) / 2;
    
    // 背景
    sf::RectangleShape bg(panelSize);
    bg.setPosition(panelPosition);
    bg.setFillColor(BG_COLOR);
    bg.setOutlineThickness(3);
    bg.setOutlineColor(BORDER_COLOR);
    window.draw(bg);
    
    // 标题栏
    sf::RectangleShape titleBar(sf::Vector2f(panelSize.x - 6, 30));
    titleBar.setPosition(panelPosition.x + 3, panelPosition.y + 3);
    titleBar.setFillColor(sf::Color(60, 45, 30, 200));
    window.draw(titleBar);
    
    if (fontLoaded) {
        sf::Text title;
        title.setFont(font);
        std::string titleStr = "工作台 - 合成";
        title.setString(sf::String::fromUtf8(titleStr.begin(), titleStr.end()));
        title.setCharacterSize(20);
        title.setFillColor(sf::Color(255, 220, 150));
        title.setPosition(panelPosition.x + 15, panelPosition.y + 5);
        window.draw(title);
    }
    
    // 关闭按钮
    sf::RectangleShape closeBtn(sf::Vector2f(20, 20));
    closeBtn.setPosition(panelPosition.x + panelSize.x - 25, panelPosition.y + 8);
    closeBtn.setFillColor(sf::Color(150, 50, 50, 200));
    closeBtn.setOutlineThickness(1);
    closeBtn.setOutlineColor(sf::Color::White);
    window.draw(closeBtn);
    
    if (fontLoaded) {
        sf::Text closeText;
        closeText.setFont(font);
        closeText.setString("X");
        closeText.setCharacterSize(14);
        closeText.setFillColor(sf::Color::White);
        closeText.setPosition(panelPosition.x + panelSize.x - 21, panelPosition.y + 7);
        window.draw(closeText);
    }
    
    // 分隔线
    sf::RectangleShape divider(sf::Vector2f(2, panelSize.y - 50));
    divider.setPosition(panelPosition.x + LIST_WIDTH, panelPosition.y + 40);
    divider.setFillColor(sf::Color(80, 80, 80));
    window.draw(divider);
    
    // 渲染配方列表
    renderRecipeList(window);
    
    // 渲染配方详情
    renderRecipeDetail(window);
}

void CraftingPanel::renderRecipeList(sf::RenderWindow& window) {
    const auto& recipes = CraftingManager::getInstance().getAllRecipes();
    
    float listX = panelPosition.x + 10;
    float listY = panelPosition.y + 45;
    float listHeight = panelSize.y - 60;
    
    // 配方列表背景
    sf::RectangleShape listBg(sf::Vector2f(LIST_WIDTH - 20, listHeight));
    listBg.setPosition(listX, listY);
    listBg.setFillColor(sf::Color(20, 20, 25, 200));
    window.draw(listBg);
    
    // 渲染可见的配方
    int visibleRecipes = static_cast<int>(listHeight / RECIPE_HEIGHT);
    
    for (int i = 0; i < visibleRecipes && (i + scrollOffset) < static_cast<int>(recipes.size()); i++) {
        int recipeIndex = i + scrollOffset;
        const CraftingRecipe& recipe = recipes[recipeIndex];
        
        float itemY = listY + i * RECIPE_HEIGHT;
        
        // 配方项背景
        sf::RectangleShape itemBg(sf::Vector2f(LIST_WIDTH - 24, RECIPE_HEIGHT - 4));
        itemBg.setPosition(listX + 2, itemY + 2);
        
        bool canCraft = CraftingManager::getInstance().canCraft(recipe, inventory);
        bool isSelected = (recipeIndex == selectedRecipe);
        bool isHovered = (recipeIndex == hoveredRecipe);
        
        if (isSelected) {
            itemBg.setFillColor(SLOT_SELECTED_COLOR);
        } else if (isHovered) {
            itemBg.setFillColor(SLOT_HOVER_COLOR);
        } else {
            itemBg.setFillColor(SLOT_COLOR);
        }
        
        itemBg.setOutlineThickness(1);
        itemBg.setOutlineColor(canCraft ? sf::Color(100, 200, 100) : sf::Color(80, 80, 80));
        window.draw(itemBg);
        
        // 配方名称
        if (fontLoaded) {
            sf::Text nameText;
            nameText.setFont(font);
            nameText.setString(sf::String::fromUtf8(recipe.name.begin(), recipe.name.end()));
            nameText.setCharacterSize(14);
            nameText.setFillColor(canCraft ? sf::Color(150, 255, 150) : sf::Color(180, 180, 180));
            nameText.setPosition(listX + 10, itemY + 8);
            window.draw(nameText);
            
            // 产物信息
            std::stringstream ss;
            ss << "-> " << recipe.resultCount << "x";
            sf::Text resultText;
            resultText.setFont(font);
            resultText.setString(ss.str());
            resultText.setCharacterSize(12);
            resultText.setFillColor(sf::Color(200, 200, 100));
            resultText.setPosition(listX + 10, itemY + 30);
            window.draw(resultText);
        }
        
        // 可合成标记
        if (canCraft) {
            sf::CircleShape indicator(5);
            indicator.setFillColor(sf::Color(100, 255, 100));
            indicator.setPosition(listX + LIST_WIDTH - 40, itemY + 25);
            window.draw(indicator);
        }
    }
}

void CraftingPanel::renderRecipeDetail(sf::RenderWindow& window) {
    const auto& recipes = CraftingManager::getInstance().getAllRecipes();
    
    if (selectedRecipe < 0 || selectedRecipe >= static_cast<int>(recipes.size())) {
        // 无选中配方时的提示
        if (fontLoaded) {
            sf::Text hintText;
            hintText.setFont(font);
            std::string hint = "选择左侧配方查看详情";
            hintText.setString(sf::String::fromUtf8(hint.begin(), hint.end()));
            hintText.setCharacterSize(14);
            hintText.setFillColor(sf::Color(150, 150, 150));
            hintText.setPosition(panelPosition.x + LIST_WIDTH + 30, panelPosition.y + 150);
            window.draw(hintText);
        }
        return;
    }
    
    const CraftingRecipe& recipe = recipes[selectedRecipe];
    float detailX = panelPosition.x + LIST_WIDTH + 20;
    float detailY = panelPosition.y + 50;
    float detailWidth = panelSize.x - LIST_WIDTH - 40;
    
    if (fontLoaded) {
        // 配方名称
        sf::Text nameText;
        nameText.setFont(font);
        nameText.setString(sf::String::fromUtf8(recipe.name.begin(), recipe.name.end()));
        nameText.setCharacterSize(18);
        nameText.setFillColor(sf::Color(255, 220, 150));
        nameText.setPosition(detailX, detailY);
        window.draw(nameText);
        
        // 描述
        sf::Text descText;
        descText.setFont(font);
        descText.setString(sf::String::fromUtf8(recipe.description.begin(), recipe.description.end()));
        descText.setCharacterSize(12);
        descText.setFillColor(sf::Color(180, 180, 180));
        descText.setPosition(detailX, detailY + 30);
        window.draw(descText);
        
        // 所需材料标题
        std::string ingTitle = "所需材料:";
        sf::Text ingTitleText;
        ingTitleText.setFont(font);
        ingTitleText.setString(sf::String::fromUtf8(ingTitle.begin(), ingTitle.end()));
        ingTitleText.setCharacterSize(14);
        ingTitleText.setFillColor(sf::Color(200, 200, 200));
        ingTitleText.setPosition(detailX, detailY + 60);
        window.draw(ingTitleText);
        
        // 渲染所需材料
        float ingY = detailY + 85;
        for (const auto& ing : recipe.ingredients) {
            bool hasEnough = inventory ? inventory->hasItem(ing.itemId, ing.count) : false;
            renderIngredient(window, ing, sf::Vector2f(detailX, ingY), hasEnough);
            ingY += 30;
        }
        
        // 产出标题
        std::string resultTitle = "产出:";
        sf::Text resultTitleText;
        resultTitleText.setFont(font);
        resultTitleText.setString(sf::String::fromUtf8(resultTitle.begin(), resultTitle.end()));
        resultTitleText.setCharacterSize(14);
        resultTitleText.setFillColor(sf::Color(200, 200, 200));
        resultTitleText.setPosition(detailX, ingY + 10);
        window.draw(resultTitleText);
        
        // 产出物品
        const ItemData* resultData = ItemDatabase::getInstance().getItemData(recipe.resultItemId);
        std::string resultName = resultData ? resultData->name : recipe.resultItemId;
        
        std::stringstream ss;
        ss << recipe.resultCount << "x " << resultName;
        sf::Text resultText;
        resultText.setFont(font);
        resultText.setString(sf::String::fromUtf8(ss.str().begin(), ss.str().end()));
        resultText.setCharacterSize(14);
        resultText.setFillColor(sf::Color(100, 255, 100));
        resultText.setPosition(detailX + 20, ingY + 35);
        window.draw(resultText);
        
        // 如果是装备，显示特殊效果
        if (recipe.isEquipment) {
            const EquipmentData* equipData = EquipmentManager::getInstance().getEquipmentData(recipe.resultItemId);
            if (equipData && equipData->stats.ignoreDefense) {
                std::string effectStr = "特效: 无视目标防御";
                sf::Text effectText;
                effectText.setFont(font);
                effectText.setString(sf::String::fromUtf8(effectStr.begin(), effectStr.end()));
                effectText.setCharacterSize(12);
                effectText.setFillColor(sf::Color(255, 200, 100));
                effectText.setPosition(detailX + 20, ingY + 55);
                window.draw(effectText);
            }
        }
    }
    
    // 合成按钮
    bool canCraft = CraftingManager::getInstance().canCraft(recipe, inventory);
    
    sf::RectangleShape craftBtn(sf::Vector2f(100, 35));
    craftBtn.setPosition(detailX + 10, panelPosition.y + panelSize.y - 50);
    craftBtn.setFillColor(canCraft ? CRAFTABLE_COLOR : NOT_CRAFTABLE_COLOR);
    craftBtn.setOutlineThickness(2);
    craftBtn.setOutlineColor(canCraft ? sf::Color(100, 200, 100) : sf::Color(150, 100, 100));
    window.draw(craftBtn);
    
    if (fontLoaded) {
        sf::Text btnText;
        btnText.setFont(font);
        std::string btnStr = canCraft ? "合成" : "材料不足";
        btnText.setString(sf::String::fromUtf8(btnStr.begin(), btnStr.end()));
        btnText.setCharacterSize(14);
        btnText.setFillColor(sf::Color::White);
        
        sf::FloatRect bounds = btnText.getLocalBounds();
        btnText.setPosition(
            detailX + 10 + (100 - bounds.width) / 2,
            panelPosition.y + panelSize.y - 45
        );
        window.draw(btnText);
    }
}

void CraftingPanel::renderIngredient(sf::RenderWindow& window, const RecipeIngredient& ing, 
                                     const sf::Vector2f& pos, bool hasEnough) {
    if (!fontLoaded) return;
    
    const ItemData* data = ItemDatabase::getInstance().getItemData(ing.itemId);
    std::string itemName = data ? data->name : ing.itemId;
    
    int owned = inventory ? inventory->getItemCount(ing.itemId) : 0;
    
    std::stringstream ss;
    ss << "  " << ing.count << "x " << itemName << " (" << owned << "/" << ing.count << ")";
    
    sf::Text text;
    text.setFont(font);
    text.setString(sf::String::fromUtf8(ss.str().begin(), ss.str().end()));
    text.setCharacterSize(13);
    text.setFillColor(hasEnough ? sf::Color(150, 255, 150) : sf::Color(255, 150, 150));
    text.setPosition(pos);
    window.draw(text);
    
    // 状态指示
    sf::CircleShape indicator(4);
    indicator.setFillColor(hasEnough ? sf::Color(100, 255, 100) : sf::Color(255, 100, 100));
    indicator.setPosition(pos.x, pos.y + 5);
    window.draw(indicator);
}

int CraftingPanel::getRecipeAtPosition(const sf::Vector2f& pos) const {
    float listX = panelPosition.x + 10;
    float listY = panelPosition.y + 45;
    float listHeight = panelSize.y - 60;
    
    sf::FloatRect listBounds(listX, listY, LIST_WIDTH - 20, listHeight);
    if (!listBounds.contains(pos)) return -1;
    
    int index = static_cast<int>((pos.y - listY) / RECIPE_HEIGHT) + scrollOffset;
    
    const auto& recipes = CraftingManager::getInstance().getAllRecipes();
    if (index >= 0 && index < static_cast<int>(recipes.size())) {
        return index;
    }
    
    return -1;
}

void CraftingPanel::open() {
    panelOpen = true;
    selectedRecipe = -1;
    hoveredRecipe = -1;
    scrollOffset = 0;
    std::cout << "[CraftingPanel] Opened" << std::endl;
}

void CraftingPanel::close() {
    panelOpen = false;
    std::cout << "[CraftingPanel] Closed" << std::endl;
}

void CraftingPanel::toggle() {
    if (panelOpen) close();
    else open();
}
