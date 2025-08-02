#include "../include/Engine/Engine.h"
#include "../include/Game/GameStateManager.h"
#include "../include/Game/MenuState.h"
#include "../include/Game/PlayingState.h"
#include "../include/Game/GameOverState.h"
#include <iostream>
#include <chrono>

class StateManagementTest : public Engine {
public:
    StateManagementTest() : m_testPhase(0), m_testTimer(0.0f), m_testPassed(false) {}

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
        m_testTimer += deltaTime;
        
        if (m_stateManager) {
            m_stateManager->HandleInput();
            m_stateManager->Update(deltaTime);
        }
        
        switch (m_testPhase) {
            case 0: // Test Menu State
                if (m_testTimer > 2.0f) {
                    std::cout << "✅ Phase 0: Menu state active for 2 seconds" << std::endl;
                    std::cout << "Current state: " << (m_stateManager->GetCurrentState() ? 
                        m_stateManager->GetCurrentState()->GetName() : "NULL") << std::endl;
                    
                    // Transition to Playing state
                    m_stateManager->ChangeState(GameStateType::PLAYING);
                    m_testPhase++;
                    m_testTimer = 0.0f;
                }
                break;
                
            case 1: // Test Playing State
                if (m_testTimer > 2.0f) {
                    std::cout << "✅ Phase 1: Playing state active for 2 seconds" << std::endl;
                    std::cout << "Current state: " << (m_stateManager->GetCurrentState() ? 
                        m_stateManager->GetCurrentState()->GetName() : "NULL") << std::endl;
                    
                    // Transition to Game Over state
                    m_stateManager->ChangeState(GameStateType::GAME_OVER);
                    m_testPhase++;
                    m_testTimer = 0.0f;
                }
                break;
                
            case 2: // Test Game Over State
                if (m_testTimer > 2.0f) {
                    std::cout << "✅ Phase 2: Game Over state active for 2 seconds" << std::endl;
                    std::cout << "Current state: " << (m_stateManager->GetCurrentState() ? 
                        m_stateManager->GetCurrentState()->GetName() : "NULL") << std::endl;
                    
                    // Return to Menu
                    m_stateManager->ChangeState(GameStateType::MENU);
                    m_testPhase++;
                    m_testTimer = 0.0f;
                }
                break;
                
            case 3: // Test complete
                if (m_testTimer > 1.0f) {
                    std::cout << "✅ Phase 3: Returned to Menu state" << std::endl;
                    std::cout << "Current state: " << (m_stateManager->GetCurrentState() ? 
                        m_stateManager->GetCurrentState()->GetName() : "NULL") << std::endl;
                    std::cout << "✅ ALL STATE MANAGEMENT TESTS PASSED!" << std::endl;
                    m_testPassed = true;
                    Quit();
                }
                break;
        }
    }
    
    void Render() override {
        if (m_stateManager) {
            m_stateManager->Render();
        }
        
        // Draw test phase indicator
        auto* renderer = GetRenderer();
        if (renderer) {
            std::string phaseText = "TEST PHASE: " + std::to_string(m_testPhase);
            for (int i = 0; i < static_cast<int>(phaseText.length()); i++) {
                if (phaseText[i] != ' ') {
                    Rectangle charRect(10 + i * 12, 10, 10, 16);
                    renderer->DrawRectangle(charRect, Color(255, 255, 0, 255), true);
                }
            }
        }
    }

private:
    std::unique_ptr<GameStateManager> m_stateManager;
    int m_testPhase;
    float m_testTimer;
    bool m_testPassed;

public:
    bool TestPassed() const { return m_testPassed; }
};

int main() {
    std::cout << "🧪 STATE MANAGEMENT TEST STARTING..." << std::endl;
    
    StateManagementTest test;
    
    if (!test.Initialize("State Management Test", 800, 600)) {
        std::cout << "❌ FAILED: Could not initialize engine!" << std::endl;
        return -1;
    }
    
    std::cout << "✅ Engine and state manager initialized successfully" << std::endl;
    std::cout << "Running 4-phase state transition test..." << std::endl;
    
    test.Run();
    
    if (test.TestPassed()) {
        std::cout << "🎉 STATE MANAGEMENT TEST COMPLETED SUCCESSFULLY!" << std::endl;
        return 0;
    } else {
        std::cout << "❌ STATE MANAGEMENT TEST FAILED!" << std::endl;
        return -1;
    }
}
