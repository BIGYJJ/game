#pragma once
#include <SFML/Graphics.hpp>
#include "../Entity/PlayerStats.h"
#include <string>
#include <vector>

// ============================================================================
// 属性面板UI
// 
// 功能：
//   - 底部显示图标按钮
//   - 点击图标打开/关闭属性面板
//   - 显示玩家所有属性信息
// ============================================================================

class StatsPanel {
public:
    StatsPanel();
    
    // 初始化（加载资源）
    bool init(const std::string& iconPath, const std::string& fontPath = "");
    
    // 设置位置（图标位置）
    void setPosition(float x, float y);
    
    // 设置图标位置（屏幕坐标）
    void setIconPosition(float x, float y);
    
    // 更新（处理动画等）
    void update(float dt);
    
    // 处理输入事件
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    
    // 渲染
    void render(sf::RenderWindow& window);
    
    // 更新显示的属性数据
    void updateStats(const PlayerStats& stats);
    
    // 面板状态
    bool isOpen() const { return panelOpen; }
    void open() { panelOpen = true; }
    void close() { panelOpen = false; }
    void toggle() { panelOpen = !panelOpen; }
    
    // 设置面板样式
    void setPanelColor(const sf::Color& bg, const sf::Color& border);
    void setTextColor(const sf::Color& title, const sf::Color& value);
    
private:
    // 创建面板内容
    void createPanel();
    
    // 更新面板文字
    void updatePanelText(const PlayerStats& stats);
    
    // 绘制进度条
    void drawProgressBar(sf::RenderWindow& window, float x, float y, 
                         float width, float height,
                         float percent, const sf::Color& fillColor,
                         const std::string& label = "");
    
    void drawProgressBar(sf::RenderWindow& window, float x, float y, 
                         float width, float height,
                         float percent, const sf::Color& fillColor,
                         const std::wstring& label);
    
    // 检查鼠标是否在图标上
    bool isMouseOverIcon(const sf::Vector2f& mousePos) const;
    
    // 检查鼠标是否在面板上
    bool isMouseOverPanel(const sf::Vector2f& mousePos) const;

private:
    // === 图标相关 ===
    sf::Texture iconTexture;
    sf::Sprite iconSprite;
    sf::Vector2f iconPosition;
    float iconScale;
    bool iconHovered;
    
    // === 面板相关 ===
    bool panelOpen;
    sf::RectangleShape panelBackground;
    sf::RectangleShape panelBorder;
    sf::Vector2f panelPosition;
    sf::Vector2f panelSize;
    
    // === 字体和文字 ===
    sf::Font font;
    bool fontLoaded;
    sf::Text titleText;
    std::vector<sf::Text> statLabels;
    std::vector<sf::Text> statValues;
    
    // === 进度条数据 ===
    float healthPercent;
    float staminaPercent;
    float hungerPercent;
    float expPercent;
    
    // === 颜色配置 ===
    sf::Color panelBgColor;
    sf::Color panelBorderColor;
    sf::Color titleColor;
    sf::Color labelColor;
    sf::Color valueColor;
    sf::Color healthColor;
    sf::Color staminaColor;
    sf::Color hungerColor;
    sf::Color expColor;
    
    // === 动画 ===
    float hoverScale;
    float targetScale;
    float panelAlpha;
    float targetAlpha;
    
    // === 缓存的属性值 ===
    int cachedLevel;
    int cachedGold;
    float cachedAttack;
    float cachedDefense;
    float cachedSpeed;
    float cachedDodge;
    float cachedLuck;
    float cachedDamageBonus;
    float cachedDodgeReduction;
    int cachedFarmingLv;
    int cachedFishingLv;
    int cachedMiningLv;
};