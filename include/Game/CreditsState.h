/**
 * @file CreditsState.h
 * @brief Credits screen state showing acknowledgments and licenses
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "GameState.h"
#include <vector>
#include <string>

/**
 * @class CreditsState
 * @brief Simple credits screen accessible from the main menu
 *
 * Features:
 * - Scrollable credits text
 * - Back to menu with Escape/B
 * - Minimal visuals reusing BitmapFont
 */
class CreditsState : public GameState {
public:
    CreditsState();
    ~CreditsState() override = default;

    void OnEnter() override;
    void OnExit() override;
    void Update(float deltaTime) override;
    void Render() override;
    void HandleInput() override;

private:
    // Rendering helpers
    void DrawBackground();
    void DrawTitle();
    void DrawCreditsText();
    void DrawInstructions();

    // Loading helper
    void LoadCreditsFromFile(const std::string& path);

    // Data
    std::vector<std::string> m_lines;
    float m_scrollY;       // current scroll offset (pixels)
    float m_scrollSpeed;   // pixels per second when holding Up/Down
};

