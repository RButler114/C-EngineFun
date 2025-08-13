/**
 * @file CharacterFactory.cpp
 * @brief Implementation of CharacterFactory methods
 * @author Ryan Butler
 * @date 2025
 */

#include "Game/CharacterFactory.h"
#include "Game/PlayerCustomization.h"
#include <iostream>

/**
 * @brief Create a player character with custom attributes and appearance
 *
 * Creates a player entity using the base player template combined with
 * player-selected customizations. This allows players to personalize
 * their character's stats, appearance, and class before starting the game.
 *
 * @param x Starting X position in world coordinates
 * @param y Starting Y position in world coordinates
 * @param customization Player customization data (name, class, stats, appearance)
 *
 * @return Entity handle for the created player character
 *
 * @note Combines base template with customization overrides
 * @note Health is calculated based on vitality attribute
 * @note Character class affects available abilities and stats
 * @note Sprite path can be customized for different character appearances
 *
 * Customization Process:
 * 1. Load base player template from configuration
 * 2. Apply player-selected name and class
 * 3. Override attributes with custom values
 * 4. Set custom sprite path for appearance
 * 5. Calculate derived stats (health from vitality)
 * 6. Create entity using the customized template
 *
 * @example
 * ```cpp
 * PlayerCustomization custom;
 * custom.playerName = "Hero";
 * custom.characterClass = "mage";
 * custom.intelligence = 15; // High intelligence for mage
 * custom.spritePath = "assets/sprites/mage_blue.png";
 *
 * Entity player = factory.CreateCustomizedPlayer(100.0f, 200.0f, custom);
 * ```
 */
Entity CharacterFactory::CreateCustomizedPlayer(float x, float y, const PlayerCustomization& customization) {
    // Step 1: Start with the base player template from configuration
    CharacterTemplate playerTemplate;
    auto it = m_templates.find("player");
    if (it != m_templates.end()) {
        // Use configured player template as base
        playerTemplate = it->second;
        std::cout << "📋 Using configured player template" << std::endl;
    } else {
        // Fallback: Create default player template if config not found
        playerTemplate = CharacterTemplate("Player", CharacterTypeComponent::CharacterType::PLAYER,
                                          CharacterTypeComponent::CharacterClass::WARRIOR);
        std::cout << "⚠️  Using default player template (config not found)" << std::endl;
    }

    // Step 2: Apply player customizations to the template
    playerTemplate.name = customization.playerName;

    // Step 3: Map character class string to enum value
    // This allows the customization system to use string names while
    // the ECS system uses efficient enum values
    // Map fine-grained job ids to broad classes used by ECS
    const std::string& job = customization.characterClass;
    auto toLower = [](std::string s){ for (auto& ch : s) ch = std::tolower(static_cast<unsigned char>(ch)); return s; };
    std::string j = toLower(job);
    auto setClass = [&](CharacterTypeComponent::CharacterClass cc){ playerTemplate.characterClass = cc; };

    // Frontline / Melee
    if (j == "warden" || j == "aegis marshal" || j == "aegis_marshal" || j == "sentinel prime" || j == "sentinel_prime"
        || j == "void bastion" || j == "void_bastion" || j == "breaker" || j == "iron prow" || j == "iron_prow") {
        setClass(CharacterTypeComponent::CharacterClass::TANK);
    } else if (j == "star reaver" || j == "star_reaver") {
        setClass(CharacterTypeComponent::CharacterClass::ROGUE);
    }
    // Ranged / Tech
    else if (j == "machinist" || j == "shockwright" || j == "gear savant" || j == "gear_savant" || j == "chronomech"
             || j == "pulse gunner" || j == "pulse_gunner" || j == "star artillerist" || j == "star_artillerist") {
        setClass(CharacterTypeComponent::CharacterClass::ARCHER);
    }
    // Support / Hybrid
    else if (j == "splicer" || j == "biowright" || j == "plague sower" || j == "plague_sower" || j == "star alchemist" || j == "star_alchemist"
             || j == "lifeforge medic" || j == "lifeforge_medic" || j == "cryo archivist" || j == "cryo_archivist") {
        setClass(CharacterTypeComponent::CharacterClass::SUPPORT);
    }
    // Stealth / Mobility
    else if (j == "shadowrunner" || j == "spectreblade" || j == "eclipse dancer" || j == "eclipse_dancer"
             || j == "null phantom" || j == "null_phantom" || j == "hollow wraith" || j == "hollow_wraith" || j == "starshade") {
        setClass(CharacterTypeComponent::CharacterClass::ROGUE);
    }
    // Cosmic / Psionic
    else if (j == "seer" || j == "mindflare" || j == "star oracle" || j == "star_oracle" || j == "eidolon weaver" || j == "eidolon_weaver"
             || j == "dreamsinger" || j == "astromancer") {
        setClass(CharacterTypeComponent::CharacterClass::MAGE);
    } else if (j == "warrior" || j == "archer" || j == "mage" || j == "rogue") {
        // Backwards-compatible support for original four generic classes
        if (j == "warrior") setClass(CharacterTypeComponent::CharacterClass::WARRIOR);
        else if (j == "archer") setClass(CharacterTypeComponent::CharacterClass::ARCHER);
        else if (j == "mage") setClass(CharacterTypeComponent::CharacterClass::MAGE);
        else if (j == "rogue") setClass(CharacterTypeComponent::CharacterClass::ROGUE);
    } else {
        // Unrecognized job id: keep the class from the configured template rather than forcing a generic default
        std::cout << "⚠️  Unrecognized job id '" << job << "' - keeping template class\n";
    }

    // Propagate the chosen fine-grained job id
    playerTemplate.jobId = job;

    // Step 4: Apply custom attribute values
    // These override the base template values with player choices
    playerTemplate.strength = customization.strength;
    playerTemplate.agility = customization.agility;
    playerTemplate.intelligence = customization.intelligence;
    playerTemplate.vitality = customization.vitality;

    // Step 5: Apply visual customization
    playerTemplate.spritePath = customization.spritePath;

    // Step 6: Calculate derived stats based on attributes
    // Health scales with vitality: base 80 + 2 per vitality point
    playerTemplate.maxHealth = 80.0f + (customization.vitality * 2.0f);

    std::cout << "🎨 Creating customized player: \"" << playerTemplate.name
              << "\" (Class: " << customization.characterClass
              << ", Health: " << playerTemplate.maxHealth << ")" << std::endl;

    // Step 7: Create the actual entity using the customized template
    return CreateCharacterFromTemplate(playerTemplate, x, y);
}
