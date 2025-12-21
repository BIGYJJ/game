#include "PetManager.h"
#include <iostream>
#include <algorithm>

// ============================================================================
// 构造函数
// ============================================================================
PetManager::PetManager()
    : currentPetIndex(-1)
{
    // 初始化宠物槽位
    petSlots.resize(MAX_PET_SLOTS);
    
    // 注册宠物类型
    petTypeNames[1] = "兔子";
    // 后续可以添加更多宠物类型
    // petTypeNames[2] = "史莱姆";
    // petTypeNames[3] = "小鸡";
}

// ============================================================================
// 初始化
// ============================================================================
bool PetManager::init(const std::string& path) {
    resourcePath = path;
    return true;
}

// ============================================================================
// 更新和渲染
// ============================================================================
void PetManager::update(float dt, const sf::Vector2f& ownerPos, bool ownerAttacking) {
    Pet* currentPet = getCurrentPet();
    if (currentPet) {
        currentPet->update(dt, ownerPos, ownerAttacking);
    }
}

void PetManager::render(sf::RenderWindow& window) {
    Pet* currentPet = getCurrentPet();
    if (currentPet) {
        currentPet->render(window);
    }
}

// ============================================================================
// 宠物管理
// ============================================================================
Pet* PetManager::getCurrentPet() {
    if (currentPetIndex < 0 || currentPetIndex >= (int)petSlots.size()) {
        return nullptr;
    }
    return petSlots[currentPetIndex].pet.get();
}

float PetManager::getCurrentPetDamage() {
    Pet* pet = getCurrentPet();
    if (pet && pet->isAttacking()) {
        return pet->performAttack();
    }
    return 0.0f;
}

size_t PetManager::getPetCount() const {
    size_t count = 0;
    for (const auto& slot : petSlots) {
        if (!slot.isEmpty()) {
            count++;
        }
    }
    return count;
}

Pet* PetManager::getPetAt(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= (int)petSlots.size()) {
        return nullptr;
    }
    return petSlots[slotIndex].pet.get();
}

bool PetManager::switchPet(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= (int)petSlots.size()) {
        return false;
    }
    
    if (petSlots[slotIndex].isEmpty()) {
        return false;
    }
    
    currentPetIndex = slotIndex;
    std::cout << "切换到宠物: " << petSlots[slotIndex].pet->getName() << std::endl;
    return true;
}

// ============================================================================
// 孵化系统
// ============================================================================
bool PetManager::hatchPet(int petTypeId, int enhancerCount) {
    // 检查是否有空槽位
    int emptySlot = findEmptySlot();
    if (emptySlot < 0) {
        std::cout << "没有空闲的宠物槽位!" << std::endl;
        return false;
    }
    
    // 创建宠物
    auto pet = createPet(petTypeId);
    if (!pet) {
        std::cout << "未知的宠物类型: " << petTypeId << std::endl;
        return false;
    }
    
    // 随机资质（受强化剂影响）
    std::mt19937 rng(std::random_device{}());
    PetQuality quality = Pet::rollHatchQuality(rng, enhancerCount);
    
    // 孵化
    pet->hatch(quality, enhancerCount);
    
    // 加载贴图
    std::string texturePath = resourcePath;
    if (petTypeId == 1) {
        texturePath += "/assets_rabbit_spritesheet.png";
    }
    pet->loadTexture(texturePath);
    
    // 放入槽位
    petSlots[emptySlot].pet = std::move(pet);
    
    // 如果这是第一个宠物，自动设为当前宠物
    if (currentPetIndex < 0) {
        currentPetIndex = emptySlot;
    }
    
    std::cout << "成功孵化宠物! 槽位: " << emptySlot << std::endl;
    return true;
}

bool PetManager::canHatchNewPet() const {
    for (const auto& slot : petSlots) {
        if (slot.isEmpty()) {
            return true;
        }
    }
    return false;
}

int PetManager::findEmptySlot() const {
    for (size_t i = 0; i < petSlots.size(); i++) {
        if (petSlots[i].isEmpty()) {
            return i;
        }
    }
    return -1;
}

std::unique_ptr<Pet> PetManager::createPet(int petTypeId) {
    switch (petTypeId) {
        case 1:  // 兔子
            return std::make_unique<PetRabbit>();
        default:
            return nullptr;
    }
}

// ============================================================================
// 洗点系统
// ============================================================================
bool PetManager::washCurrentPet(float playerLuck) {
    return washPet(currentPetIndex, playerLuck);
}

bool PetManager::washPet(int slotIndex, float playerLuck) {
    Pet* pet = getPetAt(slotIndex);
    if (!pet) {
        std::cout << "指定槽位没有宠物!" << std::endl;
        return false;
    }
    
    pet->wash(playerLuck);
    return true;
}

// ============================================================================
// 释放宠物
// ============================================================================
bool PetManager::releasePet(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= (int)petSlots.size()) {
        return false;
    }
    
    if (petSlots[slotIndex].isEmpty()) {
        return false;
    }
    
    std::cout << "释放宠物: " << petSlots[slotIndex].pet->getName() << std::endl;
    petSlots[slotIndex].pet.reset();
    
    // 如果释放的是当前宠物，切换到其他宠物
    if (slotIndex == currentPetIndex) {
        currentPetIndex = -1;
        
        // 尝试切换到其他宠物
        for (size_t i = 0; i < petSlots.size(); i++) {
            if (!petSlots[i].isEmpty()) {
                currentPetIndex = i;
                break;
            }
        }
    }
    
    return true;
}

// ============================================================================
// 宠物对主人的加成
// ============================================================================
float PetManager::getOwnerAttackBonus() const {
    if (currentPetIndex < 0 || currentPetIndex >= (int)petSlots.size()) {
        return 0.0f;
    }
    
    Pet* pet = petSlots[currentPetIndex].pet.get();
    if (!pet) return 0.0f;
    
    // 检查是否是兔子宠物
    PetRabbit* rabbit = dynamic_cast<PetRabbit*>(pet);
    if (rabbit) {
        return rabbit->getOwnerAttackBonus();
    }
    
    return 0.0f;
}

// ============================================================================
// 物品掉落检查
// ============================================================================
std::string PetManager::checkPetItemDrop(float dt) {
    Pet* pet = getCurrentPet();
    if (!pet) return "";
    
    // 检查是否是兔子宠物
    PetRabbit* rabbit = dynamic_cast<PetRabbit*>(pet);
    if (rabbit) {
        if (rabbit->checkShedding(dt)) {
            return "rabbit_fur";  // 返回兔毛物品ID
        }
    }
    
    return "";
}

// ============================================================================
// 宠物类型注册
// ============================================================================
std::vector<int> PetManager::getRegisteredPetTypes() const {
    std::vector<int> types;
    for (const auto& pair : petTypeNames) {
        types.push_back(pair.first);
    }
    return types;
}

std::string PetManager::getPetTypeName(int petTypeId) const {
    auto it = petTypeNames.find(petTypeId);
    if (it != petTypeNames.end()) {
        return it->second;
    }
    return "未知";
}
