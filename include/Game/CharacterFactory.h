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
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

/**
 * @struct CharacterTemplate
 * @brief Template for creating characters with predefined stats and components
 */
struct CharacterTemplate {
    // Basic info
    std::string name;
    CharacterTypeComponent::CharacterType type;
    CharacterTypeComponent::CharacterClass characterClass;
    // Fine-grained job/archetype identifier (e.g., "warden", "aegis_marshal")
    std::string jobId;

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
        CharacterTemplate copy = characterTemplate;
        if (!copy.jobId.empty() && copy.abilities.empty()) {
            ApplyDefaultAbilitiesForJob(copy);
        }
        m_templates[name] = copy;
}

    // Map fine-grained jobId to default abilities on a template
    void ApplyDefaultAbilitiesForJob(CharacterTemplate& tmpl) {
        // Respect explicitly defined abilities
        if (!tmpl.abilities.empty()) return;

        auto toLower = [](std::string s){ for (auto& ch : s) ch = std::tolower(static_cast<unsigned char>(ch)); return s; };
        std::string j = toLower(tmpl.jobId);

        auto add = [&](const std::string& name, float cd, float mana, float stamina, float dmg, float range){
            CharacterTemplate::AbilityTemplate a; a.name=name; a.cooldown=cd; a.manaCost=mana; a.staminaCost=stamina; a.damage=dmg; a.range=range; tmpl.abilities.push_back(a);
        };

        // Try to load from config: assets/config/customization.ini
        bool loadedFromConfig = false;
        {
            ConfigManager cfg;
            if (cfg.LoadFromFile("assets/config/job_abilities.ini") || cfg.LoadFromFile("assets/config/customization.ini")) {
                std::string sect1 = std::string("job_abilities.") + j;
                std::string sect2 = std::string("job_abilities_") + j;
                std::string sectionName;
                if (cfg.HasSection(sect1)) sectionName = sect1;
                else if (cfg.HasSection(sect2)) sectionName = sect2;

                if (!sectionName.empty()) {
                    const auto& kvs = cfg.GetSections().at(sectionName).GetAll();
                    // Collect ability keys and sort for stable order
                    std::vector<std::string> keys;
                    keys.reserve(kvs.size());
                    for (const auto& kv : kvs) keys.push_back(kv.first);
                    std::sort(keys.begin(), keys.end());

                    auto trim = [](std::string s){
                        size_t b = s.find_first_not_of(" \t\r\n");
                        size_t e = s.find_last_not_of(" \t\r\n");
                        if (b == std::string::npos) return std::string();
                        return s.substr(b, e - b + 1);
                    };

                    for (const auto& key : keys) {
                        std::string raw = kvs.at(key).AsString();
                        // Expect: name,cooldown,mana,stamina,damage,range
                        std::vector<std::string> parts;
                        parts.reserve(6);
                        std::stringstream ss(raw);
                        std::string item;
                        while (std::getline(ss, item, ',')) parts.push_back(trim(item));
                        if (parts.empty()) continue;
                        std::string name = parts[0];
                        auto getf = [&](size_t idx){ try { return idx < parts.size() ? std::stof(parts[idx]) : 0.0f; } catch (...) { return 0.0f; } };
                        float cd = getf(1), mana = getf(2), stam = getf(3), dmg = getf(4), rng = getf(5);
                        add(name, cd, mana, stam, dmg, rng);
                    }
                    if (!tmpl.abilities.empty()) {
                        loadedFromConfig = true;
                    }
                }
            }
        }

        if (loadedFromConfig) return;

        // Fallback: hardcoded defaults if no config entry present
        // Frontline / Melee
        if (j == "warden") {
            add("Shield Bash", 3.0f, 0.0f, 15.0f, 20.0f, 40.0f);
            add("Guard Stance", 8.0f, 0.0f, 10.0f, 0.0f, 0.0f);
        } else if (j == "aegis_marshal") {
            add("Aegis Wall", 10.0f, 0.0f, 20.0f, 0.0f, 0.0f);
            add("Phalanx Rush", 6.0f, 0.0f, 18.0f, 25.0f, 60.0f);
        } else if (j == "sentinel_prime") {
            add("Stunning Overdrive", 12.0f, 0.0f, 25.0f, 30.0f, 50.0f);
        } else if (j == "void_bastion") {
            add("Void Aura", 10.0f, 20.0f, 0.0f, 10.0f, 80.0f);
        } else if (j == "breaker") {
            add("Hammer Slam", 5.0f, 0.0f, 20.0f, 35.0f, 35.0f);
        } else if (j == "star_reaver") {
            add("Cosmic Reap", 7.0f, 10.0f, 10.0f, 40.0f, 60.0f);
            add("Grim Crescent", 9.0f, 15.0f, 10.0f, 55.0f, 70.0f);
        } else if (j == "iron_prow") {
            add("Ram Charge", 6.0f, 0.0f, 25.0f, 30.0f, 70.0f);
        }
        // Ranged / Tech
        else if (j == "machinist") {
            add("Turret Deploy", 10.0f, 0.0f, 15.0f, 15.0f, 120.0f);
        } else if (j == "shockwright") {
            add("Arc Burst", 6.0f, 12.0f, 0.0f, 28.0f, 90.0f);
        } else if (j == "gear_savant") {
            add("Drone Swarm", 12.0f, 20.0f, 0.0f, 22.0f, 150.0f);
        } else if (j == "chronomech") {
            add("Time Dilation", 14.0f, 25.0f, 0.0f, 0.0f, 0.0f);
        } else if (j == "pulse_gunner") {
            add("Pulse Volley", 4.0f, 0.0f, 8.0f, 24.0f, 160.0f);
        } else if (j == "star_artillerist") {
            add("Orbital Bombard", 15.0f, 30.0f, 0.0f, 60.0f, 220.0f);
        }
        // Support / Hybrid
        else if (j == "splicer") {
            add("Genome Spike", 8.0f, 15.0f, 0.0f, 18.0f, 80.0f);
        } else if (j == "biowright") {
            add("Regrowth", 10.0f, 18.0f, 0.0f, 0.0f, 0.0f);
        } else if (j == "plague_sower") {
            add("Parasite Cloud", 12.0f, 20.0f, 0.0f, 16.0f, 100.0f);
        } else if (j == "star_alchemist") {
            add("Anomaly Infusion", 14.0f, 22.0f, 0.0f, 26.0f, 90.0f);
        } else if (j == "lifeforge_medic") {
            add("Emergency Patch", 6.0f, 14.0f, 0.0f, 0.0f, 0.0f);
        } else if (j == "cryo_archivist") {
            add("Cryo Lock", 9.0f, 16.0f, 0.0f, 20.0f, 80.0f);
        }
        // Stealth / Mobility
        else if (j == "shadowrunner") {
            add("Shadowstep", 8.0f, 10.0f, 0.0f, 0.0f, 0.0f);
            add("Quick Strike", 3.0f, 0.0f, 6.0f, 18.0f, 30.0f);
        } else if (j == "spectreblade") {
            add("Silence", 7.0f, 10.0f, 0.0f, 15.0f, 40.0f);
        } else if (j == "eclipse_dancer") {
            add("Afterimage Flurry", 9.0f, 12.0f, 0.0f, 22.0f, 50.0f);
        } else if (j == "null_phantom") {
            add("Phase Through", 11.0f, 15.0f, 0.0f, 0.0f, 0.0f);
        } else if (j == "hollow_wraith") {
            add("Wall Ghost", 10.0f, 15.0f, 0.0f, 0.0f, 0.0f);
        } else if (j == "starshade") {
            add("Umbral Veil", 12.0f, 18.0f, 0.0f, 0.0f, 0.0f);
        }
        // Cosmic / Psionic
        else if (j == "seer") {
            add("Foresight", 10.0f, 18.0f, 0.0f, 0.0f, 0.0f);
        } else if (j == "mindflare") {
            add("Psychic Crush", 8.0f, 16.0f, 0.0f, 26.0f, 100.0f);
        } else if (j == "star_oracle") {
            add("Perfect Counter", 12.0f, 22.0f, 0.0f, 28.0f, 60.0f);
        } else if (j == "eidolon_weaver") {
            add("Astral Construct", 15.0f, 25.0f, 0.0f, 24.0f, 120.0f);
        } else if (j == "dreamsinger") {
            add("Resonance Bend", 10.0f, 20.0f, 0.0f, 20.0f, 90.0f);
        } else if (j == "astromancer") {
            add("Stellar Lance", 9.0f, 18.0f, 0.0f, 32.0f, 140.0f);
        }
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

        // Add character type component and set fine-grained job id
        if (auto* typeComp = m_entityManager->AddComponent<CharacterTypeComponent>(entity, tmpl.type, tmpl.characterClass, tmpl.name)) {
            typeComp->jobId = tmpl.jobId;
        }

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

        // Add player abilities (default via job if set, else fallback)
        if (!player.jobId.empty()) {
            ApplyDefaultAbilitiesForJob(player);
        } else {
            CharacterTemplate::AbilityTemplate jumpAttack;
            jumpAttack.name = "Jump Attack";
            jumpAttack.cooldown = 2.0f;
            jumpAttack.staminaCost = 20.0f;
            jumpAttack.damage = 25.0f;
            jumpAttack.range = 60.0f;
            player.abilities.push_back(jumpAttack);
        }

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

        // Parse character class (broad category) and optional fine-grained job id
        std::string classStr = config.Get(sectionName, "class", "monster").AsString();
        if (classStr == "warrior") tmpl.characterClass = CharacterTypeComponent::CharacterClass::WARRIOR;
        else if (classStr == "archer") tmpl.characterClass = CharacterTypeComponent::CharacterClass::ARCHER;
        else if (classStr == "mage") tmpl.characterClass = CharacterTypeComponent::CharacterClass::MAGE;
        else if (classStr == "rogue") tmpl.characterClass = CharacterTypeComponent::CharacterClass::ROGUE;
        else if (classStr == "tank") tmpl.characterClass = CharacterTypeComponent::CharacterClass::TANK;
        else if (classStr == "support") tmpl.characterClass = CharacterTypeComponent::CharacterClass::SUPPORT;
        else if (classStr == "beast") tmpl.characterClass = CharacterTypeComponent::CharacterClass::BEAST;
        else tmpl.characterClass = CharacterTypeComponent::CharacterClass::MONSTER;
        // job handled below

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

        // Fine-grained job/archetype id (optional)
        tmpl.jobId = config.Get(sectionName, "job", "").AsString();

        // Audio
        tmpl.attackSound = config.Get(sectionName, "attack_sound", "").AsString();
        tmpl.hurtSound = config.Get(sectionName, "hurt_sound", "").AsString();
        tmpl.deathSound = config.Get(sectionName, "death_sound", "").AsString();

        return tmpl;
    }
};
