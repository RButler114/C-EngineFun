#pragma once

#include <string>
#include <functional>

class Engine;
class Renderer;
class InputManager;
class GameStateManager;

enum class GameStateType {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER,
    OPTIONS
};

class GameState {
public:
    GameState(GameStateType type, const std::string& name)
        : m_type(type), m_name(name), m_engine(nullptr), m_stateManager(nullptr) {}
    virtual ~GameState() = default;

    // State lifecycle
    virtual void OnEnter() {}
    virtual void OnExit() {}
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
    virtual void HandleInput() {}

    // Getters
    GameStateType GetType() const { return m_type; }
    const std::string& GetName() const { return m_name; }
    Engine* GetEngine() const { return m_engine; }

    // Called by GameStateManager
    void SetEngine(Engine* engine) { m_engine = engine; }
    void SetStateManager(GameStateManager* manager) { m_stateManager = manager; }

protected:
    Renderer* GetRenderer() const;
    InputManager* GetInputManager() const;
    GameStateManager* GetStateManager() const { return m_stateManager; }

private:
    GameStateType m_type;
    std::string m_name;
    Engine* m_engine;
    GameStateManager* m_stateManager;
};
