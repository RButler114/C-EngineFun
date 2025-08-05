/**
 * @file AnimationSystem.cpp
 * @brief Implementation of the AnimationSystem
 * @author Ryan Butler
 * @date 2025
 */

#include "ECS/AnimationSystem.h"
#include "ECS/EntityManager.h"
#include <iostream>
#include <algorithm>

AnimationSystem::AnimationSystem() : m_eventCallback(nullptr) {
    std::cout << "AnimationSystem created" << std::endl;
}

void AnimationSystem::Update(float deltaTime) {
    // Get all entities with AnimationComponent
    auto entities = m_entityManager->GetEntitiesWith<AnimationComponent>();
    
    for (Entity entity : entities) {
        auto* animComp = m_entityManager->GetComponent<AnimationComponent>(entity);
        auto* spriteComp = m_entityManager->GetComponent<SpriteComponent>(entity);
        
        if (animComp) {
            UpdateEntityAnimation(entity, animComp, spriteComp, deltaTime);
        }
    }
    
    // Update state transition timers
    for (auto it = m_stateTransitionTimers.begin(); it != m_stateTransitionTimers.end();) {
        it->second -= deltaTime;
        if (it->second <= 0.0f) {
            it = m_stateTransitionTimers.erase(it);
        } else {
            ++it;
        }
    }
}

void AnimationSystem::UpdateEntityAnimation(Entity entity, AnimationComponent* animComp, 
                                          SpriteComponent* spriteComp, float deltaTime) {
    if (!animComp->isPlaying || animComp->isPaused || animComp->currentAnimation.empty()) {
        return;
    }
    
    // Find current animation
    auto animIt = animComp->animations.find(animComp->currentAnimation);
    if (animIt == animComp->animations.end() || animIt->second.frames.empty()) {
        return;
    }
    
    const Animation& currentAnim = animIt->second;
    
    // Update frame timer
    animComp->frameTimer += deltaTime;
    
    // Check if we need to advance to next frame
    if (animComp->currentFrame >= 0 && animComp->currentFrame < static_cast<int>(currentAnim.frames.size())) {
        const AnimationFrame& currentFrame = currentAnim.frames[animComp->currentFrame];
        
        if (animComp->frameTimer >= currentFrame.duration) {
            // Advance frame
            animComp->frameTimer = 0.0f;
            
            if (!animComp->reverse) {
                // Forward direction
                animComp->currentFrame++;
                
                // Check if we've reached the end
                if (animComp->currentFrame >= static_cast<int>(currentAnim.frames.size())) {
                    if (currentAnim.pingPong) {
                        // Start going backwards
                        animComp->reverse = true;
                        animComp->currentFrame = std::max(0, static_cast<int>(currentAnim.frames.size()) - 2);
                        TriggerEvent(entity, animComp->currentAnimation, "ping", animComp->currentFrame);
                    } else if (currentAnim.loop) {
                        // Loop back to beginning
                        animComp->currentFrame = 0;
                        TriggerEvent(entity, animComp->currentAnimation, "loop", animComp->currentFrame);
                    } else {
                        // Animation finished
                        animComp->currentFrame = static_cast<int>(currentAnim.frames.size()) - 1;
                        animComp->isPlaying = false;
                        TriggerEvent(entity, animComp->currentAnimation, "end", animComp->currentFrame);
                    }
                } else {
                    TriggerEvent(entity, animComp->currentAnimation, "frame", animComp->currentFrame);
                }
            } else {
                // Reverse direction (ping-pong)
                animComp->currentFrame--;
                
                if (animComp->currentFrame < 0) {
                    if (currentAnim.loop) {
                        // Start going forward again
                        animComp->reverse = false;
                        animComp->currentFrame = 1;
                        TriggerEvent(entity, animComp->currentAnimation, "pong", animComp->currentFrame);
                    } else {
                        // Ping-pong finished
                        animComp->currentFrame = 0;
                        animComp->isPlaying = false;
                        TriggerEvent(entity, animComp->currentAnimation, "end", animComp->currentFrame);
                    }
                } else {
                    TriggerEvent(entity, animComp->currentAnimation, "frame", animComp->currentFrame);
                }
            }
        }
    }
    
    // Update sprite component if available
    if (spriteComp) {
        const AnimationFrame* currentFrame = animComp->GetCurrentFrame();
        if (currentFrame) {
            UpdateSpriteFrame(spriteComp, currentFrame);
        }
    }
}

void AnimationSystem::UpdateSpriteFrame(SpriteComponent* spriteComp, const AnimationFrame* frame) {
    if (!spriteComp || !frame) return;

    // Safety check: Don't update if sprite component seems corrupted
    if (spriteComp->texturePath.empty() || spriteComp->texturePath.find('\0') != std::string::npos) {
        std::cerr << "Warning: Skipping sprite frame update due to corrupted SpriteComponent" << std::endl;
        return;
    }

    // Update sprite component with animation frame data
    spriteComp->frameX = frame->frameX;
    spriteComp->frameY = frame->frameY;
    spriteComp->frameWidth = frame->frameWidth;
    spriteComp->frameHeight = frame->frameHeight;

    // Update sprite size if needed (optional - maintains original behavior)
    // spriteComp->width = frame->frameWidth;
    // spriteComp->height = frame->frameHeight;
}

void AnimationSystem::TriggerEvent(Entity entity, const std::string& animationName, 
                                 const std::string& eventType, int frameIndex) {
    if (m_eventCallback) {
        m_eventCallback(entity, animationName, eventType, frameIndex);
    }
}

Animation AnimationSystem::CreateAnimation(const std::string& name, int startFrame, int frameCount,
                                         int frameWidth, int frameHeight, float frameDuration,
                                         int framesPerRow, bool loop) {
    Animation animation(name, loop);
    
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

void AnimationSystem::CreateAnimationStateMachine(Entity entity, AnimationState initialState) {
    m_entityStates[entity.GetID()] = initialState;
    
    // If entity has animation component, try to start default animation for initial state
    auto* animComp = m_entityManager->GetComponent<AnimationComponent>(entity);
    if (animComp) {
        std::string defaultAnim = GetDefaultAnimationForState(initialState);
        if (animComp->animations.find(defaultAnim) != animComp->animations.end()) {
            animComp->PlayAnimation(defaultAnim);
        }
    }
}

void AnimationSystem::TransitionToState(Entity entity, AnimationState newState, bool forceRestart) {
    uint32_t entityId = entity.GetID();
    
    // Check if already in this state
    auto stateIt = m_entityStates.find(entityId);
    if (stateIt != m_entityStates.end() && stateIt->second == newState && !forceRestart) {
        return; // Already in this state
    }
    
    // Update state
    m_entityStates[entityId] = newState;
    
    // Get animation component and try to play appropriate animation
    auto* animComp = m_entityManager->GetComponent<AnimationComponent>(entity);
    if (animComp) {
        std::string animName = GetDefaultAnimationForState(newState);
        if (animComp->animations.find(animName) != animComp->animations.end()) {
            animComp->PlayAnimation(animName, forceRestart);
            TriggerEvent(entity, animName, "start", 0);
        }
    }
}

AnimationState AnimationSystem::GetCurrentState(Entity entity) const {
    auto it = m_entityStates.find(entity.GetID());
    return (it != m_entityStates.end()) ? it->second : AnimationState::IDLE;
}

std::string AnimationSystem::GetDefaultAnimationForState(AnimationState state) const {
    switch (state) {
        case AnimationState::IDLE: return "idle";
        case AnimationState::WALKING: return "walk";
        case AnimationState::RUNNING: return "run";
        case AnimationState::JUMPING: return "jump";
        case AnimationState::FALLING: return "fall";
        case AnimationState::ATTACKING: return "attack";
        case AnimationState::DEFENDING: return "defend";
        case AnimationState::CASTING: return "cast";
        case AnimationState::HURT: return "hurt";
        case AnimationState::DYING: return "die";
        case AnimationState::DEAD: return "dead";
        case AnimationState::CUSTOM_1: return "custom1";
        case AnimationState::CUSTOM_2: return "custom2";
        case AnimationState::CUSTOM_3: return "custom3";
        default: return "idle";
    }
}
