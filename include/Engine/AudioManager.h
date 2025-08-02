/**
 * @file AudioManager.h
 * @brief Audio management system for the game engine
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include <SDL2/SDL_mixer.h>
#include <unordered_map>
#include <string>
#include <memory>

/**
 * @enum SoundType
 * @brief Categorizes different types of audio for volume control
 */
enum class SoundType {
    SOUND_EFFECT,   ///< Short sound effects (jump, collision, etc.)
    MUSIC,          ///< Background music tracks
    VOICE,          ///< Voice/dialogue audio
    AMBIENT         ///< Ambient/environmental sounds
};

/**
 * @struct Sound
 * @brief Wrapper for SDL_mixer sound data
 */
struct Sound {
    Mix_Chunk* chunk = nullptr;     ///< SDL_mixer sound chunk
    SoundType type = SoundType::SOUND_EFFECT;  ///< Sound category
    float volume = 1.0f;            ///< Base volume (0.0 - 1.0)
    
    Sound() = default;
    Sound(Mix_Chunk* c, SoundType t, float v = 1.0f) : chunk(c), type(t), volume(v) {}
    ~Sound() {
        if (chunk) {
            Mix_FreeChunk(chunk);
        }
    }
    
    // Make Sound non-copyable but movable
    Sound(const Sound&) = delete;
    Sound& operator=(const Sound&) = delete;
    Sound(Sound&& other) noexcept : chunk(other.chunk), type(other.type), volume(other.volume) {
        other.chunk = nullptr;
    }
    Sound& operator=(Sound&& other) noexcept {
        if (this != &other) {
            if (chunk) Mix_FreeChunk(chunk);
            chunk = other.chunk;
            type = other.type;
            volume = other.volume;
            other.chunk = nullptr;
        }
        return *this;
    }
};

/**
 * @struct Music
 * @brief Wrapper for SDL_mixer music data
 */
struct Music {
    Mix_Music* music = nullptr;     ///< SDL_mixer music
    float volume = 1.0f;            ///< Base volume (0.0 - 1.0)
    
    Music() = default;
    Music(Mix_Music* m, float v = 1.0f) : music(m), volume(v) {}
    ~Music() {
        if (music) {
            Mix_FreeMusic(music);
        }
    }
    
    // Make Music non-copyable but movable
    Music(const Music&) = delete;
    Music& operator=(const Music&) = delete;
    Music(Music&& other) noexcept : music(other.music), volume(other.volume) {
        other.music = nullptr;
    }
    Music& operator=(Music&& other) noexcept {
        if (this != &other) {
            if (music) Mix_FreeMusic(music);
            music = other.music;
            volume = other.volume;
            other.music = nullptr;
        }
        return *this;
    }
};

/**
 * @class AudioManager
 * @brief Manages all audio operations for the game engine
 *
 * The AudioManager provides a complete audio system with:
 * - Sound effect and music loading/playing
 * - Volume control by category
 * - 3D positional audio support
 * - Resource management and cleanup
 *
 * @example
 * ```cpp
 * AudioManager audioManager;
 * if (audioManager.Initialize()) {
 *     audioManager.LoadSound("jump", "assets/sounds/jump.wav", SoundType::SOUND_EFFECT);
 *     audioManager.PlaySound("jump");
 * }
 * ```
 */
class AudioManager {
public:
    /**
     * @brief Default constructor
     */
    AudioManager();

    /**
     * @brief Destructor - automatically calls Shutdown()
     */
    ~AudioManager();

    /**
     * @brief Initialize the audio system
     *
     * Sets up SDL_mixer with appropriate audio format and channels.
     * Must be called before any other audio operations.
     *
     * @param frequency Audio frequency (default: 44100 Hz)
     * @param format Audio format (default: MIX_DEFAULT_FORMAT)
     * @param channels Number of audio channels (default: 2 for stereo)
     * @param chunksize Audio buffer size (default: 2048)
     * @return true if initialization successful, false otherwise
     */
    bool Initialize(int frequency = 44100, Uint16 format = MIX_DEFAULT_FORMAT, 
                   int channels = 2, int chunksize = 2048);

    /**
     * @brief Shutdown the audio system and clean up resources
     */
    void Shutdown();

    /**
     * @brief Load a sound effect from file
     *
     * @param name Unique identifier for the sound
     * @param filepath Path to the audio file
     * @param type Category of the sound for volume control
     * @return true if loading successful, false otherwise
     */
    bool LoadSound(const std::string& name, const std::string& filepath, SoundType type = SoundType::SOUND_EFFECT);

    /**
     * @brief Load background music from file
     *
     * @param name Unique identifier for the music
     * @param filepath Path to the music file
     * @return true if loading successful, false otherwise
     */
    bool LoadMusic(const std::string& name, const std::string& filepath);

    /**
     * @brief Play a loaded sound effect
     *
     * @param name Name of the sound to play
     * @param volume Volume multiplier (0.0 - 1.0, default: 1.0)
     * @param loops Number of times to loop (-1 for infinite, 0 for once, default: 0)
     * @return Channel number if successful, -1 if failed
     */
    int PlaySound(const std::string& name, float volume = 1.0f, int loops = 0);

    /**
     * @brief Play background music
     *
     * @param name Name of the music to play
     * @param volume Volume multiplier (0.0 - 1.0, default: 1.0)
     * @param loops Number of times to loop (-1 for infinite, default: -1)
     * @return true if successful, false otherwise
     */
    bool PlayMusic(const std::string& name, float volume = 1.0f, int loops = -1);

    /**
     * @brief Stop currently playing music
     */
    void StopMusic();

    /**
     * @brief Pause/resume music playback
     *
     * @param paused true to pause, false to resume
     */
    void PauseMusic(bool paused);

    /**
     * @brief Set master volume for a sound category
     *
     * @param type Sound category
     * @param volume Volume level (0.0 - 1.0)
     */
    void SetCategoryVolume(SoundType type, float volume);

    /**
     * @brief Get master volume for a sound category
     *
     * @param type Sound category
     * @return Volume level (0.0 - 1.0)
     */
    float GetCategoryVolume(SoundType type) const;

    /**
     * @brief Set master music volume
     *
     * @param volume Volume level (0.0 - 1.0)
     */
    void SetMusicVolume(float volume);

    /**
     * @brief Get master music volume
     *
     * @return Volume level (0.0 - 1.0)
     */
    float GetMusicVolume() const;

    /**
     * @brief Check if audio system is initialized
     *
     * @return true if initialized, false otherwise
     */
    bool IsInitialized() const { return m_initialized; }

    /**
     * @brief Check if music is currently playing
     *
     * @return true if music is playing, false otherwise
     */
    bool IsMusicPlaying() const;

    /**
     * @brief Unload a sound effect
     *
     * @param name Name of the sound to unload
     */
    void UnloadSound(const std::string& name);

    /**
     * @brief Unload background music
     *
     * @param name Name of the music to unload
     */
    void UnloadMusic(const std::string& name);

    /**
     * @brief Unload all audio resources
     */
    void UnloadAll();

private:
    bool m_initialized = false;                                    ///< Initialization state
    std::unordered_map<std::string, std::unique_ptr<Sound>> m_sounds;  ///< Loaded sound effects
    std::unordered_map<std::string, std::unique_ptr<Music>> m_music;   ///< Loaded music tracks
    std::unordered_map<SoundType, float> m_categoryVolumes;       ///< Volume levels by category
    float m_musicVolume = 1.0f;                                   ///< Master music volume
    std::string m_currentMusic;                                   ///< Currently playing music name

    /**
     * @brief Calculate final volume for a sound
     *
     * @param sound Sound to calculate volume for
     * @param volumeMultiplier Additional volume multiplier
     * @return Final volume (0-128 for SDL_mixer)
     */
    int CalculateFinalVolume(const Sound* sound, float volumeMultiplier) const;
};
