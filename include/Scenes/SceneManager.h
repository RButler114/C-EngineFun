#pragma once

#include "Scene.h"
#include <unordered_map>
#include <memory>
#include <string>

class Engine;

class SceneManager {
public:
    SceneManager(Engine* engine);
    ~SceneManager();

    // Scene management
    void AddScene(const std::string& name, std::unique_ptr<Scene> scene);
    void RemoveScene(const std::string& name);
    void SetCurrentScene(const std::string& name);
    
    Scene* GetCurrentScene() const { return m_currentScene; }
    Scene* GetScene(const std::string& name) const;
    
    bool HasScene(const std::string& name) const;

    // Update current scene
    void Update(float deltaTime);
    void Render();
    void HandleInput();

private:
    Engine* m_engine;
    std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
    Scene* m_currentScene;
    std::string m_currentSceneName;
};
