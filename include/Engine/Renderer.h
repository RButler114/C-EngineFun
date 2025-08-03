/**
 * @file Renderer.h
 * @brief 2D rendering system using SDL2 with texture and primitive support
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <unordered_map>
#include <memory>

/**
 * @struct Color
 * @brief RGBA color representation
 *
 * Simple color structure with red, green, blue, and alpha components.
 * Each component ranges from 0-255.
 *
 * @example
 * ```cpp
 * Color red(255, 0, 0, 255);        // Opaque red
 * Color transparentBlue(0, 0, 255, 128); // Semi-transparent blue
 * Color white;                       // Default: opaque white
 * ```
 */
struct Color {
    Uint8 r, g, b, a; ///< Red, green, blue, alpha components (0-255)

    /**
     * @brief Construct a color with RGBA values
     * @param red Red component (0-255, default: 255)
     * @param green Green component (0-255, default: 255)
     * @param blue Blue component (0-255, default: 255)
     * @param alpha Alpha component (0-255, default: 255)
     */
    Color(Uint8 red = 255, Uint8 green = 255, Uint8 blue = 255, Uint8 alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
};

/**
 * @struct Rectangle
 * @brief 2D rectangle with position and dimensions
 *
 * Simple rectangle structure for defining areas, collision boxes,
 * and rendering regions.
 *
 * @example
 * ```cpp
 * Rectangle playerBounds(100, 200, 32, 48); // x=100, y=200, 32x48 pixels
 * Rectangle screenArea(0, 0, 800, 600);     // Full screen area
 * ```
 */
struct Rectangle {
    int x, y, width, height; ///< Position (x,y) and dimensions (width,height)

    /**
     * @brief Construct a rectangle with position and size
     * @param x X coordinate (default: 0)
     * @param y Y coordinate (default: 0)
     * @param w Width in pixels (default: 0)
     * @param h Height in pixels (default: 0)
     */
    Rectangle(int x = 0, int y = 0, int w = 0, int h = 0)
        : x(x), y(y), width(w), height(h) {}
};

class Texture {
public:
    Texture();
    ~Texture();
    
    bool LoadFromFile(const std::string& path, SDL_Renderer* renderer);
    void Free();
    
    void Render(SDL_Renderer* renderer, int x, int y, SDL_Rect* clip = nullptr);
    void Render(SDL_Renderer* renderer, int x, int y, int width, int height, SDL_Rect* clip = nullptr);
    
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    SDL_Texture* GetSDLTexture() const { return m_texture; }

private:
    SDL_Texture* m_texture;
    int m_width;
    int m_height;
};

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool Initialize(SDL_Window* window);
    void Shutdown();

    // Basic rendering
    void Clear(const Color& color = Color(0, 0, 0, 255));
    void Present();

    // Shape rendering
    void DrawRectangle(const Rectangle& rect, const Color& color, bool filled = true);
    void DrawLine(int x1, int y1, int x2, int y2, const Color& color);
    void DrawPoint(int x, int y, const Color& color);

    // Texture rendering
    std::shared_ptr<Texture> LoadTexture(const std::string& path);
    void DrawTexture(std::shared_ptr<Texture> texture, int x, int y);
    void DrawTexture(std::shared_ptr<Texture> texture, const Rectangle& srcRect, const Rectangle& destRect);
    void DrawTexture(std::shared_ptr<Texture> texture, const Rectangle& srcRect, const Rectangle& destRect, bool flipHorizontal, bool flipVertical);

    // Getters
    SDL_Renderer* GetSDLRenderer() const { return m_renderer; }

private:
    SDL_Renderer* m_renderer;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textureCache;
};
