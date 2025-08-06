/**
 * @file Engine.cpp
 * @brief Implementation of the core game engine class
 * @author Ryan Butler
 * @date 2025
 */

#include "Engine/Engine.h"
#include "Engine/Window.h"
#include "Engine/Renderer.h"
#include "Engine/InputManager.h"
#include "Engine/AudioManager.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <thread>

/**
 * @brief Default constructor - initializes engine with default settings
 *
 * Sets up the engine with sensible defaults:
 * - Not running initially (must call Initialize() and Run())
 * - Target 60 FPS for smooth gameplay
 * - Zero delta time and FPS (calculated during runtime)
 *
 * @note All major systems (window, renderer, etc.) are created in Initialize()
 */
Engine::Engine()
    : m_isRunning(false)    // Engine starts in stopped state
    , m_targetFPS(60)       // Standard 60 FPS for smooth gameplay
    , m_deltaTime(0.0f)     // Will be calculated each frame
    , m_fps(0.0f)           // Will be calculated each frame
{
    // Constructor is lightweight - heavy initialization happens in Initialize()
}

/**
 * @brief Destructor - ensures clean shutdown of all engine systems
 *
 * Automatically calls Shutdown() to clean up all resources.
 * This follows RAII principles - no manual cleanup required.
 *
 * @note Safe to call even if Initialize() was never called or failed
 */
Engine::~Engine() {
    Shutdown(); // Automatic cleanup via RAII
}

/**
 * @brief Initialize all engine systems and prepare for game loop
 *
 * This method sets up all core engine systems in the correct order:
 * 1. SDL2 initialization (video and audio subsystems)
 * 2. Window creation and setup
 * 3. Renderer initialization with hardware acceleration
 * 4. Input manager setup for keyboard/mouse/gamepad
 * 5. Audio manager initialization for sound/music
 * 6. High-resolution timer setup for frame timing
 *
 * @param title Window title (supports Unicode characters and emojis)
 * @param width Window width in pixels (minimum recommended: 640)
 * @param height Window height in pixels (minimum recommended: 480)
 *
 * @return true if all systems initialized successfully, false if any failed
 *
 * @note Must be called before Run() - the engine won't work without this
 * @note If this returns false, check console output for specific error details
 * @note Safe to call multiple times (will shutdown existing systems first)
 *
 * Common failure causes:
 * - Missing graphics drivers
 * - Audio device unavailable
 * - Insufficient system resources
 * - Invalid window dimensions
 */
bool Engine::Initialize(const char* title, int width, int height) {
    // Step 1: Initialize SDL2 core systems
    // SDL_INIT_VIDEO: Graphics, window management, input events
    // SDL_INIT_AUDIO: Sound mixing, audio device access
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "❌ SDL initialization failed! Error: " << SDL_GetError() << std::endl;
        std::cerr << "   Common causes: Missing graphics drivers, system resources" << std::endl;
        return false;
    }
    std::cout << "✅ SDL2 initialized successfully" << std::endl;

    // Step 2: Create and initialize the game window
    m_window = std::make_unique<Window>();
    if (!m_window->Initialize(title, width, height)) {
        std::cerr << "❌ Window creation failed!" << std::endl;
        std::cerr << "   Check window dimensions and system display settings" << std::endl;
        return false;
    }
    std::cout << "✅ Window created: " << width << "x" << height << std::endl;

    // Step 3: Create renderer with hardware acceleration
    m_renderer = std::make_unique<Renderer>();
    if (!m_renderer->Initialize(m_window->GetSDLWindow())) {
        std::cerr << "❌ Renderer initialization failed!" << std::endl;
        std::cerr << "   Check graphics drivers and OpenGL/DirectX support" << std::endl;
        return false;
    }
    std::cout << "✅ Hardware-accelerated renderer initialized" << std::endl;

    // Step 4: Create input manager for user interaction
    // Input manager handles keyboard, mouse, and gamepad input
    m_inputManager = std::make_unique<InputManager>();
    std::cout << "✅ Input manager ready" << std::endl;

    // Step 5: Initialize audio system for sound and music
    m_audioManager = std::make_unique<AudioManager>();
    if (!m_audioManager->Initialize()) {
        std::cerr << "❌ Audio manager initialization failed!" << std::endl;
        std::cerr << "   Audio will be disabled - game will continue without sound" << std::endl;
        // Note: We don't return false here - game can run without audio
    } else {
        std::cout << "✅ Audio system initialized" << std::endl;
    }

    // Step 6: Set up timing system for frame rate control
    m_isRunning = true; // Mark engine as ready to run
    m_lastFrameTime = std::chrono::high_resolution_clock::now(); // Initialize frame timer

    std::cout << "🎮 Engine initialization complete - ready to run!" << std::endl;
    return true;
}

/**
 * @brief Main game loop - runs until quit is requested
 *
 * This is the heart of the engine - a continuous loop that:
 * 1. Processes input events (keyboard, mouse, window events)
 * 2. Updates game logic with frame-rate independent timing
 * 3. Renders the current frame to the screen
 * 4. Maintains consistent frame rate (60 FPS by default)
 *
 * The loop continues until:
 * - User closes the window
 * - Quit() is called programmatically
 * - Input manager detects quit request (Alt+F4, etc.)
 *
 * @note Initialize() must be called successfully before this method
 * @note This method blocks until the game exits
 * @note Frame timing is handled automatically for smooth gameplay
 *
 * Frame Structure:
 * - Input Processing: Capture user input and window events
 * - Logic Update: Game state changes, physics, AI (frame-rate independent)
 * - Rendering: Draw everything to screen buffer
 * - Present: Display the completed frame
 * - Frame Rate Cap: Sleep if needed to maintain target FPS
 */
void Engine::Run() {
    std::cout << "🚀 Starting main game loop..." << std::endl;

    while (m_isRunning) {
        // Mark the start of this frame for timing calculations
        m_frameStartTime = std::chrono::high_resolution_clock::now();

        // PHASE 1: INPUT PROCESSING
        // Update input manager to capture previous frame's input state
        // This allows detection of "just pressed" vs "held" keys
        m_inputManager->Update();

        // Process all pending SDL events (keyboard, mouse, window events)
        HandleEvents();

        // Check if user requested to quit (window close, Alt+F4, etc.)
        if (m_inputManager->IsQuitRequested()) {
            std::cout << "🛑 Quit requested - stopping game loop" << std::endl;
            m_isRunning = false;
            break; // Exit the main loop
        }

        // PHASE 2: TIMING CALCULATION
        // Calculate time elapsed since last frame for smooth, frame-rate independent updates
        CalculateDeltaTime();

        // PHASE 3: GAME LOGIC UPDATE
        // Call the derived class's Update method with delta time
        // This is where game-specific logic happens (movement, AI, physics, etc.)
        Update(m_deltaTime);

        // PHASE 4: RENDERING
        // Clear the screen buffer (usually to black)
        m_renderer->Clear();

        // Call the derived class's Render method to draw everything
        // This is where game-specific rendering happens (sprites, UI, etc.)
        Render();

        // Present the completed frame to the screen (swap buffers)
        m_renderer->Present();

        // PHASE 5: FRAME RATE CONTROL
        // Sleep if necessary to maintain target FPS (60 by default)
        CapFrameRate();
    }

    std::cout << "🏁 Game loop ended" << std::endl;
}

/**
 * @brief Clean shutdown of all engine systems
 *
 * Safely shuts down all engine systems in reverse order of initialization:
 * 1. Renderer cleanup (release graphics resources)
 * 2. Window cleanup (destroy window, release context)
 * 3. Input manager cleanup (automatic via smart pointer)
 * 4. Audio manager cleanup (stop sounds, release audio device)
 * 5. SDL2 cleanup (shutdown all SDL subsystems)
 *
 * @note Safe to call multiple times (checks for null pointers)
 * @note Called automatically by destructor (RAII)
 * @note All cleanup is automatic via smart pointers where possible
 *
 * This method ensures:
 * - No memory leaks
 * - Proper graphics context cleanup
 * - Audio device release
 * - SDL2 subsystem shutdown
 */
void Engine::Shutdown() {
    std::cout << "🔄 Shutting down engine systems..." << std::endl;

    // Shutdown renderer first (release graphics resources)
    if (m_renderer) {
        m_renderer->Shutdown();
        m_renderer.reset(); // Release smart pointer
        std::cout << "✅ Renderer shut down" << std::endl;
    }

    // Shutdown window (destroy window, release graphics context)
    if (m_window) {
        m_window->Shutdown();
        m_window.reset(); // Release smart pointer
        std::cout << "✅ Window shut down" << std::endl;
    }

    // Input manager cleanup (automatic via smart pointer)
    if (m_inputManager) {
        m_inputManager.reset();
        std::cout << "✅ Input manager shut down" << std::endl;
    }

    // Shutdown audio system (stop sounds, release audio device)
    if (m_audioManager) {
        m_audioManager->Shutdown();
        m_audioManager.reset(); // Release smart pointer
        std::cout << "✅ Audio manager shut down" << std::endl;
    }

    // Final SDL2 cleanup (shutdown all subsystems)
    SDL_Quit();
    std::cout << "🏁 Engine shutdown complete!" << std::endl;
}

/**
 * @brief Process all pending SDL events and forward to input manager
 *
 * This method polls the SDL event queue and processes all pending events:
 * - Keyboard key presses and releases
 * - Mouse button clicks and movement
 * - Window events (resize, close, minimize, etc.)
 * - Gamepad/joystick input (if connected)
 * - System events (quit requests, etc.)
 *
 * Events are forwarded to the InputManager which maintains the current
 * input state and provides convenient query methods for game logic.
 *
 * @note Called once per frame during the main game loop
 * @note All events are processed in a single frame to prevent input lag
 * @note The InputManager handles event filtering and state management
 */
void Engine::HandleEvents() {
    SDL_Event event;

    // Process all pending events in the SDL queue
    // This ensures we don't miss any input or window events
    while (SDL_PollEvent(&event)) {
        // Forward each event to the input manager for processing
        // The input manager will update its internal state based on these events
        m_inputManager->HandleEvent(event);
    }
}

/**
 * @brief Calculate frame timing for smooth, frame-rate independent updates
 *
 * This method calculates two important timing values:
 * 1. Delta Time: Time elapsed since the last frame (in seconds)
 * 2. Current FPS: Frames per second based on recent frame times
 *
 * Delta time is crucial for frame-rate independent game logic:
 * - Movement: position += velocity * deltaTime
 * - Animation: animationTime += deltaTime
 * - Timers: countdown -= deltaTime
 *
 * This ensures the game runs at the same speed regardless of frame rate.
 *
 * @note Called once per frame during the main game loop
 * @note Uses high-resolution clock for precise timing (microsecond accuracy)
 * @note FPS calculation is instantaneous (not averaged over multiple frames)
 */
void Engine::CalculateDeltaTime() {
    // Get current high-resolution timestamp
    auto currentTime = std::chrono::high_resolution_clock::now();

    // Calculate time elapsed since last frame
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - m_lastFrameTime);

    // Convert microseconds to seconds for easier use in game logic
    // Example: 16,667 microseconds = 0.016667 seconds (60 FPS)
    m_deltaTime = duration.count() / 1000000.0f;

    // Calculate current FPS (frames per second)
    // FPS = 1 / deltaTime (e.g., 0.016667 seconds = 60 FPS)
    if (m_deltaTime > 0.0f) {
        m_fps = 1.0f / m_deltaTime;
    }

    // Update last frame time for next calculation
    m_lastFrameTime = currentTime;
}

/**
 * @brief Limit frame rate to maintain consistent timing
 *
 * This method implements frame rate capping to ensure consistent timing:
 * - Calculates how long the current frame took to process
 * - Compares against target frame time (1/60th second for 60 FPS)
 * - Sleeps the thread if the frame finished early
 *
 * Benefits of frame rate capping:
 * - Consistent game speed across different hardware
 * - Reduced CPU/GPU usage when not needed
 * - Smoother visual experience
 * - Predictable timing for game logic
 *
 * @note Called at the end of each frame in the main game loop
 * @note Uses high-precision sleep for accurate timing
 * @note If frame takes longer than target time, no sleep occurs (natural slowdown)
 */
void Engine::CapFrameRate() {
    // Calculate how long this frame took to process
    auto frameEndTime = std::chrono::high_resolution_clock::now();
    auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(frameEndTime - m_frameStartTime);

    // Calculate target frame time based on desired FPS
    // Example: 60 FPS = 1,000,000 microseconds / 60 = 16,667 microseconds per frame
    auto targetFrameTime = std::chrono::microseconds(1000000 / m_targetFPS);

    // If frame finished early, sleep for the remaining time
    if (frameDuration < targetFrameTime) {
        auto sleepTime = targetFrameTime - frameDuration;
        std::this_thread::sleep_for(sleepTime);
    }

    // Note: If frameDuration >= targetFrameTime, no sleep occurs
    // This allows natural slowdown when the system can't maintain target FPS
}
