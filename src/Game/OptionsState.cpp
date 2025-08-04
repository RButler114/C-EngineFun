/**
 * @file OptionsState.cpp
 * @brief Simple options menu implementation for arcade games
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
#include "Engine/KeybindingManager.h"
#include <iostream>
#include <algorithm>

OptionsState::OptionsState()
    : GameState(GameStateType::OPTIONS, "Options")
    , m_selectedOption(0)
    , m_blinkTimer(0.0f)
    , m_showSelection(true)
    , m_musicVolume(0.5f)
    , m_soundVolume(0.7f)
    , m_keybindingManager(std::make_unique<KeybindingManager>())
    , m_inKeybindingMode(false)
    , m_selectedKeybinding(0)
    , m_waitingForKey(false)
    , m_keyToRebind(GameAction::ACTION_COUNT)
    , m_rebindingPrimary(true)
    , m_keybindingScrollOffset(0)
    , m_maxVisibleKeybindings(0) {

    InitializeOptions();
}

OptionsState::~OptionsState() = default;

void OptionsState::OnEnter() {
    std::cout << "Entering Options State" << std::endl;

    // Get current audio settings
    if (GetEngine()->GetAudioManager()) {
        m_musicVolume = GetEngine()->GetAudioManager()->GetMusicVolume();
        m_soundVolume = GetEngine()->GetAudioManager()->GetCategoryVolume(SoundType::SOUND_EFFECT);
    }

    // Load keybindings
    if (!m_keybindingManager->LoadFromConfig("assets/config/keybindings.ini")) {
        std::cout << "Using default keybindings" << std::endl;
    }

    // Get configurable actions
    m_configurableActions = m_keybindingManager->GetConfigurableActions();

    // Calculate layout
    CalculateKeybindingLayout();

    // Reset selection
    m_selectedOption = 0;
    m_selectedKeybinding = 0;
    m_keybindingScrollOffset = 0;
    m_inKeybindingMode = false;
    m_waitingForKey = false;
    m_blinkTimer = 0.0f;
    m_showSelection = true;

    // Load menu sounds
    if (GetEngine()->GetAudioManager()) {
        GetEngine()->GetAudioManager()->LoadSound("menu_select", "assets/sounds/menu_select.wav", SoundType::SOUND_EFFECT);
    }
}

void OptionsState::OnExit() {
    std::cout << "Exiting Options State" << std::endl;
}

void OptionsState::Update(float deltaTime) {
    HandleInput();

    // Update selection blink effect
    m_blinkTimer += deltaTime;
    if (m_blinkTimer >= 0.5f) {
        m_showSelection = !m_showSelection;
        m_blinkTimer = 0.0f;
    }
}

void OptionsState::Render() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    DrawBackground();
    DrawTitle();

    if (m_inKeybindingMode) {
        DrawKeybindings();
    } else {
        DrawOptions();
    }

    DrawInstructions();
}

void OptionsState::HandleInput() {
    auto* input = GetInputManager();
    if (!input) return;

    if (m_inKeybindingMode) {
        HandleKeybindingInput();
    } else {
        // Navigation
        if (input->IsKeyJustPressed(SDL_SCANCODE_UP) || input->IsKeyJustPressed(SDL_SCANCODE_W)) {
            NavigateUp();
        }
        if (input->IsKeyJustPressed(SDL_SCANCODE_DOWN) || input->IsKeyJustPressed(SDL_SCANCODE_S)) {
            NavigateDown();
        }

        // Value adjustment
        if (input->IsKeyJustPressed(SDL_SCANCODE_LEFT) || input->IsKeyJustPressed(SDL_SCANCODE_A)) {
            AdjustLeft();
        }
        if (input->IsKeyJustPressed(SDL_SCANCODE_RIGHT) || input->IsKeyJustPressed(SDL_SCANCODE_D)) {
            AdjustRight();
        }

        // Selection
        if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN) || input->IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
            SelectOption();
        }

        // Back - use B key for consistent navigation
        if (input->IsKeyJustPressed(SDL_SCANCODE_B) || input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
            GoBack();
        }
    }
}

void OptionsState::NavigateUp() {
    m_selectedOption--;
    if (m_selectedOption < 0) {
        m_selectedOption = static_cast<int>(m_options.size()) - 1;
    }
    m_showSelection = true;
    m_blinkTimer = 0.0f;

    if (GetEngine()->GetAudioManager()) {
        GetEngine()->GetAudioManager()->PlaySound("menu_select", 0.7f);
    }
}

void OptionsState::NavigateDown() {
    m_selectedOption++;
    if (m_selectedOption >= static_cast<int>(m_options.size())) {
        m_selectedOption = 0;
    }
    m_showSelection = true;
    m_blinkTimer = 0.0f;

    if (GetEngine()->GetAudioManager()) {
        GetEngine()->GetAudioManager()->PlaySound("menu_select", 0.7f);
    }
}

void OptionsState::AdjustLeft() {
    auto* audioManager = GetEngine()->GetAudioManager();
    if (!audioManager) return;

    if (m_selectedOption >= 0 && m_selectedOption < static_cast<int>(m_optionTypes.size())) {
        OptionType type = m_optionTypes[m_selectedOption];

        switch (type) {
            case OptionType::VOLUME_MUSIC:
                m_musicVolume = std::max(0.0f, m_musicVolume - 0.1f);
                audioManager->SetMusicVolume(m_musicVolume);
                break;
            case OptionType::VOLUME_SOUND:
                m_soundVolume = std::max(0.0f, m_soundVolume - 0.1f);
                audioManager->SetCategoryVolume(SoundType::SOUND_EFFECT, m_soundVolume);
                audioManager->PlaySound("menu_select", m_soundVolume);
                break;
            default:
                // Other options don't support left/right adjustment
                break;
        }
    }
}

void OptionsState::AdjustRight() {
    auto* audioManager = GetEngine()->GetAudioManager();
    if (!audioManager) return;

    if (m_selectedOption >= 0 && m_selectedOption < static_cast<int>(m_optionTypes.size())) {
        OptionType type = m_optionTypes[m_selectedOption];

        switch (type) {
            case OptionType::VOLUME_MUSIC:
                m_musicVolume = std::min(1.0f, m_musicVolume + 0.1f);
                audioManager->SetMusicVolume(m_musicVolume);
                break;
            case OptionType::VOLUME_SOUND:
                m_soundVolume = std::min(1.0f, m_soundVolume + 0.1f);
                audioManager->SetCategoryVolume(SoundType::SOUND_EFFECT, m_soundVolume);
                audioManager->PlaySound("menu_select", m_soundVolume);
                break;
            default:
                // Other options don't support left/right adjustment
                break;
        }
    }
}

void OptionsState::SelectOption() {
    if (m_selectedOption >= 0 && m_selectedOption < static_cast<int>(m_optionTypes.size())) {
        OptionType type = m_optionTypes[m_selectedOption];

        switch (type) {
            case OptionType::KEYBINDINGS:
                EnterKeybindingMode();
                break;
            case OptionType::BACK_TO_MENU:
                GoBack();
                break;
            default:
                // Volume options don't need special handling on select
                break;
        }
    }
}

void OptionsState::GoBack() {
    if (GetStateManager()) {
        GetStateManager()->PopState();
    }
}

void OptionsState::DrawBackground() {
    auto* renderer = GetRenderer();

    // Simple gradient background
    for (int y = 0; y < 600; y++) {
        int intensity = 20 + (y * 30) / 600;
        Color bgColor(intensity, intensity, intensity + 10, 255);
        renderer->DrawLine(0, y, 800, y, bgColor);
    }
}

void OptionsState::DrawTitle() {
    auto* renderer = GetRenderer();

    // Title
    BitmapFont::DrawText(renderer, "OPTIONS", 320, 100, 3, Color(255, 255, 100, 255));
}

void OptionsState::DrawOptions() {
    auto* renderer = GetRenderer();

    int startY = 250;
    int spacing = 60;

    for (size_t i = 0; i < m_options.size(); i++) {
        int y = startY + static_cast<int>(i) * spacing;
        bool isSelected = (static_cast<int>(i) == m_selectedOption);

        Color textColor = isSelected && m_showSelection ?
            Color(255, 255, 100, 255) : Color(200, 200, 200, 255);

        std::string displayText = m_options[i];

        // Add value display for options
        if (i < m_optionTypes.size()) {
            OptionType type = m_optionTypes[i];
            switch (type) {
                case OptionType::VOLUME_MUSIC:
                    displayText += GetVolumeDisplayText(m_musicVolume);
                    break;
                case OptionType::VOLUME_SOUND:
                    displayText += GetVolumeDisplayText(m_soundVolume);
                    break;
                default:
                    // No additional text for other options
                    break;
            }
        }

        BitmapFont::DrawText(renderer, displayText, 200, y, 2, textColor);

        // Draw selection indicator
        if (isSelected && m_showSelection) {
            BitmapFont::DrawText(renderer, ">", 170, y, 2, Color(255, 255, 100, 255));
        }
    }
}

void OptionsState::DrawInstructions() {
    auto* renderer = GetRenderer();

    // Instructions
    BitmapFont::DrawText(renderer, "ARROW KEYS: NAVIGATE", 50, 500, 1, Color(150, 150, 150, 255));
    BitmapFont::DrawText(renderer, "LEFT/RIGHT: ADJUST VALUES", 50, 520, 1, Color(150, 150, 150, 255));
    BitmapFont::DrawText(renderer, "ENTER: SELECT", 50, 540, 1, Color(150, 150, 150, 255));
    int instructionY = 520;  // Fixed position for instructions

    if (m_inKeybindingMode) {
        if (m_waitingForKey) {
            BitmapFont::DrawText(renderer, "PRESS A KEY TO BIND (ESC TO CANCEL)", 30, instructionY, 1, Color(255, 255, 100, 255));
        } else {
            BitmapFont::DrawText(renderer, "UP/DOWN: Navigate  ENTER: Rebind Primary  RIGHT: Rebind Alt", 30, instructionY, 1, Color(150, 150, 150, 255));
            BitmapFont::DrawText(renderer, "B/ESC: Back to Options", 30, instructionY + 20, 1, Color(150, 150, 150, 255));

            // Show scroll hint if needed
            if (m_configurableActions.size() > static_cast<size_t>(m_maxVisibleKeybindings)) {
                BitmapFont::DrawText(renderer, "Use UP/DOWN to scroll through all keybindings", 30, instructionY + 40, 1, Color(100, 100, 100, 255));
            }
        }
    } else {
        BitmapFont::DrawText(renderer, "ARROW KEYS: Navigate  LEFT/RIGHT: Adjust  ENTER: Select", 30, instructionY, 1, Color(150, 150, 150, 255));
        BitmapFont::DrawText(renderer, "B/ESC: Back to Menu", 30, instructionY + 20, 1, Color(150, 150, 150, 255));
    }
}

void OptionsState::InitializeOptions() {
    m_options.clear();
    m_optionTypes.clear();

    m_options.push_back("Music Volume");
    m_optionTypes.push_back(OptionType::VOLUME_MUSIC);

    m_options.push_back("Sound Volume");
    m_optionTypes.push_back(OptionType::VOLUME_SOUND);

    m_options.push_back("Keybindings");
    m_optionTypes.push_back(OptionType::KEYBINDINGS);

    m_options.push_back("Back to Menu");
    m_optionTypes.push_back(OptionType::BACK_TO_MENU);
}

void OptionsState::EnterKeybindingMode() {
    m_inKeybindingMode = true;
    m_selectedKeybinding = 0;
    m_waitingForKey = false;
    std::cout << "Entered keybinding configuration mode" << std::endl;
}

void OptionsState::ExitKeybindingMode() {
    m_inKeybindingMode = false;
    m_waitingForKey = false;
    SaveKeybindings();
    std::cout << "Exited keybinding configuration mode" << std::endl;
}

void OptionsState::HandleKeybindingInput() {
    auto* input = GetInputManager();
    if (!input) return;

    if (m_waitingForKey) {
        // Handle key rebinding
        if (input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
            CancelKeyRebinding();
            return;
        }

        // Check for any key press
        for (int scancode = 0; scancode < SDL_NUM_SCANCODES; ++scancode) {
            if (input->IsKeyJustPressed(static_cast<SDL_Scancode>(scancode))) {
                ApplyKeyRebinding(static_cast<SDL_Scancode>(scancode));
                return;
            }
        }
    } else {
        HandleKeybindingNavigation();
    }
}

void OptionsState::HandleKeybindingNavigation() {
    auto* input = GetInputManager();
    if (!input) return;

    // Navigation
    if (input->IsKeyJustPressed(SDL_SCANCODE_UP) || input->IsKeyJustPressed(SDL_SCANCODE_W)) {
        m_selectedKeybinding = std::max(0, m_selectedKeybinding - 1);
        UpdateKeybindingScroll();
    }
    if (input->IsKeyJustPressed(SDL_SCANCODE_DOWN) || input->IsKeyJustPressed(SDL_SCANCODE_S)) {
        m_selectedKeybinding = std::min(static_cast<int>(m_configurableActions.size()) - 1, m_selectedKeybinding + 1);
        UpdateKeybindingScroll();
    }

    // Select action to rebind
    if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN) || input->IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
        if (m_selectedKeybinding >= 0 && m_selectedKeybinding < static_cast<int>(m_configurableActions.size())) {
            GameAction action = m_configurableActions[m_selectedKeybinding];
            StartKeyRebinding(action, true); // Start with primary key
        }
    }

    // Alternative key rebinding (right arrow)
    if (input->IsKeyJustPressed(SDL_SCANCODE_RIGHT) || input->IsKeyJustPressed(SDL_SCANCODE_D)) {
        if (m_selectedKeybinding >= 0 && m_selectedKeybinding < static_cast<int>(m_configurableActions.size())) {
            GameAction action = m_configurableActions[m_selectedKeybinding];
            StartKeyRebinding(action, false); // Start with alternative key
        }
    }

    // Back
    if (input->IsKeyJustPressed(SDL_SCANCODE_B) || input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        ExitKeybindingMode();
    }
}

void OptionsState::StartKeyRebinding(GameAction action, bool primary) {
    m_waitingForKey = true;
    m_keyToRebind = action;
    m_rebindingPrimary = primary;

    std::cout << "Waiting for key input for " << m_keybindingManager->GetActionDisplayName(action)
              << " (" << (primary ? "primary" : "alternative") << ")" << std::endl;
}

void OptionsState::CancelKeyRebinding() {
    m_waitingForKey = false;
    m_keyToRebind = GameAction::ACTION_COUNT;
    std::cout << "Key rebinding cancelled" << std::endl;
}

void OptionsState::ApplyKeyRebinding(SDL_Scancode key) {
    if (m_keyToRebind == GameAction::ACTION_COUNT) return;

    // Validate the key binding
    if (!m_keybindingManager->ValidateBinding(m_keyToRebind, key)) {
        std::cout << "Invalid key binding: " << m_keybindingManager->GetKeyName(key) << std::endl;
        CancelKeyRebinding();
        return;
    }

    // Apply the binding
    bool success = false;
    if (m_rebindingPrimary) {
        success = m_keybindingManager->SetPrimaryKey(m_keyToRebind, key);
    } else {
        success = m_keybindingManager->SetAlternativeKey(m_keyToRebind, key);
    }

    if (success) {
        std::cout << "Successfully bound " << m_keybindingManager->GetKeyName(key)
                  << " to " << m_keybindingManager->GetActionDisplayName(m_keyToRebind) << std::endl;
    } else {
        std::cout << "Failed to bind key" << std::endl;
    }

    m_waitingForKey = false;
    m_keyToRebind = GameAction::ACTION_COUNT;
}

void OptionsState::DrawKeybindings() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    // Layout constants
    const int titleY = 80;
    const int startY = 120;
    const int lineHeight = 22;
    const int leftMargin = 30;
    const int actionColumnWidth = 220;
    const int bindingColumnX = leftMargin + actionColumnWidth;
    const int statusColumnX = bindingColumnX + 200;

    // Draw title
    BitmapFont::DrawText(renderer, "KEYBINDING CONFIGURATION", leftMargin, titleY, 2, Color(255, 255, 100, 255));

    // Draw column headers
    BitmapFont::DrawText(renderer, "Action", leftMargin, startY - 25, 1, Color(150, 150, 150, 255));
    BitmapFont::DrawText(renderer, "Keys", bindingColumnX, startY - 25, 1, Color(150, 150, 150, 255));

    // Draw scroll indicator if needed
    if (m_configurableActions.size() > static_cast<size_t>(m_maxVisibleKeybindings)) {
        int totalActions = static_cast<int>(m_configurableActions.size());
        int currentPage = (m_selectedKeybinding / m_maxVisibleKeybindings) + 1;
        int totalPages = (totalActions + m_maxVisibleKeybindings - 1) / m_maxVisibleKeybindings;

        std::string scrollInfo = "Page " + std::to_string(currentPage) + "/" + std::to_string(totalPages);
        BitmapFont::DrawText(renderer, scrollInfo, 600, titleY, 1, Color(150, 150, 150, 255));
    }

    // Draw visible keybindings
    int visibleCount = 0;
    for (size_t i = 0; i < m_configurableActions.size() && visibleCount < m_maxVisibleKeybindings; ++i) {
        int actionIndex = static_cast<int>(i);

        // Skip items that are scrolled out of view
        if (actionIndex < m_keybindingScrollOffset) continue;

        int y = startY + visibleCount * lineHeight;
        bool isSelected = (actionIndex == m_selectedKeybinding);

        GameAction action = m_configurableActions[i];
        const KeyBinding& binding = m_keybindingManager->GetBinding(action);

        Color textColor = isSelected && m_showSelection ?
            Color(255, 255, 100, 255) : Color(200, 200, 200, 255);

        // Selection indicator
        if (isSelected && m_showSelection) {
            BitmapFont::DrawText(renderer, ">", leftMargin - 20, y, 1, Color(255, 255, 100, 255));
        }

        // Action name (truncated if too long)
        std::string actionText = m_keybindingManager->GetActionDisplayName(action);
        if (actionText.length() > 25) {
            actionText = actionText.substr(0, 22) + "...";
        }
        BitmapFont::DrawText(renderer, actionText, leftMargin, y, 1, textColor);

        // Key bindings (formatted for better readability)
        std::string bindingText = GetKeybindingDisplayText(binding);
        if (bindingText.length() > 20) {
            bindingText = bindingText.substr(0, 17) + "...";
        }
        BitmapFont::DrawText(renderer, bindingText, bindingColumnX, y, 1, textColor);

        // Status indicator
        if (m_waitingForKey && m_keyToRebind == action) {
            std::string waitText = m_rebindingPrimary ? "[PRIMARY]" : "[ALT]";
            BitmapFont::DrawText(renderer, waitText, statusColumnX, y, 1, Color(255, 100, 100, 255));
        } else if (isSelected) {
            BitmapFont::DrawText(renderer, "ENTER:Primary  RIGHT:Alt", statusColumnX, y, 1, Color(100, 255, 100, 255));
        }

        visibleCount++;
    }

    // Draw separator line
    int separatorY = startY + m_maxVisibleKeybindings * lineHeight + 10;
    for (int x = leftMargin; x < 700; x += 10) {
        BitmapFont::DrawText(renderer, "-", x, separatorY, 1, Color(100, 100, 100, 255));
    }
}

void OptionsState::SaveKeybindings() {
    if (m_keybindingManager->SaveToConfig("assets/config/keybindings.ini")) {
        std::cout << "Keybindings saved successfully" << std::endl;
    } else {
        std::cout << "Failed to save keybindings" << std::endl;
    }
}

std::string OptionsState::GetVolumeDisplayText(float volume) const {
    int volumePercent = static_cast<int>(volume * 100);
    return ": " + std::to_string(volumePercent) + "%";
}

std::string OptionsState::GetKeybindingDisplayText(const KeyBinding& binding) const {
    std::string text;

    // Get primary key name
    std::string primaryName;
    if (binding.primaryKey != SDL_SCANCODE_UNKNOWN) {
        primaryName = m_keybindingManager->GetKeyName(binding.primaryKey);
        // Shorten common key names for better display
        if (primaryName == "Left") primaryName = "←";
        else if (primaryName == "Right") primaryName = "→";
        else if (primaryName == "Up") primaryName = "↑";
        else if (primaryName == "Down") primaryName = "↓";
        else if (primaryName == "Return") primaryName = "Enter";
        else if (primaryName == "Space") primaryName = "Space";
    } else {
        primaryName = "None";
    }

    text += primaryName;

    // Get alternative key name
    if (binding.alternativeKey != SDL_SCANCODE_UNKNOWN) {
        std::string altName = m_keybindingManager->GetKeyName(binding.alternativeKey);
        // Shorten common key names
        if (altName == "Left") altName = "←";
        else if (altName == "Right") altName = "→";
        else if (altName == "Up") altName = "↑";
        else if (altName == "Down") altName = "↓";
        else if (altName == "Return") altName = "Enter";
        else if (altName == "Space") altName = "Space";

        text += " / " + altName;
    }

    return text;
}

void OptionsState::CalculateKeybindingLayout() {
    // Calculate how many keybindings can fit on screen
    const int titleHeight = 120;  // Space for title and headers
    const int instructionHeight = 80;  // Space for instructions at bottom
    const int lineHeight = 22;
    const int screenHeight = 600;  // Assuming 600px screen height

    int availableHeight = screenHeight - titleHeight - instructionHeight;
    m_maxVisibleKeybindings = std::max(1, availableHeight / lineHeight);

    std::cout << "Calculated max visible keybindings: " << m_maxVisibleKeybindings << std::endl;
}

void OptionsState::UpdateKeybindingScroll() {
    if (m_configurableActions.empty() || m_maxVisibleKeybindings <= 0) return;

    // Ensure selected item is visible
    if (m_selectedKeybinding < m_keybindingScrollOffset) {
        // Scroll up
        m_keybindingScrollOffset = m_selectedKeybinding;
    } else if (m_selectedKeybinding >= m_keybindingScrollOffset + m_maxVisibleKeybindings) {
        // Scroll down
        m_keybindingScrollOffset = m_selectedKeybinding - m_maxVisibleKeybindings + 1;
    }

    // Clamp scroll offset
    int maxOffset = std::max(0, static_cast<int>(m_configurableActions.size()) - m_maxVisibleKeybindings);
    m_keybindingScrollOffset = std::max(0, std::min(m_keybindingScrollOffset, maxOffset));
}

bool OptionsState::IsKeybindingVisible(int index) const {
    return index >= m_keybindingScrollOffset &&
           index < m_keybindingScrollOffset + m_maxVisibleKeybindings;
}