# Game Engine Tutorial

This tutorial will walk you through creating a simple game using the engine's features.

## Step 1: Basic Engine Setup

Create a basic game class that inherits from Engine:

```cpp
#include "Engine/Engine.h"

class MyGame : public Engine {
protected:
    void Update(float deltaTime) override {
        // Your game logic here
    }
    
    void Render() override {
        // Your rendering here
    }
};
```

## Step 2: Adding Basic Rendering

Use the renderer to draw shapes:

```cpp
void Render() override {
    auto* renderer = GetRenderer();
    
    // Draw a rectangle
    Rectangle rect(100, 100, 50, 50);
    Color red(255, 0, 0, 255);
    renderer->DrawRectangle(rect, red);
    
    // Draw a line
    renderer->DrawLine(0, 0, 800, 600, Color(0, 255, 0));
}
```

## Step 3: Handling Input

Access input in your Update method:

```cpp
void Update(float deltaTime) override {
    auto* input = GetInputManager();
    
    if (input->IsKeyPressed(SDL_SCANCODE_SPACE)) {
        // Space key is held down
    }
    
    if (input->IsKeyJustPressed(SDL_SCANCODE_ENTER)) {
        // Enter key was just pressed this frame
    }
    
    int mouseX, mouseY;
    input->GetMousePosition(mouseX, mouseY);
}
```

## Step 4: Using the ECS System

Create entities with components:

```cpp
#include "ECS/ECS.h"

class ECSGame : public Engine {
private:
    std::unique_ptr<EntityManager> m_entityManager;
    
public:
    bool Initialize(const char* title, int width, int height) {
        if (!Engine::Initialize(title, width, height)) {
            return false;
        }
        
        // Create entity manager
        m_entityManager = std::make_unique<EntityManager>();
        
        // Add systems
        m_entityManager->AddSystem<MovementSystem>();
        m_entityManager->AddSystem<RenderSystem>(GetRenderer());
        
        // Create a player entity
        Entity player = m_entityManager->CreateEntity();
        m_entityManager->AddComponent<TransformComponent>(player, 100.0f, 100.0f);
        m_entityManager->AddComponent<VelocityComponent>(player, 50.0f, 0.0f);
        m_entityManager->AddComponent<RenderComponent>(player, 32, 32, 0, 255, 0);
        
        return true;
    }
    
protected:
    void Update(float deltaTime) override {
        m_entityManager->Update(deltaTime);
    }
};
```

## Step 5: Adding Collision Detection

```cpp
// Add collision system
auto* collisionSystem = m_entityManager->AddSystem<CollisionSystem>();

// Set collision callback
collisionSystem->SetCollisionCallback([](const CollisionInfo& info) {
    std::cout << "Collision between entities " 
              << info.entityA.GetID() << " and " 
              << info.entityB.GetID() << std::endl;
});

// Add collision component to entities
m_entityManager->AddComponent<CollisionComponent>(player, 32.0f, 32.0f);
```

## Step 6: Scene Management

Create different scenes for your game:

```cpp
#include "Scenes/Scene.h"
#include "Scenes/SceneManager.h"

class MenuScene : public Scene {
public:
    MenuScene() : Scene("Menu") {
        InitializeEntityManager();
    }
    
    void Update(float deltaTime) override {
        // Menu logic
    }
    
    void Render() override {
        // Menu rendering
    }
};

class GameScene : public Scene {
public:
    GameScene() : Scene("Game") {
        InitializeEntityManager();
    }
    
    void OnEnter() override {
        // Setup game entities
    }
    
    void Update(float deltaTime) override {
        GetEntityManager()->Update(deltaTime);
    }
};

// In your main game class
class SceneBasedGame : public Engine {
private:
    std::unique_ptr<SceneManager> m_sceneManager;
    
public:
    bool Initialize(const char* title, int width, int height) {
        if (!Engine::Initialize(title, width, height)) {
            return false;
        }
        
        m_sceneManager = std::make_unique<SceneManager>(this);
        m_sceneManager->AddScene("menu", std::make_unique<MenuScene>());
        m_sceneManager->AddScene("game", std::make_unique<GameScene>());
        m_sceneManager->SetCurrentScene("menu");
        
        return true;
    }
    
protected:
    void Update(float deltaTime) override {
        m_sceneManager->Update(deltaTime);
    }
    
    void Render() override {
        m_sceneManager->Render();
    }
};
```

## Tips for Learning

1. **Start Simple**: Begin with basic rendering and input, then add ECS features
2. **Experiment**: Try modifying the example game to understand how components work
3. **Debug**: Use std::cout to print entity IDs and component values
4. **Iterate**: Build features incrementally and test frequently

## Common Patterns

### Moving Objects
```cpp
// In Update method
auto entities = entityManager->GetEntitiesWith<TransformComponent, VelocityComponent>();
for (Entity entity : entities) {
    auto* transform = entityManager->GetComponent<TransformComponent>(entity);
    auto* velocity = entityManager->GetComponent<VelocityComponent>(entity);
    
    transform->x += velocity->vx * deltaTime;
    transform->y += velocity->vy * deltaTime;
}
```

### Boundary Checking
```cpp
// Keep entities within screen bounds
if (transform->x < 0) {
    transform->x = 0;
    velocity->vx = -velocity->vx; // Bounce
}
if (transform->x > screenWidth - entityWidth) {
    transform->x = screenWidth - entityWidth;
    velocity->vx = -velocity->vx;
}
```

This tutorial covers the basics. Check the examples/ directory for complete implementations!
