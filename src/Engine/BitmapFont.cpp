/**
 * @file BitmapFont.cpp
 * @brief Implementation of ASCII art bitmap font system
 * @author Ryan Butler
 * @date 2025
 */

#include "Engine/BitmapFont.h"
#include <cctype>
#include <iostream>

/**
 * @brief Get ASCII art patterns for all supported characters
 *
 * Returns a static map containing ASCII art patterns for letters,
 * numbers, and common symbols. Each character is represented as
 * a 7-row bitmap pattern using '#' for filled pixels and spaces
 * for empty pixels.
 *
 * @return Map of character to vector of pattern strings
 *
 * @note Patterns are 5 characters wide, 7 rows tall
 * @note Static map ensures patterns are only created once
 * @note '#' represents filled pixels, ' ' represents empty pixels
 * @note Last row is always empty for character spacing
 *
 * Pattern Format:
 * - Width: 5 characters
 * - Height: 7 rows (6 for character + 1 empty for spacing)
 * - Characters: '#' for pixel, ' ' for empty space
 *
 * @example
 * ```cpp
 * auto patterns = BitmapFont::GetFontPatterns();
 * auto letterA = patterns['A'];
 * // letterA contains: {"  #  ", " # # ", "#####", "#   #", "#   #", "#   #", "     "}
 * ```
 */
std::unordered_map<char, std::vector<std::string>> BitmapFont::GetFontPatterns() {
    static std::unordered_map<char, std::vector<std::string>> patterns = {
        {'A', {"  #  ", " # # ", "#####", "#   #", "#   #", "#   #", "     "}},
        {'B', {"#### ", "#   #", "#### ", "#### ", "#   #", "#### ", "     "}},
        {'C', {" ####", "#    ", "#    ", "#    ", "#    ", " ####", "     "}},
        {'D', {"#### ", "#   #", "#   #", "#   #", "#   #", "#### ", "     "}},
        {'E', {"#####", "#    ", "#### ", "#### ", "#    ", "#####", "     "}},
        {'F', {"#####", "#    ", "#### ", "#### ", "#    ", "#    ", "     "}},
        {'G', {" ####", "#    ", "# ###", "#   #", "#   #", " ####", "     "}},
        {'H', {"#   #", "#   #", "#####", "#   #", "#   #", "#   #", "     "}},
        {'I', {"#####", "  #  ", "  #  ", "  #  ", "  #  ", "#####", "     "}},
        {'J', {"#####", "    #", "    #", "    #", "#   #", " ### ", "     "}},
        {'K', {"#   #", "#  # ", "###  ", "###  ", "#  # ", "#   #", "     "}},
        {'L', {"#    ", "#    ", "#    ", "#    ", "#    ", "#####", "     "}},
        {'M', {"#   #", "## ##", "# # #", "#   #", "#   #", "#   #", "     "}},
        {'N', {"#   #", "##  #", "# # #", "#  ##", "#   #", "#   #", "     "}},
        {'O', {" ### ", "#   #", "#   #", "#   #", "#   #", " ### ", "     "}},
        {'P', {"#### ", "#   #", "#### ", "#    ", "#    ", "#    ", "     "}},
        {'Q', {" ### ", "#   #", "#   #", "# # #", "#  ##", " ####", "     "}},
        {'R', {"#### ", "#   #", "#### ", "# #  ", "#  # ", "#   #", "     "}},
        {'S', {" ####", "#    ", " ### ", "    #", "    #", "#### ", "     "}},
        {'T', {"#####", "  #  ", "  #  ", "  #  ", "  #  ", "  #  ", "     "}},
        {'U', {"#   #", "#   #", "#   #", "#   #", "#   #", " ### ", "     "}},
        {'V', {"#   #", "#   #", "#   #", "#   #", " # # ", "  #  ", "     "}},
        {'W', {"#   #", "#   #", "#   #", "# # #", "## ##", "#   #", "     "}},
        {'X', {"#   #", " # # ", "  #  ", "  #  ", " # # ", "#   #", "     "}},
        {'Y', {"#   #", "#   #", " # # ", "  #  ", "  #  ", "  #  ", "     "}},
        {'Z', {"#####", "   # ", "  #  ", " #   ", "#    ", "#####", "     "}},
        {'0', {" ### ", "#   #", "#  ##", "# # #", "##  #", "#   #", " ### "}},
        {'1', {"  #  ", " ##  ", "  #  ", "  #  ", "  #  ", "  #  ", "#####"}},
        {'2', {" ### ", "#   #", "    #", "  ## ", " #   ", "#    ", "#####"}},
        {'3', {" ### ", "#   #", "    #", "  ## ", "    #", "#   #", " ### "}},
        {'4', {"   # ", "  ## ", " # # ", "#  # ", "#####", "   # ", "   # "}},
        {'5', {"#####", "#    ", "#### ", "    #", "    #", "#   #", " ### "}},
        {'6', {" ### ", "#   #", "#    ", "#### ", "#   #", "#   #", " ### "}},
        {'7', {"#####", "    #", "   # ", "  #  ", " #   ", "#    ", "#    "}},
        {'8', {" ### ", "#   #", "#   #", " ### ", "#   #", "#   #", " ### "}},
        {'9', {" ### ", "#   #", "#   #", " ####", "    #", "#   #", " ### "}},
        {' ', {"     ", "     ", "     ", "     ", "     ", "     ", "     "}},
        {'-', {"     ", "     ", "#####", "     ", "     ", "     ", "     "}},
        {':', {"     ", "  #  ", "     ", "     ", "  #  ", "     ", "     "}},
        {'.', {"     ", "     ", "     ", "     ", "     ", "  #  ", "     "}},
        {'!', {"  #  ", "  #  ", "  #  ", "  #  ", "     ", "  #  ", "     "}},
        {'?', {" ### ", "#   #", "   # ", "  #  ", "     ", "  #  ", "     "}},
    };
    return patterns;
}

void BitmapFont::DrawText(Renderer* renderer, const std::string& text, int x, int y, int scale, const Color& color) {
    auto patterns = GetFontPatterns();
    int currentX = x;

    for (char c : text) {
        char upperC = std::toupper(c);

        if (patterns.find(upperC) != patterns.end()) {
            const auto& pattern = patterns[upperC];

            for (int row = 0; row < 7; row++) {
                for (int col = 0; col < 5; col++) {
                    if (pattern[row][col] == '#') {
                        Rectangle pixelRect(currentX + col * scale, y + row * scale, scale, scale);
                        renderer->DrawRectangle(pixelRect, color, true);
                    }
                }
            }
        }

        currentX += 6 * scale; // Move to next character position (5 + 1 spacing)
    }
}
