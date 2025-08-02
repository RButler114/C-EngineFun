#pragma once

#include "GameState.h"

class GameOverState : public GameState {
public:
    GameOverState();
    
    void OnEnter() override;
    void OnExit() override;
    void Update(float deltaTime) override;
    void Render() override;
    void HandleInput() override;
    
    void SetScore(int score) { m_finalScore = score; }

private:
    int m_finalScore;
    float m_displayTimer;
    bool m_showRestartPrompt;
    
    void DrawGameOverScreen();
    void DrawScore();
    void DrawRestartPrompt();
};
