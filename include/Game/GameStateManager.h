/**
 * @file GameStateManager.h
 * @brief State management system for handling game state transitions
 * @author Ryan Butler
 * @date 2025
 *
 * This file implements a robust state management system that handles transitions
 * between different game modes (menu, playing, combat, etc.) using a stack-based
 * approach with deferred state changes for safety.
 *
 * Key Features:
 * - Stack-based state management (allows state layering/pausing)
 * - Safe state transitions (deferred until safe to execute)
 * - Automatic state lifecycle management (OnEnter/OnExit calls)
 * - Engine system integration (provides states access to renderer, input, etc.)
 * - Memory management via smart pointers
 *
 * Architecture:
 * - States are stored in a map for fast lookup by type
 * - Active states are managed via a stack (LIFO - Last In, First Out)
 * - State changes are deferred to avoid mid-frame transitions
 * - Each state gets automatic access to engine systems
 *
 * Common Usage Patterns:
 * - ChangeState(): Replace current state (menu -> playing)
 * - PushState(): Layer state on top (playing -> paused)
 * - PopState(): Return to previous state (paused -> playing)
 */

#pragma once

#include "GameState.h"
#include <memory>        // For std::unique_ptr smart pointers
#include <unordered_map> // For fast state lookup by type
#include <stack>         // For state stack management (LIFO)

// Forward declaration to avoid circular dependency
class Engine;

/**
 * @class GameStateManager
 * @brief Manages game state transitions and lifecycle
 *
 * This class is the central coordinator for all game state management.
 * It handles transitions between different game modes safely and efficiently
 * using a stack-based approach with deferred state changes.
 *
 * Key Responsibilities:
 * - Store and manage all available game states
 * - Handle safe transitions between states
 * - Maintain a stack of active states (for layering/pausing)
 * - Provide states with access to engine systems
 * - Manage state lifecycle (OnEnter/OnExit calls)
 * - Defer state changes to avoid mid-frame issues
 *
 * Design Patterns Used:
 * - State Pattern: Encapsulates different game behaviors in state objects
 * - Stack Pattern: Allows state layering (pause menu over gameplay)
 * - Deferred Execution: State changes happen at safe points in the frame
 *
 * For New Developers:
 * - Create states and add them with AddState()
 * - Use ChangeState() to switch between different game modes
 * - Use PushState()/PopState() for temporary overlays (pause menus)
 * - The manager handles all the complex transition logic automatically
 *
 * @example
 * ```cpp
 * // Setup
 * GameStateManager manager(engine);
 * manager.AddState(GameStateType::MENU, std::make_unique<MenuState>());
 * manager.AddState(GameStateType::PLAYING, std::make_unique<PlayingState>());
 *
 * // Usage
 * manager.ChangeState(GameStateType::MENU);  // Go to menu
 * manager.ChangeState(GameStateType::PLAYING); // Start game
 * manager.PushState(GameStateType::PAUSED);  // Pause (layered)
 * manager.PopState();  // Resume (back to playing)
 * ```
 */
class GameStateManager {
public:
    /**
     * @brief Constructor - initializes the state management system
     *
     * Creates a new GameStateManager instance and connects it to the
     * main engine. The engine pointer is used to provide states with
     * access to core systems (renderer, input, audio, etc.).
     *
     * @param engine Pointer to the main Engine instance
     *
     * @note The engine pointer is stored and used throughout the manager's lifetime
     * @note States added later will automatically receive this engine pointer
     * @note The manager starts with no states - you must add them with AddState()
     */
    GameStateManager(Engine* engine);

    /**
     * @brief Destructor - cleans up all states and resources
     *
     * Automatically handles cleanup of all managed states:
     * - Calls OnExit() on the current active state
     * - Clears the state stack
     * - Releases all stored state objects
     *
     * @note Cleanup is automatic thanks to smart pointers (RAII)
     * @note States are destroyed in reverse order of creation
     */
    ~GameStateManager();

    // ========== STATE MANAGEMENT METHODS ==========
    // These methods handle adding states and managing transitions

    /**
     * @brief Add a new state to the manager's registry
     *
     * Registers a new game state with the manager so it can be used
     * in transitions. The state is stored but not activated until
     * you call PushState() or ChangeState().
     *
     * @param type The GameStateType identifier for this state
     * @param state Unique pointer to the state object (transfers ownership)
     *
     * @note The manager takes ownership of the state object
     * @note States are automatically configured with engine and manager pointers
     * @note You can only have one state per GameStateType
     * @note Adding a state with an existing type replaces the old one
     *
     * @example
     * ```cpp
     * manager.AddState(GameStateType::MENU, std::make_unique<MenuState>());
     * manager.AddState(GameStateType::PLAYING, std::make_unique<PlayingState>());
     * ```
     */
    void AddState(GameStateType type, std::unique_ptr<GameState> state);

    /**
     * @brief Push a state onto the stack (layered state)
     *
     * Adds a new state on top of the current state without removing
     * the current one. This is perfect for pause menus, dialog boxes,
     * or any overlay that should return to the previous state.
     *
     * @param type The GameStateType to push onto the stack
     *
     * @note The current state remains in memory but stops receiving updates
     * @note OnExit() is NOT called on the current state (it's just paused)
     * @note OnEnter() IS called on the new state
     * @note Use PopState() to return to the previous state
     * @note State changes are deferred until the end of the frame
     *
     * @example
     * ```cpp
     * // Player is in PLAYING state, presses ESC
     * manager.PushState(GameStateType::PAUSED);
     * // Now PAUSED is active, PLAYING is underneath
     * // Player can resume or go to menu from pause screen
     * ```
     */
    void PushState(GameStateType type);

    /**
     * @brief Pop the current state from the stack (return to previous)
     *
     * Removes the current state and returns to the previous state
     * on the stack. Perfect for closing pause menus, dialog boxes,
     * or any temporary overlay.
     *
     * @note OnExit() IS called on the current state
     * @note OnEnter() is NOT called on the previous state (it was never exited)
     * @note If there's only one state, PopState() does nothing (safety)
     * @note State changes are deferred until the end of the frame
     *
     * @example
     * ```cpp
     * // Player is in PAUSED state (with PLAYING underneath)
     * manager.PopState();
     * // Now back to PLAYING state, PAUSED is destroyed
     * ```
     */
    void PopState();

    /**
     * @brief Change to a different state (replace current)
     *
     * Replaces the current state with a new one. This is the most
     * common transition type - used when switching between major
     * game modes like menu to playing, playing to game over, etc.
     *
     * @param type The GameStateType to change to
     *
     * @note OnExit() IS called on the current state
     * @note OnEnter() IS called on the new state
     * @note The entire state stack is cleared and replaced with the new state
     * @note State changes are deferred until the end of the frame
     *
     * @example
     * ```cpp
     * // Transition from menu to gameplay
     * manager.ChangeState(GameStateType::PLAYING);
     *
     * // Game over - go to game over screen
     * manager.ChangeState(GameStateType::GAME_OVER);
     * ```
     */
    void ChangeState(GameStateType type);

    /**
     * @brief Clear all states from the stack
     *
     * Removes all states from the stack, effectively stopping all
     * game state processing. This is typically used during shutdown
     * or when resetting the entire game state system.
     *
     * @note OnExit() IS called on the current active state
     * @note All states are removed from the stack
     * @note The state registry (added states) remains intact
     * @note After this, HasStates() will return false
     *
     * @example
     * ```cpp
     * // During game shutdown or reset
     * manager.ClearStates();
     * ```
     */
    void ClearStates();

    // ========== CURRENT STATE ACCESS METHODS ==========
    // These methods provide information about the current active state

    /**
     * @brief Get pointer to the currently active state
     *
     * Returns a pointer to the state that's currently at the top of
     * the state stack (the active state receiving updates and input).
     *
     * @return Pointer to current GameState, or nullptr if no states are active
     *
     * @note Returns nullptr if the state stack is empty
     * @note The returned pointer is valid until the next state transition
     * @note Use this for state-specific operations or debugging
     *
     * @example
     * ```cpp
     * GameState* current = manager.GetCurrentState();
     * if (current && current->GetType() == GameStateType::PLAYING) {
     *     // Do something specific to the playing state
     * }
     * ```
     */
    GameState* GetCurrentState() const;

    /**
     * @brief Get a pointer to a specific registered state by type
     *
     * Returns a raw pointer to the stored state instance for the given
     * GameStateType if it has been registered via AddState(). Returns
     * nullptr if no such state exists.
     */
    GameState* GetState(GameStateType type) const;

    /**
     * @brief Check if there are any active states
     *
     * Returns true if there's at least one state on the stack,
     * false if the stack is empty.
     *
     * @return true if states are active, false if stack is empty
     *
     * @note Useful for checking if the game is in a valid state
     * @note If this returns false, Update/Render/HandleInput do nothing
     *
     * @example
     * ```cpp
     * if (!manager.HasStates()) {
     *     std::cout << "No active states - game may be shutting down" << std::endl;
     * }
     * ```
     */
    bool HasStates() const { return !m_stateStack.empty(); }

    // ========== MAIN LOOP METHODS ==========
    // These methods are called every frame by the main game loop

    /**
     * @brief Update the current active state
     *
     * Calls the Update() method on the currently active state (top of stack).
     * Also processes any pending state changes that were requested during
     * the previous frame.
     *
     * @param deltaTime Time elapsed since last frame in seconds
     *
     * @note Called every frame by the main game loop
     * @note Processes pending state changes BEFORE updating the current state
     * @note If no states are active, this method does nothing
     * @note State changes requested during Update() are deferred to next frame
     *
     * @example
     * ```cpp
     * // In main game loop
     * while (running) {
     *     float deltaTime = CalculateDeltaTime();
     *     manager.Update(deltaTime);  // Updates current state
     *     manager.Render();           // Renders current state
     * }
     * ```
     */
    void Update(float deltaTime);

    /**
     * @brief Render the current active state
     *
     * Calls the Render() method on the currently active state to draw
     * all visual elements for the current game mode.
     *
     * @note Called every frame by the main game loop after Update()
     * @note If no states are active, this method does nothing
     * @note The screen is automatically cleared before state rendering
     * @note Each state is responsible for drawing its own content
     *
     * @example
     * ```cpp
     * // In main game loop
     * while (running) {
     *     manager.Update(deltaTime);
     *     manager.Render();  // Draws current state's visuals
     *     SwapBuffers();     // Display the rendered frame
     * }
     * ```
     */
    void Render();

    /**
     * @brief Handle input for the current active state
     *
     * Calls the HandleInput() method on the currently active state
     * to process keyboard, mouse, and gamepad input.
     *
     * @note Called every frame by the main game loop before Update()
     * @note If no states are active, this method does nothing
     * @note Only the top state on the stack receives input (no input bleeding)
     * @note States can request state changes during input handling
     *
     * @example
     * ```cpp
     * // In main game loop
     * while (running) {
     *     manager.HandleInput();  // Process input for current state
     *     manager.Update(deltaTime);
     *     manager.Render();
     * }
     * ```
     */
    void HandleInput();

private:
    // ========== PRIVATE MEMBER VARIABLES ==========

    /**
     * @brief Pointer to the main engine instance
     *
     * Stores a reference to the core engine that provides access to
     * all major game systems (renderer, input, audio, window, etc.).
     * This pointer is passed to all states when they're added.
     */
    Engine* m_engine;

    /**
     * @brief Registry of all available game states
     *
     * Maps GameStateType enums to their corresponding state objects.
     * This allows fast lookup of states by type when transitioning.
     * States are stored as unique_ptr for automatic memory management.
     *
     * @note Key: GameStateType enum value
     * @note Value: unique_ptr to the actual state object
     * @note States remain here even when not active (for reuse)
     */
    std::unordered_map<GameStateType, std::unique_ptr<GameState>> m_states;

    /**
     * @brief Stack of currently active state types
     *
     * Maintains the hierarchy of active states using LIFO (Last In, First Out).
     * The top of the stack is the currently active state that receives
     * updates, rendering, and input. Lower states are paused but remain in memory.
     *
     * @note Top of stack = currently active state
     * @note Lower states are paused (no updates) but can be resumed
     * @note Empty stack means no active states
     */
    std::stack<GameStateType> m_stateStack;

    // ========== DEFERRED STATE CHANGE SYSTEM ==========
    // These variables handle safe state transitions

    /**
     * @brief Flag indicating a state change is pending
     *
     * Set to true when ChangeState() is called, indicating that a state
     * transition should occur at the next safe opportunity (start of next frame).
     * This prevents mid-frame state changes that could cause crashes.
     */
    bool m_pendingChange;

    /**
     * @brief The state type to change to (when m_pendingChange is true)
     *
     * Stores which state should become active when the pending change
     * is processed. Only valid when m_pendingChange is true.
     */
    GameStateType m_pendingState;

    /**
     * @brief Flag indicating a state pop is pending
     *
     * Set to true when PopState() is called, indicating that the current
     * state should be popped from the stack at the next safe opportunity.
     * This prevents mid-frame stack modifications.
     */
    bool m_pendingPop;

    // ========== PRIVATE HELPER METHODS ==========

    /**
     * @brief Process any pending state changes safely
     *
     * This method is called at the beginning of each Update() cycle to
     * handle any state transitions that were requested during the previous
     * frame. This ensures state changes happen at safe points in the frame.
     *
     * The method handles:
     * - Pending state changes (ChangeState calls)
     * - Pending state pops (PopState calls)
     * - Proper OnExit/OnEnter lifecycle calls
     * - Stack management and cleanup
     *
     * @note This is an internal method - not called directly by users
     * @note State changes are deferred to prevent mid-frame issues
     * @note Multiple pending changes are processed in the correct order
     */
    void ProcessPendingChanges();
};
