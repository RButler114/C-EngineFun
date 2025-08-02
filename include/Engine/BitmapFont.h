/**
 * @file BitmapFont.h
 * @brief Simple bitmap font system for pixel-perfect text rendering
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "Engine/Renderer.h"
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @class BitmapFont
 * @brief Static utility class for rendering pixel-perfect bitmap text
 *
 * The BitmapFont class provides a simple system for rendering text using
 * predefined 5x7 pixel font patterns. This is ideal for retro-style games
 * or situations where you need consistent, readable text without external
 * font files.
 *
 * Features:
 * - 5x7 pixel character patterns
 * - Scalable text rendering
 * - Custom color support
 * - No external dependencies
 * - Consistent pixel-perfect appearance
 *
 * @example
 * ```cpp
 * // Draw red text at position (100, 50) with 2x scale
 * BitmapFont::DrawText(renderer, "SCORE: 1000", 100, 50, 2, Color(255, 0, 0));
 *
 * // Draw white text at normal scale
 * BitmapFont::DrawText(renderer, "GAME OVER", 200, 300, 1, Color(255, 255, 255));
 * ```
 */
class BitmapFont {
public:
    /**
     * @brief Draw text using bitmap font patterns
     *
     * Renders the specified text string using 5x7 pixel font patterns.
     * Each character is drawn as a series of colored rectangles.
     *
     * @param renderer Pointer to the renderer for drawing
     * @param text Text string to render
     * @param x X position for the text (top-left corner)
     * @param y Y position for the text (top-left corner)
     * @param scale Scale factor for the text (1 = normal size, 2 = double size, etc.)
     * @param color Color to use for the text
     *
     * @note Supports uppercase letters, numbers, and basic punctuation
     * @note Characters are spaced with 1 pixel gap between them
     */
    static void DrawText(Renderer* renderer, const std::string& text, int x, int y, int scale, const Color& color);

private:
    /**
     * @brief Get the 5x7 pixel font patterns for all supported characters
     *
     * Returns a map of characters to their corresponding 5x7 pixel patterns.
     * Each pattern is represented as a vector of strings, where each string
     * represents a row of pixels ('*' = pixel on, ' ' = pixel off).
     *
     * @return Map of character to font pattern
     */
    static std::unordered_map<char, std::vector<std::string>> GetFontPatterns();
};
