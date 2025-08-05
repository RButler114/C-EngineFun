/**
 * @file PlayerCustomization.h
 * @brief Player customization system for character appearance and attributes
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

/**
 * @enum CustomizationCategory
 * @brief Categories of customization options
 */
enum class CustomizationCategory {
    BASIC_INFO,     // Name, class
    APPEARANCE,     // Hair, skin, eyes
    ATTRIBUTES,     // Stats distribution
    EQUIPMENT       // Starting gear (future expansion)
};

/**
 * @struct CustomizationOption
 * @brief Represents a single customization choice
 */
struct CustomizationOption {
    std::string id;           // Unique identifier
    std::string displayName;  // User-friendly name
    std::string description;  // Optional description
    std::string value;        // The actual value/path
    bool unlocked = true;     // Whether this option is available
    
    CustomizationOption() = default;
    CustomizationOption(const std::string& id, const std::string& name, const std::string& val)
        : id(id), displayName(name), value(val) {}
};

/**
 * @struct CustomizationGroup
 * @brief A group of related customization options
 */
struct CustomizationGroup {
    std::string id;
    std::string displayName;
    CustomizationCategory category;
    std::vector<CustomizationOption> options;
    int selectedIndex = 0;  // Currently selected option
    
    CustomizationGroup() = default;
    CustomizationGroup(const std::string& id, const std::string& name, CustomizationCategory cat)
        : id(id), displayName(name), category(cat) {}
        
    const CustomizationOption* GetSelectedOption() const {
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(options.size())) {
            return &options[selectedIndex];
        }
        return nullptr;
    }
};

/**
 * @struct PlayerCustomization
 * @brief Stores all player customization choices
 */
struct PlayerCustomization {
    // Basic Info
    std::string playerName = "Hero";
    std::string characterClass = "warrior";
    
    // Appearance
    std::string hairColor = "brown";
    std::string skinTone = "medium";
    std::string eyeColor = "brown";
    std::string hairStyle = "short";
    
    // Attributes (point distribution)
    float strength = 15.0f;
    float agility = 12.0f;
    float intelligence = 10.0f;
    float vitality = 15.0f;
    int availablePoints = 0;  // Points left to distribute
    
    // Equipment (for future expansion)
    std::string startingWeapon = "sword";
    std::string startingArmor = "leather";
    
    // Sprite customization
    std::string spritePath = "assets/sprites/player/little_adventurer.png";
    
    /**
     * @brief Reset to default values
     */
    void Reset() {
        playerName = "Hero";
        characterClass = "warrior";
        hairColor = "brown";
        skinTone = "medium";
        eyeColor = "brown";
        hairStyle = "short";
        strength = 15.0f;
        agility = 12.0f;
        intelligence = 10.0f;
        vitality = 15.0f;
        availablePoints = 0;
        startingWeapon = "sword";
        startingArmor = "leather";
        spritePath = "assets/sprites/player/little_adventurer.png";
    }
    
    /**
     * @brief Calculate total attribute points used
     */
    float GetTotalAttributePoints() const {
        return strength + agility + intelligence + vitality;
    }
    
    /**
     * @brief Validate that customization is complete and valid
     */
    bool IsValid() const {
        return !playerName.empty() && 
               !characterClass.empty() && 
               availablePoints >= 0 &&
               GetTotalAttributePoints() > 0;
    }
};

/**
 * @class CustomizationManager
 * @brief Manages customization options and player choices
 */
class CustomizationManager {
public:
    CustomizationManager();
    ~CustomizationManager() = default;

    // Singleton access for global customization state
    static CustomizationManager& GetInstance() {
        static CustomizationManager instance;
        return instance;
    }
    
    // Configuration loading
    bool LoadFromConfig(const std::string& configPath);
    void InitializeDefaults();
    
    // Group management
    void AddGroup(const CustomizationGroup& group);
    CustomizationGroup* GetGroup(const std::string& groupId);
    const std::vector<CustomizationGroup>& GetAllGroups() const { return m_groups; }
    std::vector<CustomizationGroup*> GetGroupsByCategory(CustomizationCategory category);
    
    // Player customization
    PlayerCustomization& GetPlayerCustomization() { return m_playerCustomization; }
    const PlayerCustomization& GetPlayerCustomization() const { return m_playerCustomization; }
    void ResetPlayerCustomization() { m_playerCustomization.Reset(); }
    
    // Apply customization choices to player data
    void ApplyGroupSelection(const std::string& groupId, int optionIndex);
    void ApplyCustomizationToPlayerData();
    
    // Validation
    bool ValidateCustomization() const { return m_playerCustomization.IsValid(); }
    
private:
    std::vector<CustomizationGroup> m_groups;
    PlayerCustomization m_playerCustomization;
    
    void SetupBasicInfoGroups();
    void SetupAppearanceGroups();
    void SetupAttributeGroups();
    void SetupEquipmentGroups();
};
