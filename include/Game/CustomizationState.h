/**
 * @file CustomizationState.h
 * @brief Character customization screen state
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "Game/GameState.h"
#include "Game/PlayerCustomization.h"
#include "Game/GameConfig.h"
#include <memory>
#include <string>

/**
 * @class CustomizationState
 * @brief Game state for character customization screen
 * 
 * Provides an interface for players to customize their character's
 * appearance, attributes, and starting equipment before beginning gameplay.
 */
class CustomizationState : public GameState {
public:
    CustomizationState();
    ~CustomizationState() override = default;

    // GameState interface
    void OnEnter() override;
    void OnExit() override;
    void Update(float deltaTime) override;
    void Render() override;
    void HandleInput() override;

private:
    // Customization management
    std::unique_ptr<CustomizationManager> m_customizationManager;
    
    // UI state
    enum class UIMode {
        CATEGORY_SELECTION,  // Selecting which category to customize
        OPTION_SELECTION,    // Selecting specific options within a category
        NAME_INPUT,          // Entering player name
        ATTRIBUTE_ADJUSTMENT, // Adjusting attribute points
        CONFIRMATION         // Final confirmation before starting game
    };
    
    UIMode m_currentMode = UIMode::CATEGORY_SELECTION;
    
    // Navigation state
    int m_selectedCategoryIndex = 0;
    int m_selectedGroupIndex = 0;
    int m_selectedOptionIndex = 0;
    
    // Categories for navigation
    std::vector<CustomizationCategory> m_categories;
    std::vector<CustomizationGroup*> m_currentCategoryGroups;
    
    // Name input state
    std::string m_nameInput;
    bool m_nameInputActive = false;
    
    // Attribute adjustment state
    std::string m_selectedAttributeGroup;

    // Game configuration for animation and sprite settings
    std::unique_ptr<GameConfig> m_gameConfig;

    // UI helpers
    void UpdateCategorySelection();
    void UpdateOptionSelection();
    void UpdateNameInput();
    void UpdateAttributeAdjustment();
    void UpdateConfirmation();

    void RenderCategorySelection();
    void RenderOptionSelection();
    void RenderNameInput();
    void RenderAttributeAdjustment();
    void RenderConfirmation();
    void RenderCharacterPreview();
    void RenderInstructions();

    // Input handling
    void HandleCategoryInput();
    void HandleOptionInput();
    void HandleNameInputKeys();
    void HandleAttributeInput();
    void HandleConfirmationInput();

    // Navigation helpers
    void SelectCategory(int index);
    void SelectOption(int groupIndex, int optionIndex);
    void ApplyCurrentSelection();
    void StartGame();
    void ReturnToMenu();

    // UI constants
    static constexpr int SCREEN_WIDTH = 800;
    static constexpr int SCREEN_HEIGHT = 600;
    static constexpr int MARGIN = 40;
    static constexpr int LINE_HEIGHT = 25;
    static constexpr int TITLE_HEIGHT = 60;

    // Color constants (RGB values)
    static const int COLOR_BACKGROUND_R = 20;
    static const int COLOR_BACKGROUND_G = 25;
    static const int COLOR_BACKGROUND_B = 40;

    static const int COLOR_TEXT_R = 220;
    static const int COLOR_TEXT_G = 220;
    static const int COLOR_TEXT_B = 220;

    static const int COLOR_SELECTED_R = 100;
    static const int COLOR_SELECTED_G = 150;
    static const int COLOR_SELECTED_B = 255;

    static const int COLOR_ACCENT_R = 255;
    static const int COLOR_ACCENT_G = 200;
    static const int COLOR_ACCENT_B = 100;

    static const int COLOR_PREVIEW_BG_R = 40;
    static const int COLOR_PREVIEW_BG_G = 45;
    static const int COLOR_PREVIEW_BG_B = 60;
};
