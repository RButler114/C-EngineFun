/**
 * @file PlayerCustomization.cpp
 * @brief Implementation of player customization system
 * @author Ryan Butler
 * @date 2025
 */

#include "Game/PlayerCustomization.h"
#include "Engine/ConfigSystem.h"
#include <iostream>
#include <algorithm>

CustomizationManager::CustomizationManager() {
    InitializeDefaults();
}

bool CustomizationManager::LoadFromConfig(const std::string& configPath) {
    ConfigManager config;
    if (!config.LoadFromFile(configPath)) {
        std::cerr << "Failed to load customization config: " << configPath << std::endl;
        return false;
    }

    // Clear existing groups
    m_groups.clear();

    // Load groups from config
    // This is a simplified version - in a full implementation,
    // we'd parse the entire config structure
    std::cout << "Loaded customization config from: " << configPath << std::endl;

    // For now, fall back to defaults
    InitializeDefaults();
    return true;
    // Add a simple sprite/skin selection group under Appearance
    CustomizationGroup spriteGroup("sprite_skin", "Sprite Skin", CustomizationCategory::APPEARANCE);
    spriteGroup.options.emplace_back("adventurer", "Adventurer", "little_adventurer.png");
    spriteGroup.options.emplace_back("archer", "Archer", "archer_sprite.png");
    spriteGroup.options.emplace_back("mage", "Mage", "mage_sprite.png");
    AddGroup(spriteGroup);

}

void CustomizationManager::InitializeDefaults() {
    m_groups.clear();
    m_playerCustomization.Reset();

    SetupBasicInfoGroups();
    SetupAppearanceGroups();
    SetupAttributeGroups();
    SetupEquipmentGroups();
}

void CustomizationManager::AddGroup(const CustomizationGroup& group) {
    m_groups.push_back(group);
}

CustomizationGroup* CustomizationManager::GetGroup(const std::string& groupId) {
    auto it = std::find_if(m_groups.begin(), m_groups.end(),
        [&groupId](const CustomizationGroup& group) {
            return group.id == groupId;
        });

    return (it != m_groups.end()) ? &(*it) : nullptr;
}

std::vector<CustomizationGroup*> CustomizationManager::GetGroupsByCategory(CustomizationCategory category) {
    std::vector<CustomizationGroup*> result;

    for (auto& group : m_groups) {
        if (group.category == category) {
            result.push_back(&group);
        }
    }

    return result;
}

void CustomizationManager::ApplyGroupSelection(const std::string& groupId, int optionIndex) {
    CustomizationGroup* group = GetGroup(groupId);
    if (!group || optionIndex < 0 || optionIndex >= static_cast<int>(group->options.size())) {
        return;
    }

    group->selectedIndex = optionIndex;
    const CustomizationOption& option = group->options[optionIndex];

    // Apply the selection to player customization data
    if (groupId == "character_class") {
        m_playerCustomization.characterClass = option.value;
    } else if (groupId == "hair_color") {
        m_playerCustomization.hairColor = option.value;
    } else if (groupId == "skin_tone") {
        m_playerCustomization.skinTone = option.value;
    } else if (groupId == "eye_color") {
        m_playerCustomization.eyeColor = option.value;
    } else if (groupId == "hair_style") {
        m_playerCustomization.hairStyle = option.value;
    } else if (groupId == "starting_weapon") {
        m_playerCustomization.startingWeapon = option.value;
    } else if (groupId == "starting_armor") {
        m_playerCustomization.startingArmor = option.value;
    }
}

void CustomizationManager::ApplyCustomizationToPlayerData() {
    // This method will be called when transitioning to gameplay
    // It ensures all customization choices are properly applied

    // Update sprite path based on appearance choices
    // For now, we'll use a simple mapping
    // If a sprite skin group selection exists, prefer it over class mapping
    if (CustomizationGroup* spriteGroup = GetGroup("sprite_skin")) {
        if (spriteGroup->selectedIndex >= 0 && spriteGroup->selectedIndex < (int)spriteGroup->options.size()) {
            m_playerCustomization.spritePath = spriteGroup->options[spriteGroup->selectedIndex].value;
        }
    } else {
        // fallback by broad class
        if (m_playerCustomization.characterClass == "warrior") {
            m_playerCustomization.spritePath = "little_adventurer.png";
        } else if (m_playerCustomization.characterClass == "archer") {
            m_playerCustomization.spritePath = "archer_sprite.png";
        } else if (m_playerCustomization.characterClass == "mage") {
            m_playerCustomization.spritePath = "mage_sprite.png";
        } else if (m_playerCustomization.characterClass == "rogue") {
            m_playerCustomization.spritePath = "rogue_sprite.png";
        }
    }

    std::cout << "Applied customization: " << m_playerCustomization.playerName
              << " (" << m_playerCustomization.characterClass << ")" << std::endl;
}

void CustomizationManager::SetupBasicInfoGroups() {
    // Character Class selection
    CustomizationGroup classGroup("character_class", "Character Class", CustomizationCategory::BASIC_INFO);

    // Load from config [character_classes] if available; fallback to built-ins
    ConfigManager cfg;
    if (cfg.LoadFromFile("assets/config/customization.ini") && cfg.HasSection("character_classes")) {
        const auto& sect = cfg.GetSections().at("character_classes").GetAll();
        for (const auto& kv : sect) {
            // Expect value format: DisplayName,jobId,Description
            std::string raw = kv.second.AsString();
            std::string display, job, desc;
            size_t p1 = raw.find(',');
            size_t p2 = (p1 != std::string::npos) ? raw.find(',', p1 + 1) : std::string::npos;
            if (p1 != std::string::npos) {
                display = raw.substr(0, p1);
                if (p2 != std::string::npos) {
                    job = raw.substr(p1 + 1, p2 - p1 - 1);
                    desc = raw.substr(p2 + 1);
                } else {
                    job = raw.substr(p1 + 1);
                }
            } else {
                display = kv.first;
                job = kv.first;
            }
            classGroup.options.emplace_back(kv.first, display, job);
            if (!desc.empty()) classGroup.options.back().description = desc;
        }
    } else {
        classGroup.options.emplace_back("warrior", "Warrior", "warrior");
        classGroup.options.emplace_back("archer", "Archer", "archer");
        classGroup.options.emplace_back("mage", "Mage", "mage");
        classGroup.options.emplace_back("rogue", "Rogue", "rogue");
    }

    AddGroup(classGroup);
}

void CustomizationManager::SetupAppearanceGroups() {
    // Hair Color
    CustomizationGroup hairColorGroup("hair_color", "Hair Color", CustomizationCategory::APPEARANCE);
    hairColorGroup.options.emplace_back("brown", "Brown", "brown");
    hairColorGroup.options.emplace_back("black", "Black", "black");
    hairColorGroup.options.emplace_back("blonde", "Blonde", "blonde");
    hairColorGroup.options.emplace_back("red", "Red", "red");
    hairColorGroup.options.emplace_back("white", "White", "white");
    AddGroup(hairColorGroup);

    // Sprite/Skin selection
    CustomizationGroup spriteGroup("sprite_skin", "Sprite Skin", CustomizationCategory::APPEARANCE);
    spriteGroup.options.emplace_back("adventurer", "Adventurer", "little_adventurer.png");
    spriteGroup.options.emplace_back("archer", "Archer", "archer_sprite.png");
    spriteGroup.options.emplace_back("mage", "Mage", "mage_sprite.png");
    AddGroup(spriteGroup);

    // Skin Tone
    CustomizationGroup skinGroup("skin_tone", "Skin Tone", CustomizationCategory::APPEARANCE);
    skinGroup.options.emplace_back("light", "Light", "light");
    skinGroup.options.emplace_back("medium", "Medium", "medium");
    skinGroup.options.emplace_back("dark", "Dark", "dark");
    skinGroup.options.emplace_back("tan", "Tan", "tan");
    AddGroup(skinGroup);

    // Eye Color
    CustomizationGroup eyeGroup("eye_color", "Eye Color", CustomizationCategory::APPEARANCE);
    eyeGroup.options.emplace_back("brown", "Brown", "brown");
    eyeGroup.options.emplace_back("blue", "Blue", "blue");
    eyeGroup.options.emplace_back("green", "Green", "green");
    eyeGroup.options.emplace_back("hazel", "Hazel", "hazel");
    eyeGroup.options.emplace_back("gray", "Gray", "gray");
    AddGroup(eyeGroup);

    // Hair Style
    CustomizationGroup hairStyleGroup("hair_style", "Hair Style", CustomizationCategory::APPEARANCE);
    hairStyleGroup.options.emplace_back("short", "Short", "short");
    hairStyleGroup.options.emplace_back("long", "Long", "long");
    hairStyleGroup.options.emplace_back("curly", "Curly", "curly");
    hairStyleGroup.options.emplace_back("braided", "Braided", "braided");
    AddGroup(hairStyleGroup);
}

void CustomizationManager::SetupAttributeGroups() {
    // Attributes will be handled differently - they're numeric values
    // For now, we'll set up some preset attribute distributions

    CustomizationGroup attributeGroup("attribute_preset", "Attribute Focus", CustomizationCategory::ATTRIBUTES);
    attributeGroup.options.emplace_back("balanced", "Balanced (15/12/10/15)", "balanced");
    attributeGroup.options.emplace_back("strong", "Strong (20/10/8/14)", "strong");
    attributeGroup.options.emplace_back("agile", "Agile (12/18/10/12)", "agile");
    attributeGroup.options.emplace_back("smart", "Smart (10/12/18/12)", "smart");
    attributeGroup.options.emplace_back("tough", "Tough (14/10/8/20)", "tough");
    AddGroup(attributeGroup);
}

void CustomizationManager::SetupEquipmentGroups() {
    // Starting Weapon
    CustomizationGroup weaponGroup("starting_weapon", "Starting Weapon", CustomizationCategory::EQUIPMENT);
    weaponGroup.options.emplace_back("sword", "Iron Sword", "sword");
    weaponGroup.options.emplace_back("bow", "Wooden Bow", "bow");
    weaponGroup.options.emplace_back("staff", "Magic Staff", "staff");
    weaponGroup.options.emplace_back("dagger", "Steel Dagger", "dagger");
    AddGroup(weaponGroup);

    // Starting Armor
    CustomizationGroup armorGroup("starting_armor", "Starting Armor", CustomizationCategory::EQUIPMENT);
    armorGroup.options.emplace_back("leather", "Leather Armor", "leather");
    armorGroup.options.emplace_back("cloth", "Cloth Robes", "cloth");
    armorGroup.options.emplace_back("chain", "Chain Mail", "chain");
    armorGroup.options.emplace_back("light", "Light Armor", "light");
    AddGroup(armorGroup);
}
