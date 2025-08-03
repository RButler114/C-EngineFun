/**
 * @file RenderSystem.h
 * @brief System for rendering entities with visual components
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "System.h"
#include "EntityManager.h"
#include "Component.h"

class Renderer;

/**
 * @class RenderSystem
 * @brief System that renders entities with visual components
 *
 * The RenderSystem processes entities with TransformComponent and either
 * RenderComponent (for colored rectangles) or SpriteComponent (for textures).
 * It supports both traditional rectangle rendering and sprite-based rendering.
 *
 * Features:
 * - Renders colored rectangles based on RenderComponent data
 * - Renders sprites and textures based on SpriteComponent data
 * - Supports sprite sheets with frame-based animation
 * - Respects visibility flags and scaling
 * - Uses transform position for rendering location
 * - Supports RGBA color values and texture blending
 *
 * @example
 * ```cpp
 * // Add to entity manager with renderer
 * entityManager.AddSystem<RenderSystem>(renderer);
 *
 * // Create rectangle entity
 * Entity rect = entityManager.CreateEntity();
 * entityManager.AddComponent<TransformComponent>(rect, 100.0f, 200.0f);
 * entityManager.AddComponent<RenderComponent>(rect, 32, 32, 255, 0, 0); // Red rectangle
 *
 * // Create sprite entity
 * Entity sprite = entityManager.CreateEntity();
 * entityManager.AddComponent<TransformComponent>(sprite, 200.0f, 200.0f);
 * entityManager.AddComponent<SpriteComponent>(sprite, "player.png", 32, 32); // Sprite
 * ```
 */
class RenderSystem : public System {
public:
    /**
     * @brief Constructor with renderer dependency
     *
     * @param renderer Pointer to the renderer used for drawing entities
     *
     * @note The renderer must remain valid for the lifetime of this system
     */
    RenderSystem(Renderer* renderer) : m_renderer(renderer) {}

    /**
     * @brief Render all visible entities
     *
     * Processes all entities with TransformComponent and either RenderComponent
     * or SpriteComponent, drawing them on screen if they are marked as visible.
     *
     * @param deltaTime Time elapsed since last update (unused for rendering)
     *
     * @note Only entities with visible=true are drawn
     */
    void Update(float deltaTime) override {
        // This system doesn't need deltaTime for rendering
        (void)deltaTime;

        // Render entities with RenderComponent (colored rectangles)
        auto renderEntities = m_entityManager->GetEntitiesWith<TransformComponent, RenderComponent>();
        for (Entity entity : renderEntities) {
            auto* transform = m_entityManager->GetComponent<TransformComponent>(entity);
            auto* render = m_entityManager->GetComponent<RenderComponent>(entity);

            if (transform && render && render->visible) {
                RenderEntity(transform, render);
            }
        }

        // Render entities with SpriteComponent (textures/sprites)
        auto spriteEntities = m_entityManager->GetEntitiesWith<TransformComponent, SpriteComponent>();
        for (Entity entity : spriteEntities) {
            auto* transform = m_entityManager->GetComponent<TransformComponent>(entity);
            auto* sprite = m_entityManager->GetComponent<SpriteComponent>(entity);

            if (transform && sprite && sprite->visible) {
                RenderSprite(transform, sprite);
            }
        }
    }

private:
    Renderer* m_renderer; ///< Renderer used for drawing entities

    /**
     * @brief Render a single entity with RenderComponent
     *
     * Internal method that handles the actual rendering of an entity
     * based on its transform and render component data (colored rectangle).
     *
     * @param transform Entity's transform component (position, scale, rotation)
     * @param render Entity's render component (size, color, visibility)
     */
    void RenderEntity(const TransformComponent* transform, const RenderComponent* render);

    /**
     * @brief Render a single entity with SpriteComponent
     *
     * Internal method that handles the actual rendering of a sprite entity
     * based on its transform and sprite component data (texture/sprite).
     *
     * @param transform Entity's transform component (position, scale, rotation)
     * @param sprite Entity's sprite component (texture, frames, scaling)
     */
    void RenderSprite(const TransformComponent* transform, const SpriteComponent* sprite);
};
