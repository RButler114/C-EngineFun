#pragma once

#include <cstdint>

using EntityID = std::uint32_t;
using ComponentTypeID = std::uint32_t;

class Entity {
public:
    Entity() : m_id(0) {}
    explicit Entity(EntityID id) : m_id(id) {}
    
    EntityID GetID() const { return m_id; }
    bool IsValid() const { return m_id != 0; }
    
    bool operator==(const Entity& other) const { return m_id == other.m_id; }
    bool operator!=(const Entity& other) const { return m_id != other.m_id; }
    bool operator<(const Entity& other) const { return m_id < other.m_id; }

private:
    EntityID m_id;
};

// Helper to generate unique component type IDs
template<typename T>
ComponentTypeID GetComponentTypeID() {
    static ComponentTypeID typeID = []() {
        static ComponentTypeID counter = 0;
        return ++counter;
    }();
    return typeID;
}
