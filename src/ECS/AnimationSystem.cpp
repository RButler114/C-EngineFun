#include "ECS/AnimationSystem.h"
#include <iostream>

AnimationSystem::~AnimationSystem() = default;

void AnimationSystem::Update(float deltaTime) {
    // Get all entities with both SpriteComponent and AnimationComponent
    auto entities = m_entityManager->GetEntitiesWith<SpriteComponent, AnimationComponent>();

    static bool firstRun = true;
    if (firstRun) {
        std::cout << "AnimationSystem::Update - Found " << entities.size() << " entities with animation" << std::endl;
        firstRun = false;
    }

    for (Entity entity : entities) {
        auto* sprite = m_entityManager->GetComponent<SpriteComponent>(entity);
        auto* animation = m_entityManager->GetComponent<AnimationComponent>(entity);

        if (sprite && animation) {
            UpdateEntityAnimation(sprite, animation, deltaTime);
        }
    }
}

void AnimationSystem::UpdateEntityAnimation(SpriteComponent* sprite, AnimationComponent* animation, float deltaTime) {
    // Skip if components are null
    if (!sprite || !animation) {
        return;
    }

    // Skip if animation is not playing or is paused
    if (!animation->isPlaying || animation->isPaused) {
        return;
    }

    // Skip if no current animation is set
    if (animation->currentAnimation.empty()) {
        return;
    }

    // Find the current animation data
    auto it = animation->animations.find(animation->currentAnimation);
    if (it == animation->animations.end()) {
        std::cerr << "Animation not found: " << animation->currentAnimation << std::endl;
        animation->StopAnimation();
        return;
    }

    const Animation& animData = it->second;

    // Skip if animation has no frames
    if (animData.frames.empty()) {
        return;
    }

    // Ensure current frame is within bounds
    if (animation->currentFrame < 0 || animation->currentFrame >= static_cast<int>(animData.frames.size())) {
        animation->currentFrame = 0; // Reset to first frame
    }

    // Update frame timer
    animation->frameTimer += deltaTime;

    // Get current frame data
    const AnimationFrame& currentFrameData = animData.frames[animation->currentFrame];

    // Check if it's time to advance to the next frame
    if (animation->frameTimer >= currentFrameData.duration) {
        animation->frameTimer -= currentFrameData.duration;
        AdvanceFrame(animation, animData);
    }

    // Update sprite component with current frame data
    const AnimationFrame* frameData = animation->GetCurrentFrame();
    if (frameData) {
        sprite->frameX = frameData->frameX;
        sprite->frameY = frameData->frameY;
        sprite->frameWidth = frameData->frameWidth;
        sprite->frameHeight = frameData->frameHeight;
    }
}

void AnimationSystem::AdvanceFrame(AnimationComponent* animation, const Animation& animData) {
    int frameCount = static_cast<int>(animData.frames.size());
    
    if (frameCount <= 1) {
        return; // No need to advance if only one frame or no frames
    }

    if (animData.pingPong) {
        // Ping-pong animation logic
        if (!animation->reverse) {
            // Moving forward
            animation->currentFrame++;
            if (animation->currentFrame >= frameCount - 1) {
                animation->currentFrame = frameCount - 1;
                animation->reverse = true;
            }
        } else {
            // Moving backward
            animation->currentFrame--;
            if (animation->currentFrame <= 0) {
                animation->currentFrame = 0;
                animation->reverse = false;
                
                // If not looping, stop the animation
                if (!animData.loop) {
                    animation->StopAnimation();
                }
            }
        }
    } else {
        // Normal forward animation
        animation->currentFrame++;
        
        if (animation->currentFrame >= frameCount) {
            if (animData.loop) {
                animation->currentFrame = 0; // Loop back to start
            } else {
                animation->currentFrame = frameCount - 1; // Stay on last frame
                animation->StopAnimation(); // Stop the animation
            }
        }
    }
}
