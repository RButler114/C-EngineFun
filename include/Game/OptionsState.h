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
        HEADER_VIDEO,
        RESOLUTION,
        FULLSCREEN,
        VSYNC,
        FPS_LIMIT,
        APPLY_CHANGES,
        CANCEL_CHANGES,
        CONFIRM_CHANGES,
        STANDARDIZE,
        HEADER_AUDIO,
        VOLUME_MUSIC,
        VOLUME_SOUND,
        HEADER_CONTROLS,
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

    // Video settings
    struct Resolution { int w; int h; };
    std::vector<Resolution> m_resolutions;              ///< Supported window resolutions
    int m_resolutionIndex = 0;                          ///< Current index into m_resolutions
    bool m_fullscreen = false;                          ///< Fullscreen enabled
    bool m_vsync = true;                                ///< VSync enabled
    std::vector<int> m_fpsOptions;                      ///< FPS presets (0=Unlimited)
    int m_fpsIndex = 1;                                 ///< Index into m_fpsOptions

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

    // Scrolling for main options list
    int m_optionsScrollOffset = 0;                       ///< Scroll offset for options list
    int m_maxVisibleOptions = 0;                         ///< Maximum options visible on screen

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
    void InitializeVideoOptions();
    void SaveVideoSettings();
    void SaveKeybindings();
    int GetOptionCount() const;
    std::string GetVolumeDisplayText(float volume) const;
    std::string GetKeybindingDisplayText(const KeyBinding& binding) const;

    // Apply/Cancel model for video settings
    bool m_videoPendingChanges = false;     // user changed values not yet applied OR in confirm window
    bool m_videoAwaitingConfirm = false;    // true after Apply, until confirmed or reverted
    float m_videoRevertTimer = 0.0f;        // seconds left for auto-revert when awaiting confirm

    // Previous applied snapshot for auto-revert
    bool m_hasPrevSnapshot = false;
    int m_prevWidth = 800;
    int m_prevHeight = 600;
    bool m_prevFullscreen = false;
    bool m_prevVsync = true;
    int m_prevFps = 60; // 0 = unlimited

    void ApplyVideoChanges();
    void RevertVideoChanges();
    void StandardizeVideoSettings();


    // Layout helpers
    void CalculateKeybindingLayout();
    void UpdateKeybindingScroll();
    bool IsKeybindingVisible(int index) const;
};
