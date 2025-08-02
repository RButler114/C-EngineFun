#include "Game/GameOverState.h"
#include "Game/GameStateManager.h"
#include "Engine/Renderer.h"
#include "Engine/InputManager.h"
#include "Engine/Engine.h"
#include "Engine/BitmapFont.h"
#include <iostream>

GameOverState::GameOverState()
    : GameState(GameStateType::GAME_OVER, "GameOver")
    , m_finalScore(0)
    , m_displayTimer(0.0f)
    , m_showRestartPrompt(false) {
}

void GameOverState::OnEnter() {
    std::cout << "Entering Game Over State" << std::endl;
    m_displayTimer = 0.0f;
    m_showRestartPrompt = false;
}

void GameOverState::OnExit() {
    std::cout << "Exiting Game Over State" << std::endl;
}

void GameOverState::Update(float deltaTime) {
    m_displayTimer += deltaTime;
    
    // Show restart prompt after 2 seconds
    if (m_displayTimer > 2.0f) {
        m_showRestartPrompt = true;
    }
    
    HandleInput();
}

void GameOverState::Render() {
    auto* renderer = GetRenderer();
    if (!renderer) return;
    
    DrawGameOverScreen();
    DrawScore();
    
    if (m_showRestartPrompt) {
        DrawRestartPrompt();
    }
}

void GameOverState::HandleInput() {
    auto* input = GetInputManager();
    if (!input) return;
    
    if (m_showRestartPrompt) {
        // Restart game
        if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN) || input->IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
            std::cout << "Restarting game..." << std::endl;
            if (GetStateManager()) {
                GetStateManager()->ChangeState(GameStateType::PLAYING);
            }
        }

        // Return to menu
        if (input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || input->IsKeyJustPressed(SDL_SCANCODE_M)) {
            std::cout << "Returning to menu..." << std::endl;
            if (GetStateManager()) {
                GetStateManager()->ChangeState(GameStateType::MENU);
            }
        }
    }
    
    // Quick quit
    if (input->IsKeyJustPressed(SDL_SCANCODE_Q)) {
        GetEngine()->Quit();
    }
}

void GameOverState::DrawGameOverScreen() {
    auto* renderer = GetRenderer();
    
    // Draw dark background
    renderer->DrawRectangle(Rectangle(0, 0, 800, 600), Color(20, 20, 20, 255), true);
    
    // Draw "GAME OVER" text using bitmap font
    const char* gameOverText = "GAME OVER";
    int textWidth = strlen(gameOverText) * 6 * 6; // 6 pixels per char * 6 scale
    int startX = (800 - textWidth) / 2;
    int textY = 200;

    BitmapFont::DrawText(renderer, gameOverText, startX, textY, 6, Color(255, 0, 0, 255)); // Large red text
}

void GameOverState::DrawScore() {
    auto* renderer = GetRenderer();
    
    // Draw final score using bitmap font
    std::string scoreText = "FINAL SCORE: " + std::to_string(m_finalScore);
    int textWidth = scoreText.length() * 6 * 3; // 6 pixels per char * 3 scale
    int startX = (800 - textWidth) / 2;
    int textY = 300;

    BitmapFont::DrawText(renderer, scoreText, startX, textY, 3, Color(255, 255, 0, 255)); // Yellow text
}

void GameOverState::DrawRestartPrompt() {
    auto* renderer = GetRenderer();
    
    // Draw restart instructions using bitmap font
    const char* restartText = "PRESS ENTER TO PLAY AGAIN";
    int textWidth = strlen(restartText) * 6 * 2; // 6 pixels per char * 2 scale
    int startX = (800 - textWidth) / 2;
    int textY = 400;

    BitmapFont::DrawText(renderer, restartText, startX, textY, 2, Color(255, 255, 255, 255)); // White text

    // Draw menu instructions using bitmap font
    const char* menuText = "PRESS ESC FOR MENU - Q TO QUIT";
    int menuWidth = strlen(menuText) * 6 * 1; // 6 pixels per char * 1 scale
    int menuX = (800 - menuWidth) / 2;
    int menuY = 450;

    BitmapFont::DrawText(renderer, menuText, menuX, menuY, 1, Color(180, 180, 180, 255)); // Gray text
}
