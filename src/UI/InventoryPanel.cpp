#include "InventoryPanel.h"
#include <iostream>
#include <sstream>

// 颜色常量定义
const sf::Color InventoryPanel::BG_COLOR(30, 30, 40, 240);
const sf::Color InventoryPanel::SLOT_COLOR(50, 50, 60, 200);
const sf::Color InventoryPanel::SLOT_HOVER_COLOR(70, 70, 90, 220);
const sf::Color InventoryPanel::SLOT_SELECTED_COLOR(100, 80, 60, 240);
const sf::Color InventoryPanel::BORDER_COLOR(139, 90, 43);

InventoryPanel::InventoryPanel()
    : inventory(nullptr)
    , panelOpen(false)
    , currentPage(0)
    , selectedSlot(-1)
    , hoveredSlot(-1)
    , showContextMenu(false)
    , playerGold(0)
    , iconLoaded(false)
    , fontLoaded(false)
{
    // 计算面板尺寸
    float contentWidth = INVENTORY_COLUMNS * (SLOT_SIZE + SLOT_PADDING) - SLOT_PADDING;
    float contentHeight = INVENTORY_ROWS * (SLOT_SIZE + SLOT_PADDING) - SLOT_PADDING;
    
    panelSize.x = contentWidth + PANEL_PADDING * 2;
    panelSize.y = contentHeight + PANEL_PADDING * 2 + 80;  // 额外空间给标题和底部
}

bool InventoryPanel::init(const std::string& iconPath) {
    // 加载图标
    if (iconTexture.loadFromFile(iconPath)) {
        iconSprite.setTexture(iconTexture);
        iconSprite.setScale(0.8f, 0.8f);
        iconLoaded = true;
        std::cout << "[InventoryPanel] Icon loaded: " << iconPath << std::endl;
    } else {
        // 创建占位图标
        sf::Image placeholder;
        placeholder.create(64, 64, sf::Color(100, 80, 60, 200));
        iconTexture.loadFromImage(placeholder);
        iconSprite.setTexture(iconTexture);
        iconLoaded = true;
        std::cout << "[InventoryPanel] Using placeholder icon" << std::endl;
    }
    
    // 尝试加载字体（优先使用系统中文字体）
    std::vector<std::string> fontPaths = {
        "C:/Windows/Fonts/msyh.ttc",        // 微软雅黑
        "C:/Windows/Fonts/simhei.ttf",      // 黑体
        "C:/Windows/Fonts/simsun.ttc",      // 宋体
        "../../assets/fonts/pixel.ttf",
        "../../assets/fonts/font.ttf",
        "assets/fonts/pixel.ttf",
    };
    
    for (const auto& path : fontPaths) {
        if (loadFont(path)) break;
    }
    
    return true;
}

bool InventoryPanel::loadFont(const std::string& fontPath) {
    if (font.loadFromFile(fontPath)) {
        fontLoaded = true;
        std::cout << "[InventoryPanel] Font loaded: " << fontPath << std::endl;
        return true;
    }
    return false;
}

void InventoryPanel::setIconPosition(float x, float y) {
    iconPosition = sf::Vector2f(x, y);
    iconSprite.setPosition(iconPosition);
}

void InventoryPanel::update(float /*dt*/) {
    // 动画更新（如果需要）
}

void InventoryPanel::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    
    // 检查图标点击（打开背包）
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        if (!panelOpen && iconSprite.getGlobalBounds().contains(mousePosF)) {
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
            close();
            return;
        }
    }
    
    if (!panelOpen) return;
    
    // 计算面板区域
    sf::FloatRect panelBounds(panelPosition, panelSize);
    
    // 鼠标移动 - 更新悬浮
    if (event.type == sf::Event::MouseMoved) {
        if (panelBounds.contains(mousePosF)) {
            hoveredSlot = getSlotAtPosition(mousePosF);
        } else {
            hoveredSlot = -1;
        }
        
        // 如果显示右键菜单，检查是否在菜单外
        if (showContextMenu) {
            sf::FloatRect menuBounds(contextMenuPos.x, contextMenuPos.y, 100, 80);
            if (!menuBounds.contains(mousePosF)) {
                // 可以选择关闭菜单
            }
        }
    }
    
    // 鼠标点击
    if (event.type == sf::Event::MouseButtonPressed) {
        // 关闭按钮区域（右上角）
        sf::FloatRect closeButton(
            panelPosition.x + panelSize.x - 30,
            panelPosition.y + 5,
            25, 25
        );
        
        if (event.mouseButton.button == sf::Mouse::Left) {
            // 关闭按钮
            if (closeButton.contains(mousePosF)) {
                close();
                return;
            }
            
            // 点击面板外部关闭
            if (!panelBounds.contains(mousePosF)) {
                close();
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
                inventory->sortInventory();
                return;
            }
            
            // 格子点击
            int clickedSlot = getSlotAtPosition(mousePosF);
            if (clickedSlot >= 0) {
                // Shift+点击 = 快速丢弃
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                    selectedSlot = clickedSlot;
                    dropSelectedItem();
                } else {
                    selectSlot(clickedSlot);
                }
                showContextMenu = false;
            }
        }
        else if (event.mouseButton.button == sf::Mouse::Right) {
            // 右键点击格子
            int clickedSlot = getSlotAtPosition(mousePosF);
            if (clickedSlot >= 0 && inventory) {
                int actualSlot = currentPage * INVENTORY_SLOTS_PER_PAGE + clickedSlot;
                const ItemStack& stack = inventory->getSlot(actualSlot);
                if (!stack.isEmpty()) {
                    const ItemData* data = ItemDatabase::getInstance().getItemData(stack.itemId);
                    if (data && data->type == ItemType::Consumable) {
                        // 直接使用消耗品
                        inventory->useItem(actualSlot);
                    } else {
                        // 显示右键菜单
                        selectedSlot = clickedSlot;
                        showContextMenu = true;
                        contextMenuPos = mousePosF;
                    }
                }
            }
        }
    }
}

void InventoryPanel::render(sf::RenderWindow& window) {
    // 始终渲染图标
    if (iconLoaded) {
        // 图标背景
        sf::RectangleShape iconBg(sf::Vector2f(60, 60));
        iconBg.setPosition(iconPosition.x - 5, iconPosition.y - 5);
        iconBg.setFillColor(sf::Color(30, 30, 40, 200));
        iconBg.setOutlineThickness(2);
        iconBg.setOutlineColor(BORDER_COLOR);
        window.draw(iconBg);
        window.draw(iconSprite);
    }
    
    if (!panelOpen || !inventory) return;
    
    // 计算面板位置（屏幕中央）
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
        title.setFillColor(sf::Color(255, 215, 0));
        title.setPosition(panelPosition.x + 15, panelPosition.y + 5);
        window.draw(title);
    }
    
    // 绘制关闭按钮
    sf::RectangleShape closeBtn(sf::Vector2f(25, 25));
    closeBtn.setPosition(panelPosition.x + panelSize.x - 30, panelPosition.y + 5);
    closeBtn.setFillColor(sf::Color(150, 50, 50, 200));
    closeBtn.setOutlineThickness(1);
    closeBtn.setOutlineColor(sf::Color::White);
    window.draw(closeBtn);
    
    if (fontLoaded) {
        sf::Text closeX;
        closeX.setFont(font);
        closeX.setString("X");
        closeX.setCharacterSize(16);
        closeX.setFillColor(sf::Color::White);
        closeX.setPosition(panelPosition.x + panelSize.x - 24, panelPosition.y + 6);
        window.draw(closeX);
    }
    
    // 绘制格子
    for (int i = 0; i < INVENTORY_SLOTS_PER_PAGE; i++) {
        renderSlot(window, i);
    }
    
    // 绘制底部栏
    float bottomY = panelPosition.y + panelSize.y - 40;
    
    // 翻页按钮和页码
    if (fontLoaded) {
        // 上一页
        sf::RectangleShape prevBtn(sf::Vector2f(30, 25));
        prevBtn.setPosition(panelPosition.x + 50, bottomY);
        prevBtn.setFillColor(currentPage > 0 ? sf::Color(80, 80, 100) : sf::Color(50, 50, 60));
        prevBtn.setOutlineThickness(1);
        prevBtn.setOutlineColor(sf::Color::White);
        window.draw(prevBtn);
        
        sf::Text prevText;
        prevText.setFont(font);
        prevText.setString("<");
        prevText.setCharacterSize(16);
        prevText.setFillColor(sf::Color::White);
        prevText.setPosition(panelPosition.x + 60, bottomY + 2);
        window.draw(prevText);
        
        // 页码
        sf::Text pageText;
        pageText.setFont(font);
        pageText.setString(std::to_string(currentPage + 1) + "/" + std::to_string(inventory->getTotalPages()));
        pageText.setCharacterSize(16);
        pageText.setFillColor(sf::Color::White);
        pageText.setPosition(panelPosition.x + panelSize.x / 2 - 20, bottomY + 2);
        window.draw(pageText);
        
        // 下一页
        sf::RectangleShape nextBtn(sf::Vector2f(30, 25));
        nextBtn.setPosition(panelPosition.x + panelSize.x - 80, bottomY);
        nextBtn.setFillColor(currentPage < inventory->getTotalPages() - 1 ? sf::Color(80, 80, 100) : sf::Color(50, 50, 60));
        nextBtn.setOutlineThickness(1);
        nextBtn.setOutlineColor(sf::Color::White);
        window.draw(nextBtn);
        
        sf::Text nextText;
        nextText.setFont(font);
        nextText.setString(">");
        nextText.setCharacterSize(16);
        nextText.setFillColor(sf::Color::White);
        nextText.setPosition(panelPosition.x + panelSize.x - 70, bottomY + 2);
        window.draw(nextText);
        
        // 整理按钮
        sf::RectangleShape sortBtn(sf::Vector2f(50, 25));
        sortBtn.setPosition(panelPosition.x + panelSize.x - 140, bottomY);
        sortBtn.setFillColor(sf::Color(60, 100, 60));
        sortBtn.setOutlineThickness(1);
        sortBtn.setOutlineColor(sf::Color::White);
        window.draw(sortBtn);
        
        sf::Text sortText;
        sortText.setFont(font);
        std::string sortStr = "整理";
        sortText.setString(sf::String::fromUtf8(sortStr.begin(), sortStr.end()));
        sortText.setCharacterSize(14);
        sortText.setFillColor(sf::Color::White);
        sortText.setPosition(panelPosition.x + panelSize.x - 133, bottomY + 3);
        window.draw(sortText);
        
        // 金币显示
        sf::Text goldText;
        goldText.setFont(font);
        std::ostringstream goldStream;
        goldStream << "金币: " << playerGold;
        std::string goldStr = goldStream.str();
        goldText.setString(sf::String::fromUtf8(goldStr.begin(), goldStr.end()));
        goldText.setCharacterSize(16);
        goldText.setFillColor(sf::Color(255, 215, 0));
        goldText.setPosition(panelPosition.x + 15, bottomY + 2);
        window.draw(goldText);
    }
    
    // 绘制悬浮提示
    renderTooltip(window);
    
    // 绘制右键菜单
    if (showContextMenu) {
        renderContextMenu(window);
    }
}

void InventoryPanel::renderSlot(sf::RenderWindow& window, int index) {
    int actualSlot = currentPage * INVENTORY_SLOTS_PER_PAGE + index;
    sf::Vector2f slotPos = getSlotPosition(index);
    
    // 确定格子颜色
    sf::Color slotColor = SLOT_COLOR;
    if (index == selectedSlot) {
        slotColor = SLOT_SELECTED_COLOR;
    } else if (index == hoveredSlot) {
        slotColor = SLOT_HOVER_COLOR;
    }
    
    // 绘制格子背景
    sf::RectangleShape slot(sf::Vector2f(SLOT_SIZE, SLOT_SIZE));
    slot.setPosition(slotPos);
    slot.setFillColor(slotColor);
    slot.setOutlineThickness(1);
    slot.setOutlineColor(sf::Color(80, 80, 80));
    window.draw(slot);
    
    // 绘制物品
    const ItemStack& stack = inventory->getSlot(actualSlot);
    if (!stack.isEmpty()) {
        const sf::Texture* tex = ItemDatabase::getInstance().getTexture(stack.itemId);
        if (tex) {
            sf::Sprite itemSprite(*tex);
            // 缩放到格子大小
            sf::Vector2u texSize = tex->getSize();
            float scale = (SLOT_SIZE - 8) / std::max(texSize.x, texSize.y);
            itemSprite.setScale(scale, scale);
            itemSprite.setPosition(slotPos.x + 4, slotPos.y + 4);
            window.draw(itemSprite);
        }
        
        // 绘制数量
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

void InventoryPanel::renderTooltip(sf::RenderWindow& window) {
    if (hoveredSlot < 0 || !fontLoaded) return;
    
    int actualSlot = currentPage * INVENTORY_SLOTS_PER_PAGE + hoveredSlot;
    const ItemStack& stack = inventory->getSlot(actualSlot);
    if (stack.isEmpty()) return;
    
    const ItemData* data = ItemDatabase::getInstance().getItemData(stack.itemId);
    if (!data) return;
    
    // 获取鼠标位置
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    float tooltipX = mousePos.x + 15.0f;
    float tooltipY = mousePos.y + 15.0f;
    
    // ========================================
    // 使用 sf::String 正确显示 UTF-8 中文
    // ========================================
    std::vector<std::pair<sf::String, sf::Color>> lines;
    
    // 名称（带稀有度颜色）
    lines.push_back({sf::String::fromUtf8(data->name.begin(), data->name.end()), 
                     ItemDatabase::getRarityColor(data->rarity)});
    
    // 类型
    std::string typeName = ItemDatabase::getTypeName(data->type);
    lines.push_back({sf::String::fromUtf8(typeName.begin(), typeName.end()), sf::Color(150, 150, 150)});
    
    // 描述
    if (!data->description.empty()) {
        lines.push_back({"", sf::Color::White});  // 空行
        lines.push_back({sf::String::fromUtf8(data->description.begin(), data->description.end()), 
                        sf::Color(200, 200, 200)});
    }
    
    // 效果（消耗品）
    if (!data->effects.empty()) {
        lines.push_back({"", sf::Color::White});
        for (const auto& effect : data->effects) {
            std::ostringstream ss;
            switch (effect.type) {
                case EffectType::RestoreHealth:
                    ss << "恢复生命值 +" << (int)effect.value;
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
    
    // 价格
    lines.push_back({"", sf::Color::White});
    std::ostringstream priceStream;
    priceStream << "出售: " << data->sellPrice << " 金币";
    std::string priceStr = priceStream.str();
    lines.push_back({sf::String::fromUtf8(priceStr.begin(), priceStr.end()), sf::Color(255, 215, 0)});
    
    // 计算提示框尺寸
    float lineHeight = 22.0f;
    float padding = 12.0f;
    float maxWidth = 0.0f;
    
    sf::Text measureText;
    measureText.setFont(font);
    measureText.setCharacterSize(16);
    
    for (const auto& line : lines) {
        measureText.setString(line.first);
        float width = measureText.getLocalBounds().width;
        if (width > maxWidth) maxWidth = width;
    }
    
    float tooltipWidth = std::max(180.0f, maxWidth + padding * 2);
    float tooltipHeight = lines.size() * lineHeight + padding * 2;
    
    // 确保不超出屏幕
    sf::Vector2u windowSize = window.getSize();
    if (tooltipX + tooltipWidth > windowSize.x) {
        tooltipX = windowSize.x - tooltipWidth - 10;
    }
    if (tooltipY + tooltipHeight > windowSize.y) {
        tooltipY = windowSize.y - tooltipHeight - 10;
    }
    
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
        text.setCharacterSize(i == 0 ? 18 : 16);
        text.setFillColor(lines[i].second);
        text.setPosition(tooltipX + padding, y);
        window.draw(text);
        
        y += lineHeight;
    }
}

void InventoryPanel::renderContextMenu(sf::RenderWindow& window) {
    if (!showContextMenu || selectedSlot < 0 || !fontLoaded) return;
    
    float menuWidth = 80;
    float menuHeight = 60;
    float itemHeight = 25;
    
    // 背景
    sf::RectangleShape bg(sf::Vector2f(menuWidth, menuHeight));
    bg.setPosition(contextMenuPos);
    bg.setFillColor(sf::Color(40, 40, 50, 240));
    bg.setOutlineThickness(1);
    bg.setOutlineColor(sf::Color::White);
    window.draw(bg);
    
    // 菜单项（使用 sf::String::fromUtf8）
    std::vector<sf::String> items;
    std::string item1 = "丢弃";
    std::string item2 = "取消";
    items.push_back(sf::String::fromUtf8(item1.begin(), item1.end()));
    items.push_back(sf::String::fromUtf8(item2.begin(), item2.end()));
    
    for (size_t i = 0; i < items.size(); i++) {
        sf::Text text;
        text.setFont(font);
        text.setString(items[i]);
        text.setCharacterSize(14);
        text.setFillColor(sf::Color::White);
        text.setPosition(contextMenuPos.x + 10, contextMenuPos.y + 5 + i * itemHeight);
        window.draw(text);
    }
}

sf::Vector2f InventoryPanel::getSlotPosition(int index) const {
    int col = index % INVENTORY_COLUMNS;
    int row = index / INVENTORY_COLUMNS;
    
    return sf::Vector2f(
        panelPosition.x + PANEL_PADDING + col * (SLOT_SIZE + SLOT_PADDING),
        panelPosition.y + 40 + row * (SLOT_SIZE + SLOT_PADDING)  // 40 = 标题栏高度
    );
}

int InventoryPanel::getSlotAtPosition(const sf::Vector2f& pos) const {
    for (int i = 0; i < INVENTORY_SLOTS_PER_PAGE; i++) {
        sf::Vector2f slotPos = getSlotPosition(i);
        sf::FloatRect slotBounds(slotPos.x, slotPos.y, SLOT_SIZE, SLOT_SIZE);
        if (slotBounds.contains(pos)) {
            return i;
        }
    }
    return -1;
}

void InventoryPanel::open() {
    panelOpen = true;
    selectedSlot = -1;
    showContextMenu = false;
    std::cout << "[InventoryPanel] Opened" << std::endl;
}

void InventoryPanel::close() {
    panelOpen = false;
    selectedSlot = -1;
    showContextMenu = false;
    std::cout << "[InventoryPanel] Closed" << std::endl;
}

void InventoryPanel::toggle() {
    if (panelOpen) close();
    else open();
}

void InventoryPanel::selectSlot(int index) {
    selectedSlot = index;
}

void InventoryPanel::useSelectedItem() {
    if (selectedSlot < 0 || !inventory) return;
    int actualSlot = currentPage * INVENTORY_SLOTS_PER_PAGE + selectedSlot;
    inventory->useItem(actualSlot);
}

void InventoryPanel::dropSelectedItem(int count) {
    if (selectedSlot < 0 || !inventory) return;
    
    int actualSlot = currentPage * INVENTORY_SLOTS_PER_PAGE + selectedSlot;
    ItemStack dropped = inventory->dropItem(actualSlot, count);
    
    if (!dropped.isEmpty() && onDropItem) {
        // 在玩家位置附近生成掉落物
        onDropItem(dropped, sf::Vector2f(0, 0));  // 位置由回调处理
    }
    
    selectedSlot = -1;
}

void InventoryPanel::nextPage() {
    if (inventory && currentPage < inventory->getTotalPages() - 1) {
        currentPage++;
        selectedSlot = -1;
    }
}

void InventoryPanel::prevPage() {
    if (currentPage > 0) {
        currentPage--;
        selectedSlot = -1;
    }
}
