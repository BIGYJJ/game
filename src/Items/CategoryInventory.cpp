#include "CategoryInventory.h"
#include <algorithm>
#include <iostream>

CategoryInventory::CategoryInventory() {
    // 初始化所有格子为空
    for (auto& slot : materialSlots) slot.clear();
    for (auto& slot : consumableSlots) slot.clear();
    for (auto& slot : equipmentSlots) slot.clear();
}

// ============================================================================
// 辅助方法 - 获取分类对应的数组
// ============================================================================

std::array<ItemStack, CATEGORY_SLOTS>& getCategoryArray(
    CategoryInventory* inv,
    std::array<ItemStack, CATEGORY_SLOTS>& materials,
    std::array<ItemStack, CATEGORY_SLOTS>& consumables,
    std::array<ItemStack, CATEGORY_SLOTS>& equipment,
    InventoryCategory category) 
{
    switch (category) {
        case InventoryCategory::Materials:   return materials;
        case InventoryCategory::Consumables: return consumables;
        case InventoryCategory::Equipment:   return equipment;
        default:                             return materials;
    }
}

// ============================================================================
// 物品操作
// ============================================================================

int CategoryInventory::addItem(const std::string& itemId, int count) {
    if (itemId.empty() || count <= 0) return 0;
    
    const ItemData* data = ItemDatabase::getInstance().getItemData(itemId);
    if (!data) {
        std::cout << "[CategoryInventory] Unknown item : " << itemId << std::endl;
        return 0;
    }
    
    // 确定物品分类
    InventoryCategory category = getItemCategory(data->type);
    
    // 特殊处理：种子归类到消耗品
    if (isSeed(itemId)) {
        category = InventoryCategory::Consumables;
    }
    
    // 获取对应分类的数组
    std::array<ItemStack, CATEGORY_SLOTS>* slots = nullptr;
    switch (category) {
        case InventoryCategory::Materials:   slots = &materialSlots; break;
        case InventoryCategory::Consumables: slots = &consumableSlots; break;
        case InventoryCategory::Equipment:   slots = &equipmentSlots; break;
        default: return 0;
    }
    
    int remaining = count;
    int totalAdded = 0;
    
    // 首先尝试堆叠到现有物品
    while (remaining > 0) {
        int slotIndex = findStackableSlot(category, itemId);
        if (slotIndex < 0) break;
        
        ItemStack& slot = (*slots)[slotIndex];
        int canAdd = data->maxStack - slot.count;
        int toAdd = std::min(canAdd, remaining);
        
        slot.count += toAdd;
        remaining -= toAdd;
        totalAdded += toAdd;
        
        notifyItemAdded(ItemStack(itemId, toAdd), slotIndex, category);
    }
    
    // 然后放入空格子
    while (remaining > 0) {
        int slotIndex = findEmptySlot(category);
        if (slotIndex < 0) break;
        
        int toAdd = std::min(data->maxStack, remaining);
        (*slots)[slotIndex] = ItemStack(itemId, toAdd);
        remaining -= toAdd;
        totalAdded += toAdd;
        
        notifyItemAdded(ItemStack(itemId, toAdd), slotIndex, category);
    }
    
    if (totalAdded > 0) {
        notifyInventoryChanged();
        std::cout << "[CategoryInventory] Added " << totalAdded << "x " << data->name 
                  << " to " << getCategoryName(category) << std::endl;
    }
    
    if (remaining > 0) {
        std::cout << "[CategoryInventory] Could not add " << remaining << "x " << itemId 
                  << " (" << getCategoryName(category) << " full)" << std::endl;
    }
    
    return totalAdded;
}

int CategoryInventory::addItemStack(const ItemStack& stack) {
    return addItem(stack.itemId, stack.count);
}

int CategoryInventory::removeItem(const std::string& itemId, int count) {
    if (itemId.empty() || count <= 0) return 0;
    
    const ItemData* data = ItemDatabase::getInstance().getItemData(itemId);
    if (!data) return 0;
    
    InventoryCategory category = getItemCategory(data->type);
    if (isSeed(itemId)) category = InventoryCategory::Consumables;
    
    std::array<ItemStack, CATEGORY_SLOTS>* slots = nullptr;
    switch (category) {
        case InventoryCategory::Materials:   slots = &materialSlots; break;
        case InventoryCategory::Consumables: slots = &consumableSlots; break;
        case InventoryCategory::Equipment:   slots = &equipmentSlots; break;
        default: return 0;
    }
    
    int remaining = count;
    int totalRemoved = 0;
    
    for (int i = CATEGORY_SLOTS - 1; i >= 0 && remaining > 0; i--) {
        if ((*slots)[i].itemId == itemId) {
            int toRemove = std::min((*slots)[i].count, remaining);
            (*slots)[i].count -= toRemove;
            remaining -= toRemove;
            totalRemoved += toRemove;
            
            notifyItemRemoved(ItemStack(itemId, toRemove), i, category);
            
            if ((*slots)[i].count <= 0) {
                (*slots)[i].clear();
            }
        }
    }
    
    if (totalRemoved > 0) {
        notifyInventoryChanged();
    }
    
    return totalRemoved;
}

int CategoryInventory::removeItemFromSlot(InventoryCategory category, int slotIndex, int count) {
    if (slotIndex < 0 || slotIndex >= CATEGORY_SLOTS) return 0;
    
    std::array<ItemStack, CATEGORY_SLOTS>* slots = nullptr;
    switch (category) {
        case InventoryCategory::Materials:   slots = &materialSlots; break;
        case InventoryCategory::Consumables: slots = &consumableSlots; break;
        case InventoryCategory::Equipment:   slots = &equipmentSlots; break;
        default: return 0;
    }
    
    if ((*slots)[slotIndex].isEmpty()) return 0;
    
    int toRemove = (count <= 0) ? (*slots)[slotIndex].count : std::min(count, (*slots)[slotIndex].count);
    std::string itemId = (*slots)[slotIndex].itemId;
    
    (*slots)[slotIndex].count -= toRemove;
    
    notifyItemRemoved(ItemStack(itemId, toRemove), slotIndex, category);
    
    if ((*slots)[slotIndex].count <= 0) {
        (*slots)[slotIndex].clear();
    }
    
    notifyInventoryChanged();
    return toRemove;
}

bool CategoryInventory::useItem(InventoryCategory category, int slotIndex) {
    if (category != InventoryCategory::Consumables) return false;
    if (slotIndex < 0 || slotIndex >= CATEGORY_SLOTS) return false;
    if (consumableSlots[slotIndex].isEmpty()) return false;
    
    const ItemData* data = ItemDatabase::getInstance().getItemData(consumableSlots[slotIndex].itemId);
    if (!data) return false;
    
    // 种子不能直接使用，需要种下
    if (isSeed(consumableSlots[slotIndex].itemId)) {
        std::cout << "[CategoryInventory] Seeds must be planted, not used directly" << std::endl;
        return false;
    }
    
    // 调用使用回调
    if (onUseItemCallback) {
        if (!onUseItemCallback(consumableSlots[slotIndex], data)) {
            return false;
        }
    }
    
    // 消耗物品
    ItemStack usedItem = consumableSlots[slotIndex];
    usedItem.count = 1;
    
    consumableSlots[slotIndex].count--;
    if (consumableSlots[slotIndex].count <= 0) {
        consumableSlots[slotIndex].clear();
    }
    
    notifyItemUsed(usedItem, slotIndex, category);
    notifyInventoryChanged();
    
    std::cout << "[CategoryInventory] Used: " << data->name << std::endl;
    return true;
}

bool CategoryInventory::equipItem(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= CATEGORY_SLOTS) return false;
    if (equipmentSlots[slotIndex].isEmpty()) return false;
    
    const ItemData* data = ItemDatabase::getInstance().getItemData(equipmentSlots[slotIndex].itemId);
    if (!data || data->type != ItemType::Equipment) return false;
    
    // 调用装备回调
    if (onEquipItemCallback) {
        if (!onEquipItemCallback(equipmentSlots[slotIndex])) {
            std::cout << "[CategoryInventory] Failed to equip: " << data->name << std::endl;
            return false;
        }
    }
    
    // 成功装备后从背包移除
    std::cout << "[CategoryInventory] Equipped: " << data->name << std::endl;
    
    // 移除装备的物品
    equipmentSlots[slotIndex].count--;
    if (equipmentSlots[slotIndex].count <= 0) {
        equipmentSlots[slotIndex].clear();
    }
    
    notifyInventoryChanged();
    return true;
}

bool CategoryInventory::plantSeed(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= CATEGORY_SLOTS) return false;
    if (consumableSlots[slotIndex].isEmpty()) return false;
    
    if (!isSeed(consumableSlots[slotIndex].itemId)) {
        std::cout << "[CategoryInventory] This item is not a seed" << std::endl;
        return false;
    }
    
    // 调用种植回调
    if (onPlantSeedCallback) {
        if (!onPlantSeedCallback(consumableSlots[slotIndex])) {
            return false;
        }
    }
    
    // 消耗种子
    consumableSlots[slotIndex].count--;
    if (consumableSlots[slotIndex].count <= 0) {
        consumableSlots[slotIndex].clear();
    }
    
    notifyInventoryChanged();
    std::cout << "[CategoryInventory] Planted seed" << std::endl;
    return true;
}

bool CategoryInventory::destroyItem(InventoryCategory category, int slotIndex, int count) {
    if (slotIndex < 0 || slotIndex >= CATEGORY_SLOTS) return false;
    
    std::array<ItemStack, CATEGORY_SLOTS>* slots = nullptr;
    switch (category) {
        case InventoryCategory::Materials:   slots = &materialSlots; break;
        case InventoryCategory::Consumables: slots = &consumableSlots; break;
        case InventoryCategory::Equipment:   slots = &equipmentSlots; break;
        default: return false;
    }
    
    if ((*slots)[slotIndex].isEmpty()) return false;
    
    const ItemData* data = ItemDatabase::getInstance().getItemData((*slots)[slotIndex].itemId);
    if (data && data->type == ItemType::Quest) {
        std::cout << "[CategoryInventory] Cannot destroy quest item" << std::endl;
        return false;
    }
    
    int toDestroy = (count < 0) ? (*slots)[slotIndex].count : std::min(count, (*slots)[slotIndex].count);
    
    std::cout << "[CategoryInventory] Destroyed " << toDestroy << "x " 
              << (data ? data->name : (*slots)[slotIndex].itemId) << std::endl;
    
    (*slots)[slotIndex].count -= toDestroy;
    if ((*slots)[slotIndex].count <= 0) {
        (*slots)[slotIndex].clear();
    }
    
    notifyInventoryChanged();
    return true;
}

int CategoryInventory::sellItem(InventoryCategory category, int slotIndex, int count) {
    if (slotIndex < 0 || slotIndex >= CATEGORY_SLOTS) return 0;
    
    std::array<ItemStack, CATEGORY_SLOTS>* slots = nullptr;
    switch (category) {
        case InventoryCategory::Materials:   slots = &materialSlots; break;
        case InventoryCategory::Consumables: slots = &consumableSlots; break;
        case InventoryCategory::Equipment:   slots = &equipmentSlots; break;
        default: return 0;
    }
    
    if ((*slots)[slotIndex].isEmpty()) return 0;
    
    const ItemData* data = ItemDatabase::getInstance().getItemData((*slots)[slotIndex].itemId);
    if (!data) return 0;
    
    if (data->type == ItemType::Quest) {
        std::cout << "[CategoryInventory] Cannot sell quest item" << std::endl;
        return 0;
    }
    
    int toSell = (count < 0) ? (*slots)[slotIndex].count : std::min(count, (*slots)[slotIndex].count);
    int goldValue = toSell * data->sellPrice;
    
    // 调用卖出回调
    if (onSellItemCallback) {
        onSellItemCallback(ItemStack((*slots)[slotIndex].itemId, toSell), goldValue);
    }
    
    std::cout << "[CategoryInventory] Sold " << toSell << "x " << data->name 
              << " for " << goldValue << " gold" << std::endl;
    
    (*slots)[slotIndex].count -= toSell;
    if ((*slots)[slotIndex].count <= 0) {
        (*slots)[slotIndex].clear();
    }
    
    notifyInventoryChanged();
    return goldValue;
}

ItemStack CategoryInventory::dropItem(InventoryCategory category, int slotIndex, int count) {
    if (slotIndex < 0 || slotIndex >= CATEGORY_SLOTS) return ItemStack();
    
    std::array<ItemStack, CATEGORY_SLOTS>* slots = nullptr;
    switch (category) {
        case InventoryCategory::Materials:   slots = &materialSlots; break;
        case InventoryCategory::Consumables: slots = &consumableSlots; break;
        case InventoryCategory::Equipment:   slots = &equipmentSlots; break;
        default: return ItemStack();
    }
    
    if ((*slots)[slotIndex].isEmpty()) return ItemStack();
    
    const ItemData* data = ItemDatabase::getInstance().getItemData((*slots)[slotIndex].itemId);
    if (data && data->type == ItemType::Quest) {
        std::cout << "[CategoryInventory] Cannot drop quest item" << std::endl;
        return ItemStack();
    }
    
    int toDrop = (count < 0) ? (*slots)[slotIndex].count : std::min(count, (*slots)[slotIndex].count);
    ItemStack dropped((*slots)[slotIndex].itemId, toDrop);
    
    (*slots)[slotIndex].count -= toDrop;
    if ((*slots)[slotIndex].count <= 0) {
        (*slots)[slotIndex].clear();
    }
    
    notifyItemRemoved(dropped, slotIndex, category);
    notifyInventoryChanged();
    
    return dropped;
}

void CategoryInventory::swapSlots(InventoryCategory category, int index1, int index2) {
    if (index1 < 0 || index1 >= CATEGORY_SLOTS) return;
    if (index2 < 0 || index2 >= CATEGORY_SLOTS) return;
    if (index1 == index2) return;
    
    std::array<ItemStack, CATEGORY_SLOTS>* slots = nullptr;
    switch (category) {
        case InventoryCategory::Materials:   slots = &materialSlots; break;
        case InventoryCategory::Consumables: slots = &consumableSlots; break;
        case InventoryCategory::Equipment:   slots = &equipmentSlots; break;
        default: return;
    }
    
    std::swap((*slots)[index1], (*slots)[index2]);
    notifyInventoryChanged();
}

void CategoryInventory::sortCategory(InventoryCategory category) {
    std::array<ItemStack, CATEGORY_SLOTS>* slots = nullptr;
    switch (category) {
        case InventoryCategory::Materials:   slots = &materialSlots; break;
        case InventoryCategory::Consumables: slots = &consumableSlots; break;
        case InventoryCategory::Equipment:   slots = &equipmentSlots; break;
        default: return;
    }
    
    // 收集非空物品
    std::vector<ItemStack> items;
    for (auto& slot : *slots) {
        if (!slot.isEmpty()) {
            items.push_back(slot);
            slot.clear();
        }
    }
    
    // 按ID排序
    std::sort(items.begin(), items.end(), [](const ItemStack& a, const ItemStack& b) {
        return a.itemId < b.itemId;
    });
    
    // 重新放回（会自动堆叠）
    for (const auto& item : items) {
        // 直接放入当前分类
        for (int i = 0; i < CATEGORY_SLOTS; i++) {
            if ((*slots)[i].isEmpty()) {
                (*slots)[i] = item;
                break;
            } else if ((*slots)[i].itemId == item.itemId) {
                const ItemData* data = ItemDatabase::getInstance().getItemData(item.itemId);
                int maxStack = data ? data->maxStack : 99;
                if ((*slots)[i].count < maxStack) {
                    int canAdd = maxStack - (*slots)[i].count;
                    (*slots)[i].count += std::min(canAdd, item.count);
                    break;
                }
            }
        }
    }
    
    notifyInventoryChanged();
    std::cout << "[CategoryInventory] Sorted " << getCategoryName(category) << std::endl;
}

void CategoryInventory::sortAll() {
    sortCategory(InventoryCategory::Materials);
    sortCategory(InventoryCategory::Consumables);
    sortCategory(InventoryCategory::Equipment);
}

// ============================================================================
// 查询功能
// ============================================================================

const ItemStack& CategoryInventory::getSlot(InventoryCategory category, int index) const {
    static ItemStack emptySlot;
    if (index < 0 || index >= CATEGORY_SLOTS) return emptySlot;
    
    switch (category) {
        case InventoryCategory::Materials:   return materialSlots[index];
        case InventoryCategory::Consumables: return consumableSlots[index];
        case InventoryCategory::Equipment:   return equipmentSlots[index];
        default:                             return emptySlot;
    }
}

ItemStack& CategoryInventory::getSlot(InventoryCategory category, int index) {
    static ItemStack emptySlot;
    if (index < 0 || index >= CATEGORY_SLOTS) return emptySlot;
    
    switch (category) {
        case InventoryCategory::Materials:   return materialSlots[index];
        case InventoryCategory::Consumables: return consumableSlots[index];
        case InventoryCategory::Equipment:   return equipmentSlots[index];
        default:                             return emptySlot;
    }
}

bool CategoryInventory::hasItem(const std::string& itemId, int count) const {
    return getItemCount(itemId) >= count;
}

int CategoryInventory::getItemCount(const std::string& itemId) const {
    int total = 0;
    
    for (const auto& slot : materialSlots) {
        if (slot.itemId == itemId) total += slot.count;
    }
    for (const auto& slot : consumableSlots) {
        if (slot.itemId == itemId) total += slot.count;
    }
    for (const auto& slot : equipmentSlots) {
        if (slot.itemId == itemId) total += slot.count;
    }
    
    return total;
}

int CategoryInventory::getEmptySlotCount(InventoryCategory category) const {
    const std::array<ItemStack, CATEGORY_SLOTS>* slots = nullptr;
    switch (category) {
        case InventoryCategory::Materials:   slots = &materialSlots; break;
        case InventoryCategory::Consumables: slots = &consumableSlots; break;
        case InventoryCategory::Equipment:   slots = &equipmentSlots; break;
        default: return 0;
    }
    
    int count = 0;
    for (const auto& slot : *slots) {
        if (slot.isEmpty()) count++;
    }
    return count;
}

bool CategoryInventory::isCategoryFull(InventoryCategory category) const {
    return getEmptySlotCount(category) == 0;
}

int CategoryInventory::getCategoryPages(InventoryCategory category) const {
    return (CATEGORY_SLOTS + CATEGORY_SLOTS_PER_PAGE - 1) / CATEGORY_SLOTS_PER_PAGE;
}

std::string CategoryInventory::getCategoryName(InventoryCategory category) {
    switch (category) {
        case InventoryCategory::Materials:   return "材料";
        case InventoryCategory::Consumables: return "消耗品";
        case InventoryCategory::Equipment:   return "装备";
        default:                             return "未知";
    }
}

InventoryCategory CategoryInventory::getItemCategory(ItemType type) {
    switch (type) {
        case ItemType::Material:   return InventoryCategory::Materials;
        case ItemType::Consumable: return InventoryCategory::Consumables;
        case ItemType::Equipment:  return InventoryCategory::Equipment;
        default:                   return InventoryCategory::Materials;
    }
}

bool CategoryInventory::isSeed(const std::string& itemId) {
    return itemId == "seed" || 
           itemId.find("seed") != std::string::npos ||
           itemId.find("_seed") != std::string::npos;
}

std::vector<ContextMenuOption> CategoryInventory::getContextMenuOptions(const ItemData* data) {
    std::vector<ContextMenuOption> options;
    
    if (!data) return options;
    
    switch (data->type) {
        case ItemType::Material:
            options.push_back(ContextMenuOption::Destroy);
            options.push_back(ContextMenuOption::Sell);
            break;
            
        case ItemType::Consumable:
            // 检查是否为种子
            if (isSeed(data->id)) {
                options.push_back(ContextMenuOption::Plant);
            } else {
                options.push_back(ContextMenuOption::Use);
            }
            options.push_back(ContextMenuOption::Destroy);
            options.push_back(ContextMenuOption::Sell);
            break;
            
        case ItemType::Equipment:
            options.push_back(ContextMenuOption::Equip);
            options.push_back(ContextMenuOption::Destroy);
            options.push_back(ContextMenuOption::Sell);
            break;
            
        case ItemType::Quest:
            // 任务物品不能销毁或卖出
            break;
            
        default:
            options.push_back(ContextMenuOption::Destroy);
            options.push_back(ContextMenuOption::Sell);
            break;
    }
    
    return options;
}

// ============================================================================
// 私有方法
// ============================================================================

int CategoryInventory::findStackableSlot(InventoryCategory category, const std::string& itemId) const {
    const ItemData* data = ItemDatabase::getInstance().getItemData(itemId);
    if (!data) return -1;
    
    const std::array<ItemStack, CATEGORY_SLOTS>* slots = nullptr;
    switch (category) {
        case InventoryCategory::Materials:   slots = &materialSlots; break;
        case InventoryCategory::Consumables: slots = &consumableSlots; break;
        case InventoryCategory::Equipment:   slots = &equipmentSlots; break;
        default: return -1;
    }
    
    for (int i = 0; i < CATEGORY_SLOTS; i++) {
        if ((*slots)[i].itemId == itemId && (*slots)[i].count < data->maxStack) {
            return i;
        }
    }
    return -1;
}

int CategoryInventory::findEmptySlot(InventoryCategory category) const {
    const std::array<ItemStack, CATEGORY_SLOTS>* slots = nullptr;
    switch (category) {
        case InventoryCategory::Materials:   slots = &materialSlots; break;
        case InventoryCategory::Consumables: slots = &consumableSlots; break;
        case InventoryCategory::Equipment:   slots = &equipmentSlots; break;
        default: return -1;
    }
    
    for (int i = 0; i < CATEGORY_SLOTS; i++) {
        if ((*slots)[i].isEmpty()) {
            return i;
        }
    }
    return -1;
}

void CategoryInventory::notifyItemAdded(const ItemStack& item, int slotIndex, InventoryCategory category) {
    if (onItemAdded) onItemAdded(item, slotIndex, category);
}

void CategoryInventory::notifyItemRemoved(const ItemStack& item, int slotIndex, InventoryCategory category) {
    if (onItemRemoved) onItemRemoved(item, slotIndex, category);
}

void CategoryInventory::notifyItemUsed(const ItemStack& item, int slotIndex, InventoryCategory category) {
    if (onItemUsed) onItemUsed(item, slotIndex, category);
}

void CategoryInventory::notifyInventoryChanged() {
    if (onInventoryChanged) onInventoryChanged();
}
