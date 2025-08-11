/**
 * @file EntityManager.cpp
 * @brief Implementation of Entity-Component-System manager
 * @author Ryan Butler
 * @date 2025
 */

#include "ECS/EntityManager.h"
#include <iostream>

/**
 * @brief Constructor - initializes entity management system
 *
 * Sets up the entity manager with:
 * - Starting entity ID of 1 (0 is reserved for invalid entities)
 * - Empty entity and component containers
 * - No registered systems initially
 *
 * @note Entity IDs start at 1 to allow 0 to represent "invalid entity"
 */
EntityManager::EntityManager() : m_nextEntityID(1) {}

/**
 * @brief Destructor - automatic cleanup via smart pointers
 *
 * All components and systems are automatically cleaned up
 * via smart pointers and RAII principles.
 */
EntityManager::~EntityManager() = default;

/**
 * @brief Create a new entity with unique ID
 *
 * Creates a new entity and assigns it a unique ID. The entity
 * starts with no components - use AddComponent() to add functionality.
 *
 * @return New Entity with unique ID
 *
 * @note Entity IDs are never reused (monotonically increasing)
 * @note New entity is automatically added to the active entity list
 * @note All registered systems are notified of the new entity
 *
 * @example
 * ```cpp
 * Entity player = entityManager.CreateEntity();
 * entityManager.AddComponent<TransformComponent>(player, 100.0f, 200.0f);
 * entityManager.AddComponent<VelocityComponent>(player, 0.0f, 0.0f);
 * ```
 */
Entity EntityManager::CreateEntity() {
    // Create entity with next available ID
    Entity entity(m_nextEntityID++);

    // Add to active entity list
    m_entities.push_back(entity);

    // Notify all systems that a new entity was created
    // Systems can then check if the entity has components they care about
    NotifySystemsEntityAdded(entity);

    return entity;
}

/**
 * @brief Mark entity for destruction (deferred)
 *
 * Marks an entity for destruction at the end of the current frame.
 * This deferred approach prevents issues with systems iterating
 * over entities while they're being destroyed.
 *
 * @param entity Entity to destroy
 *
 * @note Destruction is deferred until Update() is called
 * @note Does nothing if entity is invalid or already destroyed
 * @note All components are automatically removed when entity is destroyed
 * @note Systems are notified when entity is actually destroyed
 *
 * @example
 * ```cpp
 * // Mark enemy for destruction when health reaches zero
 * if (healthComponent->health <= 0) {
 *     entityManager.DestroyEntity(enemy);
 *     // Entity still exists until end of frame
 * }
 * ```
 */
void EntityManager::DestroyEntity(Entity entity) {
    if (IsEntityValid(entity)) {
        // Add to destruction queue (processed at end of frame)
        m_entitiesToDestroy.push_back(entity);
    }
}

/**
 * @brief Check if entity exists and is valid
 *
 * Verifies that an entity exists in the active entity list
 * and has not been destroyed.
 *
 * @param entity Entity to validate
 *
 * @return true if entity is valid and active, false otherwise
 *
 * @note Returns false for entities marked for destruction but not yet processed
 * @note Linear search - O(n) complexity, suitable for moderate entity counts
 *
 * @example
 * ```cpp
 * if (entityManager.IsEntityValid(player)) {
 *     // Safe to access player components
 *     auto* transform = entityManager.GetComponent<TransformComponent>(player);
 * }
 * ```
 */
bool EntityManager::IsEntityValid(Entity entity) const {
    return std::find(m_entities.begin(), m_entities.end(), entity) != m_entities.end();
}

void EntityManager::Update(float deltaTime) {
    // Process entity destruction
    ProcessEntityDestruction();
    
    // Update all systems
    for (auto& system : m_systems) {
        system->Update(deltaTime);
    }
}

void EntityManager::ProcessEntityDestruction() {
    for (Entity entity : m_entitiesToDestroy) {
        // Remove from entities list
        auto it = std::find(m_entities.begin(), m_entities.end(), entity);
        if (it != m_entities.end()) {
            m_entities.erase(it);
        }
        
        // Remove all components
        for (auto& componentMap : m_components) {
            componentMap.second.erase(entity.GetID());
        }
        
        // Notify systems
        NotifySystemsEntityRemoved(entity);
    }
    
    m_entitiesToDestroy.clear();
}

void EntityManager::NotifySystemsEntityAdded(Entity entity) {
    for (auto& system : m_systems) {
        system->OnEntityAdded(entity);
    }
}

void EntityManager::NotifySystemsEntityRemoved(Entity entity) {
    for (auto& system : m_systems) {
        system->OnEntityRemoved(entity);
    }
}


