#pragma once

#include "Entity.h"
#include <vector>
#include <set>

class EntityManager;

class System {
public:
    virtual ~System() = default;
    
    virtual void Update(float deltaTime) = 0;
    virtual void OnEntityAdded(Entity entity) {}
    virtual void OnEntityRemoved(Entity entity) {}
    
    void SetEntityManager(EntityManager* manager) { m_entityManager = manager; }

protected:
    EntityManager* m_entityManager = nullptr;
    std::set<Entity> m_entities;
    
    void AddEntity(Entity entity) { m_entities.insert(entity); }
    void RemoveEntity(Entity entity) { m_entities.erase(entity); }
    
    const std::set<Entity>& GetEntities() const { return m_entities; }
};

// Forward declarations for common systems
class MovementSystem;
class RenderSystem;
class CollisionSystem;
