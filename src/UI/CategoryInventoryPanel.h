#pragma once
#include "../Items/CategoryInventory.h"
#include "../Items/Equipment.h"
#include <SFML/Graphics.hpp>
#include <functional>

// ============================================================================
// 分类背包面板 UI (Category Inventory Panel)
// 
// 设计规则：
// ============================================================================
// 
// 【三栏切换】
//   - 材料栏 (Materials)
//   - 消耗品栏 (Consumables)  
//   - 装备栏 (Equipment)
//
// 【交互方式】
//   - 单击: 选中物品
//   - 双击: 
//     - 消耗品: 直接使用
//     - 装备: 装备
//   - 右击: 显示上下文菜单
//     - 材料: 销毁、卖出
//     - 消耗品: 使用、销毁、卖出
//     - 种子: 种下、销毁、卖出
//     - 装备: 装备、销毁、卖出
//
// ============================================================================

class PlayerEquipment;  // Forward declaration

class CategoryInventoryPanel {
public:
    // 回调类型
    using DropItemCallback = std::function<void(const ItemStack&, sf::Vector2f)>;
    using SellCallback = std::function<void(int goldValue)>;
    using PlantCallback = std::function<bool()>;
    
    CategoryInventoryPanel();
    
    // 初始化
    bool init(const std::string& iconPath);
    bool loadFont(const std::string& fontPath);
    
    // 设置关联的背包
    void setInventory(CategoryInventory* inv) { inventory = inv; }
    
    // 设置关联的装备栏
    void setPlayerEquipment(PlayerEquipment* equip) { playerEquipment = equip; }
    
    // 设置图标位置
    void setIconPosition(float x, float y);
    
    // 设置玩家金币（用于显示）
    void setGold(int gold) { playerGold = gold; }
    
    // 更新
    void update(float dt);
    
    // 处理事件
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    
    // 渲染
    void render(sf::RenderWindow& window);
    
    // 打开/关闭
    void open();
    void close();
    void toggle();
    bool isOpen() const { return panelOpen; }
    
    // 回调设置
    void setOnDropItem(DropItemCallback cb) { onDropItem = cb; }
    void setOnSell(SellCallback cb) { onSell = cb; }
    void setOnPlant(PlantCallback cb) { onPlant = cb; }

private:
    // 内部方法
    void renderCategoryTabs(sf::RenderWindow& window);
    void renderSlots(sf::RenderWindow& window);
    void renderSlot(sf::RenderWindow& window, int index);
    void renderTooltip(sf::RenderWindow& window);
    void renderContextMenu(sf::RenderWindow& window);
    
    sf::Vector2f getSlotPosition(int index) const;
    int getSlotAtPosition(const sf::Vector2f& pos) const;
    int getTabAtPosition(const sf::Vector2f& pos) const;
    
    // 操作方法
    void selectSlot(int index);
    void switchCategory(InventoryCategory category);
    void handleDoubleClick(int slotIndex);
    void handleContextMenuClick(int optionIndex);
    void nextPage();
    void prevPage();

private:
    CategoryInventory* inventory;
    PlayerEquipment* playerEquipment;
    
    // UI状态
    bool panelOpen;
    InventoryCategory currentCategory;
    int currentPage;
    int selectedSlot;
    int hoveredSlot;
    
    // 右键菜单
    bool showContextMenu;
    sf::Vector2f contextMenuPos;
    int contextMenuSlot;
    std::vector<ContextMenuOption> contextMenuOptions;
    int hoveredMenuOption;
    
    // 双击检测
    sf::Clock doubleClickClock;
    int lastClickedSlot;
    static constexpr float DOUBLE_CLICK_TIME = 0.3f;
    
    // 金币
    int playerGold;
    
    // 图标
    sf::Texture iconTexture;
    sf::Sprite iconSprite;
    sf::Vector2f iconPosition;
    bool iconLoaded;
    
    // 面板
    sf::Vector2f panelPosition;
    sf::Vector2f panelSize;
    
    // 格子参数
    static constexpr float SLOT_SIZE = 60.0f;
    static constexpr float SLOT_PADDING = 5.0f;
    static constexpr float PANEL_PADDING = 20.0f;
    static constexpr float TAB_HEIGHT = 35.0f;
    
    // 字体
    sf::Font font;
    bool fontLoaded;
    
    // 回调
    DropItemCallback onDropItem;
    SellCallback onSell;
    PlantCallback onPlant;
    
    // 颜色常量
    static const sf::Color BG_COLOR;
    static const sf::Color SLOT_COLOR;
    static const sf::Color SLOT_HOVER_COLOR;
    static const sf::Color SLOT_SELECTED_COLOR;
    static const sf::Color TAB_COLOR;
    static const sf::Color TAB_ACTIVE_COLOR;
    static const sf::Color BORDER_COLOR;
    static const sf::Color MENU_BG_COLOR;
    static const sf::Color MENU_HOVER_COLOR;
};
