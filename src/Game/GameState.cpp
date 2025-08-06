/**
 * @file GameState.cpp
 * @brief Implementation of GameState base class helper methods
 * @author Ryan Butler
 * @date 2025
 */

#include "Game/GameState.h"
#include "Game/GameStateManager.h"
#include "Engine/Engine.h"
#include "Engine/Renderer.h"
#include "Engine/InputManager.h"

/**
 * @brief Get renderer instance for drawing operations
 *
 * Provides convenient access to the rendering system through the engine.
 * This is a helper method that derived states can use in their Render() methods.
 *
 * @return Pointer to Renderer instance, or nullptr if engine not set
 *
 * @note Returns nullptr if SetEngine() hasn't been called yet
 * @note Safe to call - includes null pointer check
 * @note Prefer this over directly accessing the engine for renderer access
 *
 * @example
 * ```cpp
 * void MyState::Render() override {
 *     Renderer* renderer = GetRenderer();
 *     if (renderer) {
 *         renderer->DrawTexture(backgroundTexture, 0, 0);
 *         renderer->DrawText("Hello World", 100, 100);
 *     }
 * }
 * ```
 */
Renderer* GameState::GetRenderer() const {
    return m_engine ? m_engine->GetRenderer() : nullptr;
}

/**
 * @brief Get input manager instance for handling user input
 *
 * Provides convenient access to the input system through the engine.
 * This is a helper method that derived states can use in their HandleInput() methods.
 *
 * @return Pointer to InputManager instance, or nullptr if engine not set
 *
 * @note Returns nullptr if SetEngine() hasn't been called yet
 * @note Safe to call - includes null pointer check
 * @note Prefer this over directly accessing the engine for input access
 *
 * @example
 * ```cpp
 * void MyState::HandleInput() override {
 *     InputManager* input = GetInputManager();
 *     if (input) {
 *         if (input->IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
 *             // Handle space key press
 *         }
 *         if (input->IsMouseButtonPressed(MouseButton::LEFT)) {
 *             // Handle mouse click
 *         }
 *     }
 * }
 * ```
 */
InputManager* GameState::GetInputManager() const {
    return m_engine ? m_engine->GetInputManager() : nullptr;
}
