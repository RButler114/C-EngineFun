/**
 * @file Window.cpp
 * @brief Implementation of cross-platform window management
 * @author Ryan Butler
 * @date 2025
 */

#include "Engine/Window.h"
#include <iostream>

/**
 * @brief Default constructor - initializes window to safe default state
 *
 * Sets all member variables to safe default values:
 * - No SDL window created yet (nullptr)
 * - Zero dimensions (will be set in Initialize())
 * - Windowed mode (not fullscreen)
 * - Empty title (will be set in Initialize())
 *
 * @note The actual window creation happens in Initialize()
 * @note This follows RAII principles - lightweight construction
 */
Window::Window()
    : m_window(nullptr)      // No SDL window created yet
    , m_width(0)             // Will be set in Initialize()
    , m_height(0)            // Will be set in Initialize()
    , m_isFullscreen(false)  // Start in windowed mode
{
    // Constructor is lightweight - actual window creation in Initialize()
}

/**
 * @brief Destructor - ensures proper cleanup of SDL window
 *
 * Automatically calls Shutdown() to destroy the SDL window and
 * release associated resources. This follows RAII principles.
 *
 * @note Safe to call even if Initialize() was never called
 * @note Prevents memory leaks and resource handle leaks
 */
Window::~Window() {
    Shutdown(); // Automatic cleanup via RAII
}

/**
 * @brief Create and configure the SDL window
 *
 * Creates an SDL2 window with the specified parameters and configuration:
 * - Window is created centered on the primary display
 * - Window is visible immediately (SDL_WINDOW_SHOWN)
 * - Window is resizable by the user (SDL_WINDOW_RESIZABLE)
 * - Window supports both windowed and fullscreen modes
 *
 * @param title Window title displayed in title bar (supports Unicode)
 * @param width Initial window width in pixels (minimum: 1)
 * @param height Initial window height in pixels (minimum: 1)
 *
 * @return true if window created successfully, false on failure
 *
 * @note SDL must be initialized with SDL_INIT_VIDEO before calling this
 * @note Window is created on the primary display, centered
 * @note Window supports resizing and fullscreen toggling
 *
 * Common failure causes:
 * - SDL not initialized
 * - Invalid dimensions (width/height <= 0)
 * - Insufficient video memory
 * - Display driver issues
 */
bool Window::Initialize(const char* title, int width, int height) {
    // Store window parameters for later use
    m_title = title;
    m_width = width;
    m_height = height;

    // Create SDL window with specified configuration
    m_window = SDL_CreateWindow(
        title,                          // Window title (displayed in title bar)
        SDL_WINDOWPOS_CENTERED,         // X position (centered on screen)
        SDL_WINDOWPOS_CENTERED,         // Y position (centered on screen)
        width,                          // Window width in pixels
        height,                         // Window height in pixels
        SDL_WINDOW_SHOWN |              // Make window visible immediately
        SDL_WINDOW_RESIZABLE            // Allow user to resize window
    );

    // Check if window creation was successful
    if (!m_window) {
        std::cerr << "❌ Window creation failed! SDL Error: " << SDL_GetError() << std::endl;
        std::cerr << "   Check display drivers and system resources" << std::endl;
        return false;
    }

    std::cout << "✅ Window created: \"" << title << "\" (" << width << "x" << height << ")" << std::endl;
    return true;
}

/**
 * @brief Clean up SDL window and reset state
 *
 * Safely destroys the SDL window and resets internal state:
 * - Destroys SDL window handle (releases video memory)
 * - Resets window pointer to nullptr (prevents double-free)
 * - Maintains other member variables for potential re-initialization
 *
 * @note Safe to call multiple times (checks for null pointer)
 * @note Called automatically by destructor
 * @note After this call, GetSDLWindow() returns nullptr
 */
void Window::Shutdown() {
    if (m_window) {
        SDL_DestroyWindow(m_window);    // Release SDL window resources
        m_window = nullptr;             // Prevent double-free
        std::cout << "✅ Window destroyed" << std::endl;
    }
    // Note: We keep m_width, m_height, m_title for potential re-initialization
}

/**
 * @brief Change the window title displayed in the title bar
 *
 * Updates both the internal title string and the actual window title
 * displayed by the operating system. The title change is immediate.
 *
 * @param title New title to display (supports Unicode characters)
 *
 * @note Does nothing if window is not initialized or title is null
 * @note Title is also stored internally for GetTitle() queries
 * @note Useful for displaying FPS, game state, or other dynamic info
 *
 * @example
 * ```cpp
 * window.SetTitle("My Game - FPS: 60");
 * window.SetTitle("🎮 Arcade Game 🎮");  // Unicode support
 * ```
 */
void Window::SetTitle(const char* title) {
    // Validate parameters before making changes
    if (m_window && title) {
        m_title = title;                        // Update internal title
        SDL_SetWindowTitle(m_window, title);    // Update OS window title
    }
}

/**
 * @brief Resize the window to new dimensions
 *
 * Changes the window size to the specified width and height.
 * The window content will be scaled/adjusted by the renderer.
 *
 * @param width New window width in pixels (must be > 0)
 * @param height New window height in pixels (must be > 0)
 *
 * @note Does nothing if window is not initialized or dimensions are invalid
 * @note Window content scaling is handled by the renderer
 * @note Triggers SDL resize events that can be handled by the application
 * @note Does not affect fullscreen mode (use SetFullscreen for that)
 *
 * @example
 * ```cpp
 * window.SetSize(1920, 1080);  // HD resolution
 * window.SetSize(800, 600);    // Classic 4:3 resolution
 * ```
 */
void Window::SetSize(int width, int height) {
    // Validate parameters before making changes
    if (m_window && width > 0 && height > 0) {
        m_width = width;                            // Update internal dimensions
        m_height = height;
        SDL_SetWindowSize(m_window, width, height); // Update actual window size
    }
}

/**
 * @brief Toggle between fullscreen and windowed mode
 *
 * Switches the window between fullscreen desktop mode and windowed mode:
 * - Fullscreen: Window covers entire screen, no borders/title bar
 * - Windowed: Normal window with borders, title bar, and controls
 *
 * Uses SDL_WINDOW_FULLSCREEN_DESKTOP which:
 * - Maintains desktop resolution (no mode switching)
 * - Faster transitions than exclusive fullscreen
 * - Better compatibility with multiple monitors
 * - Allows Alt+Tab switching
 *
 * @param fullscreen true for fullscreen mode, false for windowed mode
 *
 * @note Does nothing if window is not initialized
 * @note When exiting fullscreen, window is restored to original size and centered
 * @note Fullscreen state is tracked internally for IsFullscreen() queries
 *
 * @example
 * ```cpp
 * window.SetFullscreen(true);   // Go fullscreen
 * window.SetFullscreen(false);  // Return to windowed mode
 *
 * // Toggle fullscreen
 * window.SetFullscreen(!window.IsFullscreen());
 * ```
 */
void Window::SetFullscreen(bool fullscreen) {
    if (m_window) {
        // Choose appropriate SDL fullscreen flags
        Uint32 flags = fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;

        // Apply fullscreen mode change
        SDL_SetWindowFullscreen(m_window, flags);
        m_isFullscreen = fullscreen;

        // When exiting fullscreen, restore window to original size and position
        if (!fullscreen) {
            // Restore original window dimensions
            SDL_SetWindowSize(m_window, m_width, m_height);

            // Center the window on screen
            SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }
    }
}
