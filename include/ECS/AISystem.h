/**
 * @file AISystem.h
 * @brief AI system for intelligent entity behavior
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "System.h"
#include "Component.h"
#include <cmath>
#include <algorithm>

/**
 * @class AISystem
 * @brief System that manages AI behavior for entities
 *
 * Processes entities with AIComponent to provide intelligent behavior
 * including state management, target detection, pathfinding, and combat AI.
 */
class AISystem : public System {
public:
    AISystem() = default;
    
    void Update(float deltaTime) override {
        auto entities = m_entityManager->GetEntitiesWith<AIComponent, TransformComponent>();
        
        for (Entity entity : entities) {
            auto* ai = m_entityManager->GetComponent<AIComponent>(entity);
            auto* transform = m_entityManager->GetComponent<TransformComponent>(entity);
            auto* health = m_entityManager->GetComponent<HealthComponent>(entity);
            
            if (!ai || !transform) continue;
            
            // Skip dead entities
            if (health && health->isDead) {
                ai->ChangeState(AIComponent::AIState::DEAD);
                continue;
            }
            
            ai->stateTimer += deltaTime;
            
            // Update AI based on current state
            switch (ai->currentState) {
                case AIComponent::AIState::IDLE:
                    UpdateIdleState(entity, ai, transform, deltaTime);
                    break;
                case AIComponent::AIState::PATROL:
                    UpdatePatrolState(entity, ai, transform, deltaTime);
                    break;
                case AIComponent::AIState::CHASE:
                    UpdateChaseState(entity, ai, transform, deltaTime);
                    break;
                case AIComponent::AIState::ATTACK:
                    UpdateAttackState(entity, ai, transform, deltaTime);
                    break;
                case AIComponent::AIState::FLEE:
                    UpdateFleeState(entity, ai, transform, deltaTime);
                    break;
                case AIComponent::AIState::SEARCH:
                    UpdateSearchState(entity, ai, transform, deltaTime);
                    break;
                case AIComponent::AIState::DEAD:
                    // Dead entities don't do anything
                    break;
            }
            
            // Check for state transitions
            CheckStateTransitions(entity, ai, transform, health);
        }
    }

private:
    void UpdateIdleState(Entity entity, AIComponent* ai, TransformComponent* transform, float deltaTime) {
        (void)deltaTime; // Suppress unused parameter warning
        // Look for targets if aggressive
        if (ai->aggressive) {
            Entity target = FindNearestTarget(entity, transform, ai->detectionRange);
            if (target.IsValid()) {
                ai->target = target;
                ai->ChangeState(AIComponent::AIState::CHASE);
                return;
            }
        }
        
        // Start patrolling if patrol points exist
        if (!ai->patrolPoints.empty() && ai->stateTimer > 2.0f) {
            ai->ChangeState(AIComponent::AIState::PATROL);
        }
    }
    
    void UpdatePatrolState(Entity entity, AIComponent* ai, TransformComponent* transform, float deltaTime) {
        if (ai->patrolPoints.empty()) {
            ai->ChangeState(AIComponent::AIState::IDLE);
            return;
        }
        
        // Look for targets while patrolling
        if (ai->aggressive) {
            Entity target = FindNearestTarget(entity, transform, ai->detectionRange);
            if (target.IsValid()) {
                ai->target = target;
                ai->ChangeState(AIComponent::AIState::CHASE);
                return;
            }
        }
        
        // Move towards current patrol point
        auto& currentPoint = ai->patrolPoints[ai->currentPatrolIndex];
        float dx = currentPoint.first - transform->x;
        float dy = currentPoint.second - transform->y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        if (distance < 10.0f) {
            // Reached patrol point, move to next
            if (ai->patrolReverse) {
                ai->currentPatrolIndex--;
                if (ai->currentPatrolIndex < 0) {
                    ai->currentPatrolIndex = 1;
                    ai->patrolReverse = false;
                }
            } else {
                ai->currentPatrolIndex++;
                if (ai->currentPatrolIndex >= static_cast<int>(ai->patrolPoints.size())) {
                    ai->currentPatrolIndex = static_cast<int>(ai->patrolPoints.size()) - 2;
                    ai->patrolReverse = true;
                }
            }
        } else {
            // Move towards patrol point
            MoveTowards(entity, transform, currentPoint.first, currentPoint.second, ai->patrolSpeed, deltaTime);
        }
    }
    
    void UpdateChaseState(Entity entity, AIComponent* ai, TransformComponent* transform, float deltaTime) {
        if (!ai->target.IsValid()) {
            ai->ChangeState(AIComponent::AIState::SEARCH);
            return;
        }
        
        auto* targetTransform = m_entityManager->GetComponent<TransformComponent>(ai->target);
        if (!targetTransform) {
            ai->target = Entity(); // Invalid target
            ai->ChangeState(AIComponent::AIState::SEARCH);
            return;
        }
        
        float distance = GetDistance(transform, targetTransform);
        
        // Switch to attack if close enough
        if (distance <= ai->attackRange) {
            ai->ChangeState(AIComponent::AIState::ATTACK);
            return;
        }
        
        // Lose target if too far away
        if (distance > ai->detectionRange * 1.5f) {
            ai->target = Entity();
            ai->ChangeState(AIComponent::AIState::SEARCH);
            return;
        }
        
        // Chase the target
        MoveTowards(entity, transform, targetTransform->x, targetTransform->y, ai->chaseSpeed, deltaTime);
    }
    
    void UpdateAttackState(Entity entity, AIComponent* ai, TransformComponent* transform, float deltaTime) {
        (void)deltaTime; // Suppress unused parameter warning
        if (!ai->target.IsValid()) {
            ai->ChangeState(AIComponent::AIState::SEARCH);
            return;
        }
        
        auto* targetTransform = m_entityManager->GetComponent<TransformComponent>(ai->target);
        if (!targetTransform) {
            ai->target = Entity();
            ai->ChangeState(AIComponent::AIState::SEARCH);
            return;
        }
        
        float distance = GetDistance(transform, targetTransform);
        
        // Move back to chase if target is too far
        if (distance > ai->attackRange * 1.2f) {
            ai->ChangeState(AIComponent::AIState::CHASE);
            return;
        }
        
        // Attack if cooldown is ready
        float currentTime = ai->stateTimer; // Simplified time tracking
        if (ai->CanAttack(currentTime)) {
            PerformAttack(entity, ai->target);
            ai->lastAttackTime = currentTime;
        }
    }
    
    void UpdateFleeState(Entity entity, AIComponent* ai, TransformComponent* transform, float deltaTime) {
        (void)deltaTime; // Suppress unused parameter warning
        if (!ai->target.IsValid()) {
            ai->ChangeState(AIComponent::AIState::IDLE);
            return;
        }
        
        auto* targetTransform = m_entityManager->GetComponent<TransformComponent>(ai->target);
        if (!targetTransform) {
            ai->target = Entity();
            ai->ChangeState(AIComponent::AIState::IDLE);
            return;
        }
        
        float distance = GetDistance(transform, targetTransform);
        
        // Stop fleeing if far enough away
        if (distance > ai->detectionRange) {
            ai->target = Entity();
            ai->ChangeState(AIComponent::AIState::IDLE);
            return;
        }
        
        // Flee away from target
        float dx = transform->x - targetTransform->x;
        float dy = transform->y - targetTransform->y;
        float length = std::sqrt(dx * dx + dy * dy);
        
        if (length > 0.0f) {
            dx /= length;
            dy /= length;
            
            auto* velocity = m_entityManager->GetComponent<VelocityComponent>(entity);
            if (velocity) {
                velocity->vx = dx * ai->fleeSpeed;
                velocity->vy = dy * ai->fleeSpeed;
            }
        }
    }
    
    void UpdateSearchState(Entity entity, AIComponent* ai, TransformComponent* transform, float deltaTime) {
        (void)deltaTime; // Suppress unused parameter warning
        // Look for new targets
        Entity target = FindNearestTarget(entity, transform, ai->detectionRange);
        if (target.IsValid()) {
            ai->target = target;
            ai->ChangeState(AIComponent::AIState::CHASE);
            return;
        }
        
        // Return to patrol or idle after searching for a while
        if (ai->stateTimer > 3.0f) {
            if (ai->returnsToPatrol && !ai->patrolPoints.empty()) {
                ai->ChangeState(AIComponent::AIState::PATROL);
            } else {
                ai->ChangeState(AIComponent::AIState::IDLE);
            }
        }
    }
    
    void CheckStateTransitions(Entity entity, AIComponent* ai, TransformComponent* transform, HealthComponent* health) {
        (void)entity; (void)transform; // Suppress unused parameter warnings
        // Check if should flee due to low health
        if (ai->canFlee && health && health->GetHealthPercentage() < 0.3f && ai->target.IsValid()) {
            if (ai->currentState != AIComponent::AIState::FLEE) {
                ai->ChangeState(AIComponent::AIState::FLEE);
            }
        }
    }
    
    Entity FindNearestTarget(Entity searcher, TransformComponent* searcherTransform, float range) {
        Entity nearestTarget;
        float nearestDistance = range;
        
        // Look for player entities (simplified - in a full system you'd have better target filtering)
        auto entities = m_entityManager->GetEntitiesWith<TransformComponent, CharacterTypeComponent>();
        
        for (Entity entity : entities) {
            if (entity == searcher) continue;
            
            auto* transform = m_entityManager->GetComponent<TransformComponent>(entity);
            auto* characterType = m_entityManager->GetComponent<CharacterTypeComponent>(entity);
            auto* health = m_entityManager->GetComponent<HealthComponent>(entity);
            
            if (!transform || !characterType) continue;
            if (health && health->isDead) continue;
            
            // Only target players (this could be made more sophisticated)
            if (characterType->type != CharacterTypeComponent::CharacterType::PLAYER) continue;
            
            float distance = GetDistance(searcherTransform, transform);
            if (distance < nearestDistance) {
                nearestDistance = distance;
                nearestTarget = entity;
            }
        }
        
        return nearestTarget;
    }
    
    void MoveTowards(Entity entity, TransformComponent* transform, float targetX, float targetY, float speed, float deltaTime) {
        (void)deltaTime; // Suppress unused parameter warning
        float dx = targetX - transform->x;
        float dy = targetY - transform->y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        if (distance > 0.0f) {
            dx /= distance;
            dy /= distance;
            
            auto* velocity = m_entityManager->GetComponent<VelocityComponent>(entity);
            if (velocity) {
                velocity->vx = dx * speed;
                velocity->vy = dy * speed;
            }
        }
    }
    
    float GetDistance(const TransformComponent* a, const TransformComponent* b) {
        float dx = a->x - b->x;
        float dy = a->y - b->y;
        return std::sqrt(dx * dx + dy * dy);
    }
    
    void PerformAttack(Entity attacker, Entity target) {
        // This would trigger an attack - could send an event, deal damage, etc.
        // For now, we'll just deal basic damage if the target has health
        auto* targetHealth = m_entityManager->GetComponent<HealthComponent>(target);
        auto* attackerStats = m_entityManager->GetComponent<CharacterStatsComponent>(attacker);
        
        if (targetHealth && attackerStats) {
            float damage = attackerStats->attackDamage;
            targetHealth->TakeDamage(damage);
        }
    }
};
