#include <cstring>

#include "Game/HighScoreState.h"
#include "Game/HighScoreManager.h"
#include "Game/GameStateManager.h"
#include "Engine/Renderer.h"
#include "Engine/InputManager.h"
#include "Engine/BitmapFont.h"
#include "Engine/AudioManager.h"
#include <iostream>
#include <algorithm>

HighScoreState::HighScoreState()
    : GameState(GameStateType::HIGH_SCORES, "HighScores")
    , m_blinkTimer(0.0f)
    , m_showHint(true) {
}

void HighScoreState::OnEnter() {
    m_scores.clear();
}

void HighScoreState::OnExit() {}

void HighScoreState::Update(float deltaTime) {
    m_blinkTimer += deltaTime;
    if (m_blinkTimer >= 0.5f) { m_showHint = !m_showHint; m_blinkTimer = 0.0f; }
    HandleInput();
}

void HighScoreState::Render() {
    auto* r = GetRenderer();
    if (!r) return;

    // Background
    r->DrawRectangle(Rectangle(0, 0, 800, 600), Color(15, 15, 25, 255), true);

    // Title
    std::string title = "HIGH SCORES";
    int titleWidth = static_cast<int>(title.size()) * 6 * 3;
    int titleX = (800 - titleWidth) / 2;
    BitmapFont::DrawText(r, title, titleX, 60, 3, Color(255, 215, 0, 255));

    DrawPanel();

    if (m_showHint) {
        BitmapFont::DrawText(r, "ESC/B: Back", 20, 560, 1, Color(200,200,200,255));
    }
}

void HighScoreState::DrawPanel() {
    auto* r = GetRenderer();

    int x = 160, y = 140, w = 480, h = 360;
    r->DrawRectangle(Rectangle(x, y, w, h), Color(0, 0, 0, 200), true);
    r->DrawRectangle(Rectangle(x, y, w, h), Color(255, 255, 255, 255), false);

    // Column headers
    BitmapFont::DrawText(r, "RANK", x + 20, y + 16, 2, Color(255,255,0,255));
    BitmapFont::DrawText(r, "NAME", x + 100, y + 16, 2, Color(255,255,0,255));
    BitmapFont::DrawText(r, "SCORE", x + 260, y + 16, 2, Color(255,255,0,255));
    BitmapFont::DrawText(r, "DATE", x + 360, y + 16, 2, Color(255,255,0,255));

    // Load entries with names
    std::vector<HighScoreManager::Entry> entries;
    HighScoreManager::LoadEntries(entries);

    // Entries
    int startY = y + 50;
    int lineH = 24;
    int max = std::min(10, static_cast<int>(entries.size()));
    for (int i = 0; i < max; ++i) {
        std::string rank = std::to_string(i + 1) + ".";
        BitmapFont::DrawText(r, rank, x + 20, startY + i * lineH, 2, Color(220,220,220,255));
        BitmapFont::DrawText(r, entries[i].name, x + 100, startY + i * lineH, 2, Color(220,220,220,255));
        BitmapFont::DrawText(r, std::to_string(entries[i].score), x + 260, startY + i * lineH, 2, Color(220,220,220,255));
        BitmapFont::DrawText(r, entries[i].date.empty() ? std::string("-") : entries[i].date, x + 360, startY + i * lineH, 2, Color(200,200,200,255));
    }

    if (max == 0) {
        BitmapFont::DrawText(r, "No scores yet.", x + 20, y + 90, 2, Color(180,180,180,255));
    }
}

void HighScoreState::HandleInput() {
    auto* input = GetInputManager();
    if (!input) return;

    if (input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || input->IsKeyJustPressed(SDL_SCANCODE_B)) {
        if (GetStateManager()) GetStateManager()->PopState();
    }
}

