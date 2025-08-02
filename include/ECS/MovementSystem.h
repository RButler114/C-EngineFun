#pragma once

#include "System.h"
#include "EntityManager.h"
#include "Component.h"
#include <iostream>

class MovementSystem : public System {
public:
    void Update(float deltaTime) override {
        auto entities = m_entityManager->GetEntitiesWith<TransformComponent, VelocityComponent>();

        for (Entity entity : entities) {
            auto* transform = m_entityManager->GetComponent<TransformComponent>(entity);
            auto* velocity = m_entityManager->GetComponent<VelocityComponent>(entity);

            if (transform && velocity) {
                transform->x += velocity->vx * deltaTime;
                transform->y += velocity->vy * deltaTime;
            }
        }
    }
};
