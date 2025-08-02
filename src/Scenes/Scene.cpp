#include "Scenes/Scene.h"
#include "ECS/EntityManager.h"

Scene::Scene(const std::string& name) : m_name(name), m_engine(nullptr) {}

Scene::~Scene() = default;

void Scene::InitializeEntityManager() {
    m_entityManager = std::make_unique<EntityManager>();
}
