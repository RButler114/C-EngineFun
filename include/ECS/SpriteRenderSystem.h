/**
 * @file SpriteRenderSystem.h
 * @brief System that renders entities with SpriteComponent
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "System.h"
#include "Component.h"
#include "Engine/Renderer.h"
#include <memory>

/**
 * @class SpriteRenderSystem
 * @brief System that renders sprites for entities with SpriteComponent and TransformComponent
 *
 * The SpriteRenderSystem handles:
 * - Rendering sprites from texture files
 * - Sprite sheet frame rendering with animation support
 * - Scaling, flipping, and transformation
 * - Camera offset support for scrolling games
 * - Depth sorting and layered rendering
 * - Visibility culling for performance
 *
 * Features:
 * - Integrates seamlessly with AnimationSystem
 * - Supports sprite sheets and individual textures
 * - Automatic texture caching through Renderer
 * - Camera-relative positioning
 * - Performance optimized for arcade games
 *
 * @example
 * ```cpp
 * // Add sprite render system
 * auto* spriteSystem = entityManager.AddSystem<SpriteRenderSystem>(renderer);
 * spriteSystem->SetCameraOffset(cameraX, cameraY);
 *
 * // Create sprite entity
 * Entity player = entityManager.CreateEntity();
 * entityManager.AddComponent<TransformComponent>(player, 100.0f, 200.0f);
 * entityManager.AddComponent<SpriteComponent>(player, "player.png", 32, 32);
 * ```
 */
class SpriteRenderSystem : public System {
public:
    /**
     * @brief Constructor
     * @param renderer Pointer to the renderer to use for drawing
     */
    explicit SpriteRenderSystem(Renderer* renderer);

    /**
     * @brief Update system (renders all sprites)
     * @param deltaTime Time elapsed since last update (unused for rendering)
     */
    void Update(float deltaTime) override;

    /**
     * @brief Set camera offset for scrolling games
     * @param cameraX Camera X offset
     * @param cameraY Camera Y offset
     */
    void SetCameraOffset(float cameraX, float cameraY) {
        m_cameraX = cameraX;
        m_cameraY = cameraY;
    }

    /**
     * @brief Get current camera offset
     * @param cameraX Output camera X offset
     * @param cameraY Output camera Y offset
     */
    void GetCameraOffset(float& cameraX, float& cameraY) const {
        cameraX = m_cameraX;
        cameraY = m_cameraY;
    }

    /**
     * @brief Set screen dimensions for culling
     * @param width Screen width
     * @param height Screen height
     */
    void SetScreenDimensions(int width, int height) {
        m_screenWidth = width;
        m_screenHeight = height;
    }

    /**
     * @brief Enable or disable visibility culling
     * @param enabled Whether to enable culling
     */
    void SetCullingEnabled(bool enabled) {
        m_cullingEnabled = enabled;
    }

    /**
     * @brief Set render layer for depth sorting
     * @param layer Render layer (higher values render on top)
     */
    void SetRenderLayer(int layer) {
        m_renderLayer = layer;
    }

private:
    Renderer* m_renderer;
    float m_cameraX = 0.0f;
    float m_cameraY = 0.0f;
    int m_screenWidth = 800;
    int m_screenHeight = 600;
    bool m_cullingEnabled = true;
    int m_renderLayer = 0;

    /**
     * @brief Render a single sprite entity
     * @param entity Entity to render
     * @param transform Transform component
     * @param sprite Sprite component
     */
    void RenderSprite(Entity entity, const TransformComponent* transform, const SpriteComponent* sprite);

    /**
     * @brief Check if sprite is visible on screen
     * @param screenX Screen X position
     * @param screenY Screen Y position
     * @param width Sprite width
     * @param height Sprite height
     * @return True if sprite is visible
     */
    bool IsVisible(int screenX, int screenY, int width, int height) const;
};
