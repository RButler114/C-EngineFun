/**
 * @file ECS.h
 * @brief Convenience header that includes all Entity-Component-System headers
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

/**
 * @defgroup ECS Entity-Component-System
 * @brief Complete Entity-Component-System architecture implementation
 *
 * The ECS (Entity-Component-System) architecture is a design pattern commonly used
 * in game development that separates data (Components) from behavior (Systems) and
 * uses lightweight identifiers (Entities) to associate them.
 *
 * Key concepts:
 * - **Entities**: Unique identifiers that represent game objects
 * - **Components**: Data containers that define what an entity has
 * - **Systems**: Logic processors that define what entities do
 *
 * This implementation provides:
 * - Type-safe component management
 * - Efficient system processing
 * - Flexible entity querying
 * - Memory-efficient storage
 *
 * @example
 * ```cpp
 * #include "ECS/ECS.h"
 *
 * // Create entity manager
 * EntityManager entityManager;
 *
 * // Add systems
 * entityManager.AddSystem<MovementSystem>();
 * entityManager.AddSystem<RenderSystem>(renderer);
 *
 * // Create entity with components
 * Entity player = entityManager.CreateEntity();
 * entityManager.AddComponent<TransformComponent>(player, 100.0f, 200.0f);
 * entityManager.AddComponent<VelocityComponent>(player, 50.0f, 0.0f);
 *
 * // Update all systems
 * entityManager.Update(deltaTime);
 * ```
 */

// Include all ECS headers for convenience
#include "Entity.h"
#include "Component.h"
#include "System.h"
#include "EntityManager.h"
#include "MovementSystem.h"
#include "RenderSystem.h"
#include "CollisionSystem.h"
