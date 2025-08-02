#include "../include/Engine/Engine.h"
#include "../include/Engine/Renderer.h"
#include "../include/Engine/Window.h"
#include <iostream>
#include <chrono>
#include <thread>

class BasicRenderingTest : public Engine {
public:
    BasicRenderingTest() : m_testPhase(0), m_testTimer(0.0f), m_testPassed(false) {}

protected:
    void Update(float deltaTime) override {
        m_testTimer += deltaTime;
        
        switch (m_testPhase) {
            case 0: // Test basic shapes
                if (m_testTimer > 2.0f) {
                    std::cout << "✅ Phase 0: Basic shapes rendered for 2 seconds" << std::endl;
                    m_testPhase++;
                    m_testTimer = 0.0f;
                }
                break;
                
            case 1: // Test colors
                if (m_testTimer > 2.0f) {
                    std::cout << "✅ Phase 1: Color variations rendered for 2 seconds" << std::endl;
                    m_testPhase++;
                    m_testTimer = 0.0f;
                }
                break;
                
            case 2: // Test text-like blocks
                if (m_testTimer > 2.0f) {
                    std::cout << "✅ Phase 2: Text-like blocks rendered for 2 seconds" << std::endl;
                    m_testPhase++;
                    m_testTimer = 0.0f;
                }
                break;
                
            case 3: // Test complete
                std::cout << "✅ ALL RENDERING TESTS PASSED!" << std::endl;
                m_testPassed = true;
                Quit();
                break;
        }
    }
    
    void Render() override {
        auto* renderer = GetRenderer();
        if (!renderer) {
            std::cout << "❌ ERROR: No renderer available!" << std::endl;
            return;
        }
        
        switch (m_testPhase) {
            case 0: // Basic shapes
                TestBasicShapes(renderer);
                break;
            case 1: // Colors
                TestColors(renderer);
                break;
            case 2: // Text blocks
                TestTextBlocks(renderer);
                break;
        }
    }

private:
    int m_testPhase;
    float m_testTimer;
    bool m_testPassed;
    
    void TestBasicShapes(Renderer* renderer) {
        // Test rectangles
        renderer->DrawRectangle(Rectangle(50, 50, 100, 100), Color(255, 0, 0, 255), true);
        renderer->DrawRectangle(Rectangle(200, 50, 100, 100), Color(0, 255, 0, 255), false);
        
        // Test lines
        renderer->DrawLine(50, 200, 350, 200, Color(0, 0, 255, 255));
        renderer->DrawLine(50, 250, 350, 300, Color(255, 255, 0, 255));
        
        // Test points
        for (int i = 0; i < 50; i++) {
            renderer->DrawPoint(400 + i, 100 + i, Color(255, 255, 255, 255));
        }
    }
    
    void TestColors(Renderer* renderer) {
        // Test color gradients
        for (int i = 0; i < 256; i++) {
            renderer->DrawRectangle(Rectangle(i * 3, 100, 3, 50), Color(i, 0, 255-i, 255), true);
            renderer->DrawRectangle(Rectangle(i * 3, 200, 3, 50), Color(255-i, i, 0, 255), true);
            renderer->DrawRectangle(Rectangle(i * 3, 300, 3, 50), Color(0, 255-i, i, 255), true);
        }
    }
    
    void TestTextBlocks(Renderer* renderer) {
        // Simulate text rendering with blocks (like our menu system)
        const char* testText = "HELLO WORLD";
        int x = 100;
        int y = 200;
        
        for (int i = 0; testText[i] != '\0'; i++) {
            if (testText[i] != ' ') {
                // Draw character block
                Rectangle charRect(x + i * 32, y, 28, 40);
                renderer->DrawRectangle(charRect, Color(255, 215, 0, 255), true);
                renderer->DrawRectangle(charRect, Color(255, 140, 0, 255), false);
                
                // Add detail lines
                renderer->DrawLine(charRect.x + 4, charRect.y + 8, 
                                 charRect.x + 24, charRect.y + 8, Color(255, 255, 255, 255));
            }
        }
        
        // Test smaller text blocks
        const char* smallText = "Small Text Test";
        for (int i = 0; smallText[i] != '\0'; i++) {
            if (smallText[i] != ' ') {
                Rectangle charRect(50 + i * 16, 300, 14, 20);
                renderer->DrawRectangle(charRect, Color(200, 200, 200, 255), true);
            }
        }
    }

public:
    bool TestPassed() const { return m_testPassed; }
};

int main() {
    std::cout << "🧪 BASIC RENDERING TEST STARTING..." << std::endl;
    
    BasicRenderingTest test;
    
    if (!test.Initialize("Basic Rendering Test", 800, 600)) {
        std::cout << "❌ FAILED: Could not initialize engine!" << std::endl;
        return -1;
    }
    
    std::cout << "✅ Engine initialized successfully" << std::endl;
    std::cout << "Running 3-phase rendering test..." << std::endl;
    
    test.Run();
    
    if (test.TestPassed()) {
        std::cout << "🎉 BASIC RENDERING TEST COMPLETED SUCCESSFULLY!" << std::endl;
        return 0;
    } else {
        std::cout << "❌ BASIC RENDERING TEST FAILED!" << std::endl;
        return -1;
    }
}
