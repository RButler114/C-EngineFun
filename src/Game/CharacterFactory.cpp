/**
 * @file CharacterFactory.cpp
 * @brief Implementation of CharacterFactory methods
 * @author Ryan Butler
 * @date 2025
 */

#include "Game/CharacterFactory.h"
#include "Game/PlayerCustomization.h"
#include <iostream>

Entity CharacterFactory::CreateCustomizedPlayer(float x, float y, const PlayerCustomization& customization) {
    // Start with the base player template
    CharacterTemplate playerTemplate;
    auto it = m_templates.find("player");
    if (it != m_templates.end()) {
        playerTemplate = it->second;
    } else {
        // Use default player template if not found
        playerTemplate = CharacterTemplate("Player", CharacterTypeComponent::CharacterType::PLAYER, 
                                          CharacterTypeComponent::CharacterClass::WARRIOR);
    }
    
    // Apply customizations to the template
    playerTemplate.name = customization.playerName;
    
    // Map character class string to enum
    if (customization.characterClass == "warrior") {
        playerTemplate.characterClass = CharacterTypeComponent::CharacterClass::WARRIOR;
    } else if (customization.characterClass == "archer") {
        playerTemplate.characterClass = CharacterTypeComponent::CharacterClass::ARCHER;
    } else if (customization.characterClass == "mage") {
        playerTemplate.characterClass = CharacterTypeComponent::CharacterClass::MAGE;
    } else if (customization.characterClass == "rogue") {
        playerTemplate.characterClass = CharacterTypeComponent::CharacterClass::ROGUE;
    }
    
    // Apply attribute customizations
    playerTemplate.strength = customization.strength;
    playerTemplate.agility = customization.agility;
    playerTemplate.intelligence = customization.intelligence;
    playerTemplate.vitality = customization.vitality;
    
    // Apply sprite customization
    playerTemplate.spritePath = customization.spritePath;
    
    // Adjust health based on vitality
    playerTemplate.maxHealth = 80.0f + (customization.vitality * 2.0f);
    
    std::cout << "Creating customized player: " << playerTemplate.name 
              << " (Class: " << customization.characterClass 
              << ", Health: " << playerTemplate.maxHealth << ")" << std::endl;
    
    // Create the entity using the customized template
    return CreateCharacterFromTemplate(playerTemplate, x, y);
}
