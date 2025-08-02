#pragma once

#include <memory>
#include <chrono>

class Window;
class Renderer;
class InputManager;

class Engine {
public:
    Engine();
    ~Engine();

    bool Initialize(const char* title, int width, int height);
    void Run();
    void Shutdown();

    // Getters
    Window* GetWindow() const { return m_window.get(); }
    Renderer* GetRenderer() const { return m_renderer.get(); }
    InputManager* GetInputManager() const { return m_inputManager.get(); }
    
    float GetDeltaTime() const { return m_deltaTime; }
    float GetFPS() const { return m_fps; }
    
    void SetTargetFPS(int fps) { m_targetFPS = fps; }
    bool IsRunning() const { return m_isRunning; }
    void Quit() { m_isRunning = false; }

protected:
    virtual void Update(float deltaTime) { (void)deltaTime; }
    virtual void Render() {}

private:
    void HandleEvents();
    void CalculateDeltaTime();
    void CapFrameRate();

    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<InputManager> m_inputManager;

    bool m_isRunning;
    int m_targetFPS;
    float m_deltaTime;
    float m_fps;
    
    std::chrono::high_resolution_clock::time_point m_lastFrameTime;
    std::chrono::high_resolution_clock::time_point m_frameStartTime;
};
