#pragma once
#include "Item.h"
#include "CategoryInventory.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include <functional>
#include <cmath>
#include <string>

// ============================================================================
// 合成与锻造系统 (Crafting & Forging System)
// ============================================================================

// 武器锻造概率结构
struct ForgeProbability {
    float white;    // 普通
    float green;    // 优秀
    float blue;     // 稀有
    float purple;   // 史诗
    float orange;   // 传说
    float red;      // 神话
};

// 配方材料
struct RecipeIngredient {
    std::string itemId;
    int count;
    
    RecipeIngredient(const std::string& id = "", int c = 1)
        : itemId(id), count(c) {}
};

// 合成配方
struct CraftingRecipe {
    std::string id;
    std::string name;
    std::string description;
    std::vector<RecipeIngredient> ingredients;
    std::string resultItemId;
    int resultCount;
    bool isEquipment;
    
    // 批量合成设置
    bool allowBatchCraft;
    int maxBatchCount;
    
    // 武器锻造设置
    bool isWeaponForge;
    int maxWeaponSouls; // 最大可添加武器魂数量 (Excel: 30)
    
    CraftingRecipe()
        : resultCount(1)
        , isEquipment(false)
        , allowBatchCraft(false)
        , maxBatchCount(1)
        , isWeaponForge(false)
        , maxWeaponSouls(0) {}
};

class CraftingManager {
public:
    static CraftingManager& getInstance();
    void initialize();
    const std::vector<CraftingRecipe>& getAllRecipes() const { return recipes; }
    
    // 注册配方
    void registerRecipe(const CraftingRecipe& recipe);

    // 检查材料是否足够 (multiplier为批量倍数)
    bool canCraft(const CraftingRecipe& recipe, CategoryInventory* inventory, int multiplier = 1, int extraSoulCount = 0) const;
    
    // 执行合成/锻造
    // multiplier: 批量数量
    // soulCount: 投入的武器魂数量
    bool craft(const CraftingRecipe& recipe, CategoryInventory* inventory, int multiplier = 1, int soulCount = 0);

    // 计算锻造概率 (根据Excel公式模拟)
    ForgeProbability calculateForgeProb(int soulCount);

private:
    CraftingManager() = default;
    std::vector<CraftingRecipe> recipes;
    bool initialized = false;
};

// ============================================================================
// 改进后的工作台 UI
// ============================================================================

class CraftingPanel {
public:
    // 定义回调类型 (保持与 GameState.cpp 兼容)
    using CraftCallback = std::function<void(const CraftingRecipe&)>;
    using CraftSuccessCallback = std::function<void(const std::string& itemId, int count)>;
    
    CraftingPanel();
    bool init(const std::string& iconPath);
    void setInventory(CategoryInventory* inv) { inventory = inv; }
    void setIconPosition(float x, float y);
    void update(float dt);
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void render(sf::RenderWindow& window);
    
    void open();
    void close();
    void toggle();
    bool isOpen() const { return panelOpen; }

    // 回调设置函数 (恢复以修复 GameState.cpp 编译错误)
    void setOnCraft(CraftCallback cb) { onCraft = cb; }
    void setOnCraftSuccess(CraftSuccessCallback cb) { onCraftSuccess = cb; }

private:
    void renderRecipeList(sf::RenderWindow& window);
    void renderRecipeDetail(sf::RenderWindow& window);
    // 渲染滑动条组件
    void renderSlider(sf::RenderWindow& window, const sf::Vector2f& pos, float width, int min, int max, int& current, const std::string& label);
    // 渲染概率表
    void renderProbabilities(sf::RenderWindow& window, const sf::Vector2f& pos, const ForgeProbability& probs);

    int getRecipeAtPosition(const sf::Vector2f& pos) const;

private:
    CategoryInventory* inventory;
    bool panelOpen;
    
    // 选中状态
    int selectedRecipe;
    int hoveredRecipe;
    int scrollOffset;
    
    // 动态控制参数
    int currentBatchAmount; // 当前选择的批量合成数量
    int currentSoulAmount;  // 当前选择投入的武器魂数量
    
    // UI资源
    sf::Texture iconTexture;
    sf::Sprite iconSprite;
    sf::Vector2f iconPosition;
    sf::Font font;
    bool fontLoaded;
    
    // 布局常量
    sf::Vector2f panelPosition;
    sf::Vector2f panelSize;
    
    // 拖拽滑动条状态
    bool isDraggingBatchSlider;
    bool isDraggingSoulSlider;

    // 回调对象
    CraftCallback onCraft;
    CraftSuccessCallback onCraftSuccess;
};