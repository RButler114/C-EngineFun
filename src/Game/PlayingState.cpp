#include "Game/PlayingState.h"
#include "Game/CombatState.h"
#include "Game/GameStateManager.h"
#include "Game/GameOverState.h"
#include "Game/AnimationFactory.h"
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
#include <chrono>
#include <fstream>

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
    , m_playerVelY(0.0f)
    , m_collisionCooldown(0.0f)
    , m_pendingCombat(false)
    , m_combatPlayer()  // Initialize to invalid entity (ID 0)
    , m_combatEnemy() { // Initialize to invalid entity (ID 0)

    // Load configuration
    if (!m_gameConfig->LoadConfigs()) {
        std::cerr << "Warning: Failed to load some game configs, using defaults" << std::endl;
    }

    // Set initial player position from config
    m_playerX = m_gameConfig->GetPlayerStartX();
    m_playerY = m_gameConfig->GetPlayerStartY();

    // Initialize collision cooldown
    m_collisionCooldown = 0.0f;
}

PlayingState::~PlayingState() = default;

void PlayingState::OnEnter() {
    std::cout << "Entering Playing State" << std::endl;
    
    // Initialize ECS
    m_entityManager = std::make_unique<EntityManager>();

    // Add core systems for arcade gameplay
    m_entityManager->AddSystem<MovementSystem>();
    auto* collisionSystem = m_entityManager->AddSystem<CollisionSystem>();

    // Add animation system for sprite animations
    auto* animationSystem = m_entityManager->AddSystem<AnimationSystem>();

    // Add sprite render system for rendering animated sprites
    std::cout << "🎨 DEBUG: Adding SpriteRenderSystem..." << std::endl;
    auto* spriteRenderSystem = m_entityManager->AddSystem<SpriteRenderSystem>(GetRenderer());
    if (spriteRenderSystem) {
        spriteRenderSystem->SetScreenDimensions(m_gameConfig->GetScreenWidth(), m_gameConfig->GetScreenHeight());
        std::cout << "✅ DEBUG: SpriteRenderSystem added successfully" << std::endl;
    } else {
        std::cout << "❌ ERROR: Failed to add SpriteRenderSystem!" << std::endl;
    }

    // Set up collision callback for combat triggering
    collisionSystem->SetCollisionCallback([this](const CollisionInfo& info) {
        OnCollision(info);
    });

    // Set up animation event callback for gameplay integration
    animationSystem->SetAnimationEventCallback([this](Entity entity, const std::string& animationName,
                                                      const std::string& eventType, int frameIndex) {
        OnAnimationEvent(entity, animationName, eventType, frameIndex);
    });

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

    // Debug: Test collision detection setup
    std::cout << "=== COLLISION DETECTION TEST ===" << std::endl;
    auto entities = m_entityManager->GetEntitiesWith<TransformComponent, CollisionComponent>();
    std::cout << "Found " << entities.size() << " entities with collision components:" << std::endl;

    for (Entity entity : entities) {
        auto* transform = m_entityManager->GetComponent<TransformComponent>(entity);
        auto* collision = m_entityManager->GetComponent<CollisionComponent>(entity);
        auto* charType = m_entityManager->GetComponent<CharacterTypeComponent>(entity);

        std::cout << "  Entity " << entity.GetID() << ": pos(" << transform->x << ", " << transform->y
                  << ") size(" << collision->width << "x" << collision->height << ")";
        if (charType) {
            std::cout << " type=" << static_cast<int>(charType->type);
        }
        std::cout << std::endl;
    }
    std::cout << "=================================" << std::endl;

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

    // Update collision cooldown
    if (m_collisionCooldown > 0.0f) {
        m_collisionCooldown -= deltaTime;
    }

    // Process pending combat (deferred from collision callback to avoid threading issues)
    if (m_pendingCombat) {
        std::cout << "🎯 Processing deferred combat in main thread" << std::endl;

        // Safety check: ensure combat entities are valid before processing
        if (!m_combatPlayer.IsValid() || !m_combatEnemy.IsValid()) {
            std::cerr << "Error: Invalid combat entities detected, skipping combat (Player: "
                      << m_combatPlayer.GetID() << ", Enemy: " << m_combatEnemy.GetID() << ")" << std::endl;
            m_pendingCombat = false;
            return;
        }

        // Play collision sound (safe to do in main thread)
        if (GetEngine()->GetAudioManager()) {
            GetEngine()->GetAudioManager()->PlaySound("collision", m_gameConfig->GetCollisionSoundVolume());
        }

        // Trigger combat (safe to do in main thread)
        TriggerCombat(m_combatPlayer, m_combatEnemy);

        // Clear pending state
        m_pendingCombat = false;
    }

    // Update ECS
    if (m_entityManager) {
        // Update player animation based on movement before ECS update
        UpdatePlayerAnimation();

        // Update camera for sprite render system
        auto* spriteRenderSystem = m_entityManager->GetSystem<SpriteRenderSystem>();
        if (spriteRenderSystem) {
            spriteRenderSystem->SetCameraOffset(m_cameraX, 0.0f);
        } else {
            static int errorCount = 0;
            if (errorCount < 5) { // Only show first 5 errors to avoid spam
                std::cout << "⚠️  WARNING: SpriteRenderSystem not found in EntityManager!" << std::endl;
                errorCount++;
            }
        }

        m_entityManager->Update(deltaTime);
    }



    // TEMPORARY: Auto-test movement to verify sprite visibility
    static float testTimer = 0.0f;
    static int testPhase = 0;
    testTimer += deltaTime;

    if (testPhase == 0 && testTimer >= 2.0f) {
        std::cout << "🧪 AUTO-MOVEMENT: Testing right movement" << std::endl;
        m_playerVelX = m_gameConfig->GetPlayerMovementSpeed();
        testPhase = 1;
        testTimer = 0.0f;
    } else if (testPhase == 1 && testTimer >= 3.0f) {
        std::cout << "🧪 AUTO-MOVEMENT: Testing left movement" << std::endl;
        m_playerVelX = -m_gameConfig->GetPlayerMovementSpeed();
        testPhase = 2;
        testTimer = 0.0f;
    } else if (testPhase == 2 && testTimer >= 3.0f) {
        std::cout << "🧪 AUTO-MOVEMENT: Stopping movement" << std::endl;
        m_playerVelX = 0;
        testPhase = 3;
        testTimer = 0.0f;
    }

    // Sync simple movement variables to ECS VelocityComponent (FIX for movement issue)
    if (m_player.IsValid() && m_entityManager) {
        auto* velocity = m_entityManager->GetComponent<VelocityComponent>(m_player);
        if (velocity) {
            // Debug movement sync
            if (m_playerVelX != 0 || m_playerVelY != 0) {
                std::cout << "🏃 MOVEMENT: Setting velocity (" << m_playerVelX << ", " << m_playerVelY << ") to ECS" << std::endl;
            }
            velocity->vx = m_playerVelX;
            velocity->vy = m_playerVelY;
        } else {
            std::cout << "❌ ERROR: Player has no VelocityComponent!" << std::endl;
        }
    }

    // Let the ECS MovementSystem handle position updates
    // Get updated position from ECS after MovementSystem processes it
    if (m_player.IsValid() && m_entityManager) {
        auto* transform = m_entityManager->GetComponent<TransformComponent>(m_player);
        if (transform) {
            float oldX = m_playerX;
            float oldY = m_playerY;
            m_playerX = transform->x;
            m_playerY = transform->y;

            // Debug position changes
            if (oldX != m_playerX || oldY != m_playerY) {
                std::cout << "📍 POSITION: Player moved from (" << oldX << ", " << oldY << ") to (" << m_playerX << ", " << m_playerY << ")" << std::endl;
            }
        }
    }

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

    // Position sync is now handled above - ECS MovementSystem updates TransformComponent

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

    // Player sprite rendering is now handled by SpriteRenderSystem
    // Position is managed by ECS MovementSystem

    // Optional: Draw fallback shapes if sprite system fails
    int spriteWidth = m_gameConfig->GetAnimationSpriteWidth();
    if (playerScreenX > -spriteWidth && playerScreenX < screenWidth + spriteWidth) {
        // Check if we need fallback rendering
        auto texture = renderer->LoadTexture("assets/sprites/player/little_adventurer.png");
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
    
    // Player movement (now properly synced with ECS)
    m_playerVelX = 0;
    m_playerVelY = 0;

    const float speed = m_gameConfig->GetPlayerMovementSpeed();

    // Horizontal movement
    if (input->IsKeyPressed(SDL_SCANCODE_LEFT) || input->IsKeyPressed(SDL_SCANCODE_A)) {
        m_playerVelX = -speed;
        std::cout << "⬅️ INPUT: LEFT/A key pressed - velocity set to " << m_playerVelX << std::endl;
    }
    if (input->IsKeyPressed(SDL_SCANCODE_RIGHT) || input->IsKeyPressed(SDL_SCANCODE_D)) {
        m_playerVelX = speed;
        std::cout << "➡️ INPUT: RIGHT/D key pressed - velocity set to " << m_playerVelX << std::endl;
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

    // Sync input-driven velocity to ECS VelocityComponent
    if (m_player.IsValid() && m_entityManager) {
        auto* velocity = m_entityManager->GetComponent<VelocityComponent>(m_player);
        if (velocity) {
            velocity->vx = m_playerVelX;
            velocity->vy = m_playerVelY;
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

    // Get or update existing transform component (CharacterFactory already created it)
    auto* transform = m_entityManager->GetComponent<TransformComponent>(m_player);
    if (transform) {
        // Update existing transform to correct position
        transform->x = m_playerX;
        transform->y = m_playerY;
        std::cout << "DEBUG: Updated existing TransformComponent to (" << transform->x << ", " << transform->y << ")" << std::endl;
    } else {
        // This shouldn't happen since CharacterFactory should create it
        std::cerr << "ERROR: Player has no TransformComponent from CharacterFactory!" << std::endl;
        transform = m_entityManager->AddComponent<TransformComponent>(m_player, m_playerX, m_playerY);
        std::cout << "DEBUG: Added new TransformComponent at (" << transform->x << ", " << transform->y << ")" << std::endl;
    }

    // Update existing sprite component for animated rendering (CharacterFactory already created it)
    int spriteWidth = m_gameConfig->GetAnimationSpriteWidth();
    int spriteHeight = m_gameConfig->GetAnimationSpriteHeight();
    float spriteScale = m_gameConfig->GetAnimationSpriteScale();

    auto* sprite = m_entityManager->GetComponent<SpriteComponent>(m_player);
    if (sprite) {
        std::cout << "✅ DEBUG: Found existing SpriteComponent for player entity " << m_player.GetID() << std::endl;
        std::cout << "🎨 DEBUG: Sprite path: '" << sprite->texturePath << "', visible: " << sprite->visible << std::endl;
        std::cout << "📏 DEBUG: Sprite dimensions: " << sprite->width << "x" << sprite->height << ", scale: " << sprite->scaleX << "x" << sprite->scaleY << std::endl;

        // Force correct sprite path if it's wrong
        if (sprite->texturePath != "assets/sprites/player/little_adventurer.png") {
            std::cout << "🔧 FIXING: Correcting sprite path from '" << sprite->texturePath << "'" << std::endl;
            sprite->texturePath = "assets/sprites/player/little_adventurer.png";
            sprite->visible = true;
            std::cout << "✅ FIXED: Sprite path now: '" << sprite->texturePath << "', visible: " << sprite->visible << std::endl;
        }

        // Fix sprite scale - make it much larger so it's visible
        if (sprite->scaleX < 2.0f || sprite->scaleY < 2.0f) {
            std::cout << "🔧 FIXING: Sprite too small (" << sprite->scaleX << "x" << sprite->scaleY << "), increasing scale" << std::endl;
            sprite->scaleX = 4.0f;  // Make it 4x larger
            sprite->scaleY = 4.0f;
            std::cout << "✅ FIXED: Sprite scale now: " << sprite->scaleX << "x" << sprite->scaleY << std::endl;
        }

        // Verify file exists
        std::ifstream file(sprite->texturePath);
        if (file.good()) {
            std::cout << "📁 FILE CHECK: Sprite file exists at '" << sprite->texturePath << "'" << std::endl;
        } else {
            std::cout << "❌ FILE CHECK: Sprite file NOT FOUND at '" << sprite->texturePath << "'" << std::endl;
        }
    } else {
        std::cout << "❌ ERROR: Player entity doesn't have SpriteComponent from CharacterFactory!" << std::endl;
    }

    std::cout << "🔧 DEBUG: Finished checking SpriteComponent, about to update other components..." << std::endl;

    // CharacterFactory already added all necessary components, just update them if needed
    std::cout << "🔧 DEBUG: About to update CollisionComponent..." << std::endl;

    // Update collision component size if it exists
    auto* collision = m_entityManager->GetComponent<CollisionComponent>(m_player);
    if (collision) {
        collision->width = 32.0f;
        collision->height = 48.0f;
        std::cout << "✅ DEBUG: Updated CollisionComponent size" << std::endl;
    } else {
        std::cout << "❌ DEBUG: No CollisionComponent found" << std::endl;
    }

    std::cout << "🔧 DEBUG: About to update CombatStatsComponent..." << std::endl;

    // Update combat stats if they exist
    auto* combatStats = m_entityManager->GetComponent<CombatStatsComponent>(m_player);
    if (combatStats) {
        combatStats->attackPower = m_gameConfig->GetBaseAttackDamage();
        combatStats->defense = m_gameConfig->GetBaseDefense();
        combatStats->speed = m_gameConfig->GetBaseSpeed();
        std::cout << "✅ DEBUG: Updated CombatStatsComponent" << std::endl;
    } else {
        std::cout << "❌ DEBUG: No CombatStatsComponent found" << std::endl;
    }

    std::cout << "🔧 DEBUG: Finished CreatePlayer function" << std::endl;

    // // Add animation component with player animations
    // auto* animComp = m_entityManager->AddComponent<AnimationComponent>(m_player);
    // auto playerAnimations = AnimationFactory::CreatePlayerAnimations(spriteWidth, spriteHeight,
    //                                                                  m_gameConfig->GetAnimationFrameDuration());
    //
    // // Use traditional iterator approach instead of structured binding to avoid C++17 issues
    // std::cout << "🎬 DEBUG: Copying " << playerAnimations.size() << " animations to AnimationComponent..." << std::endl;
    // for (auto it = playerAnimations.begin(); it != playerAnimations.end(); ++it) {
    //     const std::string& name = it->first;
    //     const Animation& anim = it->second;
    //     std::cout << "🎬 DEBUG: Copying animation '" << name << "' with " << anim.frames.size() << " frames" << std::endl;
    //     animComp->animations[name] = anim;
    //     std::cout << "🎬 DEBUG: Successfully copied animation '" << name << "'" << std::endl;
    // }
    // std::cout << "🎬 DEBUG: All animations copied successfully" << std::endl;

    // // Create animation state machine and start with idle
    // auto* animSystem = m_entityManager->GetSystem<AnimationSystem>();
    // if (animSystem) {
    //     animSystem->CreateAnimationStateMachine(m_player, AnimationState::IDLE);
    // }

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

        // Ensure enemies spawn at safe distance from player
        float minDistanceFromPlayer = 150.0f; // Minimum safe distance
        if (x < m_gameConfig->GetPlayerStartX() + minDistanceFromPlayer) {
            x = m_gameConfig->GetPlayerStartX() + minDistanceFromPlayer + i * 50.0f;
            std::cout << "DEBUG: Adjusted enemy " << i << " position to safe distance at (" << x << ", " << y << ")" << std::endl;
        }

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
        (void)audio; // Suppress unused variable warning

        // Add collision component for combat triggering
        auto* collision = m_entityManager->AddComponent<CollisionComponent>(enemy,
            static_cast<float>(enemyWidth), static_cast<float>(enemyHeight));
        (void)collision; // Suppress unused variable warning

        // Add character type component to identify as enemy
        auto* charType = m_entityManager->AddComponent<CharacterTypeComponent>(enemy,
            CharacterTypeComponent::CharacterType::ENEMY,
            CharacterTypeComponent::CharacterClass::MONSTER,
            "Enemy");

        std::cout << "DEBUG: Added CharacterTypeComponent to enemy " << i << " - type: " << static_cast<int>(charType->type) << std::endl;

        // Add health component
        auto* health = m_entityManager->AddComponent<HealthComponent>(enemy, 50.0f);
        (void)health; // Suppress unused variable warning

        // Add combat stats component
        auto* combatStats = m_entityManager->AddComponent<CombatStatsComponent>(enemy);
        combatStats->attackPower = m_gameConfig->GetBaseAttackDamage() * 0.8f; // Slightly weaker
        combatStats->defense = m_gameConfig->GetBaseDefense() * 0.5f;
        combatStats->speed = m_gameConfig->GetBaseSpeed() * 0.9f;

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
    // Update player animation state based on movement
    if (!m_player.IsValid() || !m_entityManager) return;

    auto* animSystem = m_entityManager->GetSystem<AnimationSystem>();
    if (!animSystem) return;

    // Determine animation state based on player velocity
    float movementThreshold = m_gameConfig->GetPlayerMovementThreshold();
    bool isMoving = (std::abs(m_playerVelX) > movementThreshold || std::abs(m_playerVelY) > movementThreshold);

    AnimationState currentState = animSystem->GetCurrentState(m_player);
    AnimationState newState = currentState;

    if (isMoving) {
        // Check if jumping/falling
        if (std::abs(m_playerVelY) > movementThreshold) {
            newState = (m_playerVelY < 0) ? AnimationState::JUMPING : AnimationState::FALLING;
        } else {
            // Walking horizontally
            newState = AnimationState::WALKING;
        }
    } else {
        // Not moving - idle
        newState = AnimationState::IDLE;
    }

    // Transition to new state if different
    if (newState != currentState) {
        animSystem->TransitionToState(m_player, newState);
    }

    // Update sprite flipping based on horizontal movement
    auto* sprite = m_entityManager->GetComponent<SpriteComponent>(m_player);
    if (sprite && std::abs(m_playerVelX) > movementThreshold) {
        sprite->flipHorizontal = (m_playerVelX < 0);
    }
}

void PlayingState::OnAnimationEvent(Entity entity, const std::string& animationName,
                                   const std::string& eventType, int frameIndex) {
    // Handle animation events for gameplay integration
    (void)frameIndex; // Suppress unused parameter warning

    if (entity == m_player) {
        if (eventType == "start") {
            std::cout << "Player started animation: " << animationName << std::endl;
        } else if (eventType == "end") {
            std::cout << "Player finished animation: " << animationName << std::endl;

            // Handle non-looping animations
            if (animationName == "attack" || animationName == "hurt") {
                // Return to idle after attack or hurt animation
                auto* animSystem = m_entityManager->GetSystem<AnimationSystem>();
                if (animSystem) {
                    animSystem->TransitionToState(m_player, AnimationState::IDLE);
                }
            }
        }
    }
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
        auto* collisionSystem = m_entityManager->AddSystem<CollisionSystem>();

        // Re-set collision callback
        collisionSystem->SetCollisionCallback([this](const CollisionInfo& info) {
            OnCollision(info);
        });

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

void PlayingState::OnCollision(const CollisionInfo& info) {
    std::cout << "Collision detected between entities " << info.entityA.GetID() << " and " << info.entityB.GetID() << std::endl;

    // Check collision cooldown to prevent immediate re-triggering
    if (m_collisionCooldown > 0.0f) {
        std::cout << "Collision ignored - cooldown active (" << m_collisionCooldown << "s remaining)" << std::endl;
        return;
    }

    // Check if this is a player-enemy collision using actual player entity
    Entity player, enemy;
    bool isPlayerEnemyCollision = false;

    uint32_t playerID = m_player.GetID();

    if (info.entityA.GetID() == playerID) {
        // Entity A is player, Entity B might be enemy
        player = info.entityA;
        enemy = info.entityB;
        isPlayerEnemyCollision = true;
        std::cout << "Player-Enemy collision detected (A=Player:" << playerID << ", B=Enemy:" << enemy.GetID() << ")" << std::endl;
    } else if (info.entityB.GetID() == playerID) {
        // Entity B is player, Entity A might be enemy
        player = info.entityB;
        enemy = info.entityA;
        isPlayerEnemyCollision = true;
        std::cout << "Player-Enemy collision detected (A=Enemy:" << enemy.GetID() << ", B=Player:" << playerID << ")" << std::endl;
    } else {
        std::cout << "Not a player-enemy collision (IDs: " << info.entityA.GetID() << ", " << info.entityB.GetID() << ", Player:" << playerID << ")" << std::endl;
    }

    if (isPlayerEnemyCollision) {
        std::cout << "🎯 COMBAT TRIGGERED! Player vs Enemy " << enemy.GetID() << std::endl;

        // Set collision cooldown to prevent immediate re-triggering
        m_collisionCooldown = COLLISION_COOLDOWN_TIME;

        // DEFER audio and state operations to main thread to avoid macOS threading issues
        // Store the combat trigger for processing in the main Update loop
        m_pendingCombat = true;
        m_combatPlayer = player;
        m_combatEnemy = enemy;

        std::cout << "🎯 Combat deferred to main thread to avoid threading issues" << std::endl;
    }
}

void PlayingState::TriggerCombat(Entity player, Entity enemy) {
    // Safety check: ensure entities are valid
    if (!player.IsValid() || !enemy.IsValid()) {
        std::cerr << "Error: Cannot trigger combat with invalid entities (Player: "
                  << player.GetID() << ", Enemy: " << enemy.GetID() << ")" << std::endl;
        return;
    }

    // Store current player position for return after combat
    float returnX = m_playerX;
    float returnY = m_playerY;

    // Get the combat state
    auto* stateManager = GetStateManager();
    if (!stateManager) {
        std::cerr << "Error: No state manager available for combat transition" << std::endl;
        return;
    }

    std::cout << "Entering combat state..." << std::endl;

    // Push combat state
    stateManager->PushState(GameStateType::COMBAT);

    // Get the combat state and initialize it with the actual entities
    auto* combatState = dynamic_cast<CombatState*>(stateManager->GetCurrentState());
    if (combatState) {
        std::vector<Entity> enemies = {enemy};
        combatState->InitializeCombat(player, enemies);
        combatState->SetReturnPosition(returnX, returnY);
        std::cout << "Combat initialized with player " << player.GetID() << " vs enemy " << enemy.GetID() << std::endl;
    } else {
        std::cerr << "Error: Could not get CombatState after pushing" << std::endl;
    }
}
