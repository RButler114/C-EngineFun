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
 * @brief System that renders entities with TransformComponent and RenderComponent
 *
 * The RenderSystem processes all entities that have both TransformComponent
 * and RenderComponent, drawing them on screen using the provided Renderer.
 *
 * Features:
 * - Renders colored rectangles based on component data
 * - Respects visibility flags
 * - Uses transform position for rendering location
 * - Supports RGBA color values
 *
 * @example
 * ```cpp
 * // Add to entity manager with renderer
 * entityManager.AddSystem<RenderSystem>(renderer);
 *
 * // Create visible entity
 * Entity entity = entityManager.CreateEntity();
 * entityManager.AddComponent<TransformComponent>(entity, 100.0f, 200.0f);
 * entityManager.AddComponent<RenderComponent>(entity, 32, 32, 255, 0, 0); // Red 32x32 rectangle
 *
 * // System will automatically render entity each frame
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
     * Processes all entities with TransformComponent and RenderComponent,
     * drawing them on screen if they are marked as visible.
     *
     * @param deltaTime Time elapsed since last update (unused for rendering)
     *
     * @note Only entities with visible=true in their RenderComponent are drawn
     */
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
    Renderer* m_renderer; ///< Renderer used for drawing entities

    /**
     * @brief Render a single entity
     *
     * Internal method that handles the actual rendering of an entity
     * based on its transform and render component data.
     *
     * @param transform Entity's transform component (position, scale, rotation)
     * @param render Entity's render component (size, color, visibility)
     */
    void RenderEntity(const TransformComponent* transform, const RenderComponent* render);
};
