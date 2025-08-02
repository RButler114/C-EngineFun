#include "ECS/EntityManager.h"
#include <iostream>

EntityManager::EntityManager() : m_nextEntityID(1) {}

EntityManager::~EntityManager() = default;

Entity EntityManager::CreateEntity() {
    Entity entity(m_nextEntityID++);
    m_entities.push_back(entity);
    NotifySystemsEntityAdded(entity);
    return entity;
}

void EntityManager::DestroyEntity(Entity entity) {
    if (IsEntityValid(entity)) {
        m_entitiesToDestroy.push_back(entity);
    }
}

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


