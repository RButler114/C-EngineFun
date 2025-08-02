/**
 * @file Window.h
 * @brief Cross-platform window management using SDL2
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include <SDL2/SDL.h>
#include <string>

/**
 * @class Window
 * @brief Cross-platform window management wrapper for SDL2
 *
 * The Window class provides a high-level interface for creating and managing
 * application windows using SDL2. It handles window creation, resizing,
 * fullscreen toggling, and cleanup.
 *
 * @example
 * ```cpp
 * Window window;
 * if (window.Initialize("My Game", 800, 600)) {
 *     // Window created successfully
 *     SDL_Window* sdlWindow = window.GetSDLWindow();
 *     // Use with renderer...
 * }
 * ```
 */
class Window {
public:
    /**
     * @brief Default constructor
     *
     * Initializes member variables to default values.
     * Call Initialize() to create the actual window.
     */
    Window();

    /**
     * @brief Destructor
     *
     * Automatically calls Shutdown() to clean up SDL resources.
     */
    ~Window();

    /**
     * @brief Create and initialize the window
     *
     * Creates an SDL2 window with the specified parameters.
     * The window is created centered on the screen.
     *
     * @param title Window title to display in title bar
     * @param width Window width in pixels
     * @param height Window height in pixels
     * @return true if window creation successful, false otherwise
     *
     * @note SDL must be initialized before calling this function
     */
    bool Initialize(const char* title, int width, int height);

    /**
     * @brief Clean up window resources
     *
     * Destroys the SDL window and resets internal state.
     * Called automatically by destructor.
     */
    void Shutdown();

    // Getters
    /**
     * @brief Get the underlying SDL window handle
     * @return Pointer to SDL_Window, or nullptr if not initialized
     */
    SDL_Window* GetSDLWindow() const { return m_window; }

    /**
     * @brief Get the current window width
     * @return Window width in pixels
     */
    int GetWidth() const { return m_width; }

    /**
     * @brief Get the current window height
     * @return Window height in pixels
     */
    int GetHeight() const { return m_height; }

    /**
     * @brief Get the current window title
     * @return Reference to the title string
     */
    const std::string& GetTitle() const { return m_title; }

    // Window operations
    /**
     * @brief Change the window title
     * @param title New title to display
     */
    void SetTitle(const char* title);

    /**
     * @brief Resize the window
     * @param width New width in pixels
     * @param height New height in pixels
     */
    void SetSize(int width, int height);

    /**
     * @brief Toggle fullscreen mode
     * @param fullscreen true for fullscreen, false for windowed
     */
    void SetFullscreen(bool fullscreen);

    /**
     * @brief Check if window is in fullscreen mode
     * @return true if fullscreen, false if windowed
     */
    bool IsFullscreen() const { return m_isFullscreen; }

private:
    SDL_Window* m_window;       ///< SDL window handle
    int m_width;                ///< Current window width in pixels
    int m_height;               ///< Current window height in pixels
    std::string m_title;        ///< Current window title
    bool m_isFullscreen;        ///< Whether window is in fullscreen mode
};
