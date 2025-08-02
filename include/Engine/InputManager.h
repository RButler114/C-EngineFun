#pragma once

#include <SDL2/SDL.h>
#include <unordered_map>

enum class MouseButton {
    LEFT = SDL_BUTTON_LEFT,
    MIDDLE = SDL_BUTTON_MIDDLE,
    RIGHT = SDL_BUTTON_RIGHT
};

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
