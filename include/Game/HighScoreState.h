#pragma once

#include "Game/GameState.h"
#include <vector>

class HighScoreState : public GameState {
public:
    HighScoreState();
    ~HighScoreState() override = default;

    void OnEnter() override;
    void OnExit() override;
    void Update(float deltaTime) override;
    void Render() override;
    void HandleInput() override;

private:
    std::vector<int> m_scores;
    float m_blinkTimer;
    bool m_showHint;

    void DrawPanel();
};

