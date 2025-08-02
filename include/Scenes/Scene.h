#pragma once

#include <string>
#include <memory>

class Engine;
class EntityManager;

class Scene {
public:
    Scene(const std::string& name);
    virtual ~Scene();

    // Scene lifecycle
    virtual void OnEnter() {}
    virtual void OnExit() {}
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
    virtual void HandleInput() {}

    // Getters
    const std::string& GetName() const { return m_name; }
    Engine* GetEngine() const { return m_engine; }
    EntityManager* GetEntityManager() const { return m_entityManager.get(); }

    // Called by SceneManager
    void SetEngine(Engine* engine) { m_engine = engine; }

protected:
    std::string m_name;
    Engine* m_engine;
    std::unique_ptr<EntityManager> m_entityManager;

    // Helper to initialize entity manager
    void InitializeEntityManager();
};
