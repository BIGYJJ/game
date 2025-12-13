#include "StatsPanel.h"
#include <sstream>
#include <iomanip>
#include <iostream>

// ============================================================================
// 构造函数
// ============================================================================

StatsPanel::StatsPanel()
    : iconPosition(20.0f, 0.0f)
    , iconScale(2.0f)
    , iconHovered(false)
    , panelOpen(false)
    , panelSize(320.0f, 510.0f)  // 增大面板尺寸
    , fontLoaded(false)
    // 进度条
    , healthPercent(1.0f)
    , staminaPercent(1.0f)
    , hungerPercent(1.0f)
    , expPercent(0.0f)
    // 颜色
    , panelBgColor(20, 20, 30, 230)
    , panelBorderColor(100, 80, 60, 255)
    , titleColor(255, 215, 0)        // 金色标题
    , labelColor(200, 200, 200)      // 灰白标签
    , valueColor(255, 255, 255)      // 白色数值
    , healthColor(220, 60, 60)       // 红色生命
    , staminaColor(60, 180, 60)      // 绿色体力
    , hungerColor(210, 150, 60)      // 橙色饥饿
    , expColor(100, 150, 255)        // 蓝色经验
    // 动画
    , hoverScale(1.0f)
    , targetScale(1.0f)
    , panelAlpha(0.0f)
    , targetAlpha(0.0f)
    // 缓存
    , cachedLevel(1)
    , cachedGold(0)
    , cachedAttack(0)
    , cachedDefense(0)
    , cachedSpeed(0)
    , cachedDodge(0)
    , cachedLuck(0)
    , cachedDamageBonus(0)
    , cachedDodgeReduction(0)
    , cachedFarmingLv(1)
    , cachedFishingLv(1)
    , cachedMiningLv(1)
{
}

// ============================================================================
// 初始化
// ============================================================================

bool StatsPanel::init(const std::string& iconPath, const std::string& fontPath) {
    // 加载图标
    if (!iconTexture.loadFromFile(iconPath)) {
        std::cerr << "[StatsPanel] 无法加载图标: " << iconPath << std::endl;
        // 创建占位图标
        sf::Image placeholder;
        placeholder.create(32, 32, sf::Color(128, 0, 128));
        iconTexture.loadFromImage(placeholder);
    }
    
    iconSprite.setTexture(iconTexture);
    iconSprite.setScale(iconScale, iconScale);
    
    // 加载字体
    std::vector<std::string> fontPaths = {
        fontPath,
        "assets/fonts/pixel.ttf",
        "assets/fonts/font.ttf",
        "C:/Windows/Fonts/msyh.ttc",      // 微软雅黑
        "C:/Windows/Fonts/simhei.ttf",    // 黑体
        "C:/Windows/Fonts/arial.ttf",      // Arial
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
    };
    
    for (const auto& path : fontPaths) {
        if (!path.empty() && font.loadFromFile(path)) {
            fontLoaded = true;
            std::cout << "[StatsPanel] 字体加载成功: " << path << std::endl;
            break;
        }
    }
    
    if (!fontLoaded) {
        std::cerr << "[StatsPanel] 警告: 无法加载字体，文字将不显示" << std::endl;
    }
    
    // 创建面板
    createPanel();
    
    return true;
}

// ============================================================================
// 创建面板
// ============================================================================

void StatsPanel::createPanel() {
    // 面板背景
    panelBackground.setSize(panelSize);
    panelBackground.setFillColor(panelBgColor);
    
    // 面板边框
    panelBorder.setSize(sf::Vector2f(panelSize.x + 4, panelSize.y + 4));
    panelBorder.setFillColor(sf::Color::Transparent);
    panelBorder.setOutlineThickness(3.0f);
    panelBorder.setOutlineColor(panelBorderColor);
    
    if (fontLoaded) {
        // 标题
        titleText.setFont(font);
        titleText.setString(L"人物属性");
        titleText.setCharacterSize(28);  // 增大标题
        titleText.setFillColor(titleColor);
        titleText.setStyle(sf::Text::Bold);
    }
}

// ============================================================================
// 设置位置
// ============================================================================

void StatsPanel::setPosition(float x, float y) {
    setIconPosition(x, y);
}

void StatsPanel::setIconPosition(float x, float y) {
    iconPosition = sf::Vector2f(x, y);
    iconSprite.setPosition(iconPosition);
    
    // 面板位置在图标上方
    float panelX = x;
    float panelY = y - panelSize.y - 10.0f;
    
    // 确保面板不超出屏幕顶部
    if (panelY < 10.0f) {
        panelY = 10.0f;
    }
    
    panelPosition = sf::Vector2f(panelX, panelY);
    panelBackground.setPosition(panelPosition);
    panelBorder.setPosition(panelPosition.x - 2, panelPosition.y - 2);
    
    if (fontLoaded) {
        titleText.setPosition(panelPosition.x + 15.0f, panelPosition.y + 10.0f);
    }
}

// ============================================================================
// 更新
// ============================================================================

void StatsPanel::update(float dt) {
    // 图标悬停动画
    float scaleSpeed = 8.0f;
    if (iconHovered) {
        targetScale = 1.15f;
    } else {
        targetScale = 1.0f;
    }
    hoverScale += (targetScale - hoverScale) * scaleSpeed * dt;
    
    // 更新图标缩放（保持中心点）
    sf::Vector2f iconCenter = iconPosition + sf::Vector2f(
        iconTexture.getSize().x * iconScale / 2.0f,
        iconTexture.getSize().y * iconScale / 2.0f
    );
    iconSprite.setScale(iconScale * hoverScale, iconScale * hoverScale);
    iconSprite.setPosition(
        iconCenter.x - iconTexture.getSize().x * iconScale * hoverScale / 2.0f,
        iconCenter.y - iconTexture.getSize().y * iconScale * hoverScale / 2.0f
    );
    
    // 面板淡入淡出动画
    float alphaSpeed = 10.0f;
    targetAlpha = panelOpen ? 1.0f : 0.0f;
    panelAlpha += (targetAlpha - panelAlpha) * alphaSpeed * dt;
    
    // 更新面板透明度
    sf::Color bgColor = panelBgColor;
    bgColor.a = static_cast<sf::Uint8>(panelBgColor.a * panelAlpha);
    panelBackground.setFillColor(bgColor);
    
    sf::Color borderColor = panelBorderColor;
    borderColor.a = static_cast<sf::Uint8>(255 * panelAlpha);
    panelBorder.setOutlineColor(borderColor);
}

// ============================================================================
// 处理输入
// ============================================================================

void StatsPanel::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    // 获取鼠标位置（屏幕坐标）
    sf::Vector2f mousePos = window.mapPixelToCoords(
        sf::Mouse::getPosition(window),
        window.getDefaultView()
    );
    
    // 检查悬停
    iconHovered = isMouseOverIcon(mousePos);
    
    // 点击图标
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            if (isMouseOverIcon(mousePos)) {
                toggle();
            }
            // 点击面板外部关闭（可选）
            // else if (panelOpen && !isMouseOverPanel(mousePos)) {
            //     close();
            // }
        }
    }
    
    // 键盘快捷键（Tab或C键）
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Tab || 
            event.key.code == sf::Keyboard::C) {
            toggle();
        }
    }
}

// ============================================================================
// 渲染
// ============================================================================

void StatsPanel::render(sf::RenderWindow& window) {
    // 总是绘制图标
    window.draw(iconSprite);
    
    // 只有面板打开且有透明度时才绘制
    if (panelAlpha > 0.01f) {
        // 绘制面板背景和边框
        window.draw(panelBorder);
        window.draw(panelBackground);
        
        if (fontLoaded) {
            // 设置文字透明度
            sf::Color titleCol = titleColor;
            titleCol.a = static_cast<sf::Uint8>(255 * panelAlpha);
            titleText.setFillColor(titleCol);
            window.draw(titleText);
            
            // 绘制属性内容
            float startY = panelPosition.y + 50.0f;
            float lineHeight = 30.0f;   // 增加行高
            float labelX = panelPosition.x + 20.0f;
            float valueX = panelPosition.x + 180.0f;  // 数值列右移
            float barX = panelPosition.x + 20.0f;
            float barWidth = panelSize.x - 40.0f;
            float barHeight = 20.0f;    // 增加进度条高度
            
            sf::Text label, value;
            label.setFont(font);
            label.setCharacterSize(18);  // 增大字体
            value.setFont(font);
            value.setCharacterSize(18);  // 增大字体
            
            sf::Color lblCol = labelColor;
            sf::Color valCol = valueColor;
            lblCol.a = valCol.a = static_cast<sf::Uint8>(255 * panelAlpha);
            label.setFillColor(lblCol);
            value.setFillColor(valCol);
            
            float y = startY;
            
            // === 等级 ===
            label.setString(L"等级");
            label.setPosition(labelX, y);
            window.draw(label);
            value.setString("Lv." + std::to_string(cachedLevel));
            value.setFillColor(sf::Color(255, 215, 0, static_cast<sf::Uint8>(255 * panelAlpha)));
            value.setPosition(valueX, y);
            window.draw(value);
            y += lineHeight;
            
            // === 经验条 ===
            drawProgressBar(window, barX, y, barWidth, barHeight, expPercent, 
                           sf::Color(expColor.r, expColor.g, expColor.b, 
                                    static_cast<sf::Uint8>(255 * panelAlpha)), "EXP");
            y += barHeight + 14.0f;
            
            // === 生命条 ===
            drawProgressBar(window, barX, y, barWidth, barHeight, healthPercent,
                           sf::Color(healthColor.r, healthColor.g, healthColor.b,
                                    static_cast<sf::Uint8>(255 * panelAlpha)), "HP");
            y += barHeight + 10.0f;
            
            // === 体力条 ===
            drawProgressBar(window, barX, y, barWidth, barHeight, staminaPercent,
                           sf::Color(staminaColor.r, staminaColor.g, staminaColor.b,
                                    static_cast<sf::Uint8>(255 * panelAlpha)), "SP");
            y += barHeight + 10.0f;
            
            // === 饥饿条 ===
            drawProgressBar(window, barX, y, barWidth, barHeight, hungerPercent,
                           sf::Color(hungerColor.r, hungerColor.g, hungerColor.b,
                                    static_cast<sf::Uint8>(255 * panelAlpha)), L"饱食");
            y += barHeight + 18.0f;
            
            value.setFillColor(valCol);
            
            // === 战斗属性 ===
            // 攻击力
            label.setString(L"攻击力");
            label.setPosition(labelX, y);
            window.draw(label);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(0) << cachedAttack;
            value.setString(ss.str());
            value.setPosition(valueX, y);
            window.draw(value);
            y += lineHeight;
            
            // 防御
            label.setString(L"防御");
            label.setPosition(labelX, y);
            window.draw(label);
            ss.str(""); ss << std::fixed << std::setprecision(0) << cachedDefense;
            value.setString(ss.str());
            value.setPosition(valueX, y);
            window.draw(value);
            y += lineHeight;
            
            // 速度
            label.setString(L"速度");
            label.setPosition(labelX, y);
            window.draw(label);
            ss.str(""); ss << std::fixed << std::setprecision(0) << cachedSpeed;
            value.setString(ss.str());
            value.setPosition(valueX, y);
            window.draw(value);
            y += lineHeight;
            
            // 闪避
            label.setString(L"闪避");
            label.setPosition(labelX, y);
            window.draw(label);
            ss.str(""); ss << std::fixed << std::setprecision(1) << cachedDodge << "%";
            value.setString(ss.str());
            value.setPosition(valueX, y);
            window.draw(value);
            y += lineHeight;
            
            // 幸运
            label.setString(L"幸运");
            label.setPosition(labelX, y);
            window.draw(label);
            ss.str(""); ss << std::fixed << std::setprecision(0) << cachedLuck;
            value.setString(ss.str());
            value.setPosition(valueX, y);
            window.draw(value);
            y += lineHeight + 12.0f;
            
            // === 财产 ===
            label.setString(L"金币");
            label.setPosition(labelX, y);
            window.draw(label);
            value.setFillColor(sf::Color(255, 215, 0, static_cast<sf::Uint8>(255 * panelAlpha)));
            value.setString(std::to_string(cachedGold) + " G");
            value.setPosition(valueX, y);
            window.draw(value);
            y += lineHeight + 12.0f;
            
            value.setFillColor(valCol);
            
            // === 生活技能 ===
            label.setString(L"种植");
            label.setPosition(labelX, y);
            window.draw(label);
            value.setString("Lv." + std::to_string(cachedFarmingLv));
            value.setPosition(valueX, y);
            window.draw(value);
            y += lineHeight;
            
            label.setString(L"渔业");
            label.setPosition(labelX, y);
            window.draw(label);
            value.setString("Lv." + std::to_string(cachedFishingLv));
            value.setPosition(valueX, y);
            window.draw(value);
            y += lineHeight;
            
            label.setString(L"采矿");
            label.setPosition(labelX, y);
            window.draw(label);
            value.setString("Lv." + std::to_string(cachedMiningLv));
            value.setPosition(valueX, y);
            window.draw(value);
        }
    }
}

// ============================================================================
// 绘制进度条
// ============================================================================

void StatsPanel::drawProgressBar(sf::RenderWindow& window, float x, float y,
                                  float width, float height, float percent,
                                  const sf::Color& fillColor, const std::string& label) {
    // 背景
    sf::RectangleShape bg(sf::Vector2f(width, height));
    bg.setPosition(x, y);
    bg.setFillColor(sf::Color(40, 40, 40, static_cast<sf::Uint8>(200 * panelAlpha)));
    bg.setOutlineThickness(1.0f);
    bg.setOutlineColor(sf::Color(80, 80, 80, static_cast<sf::Uint8>(255 * panelAlpha)));
    window.draw(bg);
    
    // 填充
    if (percent > 0.0f) {
        sf::RectangleShape fill(sf::Vector2f(width * percent, height));
        fill.setPosition(x, y);
        fill.setFillColor(fillColor);
        window.draw(fill);
    }
    
    // 标签
    if (fontLoaded && !label.empty()) {
        sf::Text text;
        text.setFont(font);
        text.setString(label);
        text.setCharacterSize(12);
        text.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(255 * panelAlpha)));
        text.setPosition(x + 5.0f, y + 1.0f);
        window.draw(text);
    }
}

// 宽字符版本重载
void StatsPanel::drawProgressBar(sf::RenderWindow& window, float x, float y,
                                  float width, float height, float percent,
                                  const sf::Color& fillColor, const std::wstring& label) {
    // 背景
    sf::RectangleShape bg(sf::Vector2f(width, height));
    bg.setPosition(x, y);
    bg.setFillColor(sf::Color(40, 40, 40, static_cast<sf::Uint8>(200 * panelAlpha)));
    bg.setOutlineThickness(1.0f);
    bg.setOutlineColor(sf::Color(80, 80, 80, static_cast<sf::Uint8>(255 * panelAlpha)));
    window.draw(bg);
    
    // 填充
    if (percent > 0.0f) {
        sf::RectangleShape fill(sf::Vector2f(width * percent, height));
        fill.setPosition(x, y);
        fill.setFillColor(fillColor);
        window.draw(fill);
    }
    
    // 标签
    if (fontLoaded && !label.empty()) {
        sf::Text text;
        text.setFont(font);
        text.setString(label);
        text.setCharacterSize(12);
        text.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(255 * panelAlpha)));
        text.setPosition(x + 5.0f, y + 1.0f);
        window.draw(text);
    }
}

// ============================================================================
// 更新属性数据
// ============================================================================

void StatsPanel::updateStats(const PlayerStats& stats) {
    // 更新进度条
    healthPercent = stats.getHealthPercent();
    staminaPercent = stats.getStaminaPercent();
    hungerPercent = stats.getHungerPercent();
    expPercent = stats.getExpPercent();
    
    // 更新缓存值
    cachedLevel = stats.getLevel();
    cachedGold = stats.getGold();
    cachedAttack = stats.getAttack();
    cachedDefense = stats.getDefense();
    cachedSpeed = stats.getSpeed();
    cachedDodge = stats.getDodge();
    cachedLuck = stats.getLuck();
    cachedDamageBonus = stats.getDamageBonus();
    cachedDodgeReduction = stats.getDodgeReduction();
    cachedFarmingLv = stats.getSkillLevel(LifeSkill::Farming);
    cachedFishingLv = stats.getSkillLevel(LifeSkill::Fishing);
    cachedMiningLv = stats.getSkillLevel(LifeSkill::Mining);
}

// ============================================================================
// 设置样式
// ============================================================================

void StatsPanel::setPanelColor(const sf::Color& bg, const sf::Color& border) {
    panelBgColor = bg;
    panelBorderColor = border;
    panelBackground.setFillColor(panelBgColor);
    panelBorder.setOutlineColor(panelBorderColor);
}

void StatsPanel::setTextColor(const sf::Color& title, const sf::Color& value) {
    titleColor = title;
    valueColor = value;
    if (fontLoaded) {
        titleText.setFillColor(titleColor);
    }
}

// ============================================================================
// 碰撞检测
// ============================================================================

bool StatsPanel::isMouseOverIcon(const sf::Vector2f& mousePos) const {
    return iconSprite.getGlobalBounds().contains(mousePos);
}

bool StatsPanel::isMouseOverPanel(const sf::Vector2f& mousePos) const {
    return panelBackground.getGlobalBounds().contains(mousePos);
}