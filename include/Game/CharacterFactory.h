/**
 * @file CharacterFactory.h
 * @brief Factory system for creating different character types
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "ECS/EntityManager.h"
#include "ECS/Component.h"
#include "Engine/ConfigSystem.h"
#include <string>
#include <unordered_map>
#include <functional>

/**
 * @struct CharacterTemplate
 * @brief Template for creating characters with predefined stats and components
 */
struct CharacterTemplate {
    // Basic info
    std::string name;
    CharacterTypeComponent::CharacterType type;
    CharacterTypeComponent::CharacterClass characterClass;
    
    // Health stats
    float maxHealth = 100.0f;
    float armor = 0.0f;
    float healthRegen = 0.0f;
    
    // Character stats
    float strength = 10.0f;
    float agility = 10.0f;
    float intelligence = 10.0f;
    float vitality = 10.0f;
    
    // AI behavior (for non-player characters)
    bool hasAI = false;
    float detectionRange = 150.0f;
    float attackRange = 50.0f;
    float patrolSpeed = 50.0f;
    float chaseSpeed = 100.0f;
    bool aggressive = true;
    bool canFlee = false;
    
    // Visual
    std::string spritePath;
    int spriteWidth = 32;
    int spriteHeight = 32;
    
    // Audio
    std::string attackSound;
    std::string hurtSound;
    std::string deathSound;
    
    // Abilities
    struct AbilityTemplate {
        std::string name;
        float cooldown = 1.0f;
        float manaCost = 0.0f;
        float staminaCost = 0.0f;
        float damage = 10.0f;
        float range = 100.0f;
    };
    std::vector<AbilityTemplate> abilities;
    
    CharacterTemplate() = default;
    CharacterTemplate(const std::string& n, CharacterTypeComponent::CharacterType t, 
                     CharacterTypeComponent::CharacterClass c)
        : name(n), type(t), characterClass(c) {}
};

/**
 * @class CharacterFactory
 * @brief Factory for creating different types of characters
 *
 * Provides a centralized way to create characters with predefined templates,
 * making it easy to spawn different enemy types, player variants, and NPCs.
 */
class CharacterFactory {
public:
    CharacterFactory(EntityManager* entityManager) : m_entityManager(entityManager) {
        InitializeDefaultTemplates();
    }

    /**
     * @brief Load character templates from configuration file
     * @param configFile Path to the configuration file
     * @return true if loaded successfully
     */
    bool LoadFromConfig(const std::string& configFile) {
        ConfigManager config;
        if (!config.LoadFromFile(configFile)) {
            return false;
        }

        // Load all character sections
        for (const auto& [sectionName, section] : config.GetSections()) {
            // Skip non-character sections
            if (sectionName == "balance" || sectionName == "world" ||
                sectionName == "audio" || sectionName == "graphics" ||
                sectionName == "default") {
                continue;
            }

            CharacterTemplate tmpl = LoadCharacterTemplateFromConfig(config, sectionName);
            RegisterTemplate(sectionName, tmpl);
        }

        return true;
    }
    
    /**
     * @brief Create a character from a template
     * @param templateName Name of the character template to use
     * @param x Starting X position
     * @param y Starting Y position
     * @return Created entity, or invalid entity if template not found
     */
    Entity CreateCharacter(const std::string& templateName, float x, float y) {
        auto it = m_templates.find(templateName);
        if (it == m_templates.end()) {
            return Entity(); // Invalid entity
        }
        
        return CreateCharacterFromTemplate(it->second, x, y);
    }
    
    /**
     * @brief Register a new character template
     * @param name Template name
     * @param characterTemplate Template data
     */
    void RegisterTemplate(const std::string& name, const CharacterTemplate& characterTemplate) {
        m_templates[name] = characterTemplate;
    }
    
    /**
     * @brief Get a template for modification
     * @param name Template name
     * @return Pointer to template, or nullptr if not found
     */
    CharacterTemplate* GetTemplate(const std::string& name) {
        auto it = m_templates.find(name);
        return (it != m_templates.end()) ? &it->second : nullptr;
    }
    
    /**
     * @brief Create a player character
     */
    Entity CreatePlayer(float x, float y) {
        return CreateCharacter("player", x, y);
    }

    /**
     * @brief Create a customized player character
     * @param x Starting X position
     * @param y Starting Y position
     * @param customization Player customization data
     * @return Created player entity
     */
    Entity CreateCustomizedPlayer(float x, float y, const struct PlayerCustomization& customization);
    
    /**
     * @brief Create a basic enemy
     */
    Entity CreateBasicEnemy(float x, float y) {
        return CreateCharacter("basic_enemy", x, y);
    }
    
    /**
     * @brief Create a fast enemy
     */
    Entity CreateFastEnemy(float x, float y) {
        return CreateCharacter("fast_enemy", x, y);
    }
    
    /**
     * @brief Create a tank enemy
     */
    Entity CreateTankEnemy(float x, float y) {
        return CreateCharacter("tank_enemy", x, y);
    }
    
    /**
     * @brief Create a boss enemy
     */
    Entity CreateBoss(float x, float y) {
        return CreateCharacter("boss", x, y);
    }

private:
    EntityManager* m_entityManager;
    std::unordered_map<std::string, CharacterTemplate> m_templates;
    
    Entity CreateCharacterFromTemplate(const CharacterTemplate& tmpl, float x, float y) {
        Entity entity = m_entityManager->CreateEntity();
        
        // Add transform component
        m_entityManager->AddComponent<TransformComponent>(entity, x, y);
        
        // Add velocity component
        m_entityManager->AddComponent<VelocityComponent>(entity, 0.0f, 0.0f);
        
        // Add character type component
        m_entityManager->AddComponent<CharacterTypeComponent>(entity, tmpl.type, tmpl.characterClass, tmpl.name);
        
        // Add health component
        m_entityManager->AddComponent<HealthComponent>(entity, tmpl.maxHealth, tmpl.armor, tmpl.healthRegen);
        
        // Add character stats component
        auto* stats = m_entityManager->AddComponent<CharacterStatsComponent>(entity);
        stats->strength = tmpl.strength;
        stats->agility = tmpl.agility;
        stats->intelligence = tmpl.intelligence;
        stats->vitality = tmpl.vitality;
        stats->RecalculateStats();
        
        // Add sprite component if sprite path is provided
        if (!tmpl.spritePath.empty()) {
            m_entityManager->AddComponent<SpriteComponent>(entity, tmpl.spritePath, tmpl.spriteWidth, tmpl.spriteHeight);
        }
        
        // Add collision component
        m_entityManager->AddComponent<CollisionComponent>(entity, static_cast<float>(tmpl.spriteWidth), static_cast<float>(tmpl.spriteHeight));
        
        // Add AI component for non-player characters
        if (tmpl.hasAI && tmpl.type != CharacterTypeComponent::CharacterType::PLAYER) {
            auto* ai = m_entityManager->AddComponent<AIComponent>(entity);
            ai->detectionRange = tmpl.detectionRange;
            ai->attackRange = tmpl.attackRange;
            ai->patrolSpeed = tmpl.patrolSpeed;
            ai->chaseSpeed = tmpl.chaseSpeed;
            ai->aggressive = tmpl.aggressive;
            ai->canFlee = tmpl.canFlee;
        }
        
        // Add abilities if any
        if (!tmpl.abilities.empty()) {
            auto* abilityComp = m_entityManager->AddComponent<AbilityComponent>(entity);
            for (const auto& abilityTmpl : tmpl.abilities) {
                AbilityComponent::Ability ability;
                ability.name = abilityTmpl.name;
                ability.cooldown = abilityTmpl.cooldown;
                ability.manaCost = abilityTmpl.manaCost;
                ability.staminaCost = abilityTmpl.staminaCost;
                ability.damage = abilityTmpl.damage;
                ability.range = abilityTmpl.range;
                abilityComp->AddAbility(ability);
            }
        }
        
        // Add audio component if sounds are specified
        if (!tmpl.attackSound.empty()) {
            m_entityManager->AddComponent<AudioComponent>(entity, tmpl.attackSound, 0.8f, false, false, false);
        }
        
        return entity;
    }
    
    void InitializeDefaultTemplates() {
        // Player template
        CharacterTemplate player("Player", CharacterTypeComponent::CharacterType::PLAYER, 
                                CharacterTypeComponent::CharacterClass::WARRIOR);
        player.maxHealth = 100.0f;
        player.strength = 15.0f;
        player.agility = 12.0f;
        player.intelligence = 10.0f;
        player.vitality = 15.0f;
        player.spritePath = "little_adventurer.png";
        player.spriteWidth = 18;
        player.spriteHeight = 48;
        player.hasAI = false;
        
        // Add player abilities
        CharacterTemplate::AbilityTemplate jumpAttack;
        jumpAttack.name = "Jump Attack";
        jumpAttack.cooldown = 2.0f;
        jumpAttack.staminaCost = 20.0f;
        jumpAttack.damage = 25.0f;
        jumpAttack.range = 60.0f;
        player.abilities.push_back(jumpAttack);
        
        m_templates["player"] = player;
        
        // Basic enemy template
        CharacterTemplate basicEnemy("Goblin", CharacterTypeComponent::CharacterType::ENEMY,
                                   CharacterTypeComponent::CharacterClass::MONSTER);
        basicEnemy.maxHealth = 50.0f;
        basicEnemy.strength = 8.0f;
        basicEnemy.agility = 10.0f;
        basicEnemy.intelligence = 5.0f;
        basicEnemy.vitality = 8.0f;
        basicEnemy.hasAI = true;
        basicEnemy.detectionRange = 120.0f;
        basicEnemy.attackRange = 40.0f;
        basicEnemy.patrolSpeed = 40.0f;
        basicEnemy.chaseSpeed = 80.0f;
        basicEnemy.aggressive = true;
        basicEnemy.canFlee = false;
        
        m_templates["basic_enemy"] = basicEnemy;
        
        // Fast enemy template
        CharacterTemplate fastEnemy("Wolf", CharacterTypeComponent::CharacterType::ENEMY,
                                  CharacterTypeComponent::CharacterClass::BEAST);
        fastEnemy.maxHealth = 30.0f;
        fastEnemy.strength = 6.0f;
        fastEnemy.agility = 18.0f;
        fastEnemy.intelligence = 8.0f;
        fastEnemy.vitality = 6.0f;
        fastEnemy.hasAI = true;
        fastEnemy.detectionRange = 150.0f;
        fastEnemy.attackRange = 35.0f;
        fastEnemy.patrolSpeed = 60.0f;
        fastEnemy.chaseSpeed = 140.0f;
        fastEnemy.aggressive = true;
        fastEnemy.canFlee = true;
        
        m_templates["fast_enemy"] = fastEnemy;
        
        // Tank enemy template
        CharacterTemplate tankEnemy("Orc Warrior", CharacterTypeComponent::CharacterType::ENEMY,
                                  CharacterTypeComponent::CharacterClass::WARRIOR);
        tankEnemy.maxHealth = 150.0f;
        tankEnemy.armor = 0.3f; // 30% damage reduction
        tankEnemy.strength = 20.0f;
        tankEnemy.agility = 5.0f;
        tankEnemy.intelligence = 6.0f;
        tankEnemy.vitality = 20.0f;
        tankEnemy.hasAI = true;
        tankEnemy.detectionRange = 100.0f;
        tankEnemy.attackRange = 50.0f;
        tankEnemy.patrolSpeed = 30.0f;
        tankEnemy.chaseSpeed = 60.0f;
        tankEnemy.aggressive = true;
        tankEnemy.canFlee = false;
        
        m_templates["tank_enemy"] = tankEnemy;
        
        // Boss template
        CharacterTemplate boss("Dragon", CharacterTypeComponent::CharacterType::BOSS,
                             CharacterTypeComponent::CharacterClass::MONSTER);
        boss.maxHealth = 500.0f;
        boss.armor = 0.4f;
        boss.healthRegen = 2.0f;
        boss.strength = 30.0f;
        boss.agility = 8.0f;
        boss.intelligence = 15.0f;
        boss.vitality = 35.0f;
        boss.hasAI = true;
        boss.detectionRange = 200.0f;
        boss.attackRange = 80.0f;
        boss.patrolSpeed = 40.0f;
        boss.chaseSpeed = 80.0f;
        boss.aggressive = true;
        boss.canFlee = false;
        
        // Add boss abilities
        CharacterTemplate::AbilityTemplate fireBreath;
        fireBreath.name = "Fire Breath";
        fireBreath.cooldown = 5.0f;
        fireBreath.manaCost = 50.0f;
        fireBreath.damage = 40.0f;
        fireBreath.range = 150.0f;
        boss.abilities.push_back(fireBreath);
        
        CharacterTemplate::AbilityTemplate tailSwipe;
        tailSwipe.name = "Tail Swipe";
        tailSwipe.cooldown = 3.0f;
        tailSwipe.damage = 30.0f;
        tailSwipe.range = 100.0f;
        boss.abilities.push_back(tailSwipe);
        
        m_templates["boss"] = boss;
    }

    /**
     * @brief Load character template from config (internal helper)
     */
    CharacterTemplate LoadCharacterTemplateFromConfig(const ConfigManager& config, const std::string& sectionName) {
        CharacterTemplate tmpl;

        // Basic info
        tmpl.name = config.Get(sectionName, "name", "Unknown").AsString();

        // Parse character type
        std::string typeStr = config.Get(sectionName, "type", "enemy").AsString();
        if (typeStr == "player") tmpl.type = CharacterTypeComponent::CharacterType::PLAYER;
        else if (typeStr == "enemy") tmpl.type = CharacterTypeComponent::CharacterType::ENEMY;
        else if (typeStr == "boss") tmpl.type = CharacterTypeComponent::CharacterType::BOSS;
        else if (typeStr == "npc") tmpl.type = CharacterTypeComponent::CharacterType::NPC;
        else tmpl.type = CharacterTypeComponent::CharacterType::NEUTRAL;

        // Parse character class
        std::string classStr = config.Get(sectionName, "class", "monster").AsString();
        if (classStr == "warrior") tmpl.characterClass = CharacterTypeComponent::CharacterClass::WARRIOR;
        else if (classStr == "archer") tmpl.characterClass = CharacterTypeComponent::CharacterClass::ARCHER;
        else if (classStr == "mage") tmpl.characterClass = CharacterTypeComponent::CharacterClass::MAGE;
        else if (classStr == "rogue") tmpl.characterClass = CharacterTypeComponent::CharacterClass::ROGUE;
        else if (classStr == "tank") tmpl.characterClass = CharacterTypeComponent::CharacterClass::TANK;
        else if (classStr == "support") tmpl.characterClass = CharacterTypeComponent::CharacterClass::SUPPORT;
        else if (classStr == "beast") tmpl.characterClass = CharacterTypeComponent::CharacterClass::BEAST;
        else tmpl.characterClass = CharacterTypeComponent::CharacterClass::MONSTER;

        // Health stats
        tmpl.maxHealth = config.Get(sectionName, "max_health", 100.0f).AsFloat();
        tmpl.armor = config.Get(sectionName, "armor", 0.0f).AsFloat();
        tmpl.healthRegen = config.Get(sectionName, "health_regen", 0.0f).AsFloat();

        // Character stats
        tmpl.strength = config.Get(sectionName, "strength", 10.0f).AsFloat();
        tmpl.agility = config.Get(sectionName, "agility", 10.0f).AsFloat();
        tmpl.intelligence = config.Get(sectionName, "intelligence", 10.0f).AsFloat();
        tmpl.vitality = config.Get(sectionName, "vitality", 10.0f).AsFloat();

        // AI behavior
        tmpl.hasAI = config.Get(sectionName, "has_ai", false).AsBool();
        tmpl.detectionRange = config.Get(sectionName, "detection_range", 150.0f).AsFloat();
        tmpl.attackRange = config.Get(sectionName, "attack_range", 50.0f).AsFloat();
        tmpl.patrolSpeed = config.Get(sectionName, "patrol_speed", 50.0f).AsFloat();
        tmpl.chaseSpeed = config.Get(sectionName, "chase_speed", 100.0f).AsFloat();
        tmpl.aggressive = config.Get(sectionName, "aggressive", true).AsBool();
        tmpl.canFlee = config.Get(sectionName, "can_flee", false).AsBool();

        // Visual
        tmpl.spritePath = config.Get(sectionName, "sprite_path", "").AsString();
        tmpl.spriteWidth = config.Get(sectionName, "sprite_width", 32).AsInt();
        tmpl.spriteHeight = config.Get(sectionName, "sprite_height", 32).AsInt();

        // Audio
        tmpl.attackSound = config.Get(sectionName, "attack_sound", "").AsString();
        tmpl.hurtSound = config.Get(sectionName, "hurt_sound", "").AsString();
        tmpl.deathSound = config.Get(sectionName, "death_sound", "").AsString();

        return tmpl;
    }
};
