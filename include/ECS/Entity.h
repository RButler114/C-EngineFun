/**
 * @file Entity.h
 * @brief Entity-Component-System (ECS) entity definitions and utilities
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include <cstdint>

/// Type alias for entity identifiers
using EntityID = std::uint32_t;
/// Type alias for component type identifiers
using ComponentTypeID = std::uint32_t;

/**
 * @class Entity
 * @brief Lightweight entity handle for the ECS system
 *
 * An Entity represents a game object in the ECS architecture. It's essentially
 * a unique identifier that can have components attached to it. Entities themselves
 * contain no data or behavior - they serve as keys to access components.
 *
 * @example
 * ```cpp
 * Entity player = entityManager.CreateEntity();
 * entityManager.AddComponent<PositionComponent>(player, {100.0f, 200.0f});
 * entityManager.AddComponent<VelocityComponent>(player, {50.0f, 0.0f});
 * ```
 */
class Entity {
public:
    /**
     * @brief Default constructor creates an invalid entity
     *
     * Creates an entity with ID 0, which is considered invalid.
     * Use EntityManager::CreateEntity() to create valid entities.
     */
    Entity() : m_id(0) {}

    /**
     * @brief Construct entity with specific ID
     *
     * @param id The entity identifier
     * @note Typically only used internally by EntityManager
     */
    explicit Entity(EntityID id) : m_id(id) {}

    /**
     * @brief Get the entity's unique identifier
     * @return The entity ID
     */
    EntityID GetID() const { return m_id; }

    /**
     * @brief Check if this entity is valid
     * @return true if entity ID is non-zero, false otherwise
     */
    bool IsValid() const { return m_id != 0; }

    /**
     * @brief Equality comparison operator
     * @param other Entity to compare with
     * @return true if entities have the same ID
     */
    bool operator==(const Entity& other) const { return m_id == other.m_id; }

    /**
     * @brief Inequality comparison operator
     * @param other Entity to compare with
     * @return true if entities have different IDs
     */
    bool operator!=(const Entity& other) const { return m_id != other.m_id; }

    /**
     * @brief Less-than comparison operator for container sorting
     * @param other Entity to compare with
     * @return true if this entity's ID is less than other's ID
     */
    bool operator<(const Entity& other) const { return m_id < other.m_id; }

private:
    EntityID m_id; ///< Unique entity identifier
};

/**
 * @brief Generate unique component type IDs at compile time
 *
 * This template function generates a unique ComponentTypeID for each
 * component type T. The ID is generated once per type and cached
 * using static variables.
 *
 * @tparam T The component type
 * @return Unique ComponentTypeID for type T
 *
 * @example
 * ```cpp
 * ComponentTypeID positionTypeID = GetComponentTypeID<PositionComponent>();
 * ComponentTypeID velocityTypeID = GetComponentTypeID<VelocityComponent>();
 * // positionTypeID != velocityTypeID (guaranteed unique)
 * ```
 */
template<typename T>
ComponentTypeID GetComponentTypeID() {
    static ComponentTypeID typeID = []() {
        static ComponentTypeID counter = 0;
        return ++counter;
    }();
    return typeID;
}
