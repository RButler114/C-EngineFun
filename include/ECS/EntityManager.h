/**
 * @file EntityManager.h
 * @brief Central manager for entities, components, and systems in the ECS architecture
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "Entity.h"
#include "Component.h"
#include "System.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <typeindex>
#include <type_traits>
#include <iostream>
#include <algorithm>

/**
 * @class EntityManager
 * @brief Central coordinator for the Entity-Component-System architecture
 *
 * The EntityManager is the heart of the ECS system, responsible for:
 * - Creating and destroying entities
 * - Adding, removing, and querying components
 * - Managing and updating systems
 * - Providing efficient entity queries
 *
 * This class maintains all the data structures needed for efficient ECS operations
 * and ensures proper cleanup and notification of systems when entities change.
 *
 * @example
 * ```cpp
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
 * // Game loop
 * while (running) {
 *     entityManager.Update(deltaTime);
 * }
 * ```
 */
class EntityManager {
public:
    /**
     * @brief Constructor - initializes the entity manager
     */
    EntityManager();

    /**
     * @brief Destructor - cleans up all entities, components, and systems
     */
    ~EntityManager();

    /**
     * @defgroup EntityManagement Entity Management
     * @brief Functions for creating, destroying, and validating entities
     * @{
     */

    /**
     * @brief Create a new entity
     *
     * Creates a unique entity with a new ID. The entity starts with no components.
     *
     * @return New entity handle
     *
     * @example
     * ```cpp
     * Entity player = entityManager.CreateEntity();
     * Entity enemy = entityManager.CreateEntity();
     * ```
     */
    Entity CreateEntity();

    /**
     * @brief Destroy an entity and all its components
     *
     * Marks the entity for destruction. The actual cleanup happens at the end
     * of the current frame to avoid iterator invalidation during system updates.
     *
     * @param entity Entity to destroy
     */
    void DestroyEntity(Entity entity);

    /**
     * @brief Check if an entity is valid and exists
     *
     * @param entity Entity to check
     * @return true if entity exists and is valid, false otherwise
     */
    bool IsEntityValid(Entity entity) const;

    /** @} */ // end of EntityManagement group

    /**
     * @defgroup ComponentManagement Component Management
     * @brief Functions for adding, removing, and querying components
     * @{
     */

    /**
     * @brief Add a component to an entity
     *
     * Creates a new component of type T and attaches it to the entity.
     * If the entity already has this component type, it will be replaced.
     *
     * @tparam T Component type to add
     * @tparam Args Constructor argument types
     * @param entity Entity to add component to
     * @param args Arguments to pass to component constructor
     * @return Pointer to the created component
     *
     * @example
     * ```cpp
     * auto* transform = entityManager.AddComponent<TransformComponent>(player, 100.0f, 200.0f);
     * auto* velocity = entityManager.AddComponent<VelocityComponent>(player, 50.0f, 0.0f);
     * ```
     */
    template<typename T, typename... Args>
    T* AddComponent(Entity entity, Args&&... args);

    /**
     * @brief Get a component from an entity
     *
     * @tparam T Component type to retrieve
     * @param entity Entity to get component from
     * @return Pointer to component if it exists, nullptr otherwise
     */
    template<typename T>
    T* GetComponent(Entity entity);

    /**
     * @brief Get a component from an entity (const version)
     *
     * @tparam T Component type to retrieve
     * @param entity Entity to get component from
     * @return Const pointer to component if it exists, nullptr otherwise
     */
    template<typename T>
    const T* GetComponent(Entity entity) const;

    /**
     * @brief Check if an entity has a specific component
     *
     * @tparam T Component type to check for
     * @param entity Entity to check
     * @return true if entity has the component, false otherwise
     */
    template<typename T>
    bool HasComponent(Entity entity) const;

    /**
     * @brief Remove a component from an entity
     *
     * @tparam T Component type to remove
     * @param entity Entity to remove component from
     */
    template<typename T>
    void RemoveComponent(Entity entity);

    /** @} */ // end of ComponentManagement group

    /**
     * @defgroup SystemManagement System Management
     * @brief Functions for adding, removing, and managing systems
     * @{
     */

    /**
     * @brief Add a system to the entity manager
     *
     * Creates a new system of type T and adds it to the update loop.
     * Systems are updated in the order they were added.
     *
     * @tparam T System type to add
     * @tparam Args Constructor argument types
     * @param args Arguments to pass to system constructor
     * @return Pointer to the created system
     *
     * @example
     * ```cpp
     * auto* movementSystem = entityManager.AddSystem<MovementSystem>();
     * auto* renderSystem = entityManager.AddSystem<RenderSystem>(renderer);
     * ```
     */
    template<typename T, typename... Args>
    T* AddSystem(Args&&... args);

    /**
     * @brief Get a system by type
     *
     * @tparam T System type to retrieve
     * @return Pointer to system if it exists, nullptr otherwise
     */
    template<typename T>
    T* GetSystem();

    /**
     * @brief Remove a system by type
     *
     * @tparam T System type to remove
     */
    template<typename T>
    void RemoveSystem();

    /** @} */ // end of SystemManagement group

    /**
     * @brief Update all systems
     *
     * Calls Update() on all registered systems in the order they were added.
     * Also processes any pending entity destructions.
     *
     * @param deltaTime Time elapsed since last update in seconds
     */
    void Update(float deltaTime);

    /**
     * @brief Get all entities that have specific components
     *
     * Returns a vector of entities that have ALL of the specified component types.
     * This is used by systems to efficiently query for entities they can process.
     *
     * @tparam ComponentTypes Component types to query for
     * @return Vector of entities that have all specified components
     *
     * @example
     * ```cpp
     * // Get all entities with both Transform and Velocity components
     * auto entities = entityManager.GetEntitiesWith<TransformComponent, VelocityComponent>();
     * for (Entity entity : entities) {
     *     // Process entity...
     * }
     * ```
     */
    template<typename... ComponentTypes>
    std::vector<Entity> GetEntitiesWith();

private:
    EntityID m_nextEntityID;
    std::vector<Entity> m_entities;
    std::vector<Entity> m_entitiesToDestroy;
    
    // Component storage: ComponentTypeID -> EntityID -> Component
    std::unordered_map<ComponentTypeID, std::unordered_map<EntityID, std::unique_ptr<Component>>> m_components;
    
    // System storage
    std::vector<std::unique_ptr<System>> m_systems;
    std::unordered_map<std::type_index, System*> m_systemMap;
    
    void ProcessEntityDestruction();
    void NotifySystemsEntityAdded(Entity entity);
    void NotifySystemsEntityRemoved(Entity entity);
};

// Template implementations
template<typename T, typename... Args>
T* EntityManager::AddComponent(Entity entity, Args&&... args) {
    if (!IsEntityValid(entity)) {
        return nullptr;
    }

    ComponentTypeID typeID = GetComponentTypeID<T>();
    auto component = std::make_unique<T>(std::forward<Args>(args)...);
    component->owner = entity;

    T* componentPtr = component.get();
    m_components[typeID][entity.GetID()] = std::move(component);

    return componentPtr;
}

template<typename T>
T* EntityManager::GetComponent(Entity entity) {
    if (!IsEntityValid(entity)) {
        return nullptr;
    }

    ComponentTypeID typeID = GetComponentTypeID<T>();
    auto componentMapIt = m_components.find(typeID);
    if (componentMapIt == m_components.end()) {
        return nullptr;
    }

    auto componentIt = componentMapIt->second.find(entity.GetID());
    if (componentIt == componentMapIt->second.end()) {
        return nullptr;
    }

    return static_cast<T*>(componentIt->second.get());
}

template<typename T>
const T* EntityManager::GetComponent(Entity entity) const {
    return const_cast<EntityManager*>(this)->GetComponent<T>(entity);
}

template<typename T>
bool EntityManager::HasComponent(Entity entity) const {
    return GetComponent<T>(entity) != nullptr;
}

template<typename T>
void EntityManager::RemoveComponent(Entity entity) {
    if (!IsEntityValid(entity)) {
        return;
    }

    ComponentTypeID typeID = GetComponentTypeID<T>();
    auto componentMapIt = m_components.find(typeID);
    if (componentMapIt != m_components.end()) {
        componentMapIt->second.erase(entity.GetID());
    }
}

template<typename T, typename... Args>
T* EntityManager::AddSystem(Args&&... args) {
    auto system = std::make_unique<T>(std::forward<Args>(args)...);
    system->SetEntityManager(this);

    T* systemPtr = system.get();
    m_systems.push_back(std::move(system));
    m_systemMap[std::type_index(typeid(T))] = systemPtr;

    return systemPtr;
}

template<typename T>
T* EntityManager::GetSystem() {
    auto it = m_systemMap.find(std::type_index(typeid(T)));
    if (it != m_systemMap.end()) {
        return static_cast<T*>(it->second);
    }
    return nullptr;
}

template<typename T>
void EntityManager::RemoveSystem() {
    auto it = m_systemMap.find(std::type_index(typeid(T)));
    if (it != m_systemMap.end()) {
        System* systemPtr = it->second;
        m_systemMap.erase(it);

        auto systemIt = std::find_if(m_systems.begin(), m_systems.end(),
            [systemPtr](const std::unique_ptr<System>& sys) {
                return sys.get() == systemPtr;
            });

        if (systemIt != m_systems.end()) {
            m_systems.erase(systemIt);
        }
    }
}

template<typename... ComponentTypes>
std::vector<Entity> EntityManager::GetEntitiesWith() {
    std::vector<Entity> result;

    for (Entity entity : m_entities) {
        bool hasAllComponents = (HasComponent<ComponentTypes>(entity) && ...);
        if (hasAllComponents) {
            result.push_back(entity);
        }
    }

    return result;
}
