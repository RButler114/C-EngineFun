#include "Game/CreditsState.h"
#include "Engine/Engine.h"
#include "Engine/Renderer.h"
#include "Engine/InputManager.h"
#include "Engine/BitmapFont.h"
#include "Game/GameStateManager.h"
#include "Engine/AudioManager.h"

#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

CreditsState::CreditsState()
    : GameState(GameStateType::CREDITS, "Credits")
    , m_scrollY(0.0f)
    , m_scrollSpeed(60.0f) // 60 px/sec
{
    // Default placeholder lines; overridden by assets/config/credits.txt if present
    m_lines = {
        "Everharvest Voyager V",
        "",
        "CREDITS",
        "",
        "Programming: PLACEHOLDER",
        "Design: PLACEHOLDER",
        "Art & Sprites: PLACEHOLDER",
        "Audio: PLACEHOLDER",
        "",
        "Special Thanks:",
        "- PLACEHOLDER",
        "",
        "Licenses and Attributions:",
        "- PLACEHOLDER",
        "",
        "Thank you for playing!"
    };
}

void CreditsState::LoadCreditsFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "CreditsState: could not open " << path << ", using placeholders" << std::endl;
        return; // keep existing placeholder m_lines
    }

    std::vector<std::string> loaded;
    std::string line;
    while (std::getline(file, line)) {
        // Normalize CRLF and trim trailing CR if present
        if (!line.empty() && line.back() == '\r') line.pop_back();
        loaded.push_back(line);
    }
    file.close();

    if (!loaded.empty()) {
        m_lines = std::move(loaded);
    }
}


void CreditsState::OnEnter() {
    std::cout << "📜 Entering Credits" << std::endl;
    m_scrollY = 0.0f;
    // Try loading from config file
    LoadCreditsFromFile("assets/config/credits.txt");
    // Ensure menu UI sounds available for back navigation
    if (GetEngine()->GetAudioManager()) {
        auto* am = GetEngine()->GetAudioManager();
        am->LoadSound("menu_back", "assets/music/select-003-337609.mp3", SoundType::SOUND_EFFECT);
    }
}

void CreditsState::OnExit() {
    std::cout << "📜 Exiting Credits" << std::endl;
}

void CreditsState::Update(float deltaTime) {
    HandleInput();
    // Slow auto-scroll upward
    m_scrollY += 20.0f * deltaTime;
}

void CreditsState::Render() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    DrawBackground();
    DrawTitle();
    DrawCreditsText();
    DrawInstructions();
}

void CreditsState::HandleInput() {
    auto* input = GetInputManager();
    if (!input) return;

    if (input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || input->IsKeyJustPressed(SDL_SCANCODE_B)) {
        if (GetEngine()->GetAudioManager()) {
            GetEngine()->GetAudioManager()->PlaySound("menu_back", 0.9f);
        }
        if (GetStateManager()) {
            GetStateManager()->PopState(); // return to previous (menu)
        }
        return;
    }

    // Manual scroll
    if (input->IsKeyPressed(SDL_SCANCODE_UP) || input->IsKeyPressed(SDL_SCANCODE_W)) {
        m_scrollY -= m_scrollSpeed * 0.016f; // approx per-frame
    }
    if (input->IsKeyPressed(SDL_SCANCODE_DOWN) || input->IsKeyPressed(SDL_SCANCODE_S)) {
        m_scrollY += m_scrollSpeed * 0.016f;
    }
}

void CreditsState::DrawBackground() {
    auto* renderer = GetRenderer();
    // Simple dark gradient similar to OptionsState
    for (int y = 0; y < 600; y++) {
        int intensity = 10 + (y * 25) / 600;
        Color bgColor(intensity, intensity, intensity + 20, 255);
        renderer->DrawLine(0, y, 800, y, bgColor);
    }
}

void CreditsState::DrawTitle() {
    auto* renderer = GetRenderer();
    std::string title = "CREDITS";
    int titleWidth = (int)title.length() * 6 * 3;
    int startX = (800 - titleWidth) / 2;
    BitmapFont::DrawText(renderer, title, startX, 60, 3, Color(255, 215, 0, 255));
}

void CreditsState::DrawCreditsText() {
    auto* renderer = GetRenderer();
    int lineHeight = 18; // 6px * 3 scale
    int startY = 120; // below title

    for (size_t i = 0; i < m_lines.size(); ++i) {
        int y = startY + (int)i * lineHeight - (int)m_scrollY;
        if (y < -lineHeight || y > 600 + lineHeight) continue; // cull

        int scale = (m_lines[i] == "Everharvest Voyager V" || m_lines[i] == "CREDITS") ? 3 : 2;
        Color color = (scale == 3) ? Color(255, 255, 255, 255) : Color(220, 220, 220, 255);
        int width = (int)m_lines[i].length() * 6 * scale;
        int x = (800 - width) / 2;
        BitmapFont::DrawText(renderer, m_lines[i], x, y, scale, color);
    }
}

void CreditsState::DrawInstructions() {
    auto* renderer = GetRenderer();
    std::string instructions = "ESC/B: BACK  |  UP/DOWN: SCROLL";
    int w = (int)instructions.length() * 6 * 1;
    int x = (800 - w) / 2;
    BitmapFont::DrawText(renderer, instructions, x, 560, 1, Color(200, 200, 200, 255));
}

