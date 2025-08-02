#include "Engine/Window.h"
#include <iostream>

Window::Window()
    : m_window(nullptr)
    , m_width(0)
    , m_height(0)
    , m_isFullscreen(false)
{
}

Window::~Window() {
    Shutdown();
}

bool Window::Initialize(const char* title, int width, int height) {
    m_title = title;
    m_width = width;
    m_height = height;

    // Create window
    m_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!m_window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    std::cout << "Window created: " << title << " (" << width << "x" << height << ")" << std::endl;
    return true;
}

void Window::Shutdown() {
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
        std::cout << "Window destroyed" << std::endl;
    }
}

void Window::SetTitle(const char* title) {
    if (m_window && title) {
        m_title = title;
        SDL_SetWindowTitle(m_window, title);
    }
}

void Window::SetSize(int width, int height) {
    if (m_window && width > 0 && height > 0) {
        m_width = width;
        m_height = height;
        SDL_SetWindowSize(m_window, width, height);
    }
}

void Window::SetFullscreen(bool fullscreen) {
    if (m_window) {
        Uint32 flags = fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
        SDL_SetWindowFullscreen(m_window, flags);
        m_isFullscreen = fullscreen;
        
        if (!fullscreen) {
            // Reset window size when exiting fullscreen
            SDL_SetWindowSize(m_window, m_width, m_height);
            SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }
    }
}
