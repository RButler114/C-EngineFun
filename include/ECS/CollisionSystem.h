/**
 * @file CollisionSystem.h
 * @brief System for detecting and handling collisions between entities
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "System.h"
#include "EntityManager.h"
#include "Component.h"
#include <functional>

/**
 * @struct CollisionInfo
 * @brief Information about a collision between two entities
 *
 * Contains details about which entities collided and how much they overlap.
 * Used by collision callbacks to handle collision responses.
 */
struct CollisionInfo {
    Entity entityA;     ///< First entity in the collision
    Entity entityB;     ///< Second entity in the collision
    float overlapX;     ///< Amount of overlap on X axis
    float overlapY;     ///< Amount of overlap on Y axis
};

/**
 * @class CollisionSystem
 * @brief System that detects collisions between entities with collision components
 *
 * The CollisionSystem uses Axis-Aligned Bounding Box (AABB) collision detection
 * to check for overlaps between entities that have both TransformComponent and
 * CollisionComponent.
 *
 * Features:
 * - AABB collision detection
 * - Collision callbacks for custom response handling
 * - Support for trigger colliders (detection without physics response)
 * - Efficient pairwise collision checking
 *
 * @example
 * ```cpp
 * // Add collision system with callback
 * auto* collisionSystem = entityManager.AddSystem<CollisionSystem>();
 * collisionSystem->SetCollisionCallback([](const CollisionInfo& info) {
 *     std::cout << "Collision between entities " << info.entityA.GetID()
 *               << " and " << info.entityB.GetID() << std::endl;
 * });
 *
 * // Create entities with collision
 * Entity player = entityManager.CreateEntity();
 * entityManager.AddComponent<TransformComponent>(player, 100.0f, 200.0f);
 * entityManager.AddComponent<CollisionComponent>(player, 32.0f, 32.0f);
 * ```
 */
class CollisionSystem : public System {
public:
    /// Type alias for collision callback functions
    using CollisionCallback = std::function<void(const CollisionInfo&)>;

    /**
     * @brief Check for collisions between all entities
     *
     * Performs pairwise collision detection between all entities that have
     * both TransformComponent and CollisionComponent. When a collision is
     * detected, the registered callback (if any) is invoked.
     *
     * @param deltaTime Time elapsed since last update (unused for collision detection)
     *
     * @note Uses O(n²) algorithm - consider spatial partitioning for large numbers of entities
     */
    void Update(float deltaTime) override {
        (void)deltaTime; // Not needed for collision detection

        auto entities = m_entityManager->GetEntitiesWith<TransformComponent, CollisionComponent>();

        // Check all pairs of entities for collision
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                CheckCollision(entities[i], entities[j]);
            }
        }
    }

    /**
     * @brief Set callback function for collision events
     *
     * The callback will be invoked whenever a collision is detected between
     * two entities. Use this to implement custom collision responses.
     *
     * @param callback Function to call when collisions occur
     *
     * @example
     * ```cpp
     * collisionSystem->SetCollisionCallback([](const CollisionInfo& info) {
     *     // Handle collision between info.entityA and info.entityB
     *     std::cout << "Collision detected!" << std::endl;
     * });
     * ```
     */
    void SetCollisionCallback(CollisionCallback callback) {
        m_collisionCallback = callback;
    }

private:
    CollisionCallback m_collisionCallback; ///< Callback function for collision events

    /**
     * @brief Check collision between two specific entities
     *
     * @param entityA First entity to check
     * @param entityB Second entity to check
     */
    void CheckCollision(Entity entityA, Entity entityB);

    /**
     * @brief Axis-Aligned Bounding Box collision detection
     *
     * Performs AABB collision detection between two entities and calculates
     * the amount of overlap if a collision occurs.
     *
     * @param transformA Transform component of first entity
     * @param collisionA Collision component of first entity
     * @param transformB Transform component of second entity
     * @param collisionB Collision component of second entity
     * @param overlapX Output parameter for X-axis overlap amount
     * @param overlapY Output parameter for Y-axis overlap amount
     * @return true if collision detected, false otherwise
     */
    bool AABB(const TransformComponent* transformA, const CollisionComponent* collisionA,
              const TransformComponent* transformB, const CollisionComponent* collisionB,
              float& overlapX, float& overlapY);
};
