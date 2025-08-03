/**
 * @file ConfigManager.h
 * @brief Configuration management system for game settings
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>

/**
 * @brief Supported configuration value types
 */
enum class ConfigType {
    BOOL,       ///< Boolean value (true/false)
    INT,        ///< Integer value
    FLOAT,      ///< Floating point value
    STRING,     ///< String value
    ENUM        ///< Enumerated value (stored as int with string mapping)
};

/**
 * @brief Configuration value container with type safety
 */
class ConfigValue {
public:
    ConfigValue() : m_type(ConfigType::BOOL), m_boolValue(false) {}
    explicit ConfigValue(bool value) : m_type(ConfigType::BOOL), m_boolValue(value) {}
    explicit ConfigValue(int value) : m_type(ConfigType::INT), m_intValue(value) {}
    explicit ConfigValue(float value) : m_type(ConfigType::FLOAT), m_floatValue(value) {}
    explicit ConfigValue(const std::string& value) : m_type(ConfigType::STRING), m_stringValue(value) {}
    
    // Type-safe getters
    bool AsBool() const { return m_type == ConfigType::BOOL ? m_boolValue : false; }
    int AsInt() const { return m_type == ConfigType::INT ? m_intValue : 0; }
    float AsFloat() const { return m_type == ConfigType::FLOAT ? m_floatValue : 0.0f; }
    const std::string& AsString() const {
        if (m_type == ConfigType::STRING) {
            return m_stringValue;
        } else {
            static const std::string emptyString = "";
            return emptyString;
        }
    }
    
    ConfigType GetType() const { return m_type; }
    std::string ToString() const;
    bool FromString(const std::string& str);

private:
    ConfigType m_type;
    union {
        bool m_boolValue;
        int m_intValue;
        float m_floatValue;
    };
    std::string m_stringValue;
};

/**
 * @brief Configuration setting definition with validation and metadata
 */
struct ConfigSetting {
    std::string key;                                    ///< Setting identifier
    std::string displayName;                           ///< Human-readable name
    std::string description;                           ///< Setting description
    ConfigValue defaultValue;                          ///< Default value
    ConfigValue minValue;                              ///< Minimum value (for numeric types)
    ConfigValue maxValue;                              ///< Maximum value (for numeric types)
    std::vector<std::string> enumOptions;              ///< Options for enum types
    std::function<bool(const ConfigValue&)> validator; ///< Custom validation function
    std::string category;                              ///< Settings category
    bool requiresRestart;                              ///< Whether changing this setting requires restart

    // Default constructor for container usage
    ConfigSetting() : defaultValue(false), requiresRestart(false) {}

    ConfigSetting(const std::string& k, const std::string& display, const std::string& desc,
                  const ConfigValue& defaultVal, const std::string& cat = "General", bool restart = false)
        : key(k), displayName(display), description(desc), defaultValue(defaultVal),
          category(cat), requiresRestart(restart) {}
};

/**
 * @brief Comprehensive configuration management system
 * 
 * The ConfigManager provides a centralized system for managing game settings with:
 * - Type-safe configuration values
 * - Automatic validation and clamping
 * - File-based persistence (JSON format)
 * - Change notification system
 * - Category-based organization
 * - Default value management
 * 
 * @example
 * ```cpp
 * ConfigManager config;
 * config.Initialize("config.json");
 * 
 * // Register settings
 * config.RegisterSetting("audio.masterVolume", "Master Volume", "Overall audio volume", 
 *                        ConfigValue(1.0f), "Audio");
 * 
 * // Use settings
 * float volume = config.GetFloat("audio.masterVolume");
 * config.SetFloat("audio.masterVolume", 0.8f);
 * 
 * // Save changes
 * config.SaveToFile();
 * ```
 */
class ConfigManager {
public:
    /**
     * @brief Change notification callback type
     * @param key Setting key that changed
     * @param oldValue Previous value
     * @param newValue New value
     */
    using ChangeCallback = std::function<void(const std::string& key, const ConfigValue& oldValue, const ConfigValue& newValue)>;

    ConfigManager();
    ~ConfigManager();

    /**
     * @brief Initialize the configuration manager
     * @param configFile Path to configuration file
     * @return true if initialization successful
     */
    bool Initialize(const std::string& configFile = "config.json");

    /**
     * @brief Shutdown and save configuration
     */
    void Shutdown();

    /**
     * @brief Register a configuration setting
     * @param setting Setting definition
     */
    void RegisterSetting(const ConfigSetting& setting);

    /**
     * @brief Register a simple setting with default value
     * @param key Setting key
     * @param displayName Human-readable name
     * @param description Setting description
     * @param defaultValue Default value
     * @param category Settings category
     * @param requiresRestart Whether changing requires restart
     */
    void RegisterSetting(const std::string& key, const std::string& displayName, 
                        const std::string& description, const ConfigValue& defaultValue,
                        const std::string& category = "General", bool requiresRestart = false);

    // Type-safe getters
    bool GetBool(const std::string& key) const;
    int GetInt(const std::string& key) const;
    float GetFloat(const std::string& key) const;
    const std::string& GetString(const std::string& key) const;
    ConfigValue GetValue(const std::string& key) const;

    // Type-safe setters
    bool SetBool(const std::string& key, bool value);
    bool SetInt(const std::string& key, int value);
    bool SetFloat(const std::string& key, float value);
    bool SetString(const std::string& key, const std::string& value);
    bool SetValue(const std::string& key, const ConfigValue& value);

    /**
     * @brief Get all settings in a category
     * @param category Category name
     * @return Vector of setting keys in the category
     */
    std::vector<std::string> GetSettingsInCategory(const std::string& category) const;

    /**
     * @brief Get all available categories
     * @return Vector of category names
     */
    std::vector<std::string> GetCategories() const;

    /**
     * @brief Get setting definition
     * @param key Setting key
     * @return Pointer to setting definition, nullptr if not found
     */
    const ConfigSetting* GetSetting(const std::string& key) const;

    /**
     * @brief Check if a setting exists
     * @param key Setting key
     * @return true if setting exists
     */
    bool HasSetting(const std::string& key) const;

    /**
     * @brief Reset setting to default value
     * @param key Setting key
     * @return true if reset successful
     */
    bool ResetToDefault(const std::string& key);

    /**
     * @brief Reset all settings to default values
     */
    void ResetAllToDefaults();

    /**
     * @brief Load configuration from file
     * @return true if load successful
     */
    bool LoadFromFile();

    /**
     * @brief Save configuration to file
     * @return true if save successful
     */
    bool SaveToFile();

    /**
     * @brief Register change callback
     * @param callback Function to call when settings change
     * @return Callback ID for unregistering
     */
    int RegisterChangeCallback(const ChangeCallback& callback);

    /**
     * @brief Unregister change callback
     * @param callbackId ID returned from RegisterChangeCallback
     */
    void UnregisterChangeCallback(int callbackId);

    /**
     * @brief Check if any settings require restart
     * @return true if restart is needed
     */
    bool RequiresRestart() const { return m_requiresRestart; }

    /**
     * @brief Clear restart requirement flag
     */
    void ClearRestartFlag() { m_requiresRestart = false; }

private:
    std::string m_configFile;                                           ///< Configuration file path
    std::unordered_map<std::string, ConfigSetting> m_settings;         ///< Registered settings
    std::unordered_map<std::string, ConfigValue> m_values;             ///< Current values
    std::unordered_map<int, ChangeCallback> m_changeCallbacks;         ///< Change notification callbacks
    int m_nextCallbackId;                                               ///< Next callback ID
    bool m_requiresRestart;                                             ///< Whether restart is required
    bool m_initialized;                                                 ///< Initialization state

    /**
     * @brief Validate a configuration value
     * @param setting Setting definition
     * @param value Value to validate
     * @return true if value is valid
     */
    bool ValidateValue(const ConfigSetting& setting, const ConfigValue& value) const;

    /**
     * @brief Clamp numeric value to valid range
     * @param setting Setting definition
     * @param value Value to clamp
     * @return Clamped value
     */
    ConfigValue ClampValue(const ConfigSetting& setting, const ConfigValue& value) const;

    /**
     * @brief Notify change callbacks
     * @param key Setting key
     * @param oldValue Previous value
     * @param newValue New value
     */
    void NotifyChange(const std::string& key, const ConfigValue& oldValue, const ConfigValue& newValue);

    /**
     * @brief Register default game settings
     */
    void RegisterDefaultSettings();
};
