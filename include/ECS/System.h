/**
 * @file System.h
 * @brief Base system class for the Entity-Component-System architecture
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "Entity.h"
#include <vector>
#include <set>

class EntityManager;

/**
 * @class System
 * @brief Abstract base class for all ECS systems
 *
 * Systems contain the logic that operates on entities with specific components.
 * Each system maintains a set of entities it's interested in and processes
 * them during the Update() call.
 *
 * Systems are notified when entities are added or removed from the world,
 * allowing them to maintain their own entity lists based on component requirements.
 *
 * @example
 * ```cpp
 * class MovementSystem : public System {
 * public:
 *     void Update(float deltaTime) override {
 *         for (Entity entity : GetEntities()) {
 *             auto* pos = m_entityManager->GetComponent<PositionComponent>(entity);
 *             auto* vel = m_entityManager->GetComponent<VelocityComponent>(entity);
 *             if (pos && vel) {
 *                 pos->x += vel->x * deltaTime;
 *                 pos->y += vel->y * deltaTime;
 *             }
 *         }
 *     }
 * };
 * ```
 */
class System {
public:
    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~System() = default;

    /**
     * @brief Update system logic (pure virtual - must be implemented)
     *
     * Called once per frame to process all entities managed by this system.
     * Derived classes must implement this method to define their behavior.
     *
     * @param deltaTime Time elapsed since last frame in seconds
     */
    virtual void Update(float deltaTime) = 0;

    /**
     * @brief Called when an entity is added to the world
     *
     * Override this method to check if the entity has the required components
     * for this system and add it to the system's entity list if appropriate.
     *
     * @param entity The entity that was added
     */
    virtual void OnEntityAdded(Entity entity) {}

    /**
     * @brief Called when an entity is removed from the world
     *
     * Override this method to remove the entity from the system's entity list
     * and perform any necessary cleanup.
     *
     * @param entity The entity that was removed
     */
    virtual void OnEntityRemoved(Entity entity) {}

    /**
     * @brief Set the entity manager reference
     *
     * Called by the ECS framework to provide access to the entity manager.
     * Systems use this to query components and manage entities.
     *
     * @param manager Pointer to the EntityManager instance
     */
    void SetEntityManager(EntityManager* manager) { m_entityManager = manager; }

protected:
    EntityManager* m_entityManager = nullptr; ///< Reference to the entity manager
    std::set<Entity> m_entities;              ///< Set of entities this system processes

    /**
     * @brief Add an entity to this system's processing list
     * @param entity The entity to add
     */
    void AddEntity(Entity entity) { m_entities.insert(entity); }

    /**
     * @brief Remove an entity from this system's processing list
     * @param entity The entity to remove
     */
    void RemoveEntity(Entity entity) { m_entities.erase(entity); }

    /**
     * @brief Get the set of entities managed by this system
     * @return Const reference to the entity set
     */
    const std::set<Entity>& GetEntities() const { return m_entities; }
};

// Forward declarations for common systems
class MovementSystem;  ///< System for handling entity movement
class RenderSystem;    ///< System for rendering entities
class CollisionSystem; ///< System for collision detection and response
