/**
 * @file EnhancedMovementSystem.h
 * @brief Enhanced movement system with physics and status effects
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "System.h"
#include "Component.h"
#include "Engine/EventSystem.h"
#include <cmath>

/**
 * @class EnhancedMovementSystem
 * @brief Advanced movement system that handles physics, status effects, and constraints
 *
 * Extends the basic MovementSystem with:
 * - Status effect modifiers (speed boosts/reductions, stuns)
 * - Physics constraints (gravity, friction, boundaries)
 * - Character stat integration
 * - Event-driven movement responses
 */
class EnhancedMovementSystem : public System {
public:
    EnhancedMovementSystem(EventManager* eventManager = nullptr) 
        : m_eventManager(eventManager), m_gravity(500.0f), m_friction(0.8f) {}
    
    void Update(float deltaTime) override {
        auto entities = m_entityManager->GetEntitiesWith<TransformComponent, VelocityComponent>();
        
        for (Entity entity : entities) {
            auto* transform = m_entityManager->GetComponent<TransformComponent>(entity);
            auto* velocity = m_entityManager->GetComponent<VelocityComponent>(entity);
            
            if (!transform || !velocity) continue;
            
            // Check if entity is stunned
            if (IsStunned(entity)) {
                velocity->vx = 0.0f;
                velocity->vy = 0.0f;
                continue;
            }
            
            // Apply status effect modifiers
            float speedMultiplier = CalculateSpeedMultiplier(entity);
            
            // Apply character stats if available
            ApplyCharacterStats(entity, velocity, speedMultiplier);
            
            // Apply physics
            ApplyPhysics(entity, velocity, deltaTime);
            
            // Update position
            float oldX = transform->x;
            float oldY = transform->y;
            
            transform->x += velocity->vx * deltaTime;
            transform->y += velocity->vy * deltaTime;
            
            // Apply world boundaries
            ApplyWorldBoundaries(entity, transform, velocity);
            
            // Check if entity moved significantly
            float deltaX = transform->x - oldX;
            float deltaY = transform->y - oldY;
            float movementDistance = std::sqrt(deltaX * deltaX + deltaY * deltaY);
            
            // Update facing direction for sprites
            UpdateFacingDirection(entity, deltaX);
            
            // Fire movement event if entity moved significantly
            if (movementDistance > 1.0f && m_eventManager) {
                // Could fire a movement event here if needed
            }
        }
    }
    
    /**
     * @brief Set gravity strength
     */
    void SetGravity(float gravity) {
        m_gravity = gravity;
    }
    
    /**
     * @brief Set friction coefficient
     */
    void SetFriction(float friction) {
        m_friction = friction;
    }
    
    /**
     * @brief Set world boundaries
     */
    void SetWorldBoundaries(float minX, float maxX, float minY, float maxY) {
        m_worldMinX = minX;
        m_worldMaxX = maxX;
        m_worldMinY = minY;
        m_worldMaxY = maxY;
        m_hasBoundaries = true;
    }
    
    /**
     * @brief Apply impulse to an entity
     */
    void ApplyImpulse(Entity entity, float impulseX, float impulseY) {
        auto* velocity = m_entityManager->GetComponent<VelocityComponent>(entity);
        if (velocity) {
            velocity->vx += impulseX;
            velocity->vy += impulseY;
        }
    }
    
    /**
     * @brief Set velocity directly (useful for teleportation, knockback, etc.)
     */
    void SetVelocity(Entity entity, float vx, float vy) {
        auto* velocity = m_entityManager->GetComponent<VelocityComponent>(entity);
        if (velocity) {
            velocity->vx = vx;
            velocity->vy = vy;
        }
    }

private:
    EventManager* m_eventManager;
    float m_gravity;
    float m_friction;
    bool m_hasBoundaries = false;
    float m_worldMinX = 0.0f;
    float m_worldMaxX = 1000.0f;
    float m_worldMinY = 0.0f;
    float m_worldMaxY = 600.0f;
    
    bool IsStunned(Entity entity) {
        auto* statusEffects = m_entityManager->GetComponent<StatusEffectComponent>(entity);
        return statusEffects && statusEffects->HasEffect(StatusEffectComponent::StatusEffect::EffectType::STUN);
    }
    
    float CalculateSpeedMultiplier(Entity entity) {
        auto* statusEffects = m_entityManager->GetComponent<StatusEffectComponent>(entity);
        if (!statusEffects) return 1.0f;
        
        float multiplier = 1.0f;
        
        for (const auto& effect : statusEffects->effects) {
            switch (effect.type) {
                case StatusEffectComponent::StatusEffect::EffectType::SPEED_BOOST:
                    multiplier *= (1.0f + effect.magnitude);
                    break;
                case StatusEffectComponent::StatusEffect::EffectType::SPEED_REDUCTION:
                    multiplier *= (1.0f - effect.magnitude);
                    break;
                default:
                    break;
            }
        }
        
        return std::max(0.0f, multiplier); // Ensure non-negative
    }
    
    void ApplyCharacterStats(Entity entity, VelocityComponent* velocity, float speedMultiplier) {
        auto* stats = m_entityManager->GetComponent<CharacterStatsComponent>(entity);
        if (!stats) return;
        
        // Limit velocity based on character's move speed
        float maxSpeed = stats->moveSpeed * speedMultiplier;
        float currentSpeed = std::sqrt(velocity->vx * velocity->vx + velocity->vy * velocity->vy);
        
        if (currentSpeed > maxSpeed && currentSpeed > 0.0f) {
            float scale = maxSpeed / currentSpeed;
            velocity->vx *= scale;
            velocity->vy *= scale;
        }
    }
    
    void ApplyPhysics(Entity entity, VelocityComponent* velocity, float deltaTime) {
        // Apply gravity (only if entity doesn't have a ground constraint)
        auto* health = m_entityManager->GetComponent<HealthComponent>(entity);
        if (!health || !health->isDead) { // Don't apply physics to dead entities
            velocity->vy += m_gravity * deltaTime;
        }
        
        // Apply friction
        velocity->vx *= std::pow(m_friction, deltaTime);
        
        // Apply air resistance to vertical movement (less than horizontal friction)
        velocity->vy *= std::pow(m_friction * 0.99f, deltaTime);
    }
    
    void ApplyWorldBoundaries(Entity entity, TransformComponent* transform, VelocityComponent* velocity) {
        if (!m_hasBoundaries) return;
        
        auto* collision = m_entityManager->GetComponent<CollisionComponent>(entity);
        float width = collision ? collision->width : 32.0f;
        float height = collision ? collision->height : 32.0f;
        
        // X boundaries
        if (transform->x < m_worldMinX) {
            transform->x = m_worldMinX;
            velocity->vx = std::max(0.0f, velocity->vx); // Stop leftward movement
        } else if (transform->x + width > m_worldMaxX) {
            transform->x = m_worldMaxX - width;
            velocity->vx = std::min(0.0f, velocity->vx); // Stop rightward movement
        }
        
        // Y boundaries
        if (transform->y < m_worldMinY) {
            transform->y = m_worldMinY;
            velocity->vy = std::max(0.0f, velocity->vy); // Stop upward movement
        } else if (transform->y + height > m_worldMaxY) {
            transform->y = m_worldMaxY - height;
            velocity->vy = std::min(0.0f, velocity->vy); // Stop downward movement
            
            // Ground collision - could fire an event here
            if (m_eventManager && velocity->vy > 100.0f) {
                // Fire ground impact event for hard landings
            }
        }
    }
    
    void UpdateFacingDirection(Entity entity, float deltaX) {
        auto* sprite = m_entityManager->GetComponent<SpriteComponent>(entity);
        if (!sprite) return;
        
        // Update facing direction based on movement
        if (std::abs(deltaX) > 0.1f) {
            sprite->flipHorizontal = (deltaX < 0.0f);
        }
    }
};

/**
 * @class PlatformerMovementSystem
 * @brief Specialized movement system for platformer games
 *
 * Provides platformer-specific movement mechanics like jumping,
 * ground detection, and platform collision.
 */
class PlatformerMovementSystem : public EnhancedMovementSystem {
public:
    PlatformerMovementSystem(EventManager* eventManager = nullptr) 
        : EnhancedMovementSystem(eventManager), m_jumpStrength(300.0f) {}
    
    /**
     * @brief Make an entity jump if it's on the ground
     */
    bool Jump(Entity entity) {
        if (!IsOnGround(entity)) return false;
        
        auto* velocity = m_entityManager->GetComponent<VelocityComponent>(entity);
        auto* stats = m_entityManager->GetComponent<CharacterStatsComponent>(entity);
        
        if (!velocity) return false;
        
        float jumpPower = m_jumpStrength;
        if (stats) {
            // Scale jump power with agility
            jumpPower += stats->agility * 5.0f;
        }
        
        velocity->vy = -jumpPower; // Negative Y is up
        
        // Fire jump event
        if (m_eventManager) {
            // Could fire a jump event here
        }
        
        return true;
    }
    
    /**
     * @brief Check if entity is on the ground
     */
    bool IsOnGround(Entity entity) {
        auto* transform = m_entityManager->GetComponent<TransformComponent>(entity);
        auto* collision = m_entityManager->GetComponent<CollisionComponent>(entity);
        
        if (!transform || !collision) return false;
        
        // Simple ground check - entity is on ground if at bottom boundary
        float entityBottom = transform->y + collision->height;
        return entityBottom >= m_worldMaxY - 5.0f; // 5 pixel tolerance
    }
    
    /**
     * @brief Set jump strength
     */
    void SetJumpStrength(float strength) {
        m_jumpStrength = strength;
    }

private:
    float m_jumpStrength;
};
