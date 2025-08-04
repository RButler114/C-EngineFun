/**
 * @file OptionsState.h
 * @brief Simple options menu state for arcade games
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "GameState.h"
#include <vector>
#include <string>

/**
 * @brief Simple options menu for arcade games
 *
 * Basic options menu with essential settings:
 * - Audio volume controls
 * - Back to main menu
 *
 * Navigation:
 * - Arrow keys: Navigate between options
 * - Left/Right: Adjust values
 * - Enter: Select option
 * - Escape: Back to menu
 */
class OptionsState : public GameState {
public:
    OptionsState();
    ~OptionsState();

    void OnEnter() override;
    void OnExit() override;
    void Update(float deltaTime) override;
    void Render() override;
    void HandleInput() override;

private:
    std::vector<std::string> m_options;                 ///< Available options
    int m_selectedOption;                               ///< Selected option index

    float m_blinkTimer;                                 ///< Selection blink timer
    bool m_showSelection;                               ///< Whether to show selection highlight

    float m_musicVolume;                                ///< Music volume (0.0 - 1.0)
    float m_soundVolume;                                ///< Sound effects volume (0.0 - 1.0)
    
    /**
     * @brief Navigate up in options
     */
    void NavigateUp();

    /**
     * @brief Navigate down in options
     */
    void NavigateDown();

    /**
     * @brief Adjust current option value left
     */
    void AdjustLeft();

    /**
     * @brief Adjust current option value right
     */
    void AdjustRight();

    /**
     * @brief Select current option
     */
    void SelectOption();

    /**
     * @brief Go back to main menu
     */
    void GoBack();

    // Rendering methods
    void DrawBackground();
    void DrawTitle();
    void DrawOptions();
    void DrawInstructions();
};
