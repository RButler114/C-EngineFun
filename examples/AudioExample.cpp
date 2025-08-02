/**
 * @file AudioExample.cpp
 * @brief Audio system demonstration and testing
 * @author Ryan Butler
 * @date 2025
 *
 * This example demonstrates the audio capabilities of the game engine:
 * - Loading and playing sound effects
 * - Background music playback
 * - Volume control by category
 * - 3D positional audio
 * - ECS AudioComponent integration
 */

#include "Engine/Engine.h"
#include "Engine/AudioManager.h"
#include "ECS/ECS.h"
#include <iostream>
#include <thread>
#include <chrono>

class AudioDemo : public Engine {
public:
    AudioDemo() : m_demoStep(0), m_stepTimer(0.0f) {}

protected:
    void Update(float deltaTime) override {
        m_stepTimer += deltaTime;

        // Run through different audio demonstrations
        switch (m_demoStep) {
            case 0:
                if (m_stepTimer >= 1.0f) {
                    DemoBasicSounds();
                    m_demoStep++;
                    m_stepTimer = 0.0f;
                }
                break;
            case 1:
                if (m_stepTimer >= 3.0f) {
                    DemoVolumeControl();
                    m_demoStep++;
                    m_stepTimer = 0.0f;
                }
                break;
            case 2:
                if (m_stepTimer >= 3.0f) {
                    DemoBackgroundMusic();
                    m_demoStep++;
                    m_stepTimer = 0.0f;
                }
                break;
            case 3:
                if (m_stepTimer >= 5.0f) {
                    DemoECSAudio();
                    m_demoStep++;
                    m_stepTimer = 0.0f;
                }
                break;
            case 4:
                if (m_stepTimer >= 3.0f) {
                    Demo3DAudio();
                    m_demoStep++;
                    m_stepTimer = 0.0f;
                }
                break;
            case 5:
                if (m_stepTimer >= 5.0f) {
                    std::cout << "\n🎵 Audio demo completed! Press ESC to exit." << std::endl;
                    m_demoStep++;
                }
                break;
        }

        // Handle input
        if (GetInputManager()->IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
            Quit();
        }
    }

    void Render() override {
        // Simple visual feedback
        GetRenderer()->SetDrawColor(50, 50, 100, 255);
        GetRenderer()->Clear();
        
        // Draw a simple visualization based on demo step
        int barHeight = 50 + m_demoStep * 30;
        GetRenderer()->SetDrawColor(100, 200, 100, 255);
        GetRenderer()->FillRect(350, 300 - barHeight/2, 100, barHeight);
        
        GetRenderer()->Present();
    }

private:
    int m_demoStep;
    float m_stepTimer;
    std::unique_ptr<EntityManager> m_entityManager;

    void DemoBasicSounds() {
        std::cout << "\n🔊 Demo 1: Basic Sound Effects" << std::endl;
        
        auto* audio = GetAudioManager();
        if (!audio) {
            std::cout << "❌ AudioManager not available!" << std::endl;
            return;
        }

        // Load and play basic sounds
        std::cout << "Loading sounds..." << std::endl;
        audio->LoadSound("jump", "assets/sounds/jump.wav", SoundType::SOUND_EFFECT);
        audio->LoadSound("collision", "assets/sounds/collision.wav", SoundType::SOUND_EFFECT);
        audio->LoadSound("menu_select", "assets/sounds/menu_select.wav", SoundType::SOUND_EFFECT);

        std::cout << "Playing jump sound..." << std::endl;
        audio->PlaySound("jump");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        std::cout << "Playing collision sound..." << std::endl;
        audio->PlaySound("collision");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        
        std::cout << "Playing menu select sound..." << std::endl;
        audio->PlaySound("menu_select");
    }

    void DemoVolumeControl() {
        std::cout << "\n🔊 Demo 2: Volume Control" << std::endl;
        
        auto* audio = GetAudioManager();
        if (!audio) return;

        std::cout << "Playing sound at full volume..." << std::endl;
        audio->PlaySound("jump", 1.0f);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        std::cout << "Playing sound at 50% volume..." << std::endl;
        audio->PlaySound("jump", 0.5f);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        std::cout << "Setting category volume to 30%..." << std::endl;
        audio->SetCategoryVolume(SoundType::SOUND_EFFECT, 0.3f);
        audio->PlaySound("jump", 1.0f); // Will be played at 30% due to category volume
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Reset category volume
        audio->SetCategoryVolume(SoundType::SOUND_EFFECT, 1.0f);
    }

    void DemoBackgroundMusic() {
        std::cout << "\n🎵 Demo 3: Background Music" << std::endl;
        
        auto* audio = GetAudioManager();
        if (!audio) return;

        std::cout << "Loading and playing background music..." << std::endl;
        audio->LoadMusic("background", "assets/music/background.wav");
        audio->PlayMusic("background", 0.4f, -1); // 40% volume, loop infinitely
        
        std::cout << "Music is playing... (will continue in background)" << std::endl;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        
        std::cout << "Pausing music..." << std::endl;
        audio->PauseMusic(true);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        
        std::cout << "Resuming music..." << std::endl;
        audio->PauseMusic(false);
    }

    void DemoECSAudio() {
        std::cout << "\n🎮 Demo 4: ECS Audio Integration" << std::endl;
        
        auto* audio = GetAudioManager();
        if (!audio) return;

        // Create entity manager and audio system
        m_entityManager = std::make_unique<EntityManager>();
        m_entityManager->AddSystem<AudioSystem>(*audio);

        // Create an entity with audio component
        Entity entity = m_entityManager->CreateEntity();
        m_entityManager->AddComponent<TransformComponent>(entity, 400.0f, 300.0f);
        m_entityManager->AddComponent<AudioComponent>(entity, "collision", 0.8f, false, true, false);

        std::cout << "Created entity with AudioComponent (playOnCreate=true)" << std::endl;
        std::cout << "Entity should play collision sound automatically..." << std::endl;
        
        // Update the ECS to trigger the audio
        m_entityManager->Update(0.016f);
    }

    void Demo3DAudio() {
        std::cout << "\n🌍 Demo 5: 3D Positional Audio" << std::endl;
        
        auto* audio = GetAudioManager();
        if (!audio || !m_entityManager) return;

        // Get the audio system
        auto* audioSystem = m_entityManager->GetSystem<AudioSystem>();
        if (!audioSystem) {
            std::cout << "❌ AudioSystem not found!" << std::endl;
            return;
        }

        // Set listener position (center of screen)
        audioSystem->SetListenerPosition(400.0f, 300.0f);

        // Create entity with 3D audio
        Entity entity3D = m_entityManager->CreateEntity();
        m_entityManager->AddComponent<TransformComponent>(entity3D, 100.0f, 300.0f); // Far left
        auto* audioComp = m_entityManager->AddComponent<AudioComponent>(entity3D, "jump", 1.0f);
        audioComp->is3D = true;
        audioComp->maxDistance = 500.0f;

        std::cout << "Playing 3D audio from left side (distance affects volume)..." << std::endl;
        audioSystem->PlayEntitySound(entity3D);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        
        // Move entity closer
        auto* transform = m_entityManager->GetComponent<TransformComponent>(entity3D);
        if (transform) {
            transform->x = 350.0f; // Closer to listener
            std::cout << "Moving sound source closer..." << std::endl;
            audioSystem->PlayEntitySound(entity3D);
        }
    }
};

int main() {
    std::cout << "🎵 Audio System Demo" << std::endl;
    std::cout << "This demo will showcase various audio features:" << std::endl;
    std::cout << "1. Basic sound effects" << std::endl;
    std::cout << "2. Volume control" << std::endl;
    std::cout << "3. Background music" << std::endl;
    std::cout << "4. ECS audio integration" << std::endl;
    std::cout << "5. 3D positional audio" << std::endl;
    std::cout << "\nPress ESC at any time to exit.\n" << std::endl;

    AudioDemo demo;
    
    if (!demo.Initialize("Audio System Demo", 800, 600)) {
        std::cerr << "Failed to initialize audio demo!" << std::endl;
        return -1;
    }

    demo.Run();
    return 0;
}
