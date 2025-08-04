#include "Game/PlayingState.h"
#include "Game/GameStateManager.h"
#include "Game/GameOverState.h"
#include "Engine/Renderer.h"
#include "Engine/InputManager.h"
#include "Engine/Engine.h"
#include "Engine/AudioManager.h"
#include "Engine/BitmapFont.h"
#include "Engine/SpriteRenderer.h"
#include "ECS/ECS.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>

PlayingState::PlayingState()
    : GameState(GameStateType::PLAYING, "Playing")
    , m_gameConfig(std::make_unique<GameConfig>())
    , m_characterFactory(nullptr)  // Will be initialized in OnEnter when EntityManager is ready
    , m_cameraX(0.0f)
    , m_score(0)
    , m_gameTime(0.0f)
    , m_playerX(0.0f)  // Will be set from config in OnEnter
    , m_playerY(0.0f)  // Will be set from config in OnEnter
    , m_playerVelX(0.0f)
    , m_playerVelY(0.0f) {

    // Load configuration
    if (!m_gameConfig->LoadConfigs()) {
        std::cerr << "Warning: Failed to load some game configs, using defaults" << std::endl;
    }

    // Set initial player position from config
    m_playerX = m_gameConfig->GetPlayerStartX();
    m_playerY = m_gameConfig->GetPlayerStartY();
}

PlayingState::~PlayingState() = default;

void PlayingState::OnEnter() {
    std::cout << "Entering Playing State" << std::endl;
    
    // Initialize ECS
    m_entityManager = std::make_unique<EntityManager>();

    // Add core systems for arcade gameplay
    m_entityManager->AddSystem<MovementSystem>();
    m_entityManager->AddSystem<CollisionSystem>();

    // Initialize CharacterFactory now that EntityManager is ready
    m_characterFactory = std::make_unique<CharacterFactory>(m_entityManager.get());

    // Load character templates
    if (!m_characterFactory->LoadFromConfig("assets/config/characters.ini")) {
        std::cerr << "Warning: Failed to load character configs, using defaults" << std::endl;
    }

    // Add audio system if audio manager is available
    if (GetEngine()->GetAudioManager()) {
        m_entityManager->AddSystem<AudioSystem>(*GetEngine()->GetAudioManager());

        // Load game sounds
        GetEngine()->GetAudioManager()->LoadSound("jump", "assets/sounds/jump.wav", SoundType::SOUND_EFFECT);
        GetEngine()->GetAudioManager()->LoadSound("collision", "assets/sounds/collision.wav", SoundType::SOUND_EFFECT);
        GetEngine()->GetAudioManager()->LoadMusic("background", "assets/music/background.wav");

        // Start background music
        GetEngine()->GetAudioManager()->PlayMusic("background", m_gameConfig->GetBackgroundMusicVolume(), -1);
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

        // Update player animation based on movement
        UpdatePlayerAnimation();
    }

    // Update player position (simple movement system)
    m_playerX += m_playerVelX * deltaTime;
    m_playerY += m_playerVelY * deltaTime;

    // Keep player within bounds
    float leftBoundary = m_cameraX + m_gameConfig->GetPlayerCameraLeftBoundary();
    if (m_playerX < leftBoundary) {
        m_playerX = leftBoundary; // Don't go too far left of camera
    }
    if (m_playerY < m_gameConfig->GetPlayerSkyLimit()) {
        m_playerY = m_gameConfig->GetPlayerSkyLimit(); // Sky limit
    }
    if (m_playerY > m_gameConfig->GetPlayerGroundLimit()) {
        m_playerY = m_gameConfig->GetPlayerGroundLimit(); // Ground limit
    }

    UpdateCamera();
    UpdateScore();
    CheckGameOver();

    // Check for config hot-reloading (every few seconds)
    static float configCheckTimer = 0.0f;
    configCheckTimer += deltaTime;
    if (configCheckTimer >= 2.0f) { // Check every 2 seconds
        if (m_gameConfig->CheckAndReloadIfModified()) {
            std::cout << "🎮 Game config reloaded! Changes will take effect immediately." << std::endl;
        }
        configCheckTimer = 0.0f;
    }

    // Debug output at configured interval
    static float debugTimer = 0.0f;
    debugTimer += deltaTime;
    float debugInterval = m_gameConfig->GetDebugOutputInterval();
    if (debugTimer >= debugInterval) {
        float timeLeft = m_gameConfig->GetGameDurationSeconds() - m_gameTime;
        std::cout << "Game Time: " << static_cast<int>(m_gameTime) << "s, Time Left: "
                  << static_cast<int>(timeLeft) << "s, Score: " << m_score << std::endl;
        debugTimer = 0.0f;
    }
}

void PlayingState::Render() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    // Get screen dimensions once for the entire render method
    int screenWidth = m_gameConfig->GetScreenWidth();

    // Draw scrolling background
    DrawScrollingBackground();

    // Draw ground that scrolls with camera
    int groundY = m_gameConfig->GetGroundY();
    int groundHeight = m_gameConfig->GetGroundHeight();
    Color groundColor = m_gameConfig->GetGroundColor();
    Color groundDetailColor = m_gameConfig->GetGroundDetailColor();

    int groundX = static_cast<int>(-m_cameraX) % screenWidth;
    for (int x = groundX - screenWidth; x < screenWidth + screenWidth; x += screenWidth) {
        renderer->DrawRectangle(Rectangle(x, groundY, screenWidth, groundHeight), groundColor, true);

        // Add some ground details
        int detailCount = m_gameConfig->GetGroundDetailCount();
        int detailSpacing = m_gameConfig->GetGroundDetailSpacing();
        int detailOffset = m_gameConfig->GetGroundDetailOffset();
        int detailWidth = m_gameConfig->GetGroundDetailWidth();
        int detailHeight = m_gameConfig->GetGroundDetailHeight();

        for (int i = 0; i < detailCount; i++) {
            int detailX = x + i * detailSpacing + detailOffset;
            int detailY = groundY + 20; // Offset from ground surface
            renderer->DrawRectangle(Rectangle(detailX, detailY, detailWidth, detailHeight), groundDetailColor, true);
        }
    }
    
    // Render entities directly using component data (avoiding ECS query issues)
    static int debugFrameCount = 0;
    debugFrameCount++;
    int debugFrameInterval = m_gameConfig->GetDebugFrameInterval();
    bool shouldDebug = (debugFrameCount % debugFrameInterval == 1);

    // Update player transform component to match current position
    int playerScreenX = static_cast<int>(m_playerX - m_cameraX);
    int playerScreenY = static_cast<int>(m_playerY);

    if (m_entityManager && m_entityManager->IsEntityValid(m_player)) {
        auto* transform = m_entityManager->GetComponent<TransformComponent>(m_player);
        if (transform) {
            transform->x = playerScreenX; // Screen position
            transform->y = playerScreenY;

            if (shouldDebug) {
                std::cout << "  ✅ Player: world(" << m_playerX << ", " << m_playerY
                          << ") screen(" << transform->x << ", " << transform->y << ")" << std::endl;
            }
        }
    }

    // Render player sprite with animation
    int spriteWidth = m_gameConfig->GetAnimationSpriteWidth();
    if (playerScreenX > -spriteWidth && playerScreenX < screenWidth + spriteWidth) {
        // Animation logic
        static float animationTimer = 0.0f;
        static int currentFrame = 0;
        static bool facingLeft = false;

        float frameTime = m_gameConfig->GetApproximateFrameTime();
        animationTimer += frameTime;

        // Determine animation state and frame
        float movementThreshold = m_gameConfig->GetPlayerMovementThreshold();
        bool isMoving = (std::abs(m_playerVelX) > movementThreshold || std::abs(m_playerVelY) > movementThreshold);

        if (isMoving) {
            // Walking animation - cycle through frames
            float frameDuration = m_gameConfig->GetAnimationFrameDuration();
            if (animationTimer >= frameDuration) {
                int totalFrames = m_gameConfig->GetAnimationTotalFrames();
                currentFrame = (currentFrame + 1) % totalFrames;
                animationTimer = 0.0f;
            }

            // Update facing direction based on horizontal movement
            if (std::abs(m_playerVelX) > movementThreshold) {
                facingLeft = (m_playerVelX < 0);
            }
        } else {
            // Idle animation - stay on frame 0
            currentFrame = 0;
            animationTimer = 0.0f;
        }

        // Try to render sprite texture first
        int spriteHeight = m_gameConfig->GetAnimationSpriteHeight();
        int totalFrames = m_gameConfig->GetAnimationTotalFrames();
        float spriteScale = m_gameConfig->GetAnimationSpriteScale();
        SpriteFrame frame = SpriteRenderer::CreateFrame(currentFrame, spriteWidth, spriteHeight, totalFrames);
        SpriteRenderer::RenderSprite(renderer, "assets/sprites/player/little_adventurer.png",
                                   playerScreenX, playerScreenY, frame, facingLeft, spriteScale);

        // If texture fails, the SpriteRenderer will show a magenta placeholder
        // For a more detailed fallback, we can add simple shapes here
        static bool textureExists = true;
        auto texture = renderer->LoadTexture("assets/sprites/player/little_adventurer.png");
        if (!texture && textureExists) {
            textureExists = false;
            std::cout << "Using simple shape rendering for player" << std::endl;
        }

        if (!texture) {
            // Simple shape-based player character using config colors and dimensions
            Color bodyColor = m_gameConfig->GetPlayerBodyColor();
            int bodyWidth = m_gameConfig->GetPlayerBodyWidth();
            int bodyHeight = m_gameConfig->GetPlayerBodyHeight();
            int bodyOffsetX = m_gameConfig->GetPlayerBodyOffsetX();
            int bodyOffsetY = m_gameConfig->GetPlayerBodyOffsetY();
            Rectangle bodyRect(playerScreenX + bodyOffsetX, playerScreenY + bodyOffsetY, bodyWidth, bodyHeight);
            renderer->DrawRectangle(bodyRect, bodyColor, true);

            // Head
            Color headColor = m_gameConfig->GetPlayerHeadColor();
            int headWidth = m_gameConfig->GetPlayerHeadWidth();
            int headHeight = m_gameConfig->GetPlayerHeadHeight();
            int headOffsetX = m_gameConfig->GetPlayerHeadOffsetX();
            int headOffsetY = m_gameConfig->GetPlayerHeadOffsetY();
            Rectangle headRect(playerScreenX + headOffsetX, playerScreenY + headOffsetY, headWidth, headHeight);
            renderer->DrawRectangle(headRect, headColor, true);
        }
    }

    // Manual sprite rendering for better control in arcade games

    // Render enemies with config-driven positions and colors
    int enemyCount = std::min(5, m_gameConfig->GetEnemyCount()); // Limit to 5 for this simple rendering
    float enemySpawnStartX = m_gameConfig->GetEnemySpawnStartX();
    float enemySpawnSpacingX = m_gameConfig->GetEnemySpawnSpacingX();
    float enemySpawnHeightVariation = m_gameConfig->GetEnemySpawnHeightVariation();
    int enemyWidth = m_gameConfig->GetEnemyWidth();
    int enemyHeight = m_gameConfig->GetEnemyHeight();

    for (int i = 0; i < enemyCount; i++) {
        float enemyX = enemySpawnStartX + i * enemySpawnSpacingX;
        float enemyY = m_gameConfig->GetPlayerStartY() + (i % 2) * enemySpawnHeightVariation;
        int enemyScreenX = static_cast<int>(enemyX - m_cameraX);

        if (enemyScreenX > -enemyWidth && enemyScreenX < screenWidth) {
            Color enemyColor;
            switch (i % 3) {
                case 0: enemyColor = m_gameConfig->GetEnemyRedColor(); break;
                case 1: enemyColor = m_gameConfig->GetEnemyOrangeColor(); break;
                case 2: enemyColor = m_gameConfig->GetEnemyPurpleColor(); break;
            }

            Rectangle enemyRect(enemyScreenX, static_cast<int>(enemyY), enemyWidth, enemyHeight);
            renderer->DrawRectangle(enemyRect, enemyColor, true);

            if (shouldDebug) {
                std::cout << "  ✅ Enemy " << i << ": pos(" << enemyX << ", " << enemyY << ") screen(" << enemyScreenX << ", " << enemyY << ") size(" << enemyWidth << "x" << enemyHeight << ") color(" << (int)enemyColor.r << "," << (int)enemyColor.g << "," << (int)enemyColor.b << ")" << std::endl;
            }
        }
    }


    
    DrawHUD();
}

void PlayingState::HandleInput() {
    auto* input = GetInputManager();
    if (!input) return;
    
    // Pause game or return to menu
    if (input->IsKeyJustPressed(SDL_SCANCODE_P) || input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        std::cout << "Game paused (not implemented yet)" << std::endl;
    }
    if (input->IsKeyJustPressed(SDL_SCANCODE_M)) {
        std::cout << "Returning to menu..." << std::endl;
        if (GetStateManager()) {
            GetStateManager()->ChangeState(GameStateType::MENU);
        }
    }

    // Manual config reload (R key)
    if (input->IsKeyJustPressed(SDL_SCANCODE_R)) {
        std::cout << "🔄 Manual config reload requested..." << std::endl;
        if (m_gameConfig->ReloadConfigs()) {
            std::cout << "✅ Config reloaded successfully! Press R to reload configs anytime." << std::endl;
        } else {
            std::cout << "❌ Config reload failed!" << std::endl;
        }
    }

    // Level selection (1, 2, 3, B keys)
    if (input->IsKeyJustPressed(SDL_SCANCODE_1)) {
        std::cout << "🎮 Loading Level 1 (Easy)..." << std::endl;
        m_gameConfig->LoadLevelConfig("level1");
        ResetGameState();
    }
    if (input->IsKeyJustPressed(SDL_SCANCODE_2)) {
        std::cout << "🎮 Loading Level 2 (Normal)..." << std::endl;
        m_gameConfig->LoadLevelConfig("level2");
        ResetGameState();
    }
    if (input->IsKeyJustPressed(SDL_SCANCODE_3)) {
        std::cout << "🎮 Loading Level 3 (Hard)..." << std::endl;
        m_gameConfig->LoadLevelConfig("level3");
        ResetGameState();
    }
    if (input->IsKeyJustPressed(SDL_SCANCODE_B)) {
        std::cout << "🎮 Loading Boss Level..." << std::endl;
        m_gameConfig->LoadLevelConfig("boss");
        ResetGameState();
    }
    if (input->IsKeyJustPressed(SDL_SCANCODE_0)) {
        std::cout << "🎮 Clearing level overrides (Base config)..." << std::endl;
        m_gameConfig->ClearLevelOverrides();
        ResetGameState();
    }

    // Demonstrate config-aware character creation (G key)
    if (input->IsKeyJustPressed(SDL_SCANCODE_G)) {
        std::cout << "🧙 Creating config-aware goblin character..." << std::endl;
        float spawnX = m_playerX + 200.0f; // Spawn near player
        float spawnY = m_gameConfig->GetPlayerStartY();

        // Difficulty based on current level
        float difficulty = 1.0f;
        std::string currentLevel = m_gameConfig->GetCurrentLevel();
        if (currentLevel == "level1") difficulty = 0.7f;      // Easy
        else if (currentLevel == "level3") difficulty = 1.5f; // Hard
        else if (currentLevel == "boss") difficulty = 2.0f;   // Boss

        CreateConfigAwareCharacter("goblin", spawnX, spawnY, difficulty);
    }
    
    // Player movement (using simple variables to bypass ECS corruption)
    m_playerVelX = 0;
    m_playerVelY = 0;

    const float speed = m_gameConfig->GetPlayerMovementSpeed();

    // Horizontal movement
    if (input->IsKeyPressed(SDL_SCANCODE_LEFT) || input->IsKeyPressed(SDL_SCANCODE_A)) {
        m_playerVelX = -speed;
    }
    if (input->IsKeyPressed(SDL_SCANCODE_RIGHT) || input->IsKeyPressed(SDL_SCANCODE_D)) {
        m_playerVelX = speed;
    }

    // Vertical movement (constrained to ground level area)
    static bool wasMovingUp = false;
    float skyLimit = m_gameConfig->GetPlayerSkyLimit();
    float groundLimit = m_gameConfig->GetPlayerGroundLimit();
    bool isMovingUp = (input->IsKeyPressed(SDL_SCANCODE_UP) || input->IsKeyPressed(SDL_SCANCODE_W)) && m_playerY > skyLimit;

    if (isMovingUp) {
        m_playerVelY = -speed;

        // Play jump sound when starting to move up
        if (!wasMovingUp && GetEngine()->GetAudioManager()) {
            GetEngine()->GetAudioManager()->PlaySound("jump", m_gameConfig->GetJumpSoundVolume());
        }
    }
    wasMovingUp = isMovingUp;
    if (input->IsKeyPressed(SDL_SCANCODE_DOWN) || input->IsKeyPressed(SDL_SCANCODE_S)) {
        if (m_playerY < groundLimit) { // Don't go below ground
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
            float skyLimit = m_gameConfig->GetPlayerSkyLimit();
            float groundLimit = m_gameConfig->GetPlayerGroundLimit();

            if (transform->y < skyLimit) {
                transform->y = skyLimit; // Sky limit
                velocity->vy = abs(velocity->vy); // Bounce down
            }
            if (transform->y > groundLimit) {
                transform->y = groundLimit; // Ground limit
                velocity->vy = -abs(velocity->vy); // Bounce up
            }

            // Remove enemies that have moved too far left (off screen)
            float respawnDistance = m_gameConfig->GetEnemyRespawnDistance();
            if (transform->x < m_cameraX + respawnDistance) {
                // Reset enemy position to the right side for continuous gameplay
                float respawnOffset = m_gameConfig->GetEnemyRespawnOffset();
                int respawnRandomRange = m_gameConfig->GetEnemyRespawnRandomRange();
                int heightRandomRange = m_gameConfig->GetEnemyHeightRandomRange();

                transform->x = m_cameraX + respawnOffset + (rand() % respawnRandomRange);
                transform->y = m_gameConfig->GetPlayerStartY() + (rand() % heightRandomRange) - (heightRandomRange / 2);
            }
        }
    }
}

void PlayingState::CreatePlayer() {
    if (!m_entityManager || !m_characterFactory) return;

    // Get player position from config
    float playerX = m_gameConfig->GetPlayerStartX();
    float playerY = m_gameConfig->GetPlayerStartY();
    std::cout << "Creating player at position: " << playerX << ", " << playerY << std::endl;

    // Try to get customization data from the global customization manager
    const PlayerCustomization& customization = CustomizationManager::GetInstance().GetPlayerCustomization();

    // Create player using customization data if available
    if (!customization.playerName.empty() && customization.playerName != "Hero") {
        std::cout << "Creating customized player: " << customization.playerName
                  << " (" << customization.characterClass << ")" << std::endl;
        m_player = m_characterFactory->CreateCustomizedPlayer(playerX, playerY, customization);
    } else {
        std::cout << "Creating default player" << std::endl;
        m_player = m_characterFactory->CreatePlayer(playerX, playerY);
    }

    if (!m_player.IsValid()) {
        std::cerr << "Failed to create player entity!" << std::endl;
        return;
    }

    std::cout << "Created player entity with ID: " << m_player.GetID() << std::endl;

    // Add audio component for jump sound
    auto* audio = m_entityManager->AddComponent<AudioComponent>(m_player, "jump", m_gameConfig->GetJumpSoundVolume(), false, false, false);

    // Using direct sprite rendering for better control in arcade games
    // This approach gives us precise control over animation and rendering

    std::cout << "Player created successfully with customizations applied" << std::endl;
    std::cout << "  Using direct sprite rendering (bypassing ECS)" << std::endl;
}

void PlayingState::CreateEnemies() {
    if (!m_entityManager) return;

    // Create enemies spread across the scrolling world using config values
    int enemyCount = m_gameConfig->GetEnemyCount();
    float spawnStartX = m_gameConfig->GetEnemySpawnStartX();
    float spawnSpacingX = m_gameConfig->GetEnemySpawnSpacingX();
    float spawnHeightVariation = m_gameConfig->GetEnemySpawnHeightVariation();
    float baseVelocityX = m_gameConfig->GetEnemyBaseVelocityX();
    float velocityVariation = m_gameConfig->GetEnemyVelocityVariation();
    float verticalMovementRange = m_gameConfig->GetEnemyVerticalMovementRange();

    for (int i = 0; i < enemyCount; i++) {
        Entity enemy = m_entityManager->CreateEntity();
        float x = spawnStartX + i * spawnSpacingX; // Spread enemies across a wider area
        float y = m_gameConfig->GetPlayerStartY() + (i % 2) * spawnHeightVariation; // Vary height slightly

        // Vary enemy movement patterns
        float velocityX = baseVelocityX - (i % 3) * velocityVariation; // Different speeds
        float velocityY = (i % 2 == 0) ? 0.0f : (i % 4 - 2) * verticalMovementRange; // Some vertical movement

        auto* transform = m_entityManager->AddComponent<TransformComponent>(enemy, x, y);
        auto* velocity = m_entityManager->AddComponent<VelocityComponent>(enemy, velocityX, velocityY);

        // Vary enemy colors using config
        int colorVariant = i % 3;
        Color enemyColor;
        switch (colorVariant) {
            case 0: enemyColor = m_gameConfig->GetEnemyRedColor(); break;
            case 1: enemyColor = m_gameConfig->GetEnemyOrangeColor(); break;
            case 2: enemyColor = m_gameConfig->GetEnemyPurpleColor(); break;
        }

        int enemyWidth = m_gameConfig->GetEnemyWidth();
        int enemyHeight = m_gameConfig->GetEnemyHeight();
        auto* render = m_entityManager->AddComponent<RenderComponent>(enemy, enemyWidth, enemyHeight,
                                                     enemyColor.r, enemyColor.g, enemyColor.b);
        auto* audio = m_entityManager->AddComponent<AudioComponent>(enemy, "collision", m_gameConfig->GetCollisionSoundVolume(), false, false, true); // Collision sound

        std::cout << "Created enemy " << i << " with ID: " << enemy.GetID() << std::endl;
        std::cout << "  Transform: " << (transform ? "OK" : "FAILED") << " pos(" << (transform ? transform->x : 0) << "," << (transform ? transform->y : 0) << ")" << std::endl;
        std::cout << "  Velocity: " << (velocity ? "OK" : "FAILED") << " vel(" << (velocity ? velocity->vx : 0) << "," << (velocity ? velocity->vy : 0) << ")" << std::endl;
        std::cout << "  Render: " << (render ? "OK" : "FAILED") << " size(" << (render ? render->width : 0) << "x" << (render ? render->height : 0) << ") color(" << (int)enemyColor.r << "," << (int)enemyColor.g << "," << (int)enemyColor.b << ")" << std::endl;
    }

    std::cout << "Created " << enemyCount << " enemy entities across the world" << std::endl;
}

void PlayingState::UpdateCamera() {
    // Keep player slightly left of center for better forward visibility
    float targetCameraX = m_playerX + m_gameConfig->GetCameraTargetOffsetX();

    // Smooth camera following with proper delta time
    float cameraSpeed = m_gameConfig->GetCameraFollowSpeed();
    float deltaTime = m_gameConfig->GetApproximateFrameTime();
    float oldCameraX = m_cameraX;
    m_cameraX += (targetCameraX - m_cameraX) * cameraSpeed * deltaTime;

    // Prevent camera from going too far left (start of level)
    float minCameraX = m_gameConfig->GetCameraMinX();
    if (m_cameraX < minCameraX) {
        m_cameraX = minCameraX;
    }

    // Debug camera movement
    static int cameraDebugCount = 0;
    int debugFrameInterval = m_gameConfig->GetDebugFrameInterval();
    if (++cameraDebugCount % debugFrameInterval == 1) {
        std::cout << "CAMERA: player.x=" << m_playerX << " target=" << targetCameraX
                  << " camera=" << oldCameraX << "->" << m_cameraX << std::endl;
    }

    // Allow camera to move forward indefinitely for side-scrolling
    // No right limit - player can move as far right as they want
}

void PlayingState::DrawHUD() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    // Draw HUD background using config values
    int screenWidth = m_gameConfig->GetScreenWidth();
    int hudHeight = m_gameConfig->GetHudHeight();
    int hudBorderHeight = m_gameConfig->GetHudBorderHeight();
    Color hudBackgroundColor = m_gameConfig->GetHudBackgroundColor();
    Color hudBorderColor = m_gameConfig->GetHudBorderColor();

    renderer->DrawRectangle(Rectangle(0, 0, screenWidth, hudHeight), hudBackgroundColor, true);
    renderer->DrawRectangle(Rectangle(0, hudHeight - hudBorderHeight, screenWidth, hudBorderHeight), hudBorderColor, true);

    // Draw score using bitmap font
    std::string scoreText = "SCORE: " + std::to_string(m_score);
    int scoreX = m_gameConfig->GetScoreX();
    int scoreY = m_gameConfig->GetScoreY();
    int scoreScale = m_gameConfig->GetScoreScale();
    Color textNormalColor = m_gameConfig->GetTextNormalColor();
    BitmapFont::DrawText(renderer, scoreText, scoreX, scoreY, scoreScale, textNormalColor);

    // Draw countdown timer using config duration
    float gameDuration = m_gameConfig->GetGameDurationSeconds();
    float timeLeft = gameDuration - m_gameTime;
    if (timeLeft < 0) timeLeft = 0;

    int seconds = static_cast<int>(timeLeft);
    int tenths = static_cast<int>((timeLeft - seconds) * 10);
    std::string timerText = "TIME: " + std::to_string(seconds) + "." + std::to_string(tenths);

    // Color timer red when under warning threshold
    float warningThreshold = m_gameConfig->GetLowTimeWarningThreshold();
    Color timerColor = (timeLeft < warningThreshold) ? m_gameConfig->GetTextWarningColor() : textNormalColor;

    int timerX = m_gameConfig->GetTimerX();
    int timerY = m_gameConfig->GetTimerY();
    int timerScale = m_gameConfig->GetTimerScale();
    BitmapFont::DrawText(renderer, timerText, timerX, timerY, timerScale, timerColor);

    // Draw game instructions using bitmap font
    const char* instructions = "WASD: MOVE  ESC: MENU  R: RELOAD  1/2/3/B/0: LEVELS  G: SPAWN";
    int instructionsX = m_gameConfig->GetInstructionsX();
    int instructionsY = m_gameConfig->GetInstructionsY();
    int instructionsScale = m_gameConfig->GetInstructionsScale();
    Color instructionsColor = m_gameConfig->GetTextInstructionsColor();
    BitmapFont::DrawText(renderer, instructions, instructionsX, instructionsY, instructionsScale, instructionsColor);

    // Draw current level indicator
    std::string currentLevel = m_gameConfig->GetCurrentLevel();
    if (currentLevel.empty()) {
        currentLevel = "Base";
    }
    std::string levelText = "LEVEL: " + currentLevel;
    int levelX = screenWidth - 150; // Right side of screen
    int levelY = scoreY; // Same height as score
    BitmapFont::DrawText(renderer, levelText, levelX, levelY, scoreScale, textNormalColor);

    // Draw warning when time is low
    if (timeLeft < warningThreshold && static_cast<int>(m_gameTime * 4) % 2 == 0) {
        const char* warning = "TIME RUNNING OUT!";
        int warningScale = m_gameConfig->GetWarningScale();
        int warningWidth = strlen(warning) * 6 * warningScale; // 6 pixels per char * scale
        int warningX = (screenWidth - warningWidth) / 2;
        int warningY = m_gameConfig->GetWarningY();
        Color urgentColor = m_gameConfig->GetTextUrgentColor();

        BitmapFont::DrawText(renderer, warning, warningX, warningY, warningScale, urgentColor);
    }
}

void PlayingState::CheckGameOver() {
    // Game over condition using config duration
    float gameDuration = m_gameConfig->GetGameDurationSeconds();
    if (m_gameTime > gameDuration) {
        std::cout << "Game Over - Time limit reached! Final Score: " << m_score << std::endl;
        if (GetStateManager()) {
            GetStateManager()->ChangeState(GameStateType::GAME_OVER);
        }
    }
}

void PlayingState::DrawScrollingBackground() {
    auto* renderer = GetRenderer();

    // Draw sky gradient using config values
    int skyHeight = m_gameConfig->GetSkyHeight();
    int skyGradientStep = m_gameConfig->GetSkyGradientStep();
    int skyBaseIntensity = m_gameConfig->GetSkyBaseIntensity();
    int skyIntensityRange = m_gameConfig->GetSkyIntensityRange();
    int screenWidth = m_gameConfig->GetScreenWidth();

    for (int y = 0; y < skyHeight; y += skyGradientStep) {
        int intensity = skyBaseIntensity + (y * skyIntensityRange / skyHeight);
        Color skyColor(intensity, intensity + 50, intensity + 100, 255);
        renderer->DrawRectangle(Rectangle(0, y, screenWidth, skyGradientStep), skyColor, true);
    }

    // Draw distant mountains (parallax layer 1 - slowest) using config values
    float mountainParallaxSpeed = m_gameConfig->GetMountainParallaxSpeed();
    int mountainRepeatWidth = m_gameConfig->GetMountainRepeatWidth();
    int mountainSpacing = m_gameConfig->GetMountainSpacing();
    int mountainCountPerGroup = m_gameConfig->GetMountainCountPerGroup();
    int mountainWidth = m_gameConfig->GetMountainWidth();
    int mountainBaseHeight = m_gameConfig->GetMountainBaseHeight();
    int mountainHeightVariation = m_gameConfig->GetMountainHeightVariation();
    int mountainSpacingInGroup = m_gameConfig->GetMountainSpacingInGroup();
    Color mountainColor = m_gameConfig->GetMountainColor();

    int mountainOffset = static_cast<int>(-m_cameraX * mountainParallaxSpeed) % mountainRepeatWidth;
    for (int x = mountainOffset - mountainRepeatWidth; x < screenWidth + mountainRepeatWidth; x += mountainSpacing) {
        // Draw mountain silhouettes
        for (int i = 0; i < mountainCountPerGroup; i++) {
            int mountainX = x + i * mountainSpacingInGroup;
            int mountainHeight = mountainBaseHeight + (i % 3) * mountainHeightVariation;
            int mountainY = 400 - mountainHeight; // Base at y=400
            renderer->DrawRectangle(Rectangle(mountainX, mountainY, mountainWidth, mountainHeight), mountainColor, true);
        }
    }

    // Draw buildings (parallax layer 2 - medium speed) using config values
    float buildingParallaxSpeed = m_gameConfig->GetBuildingParallaxSpeed();
    int buildingRepeatWidth = m_gameConfig->GetBuildingRepeatWidth();
    int buildingSpacing = m_gameConfig->GetBuildingSpacing();
    int buildingCountPerGroup = m_gameConfig->GetBuildingCountPerGroup();
    int buildingWidth = m_gameConfig->GetBuildingWidth();
    int buildingBaseHeight = m_gameConfig->GetBuildingBaseHeight();
    int buildingHeightVariation = m_gameConfig->GetBuildingHeightVariation();
    int buildingSpacingInGroup = m_gameConfig->GetBuildingSpacingInGroup();
    Color buildingColor = m_gameConfig->GetBuildingColor();

    int buildingOffset = static_cast<int>(-m_cameraX * buildingParallaxSpeed) % buildingRepeatWidth;
    for (int x = buildingOffset - buildingRepeatWidth; x < screenWidth + buildingRepeatWidth; x += buildingSpacing) {
        // Draw building silhouettes
        for (int i = 0; i < buildingCountPerGroup; i++) {
            int buildingX = x + i * buildingSpacingInGroup;
            int buildingHeight = buildingBaseHeight + (i % 4) * buildingHeightVariation;
            int buildingY = 420 - buildingHeight; // Base at y=420
            renderer->DrawRectangle(Rectangle(buildingX, buildingY, buildingWidth, buildingHeight), buildingColor, true);

            // Add windows using config values
            int windowCountHorizontal = m_gameConfig->GetWindowCountHorizontal();
            int windowWidth = m_gameConfig->GetWindowWidth();
            int windowHeight = m_gameConfig->GetWindowHeight();
            int windowSpacingX = m_gameConfig->GetWindowSpacingX();
            int windowSpacingY = m_gameConfig->GetWindowSpacingY();
            int windowOffsetX = m_gameConfig->GetWindowOffsetX();
            Color windowColor = m_gameConfig->GetWindowColor();

            for (int w = 0; w < windowCountHorizontal; w++) {
                for (int h = 0; h < buildingHeight / windowSpacingY; h++) {
                    if ((w + h) % 2 == 0) {
                        int windowX = buildingX + windowOffsetX + w * windowSpacingX;
                        int windowY = buildingY + 5 + h * windowSpacingY; // 5 pixel offset from building top
                        renderer->DrawRectangle(Rectangle(windowX, windowY, windowWidth, windowHeight), windowColor, true);
                    }
                }
            }
        }
    }
}

void PlayingState::UpdateScore() {
    // Increase score over time and for movement using config values
    int baseScore = m_gameConfig->GetBaseScorePerFrame();
    m_score += baseScore;

    // Bonus points for player movement
    if (m_player.IsValid() && m_entityManager) {
        auto* velocity = m_entityManager->GetComponent<VelocityComponent>(m_player);
        if (velocity && (velocity->vx != 0 || velocity->vy != 0)) {
            int movementBonus = m_gameConfig->GetMovementBonusScore();
            m_score += movementBonus;
        }
    }
}

void PlayingState::UpdatePlayerAnimation() {
    // Animation is handled directly in the Render() method
    // This keeps the animation logic close to the rendering code
    // for better maintainability in arcade games
}

void PlayingState::ResetGameState() {
    // Reset game variables to initial state
    m_cameraX = 0.0f;
    m_score = 0;
    m_gameTime = 0.0f;

    // Reset player position using current config
    m_playerX = m_gameConfig->GetPlayerStartX();
    m_playerY = m_gameConfig->GetPlayerStartY();
    m_playerVelX = 0.0f;
    m_playerVelY = 0.0f;

    // Recreate entities with new config values
    if (m_entityManager) {
        m_entityManager.reset();
        m_entityManager = std::make_unique<EntityManager>();

        // Re-add systems
        m_entityManager->AddSystem<MovementSystem>();
        m_entityManager->AddSystem<CollisionSystem>();

        // Reinitialize CharacterFactory
        m_characterFactory = std::make_unique<CharacterFactory>(m_entityManager.get());
        if (!m_characterFactory->LoadFromConfig("assets/config/characters.ini")) {
            std::cerr << "Warning: Failed to reload character configs" << std::endl;
        }

        if (GetEngine()->GetAudioManager()) {
            m_entityManager->AddSystem<AudioSystem>(*GetEngine()->GetAudioManager());
        }

        // Recreate entities
        CreatePlayer();
        CreateEnemies();
    }

    std::cout << "🔄 Game state reset for level: " <<
        (m_gameConfig->GetCurrentLevel().empty() ? "Base" : m_gameConfig->GetCurrentLevel()) << std::endl;
}

void PlayingState::CreateConfigAwareCharacter(const std::string& characterType, float x, float y, float difficultyMultiplier) {
    if (!m_entityManager || !m_characterFactory) return;

    // Create character using CharacterFactory
    Entity character = m_characterFactory->CreateCharacter(characterType, x, y);

    if (!character.IsValid()) {
        std::cout << "⚠️  Failed to create character: " << characterType << std::endl;
        return;
    }

    // Apply level-specific modifications using GameConfig

    // Health scaling based on difficulty
    if (auto* health = m_entityManager->GetComponent<HealthComponent>(character)) {
        health->maxHealth *= difficultyMultiplier;
        health->currentHealth = health->maxHealth;
        std::cout << "📊 " << characterType << " health set to: " << health->maxHealth << std::endl;
    }

    // Speed from config
    if (auto* velocity = m_entityManager->GetComponent<VelocityComponent>(character)) {
        float configSpeed = m_gameConfig->GetEnemyBaseVelocityX();
        velocity->vx = configSpeed * difficultyMultiplier;
        std::cout << "🏃 " << characterType << " speed set to: " << velocity->vx << std::endl;
    }

    // Size from config
    if (auto* render = m_entityManager->GetComponent<RenderComponent>(character)) {
        render->width = m_gameConfig->GetEnemyWidth();
        render->height = m_gameConfig->GetEnemyHeight();
        std::cout << "📏 " << characterType << " size: " << render->width << "x" << render->height << std::endl;
    }

    // Audio volume from config
    if (auto* audio = m_entityManager->GetComponent<AudioComponent>(character)) {
        audio->volume = m_gameConfig->GetCollisionSoundVolume();
    }

    // Position using config-aware transform
    if (auto* transform = m_entityManager->GetComponent<TransformComponent>(character)) {
        transform->x = x;
        transform->y = y;
    }

    std::cout << "✅ Created config-aware " << characterType << " at (" << x << ", " << y << ") with difficulty " << difficultyMultiplier << std::endl;
}
