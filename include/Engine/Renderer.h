#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <unordered_map>
#include <memory>

struct Color {
    Uint8 r, g, b, a;
    Color(Uint8 red = 255, Uint8 green = 255, Uint8 blue = 255, Uint8 alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
};

struct Rectangle {
    int x, y, width, height;
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

    // Getters
    SDL_Renderer* GetSDLRenderer() const { return m_renderer; }

private:
    SDL_Renderer* m_renderer;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textureCache;
};
