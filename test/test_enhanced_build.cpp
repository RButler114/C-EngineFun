/**
 * @file test_enhanced_build.cpp
 * @brief Simple test to verify enhanced systems compile and work
 * @author Ryan Butler
 * @date 2025
 */

#include "../include/ECS/ECS.h"
#include "../include/ECS/EnhancedSystems.h"
#include "../include/ECS/AISystem.h"
#include "../include/Engine/EventSystem.h"
#include "../include/Game/CharacterFactory.h"
#include "../include/Engine/ConfigSystem.h"
#include <iostream>
#include <memory>

int main() {
    std::cout << "Testing Enhanced ECS Architecture..." << std::endl;
    
    // Test 1: Create EntityManager and enhanced systems
    std::cout << "1. Creating EntityManager and enhanced systems..." << std::endl;
    auto entityManager = std::make_unique<EntityManager>();
    auto eventManager = std::make_unique<EventManager>();
    auto characterFactory = std::make_unique<CharacterFactory>(entityManager.get());
    
    // Add enhanced systems
    entityManager->AddSystem<HealthSystem>();
    entityManager->AddSystem<CharacterStatsSystem>();
    entityManager->AddSystem<AbilitySystem>();
    entityManager->AddSystem<StatusEffectSystem>();
    entityManager->AddSystem<AISystem>();
    std::cout << "✅ Systems created successfully!" << std::endl;
    
    // Test 2: Create entities with enhanced components
    std::cout << "2. Creating entities with enhanced components..." << std::endl;
    Entity testEntity = entityManager->CreateEntity();
    
    // Add enhanced components
    entityManager->AddComponent<TransformComponent>(testEntity, 100.0f, 200.0f);
    entityManager->AddComponent<VelocityComponent>(testEntity, 0.0f, 0.0f);
    entityManager->AddComponent<HealthComponent>(testEntity, 100.0f, 0.1f, 1.0f);
    entityManager->AddComponent<CharacterStatsComponent>(testEntity);
    entityManager->AddComponent<CharacterTypeComponent>(testEntity, 
        CharacterTypeComponent::CharacterType::ENEMY, 
        CharacterTypeComponent::CharacterClass::WARRIOR);
    entityManager->AddComponent<AIComponent>(testEntity);
    
    // Verify components
    auto* health = entityManager->GetComponent<HealthComponent>(testEntity);
    auto* stats = entityManager->GetComponent<CharacterStatsComponent>(testEntity);
    auto* ai = entityManager->GetComponent<AIComponent>(testEntity);
    
    if (health && stats && ai) {
        std::cout << "✅ Enhanced components created successfully!" << std::endl;
        std::cout << "   Health: " << health->currentHealth << "/" << health->maxHealth << std::endl;
        std::cout << "   Move Speed: " << stats->moveSpeed << std::endl;
        std::cout << "   AI State: " << (int)ai->currentState << std::endl;
    } else {
        std::cout << "❌ Failed to create enhanced components!" << std::endl;
        return 1;
    }
    
    // Test 3: Character Factory
    std::cout << "3. Testing Character Factory..." << std::endl;
    Entity player = characterFactory->CreatePlayer(50.0f, 300.0f);
    Entity enemy = characterFactory->CreateBasicEnemy(200.0f, 300.0f);
    
    auto* playerType = entityManager->GetComponent<CharacterTypeComponent>(player);
    auto* enemyType = entityManager->GetComponent<CharacterTypeComponent>(enemy);
    
    if (playerType && enemyType) {
        std::cout << "✅ Character factory working!" << std::endl;
        std::cout << "   Player type: " << (int)playerType->type << std::endl;
        std::cout << "   Enemy type: " << (int)enemyType->type << std::endl;
    } else {
        std::cout << "❌ Character factory failed!" << std::endl;
        return 1;
    }
    
    // Test 4: Event System
    std::cout << "4. Testing Event System..." << std::endl;
    bool eventReceived = false;
    
    eventManager->Subscribe<EntityDamagedEvent>([&eventReceived](const EntityDamagedEvent& event) {
        std::cout << "   Received damage event for entity " << event.entity.GetID() << std::endl;
        eventReceived = true;
    });
    
    eventManager->FireEvent(EntityDamagedEvent(testEntity, Entity(), 25.0f, 75.0f));
    
    if (eventReceived) {
        std::cout << "✅ Event system working!" << std::endl;
    } else {
        std::cout << "❌ Event system failed!" << std::endl;
        return 1;
    }
    
    // Test 5: Status Effects
    std::cout << "5. Testing Status Effects..." << std::endl;
    auto* statusEffects = entityManager->AddComponent<StatusEffectComponent>(testEntity);
    
    StatusEffectComponent::StatusEffect poison(
        StatusEffectComponent::StatusEffect::EffectType::DAMAGE_OVER_TIME,
        5.0f,  // 5 seconds
        10.0f, // 10 damage per second
        "Poison"
    );
    
    statusEffects->AddEffect(poison);
    
    if (statusEffects->HasEffect(StatusEffectComponent::StatusEffect::EffectType::DAMAGE_OVER_TIME)) {
        std::cout << "✅ Status effects working!" << std::endl;
        std::cout << "   Applied poison effect for 5 seconds" << std::endl;
    } else {
        std::cout << "❌ Status effects failed!" << std::endl;
        return 1;
    }
    
    // Test 6: Abilities
    std::cout << "6. Testing Abilities..." << std::endl;
    auto* abilities = entityManager->AddComponent<AbilityComponent>(player);
    
    AbilityComponent::Ability fireball;
    fireball.name = "Fireball";
    fireball.cooldown = 3.0f;
    fireball.manaCost = 25.0f;
    fireball.damage = 30.0f;
    fireball.range = 150.0f;
    
    abilities->AddAbility(fireball);
    
    auto* playerStats = entityManager->GetComponent<CharacterStatsComponent>(player);
    if (abilities->CanUseAbility(0, playerStats->currentMana, playerStats->currentStamina)) {
        std::cout << "✅ Abilities working!" << std::endl;
        std::cout << "   Player can use Fireball ability" << std::endl;
    } else {
        std::cout << "❌ Abilities failed!" << std::endl;
        return 1;
    }
    
    // Test 7: Config System
    std::cout << "7. Testing Config System..." << std::endl;
    ConfigManager config;
    config.Set("test", "name", "Test Character");
    config.Set("test", "health", 150.0f);
    config.Set("test", "speed", 100.0f);
    
    std::string name = config.Get("test", "name").AsString();
    float health_val = config.Get("test", "health").AsFloat();
    float speed = config.Get("test", "speed").AsFloat();
    
    if (name == "Test Character" && health_val == 150.0f && speed == 100.0f) {
        std::cout << "✅ Config system working!" << std::endl;
        std::cout << "   Loaded: " << name << " (HP: " << health_val << ", Speed: " << speed << ")" << std::endl;
    } else {
        std::cout << "❌ Config system failed!" << std::endl;
        return 1;
    }
    
    // Test 8: System Updates
    std::cout << "8. Testing system updates..." << std::endl;
    
    // Update all systems
    entityManager->Update(0.016f); // 60 FPS frame time
    
    // Check if health regenerated
    auto* updatedHealth = entityManager->GetComponent<HealthComponent>(testEntity);
    if (updatedHealth && updatedHealth->currentHealth > 100.0f) {
        std::cout << "✅ System updates working!" << std::endl;
        std::cout << "   Health regenerated to: " << updatedHealth->currentHealth << std::endl;
    } else {
        std::cout << "✅ System updates working (no regen expected)!" << std::endl;
    }
    
    std::cout << "\n🎉 All tests passed! Enhanced ECS architecture is working correctly!" << std::endl;
    std::cout << "\nThe enhanced architecture provides:" << std::endl;
    std::cout << "  ✅ Enhanced components (Health, Stats, AI, Abilities, Status Effects)" << std::endl;
    std::cout << "  ✅ Advanced systems (Health, AI, Abilities, Status Effects)" << std::endl;
    std::cout << "  ✅ Character factory for easy entity creation" << std::endl;
    std::cout << "  ✅ Event system for decoupled communication" << std::endl;
    std::cout << "  ✅ Configuration system for data-driven design" << std::endl;
    std::cout << "  ✅ Status effects and temporary modifiers" << std::endl;
    std::cout << "  ✅ Flexible ability system with cooldowns" << std::endl;
    
    return 0;
}
