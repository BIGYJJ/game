#pragma once
#include "Pet.h"
#include "PetRabbit.h"
#include <memory>
#include <vector>
#include <map>

// ============================================================================
// 宠物管理器 (Pet Manager)
// 
// 功能：
//   - 管理所有宠物
//   - 切换当前宠物
//   - 孵化新宠物
//   - 洗点宠物
//   - 处理宠物物品（精元、强化剂、洗涤剂）
// ============================================================================

// 宠物槽位结构
struct PetSlot {
    std::unique_ptr<Pet> pet;
    bool isEmpty() const { return pet == nullptr; }
};

class PetManager : public PetManagerBase {
public:
    PetManager();
    ~PetManager() = default;
    
    // ========================================
    // 初始化
    // ========================================
    bool init(const std::string& resourcePath);
    
    // ========================================
    // 更新和渲染
    // ========================================
    void update(float dt, const sf::Vector2f& ownerPos, bool ownerAttacking) override;
    void render(sf::RenderWindow& window) override;
    
    // ========================================
    // 宠物管理
    // ========================================
    
    // 获取当前宠物
    Pet* getCurrentPet() override;
    
    // 获取当前宠物攻击伤害
    float getCurrentPetDamage() override;
    
    // 获取宠物数量
    size_t getPetCount() const override;
    
    // 获取所有宠物
    const std::vector<PetSlot>& getPetSlots() const { return petSlots; }
    
    // 获取指定槽位的宠物
    Pet* getPetAt(int slotIndex);
    
    // 切换当前宠物
    bool switchPet(int slotIndex);
    
    // 获取当前宠物槽位索引
    int getCurrentPetIndex() const { return currentPetIndex; }
    
    // ========================================
    // 孵化系统
    // ========================================
    
    // 孵化新宠物
    // petTypeId: 宠物类型ID（1=兔子）
    // enhancerCount: 使用的强化剂数量
    // 返回：是否孵化成功
    bool hatchPet(int petTypeId, int enhancerCount = 0);
    
    // 检查是否可以孵化（有空槽位）
    bool canHatchNewPet() const;
    
    // 获取最大宠物数量
    int getMaxPetSlots() const { return MAX_PET_SLOTS; }
    
    // ========================================
    // 洗点系统
    // ========================================
    
    // 洗点当前宠物
    // playerLuck: 玩家幸运值 0-300
    bool washCurrentPet(float playerLuck);
    
    // 洗点指定槽位的宠物
    bool washPet(int slotIndex, float playerLuck);
    
    // ========================================
    // 释放宠物
    // ========================================
    bool releasePet(int slotIndex);
    
    // ========================================
    // 宠物对主人的加成
    // ========================================
    
    // 获取当前宠物对主人攻击力的加成百分比
    float getOwnerAttackBonus() const;
    
    // ========================================
    // 物品掉落检查
    // ========================================
    
    // 检查当前宠物是否有物品掉落（如兔毛）
    // 返回：物品ID（空字符串表示无掉落）
    std::string checkPetItemDrop(float dt);
    
    // ========================================
    // 宠物类型注册
    // ========================================
    
    // 获取已注册的宠物类型ID列表
    std::vector<int> getRegisteredPetTypes() const;
    
    // 获取宠物类型名称
    std::string getPetTypeName(int petTypeId) const;

private:
    // 创建指定类型的宠物
    std::unique_ptr<Pet> createPet(int petTypeId);
    
    // 查找空槽位
    int findEmptySlot() const;

private:
    // 宠物槽位
    std::vector<PetSlot> petSlots;
    
    // 当前使用的宠物索引
    int currentPetIndex;
    
    // 资源路径
    std::string resourcePath;
    
    // 宠物类型名称映射
    std::map<int, std::string> petTypeNames;
    
    // 最大宠物数量
    static constexpr int MAX_PET_SLOTS = 6;
};
