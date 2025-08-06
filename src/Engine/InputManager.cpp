/**
 * @file InputManager.cpp
 * @brief Implementation of comprehensive input handling system
 * @author Ryan Butler
 * @date 2025
 */

#include "Engine/InputManager.h"
#include <cstring>
#include <iostream>

/**
 * @brief Constructor - initializes input tracking systems
 *
 * Sets up the input manager with:
 * - Keyboard state tracking (current and previous frame)
 * - Mouse button state tracking (current and previous frame)
 * - Mouse position tracking with delta calculation
 * - Mouse wheel support
 * - Quit request detection
 *
 * The dual-state system (current/previous) enables detection of:
 * - "Just pressed" events (pressed this frame, not last frame)
 * - "Just released" events (released this frame, was pressed last frame)
 * - "Held" events (pressed both this frame and last frame)
 *
 * @note SDL must be initialized before creating InputManager
 * @note Keyboard state is managed by SDL (pointer to SDL's internal array)
 * @note Mouse state is managed internally (our own tracking)
 */
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
    // Get SDL's keyboard state array (updated automatically by SDL)
    m_currentKeyState = SDL_GetKeyboardState(&m_keyStateLength);

    // Allocate our own array to track previous frame's keyboard state
    m_previousKeyState = new Uint8[m_keyStateLength];
    memset(m_previousKeyState, 0, m_keyStateLength); // Initialize to "not pressed"

    // Initialize mouse button states to "not pressed"
    // We track both current and previous frame states for edge detection
    m_currentMouseState[MouseButton::LEFT] = false;
    m_currentMouseState[MouseButton::MIDDLE] = false;
    m_currentMouseState[MouseButton::RIGHT] = false;

    m_previousMouseState[MouseButton::LEFT] = false;
    m_previousMouseState[MouseButton::MIDDLE] = false;
    m_previousMouseState[MouseButton::RIGHT] = false;

    std::cout << "✅ Input manager initialized - tracking " << m_keyStateLength << " keys" << std::endl;
}

/**
 * @brief Destructor - cleans up allocated keyboard state array
 *
 * Releases the dynamically allocated previous keyboard state array.
 * The current keyboard state is managed by SDL and doesn't need cleanup.
 *
 * @note Mouse state maps are automatically cleaned up (no dynamic allocation)
 */
InputManager::~InputManager() {
    delete[] m_previousKeyState; // Free our keyboard state tracking array
    std::cout << "✅ Input manager shut down" << std::endl;
}

/**
 * @brief Update input state tracking for edge detection
 *
 * This method MUST be called once per frame BEFORE processing SDL events.
 * It captures the current input state as "previous frame" state, enabling
 * detection of input changes (just pressed, just released).
 *
 * Process:
 * 1. Save current keyboard state as previous frame state
 * 2. Save current mouse button states as previous frame state
 * 3. Save current mouse position as previous frame position
 * 4. Update SDL's internal keyboard state
 * 5. Query current mouse state and position
 * 6. Reset frame-specific values (mouse wheel)
 *
 * @note CRITICAL: Must be called before HandleEvent() each frame
 * @note Enables IsKeyJustPressed(), IsMouseButtonJustPressed(), etc.
 * @note Called automatically by Engine::Run()
 *
 * @example
 * ```cpp
 * // In main game loop:
 * inputManager.Update();        // Capture previous state
 *
 * // Process events
 * SDL_Event event;
 * while (SDL_PollEvent(&event)) {
 *     inputManager.HandleEvent(event);
 * }
 *
 * // Now edge detection works correctly
 * if (inputManager.IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
 *     // Space was just pressed this frame
 * }
 * ```
 */
void InputManager::Update() {
    // STEP 1: Capture current keyboard state as "previous frame"
    // This must happen BEFORE SDL updates the keyboard state
    memcpy(m_previousKeyState, m_currentKeyState, m_keyStateLength);

    // STEP 2: Capture current mouse button states as "previous frame"
    m_previousMouseState = m_currentMouseState;

    // STEP 3: Capture current mouse position as "previous frame"
    m_previousMouseX = m_mouseX;
    m_previousMouseY = m_mouseY;

    // STEP 4: Tell SDL to process pending events and update keyboard state
    // This updates m_currentKeyState (SDL's internal array)
    SDL_PumpEvents();

    // STEP 5: Query current mouse state and position
    Uint32 mouseState = SDL_GetMouseState(&m_mouseX, &m_mouseY);

    // STEP 6: Update our mouse button state tracking
    m_currentMouseState[MouseButton::LEFT] = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    m_currentMouseState[MouseButton::MIDDLE] = (mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
    m_currentMouseState[MouseButton::RIGHT] = (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;

    // STEP 7: Reset frame-specific values
    // Mouse wheel delta is only valid for one frame
    m_mouseWheelDelta = 0;
}

/**
 * @brief Process SDL events and update input state
 *
 * Handles specific SDL events that require immediate processing:
 * - Quit requests (window close, Alt+F4, etc.)
 * - Mouse wheel scrolling (frame-specific events)
 * - Keyboard events (for future expansion)
 *
 * @param event SDL event to process
 *
 * @return true if event was handled, false if ignored
 *
 * @note Called for each SDL event by Engine::HandleEvents()
 * @note Keyboard state is handled automatically by SDL (no manual processing needed)
 * @note Mouse wheel events are captured here because they're frame-specific
 * @note Quit detection allows graceful application shutdown
 */
bool InputManager::HandleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_QUIT:
            // User closed window or pressed Alt+F4
            m_quitRequested = true;
            std::cout << "🛑 Quit requested via window close" << std::endl;
            return true;

        case SDL_MOUSEWHEEL:
            // Mouse wheel scrolling (positive = up, negative = down)
            m_mouseWheelDelta = event.wheel.y;
            return true;

        case SDL_KEYDOWN:
            // Keyboard key pressed (handled automatically by SDL's keyboard state)
            // We don't need to manually track this - SDL does it for us
            return true;

        case SDL_KEYUP:
            // Keyboard key released (handled automatically by SDL's keyboard state)
            // We don't need to manually track this - SDL does it for us
            return true;

        default:
            // Event not handled by input manager
            return false;
    }
}

// ========== KEYBOARD INPUT QUERY METHODS ==========

/**
 * @brief Check if a keyboard key is currently being held down
 *
 * Returns true if the specified key is pressed in the current frame.
 * This is a "continuous" check - returns true for every frame the key is held.
 *
 * @param key SDL scancode for the key to check (e.g., SDL_SCANCODE_SPACE)
 *
 * @return true if key is currently pressed, false otherwise
 *
 * @note Returns false for invalid key codes (safety check)
 * @note Use for continuous actions like movement, shooting
 * @note For single-action events, use IsKeyJustPressed() instead
 *
 * @example
 * ```cpp
 * // Continuous movement while key is held
 * if (inputManager.IsKeyPressed(SDL_SCANCODE_RIGHT)) {
 *     playerX += playerSpeed * deltaTime;
 * }
 * ```
 */
bool InputManager::IsKeyPressed(SDL_Scancode key) const {
    // Validate key code to prevent array bounds errors
    if (key < 0 || key >= m_keyStateLength) return false;

    // Check SDL's keyboard state array (non-zero = pressed)
    return m_currentKeyState[key] != 0;
}

/**
 * @brief Check if a keyboard key was just pressed this frame
 *
 * Returns true only on the first frame when a key is pressed.
 * Perfect for single-action events like jumping, menu selection, etc.
 *
 * Logic: Key is pressed now AND was not pressed last frame
 *
 * @param key SDL scancode for the key to check
 *
 * @return true if key was just pressed this frame, false otherwise
 *
 * @note Returns false for invalid key codes (safety check)
 * @note Only returns true for ONE frame per key press
 * @note Use for single actions like jumping, menu navigation
 *
 * @example
 * ```cpp
 * // Single jump action
 * if (inputManager.IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
 *     player.Jump(); // Only jumps once per key press
 * }
 *
 * // Menu navigation
 * if (inputManager.IsKeyJustPressed(SDL_SCANCODE_RETURN)) {
 *     menu.SelectCurrentItem();
 * }
 * ```
 */
bool InputManager::IsKeyJustPressed(SDL_Scancode key) const {
    // Validate key code to prevent array bounds errors
    if (key < 0 || key >= m_keyStateLength) return false;

    // Check current and previous frame states
    bool current = m_currentKeyState[key] != 0;   // Pressed this frame?
    bool previous = m_previousKeyState[key] != 0; // Was pressed last frame?

    // "Just pressed" = pressed now AND not pressed before
    bool justPressed = current && !previous;

    return justPressed;
}

/**
 * @brief Check if a keyboard key was just released this frame
 *
 * Returns true only on the first frame when a key is released.
 * Useful for detecting when actions should stop or for release-triggered events.
 *
 * Logic: Key is not pressed now AND was pressed last frame
 *
 * @param key SDL scancode for the key to check
 *
 * @return true if key was just released this frame, false otherwise
 *
 * @note Returns false for invalid key codes (safety check)
 * @note Only returns true for ONE frame per key release
 * @note Use for actions that trigger on key release
 *
 * @example
 * ```cpp
 * // Charge-up mechanic (release to fire)
 * if (inputManager.IsKeyPressed(SDL_SCANCODE_SPACE)) {
 *     chargeLevel += deltaTime; // Charge while held
 * }
 * if (inputManager.IsKeyJustReleased(SDL_SCANCODE_SPACE)) {
 *     FireWeapon(chargeLevel); // Fire when released
 *     chargeLevel = 0;
 * }
 * ```
 */
bool InputManager::IsKeyJustReleased(SDL_Scancode key) const {
    // Validate key code to prevent array bounds errors
    if (key < 0 || key >= m_keyStateLength) return false;

    // "Just released" = not pressed now AND was pressed before
    return m_currentKeyState[key] == 0 && m_previousKeyState[key] != 0;
}

// ========== MOUSE INPUT QUERY METHODS ==========

/**
 * @brief Check if a mouse button is currently being held down
 *
 * Returns true if the specified mouse button is pressed in the current frame.
 * This is a "continuous" check - returns true for every frame the button is held.
 *
 * @param button Mouse button to check (LEFT, MIDDLE, RIGHT)
 *
 * @return true if button is currently pressed, false otherwise
 *
 * @note Safe to call with any MouseButton value
 * @note Use for continuous actions like dragging, aiming
 * @note For single-click events, use IsMouseButtonJustPressed() instead
 *
 * @example
 * ```cpp
 * // Continuous firing while mouse held
 * if (inputManager.IsMouseButtonPressed(MouseButton::LEFT)) {
 *     weapon.Fire(deltaTime);
 * }
 * ```
 */
bool InputManager::IsMouseButtonPressed(MouseButton button) const {
    auto it = m_currentMouseState.find(button);
    return it != m_currentMouseState.end() && it->second;
}

/**
 * @brief Check if a mouse button was just pressed this frame
 *
 * Returns true only on the first frame when a mouse button is pressed.
 * Perfect for single-click actions like UI buttons, weapon firing, etc.
 *
 * Logic: Button is pressed now AND was not pressed last frame
 *
 * @param button Mouse button to check (LEFT, MIDDLE, RIGHT)
 *
 * @return true if button was just pressed this frame, false otherwise
 *
 * @note Only returns true for ONE frame per button press
 * @note Use for single actions like clicking buttons, single shots
 *
 * @example
 * ```cpp
 * // Single shot weapon
 * if (inputManager.IsMouseButtonJustPressed(MouseButton::LEFT)) {
 *     weapon.FireSingleShot();
 * }
 *
 * // UI button clicking
 * if (inputManager.IsMouseButtonJustPressed(MouseButton::LEFT)) {
 *     if (button.Contains(mouseX, mouseY)) {
 *         button.OnClick();
 *     }
 * }
 * ```
 */
bool InputManager::IsMouseButtonJustPressed(MouseButton button) const {
    auto currentIt = m_currentMouseState.find(button);
    auto previousIt = m_previousMouseState.find(button);

    // Get current and previous states (default to false if not found)
    bool current = (currentIt != m_currentMouseState.end()) && currentIt->second;
    bool previous = (previousIt != m_previousMouseState.end()) && previousIt->second;

    // "Just pressed" = pressed now AND not pressed before
    return current && !previous;
}

/**
 * @brief Check if a mouse button was just released this frame
 *
 * Returns true only on the first frame when a mouse button is released.
 * Useful for drag-and-drop operations, charge-up mechanics, etc.
 *
 * Logic: Button is not pressed now AND was pressed last frame
 *
 * @param button Mouse button to check (LEFT, MIDDLE, RIGHT)
 *
 * @return true if button was just released this frame, false otherwise
 *
 * @note Only returns true for ONE frame per button release
 * @note Use for actions that trigger on button release
 *
 * @example
 * ```cpp
 * // Drag and drop
 * if (inputManager.IsMouseButtonPressed(MouseButton::LEFT)) {
 *     DragObject(mouseX, mouseY);
 * }
 * if (inputManager.IsMouseButtonJustReleased(MouseButton::LEFT)) {
 *     DropObject(mouseX, mouseY);
 * }
 * ```
 */
bool InputManager::IsMouseButtonJustReleased(MouseButton button) const {
    auto currentIt = m_currentMouseState.find(button);
    auto previousIt = m_previousMouseState.find(button);

    // Get current and previous states (default to false if not found)
    bool current = (currentIt != m_currentMouseState.end()) && currentIt->second;
    bool previous = (previousIt != m_previousMouseState.end()) && previousIt->second;

    // "Just released" = not pressed now AND was pressed before
    return !current && previous;
}

// ========== MOUSE POSITION AND UTILITY METHODS ==========

/**
 * @brief Get current mouse position in window coordinates
 *
 * Returns the current mouse cursor position relative to the game window.
 * Coordinates are in pixels with (0,0) at the top-left corner.
 *
 * @param x Reference to store X coordinate
 * @param y Reference to store Y coordinate
 *
 * @note Coordinates are relative to window, not screen
 * @note Updated automatically each frame in Update()
 * @note Use GetMouseX()/GetMouseY() for individual coordinate access
 *
 * @example
 * ```cpp
 * int mouseX, mouseY;
 * inputManager.GetMousePosition(mouseX, mouseY);
 *
 * // Check if mouse is over a button
 * if (button.Contains(mouseX, mouseY)) {
 *     button.Highlight();
 * }
 * ```
 */
void InputManager::GetMousePosition(int& x, int& y) const {
    x = m_mouseX;
    y = m_mouseY;
}

/**
 * @brief Get mouse movement since last frame
 *
 * Returns the change in mouse position since the previous frame.
 * Useful for camera controls, mouse look, drag operations, etc.
 *
 * @param deltaX Reference to store X movement (positive = moved right)
 * @param deltaY Reference to store Y movement (positive = moved down)
 *
 * @note Values are in pixels moved since last frame
 * @note Positive deltaX = moved right, negative = moved left
 * @note Positive deltaY = moved down, negative = moved up
 * @note Reset to (0,0) if mouse didn't move
 *
 * @example
 * ```cpp
 * int deltaX, deltaY;
 * inputManager.GetMouseDelta(deltaX, deltaY);
 *
 * // Camera mouse look
 * cameraYaw += deltaX * mouseSensitivity;
 * cameraPitch += deltaY * mouseSensitivity;
 *
 * // Drag object
 * if (inputManager.IsMouseButtonPressed(MouseButton::LEFT)) {
 *     objectX += deltaX;
 *     objectY += deltaY;
 * }
 * ```
 */
void InputManager::GetMouseDelta(int& deltaX, int& deltaY) const {
    deltaX = m_mouseX - m_previousMouseX;
    deltaY = m_mouseY - m_previousMouseY;
}

/**
 * @brief Set mouse cursor position programmatically
 *
 * Moves the mouse cursor to the specified screen coordinates.
 * Useful for centering cursor, mouse lock, or teleporting cursor.
 *
 * @param x New X coordinate in screen space (global coordinates)
 * @param y New Y coordinate in screen space (global coordinates)
 *
 * @note Coordinates are in global screen space, not window-relative
 * @note Updates internal position tracking immediately
 * @note Use sparingly - can be disorienting to users
 * @note Useful for mouse lock in FPS games or cursor centering
 *
 * @example
 * ```cpp
 * // Center cursor on screen for mouse lock
 * int screenCenterX = screenWidth / 2;
 * int screenCenterY = screenHeight / 2;
 * inputManager.SetMousePosition(screenCenterX, screenCenterY);
 * ```
 */
void InputManager::SetMousePosition(int x, int y) {
    // Move cursor to specified global screen coordinates
    SDL_WarpMouseGlobal(x, y);

    // Update our internal tracking to match
    m_mouseX = x;
    m_mouseY = y;
}

/**
 * @brief Show or hide the mouse cursor
 *
 * Controls the visibility of the mouse cursor within the game window.
 * Useful for different game modes or immersive experiences.
 *
 * @param show true to show cursor, false to hide it
 *
 * @note Cursor visibility is global (affects entire application)
 * @note Hidden cursor still generates position and click events
 * @note Useful for fullscreen games or when using custom cursors
 *
 * @example
 * ```cpp
 * // Hide cursor during gameplay
 * inputManager.ShowCursor(false);
 *
 * // Show cursor in menus
 * inputManager.ShowCursor(true);
 * ```
 */
void InputManager::ShowCursor(bool show) {
    SDL_ShowCursor(show ? SDL_ENABLE : SDL_DISABLE);
}
