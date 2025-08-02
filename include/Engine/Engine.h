/**
 * @file Engine.h
 * @brief Core game engine class providing window management, rendering, and game loop functionality
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include <memory>
#include <chrono>

class Window;
class Renderer;
class InputManager;

/**
 * @class Engine
 * @brief Main game engine class that manages the core game loop, window, renderer, and input systems
 *
 * The Engine class provides a complete game framework with:
 * - Cross-platform window management via SDL2
 * - 60 FPS game loop with delta time calculation
 * - Input handling and event processing
 * - Rendering system integration
 * - Frame rate capping and performance monitoring
 *
 * @example
 * ```cpp
 * class MyGame : public Engine {
 * protected:
 *     void Update(float deltaTime) override {
 *         // Game logic here
 *     }
 *     void Render() override {
 *         // Rendering here
 *     }
 * };
 *
 * MyGame game;
 * if (game.Initialize("My Game", 800, 600)) {
 *     game.Run();
 * }
 * ```
 */
class Engine {
public:
    /**
     * @brief Default constructor
     *
     * Initializes the engine with default settings:
     * - Target FPS: 60
     * - Running state: false
     * - Delta time: 0.0f
     */
    Engine();

    /**
     * @brief Destructor
     *
     * Automatically calls Shutdown() to clean up resources
     */
    ~Engine();

    /**
     * @brief Initialize the engine with window and rendering systems
     *
     * Sets up SDL2, creates window, renderer, and input manager.
     * Must be called before Run().
     *
     * @param title Window title to display
     * @param width Window width in pixels
     * @param height Window height in pixels
     * @return true if initialization successful, false otherwise
     *
     * @note This function must be called before Run()
     * @see Run(), Shutdown()
     */
    bool Initialize(const char* title, int width, int height);

    /**
     * @brief Start the main game loop
     *
     * Runs the game loop until Quit() is called or window is closed.
     * Handles events, updates game logic, and renders frames at target FPS.
     *
     * @note Initialize() must be called first
     * @see Initialize(), Quit()
     */
    void Run();

    /**
     * @brief Clean up all engine resources
     *
     * Shuts down renderer, window, input manager, and SDL2.
     * Called automatically by destructor.
     *
     * @see Initialize()
     */
    void Shutdown();

    // Getters
    /**
     * @brief Get the window instance
     * @return Pointer to the Window object, or nullptr if not initialized
     */
    Window* GetWindow() const { return m_window.get(); }

    /**
     * @brief Get the renderer instance
     * @return Pointer to the Renderer object, or nullptr if not initialized
     */
    Renderer* GetRenderer() const { return m_renderer.get(); }

    /**
     * @brief Get the input manager instance
     * @return Pointer to the InputManager object, or nullptr if not initialized
     */
    InputManager* GetInputManager() const { return m_inputManager.get(); }

    /**
     * @brief Get the time elapsed since last frame
     * @return Delta time in seconds as a float
     */
    float GetDeltaTime() const { return m_deltaTime; }

    /**
     * @brief Get the current frames per second
     * @return Current FPS as a float
     */
    float GetFPS() const { return m_fps; }

    /**
     * @brief Set the target frames per second
     * @param fps Target FPS (default: 60)
     */
    void SetTargetFPS(int fps) { m_targetFPS = fps; }

    /**
     * @brief Check if the engine is currently running
     * @return true if the game loop is active, false otherwise
     */
    bool IsRunning() const { return m_isRunning; }

    /**
     * @brief Stop the game loop
     *
     * Sets the running state to false, causing Run() to exit
     * on the next frame.
     */
    void Quit() { m_isRunning = false; }

protected:
    /**
     * @brief Update game logic (override in derived classes)
     *
     * Called once per frame before Render(). Override this method
     * to implement your game's update logic.
     *
     * @param deltaTime Time elapsed since last frame in seconds
     *
     * @note This is a virtual method that should be overridden by derived classes
     */
    virtual void Update(float deltaTime) { (void)deltaTime; }

    /**
     * @brief Render game graphics (override in derived classes)
     *
     * Called once per frame after Update(). Override this method
     * to implement your game's rendering logic.
     *
     * @note This is a virtual method that should be overridden by derived classes
     * @note The renderer is automatically cleared before this call and presented after
     */
    virtual void Render() {}

private:
    /**
     * @brief Process SDL events and update input manager
     *
     * Polls SDL events and forwards them to the input manager
     * for processing. Called once per frame.
     */
    void HandleEvents();

    /**
     * @brief Calculate delta time and FPS
     *
     * Updates m_deltaTime with the time elapsed since last frame
     * and calculates current FPS. Called once per frame.
     */
    void CalculateDeltaTime();

    /**
     * @brief Limit frame rate to target FPS
     *
     * Sleeps the thread if necessary to maintain the target frame rate.
     * Called at the end of each frame.
     */
    void CapFrameRate();

    std::unique_ptr<Window> m_window;           ///< Window management system
    std::unique_ptr<Renderer> m_renderer;       ///< Rendering system
    std::unique_ptr<InputManager> m_inputManager; ///< Input handling system

    bool m_isRunning;                           ///< Whether the game loop is active
    int m_targetFPS;                            ///< Target frames per second (default: 60)
    float m_deltaTime;                          ///< Time elapsed since last frame in seconds
    float m_fps;                                ///< Current frames per second

    /// High-resolution timestamp of the previous frame
    std::chrono::high_resolution_clock::time_point m_lastFrameTime;
    /// High-resolution timestamp when current frame started
    std::chrono::high_resolution_clock::time_point m_frameStartTime;
};
