/**
 * @file PlayingState.h
 * @brief Main arcade gameplay state with side-scrolling action
 * @author Ryan Butler
 * @date 2025
 *
 * This file implements the core arcade gameplay state where players:
 * - Control a character in a side-scrolling environment
 * - Encounter enemies and obstacles
 * - Trigger turn-based combat on collisions
 * - Accumulate score and progress through the game
 * - Experience dynamic difficulty scaling
 *
 * Key Features:
 * - Entity-Component-System architecture for game objects
 * - Collision detection and response
 * - Scrolling camera system
 * - HUD with score, health, and game time
 * - Seamless transition to combat state
 * - Configuration-driven character creation
 * - Player customization integration
 */

#pragma once

#include "GameState.h"
#include "ECS/EntityManager.h"      // Entity-Component-System management
#include "ECS/CollisionSystem.h"    // Collision detection and response
#include "Game/GameConfig.h"        // Game configuration and settings
#include "Game/CharacterFactory.h"  // Character creation and customization
#include "Game/PlayerCustomization.h" // Player appearance and stats
#include <memory>

/**
 * @class PlayingState
 * @brief Main arcade gameplay state with side-scrolling action
 *
 * This state implements the core arcade gameplay experience:
 * - Side-scrolling movement with camera following
 * - Player character control (WASD/Arrow keys)
 * - Enemy spawning and AI behavior
 * - Collision detection triggering combat encounters
 * - Score tracking and game progression
 * - Dynamic difficulty adjustment
 * - Seamless state transitions (to combat, game over, etc.)
 *
 * Architecture:
 * - Uses Entity-Component-System for game objects
 * - Hybrid approach: ECS for complex entities, direct variables for player
 * - Configuration-driven enemy creation and difficulty
 * - Collision cooldown system prevents spam triggering
 *
 * Game Flow:
 * 1. Player moves through side-scrolling environment
 * 2. Enemies spawn based on time and difficulty
 * 3. Collision with enemy triggers combat state
 * 4. Combat resolves, returns to playing state
 * 5. Score increases, difficulty scales up
 * 6. Game over when player health reaches zero
 *
 * @example
 * ```cpp
 * // In GameStateManager setup:
 * manager.AddState(GameStateType::PLAYING, std::make_unique<PlayingState>());
 *
 * // Transition to playing state:
 * manager.ChangeState(GameStateType::PLAYING);
 * ```
 */
class PlayingState : public GameState {
public:
    /**
     * @brief Constructor - initializes playing state
     *
     * Sets up the playing state with default values and prepares
     * for initialization in OnEnter().
     */
    PlayingState();

    /**
     * @brief Destructor - ensures proper cleanup
     *
     * Automatically cleans up all resources via smart pointers.
     * OnExit() handles game-specific cleanup.
     */
    ~PlayingState();

    /**
     * @brief Initialize gameplay systems when state becomes active
     *
     * Called when transitioning to playing state. Sets up:
     * - Entity management system
     * - Game configuration loading
     * - Character factory initialization
     * - Player character creation
     * - Initial enemy spawning
     * - Camera and HUD setup
     */
    void OnEnter() override;

    /**
     * @brief Clean up gameplay systems when leaving state
     *
     * Called when transitioning away from playing state.
     * Handles cleanup and resource management.
     */
    void OnExit() override;

    /**
     * @brief Update gameplay logic each frame
     *
     * @param deltaTime Time elapsed since last frame in seconds
     *
     * Updates:
     * - Player movement and animation
     * - Enemy AI and spawning
     * - Collision detection
     * - Camera following
     * - Score and game time
     * - Game over conditions
     */
    void Update(float deltaTime) override;

    /**
     * @brief Render all gameplay visuals
     *
     * Draws in order:
     * - Scrolling background
     * - Game entities (player, enemies, objects)
     * - HUD elements (score, health, time)
     * - Debug information (if enabled)
     */
    void Render() override;

    /**
     * @brief Handle player input for gameplay
     *
     * Processes:
     * - Movement controls (WASD, Arrow keys)
     * - Action buttons (Space, Enter)
     * - Menu access (Escape)
     * - Debug commands (if enabled)
     */
    void HandleInput() override;

private:
    // ========== CORE GAME SYSTEMS ==========

    /**
     * @brief Entity-Component-System manager for game objects
     *
     * Manages all game entities (enemies, projectiles, pickups, etc.)
     * using the ECS architecture for flexible and efficient object management.
     */
    std::unique_ptr<EntityManager> m_entityManager;

    /**
     * @brief Game configuration system
     *
     * Loads and manages game settings from configuration files:
     * - Difficulty settings
     * - Character stats and abilities
     * - Spawn rates and timing
     * - Visual and audio preferences
     */
    std::unique_ptr<GameConfig> m_gameConfig;

    /**
     * @brief Factory for creating configured characters
     *
     * Creates player and enemy characters based on configuration data,
     * applying appropriate stats, abilities, and visual customizations.
     */
    std::unique_ptr<CharacterFactory> m_characterFactory;

    /**
     * @brief Player entity handle
     *
     * Reference to the player's entity in the ECS system.
     * Used for accessing player components and data.
     */
    Entity m_player;

    // ========== GAME STATE VARIABLES ==========

    /**
     * @brief Camera X position for side-scrolling
     *
     * Tracks the camera's horizontal position in world space.
     * Used to create the side-scrolling effect by offsetting all rendering.
     */
    float m_cameraX;

    /**
     * @brief Current player score
     *
     * Tracks points earned through gameplay (defeating enemies,
     * collecting items, survival time, etc.).
     */
    int m_score;

    /**
     * @brief Total time spent in current game session
     *
     * Accumulates time for scoring, difficulty scaling, and statistics.
     * Reset when starting a new game.
     */
    float m_gameTime;

    // ========== PLAYER POSITION SYSTEM ==========
    // Note: Using direct variables instead of ECS for player to avoid
    // potential ECS corruption issues during rapid state changes

    /**
     * @brief Player X position in world coordinates
     *
     * Direct position tracking for reliable player movement.
     * Bypasses ECS to ensure stability during state transitions.
     */
    float m_playerX;

    /**
     * @brief Player Y position in world coordinates
     *
     * Direct position tracking for reliable player movement.
     * Bypasses ECS to ensure stability during state transitions.
     */
    float m_playerY;

    /**
     * @brief Player X velocity in pixels per second
     *
     * Horizontal movement speed, modified by input and physics.
     * Used for smooth, frame-rate independent movement.
     */
    float m_playerVelX;

    /**
     * @brief Player Y velocity in pixels per second
     *
     * Vertical movement speed, affected by gravity and jumping.
     * Used for smooth, frame-rate independent movement.
     */
    float m_playerVelY;

    // Accumulated total score across levels in a run
    int m_totalRunScore{0};
    // Track whether boss has been defeated in this level
    bool m_bossDefeated{false};

public:
    // Accessors and helpers for run progression
    int GetScore() const { return m_score; }
    int GetTotalRunScore() const { return m_totalRunScore; }
    void ResetRunTotal() { m_totalRunScore = 0; }
    void AddToRunTotal(int amount) { m_totalRunScore += amount; }
    void LoadLevelAndReset(const std::string& levelName);

private:


    // ========== COLLISION SYSTEM ==========

    /**
     * @brief Cooldown timer to prevent collision spam
     *
     * Prevents multiple collision triggers in rapid succession.
     * Counts down from COLLISION_COOLDOWN_TIME after each collision.
     */
    float m_collisionCooldown;

    /**
     * @brief Duration of collision cooldown in seconds
     *
     * Time to wait after a collision before allowing another collision.
     * Prevents rapid-fire combat triggers and gives player time to react.
     */
    static constexpr float COLLISION_COOLDOWN_TIME = 1.0f;

    // ========== PRIVATE HELPER METHODS ==========

    /**
     * @brief Create and configure the player character
     *
     * Sets up the player entity with:
     * - Position and movement components
     * - Sprite and animation components
     * - Health and combat stats
     * - Collision detection
     */
    void CreatePlayer();

    /**
     * @brief Spawn enemies based on game time and difficulty
     *
     * Creates enemy entities with configuration-driven stats
     * and positions them appropriately in the game world.
     */
    void CreateEnemies();

    /**
     * @brief Update camera position to follow player
     *
     * Implements smooth camera following with optional boundaries
     * and side-scrolling behavior.
     */
    void UpdateCamera();

    /**
     * @brief Draw heads-up display elements
     *
     * Renders UI elements like score, health bar, game time,
     * and other important player information.
     */
    void DrawHUD();

    /**
     * @brief Check for game over conditions
     *
     * Monitors player health, time limits, or other failure
     * conditions and transitions to game over state if needed.
     */
    void CheckGameOver();

    /**
     * @brief Render scrolling background layers
     *
     * Draws parallax scrolling backgrounds that move at different
     * speeds to create depth and immersion.
     */
    void DrawScrollingBackground();

    /**
     * @brief Update score based on gameplay events
     *
     * Calculates and awards points for various player actions
     * and achievements.
     */
    void UpdateScore();

    /**
     * @brief Update player sprite animation
     *
     * Handles player animation state changes based on movement,
     * actions, and current state.
     */
    void UpdatePlayerAnimation();

    /**
     * @brief Reset all game state for new game
     *
     * Resets score, time, positions, and other variables
     * to their initial values for starting a fresh game.
     */
    void ResetGameState();
public:

    /**
     * @brief Adjust player position and cooldown on return from combat
     *
     * Moves the player slightly away from the last enemy to avoid
     * immediate re-trigger of combat and applies an extended cooldown.
     */

public:
    /**
     * @brief Handle end of a combat encounter
     * @param playerWon true if player won
     * @param wasBossEncounter true if the encounter was flagged as a boss fight
     */
    void OnCombatEnded(bool playerWon, bool wasBossEncounter);

private:

    void HandlePostCombatReturn();

private:

    /**
     * @brief Create character with configuration-based stats
     *
     * @param characterType Type of character to create (from config)
     * @param x World X position to spawn at
     * @param y World Y position to spawn at
     * @param difficultyMultiplier Scaling factor for stats (default: 1.0)
     *
     * Creates characters using the configuration system for
     * consistent stats and balanced gameplay.
     */
    void CreateConfigAwareCharacter(const std::string& characterType, float x, float y, float difficultyMultiplier = 1.0f);

    // ========== COMBAT INTEGRATION METHODS ==========

    /**
     * @brief Handle collision events between entities
     *
     * @param info Collision information (entities involved, contact points, etc.)
     *
     * Processes collision events and determines appropriate responses:
     * - Player vs Enemy: Trigger combat encounter
     * - Player vs Pickup: Collect item and award points
     * - Player vs Obstacle: Apply damage or movement restriction
     */
    void OnCollision(const CollisionInfo& info);

    /**
     * @brief Initiate turn-based combat encounter
     *
     * @param player Player entity involved in combat
     * @param enemy Enemy entity involved in combat
     *
     * Transitions to combat state while preserving the current
     * playing state for return after combat resolution.
     */
    void TriggerCombat(Entity player, Entity enemy);
};
