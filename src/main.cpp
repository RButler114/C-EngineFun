/**
 * @file main.cpp
 * @brief Main entry point for the C++ Everharvest Voyager V Engine
 * @author Ryan Butler
 * @date 2025
 *
 * Streamlined arcade game engine focused on classic side-scrolling gameplay.
 * Features:
 * - Game state management (Menu, Playing, Game Over, Options)
 * - Entity-Component-System architecture
 * - Sprite rendering and animation
 * - Audio system integration
 * - Input handling and player controls
 * - Collision detection
 * - Scrolling backgrounds
 */

#include "Engine/Engine.h"
#include "Engine/Window.h"
#include "Engine/Renderer.h"
#include "Engine/InputManager.h"
#include "Game/GameStateManager.h"
#include "Game/MenuState.h"
#include "Game/CustomizationState.h"
#include "Game/PlayingState.h"
#include "Game/CombatState.h"
#include "Game/GameOverState.h"
#include "Game/OptionsState.h"
#include "Game/CreditsState.h"
#include "Game/PauseState.h"
#include "Game/HighScoreState.h"

#include <iostream>
#include <random>
#include <chrono>

/**
 * @class ArcadeGame
 * @brief Streamlined arcade game implementation
 *
 * Classic arcade-style side-scrolling game with:
 * - Menu system
 * - Side-scrolling gameplay
 * - Player movement and sprite animation
 * - Enemy spawning and collision
 * - Scoring system
 * - Game over handling
 */
class ArcadeGame : public Engine {
public:
    /**
     * @brief Default constructor - initializes the arcade game instance
     *
     * Sets up the initial state with a null state manager pointer.
     * The actual initialization happens in the Initialize() method.
     *
     * @note This follows RAII principles - construction is lightweight,
     *       with heavy initialization deferred to Initialize()
     */
    ArcadeGame() : m_stateManager(nullptr) {}

    /**
     * @brief Initialize the arcade game engine and all game systems
     *
     * This is the main initialization function that sets up:
     * 1. Base engine systems (window, renderer, input, audio)
     * 2. Game state management system
     * 3. All available game states (menu, playing, combat, etc.)
     * 4. Initial game state (starts with main menu)
     *
     * @param title The window title to display (supports Unicode emojis)
     * @param width Window width in pixels (recommended: 800-1920)
     * @param height Window height in pixels (recommended: 600-1080)
     *
     * @return true if initialization succeeded, false if any system failed
     *
     * @note If this returns false, the game should not be run and the
     *       application should exit gracefully
     *
     * @example
     * ```cpp
     * ArcadeGame game;
     * if (!game.Initialize("My Game", 1024, 768)) {
     *     std::cerr << "Failed to start game!" << std::endl;
     *     return -1;
     * }
     * ```
     */
    bool Initialize(const char* title, int width, int height) {
        // Initialize the base engine systems (window, renderer, input, audio)
        // This must succeed before we can set up game-specific systems
        if (!Engine::Initialize(title, width, height)) {
            std::cerr << "Failed to initialize base engine systems" << std::endl;
            return false;
        }

        // Create the game state manager - this handles transitions between
        // different screens/modes of the game (menu, playing, combat, etc.)
        m_stateManager = std::make_unique<GameStateManager>(this);

        // Register all available game states with the state manager
        // Each state represents a different "screen" or mode of the game:

        // MENU: Main menu with options to start game, customize, view options
        m_stateManager->AddState(GameStateType::MENU, std::make_unique<MenuState>());

        // CUSTOMIZATION: Player character and settings customization
        m_stateManager->AddState(GameStateType::CUSTOMIZATION, std::make_unique<CustomizationState>());

        // PLAYING: Main arcade gameplay (side-scrolling action)
        m_stateManager->AddState(GameStateType::PLAYING, std::make_unique<PlayingState>());

        // COMBAT: Turn-based combat system (triggered by collisions)
        m_stateManager->AddState(GameStateType::COMBAT, std::make_unique<CombatState>());

        // PAUSED: FF10-style overlay menu for party/status/options
        m_stateManager->AddState(GameStateType::PAUSED, std::make_unique<PauseState>());

        // GAME_OVER: End game screen with score and restart options
        m_stateManager->AddState(GameStateType::GAME_OVER, std::make_unique<GameOverState>());

        // OPTIONS: Settings menu for audio, controls, graphics
        m_stateManager->AddState(GameStateType::OPTIONS, std::make_unique<OptionsState>());

        // CREDITS: Credits screen
        m_stateManager->AddState(GameStateType::CREDITS, std::make_unique<CreditsState>());

        // HIGH_SCORES: High scores screen
        m_stateManager->AddState(GameStateType::HIGH_SCORES, std::make_unique<HighScoreState>());


        // Start the game in the main menu state
        // Players will navigate from here to other states
        m_stateManager->PushState(GameStateType::MENU);

        std::cout << "✅ Game initialization complete - all systems ready!" << std::endl;
        return true;
    }

protected:
    /**
     * @brief Main game update loop - called every frame
     *
     * This function is called automatically by the engine's main loop.
     * It handles:
     * 1. Input processing for the current game state
     * 2. Game logic updates (movement, AI, physics, etc.)
     * 3. FPS display in window title (updated once per second)
     *
     * @param deltaTime Time elapsed since last frame in seconds
     *                  - Typical values: 0.016s (60 FPS) to 0.033s (30 FPS)
     *                  - Use this for frame-rate independent movement/animation
     *
     * @note This is called before Render() each frame
     * @note deltaTime allows smooth movement regardless of framerate:
     *       position += velocity * deltaTime (instead of just velocity)
     *
     * @example
     * ```cpp
     * // Frame-rate independent movement
     * float speed = 100.0f; // pixels per second
     * playerX += speed * deltaTime; // moves 100 pixels in 1 second
     * ```
     */
    void Update(float deltaTime) override {
        // Delegate input handling and updates to the current game state
        // The state manager ensures only the active state processes input/updates
        if (m_stateManager) {
            // Process keyboard, mouse, and gamepad input for current state
            m_stateManager->HandleInput();

            // Update game logic, animations, physics for current state
            m_stateManager->Update(deltaTime);
        }

        // Update window title with current FPS (frames per second)
        // This helps developers monitor performance during development
        static float titleUpdateTimer = 0.0f;
        titleUpdateTimer += deltaTime;

        // Only update title once per second to avoid flickering
        if (titleUpdateTimer >= 1.0f) {
            std::string title = "Everharvest Voyager V - FPS: " + std::to_string((int)GetFPS());
            GetWindow()->SetTitle(title.c_str());
            titleUpdateTimer = 0.0f; // Reset timer
        }
    }

    /**
     * @brief Main rendering function - called every frame after Update()
     *
     * This function is responsible for drawing everything visible on screen.
     * The rendering is delegated to the current game state, which knows
     * what should be drawn (menus, gameplay, UI, etc.).
     *
     * @note Called automatically by the engine after Update()
     * @note The renderer handles double-buffering and screen clearing
     * @note Each game state implements its own Render() method
     *
     * Typical rendering order within a state:
     * 1. Background/environment
     * 2. Game objects (sprites, enemies, player)
     * 3. UI elements (HUD, menus, text)
     * 4. Debug information (if enabled)
     */
    void Render() override {
        // Delegate rendering to the current active game state
        // Each state knows how to draw its specific content
        if (m_stateManager) {
            m_stateManager->Render();
        }

        // Note: The base Engine class handles:
        // - Screen clearing (black background)
        // - Buffer swapping (double buffering)
        // - VSync if enabled
    }

private:
    /**
     * @brief Game state manager - handles transitions between game screens
     *
     * This smart pointer manages the lifecycle of the GameStateManager,
     * which coordinates transitions between different game states like:
     * - Main menu
     * - Character customization
     * - Active gameplay
     * - Combat encounters
     * - Game over screen
     * - Options/settings
     *
     * @note Using unique_ptr ensures automatic cleanup when ArcadeGame is destroyed
     * @note The state manager is created in Initialize() and used throughout the game's lifetime
     */
    std::unique_ptr<GameStateManager> m_stateManager;
};

/**
 * @brief Application entry point - starts the arcade game
 *
 * This is where the program begins execution. It:
 * 1. Creates an instance of the arcade game
 * 2. Initializes all game systems
 * 3. Displays feature information to the console
 * 4. Starts the main game loop
 * 5. Handles any initialization failures gracefully
 *
 * @return 0 on successful execution, -1 if initialization fails
 *
 * @note This function demonstrates the typical game startup pattern:
 *       Create -> Initialize -> Run -> Cleanup (automatic)
 *
 * For new developers:
 * - The game loop runs until the user closes the window
 * - All cleanup happens automatically via RAII (destructors)
 * - Error handling ensures the program exits gracefully on failure
 */
int main() {
    // Welcome message with visual flair
    std::cout << "🎮 ARCADE GAME ENGINE 🎮" << std::endl;

    // Create the main game instance
    // This is lightweight - just sets up the object structure
    ArcadeGame game;

    // Initialize all game systems (window, renderer, audio, states, etc.)
    // This is where the heavy lifting happens - loading resources, setting up OpenGL, etc.
    if (!game.Initialize("🎮 Everharvest Voyager V 🎮", 800, 600)) {
        // If initialization fails, log the error and exit gracefully
        // Common causes: missing graphics drivers, audio issues, file permissions
        std::cerr << "❌ Failed to initialize game!" << std::endl;
        std::cerr << "   Check that your graphics drivers are up to date" << std::endl;
        std::cerr << "   and that the game has permission to access audio/graphics" << std::endl;
        return -1;
    }

    // Display feature list to help new developers understand what's available
    std::cout << "\n🎮 ARCADE GAME 🎮" << std::endl;
    std::cout << "Features implemented and ready to use:" << std::endl;
    std::cout << "  ✅ Menu system with navigation" << std::endl;
    std::cout << "  ✅ Game state management" << std::endl;
    std::cout << "  ✅ Side-scrolling gameplay" << std::endl;
    std::cout << "  ✅ Sprite animation" << std::endl;
    std::cout << "  ✅ Audio system" << std::endl;
    std::cout << "  ✅ Collision detection" << std::endl;
    std::cout << "\n🎯 Use WASD or Arrow Keys to move" << std::endl;
    std::cout << "🎯 Press ESC to access menus" << std::endl;
    std::cout << "🎯 Window shows FPS in title bar" << std::endl;

    // Start the main game loop - this runs until the user closes the window
    // The Run() method handles:
    // - Event processing (input, window events)
    // - Update() calls (game logic)
    // - Render() calls (drawing)
    // - Frame rate limiting/VSync
    std::cout << "\n🚀 Starting game loop..." << std::endl;
    game.Run();

    // Game loop has ended (user closed window)
    std::cout << "\n👋 Thanks for playing!" << std::endl;

    // Return success code
    // Note: All cleanup happens automatically via destructors (RAII)
    return 0;
}
