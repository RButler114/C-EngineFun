/**
 * @file SpriteRenderSystem.cpp
 * @brief Implementation of the SpriteRenderSystem
 * @author Ryan Butler
 * @date 2025
 */

#include "ECS/SpriteRenderSystem.h"
#include "ECS/EntityManager.h"
#include <iostream>
#include <algorithm>

SpriteRenderSystem::SpriteRenderSystem(Renderer* renderer) 
    : m_renderer(renderer) {
    if (!m_renderer) {
        std::cerr << "Warning: SpriteRenderSystem created with null renderer!" << std::endl;
    }
    std::cout << "SpriteRenderSystem created" << std::endl;
}

void SpriteRenderSystem::Update(float deltaTime) {
    (void)deltaTime; // Suppress unused parameter warning

    if (!m_renderer) {
        return;
    }

    // Debug: Check if Update is being called - use a different counter
    static int updateCount = 0;
    updateCount++;
    if (updateCount == 1) {
        std::cout << "🎨 SpriteRenderSystem::Update called for the FIRST time!" << std::endl;
    }
    if (updateCount % 60 == 0) { // Every 60 frames (1 second at 60fps)
        std::cout << "🎨 SpriteRenderSystem::Update called " << updateCount << " times" << std::endl;
    }

    // Get all entities with both TransformComponent and SpriteComponent
    auto entities = m_entityManager->GetEntitiesWith<TransformComponent, SpriteComponent>();

    // Debug: Print sprite entities found (every 60 frames)
    if (updateCount % 60 == 0) {
        std::cout << "🎨 SpriteRenderSystem: Found " << entities.size() << " entities with sprites" << std::endl;
        for (Entity entity : entities) {
            auto* sprite = m_entityManager->GetComponent<SpriteComponent>(entity);
            if (sprite) {
                std::cout << "  📽️  Entity " << entity.GetID() << ": " << sprite->texturePath
                          << " (visible: " << sprite->visible << ")" << std::endl;
            }
        }

        // Also check if player entity has the components individually
        Entity playerEntity(1); // We know player is entity 1
        auto* playerTransform = m_entityManager->GetComponent<TransformComponent>(playerEntity);
        auto* playerSprite = m_entityManager->GetComponent<SpriteComponent>(playerEntity);
        bool hasTransform = m_entityManager->HasComponent<TransformComponent>(playerEntity);
        bool hasSprite = m_entityManager->HasComponent<SpriteComponent>(playerEntity);

        std::cout << "🎮 Player Entity 1 - Transform: " << (playerTransform ? "✅" : "❌")
                  << ", Sprite: " << (playerSprite ? "✅" : "❌") << std::endl;
        std::cout << "🎮 Player Entity 1 - HasTransform: " << (hasTransform ? "✅" : "❌")
                  << ", HasSprite: " << (hasSprite ? "✅" : "❌") << std::endl;
        if (playerSprite) {
            std::cout << "🎮 Player sprite path: '" << playerSprite->texturePath << "'" << std::endl;
        }
    }

    // TODO: In the future, we could sort entities by render layer or depth here
    // For now, render in the order they're returned

    for (Entity entity : entities) {
        auto* transform = m_entityManager->GetComponent<TransformComponent>(entity);
        auto* sprite = m_entityManager->GetComponent<SpriteComponent>(entity);

        if (transform && sprite && sprite->visible) {
            // Safety check: Don't render if sprite data looks corrupted
            if (sprite->texturePath.length() > 0 && sprite->texturePath.length() < 256) {
                try {
                    RenderSprite(entity, transform, sprite);
                } catch (...) {
                    std::cout << "🚨 CRASH PREVENTED: Exception in RenderSprite for entity " << entity.GetID() << std::endl;
                }
            } else {
                std::cout << "🚨 SKIPPING RENDER: Corrupted sprite for entity " << entity.GetID() << std::endl;
            }
        }
    }
}

void SpriteRenderSystem::RenderSprite(Entity entity, const TransformComponent* transform, const SpriteComponent* sprite) {
    // Calculate screen position with camera offset
    int screenX = static_cast<int>(transform->x - m_cameraX);
    int screenY = static_cast<int>(transform->y - m_cameraY);
    
    // Calculate final sprite dimensions with scaling
    int finalWidth = static_cast<int>(sprite->width * sprite->scaleX * transform->scaleX);
    int finalHeight = static_cast<int>(sprite->height * sprite->scaleY * transform->scaleY);
    
    // Perform visibility culling if enabled
    if (m_cullingEnabled && !IsVisible(screenX, screenY, finalWidth, finalHeight)) {
        return;
    }
    
    // Debug: Check if texture path is valid and not corrupted
    if (sprite->texturePath.empty() ||
        sprite->texturePath.find('\0') != std::string::npos ||
        sprite->texturePath.length() > 256 ||  // Reasonable path length limit
        sprite->texturePath.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._/-") != std::string::npos) {

        // Corrupted texture path - render placeholder and log error
        std::cout << "🚨 CORRUPTED SPRITE PATH for Entity " << entity.GetID()
                  << ": '" << sprite->texturePath << "' (length: " << sprite->texturePath.length() << ")" << std::endl;
        Rectangle rect(screenX, screenY, finalWidth, finalHeight);
        Color placeholderColor(255, 0, 255, 255); // Magenta placeholder for corrupted path
        m_renderer->DrawRectangle(rect, placeholderColor, true);
        return;
    }

    // Load texture
    auto texture = m_renderer->LoadTexture(sprite->texturePath);
    if (!texture) {
        // Debug: Log texture loading failure
        std::cout << "🚨 TEXTURE LOAD FAILED for Entity " << entity.GetID()
                  << ": '" << sprite->texturePath << "'" << std::endl;

        // Render placeholder rectangle if texture fails to load
        Rectangle rect(screenX, screenY, finalWidth, finalHeight);
        Color placeholderColor(255, 100, 255, 255); // Slightly different magenta for missing file
        m_renderer->DrawRectangle(rect, placeholderColor, true);
        std::cout << "🎨 PLACEHOLDER: Rendered magenta rectangle at (" << screenX << ", " << screenY
                  << ") size " << finalWidth << "x" << finalHeight << std::endl;
        return;
    } else {
        // Debug: Log successful texture loading
        std::cout << "✅ TEXTURE LOADED for Entity " << entity.GetID()
                  << ": '" << sprite->texturePath << "'" << std::endl;
    }
    
    // Set up source rectangle (from sprite sheet)
    Rectangle srcRect(sprite->frameX, sprite->frameY, sprite->frameWidth, sprite->frameHeight);
    
    // Set up destination rectangle (on screen)
    Rectangle destRect(screenX, screenY, finalWidth, finalHeight);
    
    // Render with flipping support
    m_renderer->DrawTexture(texture, srcRect, destRect, sprite->flipHorizontal, sprite->flipVertical);
}

bool SpriteRenderSystem::IsVisible(int screenX, int screenY, int width, int height) const {
    // Simple AABB visibility test
    return !(screenX + width < 0 || 
             screenX > m_screenWidth || 
             screenY + height < 0 || 
             screenY > m_screenHeight);
}
