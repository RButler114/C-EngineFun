#include "../include/Engine/Engine.h"
#include "../include/Engine/InputManager.h"
#include <iostream>
#include <chrono>

class InputSystemTest : public Engine {
public:
    InputSystemTest() : m_testTimer(0.0f), m_keysPressed(0), m_testPassed(false) {}

protected:
    void Update(float deltaTime) override {
        m_testTimer += deltaTime;
        
        auto* input = GetInputManager();
        if (!input) {
            std::cout << "❌ ERROR: No input manager available!" << std::endl;
            Quit();
            return;
        }
        
        // Test various key presses
        bool anyKeyPressed = false;
        
        if (input->IsKeyPressed(SDL_SCANCODE_W)) {
            std::cout << "✅ W key detected" << std::endl;
            m_keysPressed |= 1;
            anyKeyPressed = true;
        }
        if (input->IsKeyPressed(SDL_SCANCODE_A)) {
            std::cout << "✅ A key detected" << std::endl;
            m_keysPressed |= 2;
            anyKeyPressed = true;
        }
        if (input->IsKeyPressed(SDL_SCANCODE_S)) {
            std::cout << "✅ S key detected" << std::endl;
            m_keysPressed |= 4;
            anyKeyPressed = true;
        }
        if (input->IsKeyPressed(SDL_SCANCODE_D)) {
            std::cout << "✅ D key detected" << std::endl;
            m_keysPressed |= 8;
            anyKeyPressed = true;
        }
        if (input->IsKeyPressed(SDL_SCANCODE_SPACE)) {
            std::cout << "✅ SPACE key detected" << std::endl;
            m_keysPressed |= 16;
            anyKeyPressed = true;
        }
        if (input->IsKeyPressed(SDL_SCANCODE_RETURN)) {
            std::cout << "✅ ENTER key detected" << std::endl;
            m_keysPressed |= 32;
            anyKeyPressed = true;
        }
        
        // Test mouse
        int mouseX, mouseY;
        input->GetMousePosition(mouseX, mouseY);
        static int lastMouseX = mouseX, lastMouseY = mouseY;
        if (mouseX != lastMouseX || mouseY != lastMouseY) {
            std::cout << "✅ Mouse movement detected: (" << mouseX << ", " << mouseY << ")" << std::endl;
            m_keysPressed |= 64;
            lastMouseX = mouseX;
            lastMouseY = mouseY;
        }
        
        if (input->IsMouseButtonPressed(MouseButton::LEFT)) {
            std::cout << "✅ Left mouse button detected" << std::endl;
            m_keysPressed |= 128;
            anyKeyPressed = true;
        }
        
        // Auto-complete test after 10 seconds or if escape is pressed
        if (input->IsKeyPressed(SDL_SCANCODE_ESCAPE) || m_testTimer > 10.0f) {
            std::cout << "✅ Input test completed!" << std::endl;
            std::cout << "Keys pressed bitmask: " << m_keysPressed << std::endl;
            m_testPassed = (m_keysPressed > 0); // At least some input detected
            Quit();
        }
        
        // Periodic status update
        static float statusTimer = 0.0f;
        statusTimer += deltaTime;
        if (statusTimer >= 2.0f) {
            std::cout << "Input test running... Press WASD, SPACE, ENTER, move mouse, click, or ESC to quit" << std::endl;
            std::cout << "Keys detected so far: " << m_keysPressed << std::endl;
            statusTimer = 0.0f;
        }
    }
    
    void Render() override {
        auto* renderer = GetRenderer();
        if (!renderer) return;
        
        // Draw input status
        renderer->DrawRectangle(Rectangle(0, 0, 800, 100), Color(50, 50, 50, 255), true);
        
        // Draw instructions
        const char* instructions = "PRESS KEYS TO TEST INPUT SYSTEM";
        for (int i = 0; instructions[i] != '\0'; i++) {
            if (instructions[i] != ' ') {
                Rectangle charRect(50 + i * 16, 20, 14, 20);
                renderer->DrawRectangle(charRect, Color(255, 255, 255, 255), true);
            }
        }
        
        const char* controls = "WASD SPACE ENTER MOUSE ESC";
        for (int i = 0; controls[i] != '\0'; i++) {
            if (controls[i] != ' ') {
                Rectangle charRect(100 + i * 12, 50, 10, 16);
                renderer->DrawRectangle(charRect, Color(200, 200, 200, 255), true);
            }
        }
        
        // Visual feedback for pressed keys
        if (m_keysPressed & 1) renderer->DrawRectangle(Rectangle(100, 150, 50, 50), Color(0, 255, 0, 255), true); // W
        if (m_keysPressed & 2) renderer->DrawRectangle(Rectangle(50, 200, 50, 50), Color(0, 255, 0, 255), true);  // A
        if (m_keysPressed & 4) renderer->DrawRectangle(Rectangle(100, 200, 50, 50), Color(0, 255, 0, 255), true); // S
        if (m_keysPressed & 8) renderer->DrawRectangle(Rectangle(150, 200, 50, 50), Color(0, 255, 0, 255), true); // D
        if (m_keysPressed & 16) renderer->DrawRectangle(Rectangle(250, 175, 100, 50), Color(0, 255, 0, 255), true); // SPACE
        if (m_keysPressed & 32) renderer->DrawRectangle(Rectangle(400, 175, 80, 50), Color(0, 255, 0, 255), true); // ENTER
        if (m_keysPressed & 64) renderer->DrawRectangle(Rectangle(500, 175, 60, 50), Color(0, 255, 0, 255), true); // MOUSE
        if (m_keysPressed & 128) renderer->DrawRectangle(Rectangle(600, 175, 60, 50), Color(0, 255, 0, 255), true); // CLICK
        
        // Draw timer
        std::string timerText = "TIME: " + std::to_string(static_cast<int>(m_testTimer));
        for (int i = 0; i < static_cast<int>(timerText.length()); i++) {
            if (timerText[i] != ' ') {
                Rectangle charRect(600 + i * 12, 20, 10, 16);
                renderer->DrawRectangle(charRect, Color(255, 255, 0, 255), true);
            }
        }
    }

private:
    float m_testTimer;
    int m_keysPressed;
    bool m_testPassed;

public:
    bool TestPassed() const { return m_testPassed; }
};

int main() {
    std::cout << "🧪 INPUT SYSTEM TEST STARTING..." << std::endl;
    std::cout << "This test will run for 10 seconds or until you press ESC" << std::endl;
    std::cout << "Try pressing WASD, SPACE, ENTER, moving mouse, and clicking" << std::endl;
    
    InputSystemTest test;
    
    if (!test.Initialize("Input System Test", 800, 600)) {
        std::cout << "❌ FAILED: Could not initialize engine!" << std::endl;
        return -1;
    }
    
    std::cout << "✅ Engine initialized successfully" << std::endl;
    
    test.Run();
    
    if (test.TestPassed()) {
        std::cout << "🎉 INPUT SYSTEM TEST COMPLETED SUCCESSFULLY!" << std::endl;
        return 0;
    } else {
        std::cout << "❌ INPUT SYSTEM TEST FAILED - No input detected!" << std::endl;
        return -1;
    }
}
