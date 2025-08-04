/**
 * @file EventSystem.h
 * @brief Event system for decoupled communication between game systems
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "ECS/Entity.h"
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <typeindex>
#include <queue>

/**
 * @class Event
 * @brief Base class for all events in the system
 */
class Event {
public:
    virtual ~Event() = default;
    
    /**
     * @brief Get the type index of this event
     */
    virtual std::type_index GetType() const = 0;
};

/**
 * @template EventType
 * @brief Template for creating specific event types
 */
template<typename T>
class EventType : public Event {
public:
    std::type_index GetType() const override {
        return std::type_index(typeid(T));
    }
};

// ============================================================================
// GAME EVENTS
// ============================================================================

/**
 * @struct EntityDamagedEvent
 * @brief Event fired when an entity takes damage
 */
struct EntityDamagedEvent : public EventType<EntityDamagedEvent> {
    Entity entity;          ///< Entity that was damaged
    Entity attacker;        ///< Entity that dealt the damage (can be invalid)
    float damage;           ///< Amount of damage dealt
    float remainingHealth;  ///< Health remaining after damage
    
    EntityDamagedEvent(Entity e, Entity a, float d, float h)
        : entity(e), attacker(a), damage(d), remainingHealth(h) {}
};

/**
 * @struct EntityDeathEvent
 * @brief Event fired when an entity dies
 */
struct EntityDeathEvent : public EventType<EntityDeathEvent> {
    Entity entity;          ///< Entity that died
    Entity killer;          ///< Entity that killed it (can be invalid)
    
    EntityDeathEvent(Entity e, Entity k = Entity())
        : entity(e), killer(k) {}
};

/**
 * @struct EntityHealedEvent
 * @brief Event fired when an entity is healed
 */
struct EntityHealedEvent : public EventType<EntityHealedEvent> {
    Entity entity;          ///< Entity that was healed
    float healAmount;       ///< Amount of healing
    float newHealth;        ///< Health after healing
    
    EntityHealedEvent(Entity e, float heal, float health)
        : entity(e), healAmount(heal), newHealth(health) {}
};

/**
 * @struct AbilityUsedEvent
 * @brief Event fired when an entity uses an ability
 */
struct AbilityUsedEvent : public EventType<AbilityUsedEvent> {
    Entity caster;          ///< Entity that used the ability
    int abilityIndex;       ///< Index of the ability used
    std::string abilityName; ///< Name of the ability
    Entity target;          ///< Target entity (can be invalid for self-cast or area abilities)
    
    AbilityUsedEvent(Entity c, int index, const std::string& name, Entity t = Entity())
        : caster(c), abilityIndex(index), abilityName(name), target(t) {}
};

/**
 * @struct CollisionEvent
 * @brief Event fired when two entities collide
 */
struct CollisionEvent : public EventType<CollisionEvent> {
    Entity entityA;         ///< First entity in collision
    Entity entityB;         ///< Second entity in collision
    float overlapX;         ///< Overlap amount in X direction
    float overlapY;         ///< Overlap amount in Y direction
    
    CollisionEvent(Entity a, Entity b, float ox, float oy)
        : entityA(a), entityB(b), overlapX(ox), overlapY(oy) {}
};

/**
 * @struct StatusEffectAppliedEvent
 * @brief Event fired when a status effect is applied to an entity
 */
struct StatusEffectAppliedEvent : public EventType<StatusEffectAppliedEvent> {
    Entity entity;          ///< Entity that received the effect
    Entity source;          ///< Entity that applied the effect (can be invalid)
    std::string effectName; ///< Name of the effect
    float duration;         ///< Duration of the effect
    
    StatusEffectAppliedEvent(Entity e, Entity s, const std::string& name, float dur)
        : entity(e), source(s), effectName(name), duration(dur) {}
};

/**
 * @struct LevelUpEvent
 * @brief Event fired when an entity levels up
 */
struct LevelUpEvent : public EventType<LevelUpEvent> {
    Entity entity;          ///< Entity that leveled up
    int newLevel;           ///< New level
    int oldLevel;           ///< Previous level
    
    LevelUpEvent(Entity e, int newLvl, int oldLvl)
        : entity(e), newLevel(newLvl), oldLevel(oldLvl) {}
};

/**
 * @class EventManager
 * @brief Manages event subscription and dispatching
 *
 * Provides a centralized event system for decoupled communication
 * between game systems and entities.
 */
class EventManager {
public:
    using EventHandler = std::function<void(const Event&)>;
    
    EventManager() = default;
    ~EventManager() = default;
    
    /**
     * @brief Subscribe to an event type
     * @tparam T Event type to subscribe to
     * @param handler Function to call when event is fired
     * @return Subscription ID for unsubscribing
     */
    template<typename T>
    int Subscribe(const std::function<void(const T&)>& handler) {
        static_assert(std::is_base_of_v<Event, T>, "T must derive from Event");
        
        int id = m_nextSubscriptionId++;
        std::type_index typeIndex(typeid(T));
        
        // Wrap the typed handler in a generic handler
        EventHandler genericHandler = [handler](const Event& event) {
            const T& typedEvent = static_cast<const T&>(event);
            handler(typedEvent);
        };
        
        m_subscribers[typeIndex].emplace_back(id, genericHandler);
        return id;
    }
    
    /**
     * @brief Unsubscribe from events
     * @param subscriptionId ID returned from Subscribe
     */
    void Unsubscribe(int subscriptionId) {
        for (auto& [typeIndex, subscribers] : m_subscribers) {
            subscribers.erase(
                std::remove_if(subscribers.begin(), subscribers.end(),
                    [subscriptionId](const auto& pair) { return pair.first == subscriptionId; }),
                subscribers.end()
            );
        }
    }
    
    /**
     * @brief Fire an event immediately
     * @tparam T Event type
     * @param event Event to fire
     */
    template<typename T>
    void FireEvent(const T& event) {
        static_assert(std::is_base_of_v<Event, T>, "T must derive from Event");
        
        std::type_index typeIndex(typeid(T));
        auto it = m_subscribers.find(typeIndex);
        if (it != m_subscribers.end()) {
            for (const auto& [id, handler] : it->second) {
                handler(event);
            }
        }
    }
    
    /**
     * @brief Queue an event to be processed later
     * @tparam T Event type
     * @param event Event to queue
     */
    template<typename T>
    void QueueEvent(const T& event) {
        static_assert(std::is_base_of_v<Event, T>, "T must derive from Event");
        m_eventQueue.push(std::make_unique<T>(event));
    }
    
    /**
     * @brief Process all queued events
     */
    void ProcessQueuedEvents() {
        while (!m_eventQueue.empty()) {
            auto event = std::move(m_eventQueue.front());
            m_eventQueue.pop();
            
            std::type_index typeIndex = event->GetType();
            auto it = m_subscribers.find(typeIndex);
            if (it != m_subscribers.end()) {
                for (const auto& [id, handler] : it->second) {
                    handler(*event);
                }
            }
        }
    }
    
    /**
     * @brief Clear all subscribers
     */
    void ClearSubscribers() {
        m_subscribers.clear();
    }
    
    /**
     * @brief Clear all queued events
     */
    void ClearQueue() {
        while (!m_eventQueue.empty()) {
            m_eventQueue.pop();
        }
    }

private:
    using SubscriberList = std::vector<std::pair<int, EventHandler>>;
    std::unordered_map<std::type_index, SubscriberList> m_subscribers;
    std::queue<std::unique_ptr<Event>> m_eventQueue;
    int m_nextSubscriptionId = 1;
};

/**
 * @class EventComponent
 * @brief Component that allows entities to subscribe to events
 *
 * Provides a way for entities to react to events in the game world.
 */
struct EventComponent : public Component {
    std::vector<int> subscriptionIds;  ///< List of event subscription IDs
    
    EventComponent() = default;
    
    /**
     * @brief Add a subscription ID to track
     */
    void AddSubscription(int id) {
        subscriptionIds.push_back(id);
    }
    
    /**
     * @brief Clear all subscriptions (call EventManager::Unsubscribe for each)
     */
    void ClearSubscriptions(EventManager& eventManager) {
        for (int id : subscriptionIds) {
            eventManager.Unsubscribe(id);
        }
        subscriptionIds.clear();
    }
};
