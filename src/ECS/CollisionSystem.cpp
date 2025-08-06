/**
 * @file CollisionSystem.cpp
 * @brief Implementation of AABB collision detection system
 * @author Ryan Butler
 * @date 2025
 */

#include "ECS/CollisionSystem.h"
#include <algorithm>
#include <iostream>

/**
 * @brief Update collision detection for all entities
 *
 * Performs collision detection between all entities that have both
 * TransformComponent and CollisionComponent. Uses an O(n²) brute-force
 * approach suitable for arcade games with moderate entity counts.
 *
 * Process:
 * 1. Get all entities with required components
 * 2. Check every pair of entities for collision (avoiding duplicates)
 * 3. Call collision callback for each detected collision
 * 4. Provide debug output periodically
 *
 * @param deltaTime Time elapsed since last frame (unused for collision detection)
 *
 * @note Performance: O(n²) where n is number of collidable entities
 * @note Suitable for <100 entities; consider spatial partitioning for more
 * @note Debug output appears every 5 seconds to monitor entity count
 *
 * Optimization opportunities for large entity counts:
 * - Spatial partitioning (quadtree, grid)
 * - Broad phase / narrow phase separation
 * - Component-based filtering (collision layers)
 */
void CollisionSystem::Update(float deltaTime) {
    (void)deltaTime; // Collision detection doesn't need frame timing

    // Get all entities that can participate in collision detection
    auto entities = m_entityManager->GetEntitiesWith<TransformComponent, CollisionComponent>();

    // Debug monitoring: Track entity count over time
    static int frameCount = 0;
    frameCount++;

    // Output debug info every 300 frames (approximately every 5 seconds at 60 FPS)
    // This helps developers monitor performance and entity lifecycle
    if (frameCount % 300 == 0) {
        std::cout << "🔍 CollisionSystem: Checking " << entities.size()
                  << " entities for collisions" << std::endl;
    }

    // Brute-force collision detection: check every pair of entities
    // This is O(n²) but suitable for arcade games with moderate entity counts
    for (size_t i = 0; i < entities.size(); ++i) {
        for (size_t j = i + 1; j < entities.size(); ++j) {
            // Check collision between entities[i] and entities[j]
            // Note: We start j at i+1 to avoid checking the same pair twice
            CheckCollision(entities[i], entities[j]);
        }
    }
}

/**
 * @brief Check collision between two specific entities
 *
 * Performs AABB (Axis-Aligned Bounding Box) collision detection between
 * two entities. If a collision is detected, calls the registered collision
 * callback with detailed collision information.
 *
 * @param entityA First entity to check
 * @param entityB Second entity to check
 *
 * @note Both entities must have TransformComponent and CollisionComponent
 * @note Missing components are logged for debugging purposes
 * @note Collision callback is only called if collision is detected
 * @note Overlap values indicate how much the entities are intersecting
 *
 * Collision Detection Process:
 * 1. Validate both entities have required components
 * 2. Perform AABB collision test
 * 3. Calculate overlap amounts if collision detected
 * 4. Call collision callback with collision information
 */
void CollisionSystem::CheckCollision(Entity entityA, Entity entityB) {
    // Get required components from both entities
    auto* transformA = m_entityManager->GetComponent<TransformComponent>(entityA);
    auto* collisionA = m_entityManager->GetComponent<CollisionComponent>(entityA);
    auto* transformB = m_entityManager->GetComponent<TransformComponent>(entityB);
    auto* collisionB = m_entityManager->GetComponent<CollisionComponent>(entityB);

    // Validate that both entities have all required components
    if (!transformA || !collisionA || !transformB || !collisionB) {
        // Debug output to help identify component setup issues
        // This helps developers ensure entities are properly configured
        if (!transformA) std::cout << "⚠️  Entity " << entityA.GetID() << " missing TransformComponent" << std::endl;
        if (!collisionA) std::cout << "⚠️  Entity " << entityA.GetID() << " missing CollisionComponent" << std::endl;
        if (!transformB) std::cout << "⚠️  Entity " << entityB.GetID() << " missing TransformComponent" << std::endl;
        if (!collisionB) std::cout << "⚠️  Entity " << entityB.GetID() << " missing CollisionComponent" << std::endl;
        return; // Cannot perform collision detection without required components
    }

    // Perform AABB collision detection
    float overlapX, overlapY;
    if (AABB(transformA, collisionA, transformB, collisionB, overlapX, overlapY)) {
        // Collision detected! Call the registered callback if one exists
        if (m_collisionCallback) {
            // Create collision information structure
            CollisionInfo info;
            info.entityA = entityA;      // First entity involved
            info.entityB = entityB;      // Second entity involved
            info.overlapX = overlapX;    // How much they overlap horizontally
            info.overlapY = overlapY;    // How much they overlap vertically

            // Notify the game logic about this collision
            m_collisionCallback(info);
        }
    }
}

/**
 * @brief Axis-Aligned Bounding Box collision detection
 *
 * Performs AABB collision detection between two rectangular entities.
 * This is the most common collision detection method for 2D games,
 * providing fast and reliable collision detection for rectangular objects.
 *
 * @param transformA Transform component of first entity
 * @param collisionA Collision component of first entity
 * @param transformB Transform component of second entity
 * @param collisionB Collision component of second entity
 * @param overlapX Output: horizontal overlap amount (if collision detected)
 * @param overlapY Output: vertical overlap amount (if collision detected)
 *
 * @return true if entities are colliding, false otherwise
 *
 * @note Overlap values are only valid when return value is true
 * @note Uses top-left origin coordinate system (Y increases downward)
 * @note Collision boxes are axis-aligned (no rotation support)
 *
 * AABB Algorithm:
 * - Two rectangles collide if they overlap on BOTH X and Y axes
 * - X overlap: leftA < rightB AND rightA > leftB
 * - Y overlap: topA < bottomB AND bottomA > topB
 * - Overlap amounts help determine collision response (push-back direction)
 *
 * @example
 * ```cpp
 * float overlapX, overlapY;
 * if (AABB(transformA, collisionA, transformB, collisionB, overlapX, overlapY)) {
 *     // Collision detected!
 *     // overlapX and overlapY tell us how much they're intersecting
 *     // Use this for collision response (push entities apart)
 * }
 * ```
 */
bool CollisionSystem::AABB(const TransformComponent* transformA, const CollisionComponent* collisionA,
                           const TransformComponent* transformB, const CollisionComponent* collisionB,
                           float& overlapX, float& overlapY) {

    // Calculate bounding box for entity A
    // Top-left corner is at (transformA->x, transformA->y)
    float leftA = transformA->x;                        // Left edge
    float rightA = transformA->x + collisionA->width;   // Right edge
    float topA = transformA->y;                         // Top edge
    float bottomA = transformA->y + collisionA->height; // Bottom edge

    // Calculate bounding box for entity B
    // Top-left corner is at (transformB->x, transformB->y)
    float leftB = transformB->x;                        // Left edge
    float rightB = transformB->x + collisionB->width;   // Right edge
    float topB = transformB->y;                         // Top edge
    float bottomB = transformB->y + collisionB->height; // Bottom edge

    // AABB collision test: rectangles collide if they overlap on BOTH axes
    // X-axis overlap: leftA < rightB AND rightA > leftB
    // Y-axis overlap: topA < bottomB AND bottomA > topB
    if (leftA < rightB && rightA > leftB && topA < bottomB && bottomA > topB) {
        // Collision detected! Calculate overlap amounts for collision response

        // Horizontal overlap: how much the rectangles intersect on X-axis
        overlapX = std::min(rightA, rightB) - std::max(leftA, leftB);

        // Vertical overlap: how much the rectangles intersect on Y-axis
        overlapY = std::min(bottomA, bottomB) - std::max(topA, topB);

        return true; // Collision confirmed
    }

    // No collision detected
    return false;
}
