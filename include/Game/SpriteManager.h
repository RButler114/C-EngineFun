/**
 * @file SpriteManager.h
 * @brief Simple sprite management system
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "Engine/Renderer.h"
#include "ECS/Entity.h"
#include <string>
#include <unordered_map>

/**
 * @struct SpriteData
 * @brief Simple sprite data structure
 */
struct SpriteData {
    std::string texturePath;
    int width = 32;
    int height = 32;
    int frameX = 0;
    int frameY = 0;
    int frameWidth = 32;
    int frameHeight = 32;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    bool visible = true;

    SpriteData() = default;
    SpriteData(const std::string& path, int w, int h)
        : texturePath(path), width(w), height(h), frameWidth(w), frameHeight(h) {}
};

/**
 * @class SpriteManager
 * @brief Simple sprite management system that works alongside the ECS
 */
class SpriteManager {
public:
    SpriteManager(Renderer* renderer);
    ~SpriteManager() = default;

    /**
     * @brief Add a sprite for an entity
     */
    void AddSprite(Entity entity, const std::string& texturePath, int width, int height);

    /**
     * @brief Add a sprite for an entity with frame dimensions
     */
    void AddSprite(Entity entity, const std::string& texturePath, int width, int height, int frameWidth, int frameHeight);

    /**
     * @brief Remove sprite for an entity
     */
    void RemoveSprite(Entity entity);

    /**
     * @brief Check if entity has a sprite
     */
    bool HasSprite(Entity entity) const;

    /**
     * @brief Get sprite data for an entity
     */
    SpriteData* GetSprite(Entity entity);

    /**
     * @brief Set the current frame for a sprite
     */
    void SetSpriteFrame(Entity entity, int frameX, int frameY);

    /**
     * @brief Render a sprite at the given position
     */
    void RenderSprite(Entity entity, float x, float y);

    /**
     * @brief Clear all sprites
     */
    void Clear();

private:
    Renderer* m_renderer;
    std::unordered_map<EntityID, SpriteData> m_sprites;
};
