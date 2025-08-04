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
#include <iostream>
#include <algorithm>

OptionsState::OptionsState()
    : GameState(GameStateType::OPTIONS, "Options")
    , m_selectedOption(0)
    , m_blinkTimer(0.0f)
    , m_showSelection(true)
    , m_musicVolume(0.5f)
    , m_soundVolume(0.7f) {

    // Initialize options list
    m_options = {
        "Music Volume",
        "Sound Volume",
        "Back to Menu"
    };
}

OptionsState::~OptionsState() = default;

void OptionsState::OnEnter() {
    std::cout << "Entering Options State" << std::endl;

    // Get current audio settings
    if (GetEngine()->GetAudioManager()) {
        m_musicVolume = GetEngine()->GetAudioManager()->GetMusicVolume();
        m_soundVolume = GetEngine()->GetAudioManager()->GetCategoryVolume(SoundType::SOUND_EFFECT);
    }

    // Reset selection
    m_selectedOption = 0;
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
    DrawOptions();
    DrawInstructions();
}

void OptionsState::HandleInput() {
    auto* input = GetInputManager();
    if (!input) return;

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

    // Back
    if (input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        GoBack();
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

    switch (m_selectedOption) {
        case 0: // Music Volume
            m_musicVolume = std::max(0.0f, m_musicVolume - 0.1f);
            audioManager->SetMusicVolume(m_musicVolume);
            break;
        case 1: // Sound Volume
            m_soundVolume = std::max(0.0f, m_soundVolume - 0.1f);
            audioManager->SetCategoryVolume(SoundType::SOUND_EFFECT, m_soundVolume);
            audioManager->PlaySound("menu_select", m_soundVolume);
            break;
    }
}

void OptionsState::AdjustRight() {
    auto* audioManager = GetEngine()->GetAudioManager();
    if (!audioManager) return;

    switch (m_selectedOption) {
        case 0: // Music Volume
            m_musicVolume = std::min(1.0f, m_musicVolume + 0.1f);
            audioManager->SetMusicVolume(m_musicVolume);
            break;
        case 1: // Sound Volume
            m_soundVolume = std::min(1.0f, m_soundVolume + 0.1f);
            audioManager->SetCategoryVolume(SoundType::SOUND_EFFECT, m_soundVolume);
            audioManager->PlaySound("menu_select", m_soundVolume);
            break;
    }
}

void OptionsState::SelectOption() {
    if (m_selectedOption == 2) { // Back to Menu
        GoBack();
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

        // Add value display for volume options
        if (i == 0) { // Music Volume
            int volumePercent = static_cast<int>(m_musicVolume * 100);
            displayText += ": " + std::to_string(volumePercent) + "%";
        } else if (i == 1) { // Sound Volume
            int volumePercent = static_cast<int>(m_soundVolume * 100);
            displayText += ": " + std::to_string(volumePercent) + "%";
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
    BitmapFont::DrawText(renderer, "ESCAPE: BACK TO MENU", 50, 560, 1, Color(150, 150, 150, 255));
}