#include "Engine/InputManager.h"
#include <cstring>
#include <iostream>

InputManager::InputManager()
    : m_currentKeyState(nullptr)
    , m_previousKeyState(nullptr)
    , m_keyStateLength(0)
    , m_mouseX(0)
    , m_mouseY(0)
    , m_previousMouseX(0)
    , m_previousMouseY(0)
    , m_mouseWheelDelta(0)
    , m_quitRequested(false)
{
    // Get keyboard state
    m_currentKeyState = SDL_GetKeyboardState(&m_keyStateLength);
    m_previousKeyState = new Uint8[m_keyStateLength];
    memset(m_previousKeyState, 0, m_keyStateLength);
    
    // Initialize mouse button states
    m_currentMouseState[MouseButton::LEFT] = false;
    m_currentMouseState[MouseButton::MIDDLE] = false;
    m_currentMouseState[MouseButton::RIGHT] = false;
    
    m_previousMouseState[MouseButton::LEFT] = false;
    m_previousMouseState[MouseButton::MIDDLE] = false;
    m_previousMouseState[MouseButton::RIGHT] = false;
}

InputManager::~InputManager() {
    delete[] m_previousKeyState;
}

void InputManager::Update() {
    // Update previous keyboard state BEFORE SDL processes new events
    memcpy(m_previousKeyState, m_currentKeyState, m_keyStateLength);

    // Update previous mouse state
    m_previousMouseState = m_currentMouseState;

    // Update previous mouse position
    m_previousMouseX = m_mouseX;
    m_previousMouseY = m_mouseY;

    // Pump SDL events to update keyboard state
    SDL_PumpEvents();

    // Get current mouse position
    Uint32 mouseState = SDL_GetMouseState(&m_mouseX, &m_mouseY);

    // Update mouse button states
    m_currentMouseState[MouseButton::LEFT] = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    m_currentMouseState[MouseButton::MIDDLE] = (mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
    m_currentMouseState[MouseButton::RIGHT] = (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;

    // Reset mouse wheel delta
    m_mouseWheelDelta = 0;
}

bool InputManager::HandleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_QUIT:
            m_quitRequested = true;
            return true;

        case SDL_MOUSEWHEEL:
            m_mouseWheelDelta = event.wheel.y;
            return true;

        case SDL_KEYDOWN:
            // Remove automatic quit on escape - let game states handle it
            return true;

        case SDL_KEYUP:
            return true;

        default:
            return false;
    }
}

bool InputManager::IsKeyPressed(SDL_Scancode key) const {
    if (key < 0 || key >= m_keyStateLength) return false;
    return m_currentKeyState[key] != 0;
}

bool InputManager::IsKeyJustPressed(SDL_Scancode key) const {
    if (key < 0 || key >= m_keyStateLength) return false;

    bool current = m_currentKeyState[key] != 0;
    bool previous = m_previousKeyState[key] != 0;
    bool justPressed = current && !previous;

    return justPressed;
}

bool InputManager::IsKeyJustReleased(SDL_Scancode key) const {
    if (key < 0 || key >= m_keyStateLength) return false;
    return m_currentKeyState[key] == 0 && m_previousKeyState[key] != 0;
}

bool InputManager::IsMouseButtonPressed(MouseButton button) const {
    auto it = m_currentMouseState.find(button);
    return it != m_currentMouseState.end() && it->second;
}

bool InputManager::IsMouseButtonJustPressed(MouseButton button) const {
    auto currentIt = m_currentMouseState.find(button);
    auto previousIt = m_previousMouseState.find(button);
    
    bool current = (currentIt != m_currentMouseState.end()) && currentIt->second;
    bool previous = (previousIt != m_previousMouseState.end()) && previousIt->second;
    
    return current && !previous;
}

bool InputManager::IsMouseButtonJustReleased(MouseButton button) const {
    auto currentIt = m_currentMouseState.find(button);
    auto previousIt = m_previousMouseState.find(button);
    
    bool current = (currentIt != m_currentMouseState.end()) && currentIt->second;
    bool previous = (previousIt != m_previousMouseState.end()) && previousIt->second;
    
    return !current && previous;
}

void InputManager::GetMousePosition(int& x, int& y) const {
    x = m_mouseX;
    y = m_mouseY;
}

void InputManager::GetMouseDelta(int& deltaX, int& deltaY) const {
    deltaX = m_mouseX - m_previousMouseX;
    deltaY = m_mouseY - m_previousMouseY;
}

void InputManager::SetMousePosition(int x, int y) {
    SDL_WarpMouseGlobal(x, y);
    m_mouseX = x;
    m_mouseY = y;
}

void InputManager::ShowCursor(bool show) {
    SDL_ShowCursor(show ? SDL_ENABLE : SDL_DISABLE);
}
