/**
 * @file AnimationFactory.h
 * @brief Factory for creating common game animations
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "ECS/AnimationComponent.h"
#include "ECS/AnimationSystem.h"
#include <string>
#include <map>

/**
 * @struct AnimationConfig
 * @brief Configuration for creating animations
 */
struct AnimationConfig {
    std::string name;
    int startFrame;
    int frameCount;
    int frameWidth;
    int frameHeight;
    float frameDuration;
    int framesPerRow;
    bool loop;
    bool pingPong;
    
    AnimationConfig(const std::string& animName, int start, int count, int width, int height,
                   float duration = 0.1f, int perRow = 8, bool shouldLoop = true, bool shouldPingPong = false)
        : name(animName), startFrame(start), frameCount(count), frameWidth(width), frameHeight(height),
          frameDuration(duration), framesPerRow(perRow), loop(shouldLoop), pingPong(shouldPingPong) {}
};

/**
 * @class AnimationFactory
 * @brief Factory class for creating common game animations
 *
 * The AnimationFactory provides convenient methods for creating standard
 * game animations like walking, jumping, attacking, etc. It supports
 * different sprite sheet layouts and animation patterns.
 *
 * Features:
 * - Predefined animation templates for common game actions
 * - Support for different sprite sheet layouts
 * - Easy configuration through AnimationConfig
 * - Integration with existing AnimationComponent system
 * - Batch animation creation for complete character sets
 *
 * @example
 * ```cpp
 * AnimationFactory factory;
 * 
 * // Create player animations
 * auto playerAnims = factory.CreatePlayerAnimations(32, 32);
 * 
 * // Add to entity
 * auto* animComp = entityManager.AddComponent<AnimationComponent>(player);
 * for (const auto& [name, anim] : playerAnims) {
 *     animComp->animations[name] = anim;
 * }
 * ```
 */
class AnimationFactory {
public:
    /**
     * @brief Create a complete set of player animations
     * @param frameWidth Width of each frame
     * @param frameHeight Height of each frame
     * @param frameDuration Duration of each frame
     * @return Map of animation name to Animation object
     */
    static std::map<std::string, Animation> CreatePlayerAnimations(int frameWidth, int frameHeight, 
                                                                  float frameDuration = 0.15f);

    /**
     * @brief Create a complete set of enemy animations
     * @param frameWidth Width of each frame
     * @param frameHeight Height of each frame
     * @param frameDuration Duration of each frame
     * @return Map of animation name to Animation object
     */
    static std::map<std::string, Animation> CreateEnemyAnimations(int frameWidth, int frameHeight,
                                                                 float frameDuration = 0.2f);

    /**
     * @brief Create combat-specific animations
     * @param frameWidth Width of each frame
     * @param frameHeight Height of each frame
     * @param frameDuration Duration of each frame
     * @return Map of animation name to Animation object
     */
    static std::map<std::string, Animation> CreateCombatAnimations(int frameWidth, int frameHeight,
                                                                  float frameDuration = 0.1f);

    /**
     * @brief Create animation from configuration
     * @param config Animation configuration
     * @return Created Animation object
     */
    static Animation CreateFromConfig(const AnimationConfig& config);

    /**
     * @brief Create simple walking animation
     * @param frameWidth Width of each frame
     * @param frameHeight Height of each frame
     * @param frameCount Number of frames in walk cycle
     * @param frameDuration Duration of each frame
     * @param startFrame Starting frame index
     * @return Walking Animation object
     */
    static Animation CreateWalkAnimation(int frameWidth, int frameHeight, int frameCount = 4,
                                       float frameDuration = 0.15f, int startFrame = 0);

    /**
     * @brief Create idle animation (single frame or subtle movement)
     * @param frameWidth Width of each frame
     * @param frameHeight Height of each frame
     * @param frameCount Number of frames (1 for static, more for breathing effect)
     * @param frameDuration Duration of each frame
     * @param startFrame Starting frame index
     * @return Idle Animation object
     */
    static Animation CreateIdleAnimation(int frameWidth, int frameHeight, int frameCount = 1,
                                       float frameDuration = 1.0f, int startFrame = 0);

    /**
     * @brief Create attack animation
     * @param frameWidth Width of each frame
     * @param frameHeight Height of each frame
     * @param frameCount Number of frames in attack
     * @param frameDuration Duration of each frame
     * @param startFrame Starting frame index
     * @return Attack Animation object
     */
    static Animation CreateAttackAnimation(int frameWidth, int frameHeight, int frameCount = 3,
                                         float frameDuration = 0.1f, int startFrame = 0);

    /**
     * @brief Create death animation
     * @param frameWidth Width of each frame
     * @param frameHeight Height of each frame
     * @param frameCount Number of frames in death sequence
     * @param frameDuration Duration of each frame
     * @param startFrame Starting frame index
     * @return Death Animation object (non-looping)
     */
    static Animation CreateDeathAnimation(int frameWidth, int frameHeight, int frameCount = 4,
                                        float frameDuration = 0.2f, int startFrame = 0);

    /**
     * @brief Create hurt/damage animation
     * @param frameWidth Width of each frame
     * @param frameHeight Height of each frame
     * @param frameCount Number of frames
     * @param frameDuration Duration of each frame
     * @param startFrame Starting frame index
     * @return Hurt Animation object (non-looping)
     */
    static Animation CreateHurtAnimation(int frameWidth, int frameHeight, int frameCount = 2,
                                       float frameDuration = 0.1f, int startFrame = 0);

    /**
     * @brief Create jump animation
     * @param frameWidth Width of each frame
     * @param frameHeight Height of each frame
     * @param frameCount Number of frames
     * @param frameDuration Duration of each frame
     * @param startFrame Starting frame index
     * @return Jump Animation object (non-looping)
     */
    static Animation CreateJumpAnimation(int frameWidth, int frameHeight, int frameCount = 3,
                                       float frameDuration = 0.15f, int startFrame = 0);

private:
    /**
     * @brief Helper to create animation with standard parameters
     * @param name Animation name
     * @param startFrame Starting frame index
     * @param frameCount Number of frames
     * @param frameWidth Width of each frame
     * @param frameHeight Height of each frame
     * @param frameDuration Duration of each frame
     * @param loop Whether to loop
     * @param pingPong Whether to ping-pong
     * @return Created Animation object
     */
    static Animation CreateStandardAnimation(const std::string& name, int startFrame, int frameCount,
                                           int frameWidth, int frameHeight, float frameDuration,
                                           bool loop = true, bool pingPong = false);
};
