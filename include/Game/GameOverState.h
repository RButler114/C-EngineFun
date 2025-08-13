/**
 * @file GameOverState.h
 * @brief Game over screen state with score display and restart options
 * @author Ryan Butler
 * @date 2025
 *
 * This file implements the game over screen that appears when the player
 * loses the game (health reaches zero, time runs out, etc.). It provides:
 * - Final score display
 * - Game statistics
 * - Options to restart or return to menu
 * - Visual feedback and animations
 */

#pragma once

#include "GameState.h"

/**
 * @class GameOverState
 * @brief Game over screen with score display and restart functionality
 *
 * This state is displayed when the game ends (player death, time limit, etc.).
 * It provides players with:
 * - Final score and statistics
 * - Options to restart the game or return to main menu
 * - Visual feedback about their performance
 * - Smooth transitions back to other game states
 *
 * Features:
 * - Animated score display
 * - Timed restart prompt appearance
 * - Keyboard navigation (Enter to restart, Escape to menu)
 * - Visual effects and transitions
 *
 * State Flow:
 * 1. Display "GAME OVER" message
 * 2. Show final score with animation
 * 3. After delay, show restart options
 * 4. Handle player input for next action
 *
 * Navigation:
 * - Enter/Space: Restart game (return to PLAYING state)
 * - Escape/B: Return to main menu (MENU state)
 * - Any key: Skip animations and show options immediately
 *
 * @example
 * ```cpp
 * // In PlayingState when player dies:
 * auto* gameOverState = static_cast<GameOverState*>(
 *     GetStateManager()->GetState(GameStateType::GAME_OVER)
 * );
 * gameOverState->SetScore(currentScore);
 * GetStateManager()->ChangeState(GameStateType::GAME_OVER);
 * ```
 */
class GameOverState : public GameState {
public:
    /**
     * @brief Constructor - initializes game over state
     *
     * Sets up the game over state with default values and prepares
     * for score display and user interaction.
     */
    GameOverState();

    /**
     * @brief Initialize game over screen when state becomes active
     *
     * Called when transitioning to game over state. Sets up:
     * - Display timers and animations
     * - Visual effects
     * - Audio feedback (game over sound)
     */
    void OnEnter() override;

    /**
     * @brief Clean up when leaving game over state
     *
     * Called when transitioning away from game over state.
     * Handles any necessary cleanup.
     */
    void OnExit() override;

    /**
     * @brief Update game over screen animations and timers
     *
     * @param deltaTime Time elapsed since last frame in seconds
     *
     * Updates:
     * - Display timer for showing restart prompt
     * - Animation effects
     * - Transition timing
     */
    void Update(float deltaTime) override;

    /**
     * @brief Render game over screen visuals
     *
     * Draws:
     * - "GAME OVER" title
     * - Final score display
     * - Restart prompt (after delay)
     * - Background effects
     */
    void Render() override;

    /**
     * @brief Handle input for game over screen
     *
     * Processes:
     * - Restart game (Enter, Space)
     * - Return to menu (Escape, B)
     * - Skip animations (any key)
     */
    void HandleInput() override;

    /**
     * @brief Set the final score to display
     *
     * Called by the previous game state (usually PlayingState) to
     * pass the player's final score for display on the game over screen.
     *
     * @param score Final score achieved by the player
     *
     * @note Should be called before transitioning to GameOverState
     * @note Score is displayed with animation effects
     *
     * @example
     * ```cpp
     * // In PlayingState when game ends:
     * gameOverState->SetScore(m_currentScore);
     * GetStateManager()->ChangeState(GameStateType::GAME_OVER);
     * ```
     */
    void SetScore(int score) { m_finalScore = score; }

	    // Outcome mode
	    enum class Outcome { LOSE, WIN };
	    void SetOutcome(Outcome o) { m_outcome = o; }
	    Outcome GetOutcome() const { return m_outcome; }
	    void SetNextLevel(const std::string& lvl) { m_nextLevel = lvl; }
	    const std::string& GetNextLevel() const { return m_nextLevel; }
	    void SetRunTotal(int total) { m_runTotal = total; }
	    int GetRunTotal() const { return m_runTotal; }


private:
    // ========== GAME OVER DATA ==========

    /**
     * @brief Final score achieved by the player
     *
     * Set by the previous game state and displayed on the game over screen.
     * Used for score animations and statistics.
     */
    int m_finalScore;

    /**
     * @brief Timer for controlling display sequence
     *
     * Controls when different elements appear:
     * - 0-1s: Show "GAME OVER" message
     * - 1-2s: Animate score display
     * - 2s+: Show restart prompt
     */
    float m_displayTimer;

    /**
     * @brief Whether to show the restart prompt
     *
     * Set to true after the display timer reaches the threshold.
     * Controls visibility of restart/menu options.
     */
    bool m_showRestartPrompt;

    // High score initials input
    bool m_collectingInitials{false};
    std::string m_initials; // up to 3 chars
    float m_caretTimer{0.0f};

	    // Victory/defeat and progression fields
	    Outcome m_outcome{Outcome::LOSE};
	    std::string m_nextLevel;
	    int m_runTotal{0};

    bool m_caretOn{true};

    // Optional: guard against ultra-fast repeat on first press
    float m_initialsRepeatCooldown{0.0f}; // seconds remaining before accepting next char
    float m_backspaceCooldown{0.0f};      // seconds remaining before accepting next backspace

    // Visual feedback when a character is accepted or removed
    float m_initialsFeedbackTimer{0.0f};  // countdown
    float m_feedbackDuration{0.12f};      // seconds (can be overridden by config)

    // Tunables (can be overridden by config)
    float m_repeatGuard{0.08f};    // seconds between accepted chars
    float m_backspaceGuard{0.11f}; // seconds between accepted backspaces

    void DrawInitialsPrompt();
    void HandleInitialsInput();


    // ========== PRIVATE RENDERING METHODS ==========

    /**
     * @brief Draw the main "GAME OVER" message
     *
     * Renders the game over title with appropriate styling
     * and visual effects.
     */
    void DrawGameOverScreen();

    /**
     * @brief Draw the final score with animation
     *
     * Displays the player's final score with counting animation
     * or other visual effects.
     */
    void DrawScore();

    /**
     * @brief Draw restart and menu options
     *
     * Shows the available options for what the player can do next:
     * - Restart game
     * - Return to main menu
     */
    void DrawRestartPrompt();
};
