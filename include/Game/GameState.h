/**
 * @file GameState.h
 * @brief Base class and types for the game state management system
 * @author Ryan Butler
 * @date 2025
 *
 * This file defines the foundation of the game's state management system.
 * Game states represent different "screens" or modes of the game (menu, playing, combat, etc.).
 *
 * Key Concepts:
 * - State Pattern: Each game mode is encapsulated in its own state class
 * - Polymorphism: All states inherit from GameState and implement common interface
 * - Lifecycle Management: States have clear entry/exit points for setup/cleanup
 * - Engine Integration: States have access to core engine systems (renderer, input, etc.)
 */

#pragma once

#include <string>
#include <functional>

// Forward declarations to avoid circular dependencies
// These classes are defined elsewhere but used by GameState
class Engine;        // Main engine class that manages all systems
class Renderer;      // Handles all drawing/graphics operations
class InputManager;  // Processes keyboard, mouse, and gamepad input
class GameStateManager; // Manages transitions between different game states

/**
 * @enum GameStateType
 * @brief Enumeration of all possible game states
 *
 * This enum defines every possible "screen" or mode the game can be in.
 * Each state represents a distinct phase of gameplay with its own logic,
 * rendering, and input handling.
 *
 * @note When adding new states, also update GameStateManager to handle them
 * @note States can transition to any other state via the GameStateManager
 */
enum class GameStateType {
    MENU,           ///< Main menu - start game, options, quit
    CUSTOMIZATION,  ///< Character/settings customization screen
    PLAYING,        ///< Active arcade gameplay (side-scrolling action)
    COMBAT,         ///< Turn-based combat encounters (triggered by collisions)
    PAUSED,         ///< Game paused overlay (can resume or return to menu)
    GAME_OVER,      ///< End game screen with score and restart options
    OPTIONS         ///< Settings menu for audio, controls, graphics
};

/**
 * @class GameState
 * @brief Abstract base class for all game states
 *
 * This is the foundation class that all game states inherit from. It provides:
 * - Common interface for state lifecycle (enter, exit, update, render)
 * - Access to core engine systems (renderer, input manager, etc.)
 * - State identification and management
 *
 * Design Pattern: This implements the State Pattern, where each game mode
 * (menu, playing, combat) is encapsulated in its own class that inherits
 * from this base class.
 *
 * For New Developers:
 * - Inherit from this class to create new game states
 * - Implement the pure virtual methods (Update, Render)
 * - Override lifecycle methods (OnEnter, OnExit) as needed
 * - Use GetRenderer(), GetInputManager() to access engine systems
 *
 * @example
 * ```cpp
 * class MyGameState : public GameState {
 * public:
 *     MyGameState() : GameState(GameStateType::PLAYING, "MyState") {}
 *
 *     void Update(float deltaTime) override {
 *         // Update game logic here
 *     }
 *
 *     void Render() override {
 *         // Draw everything here
 *     }
 * };
 * ```
 */
class GameState {
public:
    /**
     * @brief Constructor for game state
     *
     * Initializes a new game state with the specified type and name.
     * The engine and state manager pointers are set to null initially
     * and will be assigned by the GameStateManager when the state is added.
     *
     * @param type The type of state (from GameStateType enum)
     * @param name Human-readable name for debugging/logging
     *
     * @note Engine and StateManager pointers are set later by GameStateManager
     * @note The name is useful for debugging and logging state transitions
     */
    GameState(GameStateType type, const std::string& name)
        : m_type(type), m_name(name), m_engine(nullptr), m_stateManager(nullptr) {}

    /**
     * @brief Virtual destructor for proper cleanup
     *
     * Ensures that derived classes are properly destroyed when a GameState
     * pointer is deleted. This is essential for polymorphic inheritance.
     *
     * @note Using = default means the compiler generates the destructor
     * @note Virtual destructor is required for polymorphic base classes
     */
    virtual ~GameState() = default;

    // ========== STATE LIFECYCLE METHODS ==========
    // These methods are called automatically by GameStateManager
    // during state transitions and the main game loop

    /**
     * @brief Called when this state becomes active
     *
     * This is where you should:
     * - Initialize state-specific resources
     * - Set up UI elements
     * - Reset game variables
     * - Load assets if needed
     * - Set initial camera position
     *
     * @note Called automatically by GameStateManager
     * @note Default implementation does nothing (safe to not override)
     * @note Called BEFORE the first Update() call
     *
     * @example
     * ```cpp
     * void OnEnter() override {
     *     playerHealth = 100;
     *     score = 0;
     *     LoadLevelAssets();
     * }
     * ```
     */
    virtual void OnEnter() {}

    /**
     * @brief Called when this state is being deactivated
     *
     * This is where you should:
     * - Save game progress
     * - Clean up temporary resources
     * - Stop audio/music
     * - Reset input states
     * - Prepare for the next state
     *
     * @note Called automatically by GameStateManager
     * @note Default implementation does nothing (safe to not override)
     * @note Called AFTER the last Update() call
     *
     * @example
     * ```cpp
     * void OnExit() override {
     *     SaveGameProgress();
     *     StopBackgroundMusic();
     *     CleanupTempObjects();
     * }
     * ```
     */
    virtual void OnExit() {}

    /**
     * @brief Update game logic for this state (PURE VIRTUAL - MUST IMPLEMENT)
     *
     * This is the heart of your game state - called every frame to update:
     * - Game object positions and animations
     * - AI behavior and pathfinding
     * - Physics simulations
     * - Timer countdowns
     * - State transition conditions
     *
     * @param deltaTime Time elapsed since last frame in seconds
     *                  - Use for frame-rate independent movement
     *                  - Typical values: 0.016s (60fps) to 0.033s (30fps)
     *
     * @note MUST be implemented by derived classes (pure virtual)
     * @note Called every frame by GameStateManager
     * @note Use deltaTime for smooth, frame-rate independent updates
     *
     * @example
     * ```cpp
     * void Update(float deltaTime) override {
     *     // Move player based on input
     *     if (GetInputManager()->IsKeyPressed(KEY_RIGHT)) {
     *         playerX += playerSpeed * deltaTime;
     *     }
     *
     *     // Update animations
     *     playerSprite.Update(deltaTime);
     *
     *     // Check win condition
     *     if (score >= winScore) {
     *         GetStateManager()->ChangeState(GameStateType::GAME_OVER);
     *     }
     * }
     * ```
     */
    virtual void Update(float deltaTime) = 0;

    /**
     * @brief Render all visual elements for this state (PURE VIRTUAL - MUST IMPLEMENT)
     *
     * This is where you draw everything visible on screen:
     * - Background images/textures
     * - Game sprites and animations
     * - UI elements (buttons, text, HUD)
     * - Debug information (if enabled)
     *
     * @note MUST be implemented by derived classes (pure virtual)
     * @note Called every frame after Update()
     * @note The screen is automatically cleared before this is called
     * @note Use GetRenderer() to access drawing functions
     *
     * Typical rendering order:
     * 1. Background elements (furthest back)
     * 2. Game objects (sprites, enemies, player)
     * 3. UI elements (HUD, menus)
     * 4. Debug overlays (closest to camera)
     *
     * @example
     * ```cpp
     * void Render() override {
     *     Renderer* renderer = GetRenderer();
     *
     *     // Draw background
     *     renderer->DrawTexture(backgroundTexture, 0, 0);
     *
     *     // Draw player
     *     renderer->DrawSprite(playerSprite, playerX, playerY);
     *
     *     // Draw UI
     *     renderer->DrawText("Score: " + std::to_string(score), 10, 10);
     * }
     * ```
     */
    virtual void Render() = 0;

    /**
     * @brief Handle input events for this state
     *
     * Process keyboard, mouse, and gamepad input specific to this state:
     * - Menu navigation (arrow keys, enter, escape)
     * - Player movement (WASD, arrow keys)
     * - Action buttons (space, mouse clicks)
     * - State transitions (escape to menu, enter to start)
     *
     * @note Default implementation does nothing (safe to not override)
     * @note Called every frame before Update()
     * @note Use GetInputManager() to check input states
     * @note Consider both pressed (single frame) and held (continuous) input
     *
     * @example
     * ```cpp
     * void HandleInput() override {
     *     InputManager* input = GetInputManager();
     *
     *     // Single press events
     *     if (input->IsKeyPressed(KEY_ESCAPE)) {
     *         GetStateManager()->ChangeState(GameStateType::MENU);
     *     }
     *
     *     // Continuous input (held keys)
     *     if (input->IsKeyHeld(KEY_SPACE)) {
     *         playerJumping = true;
     *     }
     * }
     * ```
     */
    virtual void HandleInput() {}

    // ========== GETTER METHODS ==========
    // These methods provide access to state information

    /**
     * @brief Get the type of this game state
     *
     * Returns the GameStateType enum value that identifies what kind
     * of state this is (MENU, PLAYING, COMBAT, etc.).
     *
     * @return The GameStateType enum value for this state
     *
     * @note Useful for debugging and state-specific logic
     * @note The type is set in the constructor and never changes
     */
    GameStateType GetType() const { return m_type; }

    /**
     * @brief Get the human-readable name of this state
     *
     * Returns the descriptive name assigned to this state, useful
     * for debugging, logging, and development tools.
     *
     * @return Reference to the state's name string
     *
     * @note The name is set in the constructor and never changes
     * @note Useful for debug output: "Entering state: " + GetName()
     */
    const std::string& GetName() const { return m_name; }

    /**
     * @brief Get pointer to the main engine instance
     *
     * Provides access to the core engine object that manages all
     * major game systems (window, renderer, input, audio, etc.).
     *
     * @return Pointer to the Engine instance, or nullptr if not set
     *
     * @note Set automatically by GameStateManager when state is added
     * @note Use this to access engine-level functionality
     * @note Prefer GetRenderer() and GetInputManager() for common operations
     */
    Engine* GetEngine() const { return m_engine; }

    // ========== INTERNAL METHODS (CALLED BY GAMESTATEMANAGER) ==========
    // These methods are used internally by the state management system

    /**
     * @brief Set the engine pointer (called by GameStateManager)
     *
     * This method is called internally by GameStateManager when a state
     * is added to the system. It provides the state with access to the
     * main engine instance.
     *
     * @param engine Pointer to the main Engine instance
     *
     * @note This is an internal method - don't call it directly
     * @note Called automatically when state is added to GameStateManager
     */
    void SetEngine(Engine* engine) { m_engine = engine; }

    /**
     * @brief Set the state manager pointer (called by GameStateManager)
     *
     * This method is called internally by GameStateManager to give the
     * state access to the state management system for transitions.
     *
     * @param manager Pointer to the GameStateManager instance
     *
     * @note This is an internal method - don't call it directly
     * @note Called automatically when state is added to GameStateManager
     */
    void SetStateManager(GameStateManager* manager) { m_stateManager = manager; }

protected:
    // ========== PROTECTED HELPER METHODS ==========
    // These methods provide convenient access to engine subsystems

    /**
     * @brief Get pointer to the renderer for drawing operations
     *
     * Convenient access to the rendering system for drawing sprites,
     * textures, text, and other visual elements.
     *
     * @return Pointer to the Renderer instance
     *
     * @note Implemented in GameState.cpp to avoid circular dependencies
     * @note Use this in your Render() method to draw everything
     * @note Example: GetRenderer()->DrawSprite(sprite, x, y);
     */
    Renderer* GetRenderer() const;

    /**
     * @brief Get pointer to the input manager for handling user input
     *
     * Convenient access to the input system for checking keyboard,
     * mouse, and gamepad states.
     *
     * @return Pointer to the InputManager instance
     *
     * @note Implemented in GameState.cpp to avoid circular dependencies
     * @note Use this in your HandleInput() method to check input states
     * @note Example: GetInputManager()->IsKeyPressed(KEY_SPACE);
     */
    InputManager* GetInputManager() const;

    /**
     * @brief Get pointer to the state manager for state transitions
     *
     * Access to the state management system for changing between
     * different game states (menu to playing, playing to combat, etc.).
     *
     * @return Pointer to the GameStateManager instance
     *
     * @note Use this to transition between states
     * @note Example: GetStateManager()->ChangeState(GameStateType::MENU);
     */
    GameStateManager* GetStateManager() const { return m_stateManager; }

private:
    // ========== PRIVATE MEMBER VARIABLES ==========

    /**
     * @brief The type identifier for this state
     *
     * Stores which type of game state this is (MENU, PLAYING, etc.).
     * Set in constructor and never changes.
     */
    GameStateType m_type;

    /**
     * @brief Human-readable name for this state
     *
     * Descriptive name used for debugging and logging.
     * Set in constructor and never changes.
     */
    std::string m_name;

    /**
     * @brief Pointer to the main engine instance
     *
     * Provides access to all core engine systems.
     * Set by GameStateManager when state is added.
     */
    Engine* m_engine;

    /**
     * @brief Pointer to the state manager
     *
     * Allows this state to request transitions to other states.
     * Set by GameStateManager when state is added.
     */
    GameStateManager* m_stateManager;
};
