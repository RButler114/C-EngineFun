/**
 * @file CustomizationState.cpp
 * @brief Implementation of character customization screen
 * @author Ryan Butler
 * @date 2025
 */

#include "Game/CustomizationState.h"
#include "Engine/Engine.h"
#include "Engine/Renderer.h"
#include "Engine/BitmapFont.h"
#include "Engine/InputManager.h"
#include "Game/GameStateManager.h"
#include "Engine/ConfigSystem.h"
#include "Engine/AudioManager.h"

#include <SDL2/SDL.h>
#include <iostream>
#include <algorithm>

// Color constants are now defined in the header as static const int values

CustomizationState::CustomizationState()
    : GameState(GameStateType::CUSTOMIZATION, "Customization") {

    // Initialize categories in order
    m_categories = {
        CustomizationCategory::BASIC_INFO,
        CustomizationCategory::APPEARANCE,
        CustomizationCategory::ATTRIBUTES,
        CustomizationCategory::EQUIPMENT
    };
}

void CustomizationState::OnEnter() {
    std::cout << "Entering Customization State" << std::endl;

    // Use the global customization manager instance
    m_customizationManager = std::make_unique<CustomizationManager>();

    // Try to load from config, fall back to defaults if needed
    if (!m_customizationManager->LoadFromConfig("assets/config/customization.ini")) {
        std::cout << "Using default customization options" << std::endl;
    }

    // Also initialize the global instance
    CustomizationManager::GetInstance().InitializeDefaults();

    // Reset UI state
    m_currentMode = UIMode::CATEGORY_SELECTION;
    m_selectedCategoryIndex = 0;
    m_selectedGroupIndex = 0;
    m_selectedOptionIndex = 0;
    m_nameInput = m_customizationManager->GetPlayerCustomization().playerName;
    m_nameInputActive = false;

    // Load current category groups
    SelectCategory(0);

    // Load common UI sounds for consistent UX
    if (GetEngine()->GetAudioManager()) {
        auto* am = GetEngine()->GetAudioManager();
        am->LoadSound("menu_nav", "assets/music/clicking-interface-select-201946.mp3", SoundType::SOUND_EFFECT);
        am->LoadSound("menu_select", "assets/music/select-001-337218.mp3", SoundType::SOUND_EFFECT);
        am->LoadSound("menu_back", "assets/music/select-003-337609.mp3", SoundType::SOUND_EFFECT);
    }
}


void CustomizationState::OnExit() {
    std::cout << "Exiting Customization State" << std::endl;
    m_customizationManager.reset();
}

void CustomizationState::Update(float deltaTime) {
    // Update based on current UI mode
    switch (m_currentMode) {
        case UIMode::CATEGORY_SELECTION:
            UpdateCategorySelection();
            break;
        case UIMode::OPTION_SELECTION:
            UpdateOptionSelection();
            break;
        case UIMode::NAME_INPUT:
            UpdateNameInput();
            break;
        case UIMode::ATTRIBUTE_ADJUSTMENT:
            UpdateAttributeAdjustment();
            break;
        case UIMode::CONFIRMATION:
            UpdateConfirmation();
            break;
    }
}

void CustomizationState::Render() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    // Clear with background color
    renderer->Clear(Color(COLOR_BACKGROUND_R, COLOR_BACKGROUND_G, COLOR_BACKGROUND_B, 255));

    // Render title
    BitmapFont::DrawText(renderer, "CHARACTER CUSTOMIZATION", MARGIN, 20, 3,
                        Color(COLOR_ACCENT_R, COLOR_ACCENT_G, COLOR_ACCENT_B, 255));

    // Render based on current mode
    switch (m_currentMode) {
        case UIMode::CATEGORY_SELECTION:
            RenderCategorySelection();
            break;
        case UIMode::OPTION_SELECTION:
            RenderOptionSelection();
            break;
        case UIMode::NAME_INPUT:
            RenderNameInput();
            break;
        case UIMode::ATTRIBUTE_ADJUSTMENT:
            RenderAttributeAdjustment();
            break;
        case UIMode::CONFIRMATION:
            RenderConfirmation();
            break;
    }

    // Always render character preview and instructions
    RenderCharacterPreview();
    RenderInstructions();
}

void CustomizationState::HandleInput() {
    auto* input = GetInputManager();
    if (!input) return;

    // Global controls - use B for back navigation
    if (input->IsKeyJustPressed(SDL_SCANCODE_B)) {
        if (m_currentMode == UIMode::CATEGORY_SELECTION) {
            ReturnToMenu();
        } else {
            // Go back to previous mode
            if (m_currentMode == UIMode::NAME_INPUT || m_currentMode == UIMode::OPTION_SELECTION) {
                m_currentMode = UIMode::CATEGORY_SELECTION;
            } else if (m_currentMode == UIMode::CONFIRMATION) {
                m_currentMode = UIMode::CATEGORY_SELECTION;
            }
            if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_back", 0.9f);

        }
        return;
    }

    // Escape key only quits from main category selection
    if (input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        if (m_currentMode == UIMode::CATEGORY_SELECTION) {
            ReturnToMenu();
        }
        return;
    }

    // Mode-specific input handling
    switch (m_currentMode) {
        case UIMode::CATEGORY_SELECTION:
            HandleCategoryInput();
            break;
        case UIMode::OPTION_SELECTION:
            HandleOptionInput();
            break;
        case UIMode::NAME_INPUT:
            HandleNameInputKeys();
            break;
        case UIMode::ATTRIBUTE_ADJUSTMENT:
            HandleAttributeInput();
            break;
        case UIMode::CONFIRMATION:
            HandleConfirmationInput();
            break;
    }
}

void CustomizationState::UpdateCategorySelection() {
    // Nothing special to update in category selection mode
}

void CustomizationState::UpdateOptionSelection() {
    // Nothing special to update in option selection mode
}

void CustomizationState::UpdateNameInput() {
    // Update player name from input
    if (!m_nameInput.empty()) {
        m_customizationManager->GetPlayerCustomization().playerName = m_nameInput;
    }
}

void CustomizationState::UpdateAttributeAdjustment() {
    // Nothing special to update in attribute adjustment mode
}

void CustomizationState::UpdateConfirmation() {
    // Nothing special to update in confirmation mode
}

void CustomizationState::RenderCategorySelection() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    int y = TITLE_HEIGHT + MARGIN;

    BitmapFont::DrawText(renderer, "Select Category:", MARGIN, y, 2,
                        Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
    y += LINE_HEIGHT * 2;

    // Render category options
    const char* categoryNames[] = {"Basic Info", "Appearance", "Attributes", "Equipment"};

    for (int i = 0; i < static_cast<int>(m_categories.size()); ++i) {
        if (i == m_selectedCategoryIndex) {
            BitmapFont::DrawText(renderer, std::string("> ") + categoryNames[i], MARGIN, y, 2,
                               Color(COLOR_SELECTED_R, COLOR_SELECTED_G, COLOR_SELECTED_B, 255));
        } else {
            BitmapFont::DrawText(renderer, std::string("  ") + categoryNames[i], MARGIN, y, 2,
                               Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
        }
        y += LINE_HEIGHT;
    }
}

void CustomizationState::RenderOptionSelection() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    int y = TITLE_HEIGHT + MARGIN;

    if (m_currentCategoryGroups.empty()) return;

    // Show current category
    const char* categoryNames[] = {"Basic Info", "Appearance", "Attributes", "Equipment"};
    BitmapFont::DrawText(renderer, categoryNames[m_selectedCategoryIndex], MARGIN, y, 2,
                        Color(COLOR_ACCENT_R, COLOR_ACCENT_G, COLOR_ACCENT_B, 255));
    y += LINE_HEIGHT * 2;

    // Show current group
    if (m_selectedGroupIndex < static_cast<int>(m_currentCategoryGroups.size())) {
        CustomizationGroup* group = m_currentCategoryGroups[m_selectedGroupIndex];

        BitmapFont::DrawText(renderer, group->displayName + ":", MARGIN, y, 2,
                           Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
        y += LINE_HEIGHT;

        // Show options
        for (int i = 0; i < static_cast<int>(group->options.size()); ++i) {
            const auto& option = group->options[i];

            if (i == m_selectedOptionIndex) {
                BitmapFont::DrawText(renderer, "> " + option.displayName, MARGIN + 20, y, 1,
                                   Color(COLOR_SELECTED_R, COLOR_SELECTED_G, COLOR_SELECTED_B, 255));
            } else if (i == group->selectedIndex) {
                BitmapFont::DrawText(renderer, "* " + option.displayName, MARGIN + 20, y, 1,
                                   Color(COLOR_ACCENT_R, COLOR_ACCENT_G, COLOR_ACCENT_B, 255));
            } else {
                BitmapFont::DrawText(renderer, "  " + option.displayName, MARGIN + 20, y, 1,
                                   Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
            }
            y += LINE_HEIGHT;
        }
    }
}

void CustomizationState::RenderNameInput() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    int y = TITLE_HEIGHT + MARGIN;

    BitmapFont::DrawText(renderer, "Enter Character Name:", MARGIN, y, 2,
                        Color(COLOR_ACCENT_R, COLOR_ACCENT_G, COLOR_ACCENT_B, 255));
    y += LINE_HEIGHT * 2;

    // Show current name with cursor
    std::string displayName = m_nameInput;
    if (m_nameInputActive) {
        displayName += "_";  // Simple cursor
    }

    BitmapFont::DrawText(renderer, displayName, MARGIN, y, 2,
                        Color(COLOR_SELECTED_R, COLOR_SELECTED_G, COLOR_SELECTED_B, 255));
}

void CustomizationState::RenderAttributeAdjustment() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    int y = TITLE_HEIGHT + MARGIN;

    BitmapFont::DrawText(renderer, "Attribute Distribution:", MARGIN, y, 2,
                        Color(COLOR_ACCENT_R, COLOR_ACCENT_G, COLOR_ACCENT_B, 255));
    y += LINE_HEIGHT * 2;

    const auto& customization = m_customizationManager->GetPlayerCustomization();

    BitmapFont::DrawText(renderer, "Strength: " + std::to_string(static_cast<int>(customization.strength)), MARGIN, y, 1,
                        Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
    y += LINE_HEIGHT;
    BitmapFont::DrawText(renderer, "Agility: " + std::to_string(static_cast<int>(customization.agility)), MARGIN, y, 1,
                        Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
    y += LINE_HEIGHT;
    BitmapFont::DrawText(renderer, "Intelligence: " + std::to_string(static_cast<int>(customization.intelligence)), MARGIN, y, 1,
                        Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
    y += LINE_HEIGHT;
    BitmapFont::DrawText(renderer, "Vitality: " + std::to_string(static_cast<int>(customization.vitality)), MARGIN, y, 1,
                        Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
    y += LINE_HEIGHT * 2;

    BitmapFont::DrawText(renderer, "Available Points: " + std::to_string(customization.availablePoints), MARGIN, y, 1,
                        Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
}

void CustomizationState::RenderConfirmation() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    int y = TITLE_HEIGHT + MARGIN;

    BitmapFont::DrawText(renderer, "Confirm Character:", MARGIN, y, 2,
                        Color(COLOR_ACCENT_R, COLOR_ACCENT_G, COLOR_ACCENT_B, 255));
    y += LINE_HEIGHT * 2;

    const auto& customization = m_customizationManager->GetPlayerCustomization();

    BitmapFont::DrawText(renderer, "Name: " + customization.playerName, MARGIN, y, 1,
                        Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
    y += LINE_HEIGHT;
    BitmapFont::DrawText(renderer, "Class: " + customization.characterClass, MARGIN, y, 1,
                        Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
    y += LINE_HEIGHT * 2;

    BitmapFont::DrawText(renderer, "Press ENTER to start game", MARGIN, y, 1,
                        Color(COLOR_SELECTED_R, COLOR_SELECTED_G, COLOR_SELECTED_B, 255));
    y += LINE_HEIGHT;
    BitmapFont::DrawText(renderer, "Press B to go back", MARGIN, y, 1,
                        Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
}

void CustomizationState::RenderCharacterPreview() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    // Simple character preview area
    int previewX = SCREEN_WIDTH - 200;
    int previewY = TITLE_HEIGHT + MARGIN;
    int previewWidth = 150;
    int previewHeight = 200;

    // Draw preview background
    renderer->DrawRectangle(Rectangle(previewX, previewY, previewWidth, previewHeight),
                           Color(COLOR_PREVIEW_BG_R, COLOR_PREVIEW_BG_G, COLOR_PREVIEW_BG_B, 255), true);

    // Draw simple character representation
    BitmapFont::DrawText(renderer, "Preview", previewX + 10, previewY + 10, 1,
                        Color(COLOR_ACCENT_R, COLOR_ACCENT_G, COLOR_ACCENT_B, 255));

    // TODO: Render actual character sprite based on customization
    renderer->DrawRectangle(Rectangle(previewX + 60, previewY + 80, 30, 60),
                           Color(COLOR_SELECTED_R, COLOR_SELECTED_G, COLOR_SELECTED_B, 255), true);  // Simple body
    renderer->DrawRectangle(Rectangle(previewX + 65, previewY + 60, 20, 20),
                           Color(COLOR_SELECTED_R, COLOR_SELECTED_G, COLOR_SELECTED_B, 255), true);  // Simple head
}

void CustomizationState::RenderInstructions() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    int y = SCREEN_HEIGHT - 80;

    BitmapFont::DrawText(renderer, "Controls: Arrow Keys - Navigate, Enter - Select, B - Back", MARGIN, y, 1,
                        Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
    y += LINE_HEIGHT;
    BitmapFont::DrawText(renderer, "C - Confirm and Start Game, ESC - Return to Menu", MARGIN, y, 1,
                        Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
}

void CustomizationState::HandleCategoryInput() {
    auto* input = GetInputManager();
    if (!input) return;

    static bool upWas=false, downWas=false;
    bool upNow = input->IsKeyPressed(SDL_SCANCODE_UP);
    bool downNow = input->IsKeyPressed(SDL_SCANCODE_DOWN);

    if (upNow && !upWas) {
        m_selectedCategoryIndex = std::max(0, m_selectedCategoryIndex - 1);
        if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
    } else if (downNow && !downWas) {
        m_selectedCategoryIndex = std::min(static_cast<int>(m_categories.size()) - 1, m_selectedCategoryIndex + 1);
        if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
    } else if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN)) {
        SelectCategory(m_selectedCategoryIndex);
        // Special handling for name input
        if (m_categories[m_selectedCategoryIndex] == CustomizationCategory::BASIC_INFO) {
            m_currentMode = UIMode::NAME_INPUT;
            m_nameInputActive = true;
        } else {
            m_currentMode = UIMode::OPTION_SELECTION;
        }
        if (GetEngine()->GetAudioManager()) { GetEngine()->GetAudioManager()->PlaySound("menu_select", 0.9f); }
    } else if (input->IsKeyJustPressed(SDL_SCANCODE_C)) {
        // Quick confirm - go to confirmation screen
        m_currentMode = UIMode::CONFIRMATION;
    }

    upWas = upNow; downWas = downNow;
}

void CustomizationState::HandleOptionInput() {
    auto* input = GetInputManager();
    if (!input) return;

    if (m_currentCategoryGroups.empty()) return;

    static bool upWas=false, downWas=false, leftWas=false, rightWas=false;
    bool upNow = input->IsKeyPressed(SDL_SCANCODE_UP);
    bool downNow = input->IsKeyPressed(SDL_SCANCODE_DOWN);
    bool leftNow = input->IsKeyPressed(SDL_SCANCODE_LEFT);
    bool rightNow = input->IsKeyPressed(SDL_SCANCODE_RIGHT);

    if (upNow && !upWas) {
        if (m_selectedGroupIndex < static_cast<int>(m_currentCategoryGroups.size())) {
            m_selectedOptionIndex = std::max(0, m_selectedOptionIndex - 1);
            if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
        }
    } else if (downNow && !downWas) {
        if (m_selectedGroupIndex < static_cast<int>(m_currentCategoryGroups.size())) {
            auto* group = m_currentCategoryGroups[m_selectedGroupIndex];
            m_selectedOptionIndex = std::min(static_cast<int>(group->options.size()) - 1, m_selectedOptionIndex + 1);
            if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
        }
    } else if (leftNow && !leftWas) {
        m_selectedGroupIndex = std::max(0, m_selectedGroupIndex - 1);
        m_selectedOptionIndex = 0;
        if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
    } else if (rightNow && !rightWas) {
        m_selectedGroupIndex = std::min(static_cast<int>(m_currentCategoryGroups.size()) - 1, m_selectedGroupIndex + 1);
        m_selectedOptionIndex = 0;
        if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
    } else if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN)) {
        ApplyCurrentSelection();
        if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_select", 0.9f);
    }

    upWas=upNow; downWas=downNow; leftWas=leftNow; rightWas=rightNow;
}

void CustomizationState::HandleNameInputKeys() {
    auto* input = GetInputManager();
    if (!input) return;

    if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN)) {
        m_nameInputActive = false;
        m_currentMode = UIMode::CATEGORY_SELECTION;
        return;
    }

    // Handle text input (simplified)
    if (input->IsKeyJustPressed(SDL_SCANCODE_BACKSPACE) && !m_nameInput.empty()) {
        m_nameInput.pop_back();
    }

    // Add basic character input (A-Z, space)
    for (int scancode = SDL_SCANCODE_A; scancode <= SDL_SCANCODE_Z; ++scancode) {
        if (input->IsKeyJustPressed(static_cast<SDL_Scancode>(scancode))) {
            if (m_nameInput.length() < 20) {  // Limit name length
                char c = 'A' + (scancode - SDL_SCANCODE_A);
                if (!input->IsKeyPressed(SDL_SCANCODE_LSHIFT) && !input->IsKeyPressed(SDL_SCANCODE_RSHIFT)) {
                    c = c - 'A' + 'a';  // Convert to lowercase
                }
                m_nameInput += c;
            }
        }
    }

    if (input->IsKeyJustPressed(SDL_SCANCODE_SPACE) && m_nameInput.length() < 20) {
        m_nameInput += ' ';
    }
}

void CustomizationState::HandleAttributeInput() {
    auto* input = GetInputManager();
    if (!input) return;

    if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN)) {
        m_currentMode = UIMode::CATEGORY_SELECTION;
    }

    // TODO: Implement attribute point adjustment
}

void CustomizationState::HandleConfirmationInput() {
    auto* input = GetInputManager();
    if (!input) return;

    if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN)) {
        if (GetEngine()->GetAudioManager()) {
            GetEngine()->GetAudioManager()->LoadSound("game_start", "assets/music/game-start-6104.mp3", SoundType::SOUND_EFFECT);
            GetEngine()->GetAudioManager()->PlaySound("game_start", 1.0f);
        }
        StartGame();
    }
}

void CustomizationState::SelectCategory(int index) {
    if (index < 0 || index >= static_cast<int>(m_categories.size())) return;

    m_selectedCategoryIndex = index;
    m_selectedGroupIndex = 0;
    m_selectedOptionIndex = 0;

    // Load groups for this category
    m_currentCategoryGroups = m_customizationManager->GetGroupsByCategory(m_categories[index]);
}

void CustomizationState::SelectOption(int groupIndex, int optionIndex) {
    if (groupIndex < 0 || groupIndex >= static_cast<int>(m_currentCategoryGroups.size())) return;

    CustomizationGroup* group = m_currentCategoryGroups[groupIndex];
    if (optionIndex < 0 || optionIndex >= static_cast<int>(group->options.size())) return;

    m_customizationManager->ApplyGroupSelection(group->id, optionIndex);
}

void CustomizationState::ApplyCurrentSelection() {
    if (m_selectedGroupIndex < static_cast<int>(m_currentCategoryGroups.size())) {
        SelectOption(m_selectedGroupIndex, m_selectedOptionIndex);
    }
}

void CustomizationState::StartGame() {
    std::cout << "Starting game with customized character..." << std::endl;

    // Apply all customizations to local manager
    m_customizationManager->ApplyCustomizationToPlayerData();

    // Copy customization data to the global instance
    CustomizationManager::GetInstance().GetPlayerCustomization() = m_customizationManager->GetPlayerCustomization();
    // Persist chosen sprite/skin into characters.ini before leaving
    {
        const auto& pc = m_customizationManager->GetPlayerCustomization();
        ConfigManager chars;
        if (chars.LoadFromFile("assets/config/characters.ini")) {
            // write bare filename to match existing convention
            std::string filename = pc.spritePath;
            size_t slash = filename.find_last_of("/\\");
            if (slash != std::string::npos) filename = filename.substr(slash + 1);
            chars.Set("player", "sprite_path", ConfigValue(filename));
            if (chars.SaveToFile("assets/config/characters.ini")) {
                std::cout << "💾 Saved player sprite to characters.ini: " << filename << std::endl;
            } else {
                std::cout << "❌ Failed to save characters.ini" << std::endl;
            }
        } else {
            std::cout << "⚠️ Could not load characters.ini to save sprite selection" << std::endl;
        }
    }

    CustomizationManager::GetInstance().ApplyCustomizationToPlayerData();

    // Validate customization
    if (!m_customizationManager->ValidateCustomization()) {
        std::cout << "Invalid customization - cannot start game" << std::endl;
        return;
    }

    std::cout << "Customization applied globally for: "
              << CustomizationManager::GetInstance().GetPlayerCustomization().playerName << std::endl;

    // Transition to playing state
    if (GetStateManager()) {
        GetStateManager()->ChangeState(GameStateType::PLAYING);
    }
}

void CustomizationState::ReturnToMenu() {
    std::cout << "Returning to menu..." << std::endl;
    if (GetStateManager()) {
        GetStateManager()->ChangeState(GameStateType::MENU);
    }
}
