#pragma once

#include "System.h"
#include "EntityManager.h"
#include "Component.h"
#include <functional>

struct CollisionInfo {
    Entity entityA;
    Entity entityB;
    float overlapX;
    float overlapY;
};

class CollisionSystem : public System {
public:
    using CollisionCallback = std::function<void(const CollisionInfo&)>;
    
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
    
    void SetCollisionCallback(CollisionCallback callback) {
        m_collisionCallback = callback;
    }

private:
    CollisionCallback m_collisionCallback;
    
    void CheckCollision(Entity entityA, Entity entityB);
    bool AABB(const TransformComponent* transformA, const CollisionComponent* collisionA,
              const TransformComponent* transformB, const CollisionComponent* collisionB,
              float& overlapX, float& overlapY);
};
