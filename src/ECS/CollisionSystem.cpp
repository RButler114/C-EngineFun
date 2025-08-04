#include "ECS/CollisionSystem.h"
#include <algorithm>
#include <iostream>

void CollisionSystem::Update(float deltaTime) {
    (void)deltaTime; // Not needed for collision detection

    auto entities = m_entityManager->GetEntitiesWith<TransformComponent, CollisionComponent>();

    static int frameCount = 0;
    frameCount++;

    // Debug output every 300 frames (roughly once per 5 seconds at 60 FPS)
    if (frameCount % 300 == 0) {
        std::cout << "CollisionSystem: Found " << entities.size() << " entities with collision components" << std::endl;
    }

    // Check all pairs of entities for collision
    for (size_t i = 0; i < entities.size(); ++i) {
        for (size_t j = i + 1; j < entities.size(); ++j) {
            CheckCollision(entities[i], entities[j]);
        }
    }
}

void CollisionSystem::CheckCollision(Entity entityA, Entity entityB) {
    auto* transformA = m_entityManager->GetComponent<TransformComponent>(entityA);
    auto* collisionA = m_entityManager->GetComponent<CollisionComponent>(entityA);
    auto* transformB = m_entityManager->GetComponent<TransformComponent>(entityB);
    auto* collisionB = m_entityManager->GetComponent<CollisionComponent>(entityB);

    if (!transformA || !collisionA || !transformB || !collisionB) {
        // Debug: Show which components are missing
        if (!transformA) std::cout << "Entity " << entityA.GetID() << " missing TransformComponent" << std::endl;
        if (!collisionA) std::cout << "Entity " << entityA.GetID() << " missing CollisionComponent" << std::endl;
        if (!transformB) std::cout << "Entity " << entityB.GetID() << " missing TransformComponent" << std::endl;
        if (!collisionB) std::cout << "Entity " << entityB.GetID() << " missing CollisionComponent" << std::endl;
        return;
    }
    
    float overlapX, overlapY;
    if (AABB(transformA, collisionA, transformB, collisionB, overlapX, overlapY)) {
        if (m_collisionCallback) {
            CollisionInfo info;
            info.entityA = entityA;
            info.entityB = entityB;
            info.overlapX = overlapX;
            info.overlapY = overlapY;
            m_collisionCallback(info);
        }
    }
}

bool CollisionSystem::AABB(const TransformComponent* transformA, const CollisionComponent* collisionA,
                           const TransformComponent* transformB, const CollisionComponent* collisionB,
                           float& overlapX, float& overlapY) {
    
    // Calculate bounds for entity A
    float leftA = transformA->x;
    float rightA = transformA->x + collisionA->width;
    float topA = transformA->y;
    float bottomA = transformA->y + collisionA->height;
    
    // Calculate bounds for entity B
    float leftB = transformB->x;
    float rightB = transformB->x + collisionB->width;
    float topB = transformB->y;
    float bottomB = transformB->y + collisionB->height;
    
    // Check for collision
    if (leftA < rightB && rightA > leftB && topA < bottomB && bottomA > topB) {
        // Calculate overlap
        overlapX = std::min(rightA, rightB) - std::max(leftA, leftB);
        overlapY = std::min(bottomA, bottomB) - std::max(topA, topB);
        return true;
    }
    
    return false;
}
