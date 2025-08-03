#include "ECS/RenderSystem.h"
#include "Engine/Renderer.h"
#include <iostream>

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

void RenderSystem::RenderSprite(const TransformComponent* transform, const SpriteComponent* sprite) {
    if (!m_renderer || !transform || !sprite) {
        return;
    }



    // Load texture if not already loaded
    const std::string& texturePath = sprite->texturePath;
    auto texture = m_renderer->LoadTexture(texturePath);
    if (!texture) {
        // If texture loading fails, render a placeholder rectangle
        Rectangle rect(
            static_cast<int>(transform->x),
            static_cast<int>(transform->y),
            sprite->width,
            sprite->height
        );
        Color placeholderColor(255, 0, 255, 255); // Magenta placeholder
        m_renderer->DrawRectangle(rect, placeholderColor, true);

        // Log error once per frame (could be optimized)
        static std::string lastFailedTexture;
        if (lastFailedTexture != texturePath) {
            std::cerr << "❌ Failed to load texture: '" << texturePath << "'" << std::endl;
            lastFailedTexture = texturePath;
        }
        return;
    }

    // Debug: Log successful texture loading (commented out to reduce spam)
    // static bool loggedSuccess = false;
    // if (!loggedSuccess) {
    //     std::cout << "✅ Successfully loaded sprite texture: " << texturePath << std::endl;
    //     loggedSuccess = true;
    // }

    // Calculate destination rectangle with scaling
    int destWidth = static_cast<int>(sprite->width * sprite->scaleX * transform->scaleX);
    int destHeight = static_cast<int>(sprite->height * sprite->scaleY * transform->scaleY);

    Rectangle destRect(
        static_cast<int>(transform->x),
        static_cast<int>(transform->y),
        destWidth,
        destHeight
    );

    // Create source rectangle for sprite sheet frame
    Rectangle srcRect(
        sprite->frameX,
        sprite->frameY,
        sprite->frameWidth,
        sprite->frameHeight
    );

    // Debug: Log sprite rendering info (commented out to reduce spam)
    // static int debugFrameCount = 0;
    // if (debugFrameCount < 5) { // Only log first 5 frames
    //     std::cout << "🎨 Rendering sprite: src(" << sprite->frameX << "," << sprite->frameY
    //               << "," << sprite->frameWidth << "," << sprite->frameHeight
    //               << ") dest(" << destRect.x << "," << destRect.y
    //               << "," << destRect.width << "," << destRect.height
    //               << ") flip(" << sprite->flipHorizontal << "," << sprite->flipVertical << ")" << std::endl;
    //     debugFrameCount++;
    // }

    // Render the sprite with flipping support
    m_renderer->DrawTexture(texture, srcRect, destRect, sprite->flipHorizontal, sprite->flipVertical);
}
