#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>

// ============================================================================
// 碰撞系统
// 
// 核心概念：
//   1. 碰撞检测 (Collision Detection) - 判断两个物体是否重叠
//   2. 碰撞响应 (Collision Response) - 碰撞后如何处理
//   
// 本系统实现"主动推挤"机制：
//   - 谁在移动（主动方），谁就把对方（被动方）推开
//   - 玩家主动撞兔子 → 兔子被推开
//   - 兔子主动撞玩家 → 玩家被推开
// ============================================================================

namespace CollisionSystem {

// ============================================================================
// 碰撞检测
// ============================================================================

// 检测两个矩形是否碰撞
inline bool checkAABB(const sf::FloatRect& a, const sf::FloatRect& b) {
    return a.intersects(b);
}

// 检测圆形与圆形碰撞
inline bool checkCircle(const sf::Vector2f& centerA, float radiusA,
                        const sf::Vector2f& centerB, float radiusB) {
    float dx = centerB.x - centerA.x;
    float dy = centerB.y - centerA.y;
    float distSq = dx * dx + dy * dy;
    float radiusSum = radiusA + radiusB;
    return distSq < radiusSum * radiusSum;
}

// ============================================================================
// 碰撞响应 - 推挤计算
// ============================================================================

// 计算推挤向量（从A推向B的方向和力度）
// 返回：B应该移动的向量
inline sf::Vector2f calculatePushVector(const sf::FloatRect& moverBox,    // 主动方碰撞盒
                                        const sf::FloatRect& targetBox,   // 被动方碰撞盒
                                        float pushStrength = 1.0f)        // 推挤力度
{
    // 计算两个矩形的中心
    sf::Vector2f moverCenter(
        moverBox.left + moverBox.width / 2.0f,
        moverBox.top + moverBox.height / 2.0f
    );
    sf::Vector2f targetCenter(
        targetBox.left + targetBox.width / 2.0f,
        targetBox.top + targetBox.height / 2.0f
    );
    
    // 计算推挤方向（从主动方指向被动方）
    sf::Vector2f pushDir = targetCenter - moverCenter;
    
    // 计算距离
    float length = std::sqrt(pushDir.x * pushDir.x + pushDir.y * pushDir.y);
    
    // 避免除零
    if (length < 0.001f) {
        // 如果完全重叠，随机方向推开
        pushDir = sf::Vector2f(1.0f, 0.0f);
        length = 1.0f;
    }
    
    // 归一化方向
    pushDir /= length;
    
    // 计算重叠深度（用于确定推挤距离）
    float overlapX = (moverBox.width + targetBox.width) / 2.0f - std::abs(moverCenter.x - targetCenter.x);
    float overlapY = (moverBox.height + targetBox.height) / 2.0f - std::abs(moverCenter.y - targetCenter.y);
    
    // 取最小重叠作为推挤距离（+1像素确保完全分开）
    float pushDistance = std::min(overlapX, overlapY) + 1.0f;
    
    return pushDir * pushDistance * pushStrength;
}

// 计算分离向量（用于将两个重叠的物体分开）
// 这个版本只计算最小分离距离，用于阻挡型碰撞
inline sf::Vector2f calculateSeparationVector(const sf::FloatRect& boxA, 
                                               const sf::FloatRect& boxB)
{
    // 计算两个矩形的中心
    sf::Vector2f centerA(boxA.left + boxA.width / 2.0f, boxA.top + boxA.height / 2.0f);
    sf::Vector2f centerB(boxB.left + boxB.width / 2.0f, boxB.top + boxB.height / 2.0f);
    
    // 计算在每个轴上的重叠量
    float halfWidthA = boxA.width / 2.0f;
    float halfHeightA = boxA.height / 2.0f;
    float halfWidthB = boxB.width / 2.0f;
    float halfHeightB = boxB.height / 2.0f;
    
    float dx = centerB.x - centerA.x;
    float dy = centerB.y - centerA.y;
    
    float overlapX = halfWidthA + halfWidthB - std::abs(dx);
    float overlapY = halfHeightA + halfHeightB - std::abs(dy);
    
    // 选择最小重叠方向分离
    if (overlapX < overlapY) {
        // 水平方向分离
        return sf::Vector2f((dx > 0 ? 1.0f : -1.0f) * overlapX, 0.0f);
    } else {
        // 垂直方向分离
        return sf::Vector2f(0.0f, (dy > 0 ? 1.0f : -1.0f) * overlapY);
    }
}

// ============================================================================
// 高级碰撞响应
// ============================================================================

// 碰撞结果结构
struct CollisionResult {
    bool collided = false;           // 是否发生碰撞
    sf::Vector2f pushVector;         // 推挤向量
    sf::Vector2f separationVector;   // 分离向量
};

// 检测碰撞并计算响应
inline CollisionResult detectAndRespond(const sf::FloatRect& moverBox,
                                         const sf::FloatRect& targetBox,
                                         float pushStrength = 1.0f)
{
    CollisionResult result;
    
    if (checkAABB(moverBox, targetBox)) {
        result.collided = true;
        result.pushVector = calculatePushVector(moverBox, targetBox, pushStrength);
        result.separationVector = calculateSeparationVector(moverBox, targetBox);
    }
    
    return result;
}

} // namespace CollisionSystem
