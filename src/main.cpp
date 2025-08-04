/**
 * @file main.cpp
 * @brief Main entry point for the C++ Arcade Game Engine
 * @author Ryan Butler
 * @date 2025
 *
 * Streamlined arcade game engine focused on classic side-scrolling gameplay.
 * Features:
 * - Game state management (Menu, Playing, Game Over, Options)
 * - Entity-Component-System architecture
 * - Sprite rendering and animation
 * - Audio system integration
 * - Input handling and player controls
 * - Collision detection
 * - Scrolling backgrounds
 */

#include "Engine/Engine.h"
#include "Engine/Window.h"
#include "Engine/Renderer.h"
#include "Engine/InputManager.h"
#include "Game/GameStateManager.h"
#include "Game/MenuState.h"
#include "Game/PlayingState.h"
#include "Game/GameOverState.h"
#include "Game/OptionsState.h"
#include <iostream>
#include <random>
#include <chrono>

/**
 * @class ArcadeGame
 * @brief Streamlined arcade game implementation
 *
 * Classic arcade-style side-scrolling game with:
 * - Menu system
 * - Side-scrolling gameplay
 * - Player movement and sprite animation
 * - Enemy spawning and collision
 * - Scoring system
 * - Game over handling
 */
class ArcadeGame : public Engine {
public:
    /**
     * @brief Constructor - initializes game state manager
     */
    ArcadeGame() : m_stateManager(nullptr) {}

    bool Initialize(const char* title, int width, int height) {
        if (!Engine::Initialize(title, width, height)) {
            return false;
        }

        // Create state manager
        m_stateManager = std::make_unique<GameStateManager>(this);

        // Add all game states
        m_stateManager->AddState(GameStateType::MENU, std::make_unique<MenuState>());
        m_stateManager->AddState(GameStateType::PLAYING, std::make_unique<PlayingState>());
        m_stateManager->AddState(GameStateType::GAME_OVER, std::make_unique<GameOverState>());
        m_stateManager->AddState(GameStateType::OPTIONS, std::make_unique<OptionsState>());

        // Start with menu
        m_stateManager->PushState(GameStateType::MENU);

        return true;
    }

protected:
    void Update(float deltaTime) override {
        if (m_stateManager) {
            m_stateManager->HandleInput();
            m_stateManager->Update(deltaTime);
        }

        // Update window title with FPS
        static float titleUpdateTimer = 0.0f;
        titleUpdateTimer += deltaTime;
        if (titleUpdateTimer >= 1.0f) {
            std::string title = "Arcade Game - FPS: " + std::to_string((int)GetFPS());
            GetWindow()->SetTitle(title.c_str());
            titleUpdateTimer = 0.0f;
        }
    }

    void Render() override {
        if (m_stateManager) {
            m_stateManager->Render();
        }
    }

private:
    std::unique_ptr<GameStateManager> m_stateManager;
};



int main() {
    std::cout << "🎮 ARCADE GAME ENGINE 🎮" << std::endl;

    ArcadeGame game;

    if (!game.Initialize("🎮 Arcade Game 🎮", 800, 600)) {
        std::cerr << "Failed to initialize game!" << std::endl;
        return -1;
    }

    std::cout << "\n🎮 ARCADE GAME 🎮" << std::endl;
    std::cout << "Features:" << std::endl;
    std::cout << "  ✅ Menu system with navigation" << std::endl;
    std::cout << "  ✅ Game state management" << std::endl;
    std::cout << "  ✅ Side-scrolling gameplay" << std::endl;
    std::cout << "  ✅ Sprite animation" << std::endl;
    std::cout << "  ✅ Audio system" << std::endl;
    std::cout << "  ✅ Collision detection" << std::endl;

    game.Run();

    return 0;
}
