/**
 * @file InputManager.h
 * @brief Comprehensive input handling system for keyboard and mouse
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include <SDL2/SDL.h>
#include <unordered_map>

/**
 * @enum MouseButton
 * @brief Enumeration of mouse button types
 *
 * Maps to SDL mouse button constants for type safety and clarity.
 */
enum class MouseButton {
    LEFT = SDL_BUTTON_LEFT,     ///< Left mouse button
    MIDDLE = SDL_BUTTON_MIDDLE, ///< Middle mouse button (scroll wheel)
    RIGHT = SDL_BUTTON_RIGHT    ///< Right mouse button
};

/**
 * @class InputManager
 * @brief Comprehensive input handling system for keyboard and mouse input
 *
 * The InputManager provides a high-level interface for handling user input,
 * supporting both current state queries and edge detection (just pressed/released).
 * It maintains both current and previous frame state to enable proper event detection.
 *
 * Features:
 * - Keyboard state tracking with edge detection
 * - Mouse button state tracking with edge detection
 * - Mouse position and movement tracking
 * - Mouse wheel support
 * - Cursor visibility control
 * - Frame-based state management
 *
 * @example
 * ```cpp
 * InputManager inputManager;
 *
 * // In game loop:
 * inputManager.Update(); // Call before processing events
 *
 * // Handle SDL events
 * SDL_Event event;
 * while (SDL_PollEvent(&event)) {
 *     inputManager.HandleEvent(event);
 * }
 *
 * // Query input state
 * if (inputManager.IsKeyPressed(SDL_SCANCODE_SPACE)) {
 *     // Space is currently held down
 * }
 * if (inputManager.IsKeyJustPressed(SDL_SCANCODE_RETURN)) {
 *     // Enter was just pressed this frame
 * }
 * ```
 */
class InputManager {
public:
    InputManager();
    ~InputManager();

    void Update();
    bool HandleEvent(const SDL_Event& event);

    // Keyboard input
    bool IsKeyPressed(SDL_Scancode key) const;
    bool IsKeyJustPressed(SDL_Scancode key) const;
    bool IsKeyJustReleased(SDL_Scancode key) const;

    // Mouse input
    bool IsMouseButtonPressed(MouseButton button) const;
    bool IsMouseButtonJustPressed(MouseButton button) const;
    bool IsMouseButtonJustReleased(MouseButton button) const;
    
    void GetMousePosition(int& x, int& y) const;
    int GetMouseX() const { return m_mouseX; }
    int GetMouseY() const { return m_mouseY; }
    
    void GetMouseDelta(int& deltaX, int& deltaY) const;
    int GetMouseWheelDelta() const { return m_mouseWheelDelta; }

    // Utility functions
    void SetMousePosition(int x, int y);
    void ShowCursor(bool show);

private:
    // Keyboard state
    const Uint8* m_currentKeyState;
    Uint8* m_previousKeyState;
    int m_keyStateLength;

    // Mouse state
    std::unordered_map<MouseButton, bool> m_currentMouseState;
    std::unordered_map<MouseButton, bool> m_previousMouseState;
    
    int m_mouseX, m_mouseY;
    int m_previousMouseX, m_previousMouseY;
    int m_mouseWheelDelta;
    
    bool m_quitRequested;

public:
    bool IsQuitRequested() const { return m_quitRequested; }
};
