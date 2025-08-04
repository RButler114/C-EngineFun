#include "Game/GameStateManager.h"
#include "Engine/Engine.h"
#include <iostream>

GameStateManager::GameStateManager(Engine* engine) 
    : m_engine(engine), m_pendingChange(false), m_pendingPop(false) {
}

GameStateManager::~GameStateManager() {
    ClearStates();
}

void GameStateManager::AddState(GameStateType type, std::unique_ptr<GameState> state) {
    if (state) {
        state->SetEngine(m_engine);
        state->SetStateManager(this);
        m_states[type] = std::move(state);
        std::cout << "Added game state: " << static_cast<int>(type) << std::endl;
    }
}

void GameStateManager::PushState(GameStateType type) {
    auto it = m_states.find(type);
    if (it != m_states.end()) {
        // Don't exit current state when pushing - just pause it
        // This allows the underlying state (like PlayingState) to keep its entities alive

        m_stateStack.push(type);
        it->second->OnEnter();
        std::cout << "Pushed state: " << it->second->GetName() << std::endl;
    }
}

void GameStateManager::PopState() {
    if (!m_stateStack.empty()) {
        auto currentIt = m_states.find(m_stateStack.top());
        if (currentIt != m_states.end()) {
            std::cout << "Popping state: " << currentIt->second->GetName() << std::endl;
            currentIt->second->OnExit();
        }

        m_stateStack.pop();

        // Don't call OnEnter() on the resumed state - it should just continue
        // The underlying state was paused, not exited, so it should resume naturally
        if (!m_stateStack.empty()) {
            auto newCurrentIt = m_states.find(m_stateStack.top());
            if (newCurrentIt != m_states.end()) {
                std::cout << "Resumed state: " << newCurrentIt->second->GetName() << std::endl;
            }
        }
    }
}

void GameStateManager::ChangeState(GameStateType type) {
    m_pendingChange = true;
    m_pendingState = type;
}

void GameStateManager::ClearStates() {
    while (!m_stateStack.empty()) {
        PopState();
    }
    m_states.clear();
}

GameState* GameStateManager::GetCurrentState() const {
    if (m_stateStack.empty()) {
        return nullptr;
    }
    
    auto it = m_states.find(m_stateStack.top());
    return (it != m_states.end()) ? it->second.get() : nullptr;
}

void GameStateManager::Update(float deltaTime) {
    ProcessPendingChanges();
    
    GameState* currentState = GetCurrentState();
    if (currentState) {
        currentState->Update(deltaTime);
    }
}

void GameStateManager::Render() {
    GameState* currentState = GetCurrentState();
    if (currentState) {
        currentState->Render();
    }
}

void GameStateManager::HandleInput() {
    GameState* currentState = GetCurrentState();
    if (currentState) {
        currentState->HandleInput();
    }
}

void GameStateManager::ProcessPendingChanges() {
    if (m_pendingChange) {
        // Exit current state
        if (!m_stateStack.empty()) {
            auto currentIt = m_states.find(m_stateStack.top());
            if (currentIt != m_states.end()) {
                currentIt->second->OnExit();
            }
            m_stateStack.pop();
        }
        
        // Enter new state
        auto newIt = m_states.find(m_pendingState);
        if (newIt != m_states.end()) {
            m_stateStack.push(m_pendingState);
            newIt->second->OnEnter();
            std::cout << "Changed to state: " << newIt->second->GetName() << std::endl;
        }
        
        m_pendingChange = false;
    }
    
    if (m_pendingPop) {
        PopState();
        m_pendingPop = false;
    }
}
