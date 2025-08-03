/**
 * @file AnimationSystem.h
 * @brief System for updating sprite animations
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "System.h"
#include "EntityManager.h"
#include "Component.h"
#include "AnimationComponent.h"

/**
 * @class AnimationSystem
 * @brief System that updates sprite animations over time
 *
 * The AnimationSystem processes all entities that have both SpriteComponent
 * and AnimationComponent, updating their animation frames based on timing
 * and animation settings.
 *
 * Features:
 * - Frame-based animation with configurable timing
 * - Looping and ping-pong animation modes
 * - Automatic sprite frame updates
 * - Animation state management
 * - Support for multiple animations per entity
 *
 * @example
 * ```cpp
 * // Add to entity manager
 * entityManager.AddSystem<AnimationSystem>();
 *
 * // Create animated entity
 * Entity player = entityManager.CreateEntity();
 * entityManager.AddComponent<TransformComponent>(player, 100.0f, 200.0f);
 * entityManager.AddComponent<SpriteComponent>(player, "player.png", 32, 32);
 * 
 * auto* anim = entityManager.AddComponent<AnimationComponent>(player);
 * Animation walkAnim("walk", true);
 * walkAnim.frames.push_back(AnimationFrame(0, 0, 32, 32, 0.1f));
 * walkAnim.frames.push_back(AnimationFrame(32, 0, 32, 32, 0.1f));
 * anim->animations["walk"] = walkAnim;
 * anim->PlayAnimation("walk");
 * ```
 */
class AnimationSystem : public System {
public:
    /**
     * @brief Default constructor
     */
    AnimationSystem() = default;

    /**
     * @brief Virtual destructor
     */
    virtual ~AnimationSystem();

    /**
     * @brief Update all animated entities
     *
     * Processes all entities with SpriteComponent and AnimationComponent,
     * updating their animation frames based on elapsed time and animation settings.
     *
     * @param deltaTime Time elapsed since last update in seconds
     */
    void Update(float deltaTime) override;

private:
    /**
     * @brief Update animation for a single entity
     *
     * Internal method that handles the animation logic for one entity,
     * including frame advancement, looping, and sprite frame updates.
     *
     * @param sprite Entity's sprite component
     * @param animation Entity's animation component
     * @param deltaTime Time elapsed since last update
     */
    void UpdateEntityAnimation(SpriteComponent* sprite, AnimationComponent* animation, float deltaTime);

    /**
     * @brief Advance to the next frame in an animation
     *
     * Handles frame advancement logic including looping and ping-pong modes.
     *
     * @param animation Animation component to advance
     * @param animData Current animation data
     */
    void AdvanceFrame(AnimationComponent* animation, const Animation& animData);
};
