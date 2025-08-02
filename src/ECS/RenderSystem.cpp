#include "ECS/RenderSystem.h"
#include "Engine/Renderer.h"

void RenderSystem::RenderEntity(const TransformComponent* transform, const RenderComponent* render) {
    if (!m_renderer || !transform || !render) {
        return;
    }
    
    // Create rectangle for rendering
    Rectangle rect(
        static_cast<int>(transform->x),
        static_cast<int>(transform->y),
        render->width,
        render->height
    );
    
    // Create color
    Color color(render->r, render->g, render->b, render->a);
    
    // Render the rectangle
    m_renderer->DrawRectangle(rect, color, true);
}
