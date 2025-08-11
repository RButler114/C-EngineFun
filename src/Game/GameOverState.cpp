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
#include "Game/HighScoreManager.h"
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

    // Determine if score qualifies for leaderboard; if so, collect initials
    m_collectingInitials = HighScoreManager::WouldQualify(m_finalScore);
    m_initials.clear();
    m_initialsRepeatCooldown = 0.0f;
    m_backspaceCooldown = 0.0f;
    m_initialsFeedbackTimer = 0.0f;

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
    // Update caret blink when collecting initials
    if (m_collectingInitials) {
        m_caretTimer += deltaTime;
        if (m_caretTimer >= 0.5f) { m_caretOn = !m_caretOn; m_caretTimer = 0.0f; }
        if (m_initialsRepeatCooldown > 0.0f) m_initialsRepeatCooldown -= deltaTime;
        if (m_backspaceCooldown > 0.0f) m_backspaceCooldown -= deltaTime;
        if (m_initialsFeedbackTimer > 0.0f) m_initialsFeedbackTimer -= deltaTime;
    }

}

void GameOverState::Render() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    DrawGameOverScreen();
    DrawScore();

    if (m_collectingInitials) {
        DrawInitialsPrompt();
    } else if (m_showRestartPrompt) {
        DrawRestartPrompt();
    }
}

void GameOverState::HandleInput() {
    auto* input = GetInputManager();
    if (!input) return;

    if (m_collectingInitials) {
        HandleInitialsInput();
        return;
    }

    // fall through to normal prompt handling below

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



void GameOverState::DrawInitialsPrompt() {
    auto* renderer = GetRenderer();

    // Centered panel with more breathing room
    int screenW = 800, screenH = 600; // Renderer doesn't expose size; using current target
    int w = 520, h = 200;
    int x = (screenW - w) / 2;
    int y = (screenH - h) / 2;
    int pad = 24;

    // Backdrop to focus attention
    renderer->DrawRectangle(Rectangle(0, 0, screenW, screenH), Color(0, 0, 0, 120), true);

    // Panel
    renderer->DrawRectangle(Rectangle(x, y, w, h), Color(0, 0, 0, 220), true);
    renderer->DrawRectangle(Rectangle(x, y, w, h), Color(255, 255, 255, 255), false);

    // Header
    BitmapFont::DrawText(renderer, "NEW HIGH SCORE", x + pad, y + pad, 2, Color(255, 215, 0, 255));
    BitmapFont::DrawText(renderer, "ENTER INITIALS:", x + pad, y + pad + 28, 2, Color(230, 230, 230, 255));

    // Input line with simple visual feedback (slight scale pulse when updated)
    std::string display = m_initials;
    if (m_caretOn && m_initials.size() < 3) display.push_back('_');
    int inputScale = (m_initialsFeedbackTimer > 0.0f) ? 4 : 3; // pulse up briefly
    BitmapFont::DrawText(renderer, display, x + pad, y + pad + 70, inputScale, Color(255, 255, 255, 255));

    // Hints (split across two lines to reduce clutter)
    BitmapFont::DrawText(renderer, "Type A-Z/0-9. BACKSPACE to erase.", x + pad, y + h - pad - 28, 1, Color(200,200,200,255));
    BitmapFont::DrawText(renderer, "ENTER to confirm, ESC to skip.",   x + pad, y + h - pad,      1, Color(200,200,200,255));
}


void GameOverState::DrawGameOverScreen() {
    auto* renderer = GetRenderer();

    // Draw dark background
    int screenW = 800, screenH = 600;
    renderer->DrawRectangle(Rectangle(0, 0, screenW, screenH), Color(20, 20, 20, 255), true);

    // Title area with generous top margin
    const char* gameOverText = "GAME OVER";
    int titleScale = 6;
    int textWidth = static_cast<int>(strlen(gameOverText)) * 6 * titleScale;
    int startX = (screenW - textWidth) / 2;
    int textY = 120; // was 200; move up to create space for score and prompts

    BitmapFont::DrawText(renderer, gameOverText, startX, textY, titleScale, Color(255, 80, 80, 255));
}

void GameOverState::DrawScore() {
    auto* renderer = GetRenderer();

    int screenW = 800;
    // Final score with breathing room below the title
    std::string scoreText = "FINAL SCORE";
    std::string scoreValue = std::to_string(m_finalScore);

    int labelScale = 2;
    int valueScale = 4;

    int labelWidth = static_cast<int>(scoreText.length()) * 6 * labelScale;
    int labelX = (screenW - labelWidth) / 2;
    int labelY = 200;

    BitmapFont::DrawText(renderer, scoreText, labelX, labelY, labelScale, Color(230, 230, 230, 255));

    int valueWidth = static_cast<int>(scoreValue.length()) * 6 * valueScale;
    int valueX = (screenW - valueWidth) / 2;
    int valueY = labelY + 40;
    BitmapFont::DrawText(renderer, scoreValue, valueX, valueY, valueScale, Color(255, 255, 0, 255));
}

void GameOverState::DrawRestartPrompt() {
    auto* renderer = GetRenderer();

    int screenW = 800, screenH = 600;
    int padY = 24;

    // Split prompts with spacing near bottom area
    const char* restartText = "PRESS ENTER TO PLAY AGAIN";
    int restartScale = 2;
    int restartWidth = static_cast<int>(strlen(restartText)) * 6 * restartScale;
    int restartX = (screenW - restartWidth) / 2;
    int restartY = screenH - 150;
    BitmapFont::DrawText(renderer, restartText, restartX, restartY, restartScale, Color(255, 255, 255, 255));

    const char* menuText = "ESC: MENU   Q: QUIT";
    int menuScale = 1;
    int menuWidth = static_cast<int>(strlen(menuText)) * 6 * menuScale;
    int menuX = (screenW - menuWidth) / 2;
    int menuY = restartY + 28 + padY;
    BitmapFont::DrawText(renderer, menuText, menuX, menuY, menuScale, Color(180, 180, 180, 255));
}

void GameOverState::HandleInitialsInput() {
    auto* input = GetInputManager();
    if (!input) return;

    // Optional small cooldown to avoid double-accept on first press (e.g., key repeat quirks)
    // Tunables (could be overridden earlier via config)
    const float repeatGuard = m_repeatGuard;
    const float backspaceGuard = m_backspaceGuard;

    // Accept A-Z letters; Backspace to delete; Enter to confirm
    for (int code = SDL_SCANCODE_A; code <= SDL_SCANCODE_Z; ++code) {
        if (input->IsKeyJustPressed(static_cast<SDL_Scancode>(code)) && m_initialsRepeatCooldown <= 0.0f) {
            if (m_initials.size() < 3) {
                char c = 'A' + (code - SDL_SCANCODE_A);
                m_initials.push_back(c);
                m_initialsRepeatCooldown = repeatGuard;
                m_initialsFeedbackTimer = m_feedbackDuration;
            }
        }
    }
    // Accept digits 0-9
    for (int code = SDL_SCANCODE_0; code <= SDL_SCANCODE_9; ++code) {
        if (input->IsKeyJustPressed(static_cast<SDL_Scancode>(code)) && m_initialsRepeatCooldown <= 0.0f) {
            if (m_initials.size() < 3) {
                char c = '0' + (code - SDL_SCANCODE_0);
                m_initials.push_back(c);
                m_initialsRepeatCooldown = repeatGuard;
                m_initialsFeedbackTimer = m_feedbackDuration;
            }
        }
    }


    if (input->IsKeyJustPressed(SDL_SCANCODE_BACKSPACE) && !m_initials.empty() && m_backspaceCooldown <= 0.0f) {
        m_initials.pop_back();
        m_backspaceCooldown = backspaceGuard;
        m_initialsFeedbackTimer = m_feedbackDuration;
    }

    if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN) || input->IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
        if (!m_initials.empty()) {
            HighScoreManager::SubmitEntry(HighScoreManager::Entry{m_initials, m_finalScore, ""});
            m_collectingInitials = false;
            m_showRestartPrompt = true; // allow normal flow after submission
        }
    }

    if (input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        // allow cancel (skip saving)
        m_collectingInitials = false;
        m_showRestartPrompt = true;
    }
}

