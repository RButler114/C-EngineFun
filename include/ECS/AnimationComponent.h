/**
 * @file AnimationComponent.h
 * @brief Component for sprite animation data
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "Component.h"
#include <vector>
#include <string>
#include <map>

/**
 * @struct AnimationFrame
 * @brief Represents a single frame in an animation sequence
 */
struct AnimationFrame {
    int frameX = 0;         ///< X offset in sprite sheet
    int frameY = 0;         ///< Y offset in sprite sheet
    int frameWidth = 32;    ///< Width of frame
    int frameHeight = 32;   ///< Height of frame
    float duration = 0.1f;  ///< Duration to display this frame (seconds)

    AnimationFrame() = default;
    AnimationFrame(int x, int y, int w, int h, float dur = 0.1f)
        : frameX(x), frameY(y), frameWidth(w), frameHeight(h), duration(dur) {}
};

/**
 * @struct Animation
 * @brief Represents a complete animation sequence
 */
struct Animation {
    std::string name;                       ///< Name of the animation
    std::vector<AnimationFrame> frames;     ///< Frames in the animation
    bool loop = true;                       ///< Whether animation should loop
    bool pingPong = false;                  ///< Whether to reverse when reaching end

    Animation() = default;
    Animation(const std::string& animName, bool shouldLoop = true)
        : name(animName), loop(shouldLoop) {}
};

/**
 * @struct AnimationComponent
 * @brief Component that manages sprite animations
 *
 * Contains animation data and state for entities that need animated sprites.
 * Works in conjunction with SpriteComponent to update frame data over time.
 *
 * Features:
 * - Multiple named animations per entity
 * - Frame-based animation with configurable timing
 * - Looping and ping-pong animation modes
 * - Animation state management (play, pause, stop)
 * - Current frame tracking and timing
 *
 * @example
 * ```cpp
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
struct AnimationComponent : public Component {
    std::map<std::string, Animation> animations;    ///< Available animations by name
    std::string currentAnimation;                   ///< Name of currently playing animation
    int currentFrame = 0;                           ///< Current frame index
    float frameTimer = 0.0f;                        ///< Time accumulated for current frame
    bool isPlaying = false;                         ///< Whether animation is currently playing
    bool isPaused = false;                          ///< Whether animation is paused
    bool reverse = false;                           ///< Whether playing in reverse (for ping-pong)

    /**
     * @brief Default constructor
     */
    AnimationComponent() = default;

    /**
     * @brief Play an animation by name
     * @param animationName Name of animation to play
     * @param restart Whether to restart if already playing this animation
     */
    void PlayAnimation(const std::string& animationName, bool restart = false) {
        if (animations.find(animationName) == animations.end()) {
            return; // Animation not found
        }

        if (currentAnimation != animationName || restart) {
            currentAnimation = animationName;
            currentFrame = 0;
            frameTimer = 0.0f;
            reverse = false;
        }
        
        isPlaying = true;
        isPaused = false;
    }

    /**
     * @brief Pause the current animation
     */
    void PauseAnimation() {
        isPaused = true;
    }

    /**
     * @brief Resume the current animation
     */
    void ResumeAnimation() {
        isPaused = false;
    }

    /**
     * @brief Stop the current animation
     */
    void StopAnimation() {
        isPlaying = false;
        isPaused = false;
        currentFrame = 0;
        frameTimer = 0.0f;
        reverse = false;
    }

    /**
     * @brief Get the current animation frame
     * @return Pointer to current frame, or nullptr if no animation is playing
     */
    const AnimationFrame* GetCurrentFrame() const {
        if (!isPlaying || currentAnimation.empty()) {
            return nullptr;
        }

        auto it = animations.find(currentAnimation);
        if (it == animations.end() || it->second.frames.empty()) {
            return nullptr;
        }

        if (currentFrame >= 0 && currentFrame < static_cast<int>(it->second.frames.size())) {
            return &it->second.frames[currentFrame];
        }

        return nullptr;
    }

    /**
     * @brief Check if an animation is currently playing
     * @param animationName Name of animation to check (empty = any animation)
     * @return True if the specified animation (or any animation) is playing
     */
    bool IsPlaying(const std::string& animationName = "") const {
        if (animationName.empty()) {
            return isPlaying && !isPaused;
        }
        return isPlaying && !isPaused && currentAnimation == animationName;
    }
};
