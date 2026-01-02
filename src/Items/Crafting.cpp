#include "Crafting.h"
#include <iostream>
#include <sstream>
#include <iomanip>

// ============================================================================
// 样式配色常量 (RPG Dark Theme)
// ============================================================================
namespace Style {
    const sf::Color PanelBG(34, 34, 40, 250);
    const sf::Color TitleBG(50, 40, 30, 255);
    const sf::Color ListBG(25, 25, 30, 200);
    const sf::Color ItemNormal(45, 45, 50, 255);
    const sf::Color ItemHover(60, 60, 70, 255);
    const sf::Color ItemSelected(80, 70, 50, 255);
    const sf::Color Border(160, 130, 80);
    const sf::Color TextNormal(200, 200, 200);
    const sf::Color TextHighlight(255, 215, 0);
    const sf::Color TextGreen(100, 255, 100);
    const sf::Color TextRed(255, 100, 100);
    const sf::Color SliderBG(20, 20, 20);
    const sf::Color SliderFill(180, 140, 60);
    const sf::Color ButtonNormal(60, 100, 60);
    const sf::Color ButtonDisabled(60, 60, 60);
}

// ============================================================================
// CraftingManager 实现
// ============================================================================

CraftingManager& CraftingManager::getInstance() {
    static CraftingManager instance;
    return instance;
}

void CraftingManager::registerRecipe(const CraftingRecipe& recipe) {
    recipes.push_back(recipe);
}

void CraftingManager::initialize() {
    if (initialized) return;
    
    // --- 武器 (锻造) ---
    {
        CraftingRecipe r;
        r.id = "craft_axe"; r.name = "斧头";
        r.description = "基础工具，用于伐木。";
        r.ingredients = { {"stone", 3}, {"stick", 1} };
        r.resultItemId = "axe";
        r.isEquipment = true; 
        registerRecipe(r); 
    }
    {
        CraftingRecipe r;
        r.id = "craft_knife"; r.name = "小刀";
        r.description = "锋利的武器，可用于战斗。可通过锻造提升品质。";
        r.ingredients = { {"steel", 2} };
        r.resultItemId = "knife";
        r.isEquipment = true;
        r.isWeaponForge = true;
        r.maxWeaponSouls = 30; // 允许加30个魂
        registerRecipe(r);
    }
    {
        CraftingRecipe r;
        r.id = "craft_spear"; r.name = "长矛";
        r.description = "长柄武器，攻击距离较远。可通过锻造提升品质。";
        r.ingredients = { {"pig_iron", 1}, {"steel", 2} };
        r.resultItemId = "spear";
        r.isEquipment = true;
        r.isWeaponForge = true;
        r.maxWeaponSouls = 30;
        registerRecipe(r);
    }

    // --- 材料 (批量) ---
    {
        CraftingRecipe r;
        r.id = "craft_steel"; r.name = "钢铁";
        r.description = "坚硬的金属材料，用于高级制作。";
        r.ingredients = { {"pig_iron", 1}, {"coal", 2} };
        r.resultItemId = "steel";
        r.allowBatchCraft = true;
        r.maxBatchCount = 100;
        registerRecipe(r);
    }
    
    initialized = true;
}

bool CraftingManager::canCraft(const CraftingRecipe& recipe, CategoryInventory* inventory, int multiplier, int extraSoulCount) const {
    if (!inventory) return false;
    
    // 检查基础材料
    for (const auto& ing : recipe.ingredients) {
        if (!inventory->hasItem(ing.itemId, ing.count * multiplier)) {
            return false;
        }
    }
    
    // 检查武器魂 (如果配方允许且选择了数量)
    if (recipe.isWeaponForge && extraSoulCount > 0) {
        if (!inventory->hasItem("weapon_soul", extraSoulCount)) {
            return false;
        }
    }
    
    return true;
}

ForgeProbability CraftingManager::calculateForgeProb(int soulCount) {
    ForgeProbability p;
    // 基础概率 (0魂)
    float baseWhite = 0.70f;
    float baseGreen = 0.20f;
    float baseBlue  = 0.10f;
    
    // 衰减系数模拟
    float decay = 0.02f * soulCount; 
    
    p.white = std::max(0.0f, baseWhite - decay * 2.5f);
    p.green = std::max(0.0f, baseGreen - decay * 0.5f + (soulCount * 0.01f)); 
    p.blue  = std::min(0.4f, baseBlue + soulCount * 0.015f);
    p.purple = std::min(0.3f, 0.0f + soulCount * 0.01f);
    p.orange = std::min(0.1f, 0.0f + (soulCount > 10 ? (soulCount-10)*0.005f : 0));
    
    // 归一化 (确保总和100%)
    float sum = p.white + p.green + p.blue + p.purple + p.orange;
    if (sum > 0) {
        p.white /= sum; p.green /= sum; p.blue /= sum; p.purple /= sum; p.orange /= sum;
    }
    p.red = 0.0f; // 暂未开放红色
    
    return p;
}

bool CraftingManager::craft(const CraftingRecipe& recipe, CategoryInventory* inventory, int multiplier, int soulCount) {
    if (!canCraft(recipe, inventory, multiplier, soulCount)) return false;
    
    // 1. 消耗材料
    for (const auto& ing : recipe.ingredients) {
        inventory->removeItem(ing.itemId, ing.count * multiplier);
    }
    if (recipe.isWeaponForge && soulCount > 0) {
        inventory->removeItem("weapon_soul", soulCount);
    }
    
    // 2. 产出物品
    if (recipe.isWeaponForge) {
        // 锻造逻辑：随机生成品质
        ForgeProbability prob = calculateForgeProb(soulCount);
        float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        (void)prob; // 显式标记未使用，防止编译器警告
        (void)r;    // 显式标记未使用
        
        // TODO: 使用 prob 和 r 来决定具体产生哪个品质的装备
        // 目前先产出默认物品
        inventory->addItem(recipe.resultItemId, 1);
        std::cout << "[Crafting] Forged weapon with " << soulCount << " souls." << std::endl;
    } 
    else {
        // 普通/批量合成
        inventory->addItem(recipe.resultItemId, recipe.resultCount * multiplier);
        std::cout << "[Crafting] Crafted " << (recipe.resultCount * multiplier) << "x " << recipe.name << std::endl;
    }
    
    return true;
}

// ============================================================================
// CraftingPanel 实现
// ============================================================================

CraftingPanel::CraftingPanel()
    : inventory(nullptr), panelOpen(false), selectedRecipe(-1), hoveredRecipe(-1), scrollOffset(0)
    , currentBatchAmount(1), currentSoulAmount(0)
    , isDraggingBatchSlider(false), isDraggingSoulSlider(false)
    , onCraft(nullptr), onCraftSuccess(nullptr)
{
    panelSize = sf::Vector2f(700, 500); 
}

bool CraftingPanel::init(const std::string& iconPath) {
    // 尝试加载字体
    std::vector<std::string> fontPaths = {
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/simhei.ttf",
        "assets/fonts/pixel.ttf",
        "../../assets/fonts/pixel.ttf"
    };
    
    for (const auto& path : fontPaths) {
        if (font.loadFromFile(path)) {
            fontLoaded = true;
            break;
        }
    }
    
    if (iconTexture.loadFromFile(iconPath)) {
        iconSprite.setTexture(iconTexture);
        iconSprite.setScale(0.8f, 0.8f);
    } else {
        // Fallback icon
        sf::Image img;
        img.create(64, 64, sf::Color(100, 100, 100));
        iconTexture.loadFromImage(img);
        iconSprite.setTexture(iconTexture);
    }
    return true;
}

void CraftingPanel::setIconPosition(float x, float y) {
    iconPosition = sf::Vector2f(x, y);
    iconSprite.setPosition(iconPosition);
}

void CraftingPanel::update(float dt) {
    (void)dt; // 消除未使用参数警告
    // 简单的动画逻辑可放在这里
}

void CraftingPanel::open() {
    panelOpen = true;
    currentBatchAmount = 1;
    currentSoulAmount = 0;
}

void CraftingPanel::close() {
    panelOpen = false;
    isDraggingBatchSlider = false;
    isDraggingSoulSlider = false;
}

void CraftingPanel::toggle() {
    if (panelOpen) close(); else open();
}

void CraftingPanel::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    // 图标点击处理
    if (event.type == sf::Event::MouseButtonPressed && !panelOpen) {
        sf::FloatRect iconBounds(iconPosition, sf::Vector2f(60, 60));
        if (iconBounds.contains(sf::Vector2f(sf::Mouse::getPosition(window)))) {
            open();
            return;
        }
    }
    
    if (!panelOpen) return;
    
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mouseF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    
    // 计算布局区域
    float listWidth = 220.0f;
    float detailX = panelPosition.x + listWidth + 10;
    float bottomControlsY = panelPosition.y + panelSize.y - 120; // 控制区起始Y
    
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            
            // 1. 关闭按钮检测
            sf::FloatRect closeBtn(panelPosition.x + panelSize.x - 30, panelPosition.y, 30, 30);
            if (closeBtn.contains(mouseF)) { close(); return; }

            // 2. 列表点击检测
            int clicked = getRecipeAtPosition(mouseF);
            if (clicked != -1) {
                selectedRecipe = clicked;
                currentBatchAmount = 1; // 重置
                currentSoulAmount = 0;  // 重置
            }
            
            // 3. 按钮点击检测 (合成按钮)
            sf::FloatRect craftBtn(detailX, panelPosition.y + panelSize.y - 50, 120, 40);
            if (craftBtn.contains(mouseF)) {
                if (selectedRecipe != -1) {
                    const auto& r = CraftingManager::getInstance().getAllRecipes()[selectedRecipe];
                    if (CraftingManager::getInstance().craft(r, inventory, currentBatchAmount, currentSoulAmount)) {
                        // 成功合成，调用回调
                        if (onCraft) onCraft(r);
                        // GameState 可能通过这个回调来播放音效或显示提示
                        if (onCraftSuccess) {
                            onCraftSuccess(r.resultItemId, r.resultCount * currentBatchAmount);
                        }
                    }
                }
            }
            
            // 4. 滑动条点击开始
            if (selectedRecipe != -1) {
                const auto& r = CraftingManager::getInstance().getAllRecipes()[selectedRecipe];
                
                // 批量滑动条区域
                if (r.allowBatchCraft) {
                    sf::FloatRect sliderRect(detailX + 80, bottomControlsY, 200, 20);
                    if (sliderRect.contains(mouseF)) isDraggingBatchSlider = true;
                }
                // 武器魂滑动条区域
                if (r.isWeaponForge) {
                    sf::FloatRect sliderRect(detailX + 80, bottomControlsY + 30, 200, 20);
                    if (sliderRect.contains(mouseF)) isDraggingSoulSlider = true;
                }
            }
        }
    }
    
    if (event.type == sf::Event::MouseButtonReleased) {
        isDraggingBatchSlider = false;
        isDraggingSoulSlider = false;
    }
    
    // 鼠标移动 - 处理拖拽
    if (event.type == sf::Event::MouseMoved) {
        if (selectedRecipe != -1) {
            const auto& r = CraftingManager::getInstance().getAllRecipes()[selectedRecipe];
            float sliderX = detailX + 80;
            float sliderWidth = 200;
            
            if (isDraggingBatchSlider && r.allowBatchCraft) {
                float ratio = (mouseF.x - sliderX) / sliderWidth;
                ratio = std::max(0.0f, std::min(1.0f, ratio));
                currentBatchAmount = 1 + static_cast<int>(ratio * (r.maxBatchCount - 1));
            }
            
            if (isDraggingSoulSlider && r.isWeaponForge) {
                float ratio = (mouseF.x - sliderX) / sliderWidth;
                ratio = std::max(0.0f, std::min(1.0f, ratio));
                currentSoulAmount = static_cast<int>(ratio * r.maxWeaponSouls);
            }
        }
    }
}

void CraftingPanel::render(sf::RenderWindow& window) {
    // 绘制图标
    if (iconSprite.getTexture()) {
        window.draw(iconSprite);
    }
    
    if (!panelOpen) return;
    
    // 居中计算
    sf::Vector2u winSize = window.getSize();
    panelPosition = sf::Vector2f((winSize.x - panelSize.x)/2, (winSize.y - panelSize.y)/2);
    
    // 1. 背景
    sf::RectangleShape bg(panelSize);
    bg.setPosition(panelPosition);
    bg.setFillColor(Style::PanelBG);
    bg.setOutlineThickness(2);
    bg.setOutlineColor(Style::Border);
    window.draw(bg);
    
    // 2. 标题栏
    sf::RectangleShape titleBar(sf::Vector2f(panelSize.x, 40));
    titleBar.setPosition(panelPosition);
    titleBar.setFillColor(Style::TitleBG);
    window.draw(titleBar);
    
    if (fontLoaded) {
        sf::Text title;
        title.setFont(font);
        // FIX: 创建持久的 string 对象
        std::string titleStr = "工匠作坊";
        title.setString(sf::String::fromUtf8(titleStr.begin(), titleStr.end()));
        title.setCharacterSize(24);
        title.setFillColor(Style::TextHighlight);
        title.setPosition(panelPosition.x + 15, panelPosition.y + 5);
        window.draw(title);
        
        sf::Text closeX("X", font, 24);
        closeX.setPosition(panelPosition.x + panelSize.x - 30, panelPosition.y + 5);
        window.draw(closeX);
    }
    
    // 3. 配方列表
    renderRecipeList(window);
    
    // 4. 配方详情
    renderRecipeDetail(window);
}

void CraftingPanel::renderRecipeList(sf::RenderWindow& window) {
    float listWidth = 220.0f;
    float itemHeight = 50.0f;
    float startY = panelPosition.y + 50;
    
    // 列表背景
    sf::RectangleShape listBg(sf::Vector2f(listWidth, panelSize.y - 60));
    listBg.setPosition(panelPosition.x + 10, startY);
    listBg.setFillColor(Style::ListBG);
    window.draw(listBg);
    
    const auto& recipes = CraftingManager::getInstance().getAllRecipes();
    int visibleCount = static_cast<int>((panelSize.y - 70) / itemHeight);
    
    for (int i = 0; i < visibleCount; ++i) {
        int idx = i + scrollOffset;
        if (idx >= static_cast<int>(recipes.size())) break;
        
        const auto& r = recipes[idx];
        sf::Vector2f itemPos(panelPosition.x + 15, startY + i * itemHeight + 5);
        
        sf::RectangleShape itemBg(sf::Vector2f(listWidth - 10, itemHeight - 5));
        itemBg.setPosition(itemPos);
        itemBg.setFillColor(idx == selectedRecipe ? Style::ItemSelected : Style::ItemNormal);
        window.draw(itemBg);
        
        if (fontLoaded) {
            sf::Text name;
            name.setFont(font);
            name.setString(sf::String::fromUtf8(r.name.begin(), r.name.end()));
            name.setCharacterSize(18);
            name.setPosition(itemPos.x + 10, itemPos.y + 10);
            name.setFillColor(Style::TextNormal);
            window.draw(name);
        }
    }
}

void CraftingPanel::renderSlider(sf::RenderWindow& window, const sf::Vector2f& pos, float width, int min, int max, int& current, const std::string& label) {
    if (!fontLoaded) return;
    
    // 标签
    sf::Text lbl;
    lbl.setFont(font);
    lbl.setString(sf::String::fromUtf8(label.begin(), label.end()));
    lbl.setCharacterSize(16);
    lbl.setPosition(pos.x - 70, pos.y);
    lbl.setFillColor(Style::TextNormal);
    window.draw(lbl);
    
    // 槽
    sf::RectangleShape slot(sf::Vector2f(width, 20));
    slot.setPosition(pos.x, pos.y + 2);
    slot.setFillColor(Style::SliderBG);
    slot.setOutlineColor(sf::Color(100,100,100));
    slot.setOutlineThickness(1);
    window.draw(slot);
    
    // 滑块填充
    float ratio = 0.0f;
    if (max > min) ratio = (float)(current - min) / (max - min);
    
    sf::RectangleShape fill(sf::Vector2f(width * ratio, 20));
    fill.setPosition(pos.x, pos.y + 2);
    fill.setFillColor(Style::SliderFill);
    window.draw(fill);
    
    // 数值显示
    std::string valStr = std::to_string(current);
    if (max > 1000) valStr += "/" + std::to_string(max); 
    sf::Text val;
    val.setFont(font);
    val.setString(valStr);
    val.setCharacterSize(14);
    val.setPosition(pos.x + width + 10, pos.y + 2);
    window.draw(val);
}

void CraftingPanel::renderRecipeDetail(sf::RenderWindow& window) {
    if (selectedRecipe == -1 || !fontLoaded) return;
    
    const auto& r = CraftingManager::getInstance().getAllRecipes()[selectedRecipe];
    float listWidth = 220.0f;
    float x = panelPosition.x + listWidth + 20;
    float y = panelPosition.y + 50;
    
    // 1. 名称与描述
    sf::Text name;
    name.setFont(font);
    name.setString(sf::String::fromUtf8(r.name.begin(), r.name.end()));
    name.setCharacterSize(28);
    name.setPosition(x, y);
    name.setFillColor(Style::TextHighlight);
    window.draw(name);
    
    sf::Text desc;
    desc.setFont(font);
    desc.setString(sf::String::fromUtf8(r.description.begin(), r.description.end()));
    desc.setCharacterSize(16);
    desc.setPosition(x, y + 40);
    desc.setFillColor(sf::Color(180, 180, 180));
    window.draw(desc);
    
    // 2. 材料需求列表
    y += 80;
    sf::Text reqTitle;
    reqTitle.setFont(font);
    // FIX: 创建持久的 string 对象
    std::string reqStr = "所需材料:";
    reqTitle.setString(sf::String::fromUtf8(reqStr.begin(), reqStr.end()));
    reqTitle.setCharacterSize(18);
    reqTitle.setPosition(x, y);
    window.draw(reqTitle);
    
    y += 30;
    for (const auto& ing : r.ingredients) {
        int need = ing.count * currentBatchAmount;
        int have = inventory ? inventory->getItemCount(ing.itemId) : 0;
        
        std::stringstream ss;
        ss << "- " << ing.itemId << ": " << need << " (拥有: " << have << ")"; 
        
        sf::Text ingText;
        ingText.setFont(font);
        // FIX: 关键修正！保存 ss.str() 到局部变量，避免 .begin() 和 .end() 作用于不同的临时对象
        std::string ingStr = ss.str();
        ingText.setString(sf::String::fromUtf8(ingStr.begin(), ingStr.end()));
        ingText.setCharacterSize(16);
        ingText.setPosition(x + 10, y);
        ingText.setFillColor(have >= need ? Style::TextGreen : Style::TextRed);
        window.draw(ingText);
        y += 25;
    }
    
    // 3. 动态控制区 (批量 / 锻造)
    float controlY = panelPosition.y + panelSize.y - 140;
    
    // A. 批量合成滑块
    if (r.allowBatchCraft) {
        renderSlider(window, sf::Vector2f(x + 70, controlY), 200.0f, 1, r.maxBatchCount, currentBatchAmount, "批量数量:");
        
        // 显示总消耗
        if (currentBatchAmount > 1) {
            sf::Text totalHint;
            totalHint.setFont(font);
            std::string hint = "消耗将翻倍";
            totalHint.setString(sf::String::fromUtf8(hint.begin(), hint.end()));
            totalHint.setCharacterSize(14);
            totalHint.setPosition(x + 70, controlY + 25);
            totalHint.setFillColor(sf::Color(150,150,150));
            window.draw(totalHint);
        }
    }
    
    // B. 武器锻造滑块与概率
    if (r.isWeaponForge) {
        // 武器魂滑块
        int soulHave = inventory ? inventory->getItemCount("weapon_soul") : 0;
        int actualMax = r.maxWeaponSouls; 
        
        renderSlider(window, sf::Vector2f(x + 70, controlY), 200.0f, 0, actualMax, currentSoulAmount, "投入器魂:");
        
        // 概率预览表
        ForgeProbability prob = CraftingManager::getInstance().calculateForgeProb(currentSoulAmount);
        
        float probY = controlY + 35;
        auto drawProb = [&](const std::string& label, float p, sf::Color c, float offX) {
            std::stringstream ss;
            ss << label << ":" << (int)(p*100) << "%";
            sf::Text t;
            t.setFont(font);
            // FIX: 这里也需要注意，不过之前的写法 std::string str = ss.str() 已经是安全的了
            std::string str = ss.str();
            t.setString(sf::String::fromUtf8(str.begin(), str.end()));
            t.setCharacterSize(14);
            t.setPosition(x + offX, probY);
            t.setFillColor(c);
            window.draw(t);
        };
        
        drawProb("白", prob.white, sf::Color::White, 0);
        drawProb("绿", prob.green, sf::Color::Green, 60);
        drawProb("蓝", prob.blue, sf::Color::Cyan, 120);
        drawProb("紫", prob.purple, sf::Color::Magenta, 180);
        drawProb("橙", prob.orange, sf::Color(255, 165, 0), 240);
        
        // 警告：如果没有魂
        if (currentSoulAmount > soulHave) {
            sf::Text warn;
            warn.setFont(font);
            std::string w = "武器魂不足!";
            warn.setString(sf::String::fromUtf8(w.begin(), w.end()));
            warn.setCharacterSize(14);
            warn.setPosition(x + 350, controlY);
            warn.setFillColor(Style::TextRed);
            window.draw(warn);
        }
    }
    
    // 4. 底部按钮
    bool can = CraftingManager::getInstance().canCraft(r, inventory, currentBatchAmount, currentSoulAmount);
    sf::RectangleShape btn(sf::Vector2f(120, 40));
    btn.setPosition(x, panelPosition.y + panelSize.y - 50);
    btn.setFillColor(can ? Style::ButtonNormal : Style::ButtonDisabled);
    window.draw(btn);
    
    sf::Text btnTxt;
    btnTxt.setFont(font);
    std::string bStr = r.isWeaponForge ? "开始锻造" : "合成";
    btnTxt.setString(sf::String::fromUtf8(bStr.begin(), bStr.end()));
    btnTxt.setCharacterSize(18);
    btnTxt.setPosition(x + 25, panelPosition.y + panelSize.y - 42);
    window.draw(btnTxt);
}

int CraftingPanel::getRecipeAtPosition(const sf::Vector2f& pos) const {
    float listWidth = 220.0f;
    float startY = panelPosition.y + 50;
    if (pos.x > panelPosition.x + 10 && pos.x < panelPosition.x + listWidth) {
        if (pos.y > startY && pos.y < panelPosition.y + panelSize.y - 10) {
            int idx = static_cast<int>((pos.y - startY) / 50.0f) + scrollOffset;
            if (idx >= 0 && idx < static_cast<int>(CraftingManager::getInstance().getAllRecipes().size())) return idx;
        }
    }
    return -1;
}