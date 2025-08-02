#include "Engine/Engine.h"
#include "Engine/Window.h"
#include "Engine/Renderer.h"
#include "Engine/InputManager.h"
#include "Game/GameStateManager.h"
#include "Game/MenuState.h"
#include "Game/PlayingState.h"
#include "Game/GameOverState.h"
#include <iostream>
#include <random>
#include <chrono>

class ArcadeFighterGame : public Engine {
public:
    ArcadeFighterGame() : m_stateManager(nullptr) {}

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
            std::string title = "Arcade Fighter - FPS: " + std::to_string((int)GetFPS());
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

class SimpleTestGame : public Engine {
public:
    SimpleTestGame() : m_rectX(100), m_rectY(100) {}

protected:
    void Update(float deltaTime) override {
        auto* input = GetInputManager();

        // Move rectangle with arrow keys
        const float speed = 200.0f;
        if (input->IsKeyPressed(SDL_SCANCODE_LEFT) || input->IsKeyPressed(SDL_SCANCODE_A)) {
            m_rectX -= speed * deltaTime;
        }
        if (input->IsKeyPressed(SDL_SCANCODE_RIGHT) || input->IsKeyPressed(SDL_SCANCODE_D)) {
            m_rectX += speed * deltaTime;
        }
        if (input->IsKeyPressed(SDL_SCANCODE_UP) || input->IsKeyPressed(SDL_SCANCODE_W)) {
            m_rectY -= speed * deltaTime;
        }
        if (input->IsKeyPressed(SDL_SCANCODE_DOWN) || input->IsKeyPressed(SDL_SCANCODE_S)) {
            m_rectY += speed * deltaTime;
        }

        // Keep within bounds
        if (m_rectX < 0) m_rectX = 0;
        if (m_rectY < 0) m_rectY = 0;
        if (m_rectX > 750) m_rectX = 750;
        if (m_rectY > 550) m_rectY = 550;

        // Update window title with FPS
        static float titleUpdateTimer = 0.0f;
        titleUpdateTimer += deltaTime;
        if (titleUpdateTimer >= 1.0f) {
            std::string title = "Simple Test - FPS: " + std::to_string((int)GetFPS()) +
                               " Pos: (" + std::to_string((int)m_rectX) + "," + std::to_string((int)m_rectY) + ")";
            GetWindow()->SetTitle(title.c_str());
            titleUpdateTimer = 0.0f;
        }
    }

    void Render() override {
        auto* renderer = GetRenderer();

        // Draw a green player rectangle
        Rectangle playerRect(static_cast<int>(m_rectX), static_cast<int>(m_rectY), 50, 50);
        renderer->DrawRectangle(playerRect, Color(0, 255, 0, 255), true);

        // Draw some static test shapes
        renderer->DrawRectangle(Rectangle(10, 10, 100, 30), Color(255, 0, 0, 255), true);
        renderer->DrawRectangle(Rectangle(200, 200, 80, 80), Color(0, 0, 255, 255), false);
        renderer->DrawLine(0, 0, 800, 600, Color(255, 255, 0, 255));
        renderer->DrawLine(800, 0, 0, 600, Color(255, 0, 255, 255));

        // Draw walls
        renderer->DrawRectangle(Rectangle(0, 0, 800, 10), Color(128, 128, 128, 255), true);
        renderer->DrawRectangle(Rectangle(0, 590, 800, 10), Color(128, 128, 128, 255), true);
        renderer->DrawRectangle(Rectangle(0, 0, 10, 600), Color(128, 128, 128, 255), true);
        renderer->DrawRectangle(Rectangle(790, 0, 10, 600), Color(128, 128, 128, 255), true);
    }

private:
    float m_rectX, m_rectY;
};

int main() {
    std::cout << "🎮 ARCADE FIGHTER GAME 🎮" << std::endl;
    std::cout << "Choose demo:" << std::endl;
    std::cout << "1. Simple Test (basic rendering)" << std::endl;
    std::cout << "2. Arcade Fighter (full game with menu)" << std::endl;
    std::cout << "Enter choice (1 or 2): ";

    int choice;
    std::cin >> choice;

    if (choice == 1) {
        SimpleTestGame game;

        if (!game.Initialize("Simple Rendering Test", 800, 600)) {
            std::cerr << "Failed to initialize game!" << std::endl;
            return -1;
        }

        std::cout << "\nSimple Rendering Test" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  Arrow keys or WASD: Move green rectangle" << std::endl;
        std::cout << "  Escape: Quit" << std::endl;

        game.Run();
    } else {
        ArcadeFighterGame game;

        if (!game.Initialize("🥊 ARCADE FIGHTER 🥊", 800, 600)) {
            std::cerr << "Failed to initialize game!" << std::endl;
            return -1;
        }

        std::cout << "\n🥊 ARCADE FIGHTER GAME 🥊" << std::endl;
        std::cout << "Features:" << std::endl;
        std::cout << "  ✅ Start Menu with navigation" << std::endl;
        std::cout << "  ✅ Game state management" << std::endl;
        std::cout << "  ✅ Side-scrolling gameplay" << std::endl;
        std::cout << "  ✅ Player movement and camera" << std::endl;
        std::cout << "  ✅ Game Over screen" << std::endl;
        std::cout << "  🔄 More features coming..." << std::endl;

        game.Run();
    }

    return 0;
}
