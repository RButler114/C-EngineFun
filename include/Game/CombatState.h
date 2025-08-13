#pragma once

#include "GameState.h"
#include "ECS/EntityManager.h"
#include "ECS/CombatSystems.h"
#include "Game/GameConfig.h"
#include "Game/GameStateManager.h"
#include "Game/CharacterFactory.h"
#include <memory>
#include <vector>

/**
 * @enum CombatPhase
 * @brief Different phases of turn-based combat
 */
enum class CombatPhase {
    BATTLE_START,    ///< Initialize battle, show participants
    TURN_START,      ///< Start of a character's turn
    ACTION_SELECT,   ///< Player selecting action
    ACTION_EXECUTE,  ///< Executing selected action
    TURN_END,        ///< End of turn, check for battle end
    BATTLE_END,      ///< Battle finished, show results
    TRANSITION_OUT   ///< Transitioning back to playing state
};

/**
 * @enum CombatAction
 * @brief Available combat actions
 */
enum class CombatAction {
    ATTACK,    ///< Physical attack
    DEFEND,    ///< Defensive stance, reduce damage
    MAGIC,     ///< Magic spell (if available)
    ITEM,      ///< Use item (if available)
    FLEE       ///< Attempt to flee from battle
};

/**
 * @struct CombatParticipant
 * @brief Information about a battle participant
 */
struct CombatParticipant {
    Entity entity;
    bool isPlayer;
    bool isAlive;
    int turnOrder;
    float initiative;
    float currentHealth;
    float maxHealth;
    float currentMana;
    float maxMana;

    CombatParticipant(Entity e, bool player, float init)
        : entity(e), isPlayer(player), isAlive(true), turnOrder(0), initiative(init),
          currentHealth(100.0f), maxHealth(100.0f), currentMana(30.0f), maxMana(30.0f) {}
};

/**
 * @class CombatState
 * @brief Turn-based combat state following Final Fantasy style mechanics
 * 
 * Features:
 * - Turn-based combat with initiative order
 * - Player action selection (Attack, Defend, Magic, Item, Flee)
 * - AI enemy actions
 * - Health/MP management
 * - Battle results and experience
 * - Smooth transitions back to playing state
 */
class CombatState : public GameState {
public:
    CombatState();
    ~CombatState() = default;
    
    // GameState interface
    void OnEnter() override;
    void OnExit() override;
    void Update(float deltaTime) override;
    void Render() override;
    void HandleInput() override;

    /**
     * @brief Initialize combat with specific participants
     * @param player Player entity
     * @param enemies Vector of enemy entities
     */
    void InitializeCombat(Entity player, const std::vector<Entity>& enemies);

    /**
     * @brief Inform combat whether this encounter is a boss fight
     */
    void SetBossEncounter(bool isBoss) { m_isBossEncounter = isBoss; }

    /**
     * @brief Set the position to return to after combat
     * @param x X coordinate
     * @param y Y coordinate
     */
    void SetReturnPosition(float x, float y) { m_returnX = x; m_returnY = y; }

private:
    // Core systems
    std::unique_ptr<EntityManager> m_entityManager;
    std::unique_ptr<GameConfig> m_gameConfig;
    
    // Combat state
    CombatPhase m_currentPhase;
    std::vector<CombatParticipant> m_participants;
    int m_currentTurnIndex;
    float m_phaseTimer;
    
    // Player action selection
    CombatAction m_selectedAction;
    int m_selectedActionIndex;
    std::vector<CombatAction> m_availableActions;
    
    // Battle results
    int m_experienceGained;
    int m_goldGained;
    bool m_playerVictory;
    bool m_playerFled;
    bool m_isBossEncounter = false;

    // Return position
    float m_returnX;
    float m_returnY;

    // UI state
    float m_messageTimer;
    std::string m_currentMessage;
    bool m_showActionMenu;

    // Item selection menu state (for CombatAction::ITEM)
    bool m_itemMenuOpen = false;
    int m_itemMenuIndex = 0; // index into filtered consumable list
    bool m_itemTargetSelecting = false; // selecting which party member to use on
    int m_itemTargetIndex = 0;          // index into filtered player list
    int m_itemMenuChosenIndex = -1;     // chosen consumable row before target select

    // Phase management
    void UpdateBattleStart(float deltaTime);
    void UpdateTurnStart(float deltaTime);
    void UpdateActionSelect(float deltaTime);
    void UpdateActionExecute(float deltaTime);
    void UpdateTurnEnd(float deltaTime);
    void UpdateBattleEnd(float deltaTime);
    void UpdateTransitionOut(float deltaTime);

    // Combat mechanics
    void CalculateTurnOrder();
    void ProcessPlayerAction();
    void ProcessEnemyAction(const CombatParticipant& participant);
    void ExecuteAttack(Entity attacker, Entity target);
    void ExecuteDefend(Entity defender);
    void ExecuteMagic(Entity caster, Entity target);
    bool AttemptFlee();

    // Battle state checks
    bool IsBattleOver() const;
    bool AreAllEnemiesDead() const;
    bool IsPlayerDead() const;

    // UI rendering
    void RenderCombatUI();
    void RenderParticipants();
    void RenderActionMenu();
    void RenderHealthBars();
    void RenderMessage();
    void RenderTurnIndicator();
    void RenderItemMenu();

    // Input handling
    void HandleActionSelection();
    void HandleMenuNavigation();
    void HandleItemMenuInput();

    // Utility
    void ShowMessage(const std::string& message, float duration = 2.0f);
    void AdvanceToNextTurn();
    void EndBattle(bool playerWon);
    void CalculateBattleRewards();
    void HandleCombatEvent(const CombatEvent& event);
    bool UseSelectedItem();

    // Action availability
    void UpdateAvailableActions();
    bool CanUseMagic(Entity entity) const;
    bool CanUseItems(Entity entity) const;
};
