/**
 * @file AudioSystem.h
 * @brief Audio system for the ECS architecture
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "ECS/System.h"
#include "ECS/Component.h"
#include "Engine/AudioManager.h"
#include <memory>

/**
 * @class AudioSystem
 * @brief System that processes entities with AudioComponent
 *
 * The AudioSystem handles audio playback for entities in the ECS.
 * It processes entities that have AudioComponent and optionally
 * TransformComponent for 3D positional audio.
 *
 * Features:
 * - Automatic sound playback based on component settings
 * - 3D positional audio with distance attenuation
 * - Event-driven sound effects (collision, creation, destruction)
 * - Volume and looping control per entity
 *
 * @example
 * ```cpp
 * // Add audio system with audio manager
 * auto* audioSystem = entityManager.AddSystem<AudioSystem>(audioManager);
 *
 * // Create entity with audio
 * Entity entity = entityManager.CreateEntity();
 * entityManager.AddComponent<TransformComponent>(entity, 100.0f, 200.0f);
 * entityManager.AddComponent<AudioComponent>(entity, "jump_sound", 0.8f, false, true);
 * ```
 */
class AudioSystem : public System {
public:
    /**
     * @brief Constructor with audio manager reference
     * @param audioManager Reference to the audio manager
     */
    explicit AudioSystem(AudioManager& audioManager);

    /**
     * @brief Destructor
     */
    ~AudioSystem() override = default;

    /**
     * @brief Update the audio system
     *
     * Processes all entities with AudioComponent and handles:
     * - Playing sounds based on component flags
     * - 3D positional audio calculations
     * - Volume adjustments based on distance
     *
     * @param deltaTime Time elapsed since last update
     */
    void Update(float deltaTime) override;

    /**
     * @brief Called when an entity is added to the system
     *
     * Handles playOnCreate audio events.
     *
     * @param entity The entity that was added
     */
    void OnEntityAdded(Entity entity) override;

    /**
     * @brief Called when an entity is removed from the system
     *
     * Handles playOnDestroy audio events and stops any playing sounds.
     *
     * @param entity The entity that was removed
     */
    void OnEntityRemoved(Entity entity) override;

    /**
     * @brief Play a sound for a specific entity
     *
     * @param entity Entity to play sound for
     * @param soundName Name of the sound to play (overrides component soundName)
     * @param volume Volume multiplier (multiplied with component volume)
     * @param loops Number of loops (-1 for infinite, 0 for once)
     * @return Channel number if successful, -1 if failed
     */
    int PlayEntitySound(Entity entity, const std::string& soundName = "", float volume = 1.0f, int loops = 0);

    /**
     * @brief Stop sound for a specific entity
     *
     * @param entity Entity to stop sound for
     */
    void StopEntitySound(Entity entity);

    /**
     * @brief Trigger collision sound for an entity
     *
     * Called by CollisionSystem when entities collide.
     *
     * @param entity Entity that collided
     */
    void OnEntityCollision(Entity entity);

    /**
     * @brief Set listener position for 3D audio
     *
     * Usually set to the camera or player position.
     *
     * @param x X position of the listener
     * @param y Y position of the listener
     */
    void SetListenerPosition(float x, float y);

    /**
     * @brief Get listener position
     *
     * @param x Output X position
     * @param y Output Y position
     */
    void GetListenerPosition(float& x, float& y) const;

protected:
    /**
     * @brief Check if entity has required components
     *
     * Entities need AudioComponent to be processed by this system.
     * TransformComponent is optional for 3D audio.
     *
     * @param entity Entity to check
     * @return true if entity has required components
     */
    bool HasRequiredComponents(Entity entity) const;

private:
    AudioManager& m_audioManager;   ///< Reference to the audio manager
    float m_listenerX = 0.0f;       ///< Listener X position for 3D audio
    float m_listenerY = 0.0f;       ///< Listener Y position for 3D audio

    /**
     * @brief Calculate 3D audio volume based on distance
     *
     * @param entityX Entity X position
     * @param entityY Entity Y position
     * @param maxDistance Maximum audible distance
     * @return Volume multiplier (0.0 - 1.0)
     */
    float Calculate3DVolume(float entityX, float entityY, float maxDistance) const;

    /**
     * @brief Calculate distance between two points
     *
     * @param x1 First point X
     * @param y1 First point Y
     * @param x2 Second point X
     * @param y2 Second point Y
     * @return Distance between points
     */
    float CalculateDistance(float x1, float y1, float x2, float y2) const;
};
