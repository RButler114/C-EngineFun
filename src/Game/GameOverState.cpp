/**
 * @file GameOverState.cpp
 * @brief Implementation of game over screen with score display and restart options
 * @author Ryan Butler
 * @date 2025
 */

#include "Game/GameOverState.h"
#include "Game/GameStateManager.h"
#include "Engine/Renderer.h"
#include "Engine/InputManager.h"
#include "Engine/Engine.h"
#include "Engine/BitmapFont.h"
#include <iostream>

/**
 * @brief Constructor - initializes game over state with default values
 *
 * Sets up the game over state with:
 * - Zero final score (will be set by calling state)
 * - Reset display timer for animation sequence
 * - Hidden restart prompt initially
 *
 * @note Score should be set via SetScore() before transitioning to this state
 */
GameOverState::GameOverState()
    : GameState(GameStateType::GAME_OVER, "GameOver")
    , m_finalScore(0)           // Will be set by previous state
    , m_displayTimer(0.0f)      // Controls animation timing
    , m_showRestartPrompt(false) // Hidden until timer expires
{
}

/**
 * @brief Initialize game over screen when becoming active
 *
 * Called when transitioning to game over state. Resets all timing
 * and display variables to ensure consistent presentation.
 *
 * @note Final score should already be set via SetScore() before this
 * @note Display sequence: Game Over message → Score → Restart options
 */
void GameOverState::OnEnter() {
    std::cout << "💀 Game Over - Final Score: " << m_finalScore << std::endl;

    // Reset animation timing for consistent presentation
    m_displayTimer = 0.0f;      // Start timing sequence from beginning
    m_showRestartPrompt = false; // Hide restart options initially

    // TODO: Play game over sound effect here
    // if (GetEngine()->GetAudioManager()) {
    //     GetEngine()->GetAudioManager()->PlaySound("game_over");
    // }
}

/**
 * @brief Clean up when leaving game over state
 *
 * Called when transitioning away from game over state.
 * Currently minimal cleanup needed.
 */
void GameOverState::OnExit() {
    std::cout << "🔄 Leaving Game Over screen" << std::endl;
}

/**
 * @brief Update game over screen timing and animations
 *
 * Controls the display sequence timing:
 * - 0-2 seconds: Show "GAME OVER" and score
 * - 2+ seconds: Show restart prompt and accept input
 *
 * @param deltaTime Time elapsed since last frame in seconds
 *
 * @note Input is processed every frame but only acts after prompt appears
 * @note Display timer accumulates to control animation sequence
 */
void GameOverState::Update(float deltaTime) {
    // Accumulate time for controlling display sequence
    m_displayTimer += deltaTime;

    // Show restart prompt after 2 seconds of displaying score
    // This gives players time to see their final score before showing options
    if (m_displayTimer > 2.0f) {
        m_showRestartPrompt = true;
    }

    // Process input every frame (but HandleInput checks if prompt is visible)
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
