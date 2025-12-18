#include "EventLogPanel.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cmath>
#include <algorithm>

// ============================================================================
// UTF-8 字符串转换辅助函数
// ============================================================================
namespace {
    inline sf::String toSfString(const std::string& utf8Str) {
        return sf::String::fromUtf8(utf8Str.begin(), utf8Str.end());
    }
}

// ============================================================================
// 构造函数
// ============================================================================

EventLogPanel::EventLogPanel()
    : maxMessages(50)
    , defaultDuration(6.0f)
    , position(0.0f, 0.0f)
    , size(320.0f, 300.0f)
    , collapsed(false)
    , collapseAnim(1.0f)
    // 颜色配置
    , bgColor(15, 15, 25, 200)
    , borderColor(80, 70, 60, 255)
    , headerColor(30, 30, 45, 230)
    // 字体
    , fontLoaded(false)
    // 配置
    , showTimestamp(false)
    , lineHeight(22.0f)
    , padding(10.0f)
    , headerHeight(30.0f)
    // 滚动
    , scrollOffset(0.0f)
    , targetScrollOffset(0.0f)
    , maxScroll(0.0f)
    // 动画
    , globalAlpha(1.0f)
    // 事件颜色
    , itemColor(100, 220, 100)        // 绿色 - 物品
    , goldColor(255, 215, 0)          // 金色 - 金币
    , expColor(100, 180, 255)         // 蓝色 - 经验
    , levelUpColor(200, 100, 255)     // 紫色 - 升级
    , skillColor(100, 220, 220)       // 青色 - 技能
    , treeColor(180, 220, 100)        // 黄绿 - 树木
    , combatColor(255, 100, 100)      // 红色 - 战斗
    , systemColor(220, 220, 220)      // 白色 - 系统
    , achievementColor(255, 200, 50)  // 金色 - 成就
    , warningColor(255, 150, 50)      // 橙色 - 警告
{
}

// ============================================================================
// 初始化
// ============================================================================

bool EventLogPanel::init(const std::string& fontPath) {
    // 加载字体（优先使用系统中文字体）
    std::vector<std::string> fontPaths = {
        "C:/Windows/Fonts/msyh.ttc",      // 微软雅黑（优先）
        "C:/Windows/Fonts/simhei.ttf",    // 黑体
        "C:/Windows/Fonts/simsun.ttc",    // 宋体
        fontPath,
        "assets/fonts/pixel.ttf",
        "assets/fonts/font.ttf",
        "../../assets/fonts/pixel.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc"  // Linux中文字体
    };
    
    for (const auto& path : fontPaths) {
        if (!path.empty() && font.loadFromFile(path)) {
            fontLoaded = true;
            std::cout << "[EventLogPanel] 字体加载成功: " << path << std::endl;
            break;
        }
    }
    
    if (!fontLoaded) {
        std::cerr << "[EventLogPanel] 警告: 无法加载字体" << std::endl;
    }
    
    // 初始化背景
    background.setSize(size);
    background.setFillColor(bgColor);
    
    // 初始化边框
    border.setSize(sf::Vector2f(size.x + 4, size.y + 4));
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineThickness(2.0f);
    border.setOutlineColor(borderColor);
    
    // 初始化标题栏
    header.setSize(sf::Vector2f(size.x, headerHeight));
    header.setFillColor(headerColor);
    
    // 初始化标题文字
    if (fontLoaded) {
        titleText.setFont(font);
        titleText.setString(toSfString("📜 事件日志"));
        titleText.setCharacterSize(16);
        titleText.setFillColor(sf::Color(255, 220, 150));
        titleText.setStyle(sf::Text::Bold);
        
        collapseText.setFont(font);
        collapseText.setString("[-]");
        collapseText.setCharacterSize(14);
        collapseText.setFillColor(sf::Color(200, 200, 200));
    }
    
    std::cout << "[EventLogPanel] 初始化完成" << std::endl;
    return true;
}

// ============================================================================
// 设置位置和大小
// ============================================================================

void EventLogPanel::setPosition(float x, float y) {
    position = sf::Vector2f(x, y);
    
    background.setPosition(position);
    border.setPosition(position.x - 2, position.y - 2);
    header.setPosition(position);
    
    if (fontLoaded) {
        titleText.setPosition(position.x + padding, position.y + 5.0f);
        collapseText.setPosition(position.x + size.x - 30.0f, position.y + 6.0f);
    }
}

void EventLogPanel::setSize(float width, float height) {
    size = sf::Vector2f(width, height);
    
    background.setSize(size);
    border.setSize(sf::Vector2f(size.x + 4, size.y + 4));
    header.setSize(sf::Vector2f(size.x, headerHeight));
    
    // 更新位置
    setPosition(position.x, position.y);
}

// ============================================================================
// 更新
// ============================================================================

void EventLogPanel::update(float dt) {
    // 折叠动画
    float targetCollapseAnim = collapsed ? 0.0f : 1.0f;
    collapseAnim += (targetCollapseAnim - collapseAnim) * 8.0f * dt;
    
    // 更新消息
    for (auto it = messages.begin(); it != messages.end(); ) {
        EventMessage& msg = *it;
        
        // 更新生命周期
        msg.lifetime -= dt;
        
        // 新消息滑入动画
        if (msg.isNew) {
            msg.slideOffset -= msg.slideOffset * 10.0f * dt;
            if (msg.slideOffset < 1.0f) {
                msg.slideOffset = 0.0f;
                msg.isNew = false;
            }
        }
        
        // 计算透明度（淡出效果）
        if (msg.lifetime < 1.5f) {
            msg.alpha = msg.lifetime / 1.5f;
        } else if (msg.lifetime > msg.maxLifetime - 0.3f) {
            // 淡入效果
            msg.alpha = (msg.maxLifetime - msg.lifetime) / 0.3f;
        } else {
            msg.alpha = 1.0f;
        }
        
        // 特殊效果闪烁
        if (msg.type == EventType::LevelUp || msg.type == EventType::Achievement) {
            msg.flashTimer += dt * 4.0f;
            float flash = (std::sin(msg.flashTimer) + 1.0f) * 0.15f;
            msg.alpha = std::min(1.0f, msg.alpha + flash);
        }
        
        // 移除过期消息
        if (msg.lifetime <= 0.0f) {
            it = messages.erase(it);
        } else {
            ++it;
        }
    }
    
    // 平滑滚动
    scrollOffset += (targetScrollOffset - scrollOffset) * 10.0f * dt;
    
    // 计算最大滚动范围
    float contentHeight = messages.size() * lineHeight;
    float visibleHeight = size.y - headerHeight - padding * 2;
    maxScroll = std::max(0.0f, contentHeight - visibleHeight);
    
    // 自动滚动到最新消息
    if (!messages.empty() && messages.back().isNew) {
        targetScrollOffset = maxScroll;
    }
}

// ============================================================================
// 渲染
// ============================================================================

void EventLogPanel::render(sf::RenderWindow& window) {
    if (!fontLoaded) return;
    
    // 计算动画后的面板高度
    float animatedHeight = headerHeight + (size.y - headerHeight) * collapseAnim;
    
    // 更新背景大小（动画）
    sf::RectangleShape animBg = background;
    animBg.setSize(sf::Vector2f(size.x, animatedHeight));
    
    sf::RectangleShape animBorder = border;
    animBorder.setSize(sf::Vector2f(size.x + 4, animatedHeight + 4));
    
    // 绘制背景
    window.draw(animBorder);
    window.draw(animBg);
    
    // 绘制标题栏
    window.draw(header);
    window.draw(titleText);
    
    // 绘制折叠按钮
    collapseText.setString(collapsed ? "[+]" : "[-]");
    window.draw(collapseText);
    
    // 如果折叠，只显示标题
    if (collapseAnim < 0.1f) {
        return;
    }
    
    // 设置裁剪区域（只在面板内绘制消息）
    sf::View originalView = window.getView();
    
    // 计算消息区域
    float messageAreaTop = position.y + headerHeight + padding;
    float messageAreaHeight = animatedHeight - headerHeight - padding * 2;
    
    if (messageAreaHeight <= 0) return;
    
    // 创建裁剪视图
    sf::FloatRect viewport(
        position.x / window.getSize().x,
        messageAreaTop / window.getSize().y,
        size.x / window.getSize().x,
        messageAreaHeight / window.getSize().y
    );
    
    // 绘制消息（从底部向上）
    float y = messageAreaTop + messageAreaHeight;
    
    // 从最新消息开始向上绘制
    for (auto it = messages.rbegin(); it != messages.rend(); ++it) {
        const EventMessage& msg = *it;
        
        y -= lineHeight;
        
        // 跳过不可见的消息
        if (y + lineHeight < messageAreaTop) break;
        if (y > messageAreaTop + messageAreaHeight) continue;
        
        // 应用滑入偏移
        float drawY = y;
        float drawX = position.x + padding + msg.slideOffset;
        
        // 绘制消息背景（带透明度）
        sf::RectangleShape msgBg;
        msgBg.setSize(sf::Vector2f(size.x - padding * 2, lineHeight - 2));
        msgBg.setPosition(position.x + padding, drawY);
        msgBg.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(50 * msg.alpha * collapseAnim)));
        window.draw(msgBg);
        
        // 绘制消息文字
        sf::Text text;
        text.setFont(font);
        text.setCharacterSize(14);
        
        // 构建显示文本
        std::string displayText = getEventPrefix(msg.type) + " " + msg.text;
        text.setString(toSfString(displayText));
        
        // 设置颜色（带透明度）
        sf::Color color = msg.color;
        color.a = static_cast<sf::Uint8>(255 * msg.alpha * collapseAnim);
        text.setFillColor(color);
        
        text.setPosition(drawX, drawY + 2);
        window.draw(text);
    }
    
    // 绘制滚动条（如果需要）
    if (maxScroll > 0 && collapseAnim > 0.5f) {
        float scrollBarHeight = messageAreaHeight * (messageAreaHeight / (messageAreaHeight + maxScroll));
        float scrollBarY = messageAreaTop + (scrollOffset / maxScroll) * (messageAreaHeight - scrollBarHeight);
        
        sf::RectangleShape scrollBar;
        scrollBar.setSize(sf::Vector2f(4.0f, scrollBarHeight));
        scrollBar.setPosition(position.x + size.x - 8.0f, scrollBarY);
        scrollBar.setFillColor(sf::Color(100, 100, 100, static_cast<sf::Uint8>(150 * collapseAnim)));
        window.draw(scrollBar);
    }
    
    // 绘制分隔线
    sf::RectangleShape separator;
    separator.setSize(sf::Vector2f(size.x - padding * 2, 1.0f));
    separator.setPosition(position.x + padding, position.y + headerHeight);
    separator.setFillColor(sf::Color(100, 90, 80, static_cast<sf::Uint8>(200 * collapseAnim)));
    window.draw(separator);
}

// ============================================================================
// 添加消息便捷方法
// ============================================================================

void EventLogPanel::addMessage(const std::string& text, EventType type) {
    EventMessage msg;
    msg.text = text;
    msg.type = type;
    msg.color = getEventColor(type);
    msg.lifetime = defaultDuration;
    msg.maxLifetime = defaultDuration;
    
    pushMessage(msg);
}

void EventLogPanel::addItemObtained(const std::string& itemName, int count, const std::string& iconId) {
    EventMessage msg;
    msg.type = EventType::ItemObtained;
    msg.color = itemColor;
    msg.lifetime = defaultDuration;
    msg.maxLifetime = defaultDuration;
    msg.hasIcon = !iconId.empty();
    msg.iconId = iconId;
    msg.value = count;
    
    std::stringstream ss;
    if (count > 1) {
        ss << itemName << " x" << count;
    } else {
        ss << itemName;
    }
    msg.text = ss.str();
    
    pushMessage(msg);
    std::cout << "[EventLog] 获得物品: " << msg.text << std::endl;
}

void EventLogPanel::addGoldObtained(int amount) {
    EventMessage msg;
    msg.type = EventType::GoldObtained;
    msg.color = goldColor;
    msg.lifetime = defaultDuration;
    msg.maxLifetime = defaultDuration;
    msg.value = amount;
    
    std::stringstream ss;
    ss << "+" << amount << " 金币";
    msg.text = ss.str();
    
    pushMessage(msg);
    std::cout << "[EventLog] 获得金币: " << amount << std::endl;
}

void EventLogPanel::addExpObtained(int amount, const std::string& source) {
    EventMessage msg;
    msg.type = EventType::ExpObtained;
    msg.color = expColor;
    msg.lifetime = defaultDuration;
    msg.maxLifetime = defaultDuration;
    msg.value = amount;
    
    std::stringstream ss;
    ss << "+" << amount << " 经验";
    if (!source.empty()) {
        ss << " (" << source << ")";
    }
    msg.text = ss.str();
    
    pushMessage(msg);
    std::cout << "[EventLog] 获得经验: " << amount << std::endl;
}

void EventLogPanel::addLevelUp(int newLevel) {
    EventMessage msg;
    msg.type = EventType::LevelUp;
    msg.color = levelUpColor;
    msg.lifetime = defaultDuration * 1.5f;  // 升级消息显示更久
    msg.maxLifetime = msg.lifetime;
    msg.value = newLevel;
    
    std::stringstream ss;
    ss << "恭喜升级！当前等级: Lv." << newLevel;
    msg.text = ss.str();
    
    pushMessage(msg);
    std::cout << "[EventLog] ★★★ 升级到 Lv." << newLevel << " ★★★" << std::endl;
}

void EventLogPanel::addSkillLevelUp(const std::string& skillName, int newLevel) {
    EventMessage msg;
    msg.type = EventType::SkillLevelUp;
    msg.color = skillColor;
    msg.lifetime = defaultDuration * 1.2f;
    msg.maxLifetime = msg.lifetime;
    msg.value = newLevel;
    
    std::stringstream ss;
    ss << skillName << " 技能升级! Lv." << newLevel;
    msg.text = ss.str();
    
    pushMessage(msg);
    std::cout << "[EventLog] 技能升级: " << skillName << " -> Lv." << newLevel << std::endl;
}

void EventLogPanel::addTreeMature(const std::string& treeName) {
    EventMessage msg;
    msg.type = EventType::TreeMature;
    msg.color = treeColor;
    msg.lifetime = defaultDuration;
    msg.maxLifetime = defaultDuration;
    
    std::stringstream ss;
    ss << treeName << " 已成熟，可以收获了！";
    msg.text = ss.str();
    
    pushMessage(msg);
    std::cout << "[EventLog] 树木成熟: " << treeName << std::endl;
}

void EventLogPanel::addTreeChopped(const std::string& treeName) {
    EventMessage msg;
    msg.type = EventType::TreeChopped;
    msg.color = sf::Color(180, 140, 100);  // 棕色
    msg.lifetime = defaultDuration;
    msg.maxLifetime = defaultDuration;
    
    std::stringstream ss;
    ss << "砍伐了 " << treeName;
    msg.text = ss.str();
    
    pushMessage(msg);
    std::cout << "[EventLog] 砍伐树木: " << treeName << std::endl;
}

void EventLogPanel::addFruitHarvested(const std::string& fruitName, int count) {
    EventMessage msg;
    msg.type = EventType::FruitHarvested;
    msg.color = sf::Color(255, 180, 100);  // 橙色
    msg.lifetime = defaultDuration;
    msg.maxLifetime = defaultDuration;
    msg.value = count;
    
    std::stringstream ss;
    ss << "采摘了 " << fruitName;
    if (count > 1) {
        ss << " x" << count;
    }
    msg.text = ss.str();
    
    pushMessage(msg);
    std::cout << "[EventLog] 采摘果实: " << fruitName << " x" << count << std::endl;
}

void EventLogPanel::addCombatMessage(const std::string& text) {
    EventMessage msg;
    msg.type = EventType::Combat;
    msg.color = combatColor;
    msg.lifetime = defaultDuration * 0.8f;  // 战斗消息显示时间短些
    msg.maxLifetime = msg.lifetime;
    msg.text = text;
    
    pushMessage(msg);
}

void EventLogPanel::addAchievement(const std::string& achievementName) {
    EventMessage msg;
    msg.type = EventType::Achievement;
    msg.color = achievementColor;
    msg.lifetime = defaultDuration * 2.0f;  // 成就消息显示更久
    msg.maxLifetime = msg.lifetime;
    
    std::stringstream ss;
    ss << "🏆 成就达成: " << achievementName;
    msg.text = ss.str();
    
    pushMessage(msg);
    std::cout << "[EventLog] ★ 成就达成: " << achievementName << std::endl;
}

void EventLogPanel::addWarning(const std::string& text) {
    EventMessage msg;
    msg.type = EventType::Warning;
    msg.color = warningColor;
    msg.lifetime = defaultDuration;
    msg.maxLifetime = defaultDuration;
    msg.text = text;
    
    pushMessage(msg);
    std::cout << "[EventLog] 警告: " << text << std::endl;
}

// ============================================================================
// 辅助方法
// ============================================================================

void EventLogPanel::clearMessages() {
    messages.clear();
    scrollOffset = 0.0f;
    targetScrollOffset = 0.0f;
}

void EventLogPanel::setPanelStyle(const sf::Color& bg, const sf::Color& border) {
    bgColor = bg;
    borderColor = border;
    background.setFillColor(bgColor);
    this->border.setOutlineColor(borderColor);
}

sf::Color EventLogPanel::getEventColor(EventType type) const {
    switch (type) {
        case EventType::ItemObtained:   return itemColor;
        case EventType::GoldObtained:   return goldColor;
        case EventType::ExpObtained:    return expColor;
        case EventType::LevelUp:        return levelUpColor;
        case EventType::SkillLevelUp:   return skillColor;
        case EventType::TreeMature:     return treeColor;
        case EventType::TreeChopped:    return sf::Color(180, 140, 100);
        case EventType::FruitHarvested: return sf::Color(255, 180, 100);
        case EventType::Combat:         return combatColor;
        case EventType::System:         return systemColor;
        case EventType::Achievement:    return achievementColor;
        case EventType::Warning:        return warningColor;
        default:                        return systemColor;
    }
}

std::string EventLogPanel::getEventPrefix(EventType type) const {
    switch (type) {
        case EventType::ItemObtained:   return "📦";
        case EventType::GoldObtained:   return "💰";
        case EventType::ExpObtained:    return "✨";
        case EventType::LevelUp:        return "⭐";
        case EventType::SkillLevelUp:   return "📈";
        case EventType::TreeMature:     return "🌳";
        case EventType::TreeChopped:    return "🪓";
        case EventType::FruitHarvested: return "🍎";
        case EventType::Combat:         return "⚔️";
        case EventType::System:         return "📢";
        case EventType::Achievement:    return "🏆";
        case EventType::Warning:        return "⚠️";
        default:                        return "•";
    }
}

void EventLogPanel::pushMessage(const EventMessage& msg) {
    // 添加新消息
    messages.push_back(msg);
    
    // 限制消息数量
    while (messages.size() > maxMessages) {
        messages.pop_front();
    }
    
    // 自动滚动到底部
    targetScrollOffset = maxScroll;
}

void EventLogPanel::drawMessage(sf::RenderWindow& window, const EventMessage& msg, float y, float panelWidth) {
    // 此方法保留用于未来扩展（如绘制物品图标等）
}

void EventLogPanel::drawCollapseButton(sf::RenderWindow& window) {
    // 此方法保留用于未来扩展（如自定义折叠按钮样式）
}
