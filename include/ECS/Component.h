/**
 * @file Component.h
 * @brief Component definitions for the Entity-Component-System architecture
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "Entity.h"
#include <string>
#include <cstring>
#include <iostream>
#include <vector>

/**
 * @struct Component
 * @brief Base component structure for all ECS components
 *
 * All components in the ECS system inherit from this base structure.
 * Components are pure data containers with no behavior - they only
 * store the state that entities possess.
 *
 * @note No virtual functions are used to avoid memory layout issues
 *       and maintain cache-friendly data structures.
 */
struct Component {
    Entity owner; ///< The entity that owns this component

    /**
     * @brief Default constructor
     */
    Component() = default;

    /**
     * @brief Constructor with entity owner
     * @param e The entity that will own this component
     */
    Component(Entity e) : owner(e) {}
};

/**
 * @defgroup Components Common ECS Components
 * @brief Standard components used throughout the game engine
 * @{
 */

/**
 * @struct TransformComponent
 * @brief Component that defines an entity's position, rotation, and scale
 *
 * The TransformComponent is one of the most fundamental components,
 * defining where an entity exists in 2D space and how it's oriented.
 *
 * @example
 * ```cpp
 * Entity player = entityManager.CreateEntity();
 * entityManager.AddComponent<TransformComponent>(player, 100.0f, 200.0f);
 * ```
 */
struct TransformComponent : public Component {
    float x = 0.0f;         ///< X position in world coordinates
    float y = 0.0f;         ///< Y position in world coordinates
    float rotation = 0.0f;  ///< Rotation in radians
    float scaleX = 1.0f;    ///< X scale factor (1.0 = normal size)
    float scaleY = 1.0f;    ///< Y scale factor (1.0 = normal size)

    /**
     * @brief Default constructor - creates transform at origin
     */
    TransformComponent() = default;

    /**
     * @brief Constructor with position
     * @param posX X coordinate
     * @param posY Y coordinate
     */
    TransformComponent(float posX, float posY) : Component(), x(posX), y(posY) {}

    /**
     * @brief Constructor with position and rotation
     * @param posX X coordinate
     * @param posY Y coordinate
     * @param rot Rotation in radians
     */
    TransformComponent(float posX, float posY, float rot) : Component(), x(posX), y(posY), rotation(rot) {}
};

/**
 * @struct VelocityComponent
 * @brief Component that defines an entity's movement speed and direction
 *
 * Used by MovementSystem to update entity positions over time.
 * Velocity is measured in units per second.
 */
struct VelocityComponent : public Component {
    float vx = 0.0f; ///< X velocity in units per second
    float vy = 0.0f; ///< Y velocity in units per second

    /**
     * @brief Default constructor - creates stationary velocity
     */
    VelocityComponent() = default;

    /**
     * @brief Constructor with velocity values
     * @param velX X velocity
     * @param velY Y velocity
     */
    VelocityComponent(float velX, float velY) : Component(), vx(velX), vy(velY) {}
};

/**
 * @struct RenderComponent
 * @brief Component that defines how an entity should be rendered
 *
 * Contains visual properties like size, color, and visibility.
 * Used by RenderSystem to draw entities on screen.
 */
struct RenderComponent : public Component {
    int width = 32;             ///< Width in pixels
    int height = 32;            ///< Height in pixels
    unsigned char r = 255;      ///< Red color component (0-255)
    unsigned char g = 255;      ///< Green color component (0-255)
    unsigned char b = 255;      ///< Blue color component (0-255)
    unsigned char a = 255;      ///< Alpha transparency (0-255)
    bool visible = true;        ///< Whether the entity should be rendered

    /**
     * @brief Default constructor - creates white 32x32 rectangle
     */
    RenderComponent() = default;

    /**
     * @brief Constructor with size
     * @param w Width in pixels
     * @param h Height in pixels
     */
    RenderComponent(int w, int h) : Component(), width(w), height(h) {}

    /**
     * @brief Constructor with size and color
     * @param w Width in pixels
     * @param h Height in pixels
     * @param red Red component (0-255)
     * @param green Green component (0-255)
     * @param blue Blue component (0-255)
     */
    RenderComponent(int w, int h, unsigned char red, unsigned char green, unsigned char blue)
        : Component(), width(w), height(h), r(red), g(green), b(blue) {}
};

/**
 * @struct SpriteComponent
 * @brief Component that defines sprite-based rendering for entities
 *
 * Extends rendering capabilities to support texture-based sprites with
 * animation frames, sprite sheets, and texture paths. Can be used alongside
 * or instead of RenderComponent for more advanced visual representation.
 *
 * Features:
 * - Texture-based rendering from image files
 * - Sprite sheet support with frame-based animation
 * - Configurable frame dimensions and animation timing
 * - Source rectangle specification for sprite sheets
 * - Scaling and visibility controls
 *
 * @example
 * ```cpp
 * Entity player = entityManager.CreateEntity();
 * entityManager.AddComponent<TransformComponent>(player, 100.0f, 200.0f);
 * entityManager.AddComponent<SpriteComponent>(player, "assets/sprites/player/hero.png", 32, 32);
 * ```
 */
struct SpriteComponent : public Component {
    std::string texturePath;        ///< Path to the texture file
    int width = 32;                 ///< Width of sprite frame in pixels
    int height = 32;                ///< Height of sprite frame in pixels
    int frameX = 0;                 ///< X offset in sprite sheet (pixels)
    int frameY = 0;                 ///< Y offset in sprite sheet (pixels)
    int frameWidth = 32;            ///< Width of source frame in sprite sheet
    int frameHeight = 32;           ///< Height of source frame in sprite sheet
    float scaleX = 1.0f;            ///< X scale factor (1.0 = normal size)
    float scaleY = 1.0f;            ///< Y scale factor (1.0 = normal size)
    bool visible = true;            ///< Whether the sprite should be rendered
    bool flipHorizontal = false;    ///< Flip sprite horizontally
    bool flipVertical = false;      ///< Flip sprite vertically

    /**
     * @brief Default constructor - creates empty sprite
     */
    SpriteComponent() : texturePath("") {
        // texturePath is now initialized as empty string
    }

    /**
     * @brief Constructor with texture path and size
     * @param path Path to texture file
     * @param w Width of sprite
     * @param h Height of sprite
     */
    SpriteComponent(const std::string& path, int w, int h)
        : Component(), texturePath(path), width(w), height(h), frameWidth(w), frameHeight(h) {
        // texturePath is now initialized in the initializer list
    }

    /**
     * @brief Constructor with texture path, size, and frame position
     * @param path Path to texture file
     * @param w Width of sprite
     * @param h Height of sprite
     * @param fx Frame X offset in sprite sheet
     * @param fy Frame Y offset in sprite sheet
     * @param fw Frame width in sprite sheet
     * @param fh Frame height in sprite sheet
     */
    SpriteComponent(const std::string& path, int w, int h, int fx, int fy, int fw, int fh)
        : Component(), texturePath(path), width(w), height(h), frameX(fx), frameY(fy), frameWidth(fw), frameHeight(fh) {
        // texturePath is now initialized in the initializer list
    }
};

/**
 * @struct CollisionComponent
 * @brief Component that defines an entity's collision boundaries
 *
 * Used by CollisionSystem to detect when entities overlap.
 * Can be configured as a solid collider or a trigger.
 */
struct CollisionComponent : public Component {
    float width = 32.0f;    ///< Collision box width
    float height = 32.0f;   ///< Collision box height
    bool isTrigger = false; ///< If true, collision is detected but no physics response

    /**
     * @brief Default constructor - creates 32x32 solid collider
     */
    CollisionComponent() = default;

    /**
     * @brief Constructor with size
     * @param w Collision width
     * @param h Collision height
     */
    CollisionComponent(float w, float h) : Component(), width(w), height(h) {}

    /**
     * @brief Constructor with size and trigger flag
     * @param w Collision width
     * @param h Collision height
     * @param trigger Whether this is a trigger collider
     */
    CollisionComponent(float w, float h, bool trigger) : Component(), width(w), height(h), isTrigger(trigger) {}
};

/**
 * @struct AudioComponent
 * @brief Component that defines audio properties for an entity
 *
 * Used by AudioSystem to play sounds based on entity events or states.
 * Supports multiple sound effects per entity with different triggers.
 */
struct AudioComponent : public Component {
    std::string soundName;          ///< Name of the sound to play (must be loaded in AudioManager)
    float volume = 1.0f;            ///< Volume multiplier (0.0 - 1.0)
    float pitch = 1.0f;             ///< Pitch multiplier (not implemented in basic version)
    bool looping = false;           ///< Whether the sound should loop
    bool playOnCreate = false;      ///< Play sound when component is added
    bool playOnCollision = false;   ///< Play sound when entity collides
    bool playOnDestroy = false;     ///< Play sound when entity is destroyed
    bool is3D = false;              ///< Whether to use 3D positional audio
    float maxDistance = 1000.0f;    ///< Maximum distance for 3D audio
    int currentChannel = -1;        ///< Currently playing channel (-1 if not playing)

    /**
     * @brief Default constructor
     */
    AudioComponent() = default;

    /**
     * @brief Constructor with sound name
     * @param sound Name of the sound to play
     */
    AudioComponent(const std::string& sound) : Component(), soundName(sound) {}

    /**
     * @brief Constructor with sound name and volume
     * @param sound Name of the sound to play
     * @param vol Volume multiplier
     */
    AudioComponent(const std::string& sound, float vol) : Component(), soundName(sound), volume(vol) {}

    /**
     * @brief Constructor with full parameters
     * @param sound Name of the sound to play
     * @param vol Volume multiplier
     * @param loop Whether to loop the sound
     * @param onCreate Play on component creation
     * @param onCollision Play on collision
     */
    AudioComponent(const std::string& sound, float vol, bool loop, bool onCreate = false, bool onCollision = false)
        : Component(), soundName(sound), volume(vol), looping(loop), playOnCreate(onCreate), playOnCollision(onCollision) {}
};

/**
 * @struct HealthComponent
 * @brief Component that defines an entity's health and defensive properties
 */
struct HealthComponent : public Component {
    float maxHealth = 100.0f;      ///< Maximum health points
    float currentHealth = 100.0f;  ///< Current health points
    float armor = 0.0f;             ///< Damage reduction (0.0 - 1.0)
    float healthRegen = 0.0f;       ///< Health regeneration per second
    bool isDead = false;            ///< Whether the entity is dead

    HealthComponent() = default;
    HealthComponent(float maxHp, float armorVal = 0.0f, float regen = 0.0f)
        : Component(), maxHealth(maxHp), currentHealth(maxHp), armor(armorVal), healthRegen(regen) {}
};

/**
 * @struct CharacterTypeComponent
 * @brief Component that defines what type of character an entity is
 */
struct CharacterTypeComponent : public Component {
    enum class CharacterType {
        PLAYER,
        ENEMY,
        NPC,
        BOSS,
        NEUTRAL
    };

    enum class CharacterClass {
        WARRIOR,
        ARCHER,
        MAGE,
        ROGUE,
        TANK,
        SUPPORT,
        BEAST,
        MONSTER
    };

    CharacterType type = CharacterType::NEUTRAL;
    CharacterClass characterClass = CharacterClass::MONSTER;
    std::string name = "Unknown";

    CharacterTypeComponent() = default;
    CharacterTypeComponent(CharacterType t, CharacterClass c, const std::string& n = "Unknown")
        : Component(), type(t), characterClass(c), name(n) {}
};

/**
 * @struct CharacterStatsComponent
 * @brief Component that defines character attributes and stats
 */
struct CharacterStatsComponent : public Component {
    float strength = 10.0f;      ///< Physical damage and carrying capacity
    float agility = 10.0f;       ///< Speed and dodge chance
    float intelligence = 10.0f;  ///< Mana and spell effectiveness
    float vitality = 10.0f;      ///< Health and stamina
    float mana = 100.0f;         ///< Current mana points
    float maxMana = 100.0f;      ///< Maximum mana points
    float stamina = 100.0f;      ///< Current stamina points
    float maxStamina = 100.0f;   ///< Maximum stamina points

    CharacterStatsComponent() = default;

    /**
     * @brief Recalculate derived stats based on base attributes
     */
    void RecalculateStats() {
        // Recalculate max health based on vitality
        maxMana = 50.0f + intelligence * 5.0f;
        maxStamina = 50.0f + vitality * 5.0f;

        // Ensure current values don't exceed maximums
        if (mana > maxMana) mana = maxMana;
        if (stamina > maxStamina) stamina = maxStamina;
    }
};

/**
 * @struct AIComponent
 * @brief Component that defines AI behavior for entities
 */
struct AIComponent : public Component {
    enum class AIState {
        IDLE,
        PATROL,
        CHASE,
        ATTACK,
        FLEE,
        SEARCH,
        DEAD
    };

    struct PatrolPoint {
        float x, y;
        PatrolPoint(float px, float py) : x(px), y(py) {}
    };

    AIState currentState = AIState::IDLE;
    float detectionRange = 150.0f;   ///< Range to detect targets
    float attackRange = 50.0f;       ///< Range to attack targets
    float patrolSpeed = 50.0f;       ///< Speed when patrolling
    float chaseSpeed = 100.0f;       ///< Speed when chasing
    bool aggressive = true;          ///< Whether to attack on sight
    bool canFlee = false;            ///< Whether can flee when low health
    bool returnsToPatrol = true;     ///< Whether to return to patrol after losing target
    Entity target;                   ///< Current target entity

    // Patrol system
    std::vector<PatrolPoint> patrolPoints;
    int currentPatrolIndex = 0;

    // State timing
    float stateTimer = 0.0f;

    AIComponent() = default;

    /**
     * @brief Add a patrol point to the AI's patrol route
     */
    void AddPatrolPoint(float x, float y) {
        patrolPoints.emplace_back(x, y);
    }

    /**
     * @brief Change the AI's current state and reset state timer
     */
    void ChangeState(AIState newState) {
        currentState = newState;
        stateTimer = 0.0f;
    }
};

/**
 * @struct CombatStatsComponent
 * @brief Component that defines combat-specific statistics
 */
struct CombatStatsComponent : public Component {
    float attackPower = 15.0f;       ///< Base physical attack damage
    float defense = 5.0f;            ///< Physical damage reduction
    float magicPower = 10.0f;        ///< Base magic attack damage
    float magicDefense = 3.0f;       ///< Magic damage reduction
    float speed = 100.0f;            ///< Initiative/turn order speed
    float accuracy = 85.0f;          ///< Hit chance percentage
    float criticalChance = 5.0f;     ///< Critical hit chance percentage
    float criticalMultiplier = 2.0f; ///< Critical hit damage multiplier

    CombatStatsComponent() = default;
    CombatStatsComponent(float atk, float def, float spd)
        : Component(), attackPower(atk), defense(def), speed(spd) {}
};

/**
 * @struct CombatActionComponent
 * @brief Component that defines available combat actions for an entity
 */
struct CombatActionComponent : public Component {
    enum class ActionType {
        ATTACK,
        DEFEND,
        MAGIC_ATTACK,
        HEAL,
        BUFF,
        DEBUFF,
        ITEM,
        FLEE
    };

    struct CombatAction {
        ActionType type;
        std::string name;
        float mpCost = 0.0f;
        float power = 1.0f;
        bool targetsSelf = false;
        bool targetsAll = false;

        CombatAction(ActionType t, const std::string& n, float cost = 0.0f, float pow = 1.0f)
            : type(t), name(n), mpCost(cost), power(pow) {}
    };

    std::vector<CombatAction> availableActions;
    int selectedActionIndex = 0;

    CombatActionComponent() {
        // Default actions for all entities
        availableActions.emplace_back(ActionType::ATTACK, "Attack");
        availableActions.emplace_back(ActionType::DEFEND, "Defend");
    }
};

/**
 * @struct TurnOrderComponent
 * @brief Component that manages turn-based combat order
 */
struct TurnOrderComponent : public Component {
    float initiative = 100.0f;       ///< Base initiative value
    float currentInitiative = 100.0f; ///< Current turn initiative
    int turnOrder = 0;               ///< Position in turn order (0 = first)
    bool hasTakenTurn = false;       ///< Whether entity has acted this round
    bool isDefending = false;        ///< Whether entity is in defensive stance
    float defenseBonus = 0.0f;       ///< Temporary defense bonus

    TurnOrderComponent() = default;
    TurnOrderComponent(float init) : Component(), initiative(init), currentInitiative(init) {}

    /**
     * @brief Reset for new combat round
     */
    void ResetForNewRound() {
        hasTakenTurn = false;
        isDefending = false;
        defenseBonus = 0.0f;
        currentInitiative = initiative;
    }
};

/**
 * @struct BattleParticipantComponent
 * @brief Component that marks entities as participants in current battle
 */
struct BattleParticipantComponent : public Component {
    enum class ParticipantType {
        PLAYER,
        ALLY,
        ENEMY,
        NEUTRAL
    };

    ParticipantType type = ParticipantType::NEUTRAL;
    bool isAlive = true;
    bool canAct = true;
    int battlePosition = 0;          ///< Position in battle formation
    Entity originalEntity;           ///< Reference to original world entity

    BattleParticipantComponent() = default;
    BattleParticipantComponent(ParticipantType t, int pos = 0)
        : Component(), type(t), battlePosition(pos) {}
};

/**
 * @struct AbilityComponent
 * @brief Component that defines special abilities for entities
 */
struct AbilityComponent : public Component {
    struct Ability {
        std::string name;
        float cooldown = 0.0f;       ///< Cooldown time in seconds
        float currentCooldown = 0.0f; ///< Current cooldown remaining
        float manaCost = 0.0f;       ///< Mana cost to use ability
        float staminaCost = 0.0f;    ///< Stamina cost to use ability
        float damage = 0.0f;         ///< Damage dealt by ability
        float range = 0.0f;          ///< Range of ability
        bool isActive = false;       ///< Whether ability is currently active
    };

    std::vector<Ability> abilities;

    AbilityComponent() = default;

    /**
     * @brief Add a new ability to this component
     */
    void AddAbility(const Ability& ability) {
        abilities.push_back(ability);
    }

    /**
     * @brief Add a new ability with parameters
     */
    void AddAbility(const std::string& name, float cooldown, float manaCost = 0.0f,
                   float staminaCost = 0.0f, float damage = 0.0f, float range = 0.0f) {
        Ability ability;
        ability.name = name;
        ability.cooldown = cooldown;
        ability.manaCost = manaCost;
        ability.staminaCost = staminaCost;
        ability.damage = damage;
        ability.range = range;
        abilities.push_back(ability);
    }

    /**
     * @brief Check if an ability can be used based on resource costs
     */
    bool CanUseAbility(size_t abilityIndex, float currentMana, float currentStamina) const {
        if (abilityIndex >= abilities.size()) return false;

        const Ability& ability = abilities[abilityIndex];

        // Check if ability is on cooldown
        if (ability.currentCooldown > 0.0f) return false;

        // Check resource costs
        if (ability.manaCost > currentMana) return false;
        if (ability.staminaCost > currentStamina) return false;

        return true;
    }

    /**
     * @brief Update cooldowns for all abilities
     */
    void UpdateCooldowns(float deltaTime) {
        for (Ability& ability : abilities) {
            if (ability.currentCooldown > 0.0f) {
                ability.currentCooldown = std::max(0.0f, ability.currentCooldown - deltaTime);
            }
        }
    }
};

/** @} */ // end of Components group
