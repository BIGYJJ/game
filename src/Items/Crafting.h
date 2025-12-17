#pragma once
#include "Item.h"
#include "CategoryInventory.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include <functional>

// ============================================================================
// 合成系统 (Crafting System)
// 
// 工作台/合成界面
// ============================================================================
// 
// 【合成配方】
//   - 每个配方由多个输入材料和一个输出物品组成
//   - 输入材料可以是任意类型的物品
//   - 输出物品可以是装备、消耗品或材料
//
// 【示例配方】
//   - 1 石头 + 2 树枝 = 1 木斧（无视防御）
//   - 3 木材 = 1 木盾
//   - 2 木材 + 1 石头 = 1 木剑
//
// ============================================================================

// 配方材料
struct RecipeIngredient {
    std::string itemId;     // 物品ID
    int count;              // 所需数量
    
    RecipeIngredient(const std::string& id = "", int c = 1)
        : itemId(id), count(c) {}
};

// 合成配方
struct CraftingRecipe {
    std::string id;                             // 配方ID
    std::string name;                           // 配方名称
    std::string description;                    // 配方描述
    std::vector<RecipeIngredient> ingredients;  // 所需材料
    std::string resultItemId;                   // 产出物品ID
    int resultCount;                            // 产出数量
    bool isEquipment;                           // 是否为装备配方
    
    CraftingRecipe()
        : resultCount(1)
        , isEquipment(false) {}
};

// ============================================================================
// 合成管理器（单例）
// ============================================================================

class CraftingManager {
public:
    static CraftingManager& getInstance();
    
    // 初始化配方
    void initialize();
    
    // 获取配方
    const CraftingRecipe* getRecipe(const std::string& recipeId) const;
    
    // 获取所有配方
    const std::vector<CraftingRecipe>& getAllRecipes() const { return recipes; }
    
    // 注册配方
    void registerRecipe(const CraftingRecipe& recipe);
    
    // 检查是否可以合成（材料是否足够）
    bool canCraft(const CraftingRecipe& recipe, CategoryInventory* inventory) const;
    
    // 执行合成
    bool craft(const CraftingRecipe& recipe, CategoryInventory* inventory);

private:
    CraftingManager() = default;
    std::vector<CraftingRecipe> recipes;
    bool initialized = false;
};

// ============================================================================
// 工作箱面板 UI
// ============================================================================

class CraftingPanel {
public:
    using CraftCallback = std::function<void(const CraftingRecipe&)>;
    using CraftSuccessCallback = std::function<void(const std::string& itemId, int count)>;
    
    CraftingPanel();
    
    // 初始化
    bool init(const std::string& iconPath);
    bool loadFont(const std::string& fontPath);
    
    // 设置关联的背包
    void setInventory(CategoryInventory* inv) { inventory = inv; }
    
    // 设置图标位置
    void setIconPosition(float x, float y);
    
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
    
    // 回调
    void setOnCraft(CraftCallback cb) { onCraft = cb; }
    void setOnCraftSuccess(CraftSuccessCallback cb) { onCraftSuccess = cb; }

private:
    void renderRecipeList(sf::RenderWindow& window);
    void renderRecipeDetail(sf::RenderWindow& window);
    void renderIngredient(sf::RenderWindow& window, const RecipeIngredient& ing, 
                         const sf::Vector2f& pos, bool hasEnough);
    int getRecipeAtPosition(const sf::Vector2f& pos) const;

private:
    CategoryInventory* inventory;
    
    // UI状态
    bool panelOpen;
    int selectedRecipe;
    int hoveredRecipe;
    int scrollOffset;
    
    // 图标
    sf::Texture iconTexture;
    sf::Sprite iconSprite;
    sf::Vector2f iconPosition;
    bool iconLoaded;
    
    // 面板
    sf::Vector2f panelPosition;
    sf::Vector2f panelSize;
    
    // 字体
    sf::Font font;
    bool fontLoaded;
    
    // 回调
    CraftCallback onCraft;
    CraftSuccessCallback onCraftSuccess;
    
    // 常量
    static constexpr float RECIPE_HEIGHT = 60.0f;
    static constexpr float LIST_WIDTH = 200.0f;
    
    // 颜色
    static const sf::Color BG_COLOR;
    static const sf::Color SLOT_COLOR;
    static const sf::Color SLOT_HOVER_COLOR;
    static const sf::Color SLOT_SELECTED_COLOR;
    static const sf::Color CRAFTABLE_COLOR;
    static const sf::Color NOT_CRAFTABLE_COLOR;
    static const sf::Color BORDER_COLOR;
};
