/**
 * @file test_audio_system.cpp
 * @brief Unit tests for the audio system
 * @author Ryan Butler
 * @date 2025
 */

#include "Engine/AudioManager.h"
#include "ECS/ECS.h"
#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>

// Simple test framework
#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Running " #name "... "; \
    test_##name(); \
    std::cout << "PASSED" << std::endl; \
} while(0)

#define ASSERT_TRUE(condition) do { \
    if (!(condition)) { \
        std::cerr << "ASSERTION FAILED: " #condition << " at line " << __LINE__ << std::endl; \
        exit(1); \
    } \
} while(0)

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))

TEST(audio_manager_initialization) {
    AudioManager audioManager;
    
    // Test initialization
    ASSERT_TRUE(audioManager.Initialize());
    ASSERT_TRUE(audioManager.IsInitialized());
    
    // Test shutdown
    audioManager.Shutdown();
    ASSERT_FALSE(audioManager.IsInitialized());
}

TEST(sound_loading_and_unloading) {
    AudioManager audioManager;
    ASSERT_TRUE(audioManager.Initialize());
    
    // Test loading valid sound file
    bool loadResult = audioManager.LoadSound("test_jump", "bin/assets/sounds/jump.wav", SoundType::SOUND_EFFECT);
    ASSERT_TRUE(loadResult);
    
    // Test loading same sound again (should succeed)
    loadResult = audioManager.LoadSound("test_jump", "bin/assets/sounds/jump.wav", SoundType::SOUND_EFFECT);
    ASSERT_TRUE(loadResult);
    
    // Test loading invalid sound file
    loadResult = audioManager.LoadSound("invalid", "nonexistent.wav", SoundType::SOUND_EFFECT);
    ASSERT_FALSE(loadResult);
    
    // Test unloading
    audioManager.UnloadSound("test_jump");
    audioManager.UnloadSound("invalid"); // Should not crash
    
    audioManager.Shutdown();
}

TEST(music_loading_and_playback) {
    AudioManager audioManager;
    ASSERT_TRUE(audioManager.Initialize());
    
    // Test loading music
    bool loadResult = audioManager.LoadMusic("test_music", "bin/assets/music/background.wav");
    ASSERT_TRUE(loadResult);
    
    // Test music playback
    ASSERT_FALSE(audioManager.IsMusicPlaying());
    bool playResult = audioManager.PlayMusic("test_music", 0.5f, 0); // Play once
    ASSERT_TRUE(playResult);
    
    // Give it a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_TRUE(audioManager.IsMusicPlaying());
    
    // Test pause/resume
    audioManager.PauseMusic(true);
    audioManager.PauseMusic(false);
    
    // Test stop
    audioManager.StopMusic();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_FALSE(audioManager.IsMusicPlaying());
    
    audioManager.Shutdown();
}

TEST(volume_control) {
    AudioManager audioManager;
    ASSERT_TRUE(audioManager.Initialize());
    
    // Test category volume
    audioManager.SetCategoryVolume(SoundType::SOUND_EFFECT, 0.5f);
    ASSERT_TRUE(audioManager.GetCategoryVolume(SoundType::SOUND_EFFECT) == 0.5f);
    
    audioManager.SetCategoryVolume(SoundType::MUSIC, 0.8f);
    ASSERT_TRUE(audioManager.GetCategoryVolume(SoundType::MUSIC) == 0.8f);
    
    // Test music volume
    audioManager.SetMusicVolume(0.3f);
    ASSERT_TRUE(audioManager.GetMusicVolume() == 0.3f);
    
    // Test volume clamping
    audioManager.SetCategoryVolume(SoundType::SOUND_EFFECT, 2.0f); // Should clamp to 1.0
    ASSERT_TRUE(audioManager.GetCategoryVolume(SoundType::SOUND_EFFECT) == 1.0f);
    
    audioManager.SetMusicVolume(-0.5f); // Should clamp to 0.0
    ASSERT_TRUE(audioManager.GetMusicVolume() == 0.0f);
    
    audioManager.Shutdown();
}

TEST(sound_playback) {
    AudioManager audioManager;
    ASSERT_TRUE(audioManager.Initialize());
    
    // Load a sound
    ASSERT_TRUE(audioManager.LoadSound("test_sound", "bin/assets/sounds/collision.wav", SoundType::SOUND_EFFECT));
    
    // Test playing sound
    int channel = audioManager.PlaySound("test_sound", 1.0f, 0);
    ASSERT_TRUE(channel >= 0); // Valid channel number
    
    // Test playing non-existent sound
    channel = audioManager.PlaySound("nonexistent", 1.0f, 0);
    ASSERT_TRUE(channel == -1); // Should fail
    
    audioManager.Shutdown();
}

TEST(audio_component_creation) {
    // Test AudioComponent creation and properties
    AudioComponent audioComp;
    ASSERT_TRUE(audioComp.soundName.empty());
    ASSERT_TRUE(audioComp.volume == 1.0f);
    ASSERT_FALSE(audioComp.looping);
    ASSERT_FALSE(audioComp.playOnCreate);
    ASSERT_FALSE(audioComp.playOnCollision);
    ASSERT_FALSE(audioComp.is3D);
    
    // Test constructor with parameters
    AudioComponent audioComp2("test_sound", 0.8f, true, true, false);
    ASSERT_TRUE(audioComp2.soundName == "test_sound");
    ASSERT_TRUE(audioComp2.volume == 0.8f);
    ASSERT_TRUE(audioComp2.looping);
    ASSERT_TRUE(audioComp2.playOnCreate);
    ASSERT_FALSE(audioComp2.playOnCollision);
}

TEST(ecs_audio_integration) {
    AudioManager audioManager;
    ASSERT_TRUE(audioManager.Initialize());
    
    // Load test sound
    ASSERT_TRUE(audioManager.LoadSound("ecs_test", "bin/assets/sounds/menu_select.wav", SoundType::SOUND_EFFECT));
    
    // Create ECS setup
    EntityManager entityManager;
    entityManager.AddSystem<AudioSystem>(audioManager);
    
    // Create entity with audio component
    Entity entity = entityManager.CreateEntity();
    entityManager.AddComponent<TransformComponent>(entity, 100.0f, 200.0f);
    auto* audioComp = entityManager.AddComponent<AudioComponent>(entity, "ecs_test", 0.9f);
    
    // Verify component was added
    ASSERT_TRUE(audioComp != nullptr);
    ASSERT_TRUE(audioComp->soundName == "ecs_test");
    ASSERT_TRUE(audioComp->volume == 0.9f);
    
    // Test audio system processing
    entityManager.Update(0.016f); // One frame
    
    audioManager.Shutdown();
}

TEST(audio_system_3d_calculations) {
    AudioManager audioManager;
    ASSERT_TRUE(audioManager.Initialize());
    
    EntityManager entityManager;
    auto* audioSystem = entityManager.AddSystem<AudioSystem>(audioManager);
    
    // Test listener position
    audioSystem->SetListenerPosition(100.0f, 200.0f);
    
    float x, y;
    audioSystem->GetListenerPosition(x, y);
    ASSERT_TRUE(x == 100.0f);
    ASSERT_TRUE(y == 200.0f);
    
    audioManager.Shutdown();
}

int main() {
    std::cout << "🧪 Running Audio System Tests" << std::endl;
    std::cout << "==============================" << std::endl;
    
    try {
        RUN_TEST(audio_manager_initialization);
        RUN_TEST(sound_loading_and_unloading);
        RUN_TEST(music_loading_and_playback);
        RUN_TEST(volume_control);
        RUN_TEST(sound_playback);
        RUN_TEST(audio_component_creation);
        RUN_TEST(ecs_audio_integration);
        RUN_TEST(audio_system_3d_calculations);
        
        std::cout << "\n✅ All audio tests passed!" << std::endl;
        std::cout << "The audio system is working correctly." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
