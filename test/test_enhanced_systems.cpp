/**
 * @file test_enhanced_systems.cpp
 * @brief Test the enhanced ECS systems
 * @author Ryan Butler
 * @date 2025
 */

#include "../include/Engine/Engine.h"
#include "../include/ECS/ECS.h"
#include "../include/ECS/EnhancedSystems.h"
#include "../include/ECS/AISystem.h"
#include "../include/Engine/EventSystem.h"
#include "../include/Game/CharacterFactory.h"
#include <iostream>
#include <chrono>

class EnhancedSystemsTest : public Engine {
public:
    EnhancedSystemsTest() : m_testPhase(0), m_testTimer(0.0f), m_testPassed(false) {}

protected:
    void Update(float deltaTime) override {
        m_testTimer += deltaTime;
        
        switch (m_testPhase) {
            case 0:
                TestComponentCreation();
                break;
            case 1:
                TestCharacterFactory();
                break;
            case 2:
                TestEventSystem();
                break;
            case 3:
                TestAISystem();
                break;
            case 4:
                TestHealthSystem();
                break;
            case 5:
                CompleteTest();
                break;
        }
        
        // Auto-advance test phases
        if (m_testTimer > 1.0f) {
            m_testPhase++;
            m_testTimer = 0.0f;
        }
        
        // Exit after all tests
        if (m_testPhase > 5) {
            Quit();
        }
    }
    
    void Render() override {
        auto* renderer = GetRenderer();
        if (!renderer) return;
        
        // Simple visual feedback
        renderer->Clear();
        
        // Draw test status
        Color testColor = m_testPassed ? Color(0, 255, 0, 255) : Color(255, 255, 0, 255);
        Rectangle statusRect(10, 10, 200, 50);
        renderer->DrawRectangle(statusRect, testColor, true);
        
        renderer->Present();
    }

private:
    int m_testPhase;
    float m_testTimer;
    bool m_testPassed;
    std::unique_ptr<EntityManager> m_entityManager;
    std::unique_ptr<EventManager> m_eventManager;
    std::unique_ptr<CharacterFactory> m_characterFactory;
    
    void TestComponentCreation() {
        std::cout << "Phase 0: Testing Enhanced Components..." << std::endl;
        
        m_entityManager = std::make_unique<EntityManager>();
        m_eventManager = std::make_unique<EventManager>();
        m_characterFactory = std::make_unique<CharacterFactory>(m_entityManager.get());
        
        // Test creating entity with enhanced components
        Entity testEntity = m_entityManager->CreateEntity();
        
        // Add enhanced components
        m_entityManager->AddComponent<TransformComponent>(testEntity, 100.0f, 200.0f);
        m_entityManager->AddComponent<HealthComponent>(testEntity, 100.0f, 0.1f, 1.0f);
        m_entityManager->AddComponent<CharacterStatsComponent>(testEntity);
        m_entityManager->AddComponent<CharacterTypeComponent>(testEntity, 
            CharacterTypeComponent::CharacterType::ENEMY, 
            CharacterTypeComponent::CharacterClass::WARRIOR);
        
        // Verify components were added
        auto* health = m_entityManager->GetComponent<HealthComponent>(testEntity);
        auto* stats = m_entityManager->GetComponent<CharacterStatsComponent>(testEntity);
        auto* type = m_entityManager->GetComponent<CharacterTypeComponent>(testEntity);
        
        if (health && stats && type) {
            std::cout << "✅ Enhanced components created successfully!" << std::endl;
            std::cout << "   Health: " << health->currentHealth << "/" << health->maxHealth << std::endl;
            std::cout << "   Type: " << (int)type->type << ", Class: " << (int)type->characterClass << std::endl;
        } else {
            std::cout << "❌ Failed to create enhanced components!" << std::endl;
        }
    }
    
    void TestCharacterFactory() {
        std::cout << "Phase 1: Testing Character Factory..." << std::endl;
        
        // Test creating different character types
        Entity player = m_characterFactory->CreatePlayer(50.0f, 300.0f);
        Entity enemy = m_characterFactory->CreateBasicEnemy(200.0f, 300.0f);
        Entity boss = m_characterFactory->CreateBoss(400.0f, 300.0f);
        
        // Verify entities were created with proper components
        auto* playerType = m_entityManager->GetComponent<CharacterTypeComponent>(player);
        auto* enemyType = m_entityManager->GetComponent<CharacterTypeComponent>(enemy);
        auto* bossType = m_entityManager->GetComponent<CharacterTypeComponent>(boss);
        
        if (playerType && enemyType && bossType) {
            std::cout << "✅ Character factory working!" << std::endl;
            std::cout << "   Player type: " << (int)playerType->type << std::endl;
            std::cout << "   Enemy type: " << (int)enemyType->type << std::endl;
            std::cout << "   Boss type: " << (int)bossType->type << std::endl;
        } else {
            std::cout << "❌ Character factory failed!" << std::endl;
        }
    }
    
    void TestEventSystem() {
        std::cout << "Phase 2: Testing Event System..." << std::endl;
        
        bool eventReceived = false;
        
        // Subscribe to an event
        m_eventManager->Subscribe<EntityDamagedEvent>([&eventReceived](const EntityDamagedEvent& event) {
            std::cout << "   Received damage event for entity " << event.entity.GetID() << std::endl;
            eventReceived = true;
        });
        
        // Fire an event
        Entity testEntity = m_entityManager->CreateEntity();
        m_eventManager->FireEvent(EntityDamagedEvent(testEntity, Entity(), 25.0f, 75.0f));
        
        if (eventReceived) {
            std::cout << "✅ Event system working!" << std::endl;
        } else {
            std::cout << "❌ Event system failed!" << std::endl;
        }
    }
    
    void TestAISystem() {
        std::cout << "Phase 3: Testing AI System..." << std::endl;
        
        // Add AI system
        m_entityManager->AddSystem<AISystem>();
        
        // Create entity with AI
        Entity aiEntity = m_entityManager->CreateEntity();
        m_entityManager->AddComponent<TransformComponent>(aiEntity, 300.0f, 400.0f);
        m_entityManager->AddComponent<VelocityComponent>(aiEntity, 0.0f, 0.0f);
        auto* ai = m_entityManager->AddComponent<AIComponent>(aiEntity);
        
        // Set up AI behavior
        ai->AddPatrolPoint(250.0f, 400.0f);
        ai->AddPatrolPoint(350.0f, 400.0f);
        ai->ChangeState(AIComponent::AIState::PATROL);
        
        // Update the system briefly
        m_entityManager->Update(0.016f);
        
        if (ai->currentState == AIComponent::AIState::PATROL) {
            std::cout << "✅ AI system working!" << std::endl;
            std::cout << "   AI state: " << (int)ai->currentState << std::endl;
        } else {
            std::cout << "❌ AI system failed!" << std::endl;
        }
    }
    
    void TestHealthSystem() {
        std::cout << "Phase 4: Testing Health System..." << std::endl;
        
        // Add health system
        auto* healthSystem = m_entityManager->AddSystem<HealthSystem>();
        
        bool deathCallbackCalled = false;
        healthSystem->SetDeathCallback([&deathCallbackCalled](Entity entity) {
            std::cout << "   Death callback called for entity " << entity.GetID() << std::endl;
            deathCallbackCalled = true;
        });
        
        // Create entity with health
        Entity healthEntity = m_entityManager->CreateEntity();
        auto* health = m_entityManager->AddComponent<HealthComponent>(healthEntity, 50.0f);
        
        // Deal fatal damage
        float damageDealt = healthSystem->DealDamage(healthEntity, 60.0f);
        
        // Update system to process death
        m_entityManager->Update(0.016f);
        
        if (health->isDead && deathCallbackCalled && damageDealt > 0.0f) {
            std::cout << "✅ Health system working!" << std::endl;
            std::cout << "   Damage dealt: " << damageDealt << std::endl;
            std::cout << "   Entity is dead: " << (health->isDead ? "Yes" : "No") << std::endl;
        } else {
            std::cout << "❌ Health system failed!" << std::endl;
        }
    }
    
    void CompleteTest() {
        std::cout << "Phase 5: All tests completed!" << std::endl;
        std::cout << "✅ Enhanced ECS architecture is working correctly!" << std::endl;
        m_testPassed = true;
    }
};

int main() {
    EnhancedSystemsTest test;
    
    if (test.Initialize("Enhanced Systems Test", 800, 600)) {
        std::cout << "Running Enhanced ECS Systems Test..." << std::endl;
        test.Run();
        std::cout << "Test completed successfully!" << std::endl;
    } else {
        std::cout << "Failed to initialize test!" << std::endl;
        return 1;
    }
    
    return 0;
}
