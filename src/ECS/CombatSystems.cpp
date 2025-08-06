/**
 * @file CombatSystems.cpp
 * @brief Implementation of turn-based combat systems
 * @author Ryan Butler
 * @date 2025
 */

#include "ECS/CombatSystems.h"
#include <algorithm>
#include <random>
#include <iostream>

// ========== TURN MANAGEMENT SYSTEM IMPLEMENTATION ==========

/**
 * @brief Update turn management system (event-driven)
 *
 * The turn management system is primarily event-driven rather than
 * time-based. Most of the logic happens in response to combat actions
 * and turn advancement rather than continuous updates.
 *
 * @param deltaTime Time elapsed since last frame (unused)
 *
 * @note This system responds to events rather than running continuous logic
 * @note Turn advancement happens via AdvanceTurn() method calls
 * @note Combat timing is controlled by the CombatState, not this system
 */
void TurnManagementSystem::Update(float deltaTime) {
    // This system is primarily event-driven, not time-based
    // Most logic happens in response to actions and turn advances
    (void)deltaTime; // Suppress unused parameter warning
}

/**
 * @brief Initialize combat encounter with participating entities
 *
 * Sets up a new combat encounter by:
 * 1. Registering all combat participants
 * 2. Adding TurnOrderComponent to entities that need it
 * 3. Calculating initiative values for turn order
 * 4. Sorting participants by initiative (highest first)
 * 5. Setting up first turn and round tracking
 *
 * @param participants Vector of entities participating in combat
 *
 * @note Entities without CombatStatsComponent get default initiative
 * @note Turn order is sorted by initiative (highest goes first)
 * @note Combat starts with round 1, turn index 0
 *
 * @example
 * ```cpp
 * std::vector<Entity> combatants = {player, enemy1, enemy2};
 * turnSystem.InitializeCombat(combatants);
 * // Combat is now ready, first entity can take their turn
 * ```
 */
void TurnManagementSystem::InitializeCombat(const std::vector<Entity>& participants) {
    // Store combat participants and reset turn tracking
    m_turnOrder = participants;
    m_currentTurnIndex = 0;  // Start with first participant
    m_roundNumber = 1;       // Begin round 1

    // Initialize turn order components for all participants
    for (Entity entity : participants) {
        if (!m_entityManager->HasComponent<TurnOrderComponent>(entity)) {
            // Add turn order component for combat tracking
            auto* turnOrder = m_entityManager->AddComponent<TurnOrderComponent>(entity);

            // Set base initiative from combat stats or use default
            if (auto* combatStats = m_entityManager->GetComponent<CombatStatsComponent>(entity)) {
                turnOrder->initiative = combatStats->speed; // Speed determines initiative
            } else {
                turnOrder->initiative = 10; // Default initiative for entities without combat stats
            }
        }
    }

    // Calculate final initiative values (may include random modifiers)
    CalculateInitiative();

    // Sort participants by initiative (highest goes first)
    SortByInitiative();
    
    // Fire turn start event for first entity
    if (!m_turnOrder.empty() && m_eventCallback) {
        m_eventCallback(CombatEvent(CombatEvent::Type::TURN_START, m_turnOrder[0]));
    }
    
    std::cout << "Combat initialized with " << participants.size() << " participants" << std::endl;
}

Entity TurnManagementSystem::GetCurrentTurnEntity() const {
    if (m_currentTurnIndex < m_turnOrder.size()) {
        return m_turnOrder[m_currentTurnIndex];
    }
    return Entity(); // Invalid entity
}

void TurnManagementSystem::AdvanceToNextTurn() {
    // Mark current entity as having taken their turn
    if (m_currentTurnIndex < m_turnOrder.size()) {
        Entity currentEntity = m_turnOrder[m_currentTurnIndex];
        if (auto* turnOrder = m_entityManager->GetComponent<TurnOrderComponent>(currentEntity)) {
            turnOrder->hasTakenTurn = true;
        }
    }
    
    m_currentTurnIndex++;
    
    // Check if round is complete
    if (IsRoundComplete()) {
        StartNewRound();
    } else if (m_currentTurnIndex < m_turnOrder.size()) {
        // Fire turn start event for next entity
        if (m_eventCallback) {
            m_eventCallback(CombatEvent(CombatEvent::Type::TURN_START, m_turnOrder[m_currentTurnIndex]));
        }
    }
}

bool TurnManagementSystem::IsRoundComplete() const {
    return m_currentTurnIndex >= m_turnOrder.size();
}

void TurnManagementSystem::StartNewRound() {
    m_roundNumber++;
    m_currentTurnIndex = 0;
    
    // Reset turn flags for all participants
    for (Entity entity : m_turnOrder) {
        if (auto* turnOrder = m_entityManager->GetComponent<TurnOrderComponent>(entity)) {
            turnOrder->ResetForNewRound();
        }
    }
    
    // Recalculate initiative for new round
    CalculateInitiative();
    SortByInitiative();
    
    // Fire turn start event for first entity of new round
    if (!m_turnOrder.empty() && m_eventCallback) {
        m_eventCallback(CombatEvent(CombatEvent::Type::TURN_START, m_turnOrder[0]));
    }
    
    std::cout << "Starting combat round " << m_roundNumber << std::endl;
}

void TurnManagementSystem::CalculateInitiative() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.8f, 1.2f); // ±20% variance
    
    for (Entity entity : m_turnOrder) {
        if (auto* turnOrder = m_entityManager->GetComponent<TurnOrderComponent>(entity)) {
            // Add some randomness to initiative
            turnOrder->currentInitiative = turnOrder->initiative * dis(gen);
        }
    }
}

void TurnManagementSystem::SortByInitiative() {
    std::sort(m_turnOrder.begin(), m_turnOrder.end(),
        [this](Entity a, Entity b) {
            auto* turnOrderA = m_entityManager->GetComponent<TurnOrderComponent>(a);
            auto* turnOrderB = m_entityManager->GetComponent<TurnOrderComponent>(b);
            
            if (!turnOrderA || !turnOrderB) return false;
            
            return turnOrderA->currentInitiative > turnOrderB->currentInitiative;
        });
    
    // Update turn order indices
    for (size_t i = 0; i < m_turnOrder.size(); ++i) {
        if (auto* turnOrder = m_entityManager->GetComponent<TurnOrderComponent>(m_turnOrder[i])) {
            turnOrder->turnOrder = static_cast<int>(i);
        }
    }
}

// CombatActionSystem implementation
void CombatActionSystem::Update(float deltaTime) {
    // This system is primarily action-driven, not time-based
}

void CombatActionSystem::ExecuteAttack(Entity attacker, Entity target) {
    auto* attackerStats = m_entityManager->GetComponent<CombatStatsComponent>(attacker);
    auto* targetHealth = m_entityManager->GetComponent<HealthComponent>(target);
    
    if (!attackerStats || !targetHealth) {
        std::cout << "Attack failed: missing components" << std::endl;
        return;
    }
    
    // Check if attack hits
    if (!CheckHit(attacker, target)) {
        if (m_eventCallback) {
            CombatEvent event(CombatEvent::Type::ACTION_EXECUTED, attacker, target);
            event.message = "Attack missed!";
            m_eventCallback(event);
        }
        return;
    }
    
    // Calculate damage
    float damage = CalculateDamage(attacker, target, attackerStats->attackPower);
    
    // Check for critical hit
    if (CheckCritical(attacker)) {
        damage *= attackerStats->criticalMultiplier;
        if (m_eventCallback) {
            CombatEvent event(CombatEvent::Type::ACTION_EXECUTED, attacker, target, damage);
            event.message = "Critical hit!";
            m_eventCallback(event);
        }
    }
    
    // Apply damage
    ApplyDamage(target, damage);
    
    // Fire damage event
    if (m_eventCallback) {
        CombatEvent event(CombatEvent::Type::DAMAGE_DEALT, attacker, target, damage);
        event.message = "Dealt " + std::to_string((int)damage) + " damage!";
        m_eventCallback(event);
    }
    
    // Check if target is defeated
    if (targetHealth->currentHealth <= 0.0f && m_eventCallback) {
        CombatEvent event(CombatEvent::Type::ENTITY_DEFEATED, attacker, target);
        event.message = "Enemy defeated!";
        m_eventCallback(event);
    }
}

void CombatActionSystem::ExecuteDefend(Entity defender) {
    ApplyDefenseBonus(defender);
    
    if (m_eventCallback) {
        CombatEvent event(CombatEvent::Type::ACTION_EXECUTED, defender);
        event.message = "Defending! Damage reduced next turn.";
        m_eventCallback(event);
    }
}

void CombatActionSystem::ExecuteMagic(Entity caster, Entity target, int spellIndex) {
    // TODO: Implement magic system
    if (m_eventCallback) {
        CombatEvent event(CombatEvent::Type::ACTION_EXECUTED, caster, target);
        event.message = "Magic not yet implemented!";
        m_eventCallback(event);
    }
}

void CombatActionSystem::ExecuteItem(Entity user, Entity target, int itemIndex) {
    // TODO: Implement item system
    if (m_eventCallback) {
        CombatEvent event(CombatEvent::Type::ACTION_EXECUTED, user, target);
        event.message = "Items not yet implemented!";
        m_eventCallback(event);
    }
}

bool CombatActionSystem::ExecuteFlee(Entity entity) {
    float fleeChance = m_config ? m_config->GetFleeSuccessRate() : 75.0f;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 100.0f);
    
    bool success = dis(gen) < fleeChance;
    
    if (m_eventCallback) {
        CombatEvent event(CombatEvent::Type::ACTION_EXECUTED, entity);
        event.message = success ? "Successfully fled!" : "Couldn't escape!";
        m_eventCallback(event);
    }
    
    return success;
}

float CombatActionSystem::CalculateDamage(Entity attacker, Entity target, float baseDamage) {
    auto* targetStats = m_entityManager->GetComponent<CombatStatsComponent>(target);
    auto* targetTurnOrder = m_entityManager->GetComponent<TurnOrderComponent>(target);
    
    float damage = baseDamage;
    
    // Apply defense
    if (targetStats) {
        float defense = targetStats->defense;
        
        // Add defense bonus if defending
        if (targetTurnOrder && targetTurnOrder->isDefending) {
            defense += targetTurnOrder->defenseBonus;
        }
        
        damage = std::max(1.0f, damage - defense);
    }
    
    // Apply damage variance
    if (m_config) {
        float variance = m_config->GetDamageVariance();
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(1.0f - variance, 1.0f + variance);
        damage *= dis(gen);
    }
    
    return damage;
}

bool CombatActionSystem::CheckHit(Entity attacker, Entity target) {
    auto* attackerStats = m_entityManager->GetComponent<CombatStatsComponent>(attacker);
    float accuracy = attackerStats ? attackerStats->accuracy : (m_config ? m_config->GetAccuracyBase() : 85.0f);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 100.0f);
    
    return dis(gen) < accuracy;
}

bool CombatActionSystem::CheckCritical(Entity attacker) {
    auto* attackerStats = m_entityManager->GetComponent<CombatStatsComponent>(attacker);
    float critChance = attackerStats ? attackerStats->criticalChance : (m_config ? m_config->GetCriticalChance() : 5.0f);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 100.0f);
    
    return dis(gen) < critChance;
}

void CombatActionSystem::ApplyDamage(Entity target, float damage) {
    auto* health = m_entityManager->GetComponent<HealthComponent>(target);
    if (health) {
        health->currentHealth = std::max(0.0f, health->currentHealth - damage);
        
        // Mark as dead if health reaches 0
        if (health->currentHealth <= 0.0f) {
            health->isDead = true;
        }
    }
}

void CombatActionSystem::ApplyDefenseBonus(Entity defender) {
    auto* turnOrder = m_entityManager->GetComponent<TurnOrderComponent>(defender);
    if (turnOrder) {
        turnOrder->isDefending = true;
        turnOrder->defenseBonus = 5.0f; // Base defense bonus
    }
}

// CombatUISystem implementation
void CombatUISystem::Update(float deltaTime) {
    UpdateMessageTimer(deltaTime);
}

void CombatUISystem::SetCombatParticipants(const std::vector<Entity>& participants) {
    m_participants = participants;
}

void CombatUISystem::ShowMessage(const std::string& message, float duration) {
    m_currentMessage = message;
    m_messageTimer = duration;
}

void CombatUISystem::SetCurrentTurnEntity(Entity entity) {
    m_currentTurnEntity = entity;
}

void CombatUISystem::UpdateMessageTimer(float deltaTime) {
    if (m_messageTimer > 0.0f) {
        m_messageTimer -= deltaTime;
        if (m_messageTimer <= 0.0f) {
            m_currentMessage.clear();
        }
    }
}

// CombatResolutionSystem implementation
void CombatResolutionSystem::Update(float deltaTime) {
    if (IsBattleOver() && m_battleEndCallback) {
        bool playerWon = DidPlayerWin();
        int experience, gold;
        CalculateRewards(experience, gold);

        m_battleEndCallback(playerWon, experience, gold);
    }
}

bool CombatResolutionSystem::IsBattleOver() const {
    return AreAllEnemiesDefeated() || AreAllPlayersDefeated();
}

bool CombatResolutionSystem::DidPlayerWin() const {
    return AreAllEnemiesDefeated() && !AreAllPlayersDefeated();
}

void CombatResolutionSystem::CalculateRewards(int& experience, int& gold) const {
    if (!DidPlayerWin()) {
        experience = 0;
        gold = 0;
        return;
    }

    int baseExp = m_config ? m_config->GetBaseExperience() : 50;
    int baseGold = m_config ? m_config->GetBaseGold() : 25;
    int expPerLevel = m_config ? m_config->GetExperiencePerEnemyLevel() : 10;
    int goldPerLevel = m_config ? m_config->GetGoldPerEnemyLevel() : 5;

    int enemyCount = CountLivingEnemies();
    int totalEnemyLevel = GetTotalEnemyLevel();

    experience = baseExp + (totalEnemyLevel * expPerLevel);
    gold = baseGold + (totalEnemyLevel * goldPerLevel);

    std::cout << "Battle rewards: " << experience << " XP, " << gold << " gold" << std::endl;
}

bool CombatResolutionSystem::AreAllEnemiesDefeated() const {
    auto entities = m_entityManager->GetEntitiesWith<BattleParticipantComponent, HealthComponent>();

    for (Entity entity : entities) {
        auto* participant = m_entityManager->GetComponent<BattleParticipantComponent>(entity);
        auto* health = m_entityManager->GetComponent<HealthComponent>(entity);

        if (participant && health &&
            participant->type == BattleParticipantComponent::ParticipantType::ENEMY &&
            participant->isAlive && !health->isDead) {
            return false;
        }
    }

    return true;
}

bool CombatResolutionSystem::AreAllPlayersDefeated() const {
    auto entities = m_entityManager->GetEntitiesWith<BattleParticipantComponent, HealthComponent>();

    for (Entity entity : entities) {
        auto* participant = m_entityManager->GetComponent<BattleParticipantComponent>(entity);
        auto* health = m_entityManager->GetComponent<HealthComponent>(entity);

        if (participant && health &&
            participant->type == BattleParticipantComponent::ParticipantType::PLAYER &&
            participant->isAlive && !health->isDead) {
            return false;
        }
    }

    return true;
}

int CombatResolutionSystem::CountLivingEnemies() const {
    int count = 0;
    auto entities = m_entityManager->GetEntitiesWith<BattleParticipantComponent, HealthComponent>();

    for (Entity entity : entities) {
        auto* participant = m_entityManager->GetComponent<BattleParticipantComponent>(entity);
        auto* health = m_entityManager->GetComponent<HealthComponent>(entity);

        if (participant && health &&
            participant->type == BattleParticipantComponent::ParticipantType::ENEMY &&
            participant->isAlive && !health->isDead) {
            count++;
        }
    }

    return count;
}

int CombatResolutionSystem::GetTotalEnemyLevel() const {
    // For now, assume all enemies are level 1
    // TODO: Add level component and proper level calculation
    return CountLivingEnemies();
}
