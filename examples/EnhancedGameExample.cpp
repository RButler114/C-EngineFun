/**
 * @file EnhancedGameExample.cpp
 * @brief Example showing how to use the enhanced ECS architecture
 * @author Ryan Butler
 * @date 2025
 */

#include "Engine/Engine.h"
#include "ECS/ECS.h"
#include "ECS/EnhancedSystems.h"
#include "ECS/AISystem.h"
#include "ECS/EnhancedMovementSystem.h"
#include "Engine/EventSystem.h"
#include "Game/CharacterFactory.h"
#include <iostream>
#include <memory>

/**
 * @class EnhancedGameExample
 * @brief Example game demonstrating the enhanced ECS architecture
 *
 * Shows how to:
 * - Use the character factory to create different entity types
 * - Set up enhanced systems with event communication
 * - Handle complex interactions between systems
 * - Manage different character types with AI behaviors
 */
class EnhancedGameExample : public Engine {
public:
    EnhancedGameExample() 
        : m_entityManager(std::make_unique<EntityManager>())
        , m_eventManager(std::make_unique<EventManager>())
        , m_characterFactory(std::make_unique<CharacterFactory>(m_entityManager.get()))
        , m_gameTime(0.0f) {}

protected:
    void Update(float deltaTime) override {
        m_gameTime += deltaTime;
        
        // Process queued events first
        m_eventManager->ProcessQueuedEvents();
        
        // Handle input for player control
        HandlePlayerInput();
        
        // Update all ECS systems
        m_entityManager->Update(deltaTime);
        
        // Spawn enemies periodically
        SpawnEnemiesOverTime();
        
        // Check win/lose conditions
        CheckGameState();
    }
    
    void Render() override {
        auto* renderer = GetRenderer();
        if (!renderer) return;
        
        // Clear screen
        renderer->Clear();
        
        // Render all entities with sprites
        RenderEntities();
        
        // Render UI
        RenderUI();
        
        // Present frame
        renderer->Present();
    }

private:
    std::unique_ptr<EntityManager> m_entityManager;
    std::unique_ptr<EventManager> m_eventManager;
    std::unique_ptr<CharacterFactory> m_characterFactory;
    
    Entity m_player;
    float m_gameTime;
    float m_lastEnemySpawn = 0.0f;
    int m_enemiesKilled = 0;
    
    void InitializeGame() {
        SetupSystems();
        SetupEventHandlers();
        CreatePlayer();
        CreateInitialEnemies();
    }
    
    void SetupSystems() {
        // Add core systems
        m_entityManager->AddSystem<MovementSystem>();
        m_entityManager->AddSystem<CollisionSystem>();
        
        // Add enhanced systems
        auto* healthSystem = m_entityManager->AddSystem<HealthSystem>();
        m_entityManager->AddSystem<CharacterStatsSystem>();
        m_entityManager->AddSystem<AbilitySystem>();
        m_entityManager->AddSystem<StatusEffectSystem>();
        m_entityManager->AddSystem<AISystem>();
        
        // Add enhanced movement system with physics
        auto* movementSystem = m_entityManager->AddSystem<PlatformerMovementSystem>(m_eventManager.get());
        movementSystem->SetGravity(800.0f);
        movementSystem->SetFriction(0.85f);
        movementSystem->SetWorldBoundaries(0.0f, 1200.0f, 0.0f, 600.0f);
        
        // Set up system callbacks
        healthSystem->SetDeathCallback([this](Entity entity) {
            OnEntityDeath(entity);
        });
        
        // Set up collision system callback
        auto* collisionSystem = m_entityManager->GetSystem<CollisionSystem>();
        if (collisionSystem) {
            collisionSystem->SetCollisionCallback([this](const CollisionInfo& info) {
                OnCollision(info);
            });
        }
    }
    
    void SetupEventHandlers() {
        // Subscribe to entity death events
        m_eventManager->Subscribe<EntityDeathEvent>([this](const EntityDeathEvent& event) {
            HandleEntityDeath(event);
        });
        
        // Subscribe to damage events
        m_eventManager->Subscribe<EntityDamagedEvent>([this](const EntityDamagedEvent& event) {
            HandleEntityDamaged(event);
        });
        
        // Subscribe to ability use events
        m_eventManager->Subscribe<AbilityUsedEvent>([this](const AbilityUsedEvent& event) {
            HandleAbilityUsed(event);
        });
    }
    
    void CreatePlayer() {
        m_player = m_characterFactory->CreatePlayer(100.0f, 400.0f);
        
        // Add some custom abilities to the player
        auto* abilities = m_entityManager->GetComponent<AbilityComponent>(m_player);
        if (abilities) {
            AbilityComponent::Ability dashAbility;
            dashAbility.name = "Dash";
            dashAbility.cooldown = 3.0f;
            dashAbility.staminaCost = 30.0f;
            dashAbility.range = 150.0f;
            abilities->AddAbility(dashAbility);
        }
        
        std::cout << "Player created with enhanced abilities!" << std::endl;
    }
    
    void CreateInitialEnemies() {
        // Create a variety of enemies
        m_characterFactory->CreateBasicEnemy(400.0f, 400.0f);
        m_characterFactory->CreateFastEnemy(600.0f, 400.0f);
        m_characterFactory->CreateTankEnemy(800.0f, 400.0f);
        
        // Set up patrol routes for some enemies
        auto entities = m_entityManager->GetEntitiesWith<AIComponent>();
        int patrolIndex = 0;
        for (Entity entity : entities) {
            auto* ai = m_entityManager->GetComponent<AIComponent>(entity);
            if (ai && patrolIndex < 2) {
                // Add patrol points
                ai->AddPatrolPoint(300.0f + patrolIndex * 200.0f, 400.0f);
                ai->AddPatrolPoint(500.0f + patrolIndex * 200.0f, 400.0f);
                ai->AddPatrolPoint(400.0f + patrolIndex * 200.0f, 350.0f);
                ai->ChangeState(AIComponent::AIState::PATROL);
                patrolIndex++;
            }
        }
        
        std::cout << "Initial enemies created with AI behaviors!" << std::endl;
    }
    
    void HandlePlayerInput() {
        auto* inputManager = GetInputManager();
        if (!inputManager) return;
        
        auto* velocity = m_entityManager->GetComponent<VelocityComponent>(m_player);
        auto* stats = m_entityManager->GetComponent<CharacterStatsComponent>(m_player);
        auto* movementSystem = m_entityManager->GetSystem<PlatformerMovementSystem>();
        
        if (!velocity || !stats) return;
        
        // Movement input
        float moveSpeed = stats->moveSpeed;
        velocity->vx = 0.0f; // Reset horizontal velocity
        
        if (inputManager->IsKeyPressed(SDLK_a) || inputManager->IsKeyPressed(SDLK_LEFT)) {
            velocity->vx = -moveSpeed;
        }
        if (inputManager->IsKeyPressed(SDLK_d) || inputManager->IsKeyPressed(SDLK_RIGHT)) {
            velocity->vx = moveSpeed;
        }
        
        // Jump input
        if (inputManager->IsKeyJustPressed(SDLK_SPACE) || inputManager->IsKeyJustPressed(SDLK_w)) {
            if (movementSystem) {
                movementSystem->Jump(m_player);
            }
        }
        
        // Ability input
        auto* abilities = m_entityManager->GetComponent<AbilityComponent>(m_player);
        auto* abilitySystem = m_entityManager->GetSystem<AbilitySystem>();
        
        if (abilities && abilitySystem) {
            if (inputManager->IsKeyJustPressed(SDLK_q)) {
                // Use first ability (Jump Attack)
                abilitySystem->TryUseAbility(m_player, 0);
            }
            if (inputManager->IsKeyJustPressed(SDLK_e)) {
                // Use second ability (Dash)
                if (abilities->abilities.size() > 1) {
                    abilitySystem->TryUseAbility(m_player, 1);
                }
            }
        }
    }
    
    void SpawnEnemiesOverTime() {
        if (m_gameTime - m_lastEnemySpawn > 10.0f) { // Spawn enemy every 10 seconds
            // Randomly choose enemy type
            int enemyType = rand() % 3;
            float spawnX = 1000.0f + (rand() % 200);
            float spawnY = 400.0f;
            
            switch (enemyType) {
                case 0:
                    m_characterFactory->CreateBasicEnemy(spawnX, spawnY);
                    break;
                case 1:
                    m_characterFactory->CreateFastEnemy(spawnX, spawnY);
                    break;
                case 2:
                    m_characterFactory->CreateTankEnemy(spawnX, spawnY);
                    break;
            }
            
            m_lastEnemySpawn = m_gameTime;
            std::cout << "New enemy spawned!" << std::endl;
        }
    }
    
    void OnEntityDeath(Entity entity) {
        auto* characterType = m_entityManager->GetComponent<CharacterTypeComponent>(entity);
        
        if (characterType) {
            if (characterType->type == CharacterTypeComponent::CharacterType::PLAYER) {
                std::cout << "Game Over! Player died." << std::endl;
                // Handle game over
            } else if (characterType->type == CharacterTypeComponent::CharacterType::ENEMY) {
                m_enemiesKilled++;
                std::cout << "Enemy defeated! Total kills: " << m_enemiesKilled << std::endl;
                
                // Fire death event
                m_eventManager->QueueEvent(EntityDeathEvent(entity, m_player));
            }
        }
    }
    
    void OnCollision(const CollisionInfo& info) {
        // Handle collision between entities
        auto* typeA = m_entityManager->GetComponent<CharacterTypeComponent>(info.entityA);
        auto* typeB = m_entityManager->GetComponent<CharacterTypeComponent>(info.entityB);
        
        if (!typeA || !typeB) return;
        
        // Player vs Enemy collision
        if ((typeA->type == CharacterTypeComponent::CharacterType::PLAYER && 
             typeB->type == CharacterTypeComponent::CharacterType::ENEMY) ||
            (typeA->type == CharacterTypeComponent::CharacterType::ENEMY && 
             typeB->type == CharacterTypeComponent::CharacterType::PLAYER)) {
            
            Entity player = (typeA->type == CharacterTypeComponent::CharacterType::PLAYER) ? info.entityA : info.entityB;
            Entity enemy = (typeA->type == CharacterTypeComponent::CharacterType::ENEMY) ? info.entityA : info.entityB;
            
            // Deal damage to player
            auto* healthSystem = m_entityManager->GetSystem<HealthSystem>();
            if (healthSystem) {
                float damage = 10.0f; // Base collision damage
                healthSystem->DealDamage(player, damage, 1.0f); // 1 second invulnerability
                
                // Fire damage event
                m_eventManager->QueueEvent(EntityDamagedEvent(player, enemy, damage, 0.0f));
            }
        }
    }
    
    void HandleEntityDeath(const EntityDeathEvent& event) {
        std::cout << "Entity death event processed for entity " << event.entity.GetID() << std::endl;
        
        // Could add death effects, drop items, update score, etc.
    }
    
    void HandleEntityDamaged(const EntityDamagedEvent& event) {
        std::cout << "Entity " << event.entity.GetID() << " took " << event.damage << " damage!" << std::endl;
        
        // Could add damage effects, screen shake, etc.
    }
    
    void HandleAbilityUsed(const AbilityUsedEvent& event) {
        std::cout << "Entity " << event.caster.GetID() << " used ability: " << event.abilityName << std::endl;
        
        // Handle specific ability effects
        if (event.abilityName == "Dash") {
            // Apply dash movement
            auto* movementSystem = m_entityManager->GetSystem<PlatformerMovementSystem>();
            if (movementSystem) {
                movementSystem->ApplyImpulse(event.caster, 200.0f, 0.0f);
            }
        }
    }
    
    void RenderEntities() {
        // This would render all entities with sprite components
        // Implementation depends on your rendering system
    }
    
    void RenderUI() {
        // Render health bars, ability cooldowns, score, etc.
        // Implementation depends on your UI system
    }
    
    void CheckGameState() {
        // Check for win/lose conditions
        auto* playerHealth = m_entityManager->GetComponent<HealthComponent>(m_player);
        if (playerHealth && playerHealth->isDead) {
            std::cout << "Game Over!" << std::endl;
            // Handle game over
        }
        
        if (m_enemiesKilled >= 20) {
            std::cout << "Victory! You defeated 20 enemies!" << std::endl;
            // Handle victory
        }
    }
};

// Example of how to run the enhanced game
int main() {
    EnhancedGameExample game;
    
    if (game.Initialize("Enhanced ECS Game Example", 1200, 600)) {
        std::cout << "Enhanced ECS Game Example initialized!" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  A/D or Arrow Keys - Move" << std::endl;
        std::cout << "  Space/W - Jump" << std::endl;
        std::cout << "  Q - Jump Attack" << std::endl;
        std::cout << "  E - Dash" << std::endl;
        
        game.Run();
    }
    
    return 0;
}
