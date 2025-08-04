/**
 * @file EnhancedSystems.h
 * @brief Enhanced ECS systems for scalable game development
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "System.h"
#include "Component.h"
#include <functional>
#include <queue>
#include <unordered_map>

// Forward declarations
class AudioManager;

/**
 * @class HealthSystem
 * @brief System that manages entity health, damage, and regeneration
 *
 * Processes entities with HealthComponent to handle:
 * - Health regeneration over time
 * - Invulnerability timer countdown
 * - Death state management
 * - Status effect damage/healing
 */
class HealthSystem : public System {
public:
    using DeathCallback = std::function<void(Entity)>;
    
    HealthSystem() = default;
    
    void Update(float deltaTime) override {
        auto entities = m_entityManager->GetEntitiesWith<HealthComponent>();
        
        for (Entity entity : entities) {
            auto* health = m_entityManager->GetComponent<HealthComponent>(entity);
            if (!health) continue;
            
            // Update invulnerability timer
            if (health->invulnerabilityTimer > 0.0f) {
                health->invulnerabilityTimer -= deltaTime;
                if (health->invulnerabilityTimer <= 0.0f) {
                    health->invulnerable = false;
                }
            }
            
            // Handle health regeneration
            if (!health->isDead && health->regenerationRate > 0.0f) {
                health->Heal(health->regenerationRate * deltaTime);
            }
            
            // Process status effects if entity has them
            auto* statusEffects = m_entityManager->GetComponent<StatusEffectComponent>(entity);
            if (statusEffects) {
                ProcessStatusEffects(entity, health, statusEffects, deltaTime);
            }
            
            // Check for death
            if (!health->isDead && health->currentHealth <= 0.0f) {
                health->isDead = true;
                if (m_deathCallback) {
                    m_deathCallback(entity);
                }
            }
        }
    }
    
    /**
     * @brief Set callback for when entities die
     */
    void SetDeathCallback(const DeathCallback& callback) {
        m_deathCallback = callback;
    }
    
    /**
     * @brief Apply damage to an entity
     */
    float DealDamage(Entity target, float damage, float invulnerabilityDuration = 0.0f) {
        auto* health = m_entityManager->GetComponent<HealthComponent>(target);
        if (!health) return 0.0f;
        
        float actualDamage = health->TakeDamage(damage);
        
        if (actualDamage > 0.0f && invulnerabilityDuration > 0.0f) {
            health->invulnerable = true;
            health->invulnerabilityTimer = invulnerabilityDuration;
        }
        
        return actualDamage;
    }

private:
    DeathCallback m_deathCallback;
    
    void ProcessStatusEffects(Entity entity, HealthComponent* health,
                            StatusEffectComponent* statusEffects, float deltaTime) {
        (void)entity; // Suppress unused parameter warning
        for (auto& effect : statusEffects->effects) {
            effect.remainingTime -= deltaTime;
            
            switch (effect.type) {
                case StatusEffectComponent::StatusEffect::EffectType::DAMAGE_OVER_TIME:
                    health->TakeDamage(effect.magnitude * deltaTime);
                    break;
                case StatusEffectComponent::StatusEffect::EffectType::HEAL_OVER_TIME:
                    health->Heal(effect.magnitude * deltaTime);
                    break;
                // Other effect types handled by other systems
                default:
                    break;
            }
        }
        
        statusEffects->RemoveExpiredEffects();
    }
};

/**
 * @class CharacterStatsSystem
 * @brief System that manages character stats and resource regeneration
 *
 * Handles mana and stamina regeneration, stat calculations, and
 * resource management for entities with CharacterStatsComponent.
 */
class CharacterStatsSystem : public System {
public:
    void Update(float deltaTime) override {
        auto entities = m_entityManager->GetEntitiesWith<CharacterStatsComponent>();
        
        for (Entity entity : entities) {
            auto* stats = m_entityManager->GetComponent<CharacterStatsComponent>(entity);
            if (!stats) continue;
            
            // Regenerate mana
            if (stats->currentMana < stats->maxMana) {
                stats->currentMana = std::min(stats->maxMana, 
                    stats->currentMana + stats->manaRegenRate * deltaTime);
            }
            
            // Regenerate stamina
            if (stats->currentStamina < stats->maxStamina) {
                stats->currentStamina = std::min(stats->maxStamina,
                    stats->currentStamina + stats->staminaRegenRate * deltaTime);
            }
            
            // Apply status effect modifiers
            auto* statusEffects = m_entityManager->GetComponent<StatusEffectComponent>(entity);
            if (statusEffects) {
                ApplyStatusEffectModifiers(stats, statusEffects);
            }
        }
    }

private:
    void ApplyStatusEffectModifiers(CharacterStatsComponent* stats, 
                                  StatusEffectComponent* statusEffects) {
        // Reset to base values and apply modifiers
        // This is a simplified approach - in a full system you'd want to track base vs modified values
        for (const auto& effect : statusEffects->effects) {
            switch (effect.type) {
                case StatusEffectComponent::StatusEffect::EffectType::SPEED_BOOST:
                    stats->moveSpeed *= (1.0f + effect.magnitude);
                    break;
                case StatusEffectComponent::StatusEffect::EffectType::SPEED_REDUCTION:
                    stats->moveSpeed *= (1.0f - effect.magnitude);
                    break;
                case StatusEffectComponent::StatusEffect::EffectType::DAMAGE_BOOST:
                    stats->attackDamage *= (1.0f + effect.magnitude);
                    break;
                case StatusEffectComponent::StatusEffect::EffectType::DAMAGE_REDUCTION:
                    stats->attackDamage *= (1.0f - effect.magnitude);
                    break;
                default:
                    break;
            }
        }
    }
};

/**
 * @class AbilitySystem
 * @brief System that manages entity abilities and cooldowns
 *
 * Handles ability cooldown timers, resource costs, and ability activation
 * for entities with AbilityComponent.
 */
class AbilitySystem : public System {
public:
    using AbilityCallback = std::function<void(Entity, int)>; // Entity, ability index
    
    void Update(float deltaTime) override {
        auto entities = m_entityManager->GetEntitiesWith<AbilityComponent>();
        
        for (Entity entity : entities) {
            auto* abilities = m_entityManager->GetComponent<AbilityComponent>(entity);
            if (!abilities) continue;
            
            // Update cooldowns
            for (auto& ability : abilities->abilities) {
                if (ability.currentCooldown > 0.0f) {
                    ability.currentCooldown -= deltaTime;
                    if (ability.currentCooldown < 0.0f) {
                        ability.currentCooldown = 0.0f;
                    }
                }
            }
        }
    }
    
    /**
     * @brief Attempt to use an ability
     */
    bool TryUseAbility(Entity entity, int abilityIndex) {
        auto* abilities = m_entityManager->GetComponent<AbilityComponent>(entity);
        auto* stats = m_entityManager->GetComponent<CharacterStatsComponent>(entity);
        
        if (!abilities || !stats) return false;
        
        if (abilities->UseAbility(abilityIndex, stats->currentMana, stats->currentStamina)) {
            if (m_abilityCallback) {
                m_abilityCallback(entity, abilityIndex);
            }
            return true;
        }
        
        return false;
    }
    
    /**
     * @brief Set callback for when abilities are used
     */
    void SetAbilityCallback(const AbilityCallback& callback) {
        m_abilityCallback = callback;
    }

private:
    AbilityCallback m_abilityCallback;
};

/**
 * @class StatusEffectSystem
 * @brief System that manages status effects and their timers
 *
 * Updates status effect durations and removes expired effects.
 */
class StatusEffectSystem : public System {
public:
    void Update(float deltaTime) override {
        auto entities = m_entityManager->GetEntitiesWith<StatusEffectComponent>();
        
        for (Entity entity : entities) {
            auto* statusEffects = m_entityManager->GetComponent<StatusEffectComponent>(entity);
            if (!statusEffects) continue;
            
            // Update effect timers (actual effect processing is done by other systems)
            for (auto& effect : statusEffects->effects) {
                effect.remainingTime -= deltaTime;
            }
            
            // Remove expired effects
            statusEffects->RemoveExpiredEffects();
        }
    }
};
