/**
 * @file MenuState.h
 * @brief Main menu state with navigation and game entry point
 * @author Ryan Butler
 * @date 2025
 *
 * This file implements the main menu system that serves as the entry point
 * for the arcade game. Players can navigate between options and access
 * different game modes and settings.
 *
 * Features:
 * - Keyboard navigation (Up/Down arrows, WASD)
 * - Visual selection highlighting with blinking effect
 * - Multiple menu options (Start Game, Options, Quit)
 * - Smooth transitions to other game states
 * - Attractive visual presentation with title and background
 */

#pragma once

#include "GameState.h"
#include <vector>
#include <string>

/**
 * @enum MenuOption
 * @brief Available options in the main menu
 *
 * Defines all selectable options that players can choose from
 * in the main menu interface.
 */
enum class MenuOption {
    START_GAME,   ///< Begin new game (transition to PLAYING state)
    HIGH_SCORES,  ///< View high scores (only shown if any exist)
    OPTIONS,      ///< Open settings menu (transition to OPTIONS state)
    CREDITS,      ///< View game credits (transition to CREDITS state)
    QUIT          ///< Exit the application
};

/**
 * @class MenuState
 * @brief Main menu game state with keyboard navigation
 *
 * This state implements the main menu interface where players:
 * - Navigate between menu options using keyboard
 * - Select options to transition to other game states
 * - See visual feedback for current selection
 * - Experience smooth menu animations and effects
 *
 * Menu Navigation:
 * - Up/Down Arrow Keys or W/S: Navigate between options
 * - Enter or Space: Select current option
 * - Escape: Quit application (from main menu)
 *
 * Visual Features:
 * - Game title display
 * - Highlighted current selection
 * - Blinking selection indicator
 * - Background graphics/effects
 *
 * State Transitions:
 * - START_GAME → PlayingState (begin arcade gameplay)
 * - OPTIONS → OptionsState (game settings)
 * - QUIT → Application exit
 *
 * @example
 * ```cpp
 * // In GameStateManager setup:
 * manager.AddState(GameStateType::MENU, std::make_unique<MenuState>());
 * manager.PushState(GameStateType::MENU); // Start with main menu
 * ```
 */
class MenuState : public GameState {
public:
    /**
     * @brief Constructor - initializes menu state
     *
     * Sets up the menu state with default values and prepares
     * menu options for display and navigation.
     */
    MenuState();

    /**
     * @brief Initialize menu when state becomes active
     *
     * Called when transitioning to menu state. Sets up:
     * - Menu option list and selection
     * - Visual effects and timers
     * - Input state reset
     */
    void OnEnter() override;

    /**
     * @brief Clean up menu when leaving state
     *
     * Called when transitioning away from menu state.
     * Handles any necessary cleanup.
     */
    void OnExit() override;

    /**
     * @brief Update menu logic and animations
     *
     * @param deltaTime Time elapsed since last frame in seconds
     *
     * Updates:
     * - Selection blinking animation
     * - Menu transition effects
     * - Background animations (if any)
     */
    void Update(float deltaTime) override;

    /**
     * @brief Render menu visuals
     *
     * Draws:
     * - Background graphics
     * - Game title
     * - Menu options with highlighting
     * - Selection indicators
     */
    void Render() override;

    /**
     * @brief Handle menu navigation input
     *
     * Processes:
     * - Up/Down navigation (Arrow keys, WASD)
     * - Option selection (Enter, Space)
     * - Quick quit (Escape from main menu)
     */
    void HandleInput() override;

private:
    // ========== MENU DATA ==========

    /**
     * @brief List of menu option text strings
     *
     * Contains the display text for each menu option.
     * Index corresponds to MenuOption enum values.
     */
    std::vector<std::string> m_menuOptions;

    /**
     * @brief Currently selected menu option index
     *
     * Index into m_menuOptions array indicating which
     * option is currently highlighted/selected.
     */
    int m_selectedOption;

    /**
     * @brief Timer for selection blinking effect
     *
     * Accumulates time to create a blinking animation
     * for the currently selected menu option.
     */
    float m_blinkTimer;

    /**
     * @brief Whether selection indicator is currently visible
     *
     * Toggles on/off based on blink timer to create
     * a pulsing selection highlight effect.
     */
    bool m_showSelection;

    // ========== PRIVATE HELPER METHODS ==========

    /**
     * @brief Move selection up to previous option
     *
     * Moves selection up with wraparound (top option wraps to bottom).
     * Includes visual/audio feedback for navigation.
     */
    void NavigateUp();

    /**
     * @brief Move selection down to next option
     *
     * Moves selection down with wraparound (bottom option wraps to top).
     * Includes visual/audio feedback for navigation.
     */
    void NavigateDown();

    /**
     * @brief Execute the currently selected menu option
     *
     * Performs the action associated with the current selection:
     * - START_GAME: Transition to playing state
     * - OPTIONS: Transition to options state
     * - QUIT: Exit application
     */
    void SelectOption();

    /**
     * @brief Draw the game title/logo
     *
     * Renders the main game title with styling and effects.
     */
    void DrawTitle();

    /**
     * @brief Draw menu options with selection highlighting
     *
     * Renders all menu options with appropriate highlighting
     * for the currently selected option.
     */
    void DrawMenu();

    /**
     * @brief Draw background graphics and effects
     *
     * Renders background visuals, animations, or patterns
     * to make the menu visually appealing.
     */
    void DrawBackground();
};
