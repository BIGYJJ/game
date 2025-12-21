// ============================================================================
// GameState.h 更新说明
// 
// 在 #include 部分添加:
// ============================================================================

#include "../Entity/StoneBuild.h"
#include "../Entity/WildPlant.h"

// ============================================================================
// 在 private: 成员变量部分添加:
// ============================================================================

    // 石头建筑系统
    std::unique_ptr<StoneBuildManager> stoneBuildManager;
    
    // 野生植物系统
    std::unique_ptr<WildPlantManager> wildPlantManager;
    
    // 拾取按键状态
    bool pickupKeyPressed = false;
    static constexpr float PLANT_PICKUP_RANGE = 60.0f;  // 植物拾取范围
    
    // 宠物指挥攻击
    bool petCommandKeyPressed = false;  // 右键状态

// ============================================================================
// 在 private: 方法声明部分添加:
// ============================================================================

    // 初始化石头建筑
    void initStoneBuild();
    
    // 初始化野生植物
    void initWildPlants();
    
    // 处理植物拾取
    void handlePlantPickup();
    
    // 处理宠物指挥攻击
    void handlePetCommandAttack(const sf::Vector2f& targetWorldPos);
