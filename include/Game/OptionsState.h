/**
 * @file OptionsState.h
 * @brief Simple options menu state for arcade games
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "GameState.h"
#include "Engine/KeybindingManager.h"
#include <vector>
#include <string>
#include <memory>

/**
 * @brief Comprehensive options menu for arcade games
 *
 * Options menu with essential settings:
 * - Audio volume controls
 * - Keybinding configuration
 * - Back to main menu
 *
 * Navigation:
 * - Arrow keys: Navigate between options
 * - Left/Right: Adjust values
 * - Enter: Select option or configure keybinding
 * - B/Escape: Back to menu
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
    enum class OptionType {
        VOLUME_MUSIC,
        VOLUME_SOUND,
        KEYBINDINGS,
        BACK_TO_MENU
    };

    std::vector<std::string> m_options;                 ///< Available options
    std::vector<OptionType> m_optionTypes;              ///< Option type mapping
    int m_selectedOption;                               ///< Selected option index

    float m_blinkTimer;                                 ///< Selection blink timer
    bool m_showSelection;                               ///< Whether to show selection highlight

    float m_musicVolume;                                ///< Music volume (0.0 - 1.0)
    float m_soundVolume;                                ///< Sound effects volume (0.0 - 1.0)

    // Keybinding management
    std::unique_ptr<KeybindingManager> m_keybindingManager;
    bool m_inKeybindingMode;                            ///< Whether we're in keybinding configuration
    std::vector<GameAction> m_configurableActions;     ///< Actions that can be configured
    int m_selectedKeybinding;                           ///< Selected keybinding index
    bool m_waitingForKey;                               ///< Whether we're waiting for key input
    GameAction m_keyToRebind;                           ///< Action being rebound
    bool m_rebindingPrimary;                            ///< Whether rebinding primary (true) or alt (false) key

    // Scrolling and layout
    int m_keybindingScrollOffset;                       ///< Scroll offset for keybinding list
    int m_maxVisibleKeybindings;                        ///< Maximum keybindings visible on screen
    
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

    // Keybinding methods
    void EnterKeybindingMode();
    void ExitKeybindingMode();
    void HandleKeybindingInput();
    void HandleKeybindingNavigation();
    void StartKeyRebinding(GameAction action, bool primary);
    void CancelKeyRebinding();
    void ApplyKeyRebinding(SDL_Scancode key);

    // Rendering methods
    void DrawBackground();
    void DrawTitle();
    void DrawOptions();
    void DrawKeybindings();
    void DrawInstructions();

    // Helper methods
    void InitializeOptions();
    void SaveKeybindings();
    std::string GetVolumeDisplayText(float volume) const;
    std::string GetKeybindingDisplayText(const KeyBinding& binding) const;

    // Layout helpers
    void CalculateKeybindingLayout();
    void UpdateKeybindingScroll();
    bool IsKeybindingVisible(int index) const;
};
