/**
 * @file MovementSystem.h
 * @brief System for handling entity movement based on velocity
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "System.h"
#include "EntityManager.h"
#include "Component.h"
#include <iostream>

/**
 * @class MovementSystem
 * @brief System that updates entity positions based on their velocity
 *
 * The MovementSystem processes all entities that have both TransformComponent
 * and VelocityComponent, updating their positions each frame based on their
 * velocity and the elapsed time (delta time).
 *
 * This system implements basic kinematic movement:
 * - position += velocity * deltaTime
 *
 * @example
 * ```cpp
 * // Add to entity manager
 * entityManager.AddSystem<MovementSystem>();
 *
 * // Create moving entity
 * Entity entity = entityManager.CreateEntity();
 * entityManager.AddComponent<TransformComponent>(entity, 100.0f, 200.0f);
 * entityManager.AddComponent<VelocityComponent>(entity, 50.0f, -25.0f); // moves right and up
 *
 * // System will automatically update position each frame
 * ```
 */
class MovementSystem : public System {
public:
    /**
     * @brief Update entity positions based on their velocities
     *
     * Processes all entities with TransformComponent and VelocityComponent,
     * updating their positions using basic kinematic equations:
     * - new_position = old_position + velocity * deltaTime
     *
     * @param deltaTime Time elapsed since last update in seconds
     *
     * @note This system automatically processes all entities with the required components
     */
    void Update(float deltaTime) override {
        auto entities = m_entityManager->GetEntitiesWith<TransformComponent, VelocityComponent>();

        for (Entity entity : entities) {
            auto* transform = m_entityManager->GetComponent<TransformComponent>(entity);
            auto* velocity = m_entityManager->GetComponent<VelocityComponent>(entity);

            if (transform && velocity) {
                transform->x += velocity->vx * deltaTime;
                transform->y += velocity->vy * deltaTime;
            }
        }
    }
};
