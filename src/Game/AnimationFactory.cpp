/**
 * @file AnimationFactory.cpp
 * @brief Implementation of the AnimationFactory
 * @author Ryan Butler
 * @date 2025
 */

#include "Game/AnimationFactory.h"
#include <iostream>

std::map<std::string, Animation> AnimationFactory::CreatePlayerAnimations(int frameWidth, int frameHeight, float frameDuration) {
    std::cout << "🎬 DEBUG: Starting CreatePlayerAnimations with dimensions " << frameWidth << "x" << frameHeight << std::endl;
    std::map<std::string, Animation> animations;

    std::cout << "🎬 DEBUG: Creating idle animation..." << std::endl;
    animations["idle"] = CreateIdleAnimation(frameWidth, frameHeight, 1, 1.0f, 0);
    std::cout << "🎬 DEBUG: Idle animation created successfully" << std::endl;

    std::cout << "🎬 DEBUG: Creating walk animation..." << std::endl;
    animations["walk"] = CreateWalkAnimation(frameWidth, frameHeight, 3, frameDuration, 0);
    std::cout << "🎬 DEBUG: Walk animation created successfully" << std::endl;

    std::cout << "🎬 DEBUG: Creating jump animation..." << std::endl;
    animations["jump"] = CreateJumpAnimation(frameWidth, frameHeight, 1, 0.3f, 1);
    std::cout << "🎬 DEBUG: Jump animation created successfully" << std::endl;

    std::cout << "🎬 DEBUG: Creating attack animation..." << std::endl;
    animations["attack"] = CreateAttackAnimation(frameWidth, frameHeight, 2, 0.1f, 0);
    std::cout << "🎬 DEBUG: Attack animation created successfully" << std::endl;

    std::cout << "🎬 DEBUG: Creating hurt animation..." << std::endl;
    animations["hurt"] = CreateHurtAnimation(frameWidth, frameHeight, 1, 0.2f, 0);
    std::cout << "🎬 DEBUG: Hurt animation created successfully" << std::endl;

    std::cout << "Created " << animations.size() << " player animations" << std::endl;
    std::cout << "🎬 DEBUG: Returning animations map..." << std::endl;
    return animations;
}

std::map<std::string, Animation> AnimationFactory::CreateEnemyAnimations(int frameWidth, int frameHeight, float frameDuration) {
    std::map<std::string, Animation> animations;
    
    // Basic enemy animations
    animations["idle"] = CreateIdleAnimation(frameWidth, frameHeight, 1, 2.0f, 0);
    animations["move"] = CreateWalkAnimation(frameWidth, frameHeight, 2, frameDuration, 0);
    animations["attack"] = CreateAttackAnimation(frameWidth, frameHeight, 2, 0.15f, 0);
    animations["hurt"] = CreateHurtAnimation(frameWidth, frameHeight, 1, 0.15f, 0);
    animations["die"] = CreateDeathAnimation(frameWidth, frameHeight, 2, 0.3f, 0);
    
    std::cout << "Created " << animations.size() << " enemy animations" << std::endl;
    return animations;
}

std::map<std::string, Animation> AnimationFactory::CreateCombatAnimations(int frameWidth, int frameHeight, float frameDuration) {
    std::map<std::string, Animation> animations;
    
    // Combat-specific animations
    animations["attack"] = CreateAttackAnimation(frameWidth, frameHeight, 3, frameDuration, 0);
    animations["defend"] = CreateStandardAnimation("defend", 0, 1, frameWidth, frameHeight, 0.5f, true, false);
    animations["cast"] = CreateStandardAnimation("cast", 0, 4, frameWidth, frameHeight, frameDuration, false, false);
    animations["hurt"] = CreateHurtAnimation(frameWidth, frameHeight, 2, 0.1f, 0);
    animations["victory"] = CreateStandardAnimation("victory", 0, 3, frameWidth, frameHeight, 0.3f, true, false);
    animations["defeat"] = CreateDeathAnimation(frameWidth, frameHeight, 4, 0.25f, 0);
    
    std::cout << "Created " << animations.size() << " combat animations" << std::endl;
    return animations;
}

Animation AnimationFactory::CreateFromConfig(const AnimationConfig& config) {
    Animation animation(config.name, config.loop);
    animation.pingPong = config.pingPong;
    
    for (int i = 0; i < config.frameCount; ++i) {
        int frameIndex = config.startFrame + i;
        int row = frameIndex / config.framesPerRow;
        int col = frameIndex % config.framesPerRow;
        
        int x = col * config.frameWidth;
        int y = row * config.frameHeight;
        
        animation.frames.emplace_back(x, y, config.frameWidth, config.frameHeight, config.frameDuration);
    }
    
    return animation;
}

Animation AnimationFactory::CreateWalkAnimation(int frameWidth, int frameHeight, int frameCount, float frameDuration, int startFrame) {
    return CreateStandardAnimation("walk", startFrame, frameCount, frameWidth, frameHeight, frameDuration, true, false);
}

Animation AnimationFactory::CreateIdleAnimation(int frameWidth, int frameHeight, int frameCount, float frameDuration, int startFrame) {
    return CreateStandardAnimation("idle", startFrame, frameCount, frameWidth, frameHeight, frameDuration, true, false);
}

Animation AnimationFactory::CreateAttackAnimation(int frameWidth, int frameHeight, int frameCount, float frameDuration, int startFrame) {
    return CreateStandardAnimation("attack", startFrame, frameCount, frameWidth, frameHeight, frameDuration, false, false);
}

Animation AnimationFactory::CreateDeathAnimation(int frameWidth, int frameHeight, int frameCount, float frameDuration, int startFrame) {
    return CreateStandardAnimation("die", startFrame, frameCount, frameWidth, frameHeight, frameDuration, false, false);
}

Animation AnimationFactory::CreateHurtAnimation(int frameWidth, int frameHeight, int frameCount, float frameDuration, int startFrame) {
    return CreateStandardAnimation("hurt", startFrame, frameCount, frameWidth, frameHeight, frameDuration, false, false);
}

Animation AnimationFactory::CreateJumpAnimation(int frameWidth, int frameHeight, int frameCount, float frameDuration, int startFrame) {
    return CreateStandardAnimation("jump", startFrame, frameCount, frameWidth, frameHeight, frameDuration, false, false);
}

Animation AnimationFactory::CreateStandardAnimation(const std::string& name, int startFrame, int frameCount,
                                                   int frameWidth, int frameHeight, float frameDuration,
                                                   bool loop, bool pingPong) {
    Animation animation(name, loop);
    animation.pingPong = pingPong;
    
    // Assume horizontal sprite sheet layout (8 frames per row by default)
    int framesPerRow = 8;
    
    for (int i = 0; i < frameCount; ++i) {
        int frameIndex = startFrame + i;
        int row = frameIndex / framesPerRow;
        int col = frameIndex % framesPerRow;
        
        int x = col * frameWidth;
        int y = row * frameHeight;
        
        animation.frames.emplace_back(x, y, frameWidth, frameHeight, frameDuration);
    }
    
    return animation;
}
