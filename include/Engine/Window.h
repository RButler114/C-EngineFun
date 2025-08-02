#pragma once

#include <SDL2/SDL.h>
#include <string>

class Window {
public:
    Window();
    ~Window();

    bool Initialize(const char* title, int width, int height);
    void Shutdown();

    // Getters
    SDL_Window* GetSDLWindow() const { return m_window; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    const std::string& GetTitle() const { return m_title; }

    // Window operations
    void SetTitle(const char* title);
    void SetSize(int width, int height);
    void SetFullscreen(bool fullscreen);
    
    bool IsFullscreen() const { return m_isFullscreen; }

private:
    SDL_Window* m_window;
    int m_width;
    int m_height;
    std::string m_title;
    bool m_isFullscreen;
};
