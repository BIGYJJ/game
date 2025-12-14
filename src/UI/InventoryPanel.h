#pragma once
#include "../Items/Inventory.h"
#include <SFML/Graphics.hpp>
#include <functional>

// ============================================================================
// 背包面板 UI (Inventory Panel)
// 
// 设计规则：
// ============================================================================
// 
// 【显示方式】
//   - 点击背包图标或按 I/B 键打开背包
//   - 背包显示在屏幕中央
//   - 每页 30 格 (6列 x 5行)
//   - 底部有翻页按钮
//
// 【交互功能】
//   - 左键点击: 选中物品
//   - 右键点击: 使用物品（消耗品）/ 打开菜单
//   - 拖拽: 移动物品位置
//   - 悬浮: 显示物品信息
//   - Shift+点击: 快速丢弃
//
// 【UI元素】
//   - 标题栏: "背包" + 关闭按钮
//   - 物品格子: 6x5 网格
//   - 物品数量: 右下角显示
//   - 翻页按钮: < 1/4 >
//   - 金币显示: 底部显示当前金币
//   - 操作按钮: 整理、关闭
//
// ============================================================================

class InventoryPanel {
public:
    // 回调类型
    using DropItemCallback = std::function<void(const ItemStack&, sf::Vector2f)>;
    
    InventoryPanel();
    
    // 初始化
    bool init(const std::string& iconPath);
    bool loadFont(const std::string& fontPath);
    
    // 设置关联的背包
    void setInventory(Inventory* inv) { inventory = inv; }
    
    // 设置图标位置（用于点击打开）
    void setIconPosition(float x, float y);
    
    // 设置玩家金币（用于显示）
    void setGold(int gold) { playerGold = gold; }
    
    // 更新
    void update(float dt);
    
    // 处理事件
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    
    // 渲染
    void render(sf::RenderWindow& window);
    
    // 打开/关闭背包
    void open();
    void close();
    void toggle();
    bool isOpen() const { return panelOpen; }
    
    // 回调设置
    void setOnDropItem(DropItemCallback cb) { onDropItem = cb; }

private:
    // 内部方法
    void initSlots();
    void updateSlotContents();
    sf::Vector2f getSlotPosition(int index) const;
    int getSlotAtPosition(const sf::Vector2f& pos) const;
    void renderSlot(sf::RenderWindow& window, int index);
    void renderTooltip(sf::RenderWindow& window);
    void renderContextMenu(sf::RenderWindow& window);
    
    // 操作方法
    void selectSlot(int index);
    void useSelectedItem();
    void dropSelectedItem(int count = -1);
    void nextPage();
    void prevPage();

private:
    Inventory* inventory;
    
    // UI状态
    bool panelOpen;
    int currentPage;
    int selectedSlot;           // -1表示无选中
    int hoveredSlot;            // -1表示无悬浮
    bool showContextMenu;
    sf::Vector2f contextMenuPos;
    
    // 金币
    int playerGold;
    
    // 图标（用于打开背包）
    sf::Texture iconTexture;
    sf::Sprite iconSprite;
    sf::Vector2f iconPosition;
    bool iconLoaded;
    
    // 面板
    sf::Vector2f panelPosition;
    sf::Vector2f panelSize;
    
    // 格子（增大1.5倍）
    static constexpr float SLOT_SIZE = 72.0f;       // 原48 * 1.5 = 72
    static constexpr float SLOT_PADDING = 6.0f;     // 原4 * 1.5 = 6
    static constexpr float PANEL_PADDING = 30.0f;   // 原20 * 1.5 = 30
    
    // 字体
    sf::Font font;
    bool fontLoaded;
    
    // 回调
    DropItemCallback onDropItem;
    
    // 颜色常量
    static const sf::Color BG_COLOR;
    static const sf::Color SLOT_COLOR;
    static const sf::Color SLOT_HOVER_COLOR;
    static const sf::Color SLOT_SELECTED_COLOR;
    static const sf::Color BORDER_COLOR;
};
