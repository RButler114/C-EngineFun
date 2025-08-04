/**
 * @file KeybindingManager.h
 * @brief Configurable keybinding system for input mapping
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include <SDL2/SDL.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

// Forward declarations
class ConfigManager;

// Forward declarations
class InputManager;

/**
 * @enum GameAction
 * @brief Enumeration of all possible game actions that can be bound to keys
 */
enum class GameAction {
    // Movement
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    
    // Menu Navigation
    MENU_UP,
    MENU_DOWN,
    MENU_LEFT,
    MENU_RIGHT,
    MENU_SELECT,
    MENU_BACK,
    MENU_CONFIRM,
    
    // Game Controls
    PAUSE,
    RETURN_TO_MENU,
    RELOAD_CONFIG,
    QUICK_QUIT,
    
    // Level Selection
    LEVEL_1,
    LEVEL_2,
    LEVEL_3,
    LEVEL_BOSS,
    LEVEL_RESET,
    
    // Customization
    CUSTOMIZE_CONFIRM,
    
    // Debug
    DEBUG_TOGGLE,
    
    // Count (must be last)
    ACTION_COUNT
};

/**
 * @struct KeyBinding
 * @brief Represents a key binding with primary and alternative keys
 */
struct KeyBinding {
    SDL_Scancode primaryKey = SDL_SCANCODE_UNKNOWN;
    SDL_Scancode alternativeKey = SDL_SCANCODE_UNKNOWN;
    std::string displayName;
    std::string description;
    bool canBeRebound = true;
    
    KeyBinding() = default;
    KeyBinding(SDL_Scancode primary, SDL_Scancode alternative, 
               const std::string& name, const std::string& desc, bool rebindable = true)
        : primaryKey(primary), alternativeKey(alternative), 
          displayName(name), description(desc), canBeRebound(rebindable) {}
};

/**
 * @class KeybindingManager
 * @brief Manages configurable key bindings for game actions
 * 
 * This class provides a centralized system for mapping game actions to keyboard inputs.
 * It supports:
 * - Primary and alternative key bindings
 * - Configuration file loading/saving
 * - Runtime key rebinding
 * - Input validation and conflict detection
 * - Integration with the options menu
 */
class KeybindingManager {
public:
    KeybindingManager();
    ~KeybindingManager(); // Custom destructor needed for forward-declared ConfigManager
    
    // Configuration
    bool LoadFromConfig(const std::string& configPath);
    bool SaveToConfig(const std::string& configPath) const;
    void InitializeDefaults();
    
    // Key binding queries
    bool IsActionPressed(GameAction action, const InputManager* inputManager) const;
    bool IsActionJustPressed(GameAction action, const InputManager* inputManager) const;
    bool IsActionJustReleased(GameAction action, const InputManager* inputManager) const;
    
    // Key binding management
    const KeyBinding& GetBinding(GameAction action) const;
    bool SetBinding(GameAction action, SDL_Scancode primaryKey, SDL_Scancode alternativeKey = SDL_SCANCODE_UNKNOWN);
    bool SetPrimaryKey(GameAction action, SDL_Scancode key);
    bool SetAlternativeKey(GameAction action, SDL_Scancode key);
    
    // Validation
    bool IsKeyConflicted(SDL_Scancode key, GameAction excludeAction = GameAction::ACTION_COUNT) const;
    std::vector<GameAction> GetConflictingActions(SDL_Scancode key) const;
    bool ValidateBinding(GameAction action, SDL_Scancode key) const;
    
    // Utility
    std::string GetKeyName(SDL_Scancode key) const;
    SDL_Scancode GetKeyFromName(const std::string& keyName) const;
    std::string GetActionName(GameAction action) const;
    std::string GetActionDisplayName(GameAction action) const;
    
    // Options menu integration
    std::vector<GameAction> GetConfigurableActions() const;
    bool IsActionConfigurable(GameAction action) const;
    
    // Reset
    void ResetToDefaults();
    void ResetAction(GameAction action);
    
private:
    std::unordered_map<GameAction, KeyBinding> m_bindings;
    std::unique_ptr<ConfigManager> m_config;
    
    // Helper methods
    void SetupDefaultBindings();
    std::string ScancodeToString(SDL_Scancode scancode) const;
    SDL_Scancode StringToScancode(const std::string& str) const;
    bool IsKeyPressed(SDL_Scancode key, const InputManager* inputManager) const;
    bool IsKeyJustPressed(SDL_Scancode key, const InputManager* inputManager) const;
    bool IsKeyJustReleased(SDL_Scancode key, const InputManager* inputManager) const;
    
    // Action name mappings
    void InitializeActionNames();
    std::unordered_map<GameAction, std::string> m_actionNames;
    std::unordered_map<GameAction, std::string> m_actionDisplayNames;
    std::unordered_map<std::string, GameAction> m_nameToAction;
};
