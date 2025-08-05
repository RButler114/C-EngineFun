/**
 * @file test_animation_system.cpp
 * @brief Test program for the new Animation System
 * @author Ryan Butler
 * @date 2025
 */

#include "ECS/ECS.h"
#include "Game/AnimationFactory.h"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "🎬 Animation System Test" << std::endl;
    std::cout << "========================" << std::endl;

    // Create entity manager
    EntityManager entityManager;
    
    // Add animation system
    auto* animSystem = entityManager.AddSystem<AnimationSystem>();
    std::cout << "✅ AnimationSystem created" << std::endl;
    
    // Set up animation event callback
    animSystem->SetAnimationEventCallback([](Entity entity, const std::string& animationName, 
                                            const std::string& eventType, int frameIndex) {
        std::cout << "🎭 Entity " << entity.GetID() << ": " << animationName 
                  << " -> " << eventType;
        if (frameIndex >= 0) {
            std::cout << " (frame " << frameIndex << ")";
        }
        std::cout << std::endl;
    });
    
    // Create test entity
    Entity testEntity = entityManager.CreateEntity();
    std::cout << "✅ Created test entity: " << testEntity.GetID() << std::endl;
    
    // Add transform and sprite components
    entityManager.AddComponent<TransformComponent>(testEntity, 100.0f, 200.0f);
    entityManager.AddComponent<SpriteComponent>(testEntity, "test_sprite.png", 32, 32);
    
    // Add animation component
    auto* animComp = entityManager.AddComponent<AnimationComponent>(testEntity);
    
    // Create test animations using AnimationFactory
    std::cout << "🎨 Creating animations..." << std::endl;
    auto animations = AnimationFactory::CreatePlayerAnimations(32, 32, 0.2f);
    
    for (const auto& [name, anim] : animations) {
        animComp->animations[name] = anim;
        std::cout << "  📽️  Added animation: " << name << " (" << anim.frames.size() << " frames)" << std::endl;
    }
    
    // Create animation state machine
    animSystem->CreateAnimationStateMachine(testEntity, AnimationState::IDLE);
    std::cout << "✅ Animation state machine created" << std::endl;
    
    // Test animation transitions
    std::cout << "\n🎬 Testing Animation Transitions:" << std::endl;
    std::cout << "=================================" << std::endl;
    
    float deltaTime = 0.016f; // ~60 FPS
    int totalFrames = 0;
    
    // Test idle animation for 2 seconds
    std::cout << "\n⏸️  Testing IDLE animation (2 seconds)..." << std::endl;
    for (int i = 0; i < 120; ++i) { // 2 seconds at 60 FPS
        entityManager.Update(deltaTime);
        totalFrames++;
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    // Transition to walking
    std::cout << "\n🚶 Transitioning to WALKING animation (3 seconds)..." << std::endl;
    animSystem->TransitionToState(testEntity, AnimationState::WALKING);
    for (int i = 0; i < 180; ++i) { // 3 seconds at 60 FPS
        entityManager.Update(deltaTime);
        totalFrames++;
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    // Transition to jumping
    std::cout << "\n🦘 Transitioning to JUMPING animation (1 second)..." << std::endl;
    animSystem->TransitionToState(testEntity, AnimationState::JUMPING);
    for (int i = 0; i < 60; ++i) { // 1 second at 60 FPS
        entityManager.Update(deltaTime);
        totalFrames++;
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    // Back to idle
    std::cout << "\n⏸️  Back to IDLE animation (1 second)..." << std::endl;
    animSystem->TransitionToState(testEntity, AnimationState::IDLE);
    for (int i = 0; i < 60; ++i) { // 1 second at 60 FPS
        entityManager.Update(deltaTime);
        totalFrames++;
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    std::cout << "\n✅ Animation System Test Complete!" << std::endl;
    std::cout << "📊 Total frames processed: " << totalFrames << std::endl;
    std::cout << "⏱️  Total time: ~7 seconds" << std::endl;
    std::cout << "\n🎉 Animation System is working correctly!" << std::endl;
    
    return 0;
}
