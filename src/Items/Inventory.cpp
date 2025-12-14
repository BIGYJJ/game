#include "Inventory.h"
#include <algorithm>
#include <iostream>

Inventory::Inventory() {
    // 所有格子初始化为空
    for (auto& slot : slots) {
        slot.clear();
    }
}

// ============================================================================
// 物品操作
// ============================================================================

int Inventory::addItem(const std::string& itemId, int count) {
    if (itemId.empty() || count <= 0) return 0;
    
    const ItemData* data = ItemDatabase::getInstance().getItemData(itemId);
    if (!data) {
        std::cout << "[Inventory] Unknown item: " << itemId << std::endl;
        return 0;
    }
    
    int remaining = count;
    int totalAdded = 0;
    
    // 首先尝试堆叠到现有物品
    while (remaining > 0) {
        int slotIndex = findStackableSlot(itemId);
        if (slotIndex < 0) break;
        
        ItemStack& slot = slots[slotIndex];
        int canAdd = data->maxStack - slot.count;
        int toAdd = std::min(canAdd, remaining);
        
        slot.count += toAdd;
        remaining -= toAdd;
        totalAdded += toAdd;
        
        notifyItemAdded(ItemStack(itemId, toAdd), slotIndex);
    }
    
    // 然后放入空格子
    while (remaining > 0) {
        int slotIndex = findEmptySlot();
        if (slotIndex < 0) break;  // 背包已满
        
        int toAdd = std::min(data->maxStack, remaining);
        slots[slotIndex] = ItemStack(itemId, toAdd);
        remaining -= toAdd;
        totalAdded += toAdd;
        
        notifyItemAdded(ItemStack(itemId, toAdd), slotIndex);
    }
    
    if (totalAdded > 0) {
        notifyInventoryChanged();
        std::cout << "[Inventory] Added " << totalAdded << "x " << data->name << std::endl;
    }
    
    if (remaining > 0) {
        std::cout << "[Inventory] Could not add " << remaining << "x " << itemId 
                  << " (inventory full)" << std::endl;
    }
    
    return totalAdded;
}

int Inventory::addItemStack(const ItemStack& stack) {
    return addItem(stack.itemId, stack.count);
}

int Inventory::removeItem(const std::string& itemId, int count) {
    if (itemId.empty() || count <= 0) return 0;
    
    int remaining = count;
    int totalRemoved = 0;
    
    // 从后往前移除（保持前面的物品）
    for (int i = INVENTORY_TOTAL_SLOTS - 1; i >= 0 && remaining > 0; i--) {
        if (slots[i].itemId == itemId) {
            int toRemove = std::min(slots[i].count, remaining);
            slots[i].count -= toRemove;
            remaining -= toRemove;
            totalRemoved += toRemove;
            
            notifyItemRemoved(ItemStack(itemId, toRemove), i);
            
            if (slots[i].count <= 0) {
                slots[i].clear();
            }
        }
    }
    
    if (totalRemoved > 0) {
        notifyInventoryChanged();
    }
    
    return totalRemoved;
}

int Inventory::removeItemFromSlot(int slotIndex, int count) {
    if (slotIndex < 0 || slotIndex >= INVENTORY_TOTAL_SLOTS) return 0;
    if (slots[slotIndex].isEmpty()) return 0;
    
    int toRemove = (count <= 0) ? slots[slotIndex].count : std::min(count, slots[slotIndex].count);
    std::string itemId = slots[slotIndex].itemId;
    
    slots[slotIndex].count -= toRemove;
    
    notifyItemRemoved(ItemStack(itemId, toRemove), slotIndex);
    
    if (slots[slotIndex].count <= 0) {
        slots[slotIndex].clear();
    }
    
    notifyInventoryChanged();
    return toRemove;
}

bool Inventory::useItem(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= INVENTORY_TOTAL_SLOTS) return false;
    if (slots[slotIndex].isEmpty()) return false;
    
    const ItemData* data = ItemDatabase::getInstance().getItemData(slots[slotIndex].itemId);
    if (!data) return false;
    
    // 只有消耗品可以使用
    if (data->type != ItemType::Consumable) {
        std::cout << "[Inventory] Cannot use non-consumable item: " << data->name << std::endl;
        return false;
    }
    
    // 调用使用回调
    if (onUseItemCallback) {
        if (!onUseItemCallback(slots[slotIndex], data)) {
            return false;
        }
    }
    
    // 消耗物品
    ItemStack usedItem = slots[slotIndex];
    usedItem.count = 1;
    
    slots[slotIndex].count--;
    if (slots[slotIndex].count <= 0) {
        slots[slotIndex].clear();
    }
    
    notifyItemUsed(usedItem, slotIndex);
    notifyInventoryChanged();
    
    std::cout << "[Inventory] Used: " << data->name << std::endl;
    return true;
}

ItemStack Inventory::dropItem(int slotIndex, int count) {
    if (slotIndex < 0 || slotIndex >= INVENTORY_TOTAL_SLOTS) return ItemStack();
    if (slots[slotIndex].isEmpty()) return ItemStack();
    
    const ItemData* data = ItemDatabase::getInstance().getItemData(slots[slotIndex].itemId);
    
    // 任务物品不可丢弃
    if (data && data->type == ItemType::Quest) {
        std::cout << "[Inventory] Cannot drop quest item: " << data->name << std::endl;
        return ItemStack();
    }
    
    int toDrop = (count < 0) ? slots[slotIndex].count : std::min(count, slots[slotIndex].count);
    
    ItemStack dropped(slots[slotIndex].itemId, toDrop);
    
    slots[slotIndex].count -= toDrop;
    if (slots[slotIndex].count <= 0) {
        slots[slotIndex].clear();
    }
    
    notifyItemRemoved(dropped, slotIndex);
    notifyInventoryChanged();
    
    if (data) {
        std::cout << "[Inventory] Dropped " << toDrop << "x " << data->name << std::endl;
    }
    
    return dropped;
}

void Inventory::swapSlots(int index1, int index2) {
    if (index1 < 0 || index1 >= INVENTORY_TOTAL_SLOTS) return;
    if (index2 < 0 || index2 >= INVENTORY_TOTAL_SLOTS) return;
    if (index1 == index2) return;
    
    std::swap(slots[index1], slots[index2]);
    notifyInventoryChanged();
}

void Inventory::sortInventory() {
    // 先收集所有非空物品
    std::vector<ItemStack> items;
    for (auto& slot : slots) {
        if (!slot.isEmpty()) {
            items.push_back(slot);
            slot.clear();
        }
    }
    
    // 按类型和ID排序
    std::sort(items.begin(), items.end(), [](const ItemStack& a, const ItemStack& b) {
        const ItemData* dataA = ItemDatabase::getInstance().getItemData(a.itemId);
        const ItemData* dataB = ItemDatabase::getInstance().getItemData(b.itemId);
        
        if (dataA && dataB) {
            if (dataA->type != dataB->type) {
                return static_cast<int>(dataA->type) < static_cast<int>(dataB->type);
            }
        }
        
        return a.itemId < b.itemId;
    });
    
    // 合并相同物品
    for (const auto& item : items) {
        addItem(item.itemId, item.count);
    }
    
    notifyInventoryChanged();
    std::cout << "[Inventory] Sorted and organized" << std::endl;
}

// ============================================================================
// 查询功能
// ============================================================================

const ItemStack& Inventory::getSlot(int index) const {
    static ItemStack emptySlot;
    if (index < 0 || index >= INVENTORY_TOTAL_SLOTS) return emptySlot;
    return slots[index];
}

ItemStack& Inventory::getSlot(int index) {
    static ItemStack emptySlot;
    if (index < 0 || index >= INVENTORY_TOTAL_SLOTS) return emptySlot;
    return slots[index];
}

bool Inventory::hasItem(const std::string& itemId, int count) const {
    return getItemCount(itemId) >= count;
}

int Inventory::getItemCount(const std::string& itemId) const {
    int total = 0;
    for (const auto& slot : slots) {
        if (slot.itemId == itemId) {
            total += slot.count;
        }
    }
    return total;
}

int Inventory::getEmptySlotCount() const {
    int count = 0;
    for (const auto& slot : slots) {
        if (slot.isEmpty()) count++;
    }
    return count;
}

bool Inventory::isFull() const {
    return getEmptySlotCount() == 0;
}

int Inventory::getTotalPages() const {
    return (INVENTORY_TOTAL_SLOTS + INVENTORY_SLOTS_PER_PAGE - 1) / INVENTORY_SLOTS_PER_PAGE;
}

// ============================================================================
// 私有方法
// ============================================================================

int Inventory::findStackableSlot(const std::string& itemId) const {
    const ItemData* data = ItemDatabase::getInstance().getItemData(itemId);
    if (!data) return -1;
    
    for (int i = 0; i < INVENTORY_TOTAL_SLOTS; i++) {
        if (slots[i].itemId == itemId && slots[i].count < data->maxStack) {
            return i;
        }
    }
    return -1;
}

int Inventory::findEmptySlot() const {
    for (int i = 0; i < INVENTORY_TOTAL_SLOTS; i++) {
        if (slots[i].isEmpty()) {
            return i;
        }
    }
    return -1;
}

void Inventory::notifyItemAdded(const ItemStack& item, int slotIndex) {
    if (onItemAdded) onItemAdded(item, slotIndex);
}

void Inventory::notifyItemRemoved(const ItemStack& item, int slotIndex) {
    if (onItemRemoved) onItemRemoved(item, slotIndex);
}

void Inventory::notifyItemUsed(const ItemStack& item, int slotIndex) {
    if (onItemUsed) onItemUsed(item, slotIndex);
}

void Inventory::notifyInventoryChanged() {
    if (onInventoryChanged) onInventoryChanged();
}
