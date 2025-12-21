#include "PetPanel.h"
#include <iostream>
#include <sstream>
#include <iomanip>

// UTF-8 中文字符串辅助函数
static sf::String toSfString(const std::string& str) {
    return sf::String::fromUtf8(str.begin(), str.end());
}

// ============================================================================
// PetPanel 实现 - 宠物栏功能图标
// 布局：左侧宠物列表 | 右侧详细信息 | 右下技能格子
// ============================================================================

PetPanel::PetPanel()
    : petManager(nullptr)
    , fontLoaded(false)
    , iconLoaded(false)
    , panelOpen(false)
    , panelSize(680, 540)  // 增大面板尺寸以适应更大的UI元素
    , hoveredSlot(-1)
    , playerLuck(0)
    , cleanserCount(0)
{
}

bool PetPanel::init(const std::string& iconPath) {
    if (iconTexture.loadFromFile(iconPath)) {
        iconSprite.setTexture(iconTexture);
        float scale = ICON_SIZE / iconTexture.getSize().x;
        iconSprite.setScale(scale, scale);
        iconLoaded = true;
    } else {
        sf::Image placeholder;
        placeholder.create(48, 48, sf::Color(100, 150, 100));
        iconTexture.loadFromImage(placeholder);
        iconSprite.setTexture(iconTexture);
        iconLoaded = false;
    }
    
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

bool PetPanel::loadFont(const std::string& fontPath) {
    if (font.loadFromFile(fontPath)) {
        fontLoaded = true;
        std::cout << "[PetPanel] Font loaded: " << fontPath << std::endl;
        return true;
    }
    return false;
}

void PetPanel::setIconPosition(float x, float y) {
    iconPosition = sf::Vector2f(x, y);
    iconSprite.setPosition(iconPosition);
    panelPos = sf::Vector2f(x - panelSize.x / 2 + ICON_SIZE / 2, y - panelSize.y - 10);
    
    // 确保面板不超出屏幕左边
    if (panelPos.x < 10) panelPos.x = 10;
}

void PetPanel::update(float dt) {
    (void)dt;
}

void PetPanel::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    sf::Vector2i mouseScreenPos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePos(static_cast<float>(mouseScreenPos.x), static_cast<float>(mouseScreenPos.y));
    
    if (event.type == sf::Event::MouseButtonPressed && 
        event.mouseButton.button == sf::Mouse::Left) {
        
        if (getIconRect().contains(mousePos)) {
            toggle();
            return;
        }
        
        if (panelOpen) {
            if (getCloseButtonRect().contains(mousePos)) {
                close();
                return;
            }
            
            // 检查宠物槽位点击（左侧列表）
            if (petManager) {
                for (int i = 0; i < petManager->getMaxPetSlots(); i++) {
                    if (getSlotRect(i).contains(mousePos)) {
                        if (petManager->getPetAt(i) && onSwitchPet) {
                            onSwitchPet(i);
                        }
                        return;
                    }
                }
            }
            
            // 检查洗点按钮
            if (getWashButtonRect().contains(mousePos)) {
                if (cleanserCount > 0 && petManager && onWash) {
                    int currentIdx = petManager->getCurrentPetIndex();
                    if (currentIdx >= 0) {
                        onWash(currentIdx, playerLuck);
                    }
                }
                return;
            }
        }
    }
    
    // 更新悬浮状态
    if (panelOpen && petManager) {
        hoveredSlot = -1;
        for (int i = 0; i < petManager->getMaxPetSlots(); i++) {
            if (getSlotRect(i).contains(mousePos)) {
                hoveredSlot = i;
                break;
            }
        }
    }
}

void PetPanel::render(sf::RenderWindow& window) {
    renderIcon(window);
    if (panelOpen) {
        renderPanel(window);
    }
}

void PetPanel::renderIcon(sf::RenderWindow& window) {
    // 像素风格的图标背景 - 多层边框
    float bgSize = ICON_SIZE + 12;
    
    // 外层阴影
    sf::RectangleShape shadow(sf::Vector2f(bgSize, bgSize));
    shadow.setPosition(iconPosition.x - 6 + 3, iconPosition.y - 6 + 3);
    shadow.setFillColor(sf::Color(20, 20, 20, 150));
    window.draw(shadow);
    
    // 主背景
    sf::RectangleShape iconBg(sf::Vector2f(bgSize, bgSize));
    iconBg.setPosition(iconPosition.x - 6, iconPosition.y - 6);
    iconBg.setFillColor(panelOpen ? sf::Color(60, 90, 60, 240) : sf::Color(35, 35, 45, 240));
    window.draw(iconBg);
    
    // 像素风格边框 - 外层
    sf::RectangleShape outerBorder(sf::Vector2f(bgSize, bgSize));
    outerBorder.setPosition(iconPosition.x - 6, iconPosition.y - 6);
    outerBorder.setFillColor(sf::Color::Transparent);
    outerBorder.setOutlineThickness(3);
    outerBorder.setOutlineColor(panelOpen ? sf::Color(100, 160, 100) : sf::Color(70, 70, 90));
    window.draw(outerBorder);
    
    // 内层高光边框
    sf::RectangleShape innerBorder(sf::Vector2f(bgSize - 6, bgSize - 6));
    innerBorder.setPosition(iconPosition.x - 3, iconPosition.y - 3);
    innerBorder.setFillColor(sf::Color::Transparent);
    innerBorder.setOutlineThickness(2);
    innerBorder.setOutlineColor(panelOpen ? sf::Color(140, 200, 140, 150) : sf::Color(100, 100, 120, 150));
    window.draw(innerBorder);
    
    window.draw(iconSprite);
    
    if (fontLoaded) {
        std::string labelStr = "宠物";
        sf::Text label(toSfString(labelStr), font, 14);
        // 文字阴影
        sf::Text labelShadow = label;
        labelShadow.setPosition(iconPosition.x + ICON_SIZE / 2 - 14 + 1, iconPosition.y + ICON_SIZE + 6 + 1);
        labelShadow.setFillColor(sf::Color(20, 20, 20, 200));
        window.draw(labelShadow);
        
        label.setPosition(iconPosition.x + ICON_SIZE / 2 - 14, iconPosition.y + ICON_SIZE + 6);
        label.setFillColor(sf::Color::White);
        window.draw(label);
    }
}

void PetPanel::renderPanel(sf::RenderWindow& window) {
    // 像素风格的面板背景 - 阴影
    sf::RectangleShape shadow(panelSize + sf::Vector2f(6, 6));
    shadow.setPosition(panelPos.x + 4, panelPos.y + 4);
    shadow.setFillColor(sf::Color(15, 15, 15, 180));
    window.draw(shadow);
    
    // 主背景 - 像素风格深色
    sf::RectangleShape background(panelSize);
    background.setPosition(panelPos);
    background.setFillColor(sf::Color(35, 32, 28, 250));
    window.draw(background);
    
    // 像素风格多层边框
    // 外层深色边框
    sf::RectangleShape outerBorder(panelSize);
    outerBorder.setPosition(panelPos);
    outerBorder.setFillColor(sf::Color::Transparent);
    outerBorder.setOutlineThickness(4);
    outerBorder.setOutlineColor(sf::Color(90, 75, 55));
    window.draw(outerBorder);
    
    // 中层边框
    sf::RectangleShape midBorder(panelSize - sf::Vector2f(8, 8));
    midBorder.setPosition(panelPos.x + 4, panelPos.y + 4);
    midBorder.setFillColor(sf::Color::Transparent);
    midBorder.setOutlineThickness(2);
    midBorder.setOutlineColor(sf::Color(140, 120, 90));
    window.draw(midBorder);
    
    // 内层高光
    sf::RectangleShape innerHighlight(panelSize - sf::Vector2f(12, 12));
    innerHighlight.setPosition(panelPos.x + 6, panelPos.y + 6);
    innerHighlight.setFillColor(sf::Color::Transparent);
    innerHighlight.setOutlineThickness(1);
    innerHighlight.setOutlineColor(sf::Color(180, 160, 120, 100));
    window.draw(innerHighlight);
    
    // 标题栏 - 像素风格
    sf::RectangleShape titleBar(sf::Vector2f(panelSize.x - 8, 44));
    titleBar.setPosition(panelPos.x + 4, panelPos.y + 4);
    titleBar.setFillColor(sf::Color(55, 45, 38));
    titleBar.setOutlineThickness(2);
    titleBar.setOutlineColor(sf::Color(80, 65, 50));
    window.draw(titleBar);
    
    // 标题栏底部高光
    sf::RectangleShape titleHighlight(sf::Vector2f(panelSize.x - 12, 2));
    titleHighlight.setPosition(panelPos.x + 6, panelPos.y + 46);
    titleHighlight.setFillColor(sf::Color(120, 100, 75));
    window.draw(titleHighlight);
    
    if (fontLoaded) {
        std::string titleStr = "我的宠物";
        sf::Text title(toSfString(titleStr), font, 24);
        // 标题阴影
        sf::Text titleShadow = title;
        titleShadow.setPosition(panelPos.x + panelSize.x / 2 - 50 + 2, panelPos.y + 10 + 2);
        titleShadow.setFillColor(sf::Color(20, 20, 20, 200));
        window.draw(titleShadow);
        
        title.setPosition(panelPos.x + panelSize.x / 2 - 50, panelPos.y + 10);
        title.setFillColor(sf::Color(230, 210, 170));
        window.draw(title);
        
        // 关闭按钮 - 像素风格
        sf::FloatRect closeRect = getCloseButtonRect();
        
        // 按钮阴影
        sf::RectangleShape closeShadow(sf::Vector2f(32, 32));
        closeShadow.setPosition(closeRect.left + 2, closeRect.top + 2);
        closeShadow.setFillColor(sf::Color(30, 15, 15, 180));
        window.draw(closeShadow);
        
        sf::RectangleShape closeBtn(sf::Vector2f(32, 32));
        closeBtn.setPosition(closeRect.left, closeRect.top);
        closeBtn.setFillColor(sf::Color(140, 50, 50));
        closeBtn.setOutlineThickness(3);
        closeBtn.setOutlineColor(sf::Color(100, 35, 35));
        window.draw(closeBtn);
        
        // 按钮内边框高光
        sf::RectangleShape closeHighlight(sf::Vector2f(26, 26));
        closeHighlight.setPosition(closeRect.left + 3, closeRect.top + 3);
        closeHighlight.setFillColor(sf::Color::Transparent);
        closeHighlight.setOutlineThickness(1);
        closeHighlight.setOutlineColor(sf::Color(200, 100, 100, 150));
        window.draw(closeHighlight);
        
        sf::Text closeText("X", font, 20);
        closeText.setPosition(closeRect.left + 9, closeRect.top + 4);
        closeText.setFillColor(sf::Color::White);
        window.draw(closeText);
    }
    
    // 左侧区域标题
    float leftX = panelPos.x + 18;
    float leftY = panelPos.y + 58;
    
    if (fontLoaded) {
        std::string myPetStr = "我的宠物";
        sf::Text myPetTitle(toSfString(myPetStr), font, 18);
        myPetTitle.setPosition(leftX, leftY);
        myPetTitle.setFillColor(sf::Color(190, 170, 140));
        window.draw(myPetTitle);
    }
    
    // 左侧分隔线 - 像素风格双线
    sf::RectangleShape leftDivider(sf::Vector2f(150, 3));
    leftDivider.setPosition(leftX, leftY + 28);
    leftDivider.setFillColor(sf::Color(110, 90, 70));
    window.draw(leftDivider);
    
    sf::RectangleShape leftDividerHighlight(sf::Vector2f(150, 1));
    leftDividerHighlight.setPosition(leftX, leftY + 31);
    leftDividerHighlight.setFillColor(sf::Color(70, 55, 45));
    window.draw(leftDividerHighlight);
    
    // 渲染宠物列表（左侧）
    renderPetSlots(window);
    
    // 右侧分隔线 - 像素风格
    sf::RectangleShape rightDivider(sf::Vector2f(3, panelSize.y - 70));
    rightDivider.setPosition(panelPos.x + 185, panelPos.y + 58);
    rightDivider.setFillColor(sf::Color(110, 90, 70));
    window.draw(rightDivider);
    
    sf::RectangleShape rightDividerHighlight(sf::Vector2f(1, panelSize.y - 70));
    rightDividerHighlight.setPosition(panelPos.x + 188, panelPos.y + 58);
    rightDividerHighlight.setFillColor(sf::Color(70, 55, 45));
    window.draw(rightDividerHighlight);
    
    // 渲染宠物详细信息（右侧）
    renderPetInfo(window);
}

void PetPanel::renderPetSlots(sf::RenderWindow& window) {
    if (!petManager) return;
    
    for (int i = 0; i < petManager->getMaxPetSlots(); i++) {
        sf::FloatRect slotRect = getSlotRect(i);
        
        // 槽位背景（圆形风格）
        sf::RectangleShape slot(sf::Vector2f(SLOT_SIZE, SLOT_SIZE));
        slot.setPosition(slotRect.left, slotRect.top);
        
        Pet* pet = petManager->getPetAt(i);
        
        if (pet) {
            slot.setFillColor(sf::Color(70, 60, 50));
            slot.setOutlineThickness(3);
            slot.setOutlineColor(Pet::getQualityColor(pet->getQuality()));
        } else {
            slot.setFillColor(sf::Color(40, 35, 30));
            slot.setOutlineThickness(2);
            slot.setOutlineColor(sf::Color(70, 60, 50));
        }
        
        // 悬浮高亮
        if (i == hoveredSlot) {
            slot.setFillColor(sf::Color(90, 80, 70));
        }
        
        // 当前选中宠物高亮
        if (i == petManager->getCurrentPetIndex()) {
            slot.setOutlineThickness(4);
            slot.setOutlineColor(sf::Color(255, 215, 0));  // 金色边框
        }
        
        window.draw(slot);
        
        if (pet) {
            // 绘制宠物精灵表第一帧作为图标
            sf::IntRect iconRect = pet->getIconRect();
            const sf::Texture& petTexture = pet->getTexture();
            
            if (petTexture.getSize().x > 0) {
                sf::Sprite petIcon;
                petIcon.setTexture(petTexture);
                petIcon.setTextureRect(iconRect);
                
                // 缩放图标以适应槽位
                float iconScale = (SLOT_SIZE - 16) / static_cast<float>(iconRect.width);
                petIcon.setScale(iconScale, iconScale);
                petIcon.setPosition(slotRect.left + 8, slotRect.top + 4);
                
                window.draw(petIcon);
            }
            
            if (fontLoaded) {
                // 等级（在图标下方）
                std::string lvStr = "Lv." + std::to_string(pet->getLevel());
                sf::Text levelText(lvStr, font, 10);
                levelText.setPosition(slotRect.left + 8, slotRect.top + SLOT_SIZE - 18);
                levelText.setFillColor(sf::Color(200, 200, 200));
                window.draw(levelText);
            }
        } else if (fontLoaded) {
            // 空槽位显示"空"
            std::string emptyStr = "空";
            sf::Text emptyText(toSfString(emptyStr), font, 14);
            emptyText.setPosition(slotRect.left + 25, slotRect.top + 25);
            emptyText.setFillColor(sf::Color(80, 80, 80));
            window.draw(emptyText);
        }
    }
}

void PetPanel::renderPetInfo(sf::RenderWindow& window) {
    if (!fontLoaded || !petManager) return;
    
    Pet* pet = petManager->getCurrentPet();
    float infoX = panelPos.x + 190;
    float infoY = panelPos.y + 55;
    float lineHeight = 28.0f;
    float infoWidth = panelSize.x - 210;
    
    // 右侧区域标题
    std::string infoTitleStr = "宠物信息";
    sf::Text infoTitle(toSfString(infoTitleStr), font, 16);
    infoTitle.setPosition(infoX, infoY);
    infoTitle.setFillColor(sf::Color(180, 160, 130));
    window.draw(infoTitle);
    
    sf::RectangleShape infoDivider(sf::Vector2f(infoWidth - 20, 2));
    infoDivider.setPosition(infoX, infoY + 25);
    infoDivider.setFillColor(sf::Color(100, 80, 60));
    window.draw(infoDivider);
    
    infoY += 35;
    
    if (!pet) {
        std::string noStr = "请选择一个宠物";
        sf::Text noSelect(toSfString(noStr), font, 16);
        noSelect.setPosition(infoX + 100, infoY + 80);
        noSelect.setFillColor(sf::Color(120, 120, 120));
        window.draw(noSelect);
        return;
    }
    
    // 宠物头像区域（简化为色块）
    sf::RectangleShape avatar(sf::Vector2f(70, 70));
    avatar.setPosition(infoX, infoY);
    avatar.setFillColor(sf::Color(60, 50, 45));
    avatar.setOutlineThickness(3);
    avatar.setOutlineColor(Pet::getQualityColor(pet->getQuality()));
    window.draw(avatar);
    
    // 头像内显示类型名
    std::string avatarStr = pet->getPetTypeName();
    sf::Text avatarText(toSfString(avatarStr), font, 16);
    avatarText.setPosition(infoX + 15, infoY + 25);
    avatarText.setFillColor(Pet::getQualityColor(pet->getQuality()));
    window.draw(avatarText);
    
    // 名称和资质（头像右边）
    float detailX = infoX + 85;
    std::string nameStr = pet->getName();
    sf::Text nameText(toSfString(nameStr), font, 18);
    nameText.setPosition(detailX, infoY);
    nameText.setFillColor(Pet::getQualityColor(pet->getQuality()));
    window.draw(nameText);
    
    std::string qualityStr = "资质: " + Pet::getQualityName(pet->getQuality());
    sf::Text qualityText(toSfString(qualityStr), font, 14);
    qualityText.setPosition(detailX, infoY + 25);
    qualityText.setFillColor(sf::Color(200, 180, 140));
    window.draw(qualityText);
    
    std::stringstream ss;
    ss << "等级: " << pet->getLevel() << "   经验: " << pet->getExp() << "/" << pet->getExpToNextLevel();
    sf::Text levelText(toSfString(ss.str()), font, 13);
    levelText.setPosition(detailX, infoY + 48);
    levelText.setFillColor(sf::Color::White);
    window.draw(levelText);
    
    infoY += 85;
    
    // 属性区域
    float attrX = infoX;
    float attrWidth = (infoWidth - 40) / 2;
    
    // 生命值
    ss.str("");
    ss << "生命: " << (int)pet->getHealth() << " / " << (int)pet->getMaxHealth();
    sf::Text hpText(toSfString(ss.str()), font, 14);
    hpText.setPosition(attrX, infoY);
    hpText.setFillColor(sf::Color(255, 120, 120));
    window.draw(hpText);
    
    // 攻击
    ss.str("");
    ss << "攻击: " << std::fixed << std::setprecision(1) << pet->getAttack();
    sf::Text atkText(toSfString(ss.str()), font, 14);
    atkText.setPosition(attrX + attrWidth, infoY);
    atkText.setFillColor(sf::Color(255, 200, 100));
    window.draw(atkText);
    
    infoY += lineHeight;
    
    // 防御
    ss.str("");
    ss << "防御: " << std::fixed << std::setprecision(1) << pet->getDefense();
    sf::Text defText(toSfString(ss.str()), font, 14);
    defText.setPosition(attrX, infoY);
    defText.setFillColor(sf::Color(100, 180, 255));
    window.draw(defText);
    
    // 闪避
    ss.str("");
    ss << "闪避: " << std::fixed << std::setprecision(1) << pet->getDodge();
    sf::Text dodgeText(toSfString(ss.str()), font, 14);
    dodgeText.setPosition(attrX + attrWidth, infoY);
    dodgeText.setFillColor(sf::Color(150, 255, 150));
    window.draw(dodgeText);
    
    infoY += lineHeight + 15;
    
    // 技能区域标题
    std::string skillTitleStr = "技能";
    sf::Text skillTitle(toSfString(skillTitleStr), font, 16);
    skillTitle.setPosition(infoX, infoY);
    skillTitle.setFillColor(sf::Color(180, 160, 130));
    window.draw(skillTitle);
    
    sf::RectangleShape skillDivider(sf::Vector2f(infoWidth - 20, 2));
    skillDivider.setPosition(infoX, infoY + 22);
    skillDivider.setFillColor(sf::Color(100, 80, 60));
    window.draw(skillDivider);
    
    infoY += 32;
    
    // 技能格子区域
    const auto& skills = pet->getSkills();
    float skillSlotSize = 56.0f;
    float skillGap = 10.0f;
    int maxSkillSlots = 4;  // 最多显示4个技能格子
    
    for (int i = 0; i < maxSkillSlots; i++) {
        float slotX = infoX + i * (skillSlotSize + skillGap);
        
        sf::RectangleShape skillSlot(sf::Vector2f(skillSlotSize, skillSlotSize));
        skillSlot.setPosition(slotX, infoY);
        skillSlot.setFillColor(sf::Color(50, 45, 40));
        skillSlot.setOutlineThickness(2);
        skillSlot.setOutlineColor(sf::Color(80, 70, 60));
        window.draw(skillSlot);
        
        if (i < (int)skills.size()) {
            // 有技能 - 显示技能图标（用颜色代替）
            sf::RectangleShape skillIcon(sf::Vector2f(skillSlotSize - 8, skillSlotSize - 8));
            skillIcon.setPosition(slotX + 4, infoY + 4);
            
            // 根据技能类型设置颜色
            if (skills[i].isPassive) {
                skillIcon.setFillColor(sf::Color(100, 150, 100));  // 被动技能绿色
            } else {
                skillIcon.setFillColor(sf::Color(150, 100, 100));  // 主动技能红色
            }
            window.draw(skillIcon);
            
            // 技能名（缩写）
            std::string skillNameShort = skills[i].name.substr(0, 2);
            sf::Text skillNameText(toSfString(skillNameShort), font, 14);
            skillNameText.setPosition(slotX + 12, infoY + 18);
            skillNameText.setFillColor(sf::Color::White);
            window.draw(skillNameText);
        } else {
            // 空技能槽 - 显示锁
            std::string lockStr = "空";
            sf::Text lockText(toSfString(lockStr), font, 14);
            lockText.setPosition(slotX + 18, infoY + 18);
            lockText.setFillColor(sf::Color(80, 80, 80));
            window.draw(lockText);
        }
    }
    
    infoY += skillSlotSize + 10;
    
    // 技能描述
    if (!skills.empty()) {
        for (size_t i = 0; i < skills.size() && i < 3; i++) {
            std::string skillDescStr = skills[i].name + ": " + skills[i].description;
            sf::Text skillDesc(toSfString(skillDescStr), font, 11);
            skillDesc.setPosition(infoX, infoY);
            skillDesc.setFillColor(sf::Color(160, 160, 160));
            window.draw(skillDesc);
            infoY += 18;
        }
    }
    
    // 洗点按钮（右下角）
    sf::FloatRect washRect = getWashButtonRect();
    sf::RectangleShape washBtn(sf::Vector2f(140, 36));
    washBtn.setPosition(washRect.left, washRect.top);
    washBtn.setFillColor(cleanserCount > 0 ? sf::Color(100, 80, 50) : sf::Color(60, 55, 50));
    washBtn.setOutlineThickness(2);
    washBtn.setOutlineColor(cleanserCount > 0 ? sf::Color(160, 130, 80) : sf::Color(80, 70, 60));
    window.draw(washBtn);
    
    std::string washStr = "重置资质化点";
    sf::Text washText(toSfString(washStr), font, 14);
    washText.setPosition(washRect.left + 18, washRect.top + 8);
    washText.setFillColor(cleanserCount > 0 ? sf::Color::White : sf::Color(100, 100, 100));
    window.draw(washText);
    
    // 洗涤剂数量
    std::string cleanserStr = "(洗涤剂: " + std::to_string(cleanserCount) + ")";
    sf::Text cleanserText(toSfString(cleanserStr), font, 11);
    cleanserText.setPosition(washRect.left + washRect.width + 10, washRect.top + 10);
    cleanserText.setFillColor(sf::Color(150, 150, 150));
    window.draw(cleanserText);
}

void PetPanel::toggle() { panelOpen = !panelOpen; }
void PetPanel::open() { panelOpen = true; }
void PetPanel::close() { panelOpen = false; }

sf::FloatRect PetPanel::getIconRect() const {
    return sf::FloatRect(iconPosition.x - 4, iconPosition.y - 4, ICON_SIZE + 8, ICON_SIZE + 8);
}

sf::FloatRect PetPanel::getSlotRect(int index) const {
    // 左侧竖排列表
    float x = panelPos.x + 20;
    float y = panelPos.y + 85 + index * (SLOT_SIZE + 8);
    return sf::FloatRect(x, y, SLOT_SIZE, SLOT_SIZE);
}

sf::FloatRect PetPanel::getWashButtonRect() const {
    return sf::FloatRect(panelPos.x + 190, panelPos.y + panelSize.y - 55, 140, 36);
}

sf::FloatRect PetPanel::getCloseButtonRect() const {
    return sf::FloatRect(panelPos.x + panelSize.x - 38, panelPos.y + 6, 28, 28);
}

// ============================================================================
// HatchPanel 实现 - 孵化栏功能图标
// ============================================================================

HatchPanel::HatchPanel()
    : petManager(nullptr)
    , fontLoaded(false)
    , iconLoaded(false)
    , panelOpen(false)
    , panelSize(500, 480)  // 增大面板尺寸
    , selectedHatchType(1)
    , selectedEnhancerCount(0)
    , enhancerCount(0)
{
}

bool HatchPanel::init(const std::string& iconPath) {
    if (iconTexture.loadFromFile(iconPath)) {
        iconSprite.setTexture(iconTexture);
        float scale = ICON_SIZE / iconTexture.getSize().x;
        iconSprite.setScale(scale, scale);
        iconLoaded = true;
    } else {
        sf::Image placeholder;
        placeholder.create(48, 48, sf::Color(150, 100, 100));
        iconTexture.loadFromImage(placeholder);
        iconSprite.setTexture(iconTexture);
        iconLoaded = false;
    }
    
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

bool HatchPanel::loadFont(const std::string& fontPath) {
    if (font.loadFromFile(fontPath)) {
        fontLoaded = true;
        std::cout << "[HatchPanel] Font loaded: " << fontPath << std::endl;
        return true;
    }
    return false;
}

void HatchPanel::setIconPosition(float x, float y) {
    iconPosition = sf::Vector2f(x, y);
    iconSprite.setPosition(iconPosition);
    panelPos = sf::Vector2f(x - panelSize.x / 2 + ICON_SIZE / 2, y - panelSize.y - 10);
    
    if (panelPos.x < 10) panelPos.x = 10;
}

void HatchPanel::setEssenceCount(int petTypeId, int count) {
    essenceCounts[petTypeId] = count;
    std::cout << "[HatchPanel] Set essence count for type " << petTypeId << " = " << count << std::endl;
}

void HatchPanel::update(float dt) {
    (void)dt;
}

void HatchPanel::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    sf::Vector2i mouseScreenPos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePos(static_cast<float>(mouseScreenPos.x), static_cast<float>(mouseScreenPos.y));
    
    if (event.type == sf::Event::MouseButtonPressed && 
        event.mouseButton.button == sf::Mouse::Left) {
        
        if (getIconRect().contains(mousePos)) {
            toggle();
            if (panelOpen) selectedEnhancerCount = 0;
            return;
        }
        
        if (panelOpen) {
            if (getCloseButtonRect().contains(mousePos)) {
                close();
                return;
            }
            
            // 强化剂加减
            if (getEnhancerMinusRect().contains(mousePos)) {
                if (selectedEnhancerCount > 0) selectedEnhancerCount--;
                return;
            }
            if (getEnhancerPlusRect().contains(mousePos)) {
                if (selectedEnhancerCount < enhancerCount && selectedEnhancerCount < 100) {
                    selectedEnhancerCount++;
                }
                return;
            }
            
            // 孵化按钮
            if (getHatchButtonRect().contains(mousePos)) {
                int essenceCount = 0;
                auto it = essenceCounts.find(selectedHatchType);
                if (it != essenceCounts.end()) essenceCount = it->second;
                
                std::cout << "[HatchPanel] Hatch clicked, essence=" << essenceCount 
                          << ", canHatch=" << (petManager ? petManager->canHatchNewPet() : false) << std::endl;
                
                if (essenceCount > 0 && petManager && petManager->canHatchNewPet() && onHatch) {
                    if (onHatch(selectedHatchType, selectedEnhancerCount)) {
                        // 成功孵化后不关闭面板，让用户看到结果
                    }
                }
                return;
            }
        }
    }
}

void HatchPanel::render(sf::RenderWindow& window) {
    renderIcon(window);
    if (panelOpen) renderPanel(window);
}

void HatchPanel::renderIcon(sf::RenderWindow& window) {
    // 像素风格的图标背景 - 多层边框
    float bgSize = ICON_SIZE + 12;
    
    // 外层阴影
    sf::RectangleShape shadow(sf::Vector2f(bgSize, bgSize));
    shadow.setPosition(iconPosition.x - 6 + 3, iconPosition.y - 6 + 3);
    shadow.setFillColor(sf::Color(20, 20, 20, 150));
    window.draw(shadow);
    
    // 主背景
    sf::RectangleShape iconBg(sf::Vector2f(bgSize, bgSize));
    iconBg.setPosition(iconPosition.x - 6, iconPosition.y - 6);
    iconBg.setFillColor(panelOpen ? sf::Color(100, 60, 60, 240) : sf::Color(35, 35, 45, 240));
    window.draw(iconBg);
    
    // 像素风格边框 - 外层
    sf::RectangleShape outerBorder(sf::Vector2f(bgSize, bgSize));
    outerBorder.setPosition(iconPosition.x - 6, iconPosition.y - 6);
    outerBorder.setFillColor(sf::Color::Transparent);
    outerBorder.setOutlineThickness(3);
    outerBorder.setOutlineColor(panelOpen ? sf::Color(180, 100, 100) : sf::Color(70, 70, 90));
    window.draw(outerBorder);
    
    // 内层高光边框
    sf::RectangleShape innerBorder(sf::Vector2f(bgSize - 6, bgSize - 6));
    innerBorder.setPosition(iconPosition.x - 3, iconPosition.y - 3);
    innerBorder.setFillColor(sf::Color::Transparent);
    innerBorder.setOutlineThickness(2);
    innerBorder.setOutlineColor(panelOpen ? sf::Color(220, 140, 140, 150) : sf::Color(100, 100, 120, 150));
    window.draw(innerBorder);
    
    window.draw(iconSprite);
    
    if (fontLoaded) {
        std::string labelStr = "孵化";
        sf::Text label(toSfString(labelStr), font, 14);
        // 文字阴影
        sf::Text labelShadow = label;
        labelShadow.setPosition(iconPosition.x + ICON_SIZE / 2 - 14 + 1, iconPosition.y + ICON_SIZE + 6 + 1);
        labelShadow.setFillColor(sf::Color(20, 20, 20, 200));
        window.draw(labelShadow);
        
        label.setPosition(iconPosition.x + ICON_SIZE / 2 - 14, iconPosition.y + ICON_SIZE + 6);
        label.setFillColor(sf::Color::White);
        window.draw(label);
    }
}

void HatchPanel::renderPanel(sf::RenderWindow& window) {
    // 主背景
    sf::RectangleShape background(panelSize);
    background.setPosition(panelPos);
    background.setFillColor(sf::Color(45, 40, 35, 245));
    background.setOutlineThickness(3);
    background.setOutlineColor(sf::Color(140, 100, 80));
    window.draw(background);
    
    // 标题栏
    sf::RectangleShape titleBar(sf::Vector2f(panelSize.x, 40));
    titleBar.setPosition(panelPos);
    titleBar.setFillColor(sf::Color(70, 50, 45));
    window.draw(titleBar);
    
    if (!fontLoaded) return;
    
    std::string titleStr = "宠物孵化";
    sf::Text title(toSfString(titleStr), font, 22);
    title.setPosition(panelPos.x + panelSize.x / 2 - 44, panelPos.y + 8);
    title.setFillColor(sf::Color(220, 180, 150));
    window.draw(title);
    
    // 关闭按钮
    sf::FloatRect closeRect = getCloseButtonRect();
    sf::RectangleShape closeBtn(sf::Vector2f(28, 28));
    closeBtn.setPosition(closeRect.left, closeRect.top);
    closeBtn.setFillColor(sf::Color(120, 50, 50));
    closeBtn.setOutlineThickness(1);
    closeBtn.setOutlineColor(sf::Color(180, 80, 80));
    window.draw(closeBtn);
    
    sf::Text closeText("X", font, 18);
    closeText.setPosition(closeRect.left + 8, closeRect.top + 3);
    closeText.setFillColor(sf::Color::White);
    window.draw(closeText);
    
    float x = panelPos.x + 25;
    float y = panelPos.y + 55;
    
    // 可孵化精元区域
    std::string essenceTitleStr = "可孵化精元";
    sf::Text essenceTitle(toSfString(essenceTitleStr), font, 16);
    essenceTitle.setPosition(x, y);
    essenceTitle.setFillColor(sf::Color(180, 160, 130));
    window.draw(essenceTitle);
    
    y += 30;
    
    // 精元列表（显示拥有的精元）
    int essenceCount = 0;
    auto it = essenceCounts.find(selectedHatchType);
    if (it != essenceCounts.end()) essenceCount = it->second;
    
    std::string petTypeName = petManager ? petManager->getPetTypeName(selectedHatchType) : "兔子";
    
    // 精元槽位
    sf::RectangleShape essenceSlot(sf::Vector2f(80, 80));
    essenceSlot.setPosition(x, y);
    essenceSlot.setFillColor(sf::Color(60, 55, 50));
    essenceSlot.setOutlineThickness(3);
    essenceSlot.setOutlineColor(essenceCount > 0 ? sf::Color(150, 120, 80) : sf::Color(80, 70, 60));
    window.draw(essenceSlot);
    
    // 精元图标（用颜色块代替）
    if (essenceCount > 0) {
        sf::RectangleShape essenceIcon(sf::Vector2f(60, 60));
        essenceIcon.setPosition(x + 10, y + 10);
        essenceIcon.setFillColor(sf::Color(200, 150, 200));  // 紫色代表精元
        window.draw(essenceIcon);
    }
    
    // 精元名称和数量
    sf::Text essenceNameText(toSfString(petTypeName + "精元"), font, 14);
    essenceNameText.setPosition(x + 90, y + 20);
    essenceNameText.setFillColor(sf::Color::White);
    window.draw(essenceNameText);
    
    std::string countStr = "数量: " + std::to_string(essenceCount);
    sf::Text countText(toSfString(countStr), font, 14);
    countText.setPosition(x + 90, y + 45);
    countText.setFillColor(essenceCount > 0 ? sf::Color(100, 255, 100) : sf::Color(255, 100, 100));
    window.draw(countText);
    
    y += 100;
    
    // 强化剂区域 - 显示特定精元对应的强化剂
    std::string enhancerMaterialName = "兔毛";  // 根据selectedHatchType决定
    if (selectedHatchType == 1) {
        enhancerMaterialName = "兔毛";
    }
    // 未来可以添加更多类型
    // else if (selectedHatchType == 2) {
    //     enhancerMaterialName = "粘液";
    // }
    
    std::string enhancerTitleStr = "使用强化剂: " + enhancerMaterialName + " (提升稀有资质)";
    sf::Text enhancerTitle(toSfString(enhancerTitleStr), font, 14);
    enhancerTitle.setPosition(x, y);
    enhancerTitle.setFillColor(sf::Color(180, 160, 130));
    window.draw(enhancerTitle);
    
    y += 28;
    
    std::string enhancerStr = enhancerMaterialName + ": " + std::to_string(selectedEnhancerCount) + " / " + std::to_string(enhancerCount);
    sf::Text enhancerText(toSfString(enhancerStr), font, 16);
    enhancerText.setPosition(x, y);
    enhancerText.setFillColor(sf::Color::White);
    window.draw(enhancerText);
    
    // 加减按钮
    sf::FloatRect minusRect = getEnhancerMinusRect();
    sf::RectangleShape minusBtn(sf::Vector2f(36, 36));
    minusBtn.setPosition(minusRect.left, minusRect.top);
    minusBtn.setFillColor(sf::Color(80, 60, 55));
    minusBtn.setOutlineThickness(2);
    minusBtn.setOutlineColor(sf::Color(120, 100, 90));
    window.draw(minusBtn);
    
    sf::Text minusText("-", font, 24);
    minusText.setPosition(minusRect.left + 12, minusRect.top + 2);
    minusText.setFillColor(sf::Color::White);
    window.draw(minusText);
    
    sf::FloatRect plusRect = getEnhancerPlusRect();
    sf::RectangleShape plusBtn(sf::Vector2f(36, 36));
    plusBtn.setPosition(plusRect.left, plusRect.top);
    plusBtn.setFillColor(sf::Color(55, 80, 55));
    plusBtn.setOutlineThickness(2);
    plusBtn.setOutlineColor(sf::Color(90, 120, 90));
    window.draw(plusBtn);
    
    sf::Text plusText("+", font, 24);
    plusText.setPosition(plusRect.left + 10, plusRect.top + 2);
    plusText.setFillColor(sf::Color::White);
    window.draw(plusText);
    
    y += 50;
    
    // 概率预览标题
    std::string probTitleStr = "资质概率预览:";
    sf::Text probTitle(toSfString(probTitleStr), font, 14);
    probTitle.setPosition(x, y);
    probTitle.setFillColor(sf::Color(180, 180, 180));
    window.draw(probTitle);
    y += 25;
    
    // 概率条
    renderProbabilityBar(window, x, y, panelSize.x - 50, 28);
    y += 40;
    
    // 图例
    float legendX = x;
    std::vector<std::pair<std::string, PetQuality>> legends = {
        {"平庸", PetQuality::Mediocre},
        {"良好", PetQuality::Good},
        {"优秀", PetQuality::Excellent},
        {"卓越", PetQuality::Outstanding},
        {"稀有", PetQuality::Rare}
    };
    
    for (const auto& leg : legends) {
        sf::RectangleShape colorBox(sf::Vector2f(16, 16));
        colorBox.setPosition(legendX, y);
        colorBox.setFillColor(Pet::getQualityColor(leg.second));
        window.draw(colorBox);
        
        sf::Text legText(toSfString(leg.first), font, 13);
        legText.setPosition(legendX + 20, y - 2);
        legText.setFillColor(sf::Color(180, 180, 180));
        window.draw(legText);
        
        legendX += 78;
    }
    
    // 孵化按钮
    sf::FloatRect hatchRect = getHatchButtonRect();
    bool canHatch = essenceCount > 0 && petManager && petManager->canHatchNewPet();
    
    sf::RectangleShape hatchBtn(sf::Vector2f(160, 48));
    hatchBtn.setPosition(hatchRect.left, hatchRect.top);
    hatchBtn.setFillColor(canHatch ? sf::Color(60, 100, 60) : sf::Color(60, 55, 50));
    hatchBtn.setOutlineThickness(3);
    hatchBtn.setOutlineColor(canHatch ? sf::Color(100, 160, 100) : sf::Color(80, 70, 60));
    window.draw(hatchBtn);
    
    std::string hatchStr = "确认孵化";
    sf::Text hatchText(toSfString(hatchStr), font, 20);
    hatchText.setPosition(hatchRect.left + 30, hatchRect.top + 12);
    hatchText.setFillColor(canHatch ? sf::Color::White : sf::Color(100, 100, 100));
    window.draw(hatchText);
    
    // 提示文字
    if (!canHatch) {
        std::string tipStr = "";
        if (essenceCount <= 0) {
            tipStr = "需要精元才能孵化!";
        } else if (petManager && !petManager->canHatchNewPet()) {
            tipStr = "宠物栏已满!";
        }
        
        if (!tipStr.empty()) {
            sf::Text tipText(toSfString(tipStr), font, 13);
            tipText.setPosition(hatchRect.left, hatchRect.top + 55);
            tipText.setFillColor(sf::Color(255, 150, 100));
            window.draw(tipText);
        }
    }
}

void HatchPanel::renderProbabilityBar(sf::RenderWindow& window, float x, float y, float width, float height) {
    float mediocre, good, excellent, outstanding, rare;
    Pet::getEnhancerProbabilities(selectedEnhancerCount, mediocre, good, excellent, outstanding, rare);
    
    float currentX = x;
    
    auto drawSection = [&](float prob, PetQuality q) {
        float w = width * prob;
        if (w > 0) {
            sf::RectangleShape bar(sf::Vector2f(w, height));
            bar.setPosition(currentX, y);
            bar.setFillColor(Pet::getQualityColor(q));
            window.draw(bar);
            currentX += w;
        }
    };
    
    drawSection(mediocre, PetQuality::Mediocre);
    drawSection(good, PetQuality::Good);
    drawSection(excellent, PetQuality::Excellent);
    drawSection(outstanding, PetQuality::Outstanding);
    drawSection(rare, PetQuality::Rare);
    
    sf::RectangleShape border(sf::Vector2f(width, height));
    border.setPosition(x, y);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineThickness(2);
    border.setOutlineColor(sf::Color(100, 90, 80));
    window.draw(border);
}

void HatchPanel::toggle() { panelOpen = !panelOpen; }
void HatchPanel::open() { panelOpen = true; }
void HatchPanel::close() { panelOpen = false; }

sf::FloatRect HatchPanel::getIconRect() const {
    return sf::FloatRect(iconPosition.x - 4, iconPosition.y - 4, ICON_SIZE + 8, ICON_SIZE + 8);
}

sf::FloatRect HatchPanel::getHatchButtonRect() const {
    return sf::FloatRect(panelPos.x + panelSize.x / 2 - 80, panelPos.y + panelSize.y - 75, 160, 48);
}

sf::FloatRect HatchPanel::getCloseButtonRect() const {
    return sf::FloatRect(panelPos.x + panelSize.x - 38, panelPos.y + 6, 28, 28);
}

sf::FloatRect HatchPanel::getEnhancerMinusRect() const {
    return sf::FloatRect(panelPos.x + 250, panelPos.y + 180, 36, 36);
}

sf::FloatRect HatchPanel::getEnhancerPlusRect() const {
    return sf::FloatRect(panelPos.x + 295, panelPos.y + 180, 36, 36);
}
