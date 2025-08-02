#pragma once

#include "GameState.h"
#include <memory>
#include <unordered_map>
#include <stack>

class Engine;

class GameStateManager {
public:
    GameStateManager(Engine* engine);
    ~GameStateManager();

    // State management
    void AddState(GameStateType type, std::unique_ptr<GameState> state);
    void PushState(GameStateType type);
    void PopState();
    void ChangeState(GameStateType type);
    void ClearStates();

    // Current state access
    GameState* GetCurrentState() const;
    bool HasStates() const { return !m_stateStack.empty(); }

    // Update current state
    void Update(float deltaTime);
    void Render();
    void HandleInput();

private:
    Engine* m_engine;
    std::unordered_map<GameStateType, std::unique_ptr<GameState>> m_states;
    std::stack<GameStateType> m_stateStack;
    
    bool m_pendingChange;
    GameStateType m_pendingState;
    bool m_pendingPop;
    
    void ProcessPendingChanges();
};
