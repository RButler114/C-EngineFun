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

/**
 * @class Texture
 * @brief Hardware-accelerated texture wrapper for SDL2
 *
 * The Texture class provides a high-level interface for loading, managing,
 * and rendering 2D textures using SDL2. It handles:
 * - Image file loading (PNG, JPG, BMP, etc.)
 * - GPU texture creation and management
 * - Rendering with clipping and scaling support
 * - Automatic resource cleanup
 *
 * Features:
 * - Hardware acceleration (GPU memory)
 * - Transparency support (alpha channel)
 * - Efficient rendering operations
 * - RAII resource management
 *
 * @example
 * ```cpp
 * Texture playerTexture;
 * if (playerTexture.LoadFromFile("player.png", renderer)) {
 *     playerTexture.Render(renderer, 100, 200); // Draw at (100, 200)
 * }
 * ```
 */
class Texture {
public:
    /**
     * @brief Default constructor - creates empty texture
     *
     * Initializes texture to safe default state. Call LoadFromFile()
     * to actually load image data.
     */
    Texture();

    /**
     * @brief Destructor - automatically frees texture resources
     *
     * Calls Free() to release SDL texture and video memory.
     * Follows RAII principles for automatic cleanup.
     */
    ~Texture();

    /**
     * @brief Load texture from image file
     *
     * @param path File path to image (PNG, JPG, BMP, etc.)
     * @param renderer SDL renderer to create texture with
     * @return true if loaded successfully, false on failure
     *
     * @note Automatically frees any existing texture before loading
     * @note Supports transparency (PNG alpha channel)
     */
    bool LoadFromFile(const std::string& path, SDL_Renderer* renderer);

    /**
     * @brief Free texture resources and reset state
     *
     * Releases SDL texture and video memory, resets dimensions to zero.
     * Safe to call multiple times.
     */
    void Free();

    /**
     * @brief Render texture at position with optional clipping
     *
     * @param renderer SDL renderer to draw with
     * @param x Screen X coordinate
     * @param y Screen Y coordinate
     * @param clip Optional source rectangle (nullptr = full texture)
     */
    void Render(SDL_Renderer* renderer, int x, int y, SDL_Rect* clip = nullptr);

    /**
     * @brief Render texture with custom size and optional clipping
     *
     * @param renderer SDL renderer to draw with
     * @param x Screen X coordinate
     * @param y Screen Y coordinate
     * @param width Desired render width
     * @param height Desired render height
     * @param clip Optional source rectangle (nullptr = full texture)
     */
    void Render(SDL_Renderer* renderer, int x, int y, int width, int height, SDL_Rect* clip = nullptr);

    /**
     * @brief Get texture width in pixels
     * @return Width of loaded texture, or 0 if not loaded
     */
    int GetWidth() const { return m_width; }

    /**
     * @brief Get texture height in pixels
     * @return Height of loaded texture, or 0 if not loaded
     */
    int GetHeight() const { return m_height; }

    /**
     * @brief Get underlying SDL texture handle
     * @return Pointer to SDL_Texture, or nullptr if not loaded
     * @note For advanced use cases - prefer Render() methods
     */
    SDL_Texture* GetSDLTexture() const { return m_texture; }

private:
    SDL_Texture* m_texture;  ///< SDL texture handle (GPU memory)
    int m_width;             ///< Texture width in pixels
    int m_height;            ///< Texture height in pixels
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
