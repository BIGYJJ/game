#pragma once
#include "Item.h"
#include <array>
#include <functional>

// ============================================================================
// 背包系统 (Inventory System)
// 
// 设计规则：
// ============================================================================
// 
// 【背包容量】
//   - 总容量: 100 格
//   - 每页显示: 30 格 (6列 x 5行)
//   - 总页数: 4 页 (向上取整)
//
// 【物品堆叠规则】
//   - 相同物品可以堆叠
//   - 最大堆叠数由物品定义决定（材料99，消耗品20等）
//   - 添加物品时自动寻找可堆叠的格子
//
// 【操作功能】
//   - 添加物品: addItem(itemId, count)
//   - 移除物品: removeItem(itemId, count)
//   - 使用物品: useItem(slotIndex)
//   - 丢弃物品: dropItem(slotIndex, count)
//   - 交换位置: swapSlots(index1, index2)
//   - 整理背包: sortInventory()
//
// 【回调事件】
//   - onItemAdded:   物品添加时触发
//   - onItemRemoved: 物品移除时触发
//   - onItemUsed:    物品使用时触发
//   - onInventoryChanged: 背包变化时触发
//
// ============================================================================

// 背包常量
constexpr int INVENTORY_TOTAL_SLOTS = 100;      // 总格子数
constexpr int INVENTORY_SLOTS_PER_PAGE = 30;    // 每页格子数
constexpr int INVENTORY_COLUMNS = 6;            // 每行列数
constexpr int INVENTORY_ROWS = 5;               // 每页行数

class Inventory {
public:
    // 回调类型定义
    using ItemCallback = std::function<void(const ItemStack&, int slotIndex)>;
    using UseItemCallback = std::function<bool(const ItemStack&, const ItemData*)>;
    
    Inventory();
    
    // ========================================
    // 物品操作
    // ========================================
    
    // 添加物品到背包（自动堆叠和寻找空位）
    // 返回实际添加的数量
    int addItem(const std::string& itemId, int count = 1);
    
    // 添加ItemStack到背包
    int addItemStack(const ItemStack& stack);
    
    // 从背包移除指定数量的物品
    // 返回实际移除的数量
    int removeItem(const std::string& itemId, int count = 1);
    
    // 从指定格子移除物品
    int removeItemFromSlot(int slotIndex, int count = 1);
    
    // 使用指定格子的物品
    // 返回是否成功使用
    bool useItem(int slotIndex);
    
    // 丢弃指定格子的物品
    // 返回丢弃的ItemStack（用于生成掉落物）
    ItemStack dropItem(int slotIndex, int count = -1);  // -1表示全部丢弃
    
    // 交换两个格子的物品
    void swapSlots(int index1, int index2);
    
    // 整理背包（堆叠相同物品，移除空隙）
    void sortInventory();
    
    // ========================================
    // 查询功能
    // ========================================
    
    // 获取指定格子的物品
    const ItemStack& getSlot(int index) const;
    ItemStack& getSlot(int index);
    
    // 检查是否拥有指定物品
    bool hasItem(const std::string& itemId, int count = 1) const;
    
    // 获取指定物品的总数量
    int getItemCount(const std::string& itemId) const;
    
    // 获取空格子数量
    int getEmptySlotCount() const;
    
    // 检查背包是否已满
    bool isFull() const;
    
    // 获取总格子数
    int getTotalSlots() const { return INVENTORY_TOTAL_SLOTS; }
    
    // 获取每页格子数
    int getSlotsPerPage() const { return INVENTORY_SLOTS_PER_PAGE; }
    
    // 获取总页数
    int getTotalPages() const;
    
    // ========================================
    // 回调设置
    // ========================================
    
    void setOnItemAdded(ItemCallback cb) { onItemAdded = cb; }
    void setOnItemRemoved(ItemCallback cb) { onItemRemoved = cb; }
    void setOnItemUsed(ItemCallback cb) { onItemUsed = cb; }
    void setOnInventoryChanged(std::function<void()> cb) { onInventoryChanged = cb; }
    void setOnUseItem(UseItemCallback cb) { onUseItemCallback = cb; }

private:
    // 查找可以堆叠的格子
    int findStackableSlot(const std::string& itemId) const;
    
    // 查找空格子
    int findEmptySlot() const;
    
    // 触发回调
    void notifyItemAdded(const ItemStack& item, int slotIndex);
    void notifyItemRemoved(const ItemStack& item, int slotIndex);
    void notifyItemUsed(const ItemStack& item, int slotIndex);
    void notifyInventoryChanged();

private:
    std::array<ItemStack, INVENTORY_TOTAL_SLOTS> slots;
    
    // 回调函数
    ItemCallback onItemAdded;
    ItemCallback onItemRemoved;
    ItemCallback onItemUsed;
    std::function<void()> onInventoryChanged;
    UseItemCallback onUseItemCallback;
};
