/**
 * @file KeybindingManager.cpp
 * @brief Implementation of configurable keybinding system
 * @author Ryan Butler
 * @date 2025
 */

#include "Engine/KeybindingManager.h"
#include "Engine/ConfigSystem.h"
#include "Engine/InputManager.h"
#include <iostream>
#include <algorithm>

KeybindingManager::KeybindingManager()
    : m_config(std::make_unique<ConfigManager>()) {
    InitializeActionNames();
    InitializeDefaults();
}

KeybindingManager::~KeybindingManager() = default;

bool KeybindingManager::LoadFromConfig(const std::string& configPath) {
    if (!m_config->LoadFromFile(configPath)) {
        std::cerr << "Failed to load keybinding config: " << configPath << std::endl;
        return false;
    }
    
    // Load each action's bindings
    for (int i = 0; i < static_cast<int>(GameAction::ACTION_COUNT); ++i) {
        GameAction action = static_cast<GameAction>(i);
        std::string actionName = GetActionName(action);
        
        // Get primary key
        std::string primaryKeyStr = m_config->Get("keybindings", actionName + "_primary", "").AsString();
        SDL_Scancode primaryKey = StringToScancode(primaryKeyStr);
        
        // Get alternative key
        std::string altKeyStr = m_config->Get("keybindings", actionName + "_alt", "").AsString();
        SDL_Scancode altKey = StringToScancode(altKeyStr);
        
        // Update binding if valid keys were found
        if (primaryKey != SDL_SCANCODE_UNKNOWN || altKey != SDL_SCANCODE_UNKNOWN) {
            auto& binding = m_bindings[action];
            if (primaryKey != SDL_SCANCODE_UNKNOWN) {
                binding.primaryKey = primaryKey;
            }
            if (altKey != SDL_SCANCODE_UNKNOWN) {
                binding.alternativeKey = altKey;
            }
        }
    }
    
    std::cout << "Loaded keybindings from: " << configPath << std::endl;
    return true;
}

bool KeybindingManager::SaveToConfig(const std::string& configPath) const {
    // Save all current bindings to config
    for (const auto& [action, binding] : m_bindings) {
        std::string actionName = GetActionName(action);
        
        if (binding.primaryKey != SDL_SCANCODE_UNKNOWN) {
            m_config->Set("keybindings", actionName + "_primary", 
                         ConfigValue(ScancodeToString(binding.primaryKey)));
        }
        
        if (binding.alternativeKey != SDL_SCANCODE_UNKNOWN) {
            m_config->Set("keybindings", actionName + "_alt", 
                         ConfigValue(ScancodeToString(binding.alternativeKey)));
        }
    }
    
    return m_config->SaveToFile(configPath);
}

void KeybindingManager::InitializeDefaults() {
    SetupDefaultBindings();
}

bool KeybindingManager::IsActionPressed(GameAction action, const InputManager* inputManager) const {
    if (!inputManager) return false;
    
    const auto& binding = GetBinding(action);
    return IsKeyPressed(binding.primaryKey, inputManager) || 
           IsKeyPressed(binding.alternativeKey, inputManager);
}

bool KeybindingManager::IsActionJustPressed(GameAction action, const InputManager* inputManager) const {
    if (!inputManager) return false;
    
    const auto& binding = GetBinding(action);
    return IsKeyJustPressed(binding.primaryKey, inputManager) || 
           IsKeyJustPressed(binding.alternativeKey, inputManager);
}

bool KeybindingManager::IsActionJustReleased(GameAction action, const InputManager* inputManager) const {
    if (!inputManager) return false;
    
    const auto& binding = GetBinding(action);
    return IsKeyJustReleased(binding.primaryKey, inputManager) || 
           IsKeyJustReleased(binding.alternativeKey, inputManager);
}

const KeyBinding& KeybindingManager::GetBinding(GameAction action) const {
    static KeyBinding emptyBinding;
    auto it = m_bindings.find(action);
    return (it != m_bindings.end()) ? it->second : emptyBinding;
}

bool KeybindingManager::SetBinding(GameAction action, SDL_Scancode primaryKey, SDL_Scancode alternativeKey) {
    if (!ValidateBinding(action, primaryKey) || 
        (alternativeKey != SDL_SCANCODE_UNKNOWN && !ValidateBinding(action, alternativeKey))) {
        return false;
    }
    
    auto& binding = m_bindings[action];
    binding.primaryKey = primaryKey;
    binding.alternativeKey = alternativeKey;
    return true;
}

bool KeybindingManager::SetPrimaryKey(GameAction action, SDL_Scancode key) {
    if (!ValidateBinding(action, key)) return false;
    
    m_bindings[action].primaryKey = key;
    return true;
}

bool KeybindingManager::SetAlternativeKey(GameAction action, SDL_Scancode key) {
    if (key != SDL_SCANCODE_UNKNOWN && !ValidateBinding(action, key)) return false;
    
    m_bindings[action].alternativeKey = key;
    return true;
}

bool KeybindingManager::IsKeyConflicted(SDL_Scancode key, GameAction excludeAction) const {
    if (key == SDL_SCANCODE_UNKNOWN) return false;
    
    for (const auto& [action, binding] : m_bindings) {
        if (action == excludeAction) continue;
        
        if (binding.primaryKey == key || binding.alternativeKey == key) {
            return true;
        }
    }
    return false;
}

std::vector<GameAction> KeybindingManager::GetConflictingActions(SDL_Scancode key) const {
    std::vector<GameAction> conflicts;
    
    for (const auto& [action, binding] : m_bindings) {
        if (binding.primaryKey == key || binding.alternativeKey == key) {
            conflicts.push_back(action);
        }
    }
    
    return conflicts;
}

bool KeybindingManager::ValidateBinding(GameAction action, SDL_Scancode key) const {
    if (key == SDL_SCANCODE_UNKNOWN) return false;
    
    // Check if action can be rebound
    const auto& binding = GetBinding(action);
    if (!binding.canBeRebound) return false;
    
    // Check for conflicts (excluding the action we're setting)
    return !IsKeyConflicted(key, action);
}

std::string KeybindingManager::GetKeyName(SDL_Scancode key) const {
    if (key == SDL_SCANCODE_UNKNOWN) return "None";
    
    const char* name = SDL_GetScancodeName(key);
    return name ? std::string(name) : "Unknown";
}

SDL_Scancode KeybindingManager::GetKeyFromName(const std::string& keyName) const {
    if (keyName.empty() || keyName == "None" || keyName == "Unknown") {
        return SDL_SCANCODE_UNKNOWN;
    }
    
    return SDL_GetScancodeFromName(keyName.c_str());
}

std::string KeybindingManager::GetActionName(GameAction action) const {
    auto it = m_actionNames.find(action);
    return (it != m_actionNames.end()) ? it->second : "unknown";
}

std::string KeybindingManager::GetActionDisplayName(GameAction action) const {
    auto it = m_actionDisplayNames.find(action);
    return (it != m_actionDisplayNames.end()) ? it->second : "Unknown Action";
}

std::vector<GameAction> KeybindingManager::GetConfigurableActions() const {
    std::vector<GameAction> configurable;
    
    for (const auto& [action, binding] : m_bindings) {
        if (binding.canBeRebound) {
            configurable.push_back(action);
        }
    }
    
    return configurable;
}

bool KeybindingManager::IsActionConfigurable(GameAction action) const {
    const auto& binding = GetBinding(action);
    return binding.canBeRebound;
}

void KeybindingManager::ResetToDefaults() {
    SetupDefaultBindings();
}

void KeybindingManager::ResetAction(GameAction action) {
    // Temporarily store all bindings, reset, then restore others
    auto tempBindings = m_bindings;
    SetupDefaultBindings();
    
    // Keep only the reset action's default binding
    KeyBinding defaultBinding = m_bindings[action];
    m_bindings = tempBindings;
    m_bindings[action] = defaultBinding;
}

void KeybindingManager::SetupDefaultBindings() {
    m_bindings.clear();
    
    // Movement
    m_bindings[GameAction::MOVE_LEFT] = KeyBinding(SDL_SCANCODE_LEFT, SDL_SCANCODE_A, "Move Left", "Move character left");
    m_bindings[GameAction::MOVE_RIGHT] = KeyBinding(SDL_SCANCODE_RIGHT, SDL_SCANCODE_D, "Move Right", "Move character right");
    m_bindings[GameAction::MOVE_UP] = KeyBinding(SDL_SCANCODE_UP, SDL_SCANCODE_W, "Move Up", "Move character up");
    m_bindings[GameAction::MOVE_DOWN] = KeyBinding(SDL_SCANCODE_DOWN, SDL_SCANCODE_S, "Move Down", "Move character down");
    
    // Menu Navigation
    m_bindings[GameAction::MENU_UP] = KeyBinding(SDL_SCANCODE_UP, SDL_SCANCODE_W, "Menu Up", "Navigate up in menus");
    m_bindings[GameAction::MENU_DOWN] = KeyBinding(SDL_SCANCODE_DOWN, SDL_SCANCODE_S, "Menu Down", "Navigate down in menus");
    m_bindings[GameAction::MENU_LEFT] = KeyBinding(SDL_SCANCODE_LEFT, SDL_SCANCODE_A, "Menu Left", "Navigate left in menus");
    m_bindings[GameAction::MENU_RIGHT] = KeyBinding(SDL_SCANCODE_RIGHT, SDL_SCANCODE_D, "Menu Right", "Navigate right in menus");
    m_bindings[GameAction::MENU_SELECT] = KeyBinding(SDL_SCANCODE_RETURN, SDL_SCANCODE_SPACE, "Select", "Select menu option");
    m_bindings[GameAction::MENU_BACK] = KeyBinding(SDL_SCANCODE_B, SDL_SCANCODE_UNKNOWN, "Back", "Go back in menus");
    m_bindings[GameAction::MENU_CONFIRM] = KeyBinding(SDL_SCANCODE_RETURN, SDL_SCANCODE_UNKNOWN, "Confirm", "Confirm action");
    
    // Game Controls
    m_bindings[GameAction::PAUSE] = KeyBinding(SDL_SCANCODE_P, SDL_SCANCODE_ESCAPE, "Pause", "Pause/unpause game");
    m_bindings[GameAction::RETURN_TO_MENU] = KeyBinding(SDL_SCANCODE_M, SDL_SCANCODE_UNKNOWN, "Menu", "Return to main menu");
    m_bindings[GameAction::RELOAD_CONFIG] = KeyBinding(SDL_SCANCODE_R, SDL_SCANCODE_UNKNOWN, "Reload Config", "Reload configuration files");
    m_bindings[GameAction::QUICK_QUIT] = KeyBinding(SDL_SCANCODE_ESCAPE, SDL_SCANCODE_UNKNOWN, "Quick Quit", "Quick quit (context sensitive)", false);
    
    // Level Selection
    m_bindings[GameAction::LEVEL_1] = KeyBinding(SDL_SCANCODE_1, SDL_SCANCODE_UNKNOWN, "Level 1", "Load level 1");
    m_bindings[GameAction::LEVEL_2] = KeyBinding(SDL_SCANCODE_2, SDL_SCANCODE_UNKNOWN, "Level 2", "Load level 2");
    m_bindings[GameAction::LEVEL_3] = KeyBinding(SDL_SCANCODE_3, SDL_SCANCODE_UNKNOWN, "Level 3", "Load level 3");
    m_bindings[GameAction::LEVEL_BOSS] = KeyBinding(SDL_SCANCODE_B, SDL_SCANCODE_UNKNOWN, "Boss Level", "Load boss level");
    m_bindings[GameAction::LEVEL_RESET] = KeyBinding(SDL_SCANCODE_0, SDL_SCANCODE_UNKNOWN, "Reset Level", "Reset to base level");
    
    // Customization
    m_bindings[GameAction::CUSTOMIZE_CONFIRM] = KeyBinding(SDL_SCANCODE_C, SDL_SCANCODE_UNKNOWN, "Confirm Customization", "Confirm character customization");
    
    // Debug
    m_bindings[GameAction::DEBUG_TOGGLE] = KeyBinding(SDL_SCANCODE_F1, SDL_SCANCODE_UNKNOWN, "Debug Toggle", "Toggle debug mode", false);
}

void KeybindingManager::InitializeActionNames() {
    // Initialize action name mappings
    m_actionNames[GameAction::MOVE_LEFT] = "move_left";
    m_actionNames[GameAction::MOVE_RIGHT] = "move_right";
    m_actionNames[GameAction::MOVE_UP] = "move_up";
    m_actionNames[GameAction::MOVE_DOWN] = "move_down";

    m_actionNames[GameAction::MENU_UP] = "menu_up";
    m_actionNames[GameAction::MENU_DOWN] = "menu_down";
    m_actionNames[GameAction::MENU_LEFT] = "menu_left";
    m_actionNames[GameAction::MENU_RIGHT] = "menu_right";
    m_actionNames[GameAction::MENU_SELECT] = "menu_select";
    m_actionNames[GameAction::MENU_BACK] = "menu_back";
    m_actionNames[GameAction::MENU_CONFIRM] = "menu_confirm";

    m_actionNames[GameAction::PAUSE] = "pause";
    m_actionNames[GameAction::RETURN_TO_MENU] = "return_to_menu";
    m_actionNames[GameAction::RELOAD_CONFIG] = "reload_config";
    m_actionNames[GameAction::QUICK_QUIT] = "quick_quit";

    m_actionNames[GameAction::LEVEL_1] = "level_1";
    m_actionNames[GameAction::LEVEL_2] = "level_2";
    m_actionNames[GameAction::LEVEL_3] = "level_3";
    m_actionNames[GameAction::LEVEL_BOSS] = "level_boss";
    m_actionNames[GameAction::LEVEL_RESET] = "level_reset";

    m_actionNames[GameAction::CUSTOMIZE_CONFIRM] = "customize_confirm";
    m_actionNames[GameAction::DEBUG_TOGGLE] = "debug_toggle";

    // Initialize display names
    m_actionDisplayNames[GameAction::MOVE_LEFT] = "Move Left";
    m_actionDisplayNames[GameAction::MOVE_RIGHT] = "Move Right";
    m_actionDisplayNames[GameAction::MOVE_UP] = "Move Up";
    m_actionDisplayNames[GameAction::MOVE_DOWN] = "Move Down";

    m_actionDisplayNames[GameAction::MENU_UP] = "Menu Up";
    m_actionDisplayNames[GameAction::MENU_DOWN] = "Menu Down";
    m_actionDisplayNames[GameAction::MENU_LEFT] = "Menu Left";
    m_actionDisplayNames[GameAction::MENU_RIGHT] = "Menu Right";
    m_actionDisplayNames[GameAction::MENU_SELECT] = "Select";
    m_actionDisplayNames[GameAction::MENU_BACK] = "Back";
    m_actionDisplayNames[GameAction::MENU_CONFIRM] = "Confirm";

    m_actionDisplayNames[GameAction::PAUSE] = "Pause";
    m_actionDisplayNames[GameAction::RETURN_TO_MENU] = "Return to Menu";
    m_actionDisplayNames[GameAction::RELOAD_CONFIG] = "Reload Config";
    m_actionDisplayNames[GameAction::QUICK_QUIT] = "Quick Quit";

    m_actionDisplayNames[GameAction::LEVEL_1] = "Level 1";
    m_actionDisplayNames[GameAction::LEVEL_2] = "Level 2";
    m_actionDisplayNames[GameAction::LEVEL_3] = "Level 3";
    m_actionDisplayNames[GameAction::LEVEL_BOSS] = "Boss Level";
    m_actionDisplayNames[GameAction::LEVEL_RESET] = "Reset Level";

    m_actionDisplayNames[GameAction::CUSTOMIZE_CONFIRM] = "Confirm Customization";
    m_actionDisplayNames[GameAction::DEBUG_TOGGLE] = "Debug Toggle";

    // Create reverse mapping
    for (const auto& [action, name] : m_actionNames) {
        m_nameToAction[name] = action;
    }
}

std::string KeybindingManager::ScancodeToString(SDL_Scancode scancode) const {
    if (scancode == SDL_SCANCODE_UNKNOWN) return "";

    const char* name = SDL_GetScancodeName(scancode);
    return name ? std::string(name) : "";
}

SDL_Scancode KeybindingManager::StringToScancode(const std::string& str) const {
    if (str.empty()) return SDL_SCANCODE_UNKNOWN;

    return SDL_GetScancodeFromName(str.c_str());
}

bool KeybindingManager::IsKeyPressed(SDL_Scancode key, const InputManager* inputManager) const {
    if (key == SDL_SCANCODE_UNKNOWN || !inputManager) return false;
    return inputManager->IsKeyPressed(key);
}

bool KeybindingManager::IsKeyJustPressed(SDL_Scancode key, const InputManager* inputManager) const {
    if (key == SDL_SCANCODE_UNKNOWN || !inputManager) return false;
    return inputManager->IsKeyJustPressed(key);
}

bool KeybindingManager::IsKeyJustReleased(SDL_Scancode key, const InputManager* inputManager) const {
    if (key == SDL_SCANCODE_UNKNOWN || !inputManager) return false;
    return inputManager->IsKeyJustReleased(key);
}
