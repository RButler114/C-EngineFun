/**
 * @file AudioSystem.cpp
 * @brief Implementation of the AudioSystem class
 * @author Ryan Butler
 * @date 2025
 */

#include "ECS/AudioSystem.h"
#include "ECS/EntityManager.h"
#include <SDL2/SDL_mixer.h>
#include <cmath>
#include <iostream>

AudioSystem::AudioSystem(AudioManager& audioManager) 
    : m_audioManager(audioManager) {
}

void AudioSystem::Update(float deltaTime) {
    (void)deltaTime; // Suppress unused parameter warning
    for (Entity entity : m_entities) {
        AudioComponent* audioComp = m_entityManager->GetComponent<AudioComponent>(entity);
        if (!audioComp) continue;

        // Handle 3D audio if enabled and entity has transform
        if (audioComp->is3D) {
            TransformComponent* transform = m_entityManager->GetComponent<TransformComponent>(entity);
            if (transform) {
                // Calculate 3D volume based on distance
                float volume3D = Calculate3DVolume(transform->x, transform->y, audioComp->maxDistance);
                
                // If entity is playing a sound, update its volume
                if (audioComp->currentChannel != -1 && Mix_Playing(audioComp->currentChannel)) {
                    int finalVolume = static_cast<int>(audioComp->volume * volume3D * MIX_MAX_VOLUME);
                    Mix_Volume(audioComp->currentChannel, finalVolume);
                }
            }
        }

        // Check if sound finished playing
        if (audioComp->currentChannel != -1 && !Mix_Playing(audioComp->currentChannel)) {
            audioComp->currentChannel = -1;
        }
    }
}

void AudioSystem::OnEntityAdded(Entity entity) {
    AudioComponent* audioComp = m_entityManager->GetComponent<AudioComponent>(entity);
    if (audioComp && audioComp->playOnCreate && !audioComp->soundName.empty()) {
        PlayEntitySound(entity);
    }
}

void AudioSystem::OnEntityRemoved(Entity entity) {
    AudioComponent* audioComp = m_entityManager->GetComponent<AudioComponent>(entity);
    if (audioComp) {
        // Play destroy sound if configured
        if (audioComp->playOnDestroy && !audioComp->soundName.empty()) {
            // Play the sound but don't store the channel since entity is being destroyed
            float volume = audioComp->volume;
            if (audioComp->is3D) {
                TransformComponent* transform = m_entityManager->GetComponent<TransformComponent>(entity);
                if (transform) {
                    volume *= Calculate3DVolume(transform->x, transform->y, audioComp->maxDistance);
                }
            }
            m_audioManager.PlaySound(audioComp->soundName, volume, 0);
        }

        // Stop any currently playing sound
        if (audioComp->currentChannel != -1) {
            Mix_HaltChannel(audioComp->currentChannel);
        }
    }
}

int AudioSystem::PlayEntitySound(Entity entity, const std::string& soundName, float volume, int loops) {
    AudioComponent* audioComp = m_entityManager->GetComponent<AudioComponent>(entity);
    if (!audioComp) {
        return -1;
    }

    // Use provided sound name or component's sound name
    std::string actualSoundName = soundName.empty() ? audioComp->soundName : soundName;
    if (actualSoundName.empty()) {
        return -1;
    }

    // Calculate final volume
    float finalVolume = audioComp->volume * volume;

    // Apply 3D audio if enabled
    if (audioComp->is3D) {
        TransformComponent* transform = m_entityManager->GetComponent<TransformComponent>(entity);
        if (transform) {
            finalVolume *= Calculate3DVolume(transform->x, transform->y, audioComp->maxDistance);
        }
    }

    // Determine loops
    int actualLoops = (loops == 0 && audioComp->looping) ? -1 : loops;

    // Stop current sound if playing
    if (audioComp->currentChannel != -1) {
        Mix_HaltChannel(audioComp->currentChannel);
    }

    // Play the sound
    int channel = m_audioManager.PlaySound(actualSoundName, finalVolume, actualLoops);
    audioComp->currentChannel = channel;

    return channel;
}

void AudioSystem::StopEntitySound(Entity entity) {
    AudioComponent* audioComp = m_entityManager->GetComponent<AudioComponent>(entity);
    if (audioComp && audioComp->currentChannel != -1) {
        Mix_HaltChannel(audioComp->currentChannel);
        audioComp->currentChannel = -1;
    }
}

void AudioSystem::OnEntityCollision(Entity entity) {
    AudioComponent* audioComp = m_entityManager->GetComponent<AudioComponent>(entity);
    if (audioComp && audioComp->playOnCollision && !audioComp->soundName.empty()) {
        PlayEntitySound(entity);
    }
}

void AudioSystem::SetListenerPosition(float x, float y) {
    m_listenerX = x;
    m_listenerY = y;
}

void AudioSystem::GetListenerPosition(float& x, float& y) const {
    x = m_listenerX;
    y = m_listenerY;
}

bool AudioSystem::HasRequiredComponents(Entity entity) const {
    return m_entityManager->HasComponent<AudioComponent>(entity);
}

float AudioSystem::Calculate3DVolume(float entityX, float entityY, float maxDistance) const {
    float distance = CalculateDistance(entityX, entityY, m_listenerX, m_listenerY);
    
    if (distance >= maxDistance) {
        return 0.0f;
    }
    
    // Linear falloff - could be improved with more sophisticated curves
    return 1.0f - (distance / maxDistance);
}

float AudioSystem::CalculateDistance(float x1, float y1, float x2, float y2) const {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
}
