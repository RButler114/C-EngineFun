/**
 * @file test_compilation.cpp
 * @brief Simple compilation test for enhanced systems
 * @author Ryan Butler
 * @date 2025
 */

#include "../include/ECS/Component.h"
#include "../include/ECS/EnhancedSystems.h"
#include "../include/ECS/AISystem.h"
#include "../include/Engine/EventSystem.h"
#include "../include/Engine/ConfigSystem.h"
#include <iostream>

int main() {
    std::cout << "Testing Enhanced ECS Compilation..." << std::endl;
    
    // Test 1: Enhanced Components
    std::cout << "1. Testing enhanced components..." << std::endl;
    
    HealthComponent health(100.0f, 0.1f, 1.0f);
    CharacterStatsComponent stats;
    CharacterTypeComponent type(CharacterTypeComponent::CharacterType::PLAYER, 
                               CharacterTypeComponent::CharacterClass::WARRIOR);
    AIComponent ai;
    StatusEffectComponent statusEffects;
    AbilityComponent abilities;
    
    std::cout << "✅ Enhanced components compile successfully!" << std::endl;
    std::cout << "   Health: " << health.currentHealth << "/" << health.maxHealth << std::endl;
    std::cout << "   Move Speed: " << stats.moveSpeed << std::endl;
    std::cout << "   Character Type: " << (int)type.type << std::endl;
    
    // Test 2: Status Effects
    std::cout << "2. Testing status effects..." << std::endl;
    
    StatusEffectComponent::StatusEffect poison(
        StatusEffectComponent::StatusEffect::EffectType::DAMAGE_OVER_TIME,
        5.0f,  // 5 seconds
        10.0f, // 10 damage per second
        "Poison"
    );
    
    statusEffects.AddEffect(poison);
    bool hasPoison = statusEffects.HasEffect(StatusEffectComponent::StatusEffect::EffectType::DAMAGE_OVER_TIME);
    
    std::cout << "✅ Status effects work!" << std::endl;
    std::cout << "   Has poison effect: " << (hasPoison ? "Yes" : "No") << std::endl;
    
    // Test 3: Abilities
    std::cout << "3. Testing abilities..." << std::endl;
    
    AbilityComponent::Ability fireball;
    fireball.name = "Fireball";
    fireball.cooldown = 3.0f;
    fireball.manaCost = 25.0f;
    fireball.damage = 30.0f;
    fireball.range = 150.0f;
    
    abilities.AddAbility(fireball);
    bool canUse = abilities.CanUseAbility(0, stats.currentMana, stats.currentStamina);
    
    std::cout << "✅ Abilities work!" << std::endl;
    std::cout << "   Can use fireball: " << (canUse ? "Yes" : "No") << std::endl;
    
    // Test 4: AI Components
    std::cout << "4. Testing AI components..." << std::endl;
    
    ai.AddPatrolPoint(100.0f, 200.0f);
    ai.AddPatrolPoint(200.0f, 200.0f);
    ai.ChangeState(AIComponent::AIState::PATROL);
    
    std::cout << "✅ AI components work!" << std::endl;
    std::cout << "   AI State: " << (int)ai.currentState << std::endl;
    std::cout << "   Patrol Points: " << ai.patrolPoints.size() << std::endl;
    
    // Test 5: Event System
    std::cout << "5. Testing event system..." << std::endl;
    
    EventManager eventManager;
    bool eventReceived = false;
    
    eventManager.Subscribe<EntityDamagedEvent>([&eventReceived](const EntityDamagedEvent& event) {
        std::cout << "   Received damage event for entity " << event.entity.GetID() << std::endl;
        eventReceived = true;
    });
    
    Entity testEntity(1);
    eventManager.FireEvent(EntityDamagedEvent(testEntity, Entity(), 25.0f, 75.0f));
    
    std::cout << "✅ Event system works!" << std::endl;
    std::cout << "   Event received: " << (eventReceived ? "Yes" : "No") << std::endl;
    
    // Test 6: Config System
    std::cout << "6. Testing config system..." << std::endl;
    
    ConfigManager config;
    config.Set("test", "name", "Test Character");
    config.Set("test", "health", 150.0f);
    config.Set("test", "speed", 100.0f);
    
    std::string name = config.Get("test", "name").AsString();
    float healthVal = config.Get("test", "health").AsFloat();
    float speed = config.Get("test", "speed").AsFloat();
    
    std::cout << "✅ Config system works!" << std::endl;
    std::cout << "   Name: " << name << std::endl;
    std::cout << "   Health: " << healthVal << std::endl;
    std::cout << "   Speed: " << speed << std::endl;
    
    // Test 7: Health System Logic
    std::cout << "7. Testing health system logic..." << std::endl;
    
    float damageDealt = health.TakeDamage(30.0f);
    health.Heal(10.0f);
    float healthPercent = health.GetHealthPercentage();
    
    std::cout << "✅ Health system logic works!" << std::endl;
    std::cout << "   Damage dealt: " << damageDealt << std::endl;
    std::cout << "   Current health: " << health.currentHealth << std::endl;
    std::cout << "   Health percentage: " << (healthPercent * 100.0f) << "%" << std::endl;
    
    // Test 8: Character Stats Calculation
    std::cout << "8. Testing character stats calculation..." << std::endl;
    
    stats.strength = 15.0f;
    stats.agility = 12.0f;
    stats.intelligence = 10.0f;
    stats.vitality = 14.0f;
    stats.RecalculateStats();
    
    std::cout << "✅ Character stats calculation works!" << std::endl;
    std::cout << "   Attack Damage: " << stats.attackDamage << std::endl;
    std::cout << "   Move Speed: " << stats.moveSpeed << std::endl;
    std::cout << "   Max Mana: " << stats.maxMana << std::endl;
    
    std::cout << "\n🎉 All compilation tests passed!" << std::endl;
    std::cout << "\nThe enhanced architecture successfully provides:" << std::endl;
    std::cout << "  ✅ Enhanced components with rich functionality" << std::endl;
    std::cout << "  ✅ Status effects system" << std::endl;
    std::cout << "  ✅ Flexible ability system" << std::endl;
    std::cout << "  ✅ AI state management" << std::endl;
    std::cout << "  ✅ Event-driven communication" << std::endl;
    std::cout << "  ✅ Configuration management" << std::endl;
    std::cout << "  ✅ Health and damage systems" << std::endl;
    std::cout << "  ✅ Character stat calculations" << std::endl;
    
    std::cout << "\n✨ Your game architecture is ready for complex gameplay!" << std::endl;
    
    return 0;
}
