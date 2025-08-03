#include "Game/SpriteManager.h"
#include <iostream>

SpriteManager::SpriteManager(Renderer* renderer) : m_renderer(renderer) {
}

void SpriteManager::AddSprite(Entity entity, const std::string& texturePath, int width, int height) {
    SpriteData sprite(texturePath, width, height);
    m_sprites[entity.GetID()] = sprite;
    std::cout << "SpriteManager: Added sprite for entity " << entity.GetID() << " with texture: " << texturePath << std::endl;
}

void SpriteManager::AddSprite(Entity entity, const std::string& texturePath, int width, int height, int frameWidth, int frameHeight) {
    SpriteData sprite;
    sprite.texturePath = texturePath;
    sprite.width = width;
    sprite.height = height;
    sprite.frameWidth = frameWidth;
    sprite.frameHeight = frameHeight;
    sprite.frameX = 0;  // Start with first frame
    sprite.frameY = 0;

    m_sprites[entity.GetID()] = sprite;
    std::cout << "SpriteManager: Added sprite for entity " << entity.GetID()
              << " with texture: " << texturePath
              << " display(" << width << "x" << height << ") frame(" << frameWidth << "x" << frameHeight << ")" << std::endl;
}

void SpriteManager::RemoveSprite(Entity entity) {
    m_sprites.erase(entity.GetID());
}

bool SpriteManager::HasSprite(Entity entity) const {
    return m_sprites.find(entity.GetID()) != m_sprites.end();
}

SpriteData* SpriteManager::GetSprite(Entity entity) {
    auto it = m_sprites.find(entity.GetID());
    if (it != m_sprites.end()) {
        return &it->second;
    }
    return nullptr;
}

void SpriteManager::RenderSprite(Entity entity, float x, float y) {
    if (!m_renderer) return;

    auto* sprite = GetSprite(entity);
    if (!sprite || !sprite->visible) return;

    // Load texture
    auto texture = m_renderer->LoadTexture(sprite->texturePath);
    if (!texture) {
        // Render placeholder rectangle if texture fails to load
        Rectangle rect(
            static_cast<int>(x),
            static_cast<int>(y),
            sprite->width,
            sprite->height
        );
        Color placeholderColor(255, 0, 255, 255); // Magenta placeholder
        m_renderer->DrawRectangle(rect, placeholderColor, true);
        return;
    }

    // Calculate destination rectangle
    int destWidth = static_cast<int>(sprite->width * sprite->scaleX);
    int destHeight = static_cast<int>(sprite->height * sprite->scaleY);
    
    Rectangle destRect(
        static_cast<int>(x),
        static_cast<int>(y),
        destWidth,
        destHeight
    );
    
    // Create source rectangle for sprite sheet frame
    Rectangle srcRect(
        sprite->frameX,
        sprite->frameY,
        sprite->frameWidth,
        sprite->frameHeight
    );
    
    // Render the sprite
    m_renderer->DrawTexture(texture, srcRect, destRect);
}

void SpriteManager::SetSpriteFrame(Entity entity, int frameX, int frameY) {
    auto* sprite = GetSprite(entity);
    if (sprite) {
        sprite->frameX = frameX;
        sprite->frameY = frameY;
        std::cout << "SpriteManager: Set frame for entity " << entity.GetID() << " to (" << frameX << ", " << frameY << ")" << std::endl;
    }
}

void SpriteManager::Clear() {
    m_sprites.clear();
}
