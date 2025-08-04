/**
 * @file SpriteRenderer.cpp
 * @brief Implementation of sprite rendering utility
 * @author Ryan Butler
 * @date 2025
 */

#include "Engine/SpriteRenderer.h"
#include <iostream>

void SpriteRenderer::RenderSprite(Renderer* renderer, const std::string& texturePath, 
                                 int x, int y, const SpriteFrame& frame, 
                                 bool flipHorizontal, float scale) {
    if (!renderer) {
        return;
    }

    auto texture = renderer->LoadTexture(texturePath);
    if (!texture) {
        // Render placeholder rectangle if texture fails to load
        Rectangle rect(x, y, static_cast<int>(frame.width * scale), static_cast<int>(frame.height * scale));
        Color placeholderColor(255, 0, 255, 255); // Magenta placeholder
        renderer->DrawRectangle(rect, placeholderColor, true);
        return;
    }

    // Source rectangle (from sprite sheet)
    Rectangle srcRect(frame.x, frame.y, frame.width, frame.height);
    
    // Destination rectangle (on screen)
    Rectangle destRect(x, y, static_cast<int>(frame.width * scale), static_cast<int>(frame.height * scale));
    
    // Render with optional horizontal flipping
    renderer->DrawTexture(texture, srcRect, destRect, flipHorizontal, false);
}

void SpriteRenderer::RenderSprite(Renderer* renderer, const std::string& texturePath,
                                 int x, int y, int width, int height,
                                 bool flipHorizontal, float scale) {
    SpriteFrame frame(0, 0, width, height);
    RenderSprite(renderer, texturePath, x, y, frame, flipHorizontal, scale);
}

SpriteFrame SpriteRenderer::CreateFrame(int frameIndex, int frameWidth, int frameHeight, int framesPerRow) {
    int row = frameIndex / framesPerRow;
    int col = frameIndex % framesPerRow;
    
    return SpriteFrame(col * frameWidth, row * frameHeight, frameWidth, frameHeight);
}
