/**
 * @file ECS.h
 * @brief Streamlined ECS header for arcade game engine
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

/**
 * @brief Streamlined Entity-Component-System for arcade games
 *
 * Focused ECS implementation for classic arcade-style scrolling games.
 * Includes only essential systems needed for arcade gameplay.
 *
 * @example
 * ```cpp
 * #include "ECS/ECS.h"
 *
 * // Create entity manager
 * EntityManager entityManager;
 *
 * // Add core systems
 * entityManager.AddSystem<MovementSystem>();
 * entityManager.AddSystem<CollisionSystem>();
 * entityManager.AddSystem<AudioSystem>(audioManager);
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

// Core ECS components
#include "Entity.h"
#include "Component.h"
#include "System.h"
#include "EntityManager.h"

// Essential systems for arcade games
#include "MovementSystem.h"
#include "CollisionSystem.h"
#include "AudioSystem.h"

// Animation support
#include "AnimationComponent.h"
