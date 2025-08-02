#pragma once

#include "System.h"
#include "EntityManager.h"
#include "Component.h"

class Renderer;

class RenderSystem : public System {
public:
    RenderSystem(Renderer* renderer) : m_renderer(renderer) {}
    
    void Update(float deltaTime) override {
        // This system doesn't need deltaTime for rendering
        (void)deltaTime;
        
        auto entities = m_entityManager->GetEntitiesWith<TransformComponent, RenderComponent>();
        
        for (Entity entity : entities) {
            auto* transform = m_entityManager->GetComponent<TransformComponent>(entity);
            auto* render = m_entityManager->GetComponent<RenderComponent>(entity);
            
            if (transform && render && render->visible) {
                RenderEntity(transform, render);
            }
        }
    }

private:
    Renderer* m_renderer;
    
    void RenderEntity(const TransformComponent* transform, const RenderComponent* render);
};
