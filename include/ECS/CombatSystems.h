/**
 * @file CombatSystems.h
 * @brief Combat-related ECS systems for turn-based battle mechanics
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "System.h"
#include "EntityManager.h"
#include "Component.h"
#include "Game/GameConfig.h"
#include <functional>
#include <vector>
#include <memory>

/**
 * @struct CombatEvent
 * @brief Information about combat events
 */
struct CombatEvent {
    enum class Type {
        TURN_START,
        ACTION_EXECUTED,
        DAMAGE_DEALT,
        ENTITY_DEFEATED,
        BATTLE_END
    };
    
    Type type;
    Entity source;
    Entity target;
    float value = 0.0f;
    std::string message;
    
    CombatEvent(Type t, Entity src = Entity(), Entity tgt = Entity(), float val = 0.0f)
        : type(t), source(src), target(tgt), value(val) {}
};

/**
 * @class TurnManagementSystem
 * @brief Manages turn order and initiative in combat
 */
class TurnManagementSystem : public System {
public:
    using CombatEventCallback = std::function<void(const CombatEvent&)>;
    
    TurnManagementSystem(GameConfig* config = nullptr) 
        : m_config(config), m_currentTurnIndex(0), m_roundNumber(1) {}
    
    void Update(float deltaTime) override;
    
    /**
     * @brief Initialize combat with participants
     */
    void InitializeCombat(const std::vector<Entity>& participants);
    
    /**
     * @brief Get current active entity
     */
    Entity GetCurrentTurnEntity() const;
    
    /**
     * @brief Advance to next turn
     */
    void AdvanceToNextTurn();
    
    /**
     * @brief Check if all entities have taken their turn this round
     */
    bool IsRoundComplete() const;
    
    /**
     * @brief Start new combat round
     */
    void StartNewRound();
    
    /**
     * @brief Set callback for combat events
     */
    void SetEventCallback(CombatEventCallback callback) { m_eventCallback = callback; }
    
    /**
     * @brief Get turn order list
     */
    const std::vector<Entity>& GetTurnOrder() const { return m_turnOrder; }

private:
    GameConfig* m_config;
    std::vector<Entity> m_turnOrder;
    int m_currentTurnIndex;
    int m_roundNumber;
    CombatEventCallback m_eventCallback;
    
    void CalculateInitiative();
    void SortByInitiative();
};

/**
 * @class CombatActionSystem
 * @brief Processes combat actions and applies effects
 */
class CombatActionSystem : public System {
public:
    using CombatEventCallback = std::function<void(const CombatEvent&)>;
    
    CombatActionSystem(GameConfig* config = nullptr) : m_config(config) {}
    
    void Update(float deltaTime) override;
    
    /**
     * @brief Execute an attack action
     */
    void ExecuteAttack(Entity attacker, Entity target);
    
    /**
     * @brief Execute a defend action
     */
    void ExecuteDefend(Entity defender);
    
    /**
     * @brief Execute a magic action
     */
    void ExecuteMagic(Entity caster, Entity target, int spellIndex = 0);
    
    /**
     * @brief Execute an item use action
     */
    void ExecuteItem(Entity user, Entity target, int itemIndex = 0);
    
    /**
     * @brief Attempt to flee from combat
     */
    bool ExecuteFlee(Entity entity);
    
    /**
     * @brief Set callback for combat events
     */
    void SetEventCallback(CombatEventCallback callback) { m_eventCallback = callback; }

private:
    GameConfig* m_config;
    CombatEventCallback m_eventCallback;
    
    float CalculateDamage(Entity attacker, Entity target, float baseDamage);
    bool CheckHit(Entity attacker, Entity target);
    bool CheckCritical(Entity attacker);
    void ApplyDamage(Entity target, float damage);
    void ApplyDefenseBonus(Entity defender);
};

/**
 * @class CombatUISystem
 * @brief Manages combat user interface and visual feedback
 */
class CombatUISystem : public System {
public:
    CombatUISystem(GameConfig* config = nullptr) : m_config(config) {}
    
    void Update(float deltaTime) override;
    
    /**
     * @brief Set the entities to display in combat UI
     */
    void SetCombatParticipants(const std::vector<Entity>& participants);
    
    /**
     * @brief Show a combat message
     */
    void ShowMessage(const std::string& message, float duration = 2.0f);
    
    /**
     * @brief Highlight current turn entity
     */
    void SetCurrentTurnEntity(Entity entity);

private:
    GameConfig* m_config;
    std::vector<Entity> m_participants;
    Entity m_currentTurnEntity;
    std::string m_currentMessage;
    float m_messageTimer = 0.0f;
    
    void UpdateMessageTimer(float deltaTime);
};

/**
 * @class CombatResolutionSystem
 * @brief Handles combat state resolution and battle end conditions
 */
class CombatResolutionSystem : public System {
public:
    using BattleEndCallback = std::function<void(bool playerWon, int experience, int gold)>;
    
    CombatResolutionSystem(GameConfig* config = nullptr) : m_config(config) {}
    
    void Update(float deltaTime) override;
    
    /**
     * @brief Check if battle should end
     */
    bool IsBattleOver() const;
    
    /**
     * @brief Check if player side won
     */
    bool DidPlayerWin() const;
    
    /**
     * @brief Calculate battle rewards
     */
    void CalculateRewards(int& experience, int& gold) const;
    
    /**
     * @brief Set callback for battle end
     */
    void SetBattleEndCallback(BattleEndCallback callback) { m_battleEndCallback = callback; }

private:
    GameConfig* m_config;
    BattleEndCallback m_battleEndCallback;
    
    bool AreAllEnemiesDefeated() const;
    bool AreAllPlayersDefeated() const;
    int CountLivingEnemies() const;
    int GetTotalEnemyLevel() const;
};
