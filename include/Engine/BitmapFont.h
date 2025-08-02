#pragma once

#include "Engine/Renderer.h"
#include <string>
#include <unordered_map>
#include <vector>

// Simple bitmap font system for readable text rendering
class BitmapFont {
public:
    // Draw text using bitmap font patterns
    static void DrawText(Renderer* renderer, const std::string& text, int x, int y, int scale, const Color& color);
    
private:
    // 5x7 pixel font patterns
    static std::unordered_map<char, std::vector<std::string>> GetFontPatterns();
};
