#pragma once

#include "Entity.h"
#include "Component.h"
#include "System.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <typeindex>
#include <algorithm>

class EntityManager {
public:
    EntityManager();
    ~EntityManager();

    // Entity management
    Entity CreateEntity();
    void DestroyEntity(Entity entity);
    bool IsEntityValid(Entity entity) const;

    // Component management
    template<typename T, typename... Args>
    T* AddComponent(Entity entity, Args&&... args);
    
    template<typename T>
    T* GetComponent(Entity entity);
    
    template<typename T>
    const T* GetComponent(Entity entity) const;
    
    template<typename T>
    bool HasComponent(Entity entity) const;
    
    template<typename T>
    void RemoveComponent(Entity entity);

    // System management
    template<typename T, typename... Args>
    T* AddSystem(Args&&... args);
    
    template<typename T>
    T* GetSystem();
    
    template<typename T>
    void RemoveSystem();

    // Update all systems
    void Update(float deltaTime);

    // Get all entities with specific components
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
