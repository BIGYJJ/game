#include "CategoryInventoryPanel.h"
#include <iostream>
#include <sstream>

// 颜色常量定义
const sf::Color CategoryInventoryPanel::BG_COLOR(30, 30, 40, 240);
const sf::Color CategoryInventoryPanel::SLOT_COLOR(50, 50, 60, 200);
const sf::Color CategoryInventoryPanel::SLOT_HOVER_COLOR(70, 70, 90, 220);
const sf::Color CategoryInventoryPanel::SLOT_SELECTED_COLOR(100, 80, 60, 240);
const sf::Color CategoryInventoryPanel::TAB_COLOR(40, 40, 50, 220);
const sf::Color CategoryInventoryPanel::TAB_ACTIVE_COLOR(70, 60, 50, 240);
const sf::Color CategoryInventoryPanel::BORDER_COLOR(139, 90, 43);
const sf::Color CategoryInventoryPanel::MENU_BG_COLOR(40, 40, 50, 250);
const sf::Color CategoryInventoryPanel::MENU_HOVER_COLOR(70, 70, 90, 250);

CategoryInventoryPanel::CategoryInventoryPanel()
    : inventory(nullptr)
    , playerEquipment(nullptr)
    , panelOpen(false)
    , currentCategory(InventoryCategory::Materials)
    , currentPage(0)
    , selectedSlot(-1)
    , hoveredSlot(-1)
    , showContextMenu(false)
    , contextMenuSlot(-1)
    , hoveredMenuOption(-1)
    , lastClickedSlot(-1)
    , playerGold(0)
    , iconLoaded(false)
    , iconHovered(false)
    , iconHoverScale(1.0f)
    , iconTargetScale(1.0f)
    , fontLoaded(false)
{
    // 计算面板尺寸
    float contentWidth = CATEGORY_COLUMNS * (SLOT_SIZE + SLOT_PADDING) - SLOT_PADDING;
    float contentHeight = CATEGORY_ROWS * (SLOT_SIZE + SLOT_PADDING) - SLOT_PADDING;
    
    panelSize.x = contentWidth + PANEL_PADDING * 2;
    panelSize.y = contentHeight + PANEL_PADDING * 2 + TAB_HEIGHT + 80;
}

bool CategoryInventoryPanel::init(const std::string& iconPath) {
    if (iconTexture.loadFromFile(iconPath)) {
        iconSprite.setTexture(iconTexture);
        iconSprite.setScale(0.8f, 0.8f);
        iconLoaded = true;
        std::cout << "[CategoryInventoryPanel] Icon loaded: " << iconPath << std::endl;
    } else {
        sf::Image placeholder;
        placeholder.create(64, 64, sf::Color(100, 80, 60, 200));
        iconTexture.loadFromImage(placeholder);
        iconSprite.setTexture(iconTexture);
        iconLoaded = true;
        std::cout << "[CategoryInventoryPanel] Using placeholder icon" << std::endl;
    }
    
    std::vector<std::string> fontPaths = {
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/simhei.ttf",
        "C:/Windows/Fonts/simsun.ttc",
        "../../assets/fonts/pixel.ttf",
        "../../assets/fonts/font.ttf",
        "assets/fonts/pixel.ttf",
    };
    
    for (const auto& path : fontPaths) {
        if (loadFont(path)) break;
    }
    
    return true;
}

bool CategoryInventoryPanel::loadFont(const std::string& fontPath) {
    if (font.loadFromFile(fontPath)) {
        fontLoaded = true;
        std::cout << "[CategoryInventoryPanel] Font loaded: " << fontPath << std::endl;
        return true;
    }
    return false;
}

void CategoryInventoryPanel::setIconPosition(float x, float y) {
    iconPosition = sf::Vector2f(x, y);
    iconSprite.setPosition(iconPosition);
}

void CategoryInventoryPanel::update(float dt) {
    // 图标悬浮动画
    float scaleSpeed = 8.0f;
    iconHoverScale += (iconTargetScale - iconHoverScale) * scaleSpeed * dt;
    
    // 更新图标缩放（保持中心点）
    sf::Vector2f iconCenter = iconPosition + sf::Vector2f(
        iconTexture.getSize().x * ICON_BASE_SCALE / 2.0f,
        iconTexture.getSize().y * ICON_BASE_SCALE / 2.0f
    );
    iconSprite.setScale(ICON_BASE_SCALE * iconHoverScale, ICON_BASE_SCALE * iconHoverScale);
    iconSprite.setPosition(
        iconCenter.x - iconTexture.getSize().x * ICON_BASE_SCALE * iconHoverScale / 2.0f,
        iconCenter.y - iconTexture.getSize().y * ICON_BASE_SCALE * iconHoverScale / 2.0f
    );
}

void CategoryInventoryPanel::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    
    // 检查图标悬浮
    sf::FloatRect iconBounds(iconPosition.x - 5, iconPosition.y - 5, 60, 60);
    if (iconBounds.contains(mousePosF)) {
        iconHovered = true;
        iconTargetScale = 1.15f;
    } else {
        iconHovered = false;
        iconTargetScale = 1.0f;
    }
    
    // 检查图标点击
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        if (!panelOpen && iconBounds.contains(mousePosF)) {
            open();
            return;
        }
    }
    
    // 键盘快捷键
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::I || event.key.code == sf::Keyboard::B) {
            toggle();
            return;
        }
        if (event.key.code == sf::Keyboard::Escape && panelOpen) {
            if (showContextMenu) {
                showContextMenu = false;
            } else {
                close();
            }
            return;
        }
        
        // 分类切换快捷键 1, 2, 3
        if (panelOpen) {
            if (event.key.code == sf::Keyboard::Num1) {
                switchCategory(InventoryCategory::Materials);
            } else if (event.key.code == sf::Keyboard::Num2) {
                switchCategory(InventoryCategory::Consumables);
            } else if (event.key.code == sf::Keyboard::Num3) {
                switchCategory(InventoryCategory::Equipment);
            }
        }
    }
    
    if (!panelOpen) return;
    
    sf::FloatRect panelBounds(panelPosition, panelSize);
    
    // 鼠标移动
    if (event.type == sf::Event::MouseMoved) {
        if (panelBounds.contains(mousePosF)) {
            hoveredSlot = getSlotAtPosition(mousePosF);
        } else {
            hoveredSlot = -1;
        }
        
        // 右键菜单悬浮
        if (showContextMenu) {
            float menuWidth = 100;
            float menuItemHeight = 28;
            
            for (size_t i = 0; i < contextMenuOptions.size(); i++) {
                sf::FloatRect itemBounds(
                    contextMenuPos.x,
                    contextMenuPos.y + i * menuItemHeight,
                    menuWidth,
                    menuItemHeight
                );
                if (itemBounds.contains(mousePosF)) {
                    hoveredMenuOption = static_cast<int>(i);
                    break;
                } else {
                    hoveredMenuOption = -1;
                }
            }
        }
    }
    
    // 鼠标点击
    if (event.type == sf::Event::MouseButtonPressed) {
        // 关闭按钮
        sf::FloatRect closeButton(
            panelPosition.x + panelSize.x - 30,
            panelPosition.y + 5,
            25, 25
        );
        
        if (event.mouseButton.button == sf::Mouse::Left) {
            // 如果显示右键菜单，处理菜单点击
            if (showContextMenu) {
                float menuWidth = 100;
                float menuItemHeight = 28;
                float menuHeight = contextMenuOptions.size() * menuItemHeight;
                
                sf::FloatRect menuBounds(contextMenuPos.x, contextMenuPos.y, menuWidth, menuHeight);
                if (menuBounds.contains(mousePosF)) {
                    int optionIndex = static_cast<int>((mousePosF.y - contextMenuPos.y) / menuItemHeight);
                    handleContextMenuClick(optionIndex);
                }
                showContextMenu = false;
                return;
            }
            
            // 关闭按钮
            if (closeButton.contains(mousePosF)) {
                close();
                return;
            }
            
            // 点击面板外部
            if (!panelBounds.contains(mousePosF)) {
                close();
                return;
            }
            
            // 分类标签
            int tabIndex = getTabAtPosition(mousePosF);
            if (tabIndex >= 0) {
                switchCategory(static_cast<InventoryCategory>(tabIndex));
                return;
            }
            
            // 翻页按钮
            sf::FloatRect prevButton(panelPosition.x + 50, panelPosition.y + panelSize.y - 35, 30, 25);
            sf::FloatRect nextButton(panelPosition.x + panelSize.x - 80, panelPosition.y + panelSize.y - 35, 30, 25);
            
            if (prevButton.contains(mousePosF)) {
                prevPage();
                return;
            }
            if (nextButton.contains(mousePosF)) {
                nextPage();
                return;
            }
            
            // 整理按钮
            sf::FloatRect sortButton(panelPosition.x + panelSize.x - 140, panelPosition.y + panelSize.y - 35, 50, 25);
            if (sortButton.contains(mousePosF) && inventory) {
                inventory->sortCategory(currentCategory);
                return;
            }
            
            // 格子点击
            int clickedSlot = getSlotAtPosition(mousePosF);
            if (clickedSlot >= 0) {
                // 检测双击
                if (clickedSlot == lastClickedSlot && doubleClickClock.getElapsedTime().asSeconds() < DOUBLE_CLICK_TIME) {
                    handleDoubleClick(clickedSlot);
                    lastClickedSlot = -1;
                } else {
                    selectSlot(clickedSlot);
                    lastClickedSlot = clickedSlot;
                    doubleClickClock.restart();
                }
            }
        }
        else if (event.mouseButton.button == sf::Mouse::Right) {
            // 右键点击格子显示菜单
            int clickedSlot = getSlotAtPosition(mousePosF);
            if (clickedSlot >= 0 && inventory) {
                int actualSlot = currentPage * CATEGORY_SLOTS_PER_PAGE + clickedSlot;
                const ItemStack& stack = inventory->getSlot(currentCategory, actualSlot);
                
                if (!stack.isEmpty()) {
                    const ItemData* data = ItemDatabase::getInstance().getItemData(stack.itemId);
                    if (data) {
                        contextMenuSlot = actualSlot;
                        contextMenuOptions = CategoryInventory::getContextMenuOptions(data);
                        
                        if (!contextMenuOptions.empty()) {
                            showContextMenu = true;
                            contextMenuPos = mousePosF;
                            hoveredMenuOption = -1;
                        }
                    }
                }
            }
        }
    }
}

void CategoryInventoryPanel::handleDoubleClick(int slotIndex) {
    if (!inventory) return;
    
    int actualSlot = currentPage * CATEGORY_SLOTS_PER_PAGE + slotIndex;
    const ItemStack& stack = inventory->getSlot(currentCategory, actualSlot);
    if (stack.isEmpty()) return;
    
    const ItemData* data = ItemDatabase::getInstance().getItemData(stack.itemId);
    if (!data) return;
    
    switch (currentCategory) {
        case InventoryCategory::Consumables:
            // 双击种子时种下
            if (CategoryInventory::isSeed(stack.itemId)) {
                if (onPlant) {
                    if (onPlant()) {
                        inventory->plantSeed(actualSlot);
                    }
                } else {
                    inventory->plantSeed(actualSlot);
                }
            } else {
                // 双击使用消耗品
                inventory->useItem(currentCategory, actualSlot);
            }
            break;
            
        case InventoryCategory::Equipment:
            // 双击装备
            inventory->equipItem(actualSlot);
            break;
            
        default:
            break;
    }
}

void CategoryInventoryPanel::handleContextMenuClick(int optionIndex) {
    if (optionIndex < 0 || optionIndex >= static_cast<int>(contextMenuOptions.size())) return;
    if (!inventory) return;
    
    ContextMenuOption option = contextMenuOptions[optionIndex];
    
    switch (option) {
        case ContextMenuOption::Use:
            inventory->useItem(currentCategory, contextMenuSlot);
            break;
            
        case ContextMenuOption::Equip:
            inventory->equipItem(contextMenuSlot);
            break;
            
        case ContextMenuOption::Plant:
            if (onPlant) {
                if (onPlant()) {
                    inventory->plantSeed(contextMenuSlot);
                }
            } else {
                inventory->plantSeed(contextMenuSlot);
            }
            break;
            
        case ContextMenuOption::Destroy:
            inventory->destroyItem(currentCategory, contextMenuSlot);
            break;
            
        case ContextMenuOption::Sell: {
            int gold = inventory->sellItem(currentCategory, contextMenuSlot);
            if (gold > 0 && onSell) {
                onSell(gold);
            }
            break;
        }
            
        default:
            break;
    }
}

void CategoryInventoryPanel::render(sf::RenderWindow& window) {
    // 始终渲染图标
    if (iconLoaded) {
        sf::RectangleShape iconBg(sf::Vector2f(60, 60));
        iconBg.setPosition(iconPosition.x - 5, iconPosition.y - 5);
        iconBg.setFillColor(sf::Color(30, 30, 40, 200));
        iconBg.setOutlineThickness(2);
        iconBg.setOutlineColor(BORDER_COLOR);
        window.draw(iconBg);
        window.draw(iconSprite);
    }
    
    if (!panelOpen || !inventory) return;
    
    // 计算面板位置
    sf::Vector2u windowSize = window.getSize();
    panelPosition.x = (windowSize.x - panelSize.x) / 2;
    panelPosition.y = (windowSize.y - panelSize.y) / 2;
    
    // 绘制背景
    sf::RectangleShape bg(panelSize);
    bg.setPosition(panelPosition);
    bg.setFillColor(BG_COLOR);
    bg.setOutlineThickness(3);
    bg.setOutlineColor(BORDER_COLOR);
    window.draw(bg);
    
    // 绘制标题栏
    sf::RectangleShape titleBar(sf::Vector2f(panelSize.x - 6, 30));
    titleBar.setPosition(panelPosition.x + 3, panelPosition.y + 3);
    titleBar.setFillColor(sf::Color(60, 45, 30, 200));
    window.draw(titleBar);
    
    if (fontLoaded) {
        sf::Text title;
        title.setFont(font);
        std::string titleStr = "背包";
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
    
    // 渲染分类标签
    renderCategoryTabs(window);
    
    // 渲染物品格子
    renderSlots(window);
    
    // 渲染底部控制栏
    float bottomY = panelPosition.y + panelSize.y - 40;
    
    // 翻页按钮
    sf::RectangleShape prevBtn(sf::Vector2f(30, 25));
    prevBtn.setPosition(panelPosition.x + 50, bottomY);
    prevBtn.setFillColor(currentPage > 0 ? sf::Color(60, 60, 70) : sf::Color(40, 40, 45));
    prevBtn.setOutlineThickness(1);
    prevBtn.setOutlineColor(sf::Color(80, 80, 80));
    window.draw(prevBtn);
    
    sf::RectangleShape nextBtn(sf::Vector2f(30, 25));
    nextBtn.setPosition(panelPosition.x + panelSize.x - 80, bottomY);
    int totalPages = inventory->getCategoryPages(currentCategory);
    nextBtn.setFillColor(currentPage < totalPages - 1 ? sf::Color(60, 60, 70) : sf::Color(40, 40, 45));
    nextBtn.setOutlineThickness(1);
    nextBtn.setOutlineColor(sf::Color(80, 80, 80));
    window.draw(nextBtn);
    
    if (fontLoaded) {
        sf::Text prevText, nextText;
        prevText.setFont(font);
        nextText.setFont(font);
        prevText.setString("<");
        nextText.setString(">");
        prevText.setCharacterSize(16);
        nextText.setCharacterSize(16);
        prevText.setFillColor(sf::Color::White);
        nextText.setFillColor(sf::Color::White);
        prevText.setPosition(panelPosition.x + 60, bottomY + 2);
        nextText.setPosition(panelPosition.x + panelSize.x - 70, bottomY + 2);
        window.draw(prevText);
        window.draw(nextText);
        
        // 页码
        std::stringstream ss;
        ss << (currentPage + 1) << "/" << totalPages;
        sf::Text pageText;
        pageText.setFont(font);
        pageText.setString(ss.str());
        pageText.setCharacterSize(14);
        pageText.setFillColor(sf::Color(200, 200, 200));
        sf::FloatRect bounds = pageText.getLocalBounds();
        pageText.setPosition(panelPosition.x + (panelSize.x - bounds.width) / 2, bottomY + 4);
        window.draw(pageText);
    }
    
    // 整理按钮
    sf::RectangleShape sortBtn(sf::Vector2f(50, 25));
    sortBtn.setPosition(panelPosition.x + panelSize.x - 140, bottomY);
    sortBtn.setFillColor(sf::Color(60, 60, 70));
    sortBtn.setOutlineThickness(1);
    sortBtn.setOutlineColor(sf::Color(80, 80, 80));
    window.draw(sortBtn);
    
    if (fontLoaded) {
        sf::Text sortText;
        sortText.setFont(font);
        std::string sortStr = "整理";
        sortText.setString(sf::String::fromUtf8(sortStr.begin(), sortStr.end()));
        sortText.setCharacterSize(12);
        sortText.setFillColor(sf::Color::White);
        sortText.setPosition(panelPosition.x + panelSize.x - 132, bottomY + 5);
        window.draw(sortText);
    }
    
    // 金币显示（移到标题栏右侧，避免与翻页按钮重叠）
    if (fontLoaded) {
        std::stringstream goldSS;
        goldSS << "金币: " << playerGold;
        std::string goldStr = goldSS.str();
        sf::Text goldText;
        goldText.setFont(font);
        goldText.setString(sf::String::fromUtf8(goldStr.begin(), goldStr.end()));
        goldText.setCharacterSize(14);
        goldText.setFillColor(sf::Color(255, 215, 0));
        // 将金币显示移到标题栏右侧（关闭按钮左边）
        sf::FloatRect goldBounds = goldText.getLocalBounds();
        goldText.setPosition(panelPosition.x + panelSize.x - goldBounds.width - 35, panelPosition.y + 8);
        window.draw(goldText);
    }
    
    // 渲染提示框
    renderTooltip(window);
    
    // 渲染右键菜单
    renderContextMenu(window);
}

void CategoryInventoryPanel::renderCategoryTabs(sf::RenderWindow& window) {
    float tabY = panelPosition.y + 35;
    float tabWidth = (panelSize.x - 20) / 3;
    
    std::vector<std::string> tabNames = {"材料", "消耗品", "装备"};
    
    for (int i = 0; i < 3; i++) {
        InventoryCategory cat = static_cast<InventoryCategory>(i);
        bool isActive = (cat == currentCategory);
        
        sf::RectangleShape tab(sf::Vector2f(tabWidth - 4, TAB_HEIGHT - 4));
        tab.setPosition(panelPosition.x + 12 + i * tabWidth, tabY);
        tab.setFillColor(isActive ? TAB_ACTIVE_COLOR : TAB_COLOR);
        tab.setOutlineThickness(isActive ? 2 : 1);
        tab.setOutlineColor(isActive ? sf::Color(200, 150, 100) : sf::Color(80, 80, 80));
        window.draw(tab);
        
        if (fontLoaded) {
            sf::Text tabText;
            tabText.setFont(font);
            tabText.setString(sf::String::fromUtf8(tabNames[i].begin(), tabNames[i].end()));
            tabText.setCharacterSize(14);
            tabText.setFillColor(isActive ? sf::Color(255, 220, 150) : sf::Color(180, 180, 180));
            
            sf::FloatRect bounds = tabText.getLocalBounds();
            tabText.setPosition(
                panelPosition.x + 12 + i * tabWidth + (tabWidth - 4 - bounds.width) / 2,
                tabY + 8
            );
            window.draw(tabText);
        }
    }
}

void CategoryInventoryPanel::renderSlots(sf::RenderWindow& window) {
    for (int i = 0; i < CATEGORY_SLOTS_PER_PAGE; i++) {
        renderSlot(window, i);
    }
}

void CategoryInventoryPanel::renderSlot(sf::RenderWindow& window, int index) {
    sf::Vector2f slotPos = getSlotPosition(index);
    
    bool isHovered = (index == hoveredSlot);
    bool isSelected = (index == selectedSlot);
    
    // 格子背景
    sf::RectangleShape slotBg(sf::Vector2f(SLOT_SIZE, SLOT_SIZE));
    slotBg.setPosition(slotPos);
    
    if (isSelected) {
        slotBg.setFillColor(SLOT_SELECTED_COLOR);
    } else if (isHovered) {
        slotBg.setFillColor(SLOT_HOVER_COLOR);
    } else {
        slotBg.setFillColor(SLOT_COLOR);
    }
    
    slotBg.setOutlineThickness(1);
    slotBg.setOutlineColor(isSelected ? sf::Color(200, 150, 100) : sf::Color(60, 60, 60));
    window.draw(slotBg);
    
    // 获取实际格子索引
    int actualSlot = currentPage * CATEGORY_SLOTS_PER_PAGE + index;
    const ItemStack& stack = inventory->getSlot(currentCategory, actualSlot);
    
    if (!stack.isEmpty()) {
        // 获取物品贴图
        const sf::Texture* tex = ItemDatabase::getInstance().getTexture(stack.itemId);
        if (tex) {
            sf::Sprite itemSprite(*tex);
            
            // 缩放到格子大小
            float scale = (SLOT_SIZE - 8) / std::max(tex->getSize().x, tex->getSize().y);
            itemSprite.setScale(scale, scale);
            
            // 居中
            sf::FloatRect spriteBounds = itemSprite.getGlobalBounds();
            itemSprite.setPosition(
                slotPos.x + (SLOT_SIZE - spriteBounds.width) / 2,
                slotPos.y + (SLOT_SIZE - spriteBounds.height) / 2
            );
            
            window.draw(itemSprite);
        }
        
        // 数量显示
        if (stack.count > 1 && fontLoaded) {
            sf::Text countText;
            countText.setFont(font);
            countText.setString(std::to_string(stack.count));
            countText.setCharacterSize(12);
            countText.setFillColor(sf::Color::White);
            countText.setOutlineColor(sf::Color::Black);
            countText.setOutlineThickness(1);
            
            sf::FloatRect bounds = countText.getLocalBounds();
            countText.setPosition(
                slotPos.x + SLOT_SIZE - bounds.width - 4,
                slotPos.y + SLOT_SIZE - bounds.height - 8
            );
            window.draw(countText);
        }
    }
}

void CategoryInventoryPanel::renderTooltip(sf::RenderWindow& window) {
    if (hoveredSlot < 0 || !fontLoaded || showContextMenu) return;
    
    int actualSlot = currentPage * CATEGORY_SLOTS_PER_PAGE + hoveredSlot;
    const ItemStack& stack = inventory->getSlot(currentCategory, actualSlot);
    if (stack.isEmpty()) return;
    
    const ItemData* data = ItemDatabase::getInstance().getItemData(stack.itemId);
    if (!data) return;
    
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    float tooltipX = mousePos.x + 15.0f;
    float tooltipY = mousePos.y + 15.0f;
    
    std::vector<std::pair<sf::String, sf::Color>> lines;
    
    // 名称
    lines.push_back({sf::String::fromUtf8(data->name.begin(), data->name.end()), 
                     ItemDatabase::getRarityColor(data->rarity)});
    
    // 类型
    std::string typeName = ItemDatabase::getTypeName(data->type);
    if (CategoryInventory::isSeed(data->id)) {
        typeName = "种子";
    }
    lines.push_back({sf::String::fromUtf8(typeName.begin(), typeName.end()), sf::Color(150, 150, 150)});
    
    // 描述
    if (!data->description.empty()) {
        lines.push_back({"", sf::Color::White});
        lines.push_back({sf::String::fromUtf8(data->description.begin(), data->description.end()), 
                        sf::Color(200, 200, 200)});
    }
    
    // 消耗品效果
    if (!data->effects.empty()) {
        lines.push_back({"", sf::Color::White});
        for (const auto& effect : data->effects) {
            std::ostringstream ss;
            switch (effect.type) {
                case EffectType::RestoreHealth:
                    ss << "恢复生命 +" << (int)effect.value;
                    break;
                case EffectType::RestoreStamina:
                    ss << "恢复体力 +" << (int)effect.value;
                    break;
                case EffectType::BuffAttack:
                    ss << "攻击力 +" << (int)effect.value;
                    if (effect.duration > 0) ss << " (" << (int)effect.duration << "秒)";
                    break;
                default:
                    continue;
            }
            std::string effectStr = ss.str();
            lines.push_back({sf::String::fromUtf8(effectStr.begin(), effectStr.end()), 
                            sf::Color(100, 255, 100)});
        }
    }
    
    // 操作提示
    lines.push_back({"", sf::Color::White});
    std::string tipStr;
    switch (currentCategory) {
        case InventoryCategory::Materials:
            tipStr = "右键: 销毁/卖出";
            break;
        case InventoryCategory::Consumables:
            if (CategoryInventory::isSeed(data->id)) {
                tipStr = "右键: 种下/销毁/卖出";
            } else {
                tipStr = "双击: 使用 | 右键: 菜单";
            }
            break;
        case InventoryCategory::Equipment:
            tipStr = "双击: 装备 | 右键: 菜单";
            break;
        default:
            break;
    }
    lines.push_back({sf::String::fromUtf8(tipStr.begin(), tipStr.end()), sf::Color(180, 180, 100)});
    
    // 价格
    lines.push_back({"", sf::Color::White});
    std::ostringstream priceStream;
    priceStream << "出售: " << data->sellPrice << " 金币";
    std::string priceStr = priceStream.str();
    lines.push_back({sf::String::fromUtf8(priceStr.begin(), priceStr.end()), sf::Color(255, 215, 0)});
    
    // 计算尺寸
    float lineHeight = 20.0f;
    float padding = 10.0f;
    float maxWidth = 0.0f;
    
    sf::Text measureText;
    measureText.setFont(font);
    measureText.setCharacterSize(14);
    
    for (const auto& line : lines) {
        measureText.setString(line.first);
        float width = measureText.getLocalBounds().width;
        if (width > maxWidth) maxWidth = width;
    }
    
    float tooltipWidth = std::max(180.0f, maxWidth + padding * 2);
    float tooltipHeight = lines.size() * lineHeight + padding * 2;
    
    // 边界检查
    sf::Vector2u windowSize = window.getSize();
    if (tooltipX + tooltipWidth > windowSize.x) tooltipX = windowSize.x - tooltipWidth - 10;
    if (tooltipY + tooltipHeight > windowSize.y) tooltipY = windowSize.y - tooltipHeight - 10;
    
    // 绘制背景
    sf::RectangleShape bg(sf::Vector2f(tooltipWidth, tooltipHeight));
    bg.setPosition(tooltipX, tooltipY);
    bg.setFillColor(sf::Color(20, 20, 30, 245));
    bg.setOutlineThickness(2);
    bg.setOutlineColor(ItemDatabase::getRarityColor(data->rarity));
    window.draw(bg);
    
    // 绘制文字
    float y = tooltipY + padding;
    for (size_t i = 0; i < lines.size(); i++) {
        if (lines[i].first.isEmpty()) {
            y += lineHeight * 0.3f;
            continue;
        }
        
        sf::Text text;
        text.setFont(font);
        text.setString(lines[i].first);
        text.setCharacterSize(i == 0 ? 16 : 14);
        text.setFillColor(lines[i].second);
        text.setPosition(tooltipX + padding, y);
        window.draw(text);
        
        y += lineHeight;
    }
}

void CategoryInventoryPanel::renderContextMenu(sf::RenderWindow& window) {
    if (!showContextMenu || contextMenuOptions.empty() || !fontLoaded) return;
    
    float menuWidth = 100;
    float menuItemHeight = 28;
    float menuHeight = contextMenuOptions.size() * menuItemHeight;
    
    // 背景
    sf::RectangleShape bg(sf::Vector2f(menuWidth, menuHeight));
    bg.setPosition(contextMenuPos);
    bg.setFillColor(MENU_BG_COLOR);
    bg.setOutlineThickness(2);
    bg.setOutlineColor(sf::Color(100, 100, 100));
    window.draw(bg);
    
    // 菜单项
    for (size_t i = 0; i < contextMenuOptions.size(); i++) {
        bool isHovered = (static_cast<int>(i) == hoveredMenuOption);
        
        if (isHovered) {
            sf::RectangleShape hoverBg(sf::Vector2f(menuWidth - 4, menuItemHeight - 2));
            hoverBg.setPosition(contextMenuPos.x + 2, contextMenuPos.y + i * menuItemHeight + 1);
            hoverBg.setFillColor(MENU_HOVER_COLOR);
            window.draw(hoverBg);
        }
        
        std::string optionName = getContextMenuOptionName(contextMenuOptions[i]);
        sf::Text text;
        text.setFont(font);
        text.setString(sf::String::fromUtf8(optionName.begin(), optionName.end()));
        text.setCharacterSize(14);
        text.setFillColor(sf::Color::White);
        text.setPosition(contextMenuPos.x + 10, contextMenuPos.y + i * menuItemHeight + 5);
        window.draw(text);
    }
}

sf::Vector2f CategoryInventoryPanel::getSlotPosition(int index) const {
    int col = index % CATEGORY_COLUMNS;
    int row = index / CATEGORY_COLUMNS;
    
    return sf::Vector2f(
        panelPosition.x + PANEL_PADDING + col * (SLOT_SIZE + SLOT_PADDING),
        panelPosition.y + 40 + TAB_HEIGHT + row * (SLOT_SIZE + SLOT_PADDING)
    );
}

int CategoryInventoryPanel::getSlotAtPosition(const sf::Vector2f& pos) const {
    for (int i = 0; i < CATEGORY_SLOTS_PER_PAGE; i++) {
        sf::Vector2f slotPos = getSlotPosition(i);
        sf::FloatRect slotBounds(slotPos.x, slotPos.y, SLOT_SIZE, SLOT_SIZE);
        if (slotBounds.contains(pos)) {
            return i;
        }
    }
    return -1;
}

int CategoryInventoryPanel::getTabAtPosition(const sf::Vector2f& pos) const {
    float tabY = panelPosition.y + 35;
    float tabWidth = (panelSize.x - 20) / 3;
    
    for (int i = 0; i < 3; i++) {
        sf::FloatRect tabBounds(
            panelPosition.x + 12 + i * tabWidth,
            tabY,
            tabWidth - 4,
            TAB_HEIGHT - 4
        );
        if (tabBounds.contains(pos)) {
            return i;
        }
    }
    return -1;
}

void CategoryInventoryPanel::selectSlot(int index) {
    selectedSlot = index;
}

void CategoryInventoryPanel::switchCategory(InventoryCategory category) {
    if (currentCategory != category) {
        currentCategory = category;
        currentPage = 0;
        selectedSlot = -1;
        showContextMenu = false;
        std::cout << "[CategoryInventoryPanel] Switched to " 
                  << CategoryInventory::getCategoryName(category) << std::endl;
    }
}

void CategoryInventoryPanel::nextPage() {
    if (inventory) {
        int totalPages = inventory->getCategoryPages(currentCategory);
        if (currentPage < totalPages - 1) {
            currentPage++;
            selectedSlot = -1;
        }
    }
}

void CategoryInventoryPanel::prevPage() {
    if (currentPage > 0) {
        currentPage--;
        selectedSlot = -1;
    }
}

void CategoryInventoryPanel::open() {
    panelOpen = true;
    selectedSlot = -1;
    showContextMenu = false;
    std::cout << "[CategoryInventoryPanel] Opened" << std::endl;
}

void CategoryInventoryPanel::close() {
    panelOpen = false;
    selectedSlot = -1;
    showContextMenu = false;
    std::cout << "[CategoryInventoryPanel] Closed" << std::endl;
}

void CategoryInventoryPanel::toggle() {
    if (panelOpen) close();
    else open();
}
