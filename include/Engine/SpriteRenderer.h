/**
 * @file SpriteRenderer.h
 * @brief Simple sprite rendering utility for arcade games
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "Renderer.h"
#include <string>

/**
 * @struct SpriteFrame
 * @brief Represents a single frame in a sprite animation
 */
struct SpriteFrame {
    int x = 0;          ///< X position in sprite sheet
    int y = 0;          ///< Y position in sprite sheet
    int width = 32;     ///< Frame width
    int height = 32;    ///< Frame height
    
    SpriteFrame() = default;
    SpriteFrame(int x, int y, int w, int h) : x(x), y(y), width(w), height(h) {}
};

/**
 * @class SpriteRenderer
 * @brief Utility class for rendering sprites in arcade games
 * 
 * Provides simple, efficient sprite rendering with animation support.
 * Designed specifically for arcade-style games where direct control
 * over rendering is preferred over complex ECS systems.
 */
class SpriteRenderer {
public:
    /**
     * @brief Render a sprite at the specified position
     * @param renderer The renderer to use
     * @param texturePath Path to the sprite texture
     * @param x Screen X position
     * @param y Screen Y position
     * @param frame The sprite frame to render
     * @param flipHorizontal Whether to flip the sprite horizontally
     * @param scale Scale factor for the sprite
     */
    static void RenderSprite(Renderer* renderer, const std::string& texturePath, 
                           int x, int y, const SpriteFrame& frame, 
                           bool flipHorizontal = false, float scale = 1.0f);

    /**
     * @brief Render a simple sprite without animation frames
     * @param renderer The renderer to use
     * @param texturePath Path to the sprite texture
     * @param x Screen X position
     * @param y Screen Y position
     * @param width Sprite width
     * @param height Sprite height
     * @param flipHorizontal Whether to flip the sprite horizontally
     * @param scale Scale factor for the sprite
     */
    static void RenderSprite(Renderer* renderer, const std::string& texturePath,
                           int x, int y, int width, int height,
                           bool flipHorizontal = false, float scale = 1.0f);

    /**
     * @brief Create a sprite frame for animation
     * @param frameIndex The frame index (0-based)
     * @param frameWidth Width of each frame
     * @param frameHeight Height of each frame
     * @param framesPerRow Number of frames per row in sprite sheet
     * @return SpriteFrame representing the specified frame
     */
    static SpriteFrame CreateFrame(int frameIndex, int frameWidth, int frameHeight, int framesPerRow = 1);
};
