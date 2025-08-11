#include "../include/Engine/Engine.h"
#include "../include/Engine/Renderer.h"
#include <iostream>
#include <vector>
#include <string>

class MenuRenderingTest : public Engine {
public:
    MenuRenderingTest() : m_testTimer(0.0f) {}

protected:
    void Update(float deltaTime) override {
        m_testTimer += deltaTime;
        
        // Auto-quit after 5 seconds
        if (m_testTimer > 5.0f) {
            std::cout << "✅ Menu rendering test completed after 5 seconds" << std::endl;
            Quit();
        }
    }
    
    void Render() override {
        auto* renderer = GetRenderer();
        if (!renderer) {
            std::cout << "❌ ERROR: No renderer available!" << std::endl;
            return;
        }
        
        std::cout << "🎨 Rendering menu elements..." << std::endl;
        
        // Test 1: Draw background gradient (like in MenuState)
        for (int y = 0; y < 600; y += 4) {
            int intensity = 20 + (y * 40 / 600);
            Color bgColor(intensity, intensity, intensity * 2, 255);
            renderer->DrawRectangle(Rectangle(0, y, 800, 4), bgColor, true);
        }
        std::cout << "✅ Background gradient drawn" << std::endl;
        
        // Test 2: Draw title blocks (exactly like MenuState)
        const char* title = "Everharvest Voyager V";
        int titleWidth = strlen(title) * 32;
        int startX = (800 - titleWidth) / 2;
        int titleY = 150;
        
        std::cout << "Drawing title: " << title << " at position (" << startX << ", " << titleY << ")" << std::endl;
        
        for (int i = 0; title[i] != '\0'; i++) {
            if (title[i] != ' ') {
                // Draw letter block with outline
                Rectangle letterRect(startX + i * 32, titleY, 28, 40);
                renderer->DrawRectangle(letterRect, Color(255, 215, 0, 255), true); // Gold
                renderer->DrawRectangle(letterRect, Color(255, 140, 0, 255), false); // Orange outline
                
                // Add some detail lines
                renderer->DrawLine(letterRect.x + 4, letterRect.y + 8, 
                                 letterRect.x + 24, letterRect.y + 8, Color(255, 255, 255, 255));
                renderer->DrawLine(letterRect.x + 4, letterRect.y + 32, 
                                 letterRect.x + 24, letterRect.y + 32, Color(200, 160, 0, 255));
                
                std::cout << "Drew letter block " << i << " at (" << letterRect.x << ", " << letterRect.y << ")" << std::endl;
            }
        }
        
        // Test 3: Draw menu options (exactly like MenuState)
        std::vector<std::string> menuOptions = {"START GAME", "OPTIONS", "QUIT"};
        int menuStartY = 350;
        int menuSpacing = 60;
        int selectedOption = 0; // Highlight first option
        
        for (int i = 0; i < static_cast<int>(menuOptions.size()); i++) {
            int optionY = menuStartY + i * menuSpacing;
            bool isSelected = (i == selectedOption);
            
            // Calculate text width for centering
            int textWidth = menuOptions[i].length() * 16;
            int textX = (800 - textWidth) / 2;
            
            std::cout << "Drawing menu option " << i << ": " << menuOptions[i] 
                      << " at (" << textX << ", " << optionY << ")" << std::endl;
            
            // Draw selection indicator
            if (isSelected) {
                // Draw selection box
                Rectangle selectionRect(textX - 20, optionY - 5, textWidth + 40, 30);
                renderer->DrawRectangle(selectionRect, Color(255, 0, 0, 100), true);
                renderer->DrawRectangle(selectionRect, Color(255, 255, 255, 255), false);
                
                // Draw arrows
                renderer->DrawLine(textX - 40, optionY + 10, textX - 25, optionY + 10, Color(255, 255, 0, 255));
                renderer->DrawLine(textX + textWidth + 25, optionY + 10, textX + textWidth + 40, optionY + 10, Color(255, 255, 0, 255));
                
                std::cout << "Drew selection indicator for option " << i << std::endl;
            }
            
            // Draw menu text as blocks
            for (int j = 0; j < static_cast<int>(menuOptions[i].length()); j++) {
                if (menuOptions[i][j] != ' ') {
                    Rectangle charRect(textX + j * 16, optionY, 14, 20);
                    Color textColor = isSelected ? Color(255, 255, 0, 255) : Color(200, 200, 200, 255);
                    renderer->DrawRectangle(charRect, textColor, true);
                }
            }
        }
        
        // Test 4: Draw instructions at bottom
        const char* instructions = "USE ARROW KEYS TO NAVIGATE - ENTER TO SELECT - ESC TO QUIT";
        int instrWidth = strlen(instructions) * 8;
        int instrX = (800 - instrWidth) / 2;
        
        std::cout << "Drawing instructions at (" << instrX << ", 550)" << std::endl;
        
        for (int i = 0; instructions[i] != '\0'; i++) {
            if (instructions[i] != ' ') {
                Rectangle charRect(instrX + i * 8, 550, 6, 12);
                renderer->DrawRectangle(charRect, Color(128, 128, 128, 255), true);
            }
        }
        
        // Test 5: Draw a simple test rectangle to verify basic rendering
        renderer->DrawRectangle(Rectangle(10, 10, 50, 50), Color(255, 0, 255, 255), true);
        std::cout << "Drew test rectangle at (10, 10)" << std::endl;
        
        std::cout << "🎨 All menu rendering completed for this frame" << std::endl;
    }

private:
    float m_testTimer;
};

int main() {
    std::cout << "🧪 MENU RENDERING DEBUG TEST" << std::endl;
    std::cout << "============================" << std::endl;
    std::cout << "This test will render the exact same elements as the menu state." << std::endl;
    std::cout << "If you see a blank window, there's a fundamental rendering issue." << std::endl;
    std::cout << "Check the console output for detailed rendering information." << std::endl;
    
    MenuRenderingTest test;
    
    if (!test.Initialize("Menu Rendering Debug Test", 800, 600)) {
        std::cout << "❌ FAILED: Could not initialize engine!" << std::endl;
        return -1;
    }
    
    std::cout << "✅ Engine initialized successfully" << std::endl;
    std::cout << "Window should now be open. Check for visual elements..." << std::endl;
    
    test.Run();
    
    std::cout << "🎉 MENU RENDERING TEST COMPLETED!" << std::endl;
    std::cout << "If you saw visual elements, rendering is working." << std::endl;
    std::cout << "If the window was blank, there's a rendering pipeline issue." << std::endl;
    
    return 0;
}
