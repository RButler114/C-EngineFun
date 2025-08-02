#include "Engine/Engine.h"
#include "Engine/Window.h"
#include "Engine/Renderer.h"
#include "Engine/InputManager.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <thread>

Engine::Engine()
    : m_isRunning(false)
    , m_targetFPS(60)
    , m_deltaTime(0.0f)
    , m_fps(0.0f)
{
}

Engine::~Engine() {
    Shutdown();
}

bool Engine::Initialize(const char* title, int width, int height) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create window
    m_window = std::make_unique<Window>();
    if (!m_window->Initialize(title, width, height)) {
        std::cerr << "Failed to create window!" << std::endl;
        return false;
    }

    // Create renderer
    m_renderer = std::make_unique<Renderer>();
    if (!m_renderer->Initialize(m_window->GetSDLWindow())) {
        std::cerr << "Failed to create renderer!" << std::endl;
        return false;
    }

    // Create input manager
    m_inputManager = std::make_unique<InputManager>();

    m_isRunning = true;
    m_lastFrameTime = std::chrono::high_resolution_clock::now();
    
    std::cout << "Engine initialized successfully!" << std::endl;
    return true;
}

void Engine::Run() {
    while (m_isRunning) {
        m_frameStartTime = std::chrono::high_resolution_clock::now();

        // Update input manager (capture previous state before processing events)
        m_inputManager->Update();

        // Handle events
        HandleEvents();

        // Check for quit
        if (m_inputManager->IsQuitRequested()) {
            m_isRunning = false;
        }

        // Calculate delta time
        CalculateDeltaTime();

        // Update game logic
        Update(m_deltaTime);

        // Render
        m_renderer->Clear();
        Render();
        m_renderer->Present();

        // Cap frame rate
        CapFrameRate();
    }
}

void Engine::Shutdown() {
    if (m_renderer) {
        m_renderer->Shutdown();
        m_renderer.reset();
    }
    
    if (m_window) {
        m_window->Shutdown();
        m_window.reset();
    }
    
    m_inputManager.reset();
    
    SDL_Quit();
    std::cout << "Engine shut down successfully!" << std::endl;
}

void Engine::HandleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        m_inputManager->HandleEvent(event);
    }
}

void Engine::CalculateDeltaTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - m_lastFrameTime);
    m_deltaTime = duration.count() / 1000000.0f; // Convert to seconds
    
    // Calculate FPS
    if (m_deltaTime > 0.0f) {
        m_fps = 1.0f / m_deltaTime;
    }
    
    m_lastFrameTime = currentTime;
}

void Engine::CapFrameRate() {
    auto frameEndTime = std::chrono::high_resolution_clock::now();
    auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(frameEndTime - m_frameStartTime);
    
    auto targetFrameTime = std::chrono::microseconds(1000000 / m_targetFPS);
    
    if (frameDuration < targetFrameTime) {
        auto sleepTime = targetFrameTime - frameDuration;
        std::this_thread::sleep_for(sleepTime);
    }
}
