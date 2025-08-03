#pragma once

#include "GameState.h"
#include "ECS/EntityManager.h"
#include <memory>

class PlayingState : public GameState {
public:
    PlayingState();
    ~PlayingState();
    
    void OnEnter() override;
    void OnExit() override;
    void Update(float deltaTime) override;
    void Render() override;
    void HandleInput() override;

private:
    std::unique_ptr<EntityManager> m_entityManager;
    Entity m_player;
    float m_cameraX;
    int m_score;
    float m_gameTime;

    // Simple player position (bypassing ECS corruption)
    float m_playerX;
    float m_playerY;
    float m_playerVelX;
    float m_playerVelY;
    
    void CreatePlayer();
    void CreateEnemies();
    void UpdateCamera();
    void DrawHUD();
    void CheckGameOver();
    void DrawScrollingBackground();
    void UpdateScore();
    void UpdatePlayerAnimation();
};
