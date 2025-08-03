#include "Engine/Renderer.h"
#include <iostream>

// Texture implementation
Texture::Texture() : m_texture(nullptr), m_width(0), m_height(0) {}

Texture::~Texture() {
    Free();
}

bool Texture::LoadFromFile(const std::string& path, SDL_Renderer* renderer) {
    Free();



    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (!loadedSurface) {
        std::cerr << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }
    
    m_texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    if (!m_texture) {
        std::cerr << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << std::endl;
    } else {
        m_width = loadedSurface->w;
        m_height = loadedSurface->h;
    }
    
    SDL_FreeSurface(loadedSurface);
    return m_texture != nullptr;
}

void Texture::Free() {
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
        m_width = 0;
        m_height = 0;
    }
}

void Texture::Render(SDL_Renderer* renderer, int x, int y, SDL_Rect* clip) {
    SDL_Rect renderQuad = { x, y, m_width, m_height };
    
    if (clip) {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }
    
    SDL_RenderCopy(renderer, m_texture, clip, &renderQuad);
}

void Texture::Render(SDL_Renderer* renderer, int x, int y, int width, int height, SDL_Rect* clip) {
    SDL_Rect renderQuad = { x, y, width, height };
    SDL_RenderCopy(renderer, m_texture, clip, &renderQuad);
}

// Renderer implementation
Renderer::Renderer() : m_renderer(nullptr) {}

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Initialize(SDL_Window* window) {
    m_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_renderer) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }
    
    // Set default blend mode
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    
    std::cout << "Renderer created successfully" << std::endl;
    return true;
}

void Renderer::Shutdown() {
    m_textureCache.clear();
    
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
        std::cout << "Renderer destroyed" << std::endl;
    }
    
    IMG_Quit();
}

void Renderer::Clear(const Color& color) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(m_renderer);
}

void Renderer::Present() {
    SDL_RenderPresent(m_renderer);
}

void Renderer::DrawRectangle(const Rectangle& rect, const Color& color, bool filled) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_Rect sdlRect = { rect.x, rect.y, rect.width, rect.height };
    
    if (filled) {
        SDL_RenderFillRect(m_renderer, &sdlRect);
    } else {
        SDL_RenderDrawRect(m_renderer, &sdlRect);
    }
}

void Renderer::DrawLine(int x1, int y1, int x2, int y2, const Color& color) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(m_renderer, x1, y1, x2, y2);
}

void Renderer::DrawPoint(int x, int y, const Color& color) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(m_renderer, x, y);
}

std::shared_ptr<Texture> Renderer::LoadTexture(const std::string& path) {


    // Check if texture is already cached
    auto it = m_textureCache.find(path);
    if (it != m_textureCache.end()) {
        return it->second;
    }

    // Create new texture
    auto texture = std::make_shared<Texture>();
    if (texture->LoadFromFile(path, m_renderer)) {
        m_textureCache[path] = texture;
        return texture;
    }

    return nullptr;
}

void Renderer::DrawTexture(std::shared_ptr<Texture> texture, int x, int y) {
    if (texture) {
        texture->Render(m_renderer, x, y);
    }
}

void Renderer::DrawTexture(std::shared_ptr<Texture> texture, const Rectangle& srcRect, const Rectangle& destRect) {
    if (texture) {
        SDL_Rect src = { srcRect.x, srcRect.y, srcRect.width, srcRect.height };
        texture->Render(m_renderer, destRect.x, destRect.y, destRect.width, destRect.height, &src);
    }
}

void Renderer::DrawTexture(std::shared_ptr<Texture> texture, const Rectangle& srcRect, const Rectangle& destRect, bool flipHorizontal, bool flipVertical) {
    if (!texture) return;

    SDL_Rect src = { srcRect.x, srcRect.y, srcRect.width, srcRect.height };
    SDL_Rect dest = { destRect.x, destRect.y, destRect.width, destRect.height };

    // Determine flip flags
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (flipHorizontal) flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_HORIZONTAL);
    if (flipVertical) flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_VERTICAL);

    // Use SDL_RenderCopyEx for flipping support
    SDL_RenderCopyEx(m_renderer, texture->GetSDLTexture(), &src, &dest, 0.0, nullptr, flip);
}
