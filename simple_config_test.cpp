#include "include/Engine/ConfigSystem.h"
#include <iostream>

int main() {
    std::cout << "=== Simple Config Test ===" << std::endl;
    
    ConfigManager config;
    if (!config.LoadFromFile("assets/config/gameplay.ini")) {
        std::cerr << "Failed to load gameplay.ini!" << std::endl;
        return 1;
    }
    
    std::cout << "Config loaded successfully!" << std::endl;
    
    // Test the specific values you changed
    int spriteWidth = config.Get("animation", "sprite_width", 18).AsInt();
    int spriteHeight = config.Get("animation", "sprite_height", 48).AsInt();
    int totalFrames = config.Get("animation", "total_frames", 3).AsInt();
    float frameDuration = config.Get("animation", "frame_duration", 0.15f).AsFloat();
    
    std::cout << "Sprite Width: " << spriteWidth << " (expected: 6)" << std::endl;
    std::cout << "Sprite Height: " << spriteHeight << " (expected: 16)" << std::endl;
    std::cout << "Total Frames: " << totalFrames << std::endl;
    std::cout << "Frame Duration: " << frameDuration << std::endl;
    
    // Verify the changes were applied
    if (spriteWidth == 6 && spriteHeight == 16) {
        std::cout << "✅ Config changes detected correctly!" << std::endl;
    } else {
        std::cout << "❌ Config changes NOT detected!" << std::endl;
        std::cout << "   Expected: width=6, height=16" << std::endl;
        std::cout << "   Got: width=" << spriteWidth << ", height=" << spriteHeight << std::endl;
    }
    
    return 0;
}
