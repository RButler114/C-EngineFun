#pragma once

#include "GameState.h"
#include "ECS/EntityManager.h"
#include "ECS/CollisionSystem.h"
#include "Game/GameConfig.h"
#include "Game/CharacterFactory.h"
#include "Game/PlayerCustomization.h"
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
    std::unique_ptr<GameConfig> m_gameConfig;
    std::unique_ptr<CharacterFactory> m_characterFactory;
    Entity m_player;
    float m_cameraX;
    int m_score;
    float m_gameTime;

    // Simple player position (bypassing ECS corruption)
    float m_playerX;
    float m_playerY;
    float m_playerVelX;
    float m_playerVelY;

    // Collision cooldown to prevent immediate re-triggering
    float m_collisionCooldown;
    static constexpr float COLLISION_COOLDOWN_TIME = 1.0f; // 1 second cooldown

    // Pending combat state (to avoid threading issues on macOS)
    bool m_pendingCombat;
    Entity m_combatPlayer;
    Entity m_combatEnemy;
    
    void CreatePlayer();
    void CreateEnemies();
    void UpdateCamera();
    void DrawHUD();
    void CheckGameOver();
    void DrawScrollingBackground();
    void UpdateScore();
    void UpdatePlayerAnimation();
    void ResetGameState();
    void CreateConfigAwareCharacter(const std::string& characterType, float x, float y, float difficultyMultiplier = 1.0f);

    // Combat integration
    void OnCollision(const CollisionInfo& info);
    void TriggerCombat(Entity player, Entity enemy);

    // Animation integration
    void OnAnimationEvent(Entity entity, const std::string& animationName,
                         const std::string& eventType, int frameIndex);
};
