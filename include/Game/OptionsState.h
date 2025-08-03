/**
 * @file OptionsState.h
 * @brief Options menu state for game configuration
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "GameState.h"
#include "Engine/ConfigManager.h"
#include <vector>
#include <string>
#include <memory>

// Forward declarations
struct Color;

/**
 * @brief Options menu navigation modes
 */
enum class OptionsMode {
    CATEGORY_SELECTION,  ///< Selecting which category to configure
    SETTING_SELECTION,   ///< Selecting which setting to modify
    VALUE_EDITING       ///< Editing a specific setting value
};

/**
 * @brief UI element types for options display
 */
enum class OptionUIType {
    BOOLEAN,        ///< Toggle button (On/Off)
    SLIDER,         ///< Slider for numeric values
    DROPDOWN,       ///< Dropdown for enum values
    TEXT_INPUT,     ///< Text input field
    BUTTON          ///< Action button
};

/**
 * @brief Options menu UI element
 */
struct OptionUIElement {
    std::string key;                ///< Configuration key
    std::string displayName;        ///< Display name
    std::string description;        ///< Description text
    OptionUIType type;              ///< UI element type
    float currentValue;             ///< Current numeric value (for sliders)
    int currentIndex;               ///< Current selection index (for dropdowns)
    std::vector<std::string> options; ///< Available options (for dropdowns)
    bool requiresRestart;           ///< Whether changing requires restart
    
    OptionUIElement(const std::string& k, const std::string& name, const std::string& desc, 
                   OptionUIType t, bool restart = false)
        : key(k), displayName(name), description(desc), type(t), 
          currentValue(0.0f), currentIndex(0), requiresRestart(restart) {}
};

/**
 * @brief Comprehensive options menu state
 * 
 * The OptionsState provides a full-featured options menu with:
 * - Category-based organization (Audio, Graphics, Input, Gameplay)
 * - Multiple UI element types (sliders, toggles, dropdowns)
 * - Real-time preview of changes
 * - Validation and error handling
 * - Restart notifications for settings that require it
 * - Keyboard and mouse navigation
 * - Save/Cancel/Reset functionality
 * 
 * Navigation:
 * - Arrow keys: Navigate between options
 * - Enter: Select/Edit option
 * - Left/Right: Modify values (sliders, toggles, dropdowns)
 * - Escape: Go back/Cancel
 * - Tab: Switch between categories
 * 
 * @example
 * ```cpp
 * // Add to game state manager
 * stateManager->AddState(GameStateType::OPTIONS, std::make_unique<OptionsState>());
 * 
 * // Navigate from menu
 * stateManager->ChangeState(GameStateType::OPTIONS);
 * ```
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
    std::unique_ptr<ConfigManager> m_configManager;     ///< Configuration manager
    std::vector<std::string> m_categories;              ///< Available categories
    std::vector<OptionUIElement> m_currentOptions;     ///< Current category options
    
    OptionsMode m_currentMode;                          ///< Current navigation mode
    int m_selectedCategory;                             ///< Selected category index
    int m_selectedOption;                               ///< Selected option index
    
    float m_blinkTimer;                                 ///< Selection blink timer
    bool m_showSelection;                               ///< Whether to show selection highlight
    
    bool m_hasUnsavedChanges;                          ///< Whether there are unsaved changes
    bool m_showRestartWarning;                         ///< Whether to show restart warning
    float m_warningTimer;                              ///< Warning display timer
    
    // Input state tracking
    bool m_leftPressed;                                ///< Left key state
    bool m_rightPressed;                               ///< Right key state
    float m_inputRepeatTimer;                          ///< Input repeat timer
    float m_inputRepeatDelay;                          ///< Input repeat delay
    
    /**
     * @brief Initialize the options menu
     */
    void InitializeOptions();
    
    /**
     * @brief Load options for the current category
     */
    void LoadCategoryOptions();
    
    /**
     * @brief Create UI element from config setting
     * @param setting Configuration setting
     * @return UI element
     */
    OptionUIElement CreateUIElement(const ConfigSetting& setting);
    
    /**
     * @brief Handle category selection input
     */
    void HandleCategoryInput();
    
    /**
     * @brief Handle setting selection input
     */
    void HandleSettingInput();
    
    /**
     * @brief Handle value editing input
     */
    void HandleValueInput();
    
    /**
     * @brief Navigate up in current list
     */
    void NavigateUp();
    
    /**
     * @brief Navigate down in current list
     */
    void NavigateDown();
    
    /**
     * @brief Navigate left (previous category or decrease value)
     */
    void NavigateLeft();
    
    /**
     * @brief Navigate right (next category or increase value)
     */
    void NavigateRight();
    
    /**
     * @brief Select current item
     */
    void SelectItem();
    
    /**
     * @brief Go back to previous mode/menu
     */
    void GoBack();
    
    /**
     * @brief Apply current setting value
     */
    void ApplyCurrentSetting();
    
    /**
     * @brief Save all changes
     */
    void SaveChanges();
    
    /**
     * @brief Cancel all changes
     */
    void CancelChanges();
    
    /**
     * @brief Reset current category to defaults
     */
    void ResetToDefaults();
    
    /**
     * @brief Update UI element value from config
     * @param element UI element to update
     */
    void UpdateElementFromConfig(OptionUIElement& element);
    
    /**
     * @brief Apply UI element value to config
     * @param element UI element to apply
     */
    void ApplyElementToConfig(const OptionUIElement& element);
    
    // Rendering methods
    void DrawBackground();
    void DrawTitle();
    void DrawCategorySelection();
    void DrawSettingsList();
    void DrawValueEditor();
    void DrawInstructions();
    void DrawWarnings();
    
    /**
     * @brief Draw a slider UI element
     * @param element UI element
     * @param x X position
     * @param y Y position
     * @param selected Whether element is selected
     */
    void DrawSlider(const OptionUIElement& element, int x, int y, bool selected);
    
    /**
     * @brief Draw a toggle UI element
     * @param element UI element
     * @param x X position
     * @param y Y position
     * @param selected Whether element is selected
     */
    void DrawToggle(const OptionUIElement& element, int x, int y, bool selected);
    
    /**
     * @brief Draw a dropdown UI element
     * @param element UI element
     * @param x X position
     * @param y Y position
     * @param selected Whether element is selected
     */
    void DrawDropdown(const OptionUIElement& element, int x, int y, bool selected);
    
    /**
     * @brief Get color for UI element based on state
     * @param selected Whether element is selected
     * @param requiresRestart Whether element requires restart
     * @return Color for the element
     */
    Color GetElementColor(bool selected, bool requiresRestart) const;
    
    /**
     * @brief Format value for display
     * @param element UI element
     * @return Formatted value string
     */
    std::string FormatValue(const OptionUIElement& element) const;
    
    /**
     * @brief Play menu navigation sound
     */
    void PlayNavigationSound();
    
    /**
     * @brief Play menu selection sound
     */
    void PlaySelectionSound();
    
    /**
     * @brief Play error sound
     */
    void PlayErrorSound();
};
