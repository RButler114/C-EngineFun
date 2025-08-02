#pragma once

#include "Entity.h"

// Base component struct (no virtual functions to avoid memory layout issues)
struct Component {
    Entity owner;

    Component() = default;
    Component(Entity e) : owner(e) {}
};

// Common components
struct TransformComponent : public Component {
    float x = 0.0f;
    float y = 0.0f;
    float rotation = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;

    TransformComponent() = default;
    TransformComponent(float posX, float posY) : Component(), x(posX), y(posY) {}
    TransformComponent(float posX, float posY, float rot) : Component(), x(posX), y(posY), rotation(rot) {}
};

struct VelocityComponent : public Component {
    float vx = 0.0f;
    float vy = 0.0f;

    VelocityComponent() = default;
    VelocityComponent(float velX, float velY) : Component(), vx(velX), vy(velY) {}
};

struct RenderComponent : public Component {
    int width = 32;
    int height = 32;
    unsigned char r = 255;
    unsigned char g = 255;
    unsigned char b = 255;
    unsigned char a = 255;
    bool visible = true;

    RenderComponent() = default;
    RenderComponent(int w, int h) : Component(), width(w), height(h) {}
    RenderComponent(int w, int h, unsigned char red, unsigned char green, unsigned char blue)
        : Component(), width(w), height(h), r(red), g(green), b(blue) {}
};

struct CollisionComponent : public Component {
    float width = 32.0f;
    float height = 32.0f;
    bool isTrigger = false;

    CollisionComponent() = default;
    CollisionComponent(float w, float h) : Component(), width(w), height(h) {}
    CollisionComponent(float w, float h, bool trigger) : Component(), width(w), height(h), isTrigger(trigger) {}
};
