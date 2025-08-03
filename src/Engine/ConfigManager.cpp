/**
 * @file ConfigManager.cpp
 * @brief Implementation of the ConfigManager class
 * @author Ryan Butler
 * @date 2025
 */

#include "Engine/ConfigManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

// ConfigValue implementation

// ConfigValue implementation
std::string ConfigValue::ToString() const {
    switch (m_type) {
        case ConfigType::BOOL:
            return m_boolValue ? "true" : "false";
        case ConfigType::INT:
            return std::to_string(m_intValue);
        case ConfigType::FLOAT:
            return std::to_string(m_floatValue);
        case ConfigType::STRING:
            return m_stringValue;
        case ConfigType::ENUM:
            return std::to_string(m_intValue);
        default:
            return "";
    }
}

bool ConfigValue::FromString(const std::string& str) {
    try {
        switch (m_type) {
            case ConfigType::BOOL:
                if (str == "true" || str == "1") {
                    m_boolValue = true;
                } else if (str == "false" || str == "0") {
                    m_boolValue = false;
                } else {
                    return false;
                }
                break;
            case ConfigType::INT:
            case ConfigType::ENUM:
                m_intValue = std::stoi(str);
                break;
            case ConfigType::FLOAT:
                m_floatValue = std::stof(str);
                break;
            case ConfigType::STRING:
                m_stringValue = str;
                break;
            default:
                return false;
        }
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

// ConfigManager implementation
ConfigManager::ConfigManager()
    : m_nextCallbackId(1)
    , m_requiresRestart(false)
    , m_initialized(false) {
}

ConfigManager::~ConfigManager() {
    Shutdown();
}

bool ConfigManager::Initialize(const std::string& configFile) {
    if (m_initialized) {
        return true;
    }

    m_configFile = configFile;
    
    // Register default settings
    RegisterDefaultSettings();
    
    // Load existing configuration
    LoadFromFile();
    
    m_initialized = true;
    std::cout << "ConfigManager initialized with file: " << configFile << std::endl;
    return true;
}

void ConfigManager::Shutdown() {
    if (m_initialized) {
        SaveToFile();
        m_initialized = false;
        std::cout << "ConfigManager shutdown" << std::endl;
    }
}

void ConfigManager::RegisterSetting(const ConfigSetting& setting) {
    m_settings[setting.key] = setting;
    
    // Set default value if not already set
    if (m_values.find(setting.key) == m_values.end()) {
        m_values[setting.key] = setting.defaultValue;
    }
}

void ConfigManager::RegisterSetting(const std::string& key, const std::string& displayName, 
                                   const std::string& description, const ConfigValue& defaultValue,
                                   const std::string& category, bool requiresRestart) {
    ConfigSetting setting(key, displayName, description, defaultValue, category, requiresRestart);
    RegisterSetting(setting);
}

bool ConfigManager::GetBool(const std::string& key) const {
    auto it = m_values.find(key);
    return (it != m_values.end()) ? it->second.AsBool() : false;
}

int ConfigManager::GetInt(const std::string& key) const {
    auto it = m_values.find(key);
    return (it != m_values.end()) ? it->second.AsInt() : 0;
}

float ConfigManager::GetFloat(const std::string& key) const {
    auto it = m_values.find(key);
    return (it != m_values.end()) ? it->second.AsFloat() : 0.0f;
}

const std::string& ConfigManager::GetString(const std::string& key) const {
    auto it = m_values.find(key);
    if (it != m_values.end()) {
        return it->second.AsString();
    } else {
        static const std::string emptyString = "";
        return emptyString;
    }
}

ConfigValue ConfigManager::GetValue(const std::string& key) const {
    auto it = m_values.find(key);
    return (it != m_values.end()) ? it->second : ConfigValue();
}

bool ConfigManager::SetBool(const std::string& key, bool value) {
    return SetValue(key, ConfigValue(value));
}

bool ConfigManager::SetInt(const std::string& key, int value) {
    return SetValue(key, ConfigValue(value));
}

bool ConfigManager::SetFloat(const std::string& key, float value) {
    return SetValue(key, ConfigValue(value));
}

bool ConfigManager::SetString(const std::string& key, const std::string& value) {
    return SetValue(key, ConfigValue(value));
}

bool ConfigManager::SetValue(const std::string& key, const ConfigValue& value) {
    auto settingIt = m_settings.find(key);
    if (settingIt == m_settings.end()) {
        std::cerr << "ConfigManager: Unknown setting key: " << key << std::endl;
        return false;
    }

    const ConfigSetting& setting = settingIt->second;
    
    // Validate and clamp value
    if (!ValidateValue(setting, value)) {
        std::cerr << "ConfigManager: Invalid value for setting: " << key << std::endl;
        return false;
    }

    ConfigValue clampedValue = ClampValue(setting, value);
    
    // Check if value actually changed
    auto valueIt = m_values.find(key);
    if (valueIt != m_values.end() && valueIt->second.ToString() == clampedValue.ToString()) {
        return true; // No change needed
    }

    ConfigValue oldValue = (valueIt != m_values.end()) ? valueIt->second : setting.defaultValue;
    m_values[key] = clampedValue;

    // Check if restart is required
    if (setting.requiresRestart) {
        m_requiresRestart = true;
    }

    // Notify callbacks
    NotifyChange(key, oldValue, clampedValue);

    return true;
}

std::vector<std::string> ConfigManager::GetSettingsInCategory(const std::string& category) const {
    std::vector<std::string> result;
    for (const auto& pair : m_settings) {
        if (pair.second.category == category) {
            result.push_back(pair.first);
        }
    }
    return result;
}

std::vector<std::string> ConfigManager::GetCategories() const {
    std::vector<std::string> categories;
    for (const auto& pair : m_settings) {
        const std::string& category = pair.second.category;
        if (std::find(categories.begin(), categories.end(), category) == categories.end()) {
            categories.push_back(category);
        }
    }
    std::sort(categories.begin(), categories.end());
    return categories;
}

const ConfigSetting* ConfigManager::GetSetting(const std::string& key) const {
    auto it = m_settings.find(key);
    return (it != m_settings.end()) ? &it->second : nullptr;
}

bool ConfigManager::HasSetting(const std::string& key) const {
    return m_settings.find(key) != m_settings.end();
}

bool ConfigManager::ResetToDefault(const std::string& key) {
    auto it = m_settings.find(key);
    if (it == m_settings.end()) {
        return false;
    }
    
    return SetValue(key, it->second.defaultValue);
}

void ConfigManager::ResetAllToDefaults() {
    for (const auto& pair : m_settings) {
        SetValue(pair.first, pair.second.defaultValue);
    }
}

int ConfigManager::RegisterChangeCallback(const ChangeCallback& callback) {
    int id = m_nextCallbackId++;
    m_changeCallbacks[id] = callback;
    return id;
}

void ConfigManager::UnregisterChangeCallback(int callbackId) {
    m_changeCallbacks.erase(callbackId);
}

bool ConfigManager::ValidateValue(const ConfigSetting& setting, const ConfigValue& value) const {
    // Type check
    if (value.GetType() != setting.defaultValue.GetType()) {
        return false;
    }

    // Custom validator
    if (setting.validator && !setting.validator(value)) {
        return false;
    }

    return true;
}

ConfigValue ConfigManager::ClampValue(const ConfigSetting& setting, const ConfigValue& value) const {
    ConfigValue result = value;
    
    switch (value.GetType()) {
        case ConfigType::INT:
        case ConfigType::ENUM: {
            int val = value.AsInt();
            if (setting.minValue.GetType() == ConfigType::INT) {
                val = std::max(val, setting.minValue.AsInt());
            }
            if (setting.maxValue.GetType() == ConfigType::INT) {
                val = std::min(val, setting.maxValue.AsInt());
            }
            result = ConfigValue(val);
            break;
        }
        case ConfigType::FLOAT: {
            float val = value.AsFloat();
            if (setting.minValue.GetType() == ConfigType::FLOAT) {
                val = std::max(val, setting.minValue.AsFloat());
            }
            if (setting.maxValue.GetType() == ConfigType::FLOAT) {
                val = std::min(val, setting.maxValue.AsFloat());
            }
            result = ConfigValue(val);
            break;
        }
        default:
            // No clamping for bool/string types
            break;
    }
    
    return result;
}

void ConfigManager::NotifyChange(const std::string& key, const ConfigValue& oldValue, const ConfigValue& newValue) {
    for (const auto& pair : m_changeCallbacks) {
        try {
            pair.second(key, oldValue, newValue);
        } catch (const std::exception& e) {
            std::cerr << "ConfigManager: Exception in change callback: " << e.what() << std::endl;
        }
    }
}

bool ConfigManager::LoadFromFile() {
    std::ifstream file(m_configFile);
    if (!file.is_open()) {
        std::cout << "ConfigManager: Config file not found, using defaults: " << m_configFile << std::endl;
        return true; // Not an error, just use defaults
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Parse key=value format
        size_t equalPos = line.find('=');
        if (equalPos == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, equalPos);
        std::string valueStr = line.substr(equalPos + 1);

        // Trim whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        valueStr.erase(0, valueStr.find_first_not_of(" \t"));
        valueStr.erase(valueStr.find_last_not_of(" \t") + 1);

        // Set value if setting exists
        auto settingIt = m_settings.find(key);
        if (settingIt != m_settings.end()) {
            ConfigValue value = settingIt->second.defaultValue; // Start with correct type
            if (value.FromString(valueStr)) {
                m_values[key] = value;
            } else {
                std::cerr << "ConfigManager: Failed to parse value for " << key << ": " << valueStr << std::endl;
            }
        }
    }

    file.close();
    std::cout << "ConfigManager: Loaded configuration from " << m_configFile << std::endl;
    return true;
}

bool ConfigManager::SaveToFile() {
    std::ofstream file(m_configFile);
    if (!file.is_open()) {
        std::cerr << "ConfigManager: Failed to open config file for writing: " << m_configFile << std::endl;
        return false;
    }

    file << "# Game Configuration File\n";
    file << "# Generated automatically - modify with caution\n\n";

    // Group settings by category
    auto categories = GetCategories();
    for (const std::string& category : categories) {
        file << "# " << category << " Settings\n";

        auto settingsInCategory = GetSettingsInCategory(category);
        for (const std::string& key : settingsInCategory) {
            const ConfigSetting* setting = GetSetting(key);
            if (setting) {
                file << "# " << setting->description << "\n";
                file << key << "=" << GetValue(key).ToString() << "\n";
            }
        }
        file << "\n";
    }

    file.close();
    std::cout << "ConfigManager: Saved configuration to " << m_configFile << std::endl;
    return true;
}

void ConfigManager::RegisterDefaultSettings() {
    // Audio Settings
    RegisterSetting("audio.masterVolume", "Master Volume", "Overall audio volume (0.0 to 1.0)",
                   ConfigValue(1.0f), "Audio");
    RegisterSetting("audio.musicVolume", "Music Volume", "Background music volume (0.0 to 1.0)",
                   ConfigValue(0.7f), "Audio");
    RegisterSetting("audio.sfxVolume", "Sound Effects Volume", "Sound effects volume (0.0 to 1.0)",
                   ConfigValue(0.8f), "Audio");
    RegisterSetting("audio.voiceVolume", "Voice Volume", "Voice and dialogue volume (0.0 to 1.0)",
                   ConfigValue(1.0f), "Audio");
    RegisterSetting("audio.ambientVolume", "Ambient Volume", "Ambient sound volume (0.0 to 1.0)",
                   ConfigValue(0.6f), "Audio");
    RegisterSetting("audio.muted", "Mute Audio", "Mute all audio",
                   ConfigValue(false), "Audio");

    // Graphics Settings
    RegisterSetting("graphics.resolution", "Resolution", "Screen resolution (0=800x600, 1=1024x768, 2=1280x720, 3=1920x1080)",
                   ConfigValue(2), "Graphics", true);
    RegisterSetting("graphics.fullscreen", "Fullscreen", "Enable fullscreen mode",
                   ConfigValue(false), "Graphics", true);
    RegisterSetting("graphics.vsync", "VSync", "Enable vertical synchronization",
                   ConfigValue(true), "Graphics", true);
    RegisterSetting("graphics.frameRateLimit", "Frame Rate Limit", "Maximum frame rate (0=unlimited, 30, 60, 120, 144)",
                   ConfigValue(60), "Graphics");
    RegisterSetting("graphics.showFPS", "Show FPS", "Display frame rate counter",
                   ConfigValue(false), "Graphics");

    // Input Settings
    RegisterSetting("input.mouseSensitivity", "Mouse Sensitivity", "Mouse sensitivity multiplier (0.1 to 5.0)",
                   ConfigValue(1.0f), "Input");
    RegisterSetting("input.invertMouseY", "Invert Mouse Y", "Invert vertical mouse movement",
                   ConfigValue(false), "Input");
    RegisterSetting("input.keyRepeatDelay", "Key Repeat Delay", "Delay before key repeat starts (ms)",
                   ConfigValue(500), "Input");
    RegisterSetting("input.keyRepeatRate", "Key Repeat Rate", "Key repeat rate (keys per second)",
                   ConfigValue(30), "Input");

    // Gameplay Settings
    RegisterSetting("gameplay.difficulty", "Difficulty", "Game difficulty (0=Easy, 1=Normal, 2=Hard, 3=Expert)",
                   ConfigValue(1), "Gameplay");
    RegisterSetting("gameplay.autoSave", "Auto Save", "Enable automatic saving",
                   ConfigValue(true), "Gameplay");
    RegisterSetting("gameplay.autoSaveInterval", "Auto Save Interval", "Auto save interval in minutes",
                   ConfigValue(5), "Gameplay");
    RegisterSetting("gameplay.showTutorials", "Show Tutorials", "Display tutorial messages",
                   ConfigValue(true), "Gameplay");
    RegisterSetting("gameplay.pauseOnFocusLoss", "Pause on Focus Loss", "Pause game when window loses focus",
                   ConfigValue(true), "Gameplay");

    // Set min/max values for numeric settings
    auto* masterVolume = const_cast<ConfigSetting*>(GetSetting("audio.masterVolume"));
    if (masterVolume) {
        masterVolume->minValue = ConfigValue(0.0f);
        masterVolume->maxValue = ConfigValue(1.0f);
    }

    auto* musicVolume = const_cast<ConfigSetting*>(GetSetting("audio.musicVolume"));
    if (musicVolume) {
        musicVolume->minValue = ConfigValue(0.0f);
        musicVolume->maxValue = ConfigValue(1.0f);
    }

    auto* sfxVolume = const_cast<ConfigSetting*>(GetSetting("audio.sfxVolume"));
    if (sfxVolume) {
        sfxVolume->minValue = ConfigValue(0.0f);
        sfxVolume->maxValue = ConfigValue(1.0f);
    }

    auto* mouseSensitivity = const_cast<ConfigSetting*>(GetSetting("input.mouseSensitivity"));
    if (mouseSensitivity) {
        mouseSensitivity->minValue = ConfigValue(0.1f);
        mouseSensitivity->maxValue = ConfigValue(5.0f);
    }

    auto* difficulty = const_cast<ConfigSetting*>(GetSetting("gameplay.difficulty"));
    if (difficulty) {
        difficulty->minValue = ConfigValue(0);
        difficulty->maxValue = ConfigValue(3);
        difficulty->enumOptions = {"Easy", "Normal", "Hard", "Expert"};
    }

    auto* resolution = const_cast<ConfigSetting*>(GetSetting("graphics.resolution"));
    if (resolution) {
        resolution->minValue = ConfigValue(0);
        resolution->maxValue = ConfigValue(3);
        resolution->enumOptions = {"800x600", "1024x768", "1280x720", "1920x1080"};
    }
}
