#include "Game/GameState.h"
#include "Game/GameStateManager.h"
#include "Engine/Engine.h"
#include "Engine/Renderer.h"
#include "Engine/InputManager.h"

Renderer* GameState::GetRenderer() const {
    return m_engine ? m_engine->GetRenderer() : nullptr;
}

InputManager* GameState::GetInputManager() const {
    return m_engine ? m_engine->GetInputManager() : nullptr;
}
