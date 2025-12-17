#include "Equipment.h"
#include <iostream>
#include <algorithm>

// ============================================================================
// 颜色常量
// ============================================================================

const sf::Color EquipmentPanel::BG_COLOR(30, 30, 40, 240);
const sf::Color EquipmentPanel::SLOT_COLOR(50, 50, 60, 200);
const sf::Color EquipmentPanel::SLOT_HOVER_COLOR(70, 70, 90, 220);
const sf::Color EquipmentPanel::SLOT_EQUIPPED_COLOR(60, 80, 60, 220);
const sf::Color EquipmentPanel::BORDER_COLOR(139, 90, 43);

// ============================================================================
// EquipmentManager 实现
// ============================================================================

EquipmentManager& EquipmentManager::getInstance() {
    static EquipmentManager instance;
    return instance;
}

void EquipmentManager::initialize() {
    if (initialized) return;
    
    std::cout << "[EquipmentManager] Initializing equipment definitions..." << std::endl;
    
    // ========================================
    // 武器 - 斧头系列（无视防御）
    // ========================================
    {
        EquipmentData axe;
        axe.id = "wooden_axe";
        axe.name = "木斧";
        axe.description = "简单的木制斧头，可以无视目标的防御";
        axe.slot = EquipmentSlot::Weapon;
        axe.weaponType = WeaponType::Axe;
        axe.rarity = ItemRarity::Common;
        axe.requiredLevel = 1;
        axe.stats.attack = 8;
        axe.stats.ignoreDefense = true;
        axe.stats.ignoreDefenseRate = 1.0f;  // 完全无视防御
        axe.texturePath = "assets/equipment/wooden_axe.png";
        registerEquipment(axe);
    }
    
    {
        EquipmentData axe;
        axe.id = "iron_axe";
        axe.name = "铁斧";
        axe.description = "坚固的铁制斧头，可以无视目标的防御";
        axe.slot = EquipmentSlot::Weapon;
        axe.weaponType = WeaponType::Axe;
        axe.rarity = ItemRarity::Uncommon;
        axe.requiredLevel = 10;
        axe.requiredStrength = 15;
        axe.stats.attack = 18;
        axe.stats.strength = 2;
        axe.stats.ignoreDefense = true;
        axe.stats.ignoreDefenseRate = 1.0f;
        axe.texturePath = "assets/equipment/iron_axe.png";
        registerEquipment(axe);
    }
    
    // ========================================
    // 武器 - 剑系列
    // ========================================
    {
        EquipmentData sword;
        sword.id = "wooden_sword";
        sword.name = "木剑";
        sword.description = "新手战士的第一把武器";
        sword.slot = EquipmentSlot::Weapon;
        sword.weaponType = WeaponType::Sword;
        sword.rarity = ItemRarity::Common;
        sword.requiredLevel = 1;
        sword.stats.attack = 5;
        sword.texturePath = "assets/equipment/wooden_sword.png";
        registerEquipment(sword);
    }
    
    {
        EquipmentData sword;
        sword.id = "iron_sword";
        sword.name = "铁剑";
        sword.description = "标准的铁制长剑";
        sword.slot = EquipmentSlot::Weapon;
        sword.weaponType = WeaponType::Sword;
        sword.rarity = ItemRarity::Uncommon;
        sword.requiredLevel = 10;
        sword.requiredStrength = 10;
        sword.stats.attack = 15;
        sword.stats.strength = 1;
        sword.texturePath = "assets/equipment/iron_sword.png";
        registerEquipment(sword);
    }
    
    // ========================================
    // 防具 - 头盔
    // ========================================
    {
        EquipmentData helmet;
        helmet.id = "leather_cap";
        helmet.name = "皮帽";
        helmet.description = "简单的皮制帽子";
        helmet.slot = EquipmentSlot::Helmet;
        helmet.rarity = ItemRarity::Common;
        helmet.requiredLevel = 1;
        helmet.stats.defense = 2;
        helmet.stats.hp = 10;
        helmet.texturePath = "assets/equipment/leather_cap.png";
        registerEquipment(helmet);
    }
    
    {
        EquipmentData helmet;
        helmet.id = "iron_helmet";
        helmet.name = "铁盔";
        helmet.description = "坚固的铁制头盔";
        helmet.slot = EquipmentSlot::Helmet;
        helmet.rarity = ItemRarity::Uncommon;
        helmet.requiredLevel = 10;
        helmet.requiredStrength = 5;
        helmet.stats.defense = 8;
        helmet.stats.hp = 30;
        helmet.stats.strength = 1;
        helmet.texturePath = "assets/equipment/iron_helmet.png";
        registerEquipment(helmet);
    }
    
    // ========================================
    // 防具 - 铠甲
    // ========================================
    {
        EquipmentData armor;
        armor.id = "leather_armor";
        armor.name = "皮甲";
        armor.description = "轻便的皮制护甲";
        armor.slot = EquipmentSlot::Armor;
        armor.rarity = ItemRarity::Common;
        armor.requiredLevel = 1;
        armor.stats.defense = 5;
        armor.stats.hp = 20;
        armor.texturePath = "assets/equipment/leather_armor.png";
        registerEquipment(armor);
    }
    
    {
        EquipmentData armor;
        armor.id = "iron_armor";
        armor.name = "铁甲";
        armor.description = "厚重的铁制铠甲";
        armor.slot = EquipmentSlot::Armor;
        armor.rarity = ItemRarity::Uncommon;
        armor.requiredLevel = 10;
        armor.requiredStrength = 20;
        armor.stats.defense = 15;
        armor.stats.hp = 50;
        armor.stats.strength = 2;
        armor.texturePath = "assets/equipment/iron_armor.png";
        registerEquipment(armor);
    }
    
    // ========================================
    // 防具 - 裤子
    // ========================================
    {
        EquipmentData pants;
        pants.id = "leather_pants";
        pants.name = "皮裤";
        pants.description = "简单的皮制裤子";
        pants.slot = EquipmentSlot::Pants;
        pants.rarity = ItemRarity::Common;
        pants.requiredLevel = 1;
        pants.stats.defense = 3;
        pants.stats.hp = 10;
        pants.texturePath = "assets/equipment/leather_pants.png";
        registerEquipment(pants);
    }
    
    // ========================================
    // 防具 - 手套
    // ========================================
    {
        EquipmentData gloves;
        gloves.id = "leather_gloves";
        gloves.name = "皮手套";
        gloves.description = "简单的皮制手套";
        gloves.slot = EquipmentSlot::Gloves;
        gloves.rarity = ItemRarity::Common;
        gloves.requiredLevel = 1;
        gloves.stats.defense = 1;
        gloves.stats.attack = 1;
        gloves.texturePath = "assets/equipment/leather_gloves.png";
        registerEquipment(gloves);
    }
    
    {
        EquipmentData gloves;
        gloves.id = "warrior_gloves";
        gloves.name = "战士手套";
        gloves.description = "战士专用的强化手套";
        gloves.slot = EquipmentSlot::Gloves;
        gloves.rarity = ItemRarity::Uncommon;
        gloves.requiredLevel = 10;
        gloves.requiredStrength = 8;
        gloves.stats.defense = 3;
        gloves.stats.attack = 3;
        gloves.stats.strength = 1;
        gloves.texturePath = "assets/equipment/warrior_gloves.png";
        registerEquipment(gloves);
    }
    
    // ========================================
    // 防具 - 鞋子
    // ========================================
    {
        EquipmentData boots;
        boots.id = "leather_boots";
        boots.name = "皮靴";
        boots.description = "简单的皮制靴子";
        boots.slot = EquipmentSlot::Boots;
        boots.rarity = ItemRarity::Common;
        boots.requiredLevel = 1;
        boots.stats.defense = 2;
        boots.stats.speed = 5;
        boots.texturePath = "assets/equipment/leather_boots.png";
        registerEquipment(boots);
    }
    
    // ========================================
    // 饰品 - 披风
    // ========================================
    {
        EquipmentData cape;
        cape.id = "simple_cape";
        cape.name = "简易披风";
        cape.description = "一件普通的披风";
        cape.slot = EquipmentSlot::Cape;
        cape.rarity = ItemRarity::Common;
        cape.requiredLevel = 1;
        cape.stats.defense = 1;
        cape.stats.hp = 5;
        cape.texturePath = "assets/equipment/simple_cape.png";
        registerEquipment(cape);
    }
    
    // ========================================
    // 饰品 - 副手盾牌
    // ========================================
    {
        EquipmentData shield;
        shield.id = "wooden_shield";
        shield.name = "木盾";
        shield.description = "简单的木制盾牌";
        shield.slot = EquipmentSlot::SecondHand;
        shield.rarity = ItemRarity::Common;
        shield.requiredLevel = 1;
        shield.stats.defense = 5;
        shield.texturePath = "assets/equipment/wooden_shield.png";
        registerEquipment(shield);
    }
    
    {
        EquipmentData shield;
        shield.id = "iron_shield";
        shield.name = "铁盾";
        shield.description = "坚固的铁制盾牌";
        shield.slot = EquipmentSlot::SecondHand;
        shield.rarity = ItemRarity::Uncommon;
        shield.requiredLevel = 10;
        shield.requiredStrength = 15;
        shield.stats.defense = 12;
        shield.stats.hp = 20;
        shield.texturePath = "assets/equipment/iron_shield.png";
        registerEquipment(shield);
    }
    
    initialized = true;
    std::cout << "[EquipmentManager] Registered " << equipments.size() << " equipments" << std::endl;
}

const EquipmentData* EquipmentManager::getEquipmentData(const std::string& equipId) const {
    auto it = equipments.find(equipId);
    if (it != equipments.end()) {
        return &it->second;
    }
    return nullptr;
}

void EquipmentManager::registerEquipment(const EquipmentData& data) {
    equipments[data.id] = data;
    std::cout << "[EquipmentManager] Registered: " << data.id << " (" << data.name << ")" << std::endl;
}

std::string EquipmentManager::getSlotName(EquipmentSlot slot) {
    switch (slot) {
        case EquipmentSlot::Weapon:     return "武器";
        case EquipmentSlot::SecondHand: return "副手";
        case EquipmentSlot::Helmet:     return "头盔";
        case EquipmentSlot::Armor:      return "铠甲";
        case EquipmentSlot::Pants:      return "裤子";
        case EquipmentSlot::Gloves:     return "手套";
        case EquipmentSlot::Boots:      return "鞋子";
        case EquipmentSlot::Cape:       return "披风";
        case EquipmentSlot::Necklace:   return "项链";
        case EquipmentSlot::Ring1:      return "戒指";
        case EquipmentSlot::Ring2:      return "戒指";
        case EquipmentSlot::Earring:    return "耳环";
        case EquipmentSlot::Belt:       return "腰带";
        case EquipmentSlot::Shoulder:   return "肩甲";
        default:                        return "未知";
    }
}

std::string EquipmentManager::getWeaponTypeName(WeaponType type) {
    switch (type) {
        case WeaponType::Sword:       return "单手剑";
        case WeaponType::TwoHandSword:return "双手剑";
        case WeaponType::Axe:         return "单手斧";
        case WeaponType::TwoHandAxe:  return "双手斧";
        case WeaponType::Mace:        return "单手锤";
        case WeaponType::TwoHandMace: return "双手锤";
        case WeaponType::Spear:       return "枪";
        case WeaponType::Polearm:     return "矛";
        default:                      return "无";
    }
}

// ============================================================================
// PlayerEquipment 实现
// ============================================================================

PlayerEquipment::PlayerEquipment() {
    for (auto& item : equippedItems) {
        item.clear();
    }
}

std::string PlayerEquipment::equip(const std::string& equipId) {
    const EquipmentData* data = EquipmentManager::getInstance().getEquipmentData(equipId);
    if (!data) {
        std::cout << "[PlayerEquipment] Unknown equipment: " << equipId << std::endl;
        return "";
    }
    
    size_t slotIndex = static_cast<size_t>(data->slot);
    std::string replaced = equippedItems[slotIndex];
    
    equippedItems[slotIndex] = equipId;
    
    if (onEquip) {
        onEquip(data->slot, equipId);
    }
    
    std::cout << "[PlayerEquipment] Equipped: " << data->name << " -> " 
              << EquipmentManager::getSlotName(data->slot) << std::endl;
    
    return replaced;
}

ItemStack PlayerEquipment::equip(const EquipmentData& equipData) {
    size_t slotIndex = static_cast<size_t>(equipData.slot);
    std::string replaced = equippedItems[slotIndex];
    
    equippedItems[slotIndex] = equipData.id;
    
    if (onEquip) {
        onEquip(equipData.slot, equipData.id);
    }
    
    std::cout << "[PlayerEquipment] Equipped: " << equipData.name << " -> " 
              << EquipmentManager::getSlotName(equipData.slot) << std::endl;
    
    // 返回被替换的装备作为ItemStack
    if (!replaced.empty()) {
        return ItemStack(replaced, 1);
    }
    return ItemStack();
}

std::string PlayerEquipment::unequip(EquipmentSlot slot) {
    size_t slotIndex = static_cast<size_t>(slot);
    std::string removed = equippedItems[slotIndex];
    
    if (!removed.empty()) {
        equippedItems[slotIndex].clear();
        
        if (onUnequip) {
            onUnequip(slot, removed);
        }
        
        const EquipmentData* data = EquipmentManager::getInstance().getEquipmentData(removed);
        std::cout << "[PlayerEquipment] Unequipped: " 
                  << (data ? data->name : removed) << std::endl;
    }
    
    return removed;
}

ItemStack PlayerEquipment::unequipToStack(EquipmentSlot slot) {
    std::string removed = unequip(slot);
    if (!removed.empty()) {
        return ItemStack(removed, 1);
    }
    return ItemStack();
}

const std::string& PlayerEquipment::getEquippedItem(EquipmentSlot slot) const {
    static std::string empty;
    size_t slotIndex = static_cast<size_t>(slot);
    if (slotIndex < equippedItems.size()) {
        return equippedItems[slotIndex];
    }
    return empty;
}

bool PlayerEquipment::hasEquipment(EquipmentSlot slot) const {
    return !getEquippedItem(slot).empty();
}

EquipmentStats PlayerEquipment::getTotalStats() const {
    EquipmentStats total;
    
    for (size_t i = 0; i < equippedItems.size(); i++) {
        if (!equippedItems[i].empty()) {
            const EquipmentData* data = EquipmentManager::getInstance().getEquipmentData(equippedItems[i]);
            if (data) {
                total = total + data->stats;
            }
        }
    }
    
    return total;
}

bool PlayerEquipment::hasIgnoreDefense() const {
    return getTotalStats().ignoreDefense;
}

float PlayerEquipment::getIgnoreDefenseRate() const {
    return getTotalStats().ignoreDefenseRate;
}

WeaponType PlayerEquipment::getCurrentWeaponType() const {
    const std::string& weaponId = getEquippedItem(EquipmentSlot::Weapon);
    if (weaponId.empty()) return WeaponType::None;
    
    const EquipmentData* data = EquipmentManager::getInstance().getEquipmentData(weaponId);
    return data ? data->weaponType : WeaponType::None;
}

// ============================================================================
// EquipmentPanel 实现
// ============================================================================

EquipmentPanel::EquipmentPanel()
    : equipment(nullptr)
    , onUnequipCallback(nullptr)
    , panelOpen(false)
    , hoveredSlot(EquipmentSlot::Count)
    , selectedSlot(EquipmentSlot::Count)
    , iconLoaded(false)
    , fontLoaded(false)
    , characterLoaded(false)
{
    panelSize = sf::Vector2f(350, 450);
}

bool EquipmentPanel::init(const std::string& iconPath) {
    if (iconTexture.loadFromFile(iconPath)) {
        iconSprite.setTexture(iconTexture);
        iconSprite.setScale(0.8f, 0.8f);
        iconLoaded = true;
        std::cout << "[EquipmentPanel] Icon loaded: " << iconPath << std::endl;
    } else {
        sf::Image placeholder;
        placeholder.create(64, 64, sf::Color(80, 60, 100, 200));
        iconTexture.loadFromImage(placeholder);
        iconSprite.setTexture(iconTexture);
        iconLoaded = true;
        std::cout << "[EquipmentPanel] Using placeholder icon" << std::endl;
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

bool EquipmentPanel::loadFont(const std::string& fontPath) {
    if (font.loadFromFile(fontPath)) {
        fontLoaded = true;
        std::cout << "[EquipmentPanel] Font loaded: " << fontPath << std::endl;
        return true;
    }
    return false;
}

void EquipmentPanel::setIconPosition(float x, float y) {
    iconPosition = sf::Vector2f(x, y);
    iconSprite.setPosition(iconPosition);
}

void EquipmentPanel::update(float /*dt*/) {
    // 动画更新
}

void EquipmentPanel::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    
    // 检查图标点击
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        if (!panelOpen && iconSprite.getGlobalBounds().contains(mousePosF)) {
            open();
            return;
        }
    }
    
    // 键盘快捷键
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::E) {
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
            hoveredSlot = getSlotAtPosition(mousePosF);
        } else {
            hoveredSlot = EquipmentSlot::Count;
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
            
            // 点击槽位
            EquipmentSlot clickedSlot = getSlotAtPosition(mousePosF);
            if (clickedSlot != EquipmentSlot::Count) {
                selectedSlot = clickedSlot;
            }
        }
        else if (event.mouseButton.button == sf::Mouse::Right) {
            // 右键卸下装备
            EquipmentSlot clickedSlot = getSlotAtPosition(mousePosF);
            if (clickedSlot != EquipmentSlot::Count && equipment && 
                equipment->hasEquipment(clickedSlot)) {
                if (onUnequipCallback) {
                    onUnequipCallback(clickedSlot);
                } else {
                    equipment->unequip(clickedSlot);
                }
            }
        }
    }
}

void EquipmentPanel::render(sf::RenderWindow& window) {
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
    
    if (!panelOpen || !equipment) return;
    
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
    
    // 标题栏
    sf::RectangleShape titleBar(sf::Vector2f(panelSize.x - 6, 30));
    titleBar.setPosition(panelPosition.x + 3, panelPosition.y + 3);
    titleBar.setFillColor(sf::Color(60, 45, 30, 200));
    window.draw(titleBar);
    
    if (fontLoaded) {
        sf::Text title;
        title.setFont(font);
        std::string titleStr = "装备栏";
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
    
    // 绘制角色轮廓（简化版）
    float centerX = panelPosition.x + panelSize.x / 2;
    float centerY = panelPosition.y + 200;
    
    // 角色身体轮廓
    sf::CircleShape head(25);
    head.setFillColor(sf::Color(80, 80, 90, 150));
    head.setOutlineThickness(2);
    head.setOutlineColor(sf::Color(100, 100, 110));
    head.setPosition(centerX - 25, centerY - 80);
    window.draw(head);
    
    sf::RectangleShape body(sf::Vector2f(60, 80));
    body.setFillColor(sf::Color(80, 80, 90, 150));
    body.setOutlineThickness(2);
    body.setOutlineColor(sf::Color(100, 100, 110));
    body.setPosition(centerX - 30, centerY - 30);
    window.draw(body);
    
    // 槽位尺寸
    sf::Vector2f slotSize(50, 50);
    
    // 绘制各装备槽位
    // 头盔 - 头顶
    renderSlot(window, EquipmentSlot::Helmet, 
               sf::Vector2f(centerX - 25, centerY - 140), slotSize);
    
    // 武器 - 左侧
    renderSlot(window, EquipmentSlot::Weapon, 
               sf::Vector2f(centerX - 100, centerY - 40), slotSize);
    
    // 副手 - 右侧
    renderSlot(window, EquipmentSlot::SecondHand, 
               sf::Vector2f(centerX + 50, centerY - 40), slotSize);
    
    // 铠甲 - 身体
    renderSlot(window, EquipmentSlot::Armor, 
               sf::Vector2f(centerX - 25, centerY - 20), slotSize);
    
    // 手套 - 两侧偏下
    renderSlot(window, EquipmentSlot::Gloves, 
               sf::Vector2f(centerX - 100, centerY + 40), slotSize);
    
    // 披风 - 右侧偏上
    renderSlot(window, EquipmentSlot::Cape, 
               sf::Vector2f(centerX + 50, centerY + 40), slotSize);
    
    // 裤子 - 身体下方
    renderSlot(window, EquipmentSlot::Pants, 
               sf::Vector2f(centerX - 25, centerY + 60), slotSize);
    
    // 鞋子 - 最底部
    renderSlot(window, EquipmentSlot::Boots, 
               sf::Vector2f(centerX - 25, centerY + 120), slotSize);
    
    // 饰品区域（右侧）
    float accessoryX = panelPosition.x + panelSize.x - 70;
    float accessoryY = panelPosition.y + 60;
    
    renderSlot(window, EquipmentSlot::Necklace, 
               sf::Vector2f(accessoryX, accessoryY), sf::Vector2f(40, 40));
    renderSlot(window, EquipmentSlot::Earring, 
               sf::Vector2f(accessoryX, accessoryY + 50), sf::Vector2f(40, 40));
    renderSlot(window, EquipmentSlot::Ring1, 
               sf::Vector2f(accessoryX, accessoryY + 100), sf::Vector2f(40, 40));
    renderSlot(window, EquipmentSlot::Ring2, 
               sf::Vector2f(accessoryX, accessoryY + 150), sf::Vector2f(40, 40));
    renderSlot(window, EquipmentSlot::Belt, 
               sf::Vector2f(accessoryX, accessoryY + 200), sf::Vector2f(40, 40));
    renderSlot(window, EquipmentSlot::Shoulder, 
               sf::Vector2f(accessoryX, accessoryY + 250), sf::Vector2f(40, 40));
    
    // 显示总属性
    if (fontLoaded) {
        EquipmentStats stats = equipment->getTotalStats();
        
        float statsX = panelPosition.x + 15;
        float statsY = panelPosition.y + panelSize.y - 80;
        
        sf::Text statsTitle;
        statsTitle.setFont(font);
        std::string statsTitleStr = "属性加成";
        statsTitle.setString(sf::String::fromUtf8(statsTitleStr.begin(), statsTitleStr.end()));
        statsTitle.setCharacterSize(14);
        statsTitle.setFillColor(sf::Color(200, 200, 200));
        statsTitle.setPosition(statsX, statsY);
        window.draw(statsTitle);
        
        std::stringstream ss;
        ss << "ATK+" << stats.attack << "  DEF+" << stats.defense 
           << "  HP+" << stats.hp;
        
        sf::Text statsText;
        statsText.setFont(font);
        statsText.setString(ss.str());
        statsText.setCharacterSize(12);
        statsText.setFillColor(sf::Color(150, 255, 150));
        statsText.setPosition(statsX, statsY + 20);
        window.draw(statsText);
        
        if (stats.ignoreDefense) {
            std::string ignoreStr = "特效: 无视防御";
            sf::Text ignoreText;
            ignoreText.setFont(font);
            ignoreText.setString(sf::String::fromUtf8(ignoreStr.begin(), ignoreStr.end()));
            ignoreText.setCharacterSize(12);
            ignoreText.setFillColor(sf::Color(255, 200, 100));
            ignoreText.setPosition(statsX, statsY + 40);
            window.draw(ignoreText);
        }
    }
    
    // 渲染提示框
    renderTooltip(window);
}

void EquipmentPanel::renderSlot(sf::RenderWindow& window, EquipmentSlot slot, 
                                const sf::Vector2f& pos, const sf::Vector2f& size) {
    bool isHovered = (hoveredSlot == slot);
    bool hasEquip = equipment && equipment->hasEquipment(slot);
    
    sf::RectangleShape slotBg(size);
    slotBg.setPosition(pos);
    
    if (isHovered) {
        slotBg.setFillColor(SLOT_HOVER_COLOR);
    } else if (hasEquip) {
        slotBg.setFillColor(SLOT_EQUIPPED_COLOR);
    } else {
        slotBg.setFillColor(SLOT_COLOR);
    }
    
    slotBg.setOutlineThickness(2);
    slotBg.setOutlineColor(isHovered ? sf::Color::White : sf::Color(80, 80, 80));
    window.draw(slotBg);
    
    // 绘制槽位名称
    if (fontLoaded && !hasEquip) {
        std::string slotName = EquipmentManager::getSlotName(slot);
        sf::Text nameText;
        nameText.setFont(font);
        nameText.setString(sf::String::fromUtf8(slotName.begin(), slotName.end()));
        nameText.setCharacterSize(10);
        nameText.setFillColor(sf::Color(120, 120, 120));
        
        sf::FloatRect bounds = nameText.getLocalBounds();
        nameText.setPosition(
            pos.x + (size.x - bounds.width) / 2,
            pos.y + (size.y - bounds.height) / 2 - 2
        );
        window.draw(nameText);
    }
    
    // TODO: 绘制装备图标
}

void EquipmentPanel::renderTooltip(sf::RenderWindow& window) {
    if (hoveredSlot == EquipmentSlot::Count || !equipment || !fontLoaded) return;
    
    const std::string& equipId = equipment->getEquippedItem(hoveredSlot);
    if (equipId.empty()) return;
    
    const EquipmentData* data = EquipmentManager::getInstance().getEquipmentData(equipId);
    if (!data) return;
    
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    float tooltipX = mousePos.x + 15.0f;
    float tooltipY = mousePos.y + 15.0f;
    
    // 构建提示内容
    std::vector<std::pair<sf::String, sf::Color>> lines;
    
    lines.push_back({sf::String::fromUtf8(data->name.begin(), data->name.end()), 
                     ItemDatabase::getRarityColor(data->rarity)});
    
    std::string slotName = EquipmentManager::getSlotName(data->slot);
    lines.push_back({sf::String::fromUtf8(slotName.begin(), slotName.end()), 
                     sf::Color(150, 150, 150)});
    
    if (!data->description.empty()) {
        lines.push_back({"", sf::Color::White});
        lines.push_back({sf::String::fromUtf8(data->description.begin(), data->description.end()), 
                        sf::Color(200, 200, 200)});
    }
    
    // 属性
    lines.push_back({"", sf::Color::White});
    if (data->stats.attack > 0) {
        std::string str = "攻击力 +" + std::to_string(data->stats.attack);
        lines.push_back({sf::String::fromUtf8(str.begin(), str.end()), sf::Color(255, 100, 100)});
    }
    if (data->stats.defense > 0) {
        std::string str = "防御力 +" + std::to_string(data->stats.defense);
        lines.push_back({sf::String::fromUtf8(str.begin(), str.end()), sf::Color(100, 100, 255)});
    }
    if (data->stats.hp > 0) {
        std::string str = "生命值 +" + std::to_string(data->stats.hp);
        lines.push_back({sf::String::fromUtf8(str.begin(), str.end()), sf::Color(100, 255, 100)});
    }
    if (data->stats.ignoreDefense) {
        std::string str = "无视目标防御";
        lines.push_back({sf::String::fromUtf8(str.begin(), str.end()), sf::Color(255, 200, 100)});
    }
    
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
    
    float tooltipWidth = std::max(160.0f, maxWidth + padding * 2);
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

sf::Vector2f EquipmentPanel::getSlotPosition(EquipmentSlot slot) const {
    float centerX = panelPosition.x + panelSize.x / 2;
    float centerY = panelPosition.y + 200;
    float accessoryX = panelPosition.x + panelSize.x - 70;
    float accessoryY = panelPosition.y + 60;
    
    switch (slot) {
        case EquipmentSlot::Helmet:     return sf::Vector2f(centerX - 25, centerY - 140);
        case EquipmentSlot::Weapon:     return sf::Vector2f(centerX - 100, centerY - 40);
        case EquipmentSlot::SecondHand: return sf::Vector2f(centerX + 50, centerY - 40);
        case EquipmentSlot::Armor:      return sf::Vector2f(centerX - 25, centerY - 20);
        case EquipmentSlot::Gloves:     return sf::Vector2f(centerX - 100, centerY + 40);
        case EquipmentSlot::Cape:       return sf::Vector2f(centerX + 50, centerY + 40);
        case EquipmentSlot::Pants:      return sf::Vector2f(centerX - 25, centerY + 60);
        case EquipmentSlot::Boots:      return sf::Vector2f(centerX - 25, centerY + 120);
        case EquipmentSlot::Necklace:   return sf::Vector2f(accessoryX, accessoryY);
        case EquipmentSlot::Earring:    return sf::Vector2f(accessoryX, accessoryY + 50);
        case EquipmentSlot::Ring1:      return sf::Vector2f(accessoryX, accessoryY + 100);
        case EquipmentSlot::Ring2:      return sf::Vector2f(accessoryX, accessoryY + 150);
        case EquipmentSlot::Belt:       return sf::Vector2f(accessoryX, accessoryY + 200);
        case EquipmentSlot::Shoulder:   return sf::Vector2f(accessoryX, accessoryY + 250);
        default:                        return sf::Vector2f(0, 0);
    }
}

EquipmentSlot EquipmentPanel::getSlotAtPosition(const sf::Vector2f& pos) const {
    // 检查每个槽位
    for (int i = 0; i < static_cast<int>(EquipmentSlot::Count); i++) {
        EquipmentSlot slot = static_cast<EquipmentSlot>(i);
        sf::Vector2f slotPos = getSlotPosition(slot);
        
        // 确定槽位尺寸
        sf::Vector2f size(50, 50);
        if (slot >= EquipmentSlot::Necklace) {
            size = sf::Vector2f(40, 40);
        }
        
        sf::FloatRect bounds(slotPos.x, slotPos.y, size.x, size.y);
        if (bounds.contains(pos)) {
            return slot;
        }
    }
    
    return EquipmentSlot::Count;
}

void EquipmentPanel::open() {
    panelOpen = true;
    hoveredSlot = EquipmentSlot::Count;
    selectedSlot = EquipmentSlot::Count;
    std::cout << "[EquipmentPanel] Opened" << std::endl;
}

void EquipmentPanel::close() {
    panelOpen = false;
    std::cout << "[EquipmentPanel] Closed" << std::endl;
}

void EquipmentPanel::toggle() {
    if (panelOpen) close();
    else open();
}
