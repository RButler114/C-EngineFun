#include "Engine/Engine.h"
#include "Engine/Window.h"
#include "Engine/Renderer.h"
#include "Engine/InputManager.h"
#include "ECS/ECS.h"
#include "Scenes/Scene.h"
#include "Scenes/SceneManager.h"
#include <iostream>
#include <random>

class GameScene : public Scene {
public:
    GameScene() : Scene("GameScene") {
        InitializeEntityManager();
    }

    void OnEnter() override {
        std::cout << "Entering Game Scene" << std::endl;
        
        // Add systems (no render system - we'll render manually)
        m_entityManager->AddSystem<MovementSystem>();
        auto* collisionSystem = m_entityManager->AddSystem<CollisionSystem>();
        
        // Set collision callback
        collisionSystem->SetCollisionCallback([this](const CollisionInfo& info) {
            OnCollision(info);
        });
        
        CreateEntities();
    }

    void Update(float deltaTime) override {
        HandleInput();

        // Update only movement and collision systems
        auto* movementSystem = m_entityManager->GetSystem<MovementSystem>();
        auto* collisionSystem = m_entityManager->GetSystem<CollisionSystem>();

        if (movementSystem) movementSystem->Update(deltaTime);
        if (collisionSystem) collisionSystem->Update(deltaTime);
    }

    void Render() override {
        // Manually render all entities
        auto entities = m_entityManager->GetEntitiesWith<TransformComponent, RenderComponent>();
        auto* renderer = GetEngine()->GetRenderer();

        // Debug output (once per second)
        static float debugTimer = 0.0f;
        static bool firstTime = true;
        debugTimer += 0.016f; // Approximate frame time

        bool shouldDebug = firstTime || debugTimer >= 1.0f;
        if (shouldDebug) {
            std::cout << "Rendering " << entities.size() << " entities" << std::endl;
            debugTimer = 0.0f;
            firstTime = false;
        }

        int entityCount = 0;
        for (Entity entity : entities) {
            auto* transform = m_entityManager->GetComponent<TransformComponent>(entity);
            auto* render = m_entityManager->GetComponent<RenderComponent>(entity);

            if (transform && render && render->visible) {
                Rectangle rect(
                    static_cast<int>(transform->x),
                    static_cast<int>(transform->y),
                    render->width,
                    render->height
                );

                Color color(render->r, render->g, render->b, render->a);
                renderer->DrawRectangle(rect, color, true);

                // Debug first few entities
                if (shouldDebug && entityCount < 3) {
                    std::cout << "Entity " << entity.GetID() << " at (" << transform->x << "," << transform->y
                              << ") size(" << render->width << "," << render->height
                              << ") color(" << (int)render->r << "," << (int)render->g << "," << (int)render->b << ")" << std::endl;
                }
                entityCount++;
            }
        }

        // Also draw a simple test rectangle to verify rendering works
        renderer->DrawRectangle(Rectangle(50, 50, 100, 100), Color(255, 255, 0, 255), true);
    }

    void HandleInput() override {
        auto* input = GetEngine()->GetInputManager();
        
        // Move player with arrow keys
        if (m_playerEntity.IsValid()) {
            auto* velocity = m_entityManager->GetComponent<VelocityComponent>(m_playerEntity);
            if (velocity) {
                velocity->vx = 0;
                velocity->vy = 0;
                
                const float speed = 200.0f;
                if (input->IsKeyPressed(SDL_SCANCODE_LEFT) || input->IsKeyPressed(SDL_SCANCODE_A)) {
                    velocity->vx = -speed;
                }
                if (input->IsKeyPressed(SDL_SCANCODE_RIGHT) || input->IsKeyPressed(SDL_SCANCODE_D)) {
                    velocity->vx = speed;
                }
                if (input->IsKeyPressed(SDL_SCANCODE_UP) || input->IsKeyPressed(SDL_SCANCODE_W)) {
                    velocity->vy = -speed;
                }
                if (input->IsKeyPressed(SDL_SCANCODE_DOWN) || input->IsKeyPressed(SDL_SCANCODE_S)) {
                    velocity->vy = speed;
                }
            }
        }
        
        // Reset on R key
        if (input->IsKeyJustPressed(SDL_SCANCODE_R)) {
            ResetEntities();
        }
    }

private:
    Entity m_playerEntity;
    std::vector<Entity> m_enemies;
    std::mt19937 m_randomEngine;
    std::uniform_real_distribution<float> m_positionDist;
    std::uniform_int_distribution<int> m_colorDist;

    void CreateEntities() {
        std::cout << "Creating entities..." << std::endl;

        m_randomEngine.seed(std::chrono::steady_clock::now().time_since_epoch().count());
        m_positionDist = std::uniform_real_distribution<float>(50.0f, 700.0f);
        m_colorDist = std::uniform_int_distribution<int>(0, 255);

        // Create player
        m_playerEntity = m_entityManager->CreateEntity();
        std::cout << "Created player entity with ID: " << m_playerEntity.GetID() << std::endl;

        m_entityManager->AddComponent<TransformComponent>(m_playerEntity, 100.0f, 100.0f);
        m_entityManager->AddComponent<VelocityComponent>(m_playerEntity, 0.0f, 0.0f);
        m_entityManager->AddComponent<RenderComponent>(m_playerEntity, 32, 32, 0, 255, 0); // Green
        m_entityManager->AddComponent<CollisionComponent>(m_playerEntity, 32.0f, 32.0f);

        std::cout << "Added components to player" << std::endl;
        
        // Create enemies
        for (int i = 0; i < 5; ++i) {
            Entity enemy = m_entityManager->CreateEntity();
            m_enemies.push_back(enemy);
            
            float x = m_positionDist(m_randomEngine);
            float y = m_positionDist(m_randomEngine);
            float vx = (m_randomEngine() % 2 == 0 ? 1 : -1) * (50.0f + m_randomEngine() % 100);
            float vy = (m_randomEngine() % 2 == 0 ? 1 : -1) * (50.0f + m_randomEngine() % 100);
            
            m_entityManager->AddComponent<TransformComponent>(enemy, x, y);
            m_entityManager->AddComponent<VelocityComponent>(enemy, vx, vy);
            m_entityManager->AddComponent<RenderComponent>(enemy, 24, 24, 255, 0, 0); // Red
            m_entityManager->AddComponent<CollisionComponent>(enemy, 24.0f, 24.0f);
        }
        
        // Create walls
        CreateWalls();
    }
    
    void CreateWalls() {
        // Top wall
        Entity topWall = m_entityManager->CreateEntity();
        m_entityManager->AddComponent<TransformComponent>(topWall, 0.0f, 0.0f);
        m_entityManager->AddComponent<RenderComponent>(topWall, 800, 10, 128, 128, 128);
        m_entityManager->AddComponent<CollisionComponent>(topWall, 800.0f, 10.0f);
        
        // Bottom wall
        Entity bottomWall = m_entityManager->CreateEntity();
        m_entityManager->AddComponent<TransformComponent>(bottomWall, 0.0f, 590.0f);
        m_entityManager->AddComponent<RenderComponent>(bottomWall, 800, 10, 128, 128, 128);
        m_entityManager->AddComponent<CollisionComponent>(bottomWall, 800.0f, 10.0f);
        
        // Left wall
        Entity leftWall = m_entityManager->CreateEntity();
        m_entityManager->AddComponent<TransformComponent>(leftWall, 0.0f, 0.0f);
        m_entityManager->AddComponent<RenderComponent>(leftWall, 10, 600, 128, 128, 128);
        m_entityManager->AddComponent<CollisionComponent>(leftWall, 10.0f, 600.0f);
        
        // Right wall
        Entity rightWall = m_entityManager->CreateEntity();
        m_entityManager->AddComponent<TransformComponent>(rightWall, 790.0f, 0.0f);
        m_entityManager->AddComponent<RenderComponent>(rightWall, 10, 600, 128, 128, 128);
        m_entityManager->AddComponent<CollisionComponent>(rightWall, 10.0f, 600.0f);
    }
    
    void ResetEntities() {
        // Reset player position
        if (m_playerEntity.IsValid()) {
            auto* transform = m_entityManager->GetComponent<TransformComponent>(m_playerEntity);
            if (transform) {
                transform->x = 100.0f;
                transform->y = 100.0f;
            }
        }
        
        // Reset enemy positions and velocities
        for (Entity enemy : m_enemies) {
            if (enemy.IsValid()) {
                auto* transform = m_entityManager->GetComponent<TransformComponent>(enemy);
                auto* velocity = m_entityManager->GetComponent<VelocityComponent>(enemy);
                
                if (transform && velocity) {
                    transform->x = m_positionDist(m_randomEngine);
                    transform->y = m_positionDist(m_randomEngine);
                    
                    float vx = (m_randomEngine() % 2 == 0 ? 1 : -1) * (50.0f + m_randomEngine() % 100);
                    float vy = (m_randomEngine() % 2 == 0 ? 1 : -1) * (50.0f + m_randomEngine() % 100);
                    velocity->vx = vx;
                    velocity->vy = vy;
                }
            }
        }
    }
    
    void OnCollision(const CollisionInfo& info) {
        // Handle wall collisions by bouncing
        auto* transformA = m_entityManager->GetComponent<TransformComponent>(info.entityA);
        auto* velocityA = m_entityManager->GetComponent<VelocityComponent>(info.entityA);
        auto* transformB = m_entityManager->GetComponent<TransformComponent>(info.entityB);
        auto* velocityB = m_entityManager->GetComponent<VelocityComponent>(info.entityB);
        
        // Simple bounce logic
        if (velocityA && !velocityB) { // A is moving, B is static (wall)
            if (info.overlapX > info.overlapY) {
                velocityA->vy = -velocityA->vy;
                transformA->y += (velocityA->vy > 0) ? -info.overlapY : info.overlapY;
            } else {
                velocityA->vx = -velocityA->vx;
                transformA->x += (velocityA->vx > 0) ? -info.overlapX : info.overlapX;
            }
        } else if (velocityB && !velocityA) { // B is moving, A is static
            if (info.overlapX > info.overlapY) {
                velocityB->vy = -velocityB->vy;
                transformB->y += (velocityB->vy > 0) ? -info.overlapY : info.overlapY;
            } else {
                velocityB->vx = -velocityB->vx;
                transformB->x += (velocityB->vx > 0) ? -info.overlapX : info.overlapX;
            }
        } else if (velocityA && velocityB) { // Both moving
            // Simple collision response - swap velocities
            std::swap(velocityA->vx, velocityB->vx);
            std::swap(velocityA->vy, velocityB->vy);
        }
    }
};
