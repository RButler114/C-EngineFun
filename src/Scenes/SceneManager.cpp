#include "Scenes/SceneManager.h"
#include "Engine/Engine.h"
#include <iostream>

SceneManager::SceneManager(Engine* engine) 
    : m_engine(engine), m_currentScene(nullptr) {
}

SceneManager::~SceneManager() {
    if (m_currentScene) {
        m_currentScene->OnExit();
    }
}

void SceneManager::AddScene(const std::string& name, std::unique_ptr<Scene> scene) {
    if (scene) {
        scene->SetEngine(m_engine);
        m_scenes[name] = std::move(scene);
        std::cout << "Added scene: " << name << std::endl;
    }
}

void SceneManager::RemoveScene(const std::string& name) {
    auto it = m_scenes.find(name);
    if (it != m_scenes.end()) {
        // If this is the current scene, exit it first
        if (m_currentScene == it->second.get()) {
            m_currentScene->OnExit();
            m_currentScene = nullptr;
            m_currentSceneName.clear();
        }
        
        m_scenes.erase(it);
        std::cout << "Removed scene: " << name << std::endl;
    }
}

void SceneManager::SetCurrentScene(const std::string& name) {
    auto it = m_scenes.find(name);
    if (it != m_scenes.end()) {
        // Exit current scene
        if (m_currentScene) {
            m_currentScene->OnExit();
        }
        
        // Set new scene
        m_currentScene = it->second.get();
        m_currentSceneName = name;
        
        // Enter new scene
        if (m_currentScene) {
            m_currentScene->OnEnter();
        }
        
        std::cout << "Switched to scene: " << name << std::endl;
    } else {
        std::cerr << "Scene not found: " << name << std::endl;
    }
}

Scene* SceneManager::GetScene(const std::string& name) const {
    auto it = m_scenes.find(name);
    return (it != m_scenes.end()) ? it->second.get() : nullptr;
}

bool SceneManager::HasScene(const std::string& name) const {
    return m_scenes.find(name) != m_scenes.end();
}

void SceneManager::Update(float deltaTime) {
    if (m_currentScene) {
        m_currentScene->Update(deltaTime);
    }
}

void SceneManager::Render() {
    if (m_currentScene) {
        m_currentScene->Render();
    }
}

void SceneManager::HandleInput() {
    if (m_currentScene) {
        m_currentScene->HandleInput();
    }
}
