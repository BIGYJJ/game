#pragma once
#include "Item.h"
#include "Inventory.h"
#include <array>
#include <functional>

// ============================================================================
// 分类背包系统 (Category Inventory System)
// 
// 设计规则：
// ============================================================================
// 
// 【三大分类】
//   - Materials:    材料栏（木材、石头、种子等）- 100格
//   - Consumables:  消耗品栏（苹果、药水、种子等）- 100格  
//   - Equipment:    装备栏（武器、防具等）- 100格
//
// 【交互规则】
//   材料：
//     - 右键菜单：销毁、卖出
//   消耗品：
//     - 双击：直接使用
//     - 右键菜单：使用、销毁、卖出
//   种子（特殊消耗品）：
//     - 右键菜单：种下、销毁、卖出
//   装备：
//     - 双击：装备
//     - 右键菜单：装备、销毁、卖出
//
// ============================================================================

// 背包分类枚举
enum class InventoryCategory {
    Materials,      // 材料
    Consumables,    // 消耗品
    Equipment,      // 装备
    Count           // 分类数量
};

// 每个分类的容量
constexpr int CATEGORY_SLOTS = 100;
constexpr int CATEGORY_SLOTS_PER_PAGE = 30;
constexpr int CATEGORY_COLUMNS = 6;
constexpr int CATEGORY_ROWS = 5;

// 右键菜单选项
enum class ContextMenuOption {
    None,
    Use,        // 使用（消耗品）
    Equip,      // 装备
    Plant,      // 种下（种子）
    Destroy,    // 销毁
    Sell        // 卖出
};

class CategoryInventory {
public:
    // 回调类型定义
    using ItemCallback = std::function<void(const ItemStack&, int slotIndex, InventoryCategory)>;
    using UseItemCallback = std::function<bool(const ItemStack&, const ItemData*)>;
    using SellItemCallback = std::function<void(const ItemStack&, int goldValue)>;
    using PlantSeedCallback = std::function<bool(const ItemStack&)>;
    using EquipItemCallback = std::function<bool(const ItemStack&)>;  // Returns old equipped item or empty
    
    CategoryInventory();
    
    // ========================================
    // 物品操作
    // ========================================
    
    // 添加物品（自动分类到对应栏位）
    int addItem(const std::string& itemId, int count = 1);
    int addItemStack(const ItemStack& stack);
    
    // 从指定分类移除物品
    int removeItem(const std::string& itemId, int count = 1);
    int removeItemFromSlot(InventoryCategory category, int slotIndex, int count = 1);
    
    // 使用物品
    bool useItem(InventoryCategory category, int slotIndex);
    
    // 装备物品
    bool equipItem(int slotIndex);
    
    // 种下种子
    bool plantSeed(int slotIndex);
    
    // 销毁物品
    bool destroyItem(InventoryCategory category, int slotIndex, int count = -1);
    
    // 卖出物品
    int sellItem(InventoryCategory category, int slotIndex, int count = -1);
    
    // 丢弃物品
    ItemStack dropItem(InventoryCategory category, int slotIndex, int count = -1);
    
    // 交换位置（同分类内）
    void swapSlots(InventoryCategory category, int index1, int index2);
    
    // 整理指定分类
    void sortCategory(InventoryCategory category);
    
    // 整理所有分类
    void sortAll();
    
    // ========================================
    // 查询功能
    // ========================================
    
    // 获取指定分类的格子
    const ItemStack& getSlot(InventoryCategory category, int index) const;
    ItemStack& getSlot(InventoryCategory category, int index);
    
    // 检查是否拥有物品
    bool hasItem(const std::string& itemId, int count = 1) const;
    
    // 获取物品总数量
    int getItemCount(const std::string& itemId) const;
    
    // 获取分类的空格子数
    int getEmptySlotCount(InventoryCategory category) const;
    
    // 检查分类是否已满
    bool isCategoryFull(InventoryCategory category) const;
    
    // 获取分类页数
    int getCategoryPages(InventoryCategory category) const;
    
    // 获取分类名称
    static std::string getCategoryName(InventoryCategory category);
    
    // 根据物品类型确定分类
    static InventoryCategory getItemCategory(ItemType type);
    
    // 判断物品是否为种子
    static bool isSeed(const std::string& itemId);
    
    // 获取物品的右键菜单选项
    static std::vector<ContextMenuOption> getContextMenuOptions(const ItemData* data);
    
    // ========================================
    // 回调设置
    // ========================================
    
    void setOnItemAdded(ItemCallback cb) { onItemAdded = cb; }
    void setOnItemRemoved(ItemCallback cb) { onItemRemoved = cb; }
    void setOnItemUsed(ItemCallback cb) { onItemUsed = cb; }
    void setOnInventoryChanged(std::function<void()> cb) { onInventoryChanged = cb; }
    void setOnUseItem(UseItemCallback cb) { onUseItemCallback = cb; }
    void setOnSellItem(SellItemCallback cb) { onSellItemCallback = cb; }
    void setOnPlantSeed(PlantSeedCallback cb) { onPlantSeedCallback = cb; }
    void setOnEquipItem(EquipItemCallback cb) { onEquipItemCallback = cb; }

private:
    // 在指定分类中查找可堆叠的格子
    int findStackableSlot(InventoryCategory category, const std::string& itemId) const;
    
    // 在指定分类中查找空格子
    int findEmptySlot(InventoryCategory category) const;
    
    // 通知回调
    void notifyItemAdded(const ItemStack& item, int slotIndex, InventoryCategory category);
    void notifyItemRemoved(const ItemStack& item, int slotIndex, InventoryCategory category);
    void notifyItemUsed(const ItemStack& item, int slotIndex, InventoryCategory category);
    void notifyInventoryChanged();

private:
    // 三个分类的背包格子
    std::array<ItemStack, CATEGORY_SLOTS> materialSlots;
    std::array<ItemStack, CATEGORY_SLOTS> consumableSlots;
    std::array<ItemStack, CATEGORY_SLOTS> equipmentSlots;
    
    // 回调函数
    ItemCallback onItemAdded;
    ItemCallback onItemRemoved;
    ItemCallback onItemUsed;
    std::function<void()> onInventoryChanged;
    UseItemCallback onUseItemCallback;
    SellItemCallback onSellItemCallback;
    PlantSeedCallback onPlantSeedCallback;
    EquipItemCallback onEquipItemCallback;
};

// ============================================================================
// 右键菜单选项名称
// ============================================================================
inline std::string getContextMenuOptionName(ContextMenuOption option) {
    switch (option) {
        case ContextMenuOption::Use:     return "使用";
        case ContextMenuOption::Equip:   return "装备";
        case ContextMenuOption::Plant:   return "种下";
        case ContextMenuOption::Destroy: return "销毁";
        case ContextMenuOption::Sell:    return "卖出";
        default:                         return "";
    }
}
