#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include "../Pet/PetManager.h"

// ============================================================================
// 宠物UI面板 (Pet Panel) - 宠物栏功能图标
// 
// 功能：
//   - 显示当前宠物信息
//   - 切换宠物
//   - 查看宠物详情
//   - 洗点按钮
// ============================================================================

using WashCallback = std::function<bool(int slotIndex, float playerLuck)>;
using SwitchPetCallback = std::function<bool(int slotIndex)>;

class PetPanel {
public:
    PetPanel();
    
    // ========================================
    // 初始化
    // ========================================
    bool init(const std::string& iconPath);
    void setInventoryManager(PetManager* manager) { petManager = manager; }
    
    // ========================================
    // 更新和渲染
    // ========================================
    void update(float dt);
    void render(sf::RenderWindow& window);
    
    // ========================================
    // 事件处理
    // ========================================
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    
    // ========================================
    // 面板显示控制
    // ========================================
    void toggle();
    void open();
    void close();
    bool isOpen() const { return panelOpen; }
    
    // ========================================
    // 图标位置
    // ========================================
    void setIconPosition(float x, float y);
    sf::Vector2f getIconPosition() const { return iconPosition; }
    
    // ========================================
    // 回调设置
    // ========================================
    void setWashCallback(WashCallback cb) { onWash = cb; }
    void setSwitchPetCallback(SwitchPetCallback cb) { onSwitchPet = cb; }
    
    // ========================================
    // 设置数据
    // ========================================
    void setPlayerLuck(float luck) { playerLuck = luck; }
    void setCleanserCount(int count) { cleanserCount = count; }

private:
    bool loadFont(const std::string& fontPath);
    void renderIcon(sf::RenderWindow& window);
    void renderPanel(sf::RenderWindow& window);
    void renderPetSlots(sf::RenderWindow& window);
    void renderPetInfo(sf::RenderWindow& window);
    
    sf::FloatRect getIconRect() const;
    sf::FloatRect getSlotRect(int index) const;
    sf::FloatRect getWashButtonRect() const;
    sf::FloatRect getCloseButtonRect() const;

private:
    PetManager* petManager;
    
    // 字体
    sf::Font font;
    bool fontLoaded;
    
    // 图标
    sf::Texture iconTexture;
    sf::Sprite iconSprite;
    sf::Vector2f iconPosition;
    bool iconLoaded;
    
    // 面板状态
    bool panelOpen;
    sf::Vector2f panelPos;
    sf::Vector2f panelSize;
    
    // 选中状态
    int hoveredSlot;
    
    // 数据
    float playerLuck;
    int cleanserCount;
    
    // 回调
    WashCallback onWash;
    SwitchPetCallback onSwitchPet;
    
    // 常量 - 增大图标尺寸
    static constexpr float ICON_SIZE = 64.0f;      // 从48增大到64
    static constexpr float SLOT_SIZE = 80.0f;      // 从68增大到80
    static constexpr float SLOT_PADDING = 10.0f;   // 从8增大到10
    static constexpr float PANEL_PADDING = 24.0f;  // 从20增大到24
};

// ============================================================================
// 宠物孵化面板 (Hatch Panel) - 孵化栏功能图标
// 
// 功能：
//   - 显示精元数量
//   - 使用强化剂
//   - 孵化宠物
//   - 概率预览
// ============================================================================

using HatchCallback = std::function<bool(int petTypeId, int enhancerCount)>;

class HatchPanel {
public:
    HatchPanel();
    
    // ========================================
    // 初始化
    // ========================================
    bool init(const std::string& iconPath);
    void setPetManager(PetManager* manager) { petManager = manager; }
    
    // ========================================
    // 更新和渲染
    // ========================================
    void update(float dt);
    void render(sf::RenderWindow& window);
    
    // ========================================
    // 事件处理
    // ========================================
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    
    // ========================================
    // 面板显示控制
    // ========================================
    void toggle();
    void open();
    void close();
    bool isOpen() const { return panelOpen; }
    
    // ========================================
    // 图标位置
    // ========================================
    void setIconPosition(float x, float y);
    sf::Vector2f getIconPosition() const { return iconPosition; }
    
    // ========================================
    // 回调设置
    // ========================================
    void setHatchCallback(HatchCallback cb) { onHatch = cb; }
    
    // ========================================
    // 设置物品数量
    // ========================================
    void setEssenceCount(int petTypeId, int count);
    void setEnhancerCount(int count) { enhancerCount = count; }
    int getSelectedEnhancerCount() const { return selectedEnhancerCount; }

private:
    bool loadFont(const std::string& fontPath);
    void renderIcon(sf::RenderWindow& window);
    void renderPanel(sf::RenderWindow& window);
    void renderProbabilityBar(sf::RenderWindow& window, float x, float y, float width, float height);
    
    sf::FloatRect getIconRect() const;
    sf::FloatRect getHatchButtonRect() const;
    sf::FloatRect getCloseButtonRect() const;
    sf::FloatRect getEnhancerPlusRect() const;
    sf::FloatRect getEnhancerMinusRect() const;

private:
    PetManager* petManager;
    
    // 字体
    sf::Font font;
    bool fontLoaded;
    
    // 图标
    sf::Texture iconTexture;
    sf::Sprite iconSprite;
    sf::Vector2f iconPosition;
    bool iconLoaded;
    
    // 面板状态
    bool panelOpen;
    sf::Vector2f panelPos;
    sf::Vector2f panelSize;
    
    // 孵化选项
    int selectedHatchType;
    int selectedEnhancerCount;
    
    // 物品数量
    std::map<int, int> essenceCounts;
    int enhancerCount;
    
    // 回调
    HatchCallback onHatch;
    
    // 常量 - 增大图标尺寸
    static constexpr float ICON_SIZE = 64.0f;      // 从48增大到64
    static constexpr float PANEL_PADDING = 20.0f;
};
