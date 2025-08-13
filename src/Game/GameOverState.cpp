/**
 * @file GameOverState.cpp
 * @brief Implementation of game over screen with score display and restart options
 * @author Ryan Butler
 * @date 2025
 */

#include "Game/GameOverState.h"
#include "Game/PlayingState.h"
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
    if (m_outcome == Outcome::WIN) {
        std::cout << "🎉 Victory - Final Score: " << m_finalScore << " | Run Total: " << m_runTotal << std::endl;
    } else {
        std::cout << "💀 Game Over - Final Score: " << m_finalScore << std::endl;
    }

    // For end-of-run victories (no next level), use run total for high score qualification
    int scoreForHighScore = (m_outcome == Outcome::WIN && m_nextLevel.empty()) ? m_runTotal : m_finalScore;
    m_collectingInitials = HighScoreManager::WouldQualify(scoreForHighScore);
    m_initials.clear();
    m_initialsRepeatCooldown = 0.0f;
    m_backspaceCooldown = 0.0f;
    m_initialsFeedbackTimer = 0.0f;

    // if (GetEngine()->GetAudioManager()) {
    //     GetEngine()->GetAudioManager()->PlaySound("game_over");
    // If this is a WIN with no next level, auto-switch to initials for cumulative run total
    if (m_outcome == Outcome::WIN && m_nextLevel.empty()) {
        // Treat m_finalScore as the cumulative total for submission
        m_collectingInitials = HighScoreManager::WouldQualify(m_runTotal > 0 ? m_runTotal : m_finalScore);
        if (!m_collectingInitials) {
            // No initials needed; allow immediate restart/menu prompt
            m_showRestartPrompt = true;
        }
    }

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

    // Auto-submit run total immediately if it's an end-of-run victory and no initials required
    if (m_outcome == Outcome::WIN && m_nextLevel.empty() && !m_collectingInitials) {
        if (m_runTotal > 0) {
            HighScoreManager::SubmitEntry(HighScoreManager::Entry{"YOU", m_runTotal, ""});
        }
    }

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
            // If we just finished a level in a run, add level score to run total now
            if (m_outcome == Outcome::WIN) {
                if (auto* manager = GetStateManager()) {
                    if (auto* ps = dynamic_cast<PlayingState*>(manager->GetState(GameStateType::PLAYING))) {
                        ps->AddToRunTotal(m_finalScore);
                        if (!m_nextLevel.empty()) {
                            ps->LoadLevelAndReset(m_nextLevel);
                        } else {
                            // End of run: submit to highscores
                            HighScoreManager::SubmitEntry(HighScoreManager::Entry{"YOU", ps->GetTotalRunScore(), ""});
                            ps->ResetRunTotal();
                        }
                    }
                }
            }


    if (m_collectingInitials) {
        HandleInitialsInput();
        return;
    }

    // fall through to normal prompt handling below

    if (m_showRestartPrompt) {
        auto* manager = GetStateManager();
        // Primary action: Enter/Space
        if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN) || input->IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
            if (m_outcome == Outcome::WIN) {
                if (!m_nextLevel.empty()) {
                    std::cout << "Proceeding to next level: " << m_nextLevel << std::endl;
                } else {
                    std::cout << "Run complete! Returning to Playing to start new run." << std::endl;
                }
                if (manager) {
                    manager->ChangeState(GameStateType::PLAYING);
                }
            } else {
                std::cout << "Restarting game..." << std::endl;
                if (manager) {
                    manager->ChangeState(GameStateType::PLAYING);
                }
            }
        }

        // Secondary action: Escape/Menu goes back to main menu
        if (input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || input->IsKeyJustPressed(SDL_SCANCODE_M)) {
            std::cout << "Returning to menu..." << std::endl;
            if (manager) {
                manager->ChangeState(GameStateType::MENU);
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
    const char* title = (m_outcome == Outcome::WIN) ? "VICTORY" : "GAME OVER";
    Color titleColor = (m_outcome == Outcome::WIN) ? Color(80, 255, 80, 255) : Color(255, 80, 80, 255);
    int titleScale = 6;
    int textWidth = static_cast<int>(strlen(title)) * 6 * titleScale;
    int startX = (screenW - textWidth) / 2;
    int textY = 120; // was 200; move up to create space for score and prompts

    BitmapFont::DrawText(renderer, title, startX, textY, titleScale, titleColor);
}

void GameOverState::DrawScore() {
    auto* renderer = GetRenderer();

    int screenW = 800;
    // Use run total display on victory (shows both)
    std::string scoreText = (m_outcome == Outcome::WIN) ? "LEVEL SCORE" : "FINAL SCORE";
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

    if (m_outcome == Outcome::WIN) {
        std::string totalText = "RUN TOTAL";
        std::string totalValue = std::to_string(m_runTotal);
        int tLabelWidth = static_cast<int>(totalText.length()) * 6 * labelScale;
        int tLabelX = (screenW - tLabelWidth) / 2;
        int tLabelY = valueY + 48;
        BitmapFont::DrawText(renderer, totalText, tLabelX, tLabelY, labelScale, Color(180, 220, 255, 255));

        int tValueWidth = static_cast<int>(totalValue.length()) * 6 * valueScale;
        int tValueX = (screenW - tValueWidth) / 2;
        int tValueY = tLabelY + 34;
        BitmapFont::DrawText(renderer, totalValue, tValueX, tValueY, valueScale, Color(180, 255, 180, 255));
    }
}

void GameOverState::DrawRestartPrompt() {
    auto* renderer = GetRenderer();

    int screenW = 800, screenH = 600;
    int padY = 24;

    // Split prompts with spacing near bottom area
    bool isWin = (m_outcome == Outcome::WIN);
    std::string primary = isWin ? (m_nextLevel.empty() ? "PRESS ENTER TO CONTINUE" : ("PRESS ENTER FOR NEXT LEVEL: " + m_nextLevel))
                                : "PRESS ENTER TO PLAY AGAIN";
    int restartScale = 2;
    int restartWidth = static_cast<int>(primary.length()) * 6 * restartScale;
    int restartX = (screenW - restartWidth) / 2;
    int restartY = screenH - 150;
    BitmapFont::DrawText(renderer, primary.c_str(), restartX, restartY, restartScale, Color(255, 255, 255, 255));

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

