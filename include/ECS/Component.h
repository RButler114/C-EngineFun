/**
 * @file Component.h
 * @brief Component definitions for the Entity-Component-System architecture
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "Entity.h"
#include <string>

/**
 * @struct Component
 * @brief Base component structure for all ECS components
 *
 * All components in the ECS system inherit from this base structure.
 * Components are pure data containers with no behavior - they only
 * store the state that entities possess.
 *
 * @note No virtual functions are used to avoid memory layout issues
 *       and maintain cache-friendly data structures.
 */
struct Component {
    Entity owner; ///< The entity that owns this component

    /**
     * @brief Default constructor
     */
    Component() = default;

    /**
     * @brief Constructor with entity owner
     * @param e The entity that will own this component
     */
    Component(Entity e) : owner(e) {}
};

/**
 * @defgroup Components Common ECS Components
 * @brief Standard components used throughout the game engine
 * @{
 */

/**
 * @struct TransformComponent
 * @brief Component that defines an entity's position, rotation, and scale
 *
 * The TransformComponent is one of the most fundamental components,
 * defining where an entity exists in 2D space and how it's oriented.
 *
 * @example
 * ```cpp
 * Entity player = entityManager.CreateEntity();
 * entityManager.AddComponent<TransformComponent>(player, 100.0f, 200.0f);
 * ```
 */
struct TransformComponent : public Component {
    float x = 0.0f;         ///< X position in world coordinates
    float y = 0.0f;         ///< Y position in world coordinates
    float rotation = 0.0f;  ///< Rotation in radians
    float scaleX = 1.0f;    ///< X scale factor (1.0 = normal size)
    float scaleY = 1.0f;    ///< Y scale factor (1.0 = normal size)

    /**
     * @brief Default constructor - creates transform at origin
     */
    TransformComponent() = default;

    /**
     * @brief Constructor with position
     * @param posX X coordinate
     * @param posY Y coordinate
     */
    TransformComponent(float posX, float posY) : Component(), x(posX), y(posY) {}

    /**
     * @brief Constructor with position and rotation
     * @param posX X coordinate
     * @param posY Y coordinate
     * @param rot Rotation in radians
     */
    TransformComponent(float posX, float posY, float rot) : Component(), x(posX), y(posY), rotation(rot) {}
};

/**
 * @struct VelocityComponent
 * @brief Component that defines an entity's movement speed and direction
 *
 * Used by MovementSystem to update entity positions over time.
 * Velocity is measured in units per second.
 */
struct VelocityComponent : public Component {
    float vx = 0.0f; ///< X velocity in units per second
    float vy = 0.0f; ///< Y velocity in units per second

    /**
     * @brief Default constructor - creates stationary velocity
     */
    VelocityComponent() = default;

    /**
     * @brief Constructor with velocity values
     * @param velX X velocity
     * @param velY Y velocity
     */
    VelocityComponent(float velX, float velY) : Component(), vx(velX), vy(velY) {}
};

/**
 * @struct RenderComponent
 * @brief Component that defines how an entity should be rendered
 *
 * Contains visual properties like size, color, and visibility.
 * Used by RenderSystem to draw entities on screen.
 */
struct RenderComponent : public Component {
    int width = 32;             ///< Width in pixels
    int height = 32;            ///< Height in pixels
    unsigned char r = 255;      ///< Red color component (0-255)
    unsigned char g = 255;      ///< Green color component (0-255)
    unsigned char b = 255;      ///< Blue color component (0-255)
    unsigned char a = 255;      ///< Alpha transparency (0-255)
    bool visible = true;        ///< Whether the entity should be rendered

    /**
     * @brief Default constructor - creates white 32x32 rectangle
     */
    RenderComponent() = default;

    /**
     * @brief Constructor with size
     * @param w Width in pixels
     * @param h Height in pixels
     */
    RenderComponent(int w, int h) : Component(), width(w), height(h) {}

    /**
     * @brief Constructor with size and color
     * @param w Width in pixels
     * @param h Height in pixels
     * @param red Red component (0-255)
     * @param green Green component (0-255)
     * @param blue Blue component (0-255)
     */
    RenderComponent(int w, int h, unsigned char red, unsigned char green, unsigned char blue)
        : Component(), width(w), height(h), r(red), g(green), b(blue) {}
};

/**
 * @struct CollisionComponent
 * @brief Component that defines an entity's collision boundaries
 *
 * Used by CollisionSystem to detect when entities overlap.
 * Can be configured as a solid collider or a trigger.
 */
struct CollisionComponent : public Component {
    float width = 32.0f;    ///< Collision box width
    float height = 32.0f;   ///< Collision box height
    bool isTrigger = false; ///< If true, collision is detected but no physics response

    /**
     * @brief Default constructor - creates 32x32 solid collider
     */
    CollisionComponent() = default;

    /**
     * @brief Constructor with size
     * @param w Collision width
     * @param h Collision height
     */
    CollisionComponent(float w, float h) : Component(), width(w), height(h) {}

    /**
     * @brief Constructor with size and trigger flag
     * @param w Collision width
     * @param h Collision height
     * @param trigger Whether this is a trigger collider
     */
    CollisionComponent(float w, float h, bool trigger) : Component(), width(w), height(h), isTrigger(trigger) {}
};

/**
 * @struct AudioComponent
 * @brief Component that defines audio properties for an entity
 *
 * Used by AudioSystem to play sounds based on entity events or states.
 * Supports multiple sound effects per entity with different triggers.
 */
struct AudioComponent : public Component {
    std::string soundName;          ///< Name of the sound to play (must be loaded in AudioManager)
    float volume = 1.0f;            ///< Volume multiplier (0.0 - 1.0)
    float pitch = 1.0f;             ///< Pitch multiplier (not implemented in basic version)
    bool looping = false;           ///< Whether the sound should loop
    bool playOnCreate = false;      ///< Play sound when component is added
    bool playOnCollision = false;   ///< Play sound when entity collides
    bool playOnDestroy = false;     ///< Play sound when entity is destroyed
    bool is3D = false;              ///< Whether to use 3D positional audio
    float maxDistance = 1000.0f;    ///< Maximum distance for 3D audio
    int currentChannel = -1;        ///< Currently playing channel (-1 if not playing)

    /**
     * @brief Default constructor
     */
    AudioComponent() = default;

    /**
     * @brief Constructor with sound name
     * @param sound Name of the sound to play
     */
    AudioComponent(const std::string& sound) : Component(), soundName(sound) {}

    /**
     * @brief Constructor with sound name and volume
     * @param sound Name of the sound to play
     * @param vol Volume multiplier
     */
    AudioComponent(const std::string& sound, float vol) : Component(), soundName(sound), volume(vol) {}

    /**
     * @brief Constructor with full parameters
     * @param sound Name of the sound to play
     * @param vol Volume multiplier
     * @param loop Whether to loop the sound
     * @param onCreate Play on component creation
     * @param onCollision Play on collision
     */
    AudioComponent(const std::string& sound, float vol, bool loop, bool onCreate = false, bool onCollision = false)
        : Component(), soundName(sound), volume(vol), looping(loop), playOnCreate(onCreate), playOnCollision(onCollision) {}
};

/** @} */ // end of Components group
