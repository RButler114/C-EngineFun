/**
 * @file AnimationSystem.h
 * @brief System that manages sprite animations for entities
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "System.h"
#include "AnimationComponent.h"
#include "Component.h"
#include <functional>
#include <unordered_map>

/**
 * @enum AnimationState
 * @brief Predefined animation states for common game entities
 */
enum class AnimationState {
    IDLE,
    WALKING,
    RUNNING,
    JUMPING,
    FALLING,
    ATTACKING,
    DEFENDING,
    CASTING,
    HURT,
    DYING,
    DEAD,
    CUSTOM_1,
    CUSTOM_2,
    CUSTOM_3
};

/**
 * @struct AnimationStateInfo
 * @brief Information about an animation state transition
 */
struct AnimationStateInfo {
    AnimationState fromState;
    AnimationState toState;
    std::string animationName;
    bool forceRestart;
    float transitionDelay;
    
    AnimationStateInfo(AnimationState from, AnimationState to, const std::string& anim, 
                      bool restart = false, float delay = 0.0f)
        : fromState(from), toState(to), animationName(anim), forceRestart(restart), transitionDelay(delay) {}
};

/**
 * @class AnimationSystem
 * @brief System that processes entities with AnimationComponent and SpriteComponent
 *
 * The AnimationSystem handles:
 * - Frame-by-frame animation updates based on timing
 * - Animation state machine transitions
 * - Automatic sprite frame updates for SpriteComponent
 * - Animation event callbacks (start, end, loop)
 * - Ping-pong and looping animation modes
 * - Animation blending and transitions
 *
 * Features:
 * - Integrates with existing AnimationComponent
 * - Updates SpriteComponent frame data automatically
 * - Supports complex animation state machines
 * - Provides animation event system for gameplay integration
 * - Optimized for arcade-style games
 *
 * @example
 * ```cpp
 * // Add animation system
 * auto* animSystem = entityManager.AddSystem<AnimationSystem>();
 *
 * // Create animated entity
 * Entity player = entityManager.CreateEntity();
 * entityManager.AddComponent<TransformComponent>(player, 100.0f, 200.0f);
 * entityManager.AddComponent<SpriteComponent>(player, "player.png", 32, 32);
 * 
 * auto* anim = entityManager.AddComponent<AnimationComponent>(player);
 * 
 * // Create walk animation
 * Animation walkAnim("walk", true);
 * walkAnim.frames.push_back(AnimationFrame(0, 0, 32, 32, 0.1f));
 * walkAnim.frames.push_back(AnimationFrame(32, 0, 32, 32, 0.1f));
 * walkAnim.frames.push_back(AnimationFrame(64, 0, 32, 32, 0.1f));
 * anim->animations["walk"] = walkAnim;
 * 
 * // Start animation
 * anim->PlayAnimation("walk");
 * ```
 */
class AnimationSystem : public System {
public:
    /**
     * @brief Animation event callback function type
     * @param entity The entity that triggered the event
     * @param animationName Name of the animation
     * @param eventType Type of event ("start", "end", "loop", "frame")
     * @param frameIndex Current frame index (for "frame" events)
     */
    using AnimationEventCallback = std::function<void(Entity entity, const std::string& animationName, 
                                                    const std::string& eventType, int frameIndex)>;

    /**
     * @brief Constructor
     */
    AnimationSystem();

    /**
     * @brief Update all animated entities
     * @param deltaTime Time elapsed since last update
     */
    void Update(float deltaTime) override;

    /**
     * @brief Set callback for animation events
     * @param callback Function to call when animation events occur
     */
    void SetAnimationEventCallback(AnimationEventCallback callback) {
        m_eventCallback = callback;
    }

    /**
     * @brief Create a simple animation from sprite sheet parameters
     * @param name Animation name
     * @param startFrame Starting frame index
     * @param frameCount Number of frames
     * @param frameWidth Width of each frame
     * @param frameHeight Height of each frame
     * @param frameDuration Duration of each frame in seconds
     * @param framesPerRow Frames per row in sprite sheet
     * @param loop Whether animation should loop
     * @return Created Animation object
     */
    static Animation CreateAnimation(const std::string& name, int startFrame, int frameCount,
                                   int frameWidth, int frameHeight, float frameDuration = 0.1f,
                                   int framesPerRow = 8, bool loop = true);

    /**
     * @brief Create animation state machine for an entity
     * @param entity Entity to create state machine for
     * @param initialState Starting animation state
     */
    void CreateAnimationStateMachine(Entity entity, AnimationState initialState = AnimationState::IDLE);

    /**
     * @brief Transition entity to new animation state
     * @param entity Entity to transition
     * @param newState New animation state
     * @param forceRestart Whether to restart animation if already in this state
     */
    void TransitionToState(Entity entity, AnimationState newState, bool forceRestart = false);

    /**
     * @brief Get current animation state of entity
     * @param entity Entity to check
     * @return Current animation state
     */
    AnimationState GetCurrentState(Entity entity) const;

private:
    AnimationEventCallback m_eventCallback;
    std::unordered_map<uint32_t, AnimationState> m_entityStates; ///< Current state per entity
    std::unordered_map<uint32_t, float> m_stateTransitionTimers; ///< Transition delay timers

    /**
     * @brief Update a single entity's animation
     * @param entity Entity to update
     * @param animComp Animation component
     * @param spriteComp Sprite component (optional)
     * @param deltaTime Time elapsed
     */
    void UpdateEntityAnimation(Entity entity, AnimationComponent* animComp, 
                             SpriteComponent* spriteComp, float deltaTime);

    /**
     * @brief Update sprite component with current animation frame
     * @param spriteComp Sprite component to update
     * @param frame Current animation frame
     */
    void UpdateSpriteFrame(SpriteComponent* spriteComp, const AnimationFrame* frame);

    /**
     * @brief Trigger animation event
     * @param entity Entity that triggered event
     * @param animationName Animation name
     * @param eventType Event type
     * @param frameIndex Frame index (for frame events)
     */
    void TriggerEvent(Entity entity, const std::string& animationName, 
                     const std::string& eventType, int frameIndex = -1);

    /**
     * @brief Get default animation name for a state
     * @param state Animation state
     * @return Default animation name
     */
    std::string GetDefaultAnimationForState(AnimationState state) const;
};
