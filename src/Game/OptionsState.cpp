/**
 * @file OptionsState.cpp
 * @brief Implementation of the OptionsState class
 * @author Ryan Butler
 * @date 2025
 */

#include "Game/OptionsState.h"
#include "Game/GameStateManager.h"
#include "Engine/Renderer.h"
#include "Engine/InputManager.h"
#include "Engine/Engine.h"
#include "Engine/AudioManager.h"
#include "Engine/BitmapFont.h"
#include <iostream>
#include <algorithm>
#include <cmath>

OptionsState::OptionsState()
    : GameState(GameStateType::OPTIONS, "Options")
    , m_configManager(std::make_unique<ConfigManager>())
    , m_currentMode(OptionsMode::CATEGORY_SELECTION)
    , m_selectedCategory(0)
    , m_selectedOption(0)
    , m_blinkTimer(0.0f)
    , m_showSelection(true)
    , m_hasUnsavedChanges(false)
    , m_showRestartWarning(false)
    , m_warningTimer(0.0f)
    , m_leftPressed(false)
    , m_rightPressed(false)
    , m_inputRepeatTimer(0.0f)
    , m_inputRepeatDelay(0.5f) {
}

OptionsState::~OptionsState() = default;

void OptionsState::OnEnter() {
    std::cout << "Entering Options State" << std::endl;
    
    // Initialize configuration manager
    if (!m_configManager->Initialize("config.txt")) {
        std::cerr << "Failed to initialize configuration manager" << std::endl;
    }
    
    InitializeOptions();
    
    // Reset state
    m_currentMode = OptionsMode::CATEGORY_SELECTION;
    m_selectedCategory = 0;
    m_selectedOption = 0;
    m_blinkTimer = 0.0f;
    m_showSelection = true;
    m_hasUnsavedChanges = false;
    m_showRestartWarning = false;
    
    // Load menu sounds
    if (GetEngine()->GetAudioManager()) {
        GetEngine()->GetAudioManager()->LoadSound("menu_navigate", "assets/sounds/menu_select.wav", SoundType::SOUND_EFFECT);
        GetEngine()->GetAudioManager()->LoadSound("menu_select", "assets/sounds/menu_select.wav", SoundType::SOUND_EFFECT);
        GetEngine()->GetAudioManager()->LoadSound("menu_error", "assets/sounds/menu_select.wav", SoundType::SOUND_EFFECT);
    }
    
    LoadCategoryOptions();
}

void OptionsState::OnExit() {
    std::cout << "Exiting Options State" << std::endl;
    
    // Save configuration if there are unsaved changes
    if (m_hasUnsavedChanges) {
        SaveChanges();
    }
}

void OptionsState::Update(float deltaTime) {
    HandleInput();
    
    // Update selection blink effect
    m_blinkTimer += deltaTime;
    if (m_blinkTimer >= 0.5f) {
        m_showSelection = !m_showSelection;
        m_blinkTimer = 0.0f;
    }
    
    // Update warning timer
    if (m_showRestartWarning) {
        m_warningTimer += deltaTime;
        if (m_warningTimer >= 3.0f) {
            m_showRestartWarning = false;
            m_warningTimer = 0.0f;
        }
    }
    
    // Update input repeat timer
    if (m_leftPressed || m_rightPressed) {
        m_inputRepeatTimer += deltaTime;
    } else {
        m_inputRepeatTimer = 0.0f;
    }
}

void OptionsState::Render() {
    auto* renderer = GetRenderer();
    if (!renderer) {
        std::cout << "❌ OptionsState: No renderer available!" << std::endl;
        return;
    }
    
    DrawBackground();
    DrawTitle();
    
    switch (m_currentMode) {
        case OptionsMode::CATEGORY_SELECTION:
            DrawCategorySelection();
            break;
        case OptionsMode::SETTING_SELECTION:
            DrawSettingsList();
            break;
        case OptionsMode::VALUE_EDITING:
            DrawValueEditor();
            break;
    }
    
    DrawInstructions();
    DrawWarnings();
}

void OptionsState::HandleInput() {
    switch (m_currentMode) {
        case OptionsMode::CATEGORY_SELECTION:
            HandleCategoryInput();
            break;
        case OptionsMode::SETTING_SELECTION:
            HandleSettingInput();
            break;
        case OptionsMode::VALUE_EDITING:
            HandleValueInput();
            break;
    }
}

void OptionsState::InitializeOptions() {
    m_categories = m_configManager->GetCategories();
    if (m_categories.empty()) {
        std::cerr << "No configuration categories found!" << std::endl;
        m_categories.push_back("General");
    }
    
    std::cout << "Initialized options with " << m_categories.size() << " categories" << std::endl;
}

void OptionsState::LoadCategoryOptions() {
    m_currentOptions.clear();
    
    if (m_selectedCategory >= 0 && m_selectedCategory < static_cast<int>(m_categories.size())) {
        const std::string& category = m_categories[m_selectedCategory];
        auto settingKeys = m_configManager->GetSettingsInCategory(category);
        
        for (const std::string& key : settingKeys) {
            const ConfigSetting* setting = m_configManager->GetSetting(key);
            if (setting) {
                OptionUIElement element = CreateUIElement(*setting);
                UpdateElementFromConfig(element);
                m_currentOptions.push_back(element);
            }
        }
        
        std::cout << "Loaded " << m_currentOptions.size() << " options for category: " << category << std::endl;
    }
    
    m_selectedOption = 0;
}

OptionUIElement OptionsState::CreateUIElement(const ConfigSetting& setting) {
    OptionUIType uiType = OptionUIType::TEXT_INPUT;
    
    switch (setting.defaultValue.GetType()) {
        case ConfigType::BOOL:
            uiType = OptionUIType::BOOLEAN;
            break;
        case ConfigType::INT:
        case ConfigType::ENUM:
            if (!setting.enumOptions.empty()) {
                uiType = OptionUIType::DROPDOWN;
            } else {
                uiType = OptionUIType::SLIDER;
            }
            break;
        case ConfigType::FLOAT:
            uiType = OptionUIType::SLIDER;
            break;
        case ConfigType::STRING:
            uiType = OptionUIType::TEXT_INPUT;
            break;
    }
    
    OptionUIElement element(setting.key, setting.displayName, setting.description, uiType, setting.requiresRestart);
    
    // Set up dropdown options
    if (uiType == OptionUIType::DROPDOWN) {
        element.options = setting.enumOptions;
    }
    
    return element;
}

void OptionsState::HandleCategoryInput() {
    auto* input = GetInputManager();
    if (!input) return;
    
    // Navigation
    if (input->IsKeyJustPressed(SDL_SCANCODE_UP) || input->IsKeyJustPressed(SDL_SCANCODE_W)) {
        NavigateUp();
    }
    if (input->IsKeyJustPressed(SDL_SCANCODE_DOWN) || input->IsKeyJustPressed(SDL_SCANCODE_S)) {
        NavigateDown();
    }
    
    // Selection
    if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN) || input->IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
        SelectItem();
    }
    
    // Back to menu
    if (input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        GoBack();
    }
}

void OptionsState::HandleSettingInput() {
    auto* input = GetInputManager();
    if (!input) return;
    
    // Navigation
    if (input->IsKeyJustPressed(SDL_SCANCODE_UP) || input->IsKeyJustPressed(SDL_SCANCODE_W)) {
        NavigateUp();
    }
    if (input->IsKeyJustPressed(SDL_SCANCODE_DOWN) || input->IsKeyJustPressed(SDL_SCANCODE_S)) {
        NavigateDown();
    }
    
    // Quick value modification
    bool leftPressed = input->IsKeyPressed(SDL_SCANCODE_LEFT) || input->IsKeyPressed(SDL_SCANCODE_A);
    bool rightPressed = input->IsKeyPressed(SDL_SCANCODE_RIGHT) || input->IsKeyPressed(SDL_SCANCODE_D);
    
    // Handle input repeat
    if (leftPressed && (!m_leftPressed || m_inputRepeatTimer >= m_inputRepeatDelay)) {
        NavigateLeft();
        m_inputRepeatTimer = m_leftPressed ? 0.0f : -0.3f; // Initial delay
    }
    if (rightPressed && (!m_rightPressed || m_inputRepeatTimer >= m_inputRepeatDelay)) {
        NavigateRight();
        m_inputRepeatTimer = m_rightPressed ? 0.0f : -0.3f; // Initial delay
    }
    
    m_leftPressed = leftPressed;
    m_rightPressed = rightPressed;
    
    // Selection for detailed editing
    if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN)) {
        SelectItem();
    }
    
    // Back to categories
    if (input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        GoBack();
    }
    
    // Tab to switch categories
    if (input->IsKeyJustPressed(SDL_SCANCODE_TAB)) {
        m_selectedCategory = (m_selectedCategory + 1) % static_cast<int>(m_categories.size());
        LoadCategoryOptions();
        PlayNavigationSound();
    }
}

void OptionsState::HandleValueInput() {
    auto* input = GetInputManager();
    if (!input) return;
    
    // Quick value modification
    bool leftPressed = input->IsKeyPressed(SDL_SCANCODE_LEFT) || input->IsKeyPressed(SDL_SCANCODE_A);
    bool rightPressed = input->IsKeyPressed(SDL_SCANCODE_RIGHT) || input->IsKeyPressed(SDL_SCANCODE_D);
    
    // Handle input repeat
    if (leftPressed && (!m_leftPressed || m_inputRepeatTimer >= 0.1f)) {
        NavigateLeft();
        m_inputRepeatTimer = 0.0f;
    }
    if (rightPressed && (!m_rightPressed || m_inputRepeatTimer >= 0.1f)) {
        NavigateRight();
        m_inputRepeatTimer = 0.0f;
    }
    
    m_leftPressed = leftPressed;
    m_rightPressed = rightPressed;
    
    // Apply changes
    if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN)) {
        ApplyCurrentSetting();
        m_currentMode = OptionsMode::SETTING_SELECTION;
        PlaySelectionSound();
    }
    
    // Cancel changes
    if (input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        // Restore original value
        if (m_selectedOption >= 0 && m_selectedOption < static_cast<int>(m_currentOptions.size())) {
            UpdateElementFromConfig(m_currentOptions[m_selectedOption]);
        }
        m_currentMode = OptionsMode::SETTING_SELECTION;
        PlayNavigationSound();
    }
}

void OptionsState::NavigateUp() {
    if (m_currentMode == OptionsMode::CATEGORY_SELECTION) {
        m_selectedCategory--;
        if (m_selectedCategory < 0) {
            m_selectedCategory = static_cast<int>(m_categories.size()) - 1;
        }
        LoadCategoryOptions();
    } else if (m_currentMode == OptionsMode::SETTING_SELECTION) {
        m_selectedOption--;
        if (m_selectedOption < 0) {
            m_selectedOption = static_cast<int>(m_currentOptions.size()) - 1;
        }
    }

    m_showSelection = true;
    m_blinkTimer = 0.0f;
    PlayNavigationSound();
}

void OptionsState::NavigateDown() {
    if (m_currentMode == OptionsMode::CATEGORY_SELECTION) {
        m_selectedCategory++;
        if (m_selectedCategory >= static_cast<int>(m_categories.size())) {
            m_selectedCategory = 0;
        }
        LoadCategoryOptions();
    } else if (m_currentMode == OptionsMode::SETTING_SELECTION) {
        m_selectedOption++;
        if (m_selectedOption >= static_cast<int>(m_currentOptions.size())) {
            m_selectedOption = 0;
        }
    }

    m_showSelection = true;
    m_blinkTimer = 0.0f;
    PlayNavigationSound();
}

void OptionsState::NavigateLeft() {
    if (m_currentMode == OptionsMode::SETTING_SELECTION || m_currentMode == OptionsMode::VALUE_EDITING) {
        if (m_selectedOption >= 0 && m_selectedOption < static_cast<int>(m_currentOptions.size())) {
            OptionUIElement& element = m_currentOptions[m_selectedOption];

            switch (element.type) {
                case OptionUIType::BOOLEAN:
                    // Toggle boolean
                    element.currentValue = element.currentValue > 0.5f ? 0.0f : 1.0f;
                    if (m_currentMode == OptionsMode::SETTING_SELECTION) {
                        ApplyElementToConfig(element);
                    }
                    break;

                case OptionUIType::SLIDER: {
                    // Decrease slider value
                    const ConfigSetting* setting = m_configManager->GetSetting(element.key);
                    if (setting) {
                        float step = (setting->defaultValue.GetType() == ConfigType::FLOAT) ? 0.05f : 1.0f;
                        element.currentValue -= step;

                        // Clamp to min value
                        if (setting->minValue.GetType() == setting->defaultValue.GetType()) {
                            float minVal = (setting->defaultValue.GetType() == ConfigType::FLOAT) ?
                                         setting->minValue.AsFloat() : static_cast<float>(setting->minValue.AsInt());
                            element.currentValue = std::max(element.currentValue, minVal);
                        }

                        if (m_currentMode == OptionsMode::SETTING_SELECTION) {
                            ApplyElementToConfig(element);
                        }
                    }
                    break;
                }

                case OptionUIType::DROPDOWN:
                    // Previous option
                    element.currentIndex--;
                    if (element.currentIndex < 0) {
                        element.currentIndex = static_cast<int>(element.options.size()) - 1;
                    }
                    if (m_currentMode == OptionsMode::SETTING_SELECTION) {
                        ApplyElementToConfig(element);
                    }
                    break;

                default:
                    break;
            }

            PlayNavigationSound();
        }
    }
}

void OptionsState::NavigateRight() {
    if (m_currentMode == OptionsMode::SETTING_SELECTION || m_currentMode == OptionsMode::VALUE_EDITING) {
        if (m_selectedOption >= 0 && m_selectedOption < static_cast<int>(m_currentOptions.size())) {
            OptionUIElement& element = m_currentOptions[m_selectedOption];

            switch (element.type) {
                case OptionUIType::BOOLEAN:
                    // Toggle boolean
                    element.currentValue = element.currentValue > 0.5f ? 0.0f : 1.0f;
                    if (m_currentMode == OptionsMode::SETTING_SELECTION) {
                        ApplyElementToConfig(element);
                    }
                    break;

                case OptionUIType::SLIDER: {
                    // Increase slider value
                    const ConfigSetting* setting = m_configManager->GetSetting(element.key);
                    if (setting) {
                        float step = (setting->defaultValue.GetType() == ConfigType::FLOAT) ? 0.05f : 1.0f;
                        element.currentValue += step;

                        // Clamp to max value
                        if (setting->maxValue.GetType() == setting->defaultValue.GetType()) {
                            float maxVal = (setting->defaultValue.GetType() == ConfigType::FLOAT) ?
                                         setting->maxValue.AsFloat() : static_cast<float>(setting->maxValue.AsInt());
                            element.currentValue = std::min(element.currentValue, maxVal);
                        }

                        if (m_currentMode == OptionsMode::SETTING_SELECTION) {
                            ApplyElementToConfig(element);
                        }
                    }
                    break;
                }

                case OptionUIType::DROPDOWN:
                    // Next option
                    element.currentIndex++;
                    if (element.currentIndex >= static_cast<int>(element.options.size())) {
                        element.currentIndex = 0;
                    }
                    if (m_currentMode == OptionsMode::SETTING_SELECTION) {
                        ApplyElementToConfig(element);
                    }
                    break;

                default:
                    break;
            }

            PlayNavigationSound();
        }
    }
}

void OptionsState::SelectItem() {
    if (m_currentMode == OptionsMode::CATEGORY_SELECTION) {
        m_currentMode = OptionsMode::SETTING_SELECTION;
        m_selectedOption = 0;
        PlaySelectionSound();
    } else if (m_currentMode == OptionsMode::SETTING_SELECTION) {
        if (m_selectedOption >= 0 && m_selectedOption < static_cast<int>(m_currentOptions.size())) {
            const OptionUIElement& element = m_currentOptions[m_selectedOption];

            // For text input, enter editing mode
            if (element.type == OptionUIType::TEXT_INPUT) {
                m_currentMode = OptionsMode::VALUE_EDITING;
                PlaySelectionSound();
            }
            // For other types, they're already being modified with left/right
        }
    }
}

void OptionsState::GoBack() {
    if (m_currentMode == OptionsMode::SETTING_SELECTION) {
        m_currentMode = OptionsMode::CATEGORY_SELECTION;
        PlayNavigationSound();
    } else if (m_currentMode == OptionsMode::VALUE_EDITING) {
        m_currentMode = OptionsMode::SETTING_SELECTION;
        PlayNavigationSound();
    } else {
        // Exit options menu
        if (GetStateManager()) {
            GetStateManager()->ChangeState(GameStateType::MENU);
        }
    }
}

void OptionsState::ApplyCurrentSetting() {
    if (m_selectedOption >= 0 && m_selectedOption < static_cast<int>(m_currentOptions.size())) {
        ApplyElementToConfig(m_currentOptions[m_selectedOption]);
    }
}

void OptionsState::SaveChanges() {
    if (m_configManager->SaveToFile()) {
        m_hasUnsavedChanges = false;
        std::cout << "Configuration saved successfully" << std::endl;
    } else {
        std::cerr << "Failed to save configuration" << std::endl;
        PlayErrorSound();
    }
}

void OptionsState::CancelChanges() {
    // Reload from file to discard changes
    m_configManager->LoadFromFile();
    LoadCategoryOptions();
    m_hasUnsavedChanges = false;
    std::cout << "Configuration changes cancelled" << std::endl;
}

void OptionsState::ResetToDefaults() {
    if (m_selectedCategory >= 0 && m_selectedCategory < static_cast<int>(m_categories.size())) {
        const std::string& category = m_categories[m_selectedCategory];
        auto settingKeys = m_configManager->GetSettingsInCategory(category);

        for (const std::string& key : settingKeys) {
            m_configManager->ResetToDefault(key);
        }

        LoadCategoryOptions();
        m_hasUnsavedChanges = true;
        std::cout << "Reset " << category << " settings to defaults" << std::endl;
    }
}

void OptionsState::UpdateElementFromConfig(OptionUIElement& element) {
    const ConfigSetting* setting = m_configManager->GetSetting(element.key);
    if (!setting) return;

    ConfigValue value = m_configManager->GetValue(element.key);

    switch (element.type) {
        case OptionUIType::BOOLEAN:
            element.currentValue = value.AsBool() ? 1.0f : 0.0f;
            break;

        case OptionUIType::SLIDER:
            if (value.GetType() == ConfigType::FLOAT) {
                element.currentValue = value.AsFloat();
            } else {
                element.currentValue = static_cast<float>(value.AsInt());
            }
            break;

        case OptionUIType::DROPDOWN:
            element.currentIndex = value.AsInt();
            // Clamp to valid range
            if (element.currentIndex < 0) element.currentIndex = 0;
            if (element.currentIndex >= static_cast<int>(element.options.size())) {
                element.currentIndex = static_cast<int>(element.options.size()) - 1;
            }
            break;

        case OptionUIType::TEXT_INPUT:
            // Text input handled separately
            break;

        default:
            break;
    }
}

void OptionsState::ApplyElementToConfig(const OptionUIElement& element) {
    const ConfigSetting* setting = m_configManager->GetSetting(element.key);
    if (!setting) return;

    bool changed = false;

    switch (element.type) {
        case OptionUIType::BOOLEAN:
            changed = m_configManager->SetBool(element.key, element.currentValue > 0.5f);
            break;

        case OptionUIType::SLIDER:
            if (setting->defaultValue.GetType() == ConfigType::FLOAT) {
                changed = m_configManager->SetFloat(element.key, element.currentValue);
            } else {
                changed = m_configManager->SetInt(element.key, static_cast<int>(element.currentValue));
            }
            break;

        case OptionUIType::DROPDOWN:
            changed = m_configManager->SetInt(element.key, element.currentIndex);
            break;

        default:
            break;
    }

    if (changed) {
        m_hasUnsavedChanges = true;

        // Show restart warning if needed
        if (element.requiresRestart) {
            m_showRestartWarning = true;
            m_warningTimer = 0.0f;
        }

        // Apply audio changes immediately
        if (element.key.find("audio.") == 0 && GetEngine()->GetAudioManager()) {
            if (element.key == "audio.masterVolume") {
                // Update master volume - would need AudioManager integration
            } else if (element.key == "audio.musicVolume") {
                GetEngine()->GetAudioManager()->SetMusicVolume(m_configManager->GetFloat(element.key));
            }
        }
    }
}

// Sound helper methods
void OptionsState::PlayNavigationSound() {
    if (GetEngine()->GetAudioManager()) {
        GetEngine()->GetAudioManager()->PlaySound("menu_navigate", 0.5f);
    }
}

void OptionsState::PlaySelectionSound() {
    if (GetEngine()->GetAudioManager()) {
        GetEngine()->GetAudioManager()->PlaySound("menu_select", 0.7f);
    }
}

void OptionsState::PlayErrorSound() {
    if (GetEngine()->GetAudioManager()) {
        GetEngine()->GetAudioManager()->PlaySound("menu_error", 0.8f);
    }
}

// Rendering methods
void OptionsState::DrawBackground() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    // Draw gradient background
    for (int y = 0; y < 600; y++) {
        int intensity = 20 + (y * 15) / 600;
        Color bgColor(intensity, intensity, intensity * 1.2f, 255);
        renderer->DrawLine(0, y, 800, y, bgColor);
    }
}

void OptionsState::DrawTitle() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    std::string title = "OPTIONS";
    int titleWidth = title.length() * 24; // Assuming 24 pixels per character at scale 3
    int titleX = (800 - titleWidth) / 2;

    // Draw title with glow effect
    BitmapFont::DrawText(renderer, title, titleX + 2, 52, 3, Color(100, 100, 0, 255)); // Shadow
    BitmapFont::DrawText(renderer, title, titleX, 50, 3, Color(255, 255, 100, 255)); // Main text
}

void OptionsState::DrawCategorySelection() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    int startY = 150;
    int spacing = 50;

    // Draw category list
    for (int i = 0; i < static_cast<int>(m_categories.size()); i++) {
        int y = startY + i * spacing;
        bool isSelected = (i == m_selectedCategory);

        // Calculate text position
        int textWidth = m_categories[i].length() * 16; // 16 pixels per char at scale 2
        int x = (800 - textWidth) / 2;

        // Draw selection highlight
        if (isSelected && m_showSelection) {
            Rectangle highlight = {x - 20, y - 5, textWidth + 40, 30};
            renderer->DrawRectangle(highlight, Color(100, 100, 0, 100), true);
            renderer->DrawRectangle(highlight, Color(255, 255, 0, 255), false);
        }

        // Draw category name
        Color textColor = isSelected ? Color(255, 255, 100, 255) : Color(200, 200, 200, 255);
        BitmapFont::DrawText(renderer, m_categories[i], x, y, 2, textColor);
    }
}

void OptionsState::DrawSettingsList() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    // Draw category title
    if (m_selectedCategory >= 0 && m_selectedCategory < static_cast<int>(m_categories.size())) {
        std::string categoryTitle = m_categories[m_selectedCategory] + " Settings";
        int titleWidth = categoryTitle.length() * 12; // 12 pixels per char at scale 1.5
        int titleX = (800 - titleWidth) / 2;

        BitmapFont::DrawText(renderer, categoryTitle, titleX, 120, 2, Color(255, 255, 150, 255));
    }

    int startY = 180;
    int spacing = 45;

    // Draw settings list
    for (int i = 0; i < static_cast<int>(m_currentOptions.size()); i++) {
        int y = startY + i * spacing;
        bool isSelected = (i == m_selectedOption);

        const OptionUIElement& element = m_currentOptions[i];

        // Draw setting name
        int nameX = 50;
        Color nameColor = GetElementColor(isSelected, element.requiresRestart);
        BitmapFont::DrawText(renderer, element.displayName, nameX, y, 1, nameColor);

        // Draw setting value/control
        int valueX = 450;
        switch (element.type) {
            case OptionUIType::BOOLEAN:
                DrawToggle(element, valueX, y, isSelected);
                break;
            case OptionUIType::SLIDER:
                DrawSlider(element, valueX, y, isSelected);
                break;
            case OptionUIType::DROPDOWN:
                DrawDropdown(element, valueX, y, isSelected);
                break;
            default:
                // Draw current value as text
                std::string valueStr = FormatValue(element);
                BitmapFont::DrawText(renderer, valueStr, valueX, y, 1, nameColor);
                break;
        }

        // Draw restart indicator
        if (element.requiresRestart) {
            BitmapFont::DrawText(renderer, "*", nameX - 15, y, 1, Color(255, 100, 100, 255));
        }
    }
}

void OptionsState::DrawValueEditor() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    // For now, value editing is handled inline with the settings list
    DrawSettingsList();

    // Draw editing indicator
    if (m_selectedOption >= 0 && m_selectedOption < static_cast<int>(m_currentOptions.size())) {
        std::string editText = "EDITING - Use Left/Right to adjust, Enter to confirm, Escape to cancel";
        int textWidth = editText.length() * 6;
        int x = (800 - textWidth) / 2;

        BitmapFont::DrawText(renderer, editText, x, 520, 1, Color(255, 255, 100, 255));
    }
}

void OptionsState::DrawInstructions() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    std::vector<std::string> instructions;

    switch (m_currentMode) {
        case OptionsMode::CATEGORY_SELECTION:
            instructions = {
                "UP/DOWN: Navigate categories",
                "ENTER: Select category",
                "ESCAPE: Back to main menu"
            };
            break;
        case OptionsMode::SETTING_SELECTION:
            instructions = {
                "UP/DOWN: Navigate settings",
                "LEFT/RIGHT: Adjust values",
                "ENTER: Edit value",
                "TAB: Switch category",
                "ESCAPE: Back to categories"
            };
            break;
        case OptionsMode::VALUE_EDITING:
            instructions = {
                "LEFT/RIGHT: Adjust value",
                "ENTER: Confirm",
                "ESCAPE: Cancel"
            };
            break;
    }

    int startY = 550;
    for (size_t i = 0; i < instructions.size(); i++) {
        int textWidth = instructions[i].length() * 6;
        int x = (800 - textWidth) / 2;
        int y = startY + static_cast<int>(i) * 15;

        BitmapFont::DrawText(renderer, instructions[i], x, y, 1, Color(180, 180, 180, 255));
    }
}

void OptionsState::DrawWarnings() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    if (m_showRestartWarning) {
        std::string warning = "* Settings marked with * require restart to take effect";
        int textWidth = warning.length() * 6;
        int x = (800 - textWidth) / 2;

        // Blink effect
        float alpha = 0.5f + 0.5f * std::sin(m_warningTimer * 6.0f);
        Color warningColor(255, 150, 100, static_cast<int>(255 * alpha));

        BitmapFont::DrawText(renderer, warning, x, 100, 1, warningColor);
    }

    if (m_hasUnsavedChanges) {
        std::string unsavedText = "Unsaved changes - will be saved when exiting options";
        int textWidth = unsavedText.length() * 6;
        int x = (800 - textWidth) / 2;

        BitmapFont::DrawText(renderer, unsavedText, x, 85, 1, Color(255, 200, 100, 255));
    }
}

void OptionsState::DrawSlider(const OptionUIElement& element, int x, int y, bool selected) {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    const ConfigSetting* setting = m_configManager->GetSetting(element.key);
    if (!setting) return;

    // Calculate slider parameters
    float minVal = 0.0f, maxVal = 1.0f;
    if (setting->minValue.GetType() == setting->defaultValue.GetType()) {
        minVal = (setting->defaultValue.GetType() == ConfigType::FLOAT) ?
                 setting->minValue.AsFloat() : static_cast<float>(setting->minValue.AsInt());
    }
    if (setting->maxValue.GetType() == setting->defaultValue.GetType()) {
        maxVal = (setting->defaultValue.GetType() == ConfigType::FLOAT) ?
                 setting->maxValue.AsFloat() : static_cast<float>(setting->maxValue.AsInt());
    }

    float normalizedValue = (element.currentValue - minVal) / (maxVal - minVal);
    normalizedValue = std::max(0.0f, std::min(1.0f, normalizedValue));

    // Draw slider track
    int sliderWidth = 200;
    int sliderHeight = 6;
    Rectangle track = {x, y + 6, sliderWidth, sliderHeight};

    Color trackColor = selected ? Color(100, 100, 100, 255) : Color(60, 60, 60, 255);
    renderer->DrawRectangle(track, trackColor, true);

    // Draw slider fill
    Rectangle fill = {x, y + 6, static_cast<int>(sliderWidth * normalizedValue), sliderHeight};
    Color fillColor = selected ? Color(100, 200, 255, 255) : Color(80, 160, 200, 255);
    renderer->DrawRectangle(fill, fillColor, true);

    // Draw slider handle
    int handleX = x + static_cast<int>(sliderWidth * normalizedValue) - 4;
    Rectangle handle = {handleX, y + 2, 8, 14};
    Color handleColor = selected ? Color(255, 255, 255, 255) : Color(200, 200, 200, 255);
    renderer->DrawRectangle(handle, handleColor, true);

    // Draw value text
    std::string valueText = FormatValue(element);
    BitmapFont::DrawText(renderer, valueText, x + sliderWidth + 10, y, 1,
                        selected ? Color(255, 255, 255, 255) : Color(200, 200, 200, 255));
}

void OptionsState::DrawToggle(const OptionUIElement& element, int x, int y, bool selected) {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    bool isOn = element.currentValue > 0.5f;

    // Draw toggle background
    Rectangle toggleBg = {x, y + 2, 60, 16};
    Color bgColor = isOn ? Color(50, 150, 50, 255) : Color(150, 50, 50, 255);
    if (selected) {
        bgColor.r = std::min(255, bgColor.r + 50);
        bgColor.g = std::min(255, bgColor.g + 50);
        bgColor.b = std::min(255, bgColor.b + 50);
    }
    renderer->DrawRectangle(toggleBg, bgColor, true);

    // Draw toggle border
    Color borderColor = selected ? Color(255, 255, 255, 255) : Color(150, 150, 150, 255);
    renderer->DrawRectangle(toggleBg, borderColor, false);

    // Draw toggle switch
    int switchX = isOn ? x + 44 : x + 4;
    Rectangle toggleSwitch = {switchX, y + 4, 12, 12};
    Color switchColor = selected ? Color(255, 255, 255, 255) : Color(220, 220, 220, 255);
    renderer->DrawRectangle(toggleSwitch, switchColor, true);

    // Draw state text
    std::string stateText = isOn ? "ON" : "OFF";
    BitmapFont::DrawText(renderer, stateText, x + 70, y, 1,
                        selected ? Color(255, 255, 255, 255) : Color(200, 200, 200, 255));
}

void OptionsState::DrawDropdown(const OptionUIElement& element, int x, int y, bool selected) {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    // Draw dropdown background
    Rectangle dropdownBg = {x, y, 200, 18};
    Color bgColor = selected ? Color(80, 80, 80, 255) : Color(50, 50, 50, 255);
    renderer->DrawRectangle(dropdownBg, bgColor, true);

    // Draw dropdown border
    Color borderColor = selected ? Color(255, 255, 255, 255) : Color(150, 150, 150, 255);
    renderer->DrawRectangle(dropdownBg, borderColor, false);

    // Draw current selection
    if (element.currentIndex >= 0 && element.currentIndex < static_cast<int>(element.options.size())) {
        const std::string& currentOption = element.options[element.currentIndex];
        Color textColor = selected ? Color(255, 255, 255, 255) : Color(200, 200, 200, 255);
        BitmapFont::DrawText(renderer, currentOption, x + 5, y + 2, 1, textColor);
    }

    // Draw dropdown arrow
    std::string arrow = selected ? "▼" : "▽";
    BitmapFont::DrawText(renderer, arrow, x + 180, y + 2, 1, borderColor);
}

Color OptionsState::GetElementColor(bool selected, bool requiresRestart) const {
    if (requiresRestart) {
        return selected ? Color(255, 200, 200, 255) : Color(200, 150, 150, 255);
    } else {
        return selected ? Color(255, 255, 255, 255) : Color(200, 200, 200, 255);
    }
}

std::string OptionsState::FormatValue(const OptionUIElement& element) const {
    switch (element.type) {
        case OptionUIType::BOOLEAN:
            return element.currentValue > 0.5f ? "ON" : "OFF";

        case OptionUIType::SLIDER: {
            const ConfigSetting* setting = m_configManager->GetSetting(element.key);
            if (setting && setting->defaultValue.GetType() == ConfigType::FLOAT) {
                return std::to_string(element.currentValue).substr(0, 4); // Limit decimal places
            } else {
                return std::to_string(static_cast<int>(element.currentValue));
            }
        }

        case OptionUIType::DROPDOWN:
            if (element.currentIndex >= 0 && element.currentIndex < static_cast<int>(element.options.size())) {
                return element.options[element.currentIndex];
            }
            return "Unknown";

        default:
            return "N/A";
    }
}
