#include "Game/PlayingState.h"
#include "Game/GameStateManager.h"
#include "Game/GameOverState.h"
#include "Engine/Renderer.h"
#include "Engine/InputManager.h"
#include "Engine/Engine.h"
#include "Engine/AudioManager.h"
#include "Engine/BitmapFont.h"
#include "ECS/ECS.h"
#include <iostream>
#include <cstdlib>
#include <cmath>

PlayingState::PlayingState()
    : GameState(GameStateType::PLAYING, "Playing")
    , m_cameraX(0.0f)
    , m_score(0)
    , m_gameTime(0.0f)
    , m_playerX(100.0f)
    , m_playerY(400.0f)
    , m_playerVelX(0.0f)
    , m_playerVelY(0.0f) {
}

PlayingState::~PlayingState() = default;

void PlayingState::OnEnter() {
    std::cout << "Entering Playing State" << std::endl;
    
    // Initialize ECS
    m_entityManager = std::make_unique<EntityManager>();
    
    // Add systems
    m_entityManager->AddSystem<MovementSystem>();
    m_entityManager->AddSystem<RenderSystem>(GetRenderer());

    // Add audio system if audio manager is available
    if (GetEngine()->GetAudioManager()) {
        m_entityManager->AddSystem<AudioSystem>(*GetEngine()->GetAudioManager());

        // Load game sounds
        GetEngine()->GetAudioManager()->LoadSound("jump", "assets/sounds/jump.wav", SoundType::SOUND_EFFECT);
        GetEngine()->GetAudioManager()->LoadSound("collision", "assets/sounds/collision.wav", SoundType::SOUND_EFFECT);
        GetEngine()->GetAudioManager()->LoadMusic("background", "assets/music/background.wav");

        // Start background music
        GetEngine()->GetAudioManager()->PlayMusic("background", 0.3f, -1);
    }
    
    // Create game entities
    CreatePlayer();
    CreateEnemies();
    
    // Reset game state
    m_cameraX = 0.0f;
    m_score = 0;
    m_gameTime = 0.0f;
}

void PlayingState::OnExit() {
    std::cout << "Exiting Playing State" << std::endl;
    m_entityManager.reset();
}

void PlayingState::Update(float deltaTime) {
    m_gameTime += deltaTime;

    // Update ECS
    if (m_entityManager) {
        m_entityManager->Update(deltaTime);
    }

    // Update player position (simple movement system)
    m_playerX += m_playerVelX * deltaTime;
    m_playerY += m_playerVelY * deltaTime;

    // Keep player within bounds
    if (m_playerX < m_cameraX - 50) {
        m_playerX = m_cameraX - 50; // Don't go too far left of camera
    }
    if (m_playerY < 350.0f) {
        m_playerY = 350.0f; // Sky limit
    }
    if (m_playerY > 450.0f) {
        m_playerY = 450.0f; // Ground limit
    }

    UpdateCamera();
    UpdateScore();
    CheckGameOver();

    // Debug output every 5 seconds
    static float debugTimer = 0.0f;
    debugTimer += deltaTime;
    if (debugTimer >= 5.0f) {
        float timeLeft = 30.0f - m_gameTime;
        std::cout << "Game Time: " << static_cast<int>(m_gameTime) << "s, Time Left: "
                  << static_cast<int>(timeLeft) << "s, Score: " << m_score << std::endl;
        debugTimer = 0.0f;
    }
}

void PlayingState::Render() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    // Draw scrolling background
    DrawScrollingBackground();

    // Draw ground that scrolls with camera
    int groundX = static_cast<int>(-m_cameraX) % 800;
    for (int x = groundX - 800; x < 800 + 800; x += 800) {
        renderer->DrawRectangle(Rectangle(x, 500, 800, 100), Color(100, 150, 50, 255), true);

        // Add some ground details
        for (int i = 0; i < 10; i++) {
            int detailX = x + i * 80 + 20;
            renderer->DrawRectangle(Rectangle(detailX, 520, 40, 10), Color(80, 120, 30, 255), true);
        }
    }
    
    // Render entities directly using component data (avoiding ECS query issues)
    static int debugFrameCount = 0;
    debugFrameCount++;
    bool shouldDebug = (debugFrameCount % 600 == 1); // Debug every 10 seconds at 60fps

    // Render player using simple position variables
    int playerScreenX = static_cast<int>(m_playerX - m_cameraX);
    int playerScreenY = static_cast<int>(m_playerY);

    if (playerScreenX > -32 && playerScreenX < 800 + 32) {
        Rectangle playerRect(playerScreenX, playerScreenY, 32, 48);
        Color playerColor(0, 255, 0, 255); // Green player
        renderer->DrawRectangle(playerRect, playerColor, true);

        if (shouldDebug) {
            std::cout << "  ✅ Player: pos(" << m_playerX << ", " << m_playerY
                      << ") screen(" << playerScreenX << ", " << playerScreenY
                      << ") size(32x48) color(0,255,0)" << std::endl;
        }
    }

    // Render enemies with hardcoded positions for now (to avoid ECS corruption)
    for (int i = 0; i < 5; i++) {
        float enemyX = 400.0f + i * 200.0f;
        float enemyY = 400.0f + (i % 2) * 30.0f;
        int enemyScreenX = static_cast<int>(enemyX - m_cameraX);

        if (enemyScreenX > -28 && enemyScreenX < 800) {
            Color enemyColor;
            switch (i % 3) {
                case 0: enemyColor = Color(255, 0, 0, 255); break;   // Red
                case 1: enemyColor = Color(255, 100, 0, 255); break; // Orange
                case 2: enemyColor = Color(200, 0, 100, 255); break; // Purple
            }

            Rectangle enemyRect(enemyScreenX, static_cast<int>(enemyY), 28, 44);
            renderer->DrawRectangle(enemyRect, enemyColor, true);

            if (shouldDebug) {
                std::cout << "  ✅ Enemy " << i << ": pos(" << enemyX << ", " << enemyY << ") screen(" << enemyScreenX << ", " << enemyY << ") size(28x44) color(" << (int)enemyColor.r << "," << (int)enemyColor.g << "," << (int)enemyColor.b << ")" << std::endl;
            }
        }
    }


    
    DrawHUD();
}

void PlayingState::HandleInput() {
    auto* input = GetInputManager();
    if (!input) return;
    
    // Pause game or return to menu
    if (input->IsKeyJustPressed(SDL_SCANCODE_P)) {
        std::cout << "Game paused (not implemented yet)" << std::endl;
    }
    if (input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        std::cout << "Returning to menu..." << std::endl;
        if (GetStateManager()) {
            GetStateManager()->ChangeState(GameStateType::MENU);
        }
    }
    
    // Player movement (using simple variables to bypass ECS corruption)
    m_playerVelX = 0;
    m_playerVelY = 0;

    const float speed = 250.0f;

    // Horizontal movement
    if (input->IsKeyPressed(SDL_SCANCODE_LEFT) || input->IsKeyPressed(SDL_SCANCODE_A)) {
        m_playerVelX = -speed;
    }
    if (input->IsKeyPressed(SDL_SCANCODE_RIGHT) || input->IsKeyPressed(SDL_SCANCODE_D)) {
        m_playerVelX = speed;
    }

    // Vertical movement (constrained to ground level area)
    static bool wasMovingUp = false;
    bool isMovingUp = (input->IsKeyPressed(SDL_SCANCODE_UP) || input->IsKeyPressed(SDL_SCANCODE_W)) && m_playerY > 350.0f;

    if (isMovingUp) {
        m_playerVelY = -speed;

        // Play jump sound when starting to move up
        if (!wasMovingUp && GetEngine()->GetAudioManager()) {
            GetEngine()->GetAudioManager()->PlaySound("jump", 0.8f);
        }
    }
    wasMovingUp = isMovingUp;
    if (input->IsKeyPressed(SDL_SCANCODE_DOWN) || input->IsKeyPressed(SDL_SCANCODE_S)) {
        if (m_playerY < 450.0f) { // Don't go below ground
            m_playerVelY = speed;
        }
    }

    // Keep all entities (including enemies) within reasonable bounds to prevent rendering issues
    auto allEntities = m_entityManager->GetEntitiesWith<TransformComponent, VelocityComponent>();
    for (Entity entity : allEntities) {
        if (entity == m_player) continue; // Skip player, already handled above

        auto* transform = m_entityManager->GetComponent<TransformComponent>(entity);
        auto* velocity = m_entityManager->GetComponent<VelocityComponent>(entity);

        if (transform && velocity) {
            // Keep enemies within vertical bounds
            if (transform->y < 350.0f) {
                transform->y = 350.0f; // Sky limit
                velocity->vy = abs(velocity->vy); // Bounce down
            }
            if (transform->y > 450.0f) {
                transform->y = 450.0f; // Ground limit
                velocity->vy = -abs(velocity->vy); // Bounce up
            }

            // Remove enemies that have moved too far left (off screen)
            if (transform->x < m_cameraX - 200.0f) {
                // Reset enemy position to the right side for continuous gameplay
                transform->x = m_cameraX + 1000.0f + (rand() % 500);
                transform->y = 400.0f + (rand() % 60) - 30.0f; // Random height variation
            }
        }
    }
}

void PlayingState::CreatePlayer() {
    if (!m_entityManager) return;

    m_player = m_entityManager->CreateEntity();
    std::cout << "Created player entity with ID: " << m_player.GetID() << std::endl;

    // Create components with explicit values to avoid template forwarding issues
    float playerX = 100.0f;
    float playerY = 400.0f;
    std::cout << "Creating player at position: " << playerX << ", " << playerY << std::endl;

    auto* transform = m_entityManager->AddComponent<TransformComponent>(m_player, playerX, playerY);
    auto* velocity = m_entityManager->AddComponent<VelocityComponent>(m_player, 0.0f, 0.0f);
    auto* render = m_entityManager->AddComponent<RenderComponent>(m_player, 32, 48, 0, 255, 0); // Green player
    auto* audio = m_entityManager->AddComponent<AudioComponent>(m_player, "jump", 0.8f, false, false, false); // Jump sound on demand

    std::cout << "Added components to player:" << std::endl;
    std::cout << "  Transform: " << (transform ? "OK" : "FAILED") << " pos(" << (transform ? transform->x : 0) << "," << (transform ? transform->y : 0) << ")" << std::endl;
    std::cout << "  Velocity: " << (velocity ? "OK" : "FAILED") << std::endl;
    std::cout << "  Render: " << (render ? "OK" : "FAILED") << " size(" << (render ? render->width : 0) << "x" << (render ? render->height : 0) << ")" << std::endl;
}

void PlayingState::CreateEnemies() {
    if (!m_entityManager) return;

    // Create enemies spread across the scrolling world
    for (int i = 0; i < 8; i++) {
        Entity enemy = m_entityManager->CreateEntity();
        float x = 400.0f + i * 200.0f; // Spread enemies across a wider area
        float y = 400.0f + (i % 2) * 30.0f; // Vary height slightly

        // Vary enemy movement patterns
        float velocityX = -30.0f - (i % 3) * 20.0f; // Different speeds
        float velocityY = (i % 2 == 0) ? 0.0f : (i % 4 - 2) * 10.0f; // Some vertical movement

        auto* transform = m_entityManager->AddComponent<TransformComponent>(enemy, x, y);
        auto* velocity = m_entityManager->AddComponent<VelocityComponent>(enemy, velocityX, velocityY);

        // Vary enemy colors
        int colorVariant = i % 3;
        Color enemyColor;
        switch (colorVariant) {
            case 0: enemyColor = Color(255, 0, 0, 255); break;   // Red
            case 1: enemyColor = Color(255, 100, 0, 255); break; // Orange
            case 2: enemyColor = Color(200, 0, 100, 255); break; // Purple
        }

        auto* render = m_entityManager->AddComponent<RenderComponent>(enemy, 28, 44,
                                                     enemyColor.r, enemyColor.g, enemyColor.b);
        auto* audio = m_entityManager->AddComponent<AudioComponent>(enemy, "collision", 0.6f, false, false, true); // Collision sound

        std::cout << "Created enemy " << i << " with ID: " << enemy.GetID() << std::endl;
        std::cout << "  Transform: " << (transform ? "OK" : "FAILED") << " pos(" << (transform ? transform->x : 0) << "," << (transform ? transform->y : 0) << ")" << std::endl;
        std::cout << "  Velocity: " << (velocity ? "OK" : "FAILED") << " vel(" << (velocity ? velocity->vx : 0) << "," << (velocity ? velocity->vy : 0) << ")" << std::endl;
        std::cout << "  Render: " << (render ? "OK" : "FAILED") << " size(" << (render ? render->width : 0) << "x" << (render ? render->height : 0) << ") color(" << (int)enemyColor.r << "," << (int)enemyColor.g << "," << (int)enemyColor.b << ")" << std::endl;
    }

    std::cout << "Created " << 8 << " enemy entities across the world" << std::endl;
}

void PlayingState::UpdateCamera() {
    // Keep player slightly left of center for better forward visibility
    float targetCameraX = m_playerX - 300.0f;

    // Smooth camera following with proper delta time
    float cameraSpeed = 3.0f;
    float deltaTime = 0.016f; // Approximate frame time
    float oldCameraX = m_cameraX;
    m_cameraX += (targetCameraX - m_cameraX) * cameraSpeed * deltaTime;

    // Prevent camera from going too far left (start of level)
    if (m_cameraX < 0) {
        m_cameraX = 0;
    }

    // Debug camera movement
    static int cameraDebugCount = 0;
    if (++cameraDebugCount % 300 == 1) { // Every 5 seconds at 60fps
        std::cout << "CAMERA: player.x=" << m_playerX << " target=" << targetCameraX
                  << " camera=" << oldCameraX << "->" << m_cameraX << std::endl;
    }

    // Allow camera to move forward indefinitely for side-scrolling
    // No right limit - player can move as far right as they want
}

void PlayingState::DrawHUD() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    // Draw HUD background
    renderer->DrawRectangle(Rectangle(0, 0, 800, 60), Color(0, 0, 0, 200), true);
    renderer->DrawRectangle(Rectangle(0, 58, 800, 2), Color(255, 255, 0, 255), true);

    // Draw score using bitmap font
    std::string scoreText = "SCORE: " + std::to_string(m_score);
    BitmapFont::DrawText(renderer, scoreText, 10, 15, 2, Color(255, 255, 255, 255));

    // Draw countdown timer (30 seconds)
    float timeLeft = 30.0f - m_gameTime;
    if (timeLeft < 0) timeLeft = 0;

    int seconds = static_cast<int>(timeLeft);
    int tenths = static_cast<int>((timeLeft - seconds) * 10);
    std::string timerText = "TIME: " + std::to_string(seconds) + "." + std::to_string(tenths);

    // Color timer red when under 10 seconds
    Color timerColor = (timeLeft < 10.0f) ? Color(255, 100, 100, 255) : Color(255, 255, 255, 255);

    BitmapFont::DrawText(renderer, timerText, 500, 15, 2, timerColor);

    // Draw game instructions using bitmap font
    const char* instructions = "WASD/ARROWS: MOVE  ESC: MENU";
    BitmapFont::DrawText(renderer, instructions, 10, 35, 1, Color(180, 180, 180, 255));

    // Draw warning when time is low
    if (timeLeft < 10.0f && static_cast<int>(m_gameTime * 4) % 2 == 0) {
        const char* warning = "TIME RUNNING OUT!";
        int warningWidth = strlen(warning) * 6 * 3; // 6 pixels per char * 3 scale
        int warningX = (800 - warningWidth) / 2;

        BitmapFont::DrawText(renderer, warning, warningX, 100, 3, Color(255, 0, 0, 255));
    }
}

void PlayingState::CheckGameOver() {
    // Simple game over condition - 30 second timer
    if (m_gameTime > 30.0f) {
        std::cout << "Game Over - Time limit reached! Final Score: " << m_score << std::endl;
        if (GetStateManager()) {
            GetStateManager()->ChangeState(GameStateType::GAME_OVER);
        }
    }
}

void PlayingState::DrawScrollingBackground() {
    auto* renderer = GetRenderer();

    // Draw sky gradient
    for (int y = 0; y < 500; y += 4) {
        int intensity = 50 + (y * 100 / 500);
        Color skyColor(intensity, intensity + 50, intensity + 100, 255);
        renderer->DrawRectangle(Rectangle(0, y, 800, 4), skyColor, true);
    }

    // Draw distant mountains (parallax layer 1 - slowest)
    int mountainOffset = static_cast<int>(-m_cameraX * 0.2f) % 1600;
    for (int x = mountainOffset - 1600; x < 800 + 1600; x += 200) {
        // Draw mountain silhouettes
        for (int i = 0; i < 5; i++) {
            int mountainX = x + i * 40;
            int mountainHeight = 100 + (i % 3) * 30;
            renderer->DrawRectangle(Rectangle(mountainX, 400 - mountainHeight, 60, mountainHeight),
                                  Color(60, 80, 120, 255), true);
        }
    }

    // Draw buildings (parallax layer 2 - medium speed)
    int buildingOffset = static_cast<int>(-m_cameraX * 0.5f) % 1200;
    for (int x = buildingOffset - 1200; x < 800 + 1200; x += 150) {
        // Draw building silhouettes
        for (int i = 0; i < 4; i++) {
            int buildingX = x + i * 35;
            int buildingHeight = 80 + (i % 4) * 20;
            renderer->DrawRectangle(Rectangle(buildingX, 420 - buildingHeight, 30, buildingHeight),
                                  Color(40, 60, 80, 255), true);

            // Add windows
            for (int w = 0; w < 3; w++) {
                for (int h = 0; h < buildingHeight / 15; h++) {
                    if ((w + h) % 2 == 0) {
                        renderer->DrawRectangle(Rectangle(buildingX + 5 + w * 8, 425 - buildingHeight + h * 15, 4, 6),
                                              Color(255, 255, 150, 255), true);
                    }
                }
            }
        }
    }
}

void PlayingState::UpdateScore() {
    // Increase score over time and for movement
    m_score += static_cast<int>(1.0f); // 1 point per frame

    // Bonus points for player movement
    if (m_player.IsValid() && m_entityManager) {
        auto* velocity = m_entityManager->GetComponent<VelocityComponent>(m_player);
        if (velocity && (velocity->vx != 0 || velocity->vy != 0)) {
            m_score += 2; // Bonus for moving
        }
    }
}
