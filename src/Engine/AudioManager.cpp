/**
 * @file AudioManager.cpp
 * @brief Implementation of the AudioManager class
 * @author Ryan Butler
 * @date 2025
 */

#include "Engine/AudioManager.h"
#include <iostream>
#include <algorithm>

AudioManager::AudioManager() {
    // Initialize category volumes
    m_categoryVolumes[SoundType::SOUND_EFFECT] = 1.0f;
    m_categoryVolumes[SoundType::MUSIC] = 1.0f;
    m_categoryVolumes[SoundType::VOICE] = 1.0f;
    m_categoryVolumes[SoundType::AMBIENT] = 1.0f;
}

AudioManager::~AudioManager() {
    Shutdown();
}

bool AudioManager::Initialize(int frequency, Uint16 format, int channels, int chunksize) {
    if (m_initialized) {
        std::cout << "AudioManager already initialized" << std::endl;
        return true;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(frequency, format, channels, chunksize) < 0) {
        std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    // Allocate mixing channels
    Mix_AllocateChannels(16); // 16 simultaneous sound effects

    m_initialized = true;
    std::cout << "AudioManager initialized successfully" << std::endl;
    std::cout << "  Frequency: " << frequency << " Hz" << std::endl;
    std::cout << "  Channels: " << channels << std::endl;
    std::cout << "  Chunk size: " << chunksize << std::endl;
    
    return true;
}

void AudioManager::Shutdown() {
    if (!m_initialized) {
        return;
    }

    // Stop all audio
    Mix_HaltChannel(-1);
    Mix_HaltMusic();

    // Unload all resources
    UnloadAll();

    // Close SDL_mixer
    Mix_CloseAudio();

    m_initialized = false;
    std::cout << "AudioManager shut down successfully" << std::endl;
}

bool AudioManager::LoadSound(const std::string& name, const std::string& filepath, SoundType type) {
    if (!m_initialized) {
        std::cerr << "AudioManager not initialized!" << std::endl;
        return false;
    }

    // Check if sound already loaded
    if (m_sounds.find(name) != m_sounds.end()) {
        std::cout << "Sound '" << name << "' already loaded" << std::endl;
        return true;
    }

    // Load the sound
    Mix_Chunk* chunk = Mix_LoadWAV(filepath.c_str());
    if (!chunk) {
        std::cerr << "Failed to load sound '" << name << "' from '" << filepath << "': " << Mix_GetError() << std::endl;
        return false;
    }

    // Store the sound
    m_sounds[name] = std::make_unique<Sound>(chunk, type);
    std::cout << "Loaded sound: " << name << " (" << filepath << ")" << std::endl;
    
    return true;
}

bool AudioManager::LoadMusic(const std::string& name, const std::string& filepath) {
    if (!m_initialized) {
        std::cerr << "AudioManager not initialized!" << std::endl;
        return false;
    }

    // Check if music already loaded
    if (m_music.find(name) != m_music.end()) {
        std::cout << "Music '" << name << "' already loaded" << std::endl;
        return true;
    }

    // Load the music
    Mix_Music* music = Mix_LoadMUS(filepath.c_str());
    if (!music) {
        std::cerr << "Failed to load music '" << name << "' from '" << filepath << "': " << Mix_GetError() << std::endl;
        return false;
    }

    // Store the music
    m_music[name] = std::make_unique<Music>(music);
    std::cout << "Loaded music: " << name << " (" << filepath << ")" << std::endl;
    
    return true;
}

int AudioManager::PlaySound(const std::string& name, float volume, int loops) {
    if (!m_initialized) {
        std::cerr << "AudioManager not initialized!" << std::endl;
        return -1;
    }

    auto it = m_sounds.find(name);
    if (it == m_sounds.end()) {
        std::cerr << "Sound '" << name << "' not found!" << std::endl;
        return -1;
    }

    Sound* sound = it->second.get();
    int finalVolume = CalculateFinalVolume(sound, volume);
    
    // Set the chunk volume
    Mix_VolumeChunk(sound->chunk, finalVolume);
    
    // Play the sound
    int channel = Mix_PlayChannel(-1, sound->chunk, loops);
    if (channel == -1) {
        std::cerr << "Failed to play sound '" << name << "': " << Mix_GetError() << std::endl;
    }
    
    return channel;
}

bool AudioManager::PlayMusic(const std::string& name, float volume, int loops) {
    if (!m_initialized) {
        std::cerr << "AudioManager not initialized!" << std::endl;
        return false;
    }

    auto it = m_music.find(name);
    if (it == m_music.end()) {
        std::cerr << "Music '" << name << "' not found!" << std::endl;
        return false;
    }

    // Stop current music if playing
    if (Mix_PlayingMusic()) {
        Mix_HaltMusic();
    }

    // Set music volume
    int finalVolume = static_cast<int>(std::clamp(volume * m_musicVolume * it->second->volume, 0.0f, 1.0f) * MIX_MAX_VOLUME);
    Mix_VolumeMusic(finalVolume);

    // Play the music
    if (Mix_PlayMusic(it->second->music, loops) == -1) {
        std::cerr << "Failed to play music '" << name << "': " << Mix_GetError() << std::endl;
        return false;
    }

    m_currentMusic = name;
    return true;
}

void AudioManager::StopMusic() {
    if (m_initialized && Mix_PlayingMusic()) {
        Mix_HaltMusic();
        m_currentMusic.clear();
    }
}

void AudioManager::PauseMusic(bool paused) {
    if (!m_initialized) return;

    if (paused) {
        if (Mix_PlayingMusic() && !Mix_PausedMusic()) {
            Mix_PauseMusic();
        }
    } else {
        if (Mix_PausedMusic()) {
            Mix_ResumeMusic();
        }
    }
}

void AudioManager::SetCategoryVolume(SoundType type, float volume) {
    m_categoryVolumes[type] = std::clamp(volume, 0.0f, 1.0f);
}

float AudioManager::GetCategoryVolume(SoundType type) const {
    auto it = m_categoryVolumes.find(type);
    return (it != m_categoryVolumes.end()) ? it->second : 1.0f;
}

void AudioManager::SetMusicVolume(float volume) {
    m_musicVolume = std::clamp(volume, 0.0f, 1.0f);
    
    // Update current music volume if playing
    if (m_initialized && Mix_PlayingMusic() && !m_currentMusic.empty()) {
        auto it = m_music.find(m_currentMusic);
        if (it != m_music.end()) {
            int finalVolume = static_cast<int>(m_musicVolume * it->second->volume * MIX_MAX_VOLUME);
            Mix_VolumeMusic(finalVolume);
        }
    }
}

float AudioManager::GetMusicVolume() const {
    return m_musicVolume;
}

bool AudioManager::IsMusicPlaying() const {
    return m_initialized && Mix_PlayingMusic();
}

void AudioManager::UnloadSound(const std::string& name) {
    auto it = m_sounds.find(name);
    if (it != m_sounds.end()) {
        m_sounds.erase(it);
        std::cout << "Unloaded sound: " << name << std::endl;
    }
}

void AudioManager::UnloadMusic(const std::string& name) {
    auto it = m_music.find(name);
    if (it != m_music.end()) {
        // Stop music if it's currently playing
        if (m_currentMusic == name) {
            StopMusic();
        }
        m_music.erase(it);
        std::cout << "Unloaded music: " << name << std::endl;
    }
}

void AudioManager::UnloadAll() {
    m_sounds.clear();
    m_music.clear();
    m_currentMusic.clear();
    std::cout << "Unloaded all audio resources" << std::endl;
}

int AudioManager::CalculateFinalVolume(const Sound* sound, float volumeMultiplier) const {
    float categoryVolume = GetCategoryVolume(sound->type);
    float finalVolume = sound->volume * categoryVolume * volumeMultiplier;
    return static_cast<int>(std::clamp(finalVolume, 0.0f, 1.0f) * MIX_MAX_VOLUME);
}
