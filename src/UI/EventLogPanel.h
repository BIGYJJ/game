#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <deque>
#include <functional>

// ============================================================================
// 事件日志面板 UI
// 
// 功能：
//   - 在屏幕右侧显示游戏事件信息
//   - 支持多种事件类型（物品获得、金币、经验、升级等）
//   - 消息带有淡入淡出动画效果
//   - 自动滚动显示最新消息
//   - 可折叠/展开
// ============================================================================

// 事件类型枚举
enum class EventType {
    ItemObtained,       // 获得物品（绿色）
    GoldObtained,       // 获得金币（金色）
    ExpObtained,        // 获得经验（蓝色）
    LevelUp,            // 升级（紫色闪烁）
    SkillLevelUp,       // 技能升级（青色）
    TreeMature,         // 树木成熟（黄绿色）
    TreeChopped,        // 砍伐树木（棕色）
    FruitHarvested,     // 采摘果实（橙色）
    Combat,             // 战斗信息（红色）
    System,             // 系统消息（白色）
    Achievement,        // 成就达成（金色闪烁）
    Warning             // 警告信息（橙红色）
};

// 单条事件消息
struct EventMessage {
    std::string text;           // 消息文本
    EventType type;             // 事件类型
    float lifetime;             // 剩余显示时间
    float maxLifetime;          // 最大显示时间
    float alpha;                // 当前透明度 (0-1)
    sf::Color color;            // 文字颜色
    bool hasIcon;               // 是否显示图标
    std::string iconId;         // 图标ID（用于物品图标）
    int value;                  // 数值（用于显示+XX）
    float flashTimer;           // 闪烁计时器（用于特殊效果）
    bool isNew;                 // 是否是新消息（用于弹出动画）
    float slideOffset;          // 滑入偏移量
    
    EventMessage() 
        : type(EventType::System)
        , lifetime(5.0f)
        , maxLifetime(5.0f)
        , alpha(1.0f)
        , hasIcon(false)
        , value(0)
        , flashTimer(0.0f)
        , isNew(true)
        , slideOffset(100.0f)
    {}
};

class EventLogPanel {
public:
    EventLogPanel();
    
    // 初始化（加载字体和资源）
    bool init(const std::string& fontPath = "");
    
    // 设置面板位置（右上角）
    void setPosition(float x, float y);
    
    // 设置面板大小
    void setSize(float width, float height);
    
    // 更新（处理动画和消息生命周期）
    void update(float dt);
    
    // 渲染
    void render(sf::RenderWindow& window);
    
    // ========================================
    // 添加事件消息的便捷方法
    // ========================================
    
    // 通用添加消息
    void addMessage(const std::string& text, EventType type = EventType::System);
    
    // 获得物品
    void addItemObtained(const std::string& itemName, int count = 1, const std::string& iconId = "");
    
    // 获得金币
    void addGoldObtained(int amount);
    
    // 获得经验
    void addExpObtained(int amount, const std::string& source = "");
    
    // 升级
    void addLevelUp(int newLevel);
    
    // 技能升级
    void addSkillLevelUp(const std::string& skillName, int newLevel);
    
    // 树木成熟
    void addTreeMature(const std::string& treeName);
    
    // 砍伐树木
    void addTreeChopped(const std::string& treeName);
    
    // 采摘果实
    void addFruitHarvested(const std::string& fruitName, int count = 1);
    
    // 战斗信息
    void addCombatMessage(const std::string& text);
    
    // 成就达成
    void addAchievement(const std::string& achievementName);
    
    // 警告
    void addWarning(const std::string& text);
    
    // ========================================
    // 面板控制
    // ========================================
    
    // 清空所有消息
    void clearMessages();
    
    // 折叠/展开
    void toggle() { collapsed = !collapsed; }
    void collapse() { collapsed = true; }
    void expand() { collapsed = false; }
    bool isCollapsed() const { return collapsed; }
    
    // 设置最大消息数量
    void setMaxMessages(size_t max) { maxMessages = max; }
    
    // 设置消息默认持续时间
    void setMessageDuration(float seconds) { defaultDuration = seconds; }
    
    // 设置是否显示时间戳
    void setShowTimestamp(bool show) { showTimestamp = show; }
    
    // 面板样式
    void setPanelStyle(const sf::Color& bgColor, const sf::Color& borderColor);

private:
    // 获取事件类型对应的颜色
    sf::Color getEventColor(EventType type) const;
    
    // 获取事件类型对应的前缀图标
    std::string getEventPrefix(EventType type) const;
    
    // 添加消息到队列
    void pushMessage(const EventMessage& msg);
    
    // 绘制单条消息
    void drawMessage(sf::RenderWindow& window, const EventMessage& msg, float y, float panelWidth);
    
    // 绘制折叠按钮
    void drawCollapseButton(sf::RenderWindow& window);

private:
    // === 消息队列 ===
    std::deque<EventMessage> messages;
    size_t maxMessages;
    float defaultDuration;
    
    // === 面板属性 ===
    sf::Vector2f position;          // 面板位置
    sf::Vector2f size;              // 面板大小
    bool collapsed;                 // 是否折叠
    float collapseAnim;             // 折叠动画进度 (0-1)
    
    // === 外观 ===
    sf::RectangleShape background;
    sf::RectangleShape border;
    sf::RectangleShape header;
    sf::Color bgColor;
    sf::Color borderColor;
    sf::Color headerColor;
    
    // === 字体和文字 ===
    sf::Font font;
    bool fontLoaded;
    sf::Text titleText;
    sf::Text collapseText;
    
    // === 配置 ===
    bool showTimestamp;
    float lineHeight;
    float padding;
    float headerHeight;
    
    // === 滚动 ===
    float scrollOffset;
    float targetScrollOffset;
    float maxScroll;
    
    // === 动画 ===
    float globalAlpha;
    
    // === 颜色配置 ===
    sf::Color itemColor;        // 物品获得
    sf::Color goldColor;        // 金币
    sf::Color expColor;         // 经验
    sf::Color levelUpColor;     // 升级
    sf::Color skillColor;       // 技能
    sf::Color treeColor;        // 树木
    sf::Color combatColor;      // 战斗
    sf::Color systemColor;      // 系统
    sf::Color achievementColor; // 成就
    sf::Color warningColor;     // 警告
};
