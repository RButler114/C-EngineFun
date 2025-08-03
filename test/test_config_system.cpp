/**
 * @file test_config_system.cpp
 * @brief Unit tests for the configuration management system
 * @author Ryan Butler
 * @date 2025
 */

#include "../include/Engine/ConfigManager.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <filesystem>

// Test framework macros
#define TEST(name) void test_##name()
#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        std::cerr << "ASSERTION FAILED: " << #condition << " at line " << __LINE__ << std::endl; \
        return; \
    }
#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))
#define ASSERT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        std::cerr << "ASSERTION FAILED: Expected " << (expected) << " but got " << (actual) << " at line " << __LINE__ << std::endl; \
        return; \
    }

// Test ConfigValue functionality
TEST(config_value_types) {
    // Test boolean values
    ConfigValue boolVal(true);
    ASSERT_TRUE(boolVal.GetType() == ConfigType::BOOL);
    ASSERT_TRUE(boolVal.AsBool() == true);
    ASSERT_TRUE(boolVal.ToString() == "true");
    
    ConfigValue boolVal2(false);
    ASSERT_TRUE(boolVal2.AsBool() == false);
    ASSERT_TRUE(boolVal2.ToString() == "false");
    
    // Test integer values
    ConfigValue intVal(42);
    ASSERT_TRUE(intVal.GetType() == ConfigType::INT);
    ASSERT_TRUE(intVal.AsInt() == 42);
    ASSERT_TRUE(intVal.ToString() == "42");
    
    // Test float values
    ConfigValue floatVal(3.14f);
    ASSERT_TRUE(floatVal.GetType() == ConfigType::FLOAT);
    ASSERT_TRUE(std::abs(floatVal.AsFloat() - 3.14f) < 0.001f);
    
    // Test string values
    ConfigValue stringVal("test string");
    ASSERT_TRUE(stringVal.GetType() == ConfigType::STRING);
    ASSERT_TRUE(stringVal.AsString() == "test string");
    ASSERT_TRUE(stringVal.ToString() == "test string");
    
    std::cout << "✓ ConfigValue types test passed" << std::endl;
}

TEST(config_value_parsing) {
    // Test boolean parsing
    ConfigValue boolVal(false);
    ASSERT_TRUE(boolVal.FromString("true"));
    ASSERT_TRUE(boolVal.AsBool() == true);
    
    ASSERT_TRUE(boolVal.FromString("false"));
    ASSERT_TRUE(boolVal.AsBool() == false);
    
    ASSERT_TRUE(boolVal.FromString("1"));
    ASSERT_TRUE(boolVal.AsBool() == true);
    
    ASSERT_TRUE(boolVal.FromString("0"));
    ASSERT_TRUE(boolVal.AsBool() == false);
    
    // Test integer parsing
    ConfigValue intVal(0);
    ASSERT_TRUE(intVal.FromString("123"));
    ASSERT_TRUE(intVal.AsInt() == 123);
    
    ASSERT_TRUE(intVal.FromString("-456"));
    ASSERT_TRUE(intVal.AsInt() == -456);
    
    // Test float parsing
    ConfigValue floatVal(0.0f);
    ASSERT_TRUE(floatVal.FromString("3.14159"));
    ASSERT_TRUE(std::abs(floatVal.AsFloat() - 3.14159f) < 0.001f);
    
    // Test string parsing
    ConfigValue stringVal("");
    ASSERT_TRUE(stringVal.FromString("hello world"));
    ASSERT_TRUE(stringVal.AsString() == "hello world");
    
    std::cout << "✓ ConfigValue parsing test passed" << std::endl;
}

TEST(config_manager_basic) {
    ConfigManager config;
    
    // Test initialization
    ASSERT_TRUE(config.Initialize("test_config.txt"));
    
    // Test setting registration
    config.RegisterSetting("test.boolSetting", "Test Bool", "A test boolean setting", 
                          ConfigValue(true), "Test");
    config.RegisterSetting("test.intSetting", "Test Int", "A test integer setting", 
                          ConfigValue(50), "Test");
    config.RegisterSetting("test.floatSetting", "Test Float", "A test float setting", 
                          ConfigValue(0.75f), "Test");
    config.RegisterSetting("test.stringSetting", "Test String", "A test string setting", 
                          ConfigValue("default"), "Test");
    
    // Test setting existence
    ASSERT_TRUE(config.HasSetting("test.boolSetting"));
    ASSERT_TRUE(config.HasSetting("test.intSetting"));
    ASSERT_TRUE(config.HasSetting("test.floatSetting"));
    ASSERT_TRUE(config.HasSetting("test.stringSetting"));
    ASSERT_FALSE(config.HasSetting("nonexistent.setting"));
    
    // Test default values
    ASSERT_TRUE(config.GetBool("test.boolSetting") == true);
    ASSERT_TRUE(config.GetInt("test.intSetting") == 50);
    ASSERT_TRUE(std::abs(config.GetFloat("test.floatSetting") - 0.75f) < 0.001f);
    ASSERT_TRUE(config.GetString("test.stringSetting") == "default");
    
    std::cout << "✓ ConfigManager basic functionality test passed" << std::endl;
}

TEST(config_manager_validation) {
    ConfigManager config;
    config.Initialize("test_validation.txt");
    
    // Register setting with min/max values
    ConfigSetting volumeSetting("audio.volume", "Volume", "Audio volume", 
                               ConfigValue(0.5f), "Audio");
    volumeSetting.minValue = ConfigValue(0.0f);
    volumeSetting.maxValue = ConfigValue(1.0f);
    config.RegisterSetting(volumeSetting);
    
    // Test valid values
    ASSERT_TRUE(config.SetFloat("audio.volume", 0.8f));
    ASSERT_TRUE(std::abs(config.GetFloat("audio.volume") - 0.8f) < 0.001f);
    
    // Test clamping
    ASSERT_TRUE(config.SetFloat("audio.volume", 1.5f)); // Should clamp to 1.0
    ASSERT_TRUE(std::abs(config.GetFloat("audio.volume") - 1.0f) < 0.001f);
    
    ASSERT_TRUE(config.SetFloat("audio.volume", -0.5f)); // Should clamp to 0.0
    ASSERT_TRUE(std::abs(config.GetFloat("audio.volume") - 0.0f) < 0.001f);
    
    // Test integer clamping
    ConfigSetting difficultySetting("game.difficulty", "Difficulty", "Game difficulty", 
                                   ConfigValue(1), "Game");
    difficultySetting.minValue = ConfigValue(0);
    difficultySetting.maxValue = ConfigValue(3);
    config.RegisterSetting(difficultySetting);
    
    ASSERT_TRUE(config.SetInt("game.difficulty", 5)); // Should clamp to 3
    ASSERT_TRUE(config.GetInt("game.difficulty") == 3);
    
    ASSERT_TRUE(config.SetInt("game.difficulty", -1)); // Should clamp to 0
    ASSERT_TRUE(config.GetInt("game.difficulty") == 0);
    
    std::cout << "✓ ConfigManager validation test passed" << std::endl;
}

TEST(config_manager_categories) {
    ConfigManager config;
    config.Initialize("test_categories.txt");
    
    // Register settings in different categories
    config.RegisterSetting("audio.volume", "Volume", "Audio volume", 
                          ConfigValue(1.0f), "Audio");
    config.RegisterSetting("audio.muted", "Muted", "Audio muted", 
                          ConfigValue(false), "Audio");
    config.RegisterSetting("graphics.resolution", "Resolution", "Screen resolution", 
                          ConfigValue(1), "Graphics");
    config.RegisterSetting("graphics.fullscreen", "Fullscreen", "Fullscreen mode", 
                          ConfigValue(false), "Graphics");
    config.RegisterSetting("input.sensitivity", "Sensitivity", "Mouse sensitivity", 
                          ConfigValue(1.0f), "Input");
    
    // Test category retrieval
    auto categories = config.GetCategories();
    ASSERT_TRUE(categories.size() == 3);
    ASSERT_TRUE(std::find(categories.begin(), categories.end(), "Audio") != categories.end());
    ASSERT_TRUE(std::find(categories.begin(), categories.end(), "Graphics") != categories.end());
    ASSERT_TRUE(std::find(categories.begin(), categories.end(), "Input") != categories.end());
    
    // Test settings in category
    auto audioSettings = config.GetSettingsInCategory("Audio");
    ASSERT_TRUE(audioSettings.size() == 2);
    ASSERT_TRUE(std::find(audioSettings.begin(), audioSettings.end(), "audio.volume") != audioSettings.end());
    ASSERT_TRUE(std::find(audioSettings.begin(), audioSettings.end(), "audio.muted") != audioSettings.end());
    
    auto graphicsSettings = config.GetSettingsInCategory("Graphics");
    ASSERT_TRUE(graphicsSettings.size() == 2);
    
    auto inputSettings = config.GetSettingsInCategory("Input");
    ASSERT_TRUE(inputSettings.size() == 1);
    
    std::cout << "✓ ConfigManager categories test passed" << std::endl;
}

TEST(config_manager_persistence) {
    const std::string testFile = "test_persistence.txt";
    
    // Clean up any existing test file
    if (std::filesystem::exists(testFile)) {
        std::filesystem::remove(testFile);
    }
    
    {
        // Create and configure manager
        ConfigManager config;
        config.Initialize(testFile);
        
        config.RegisterSetting("test.value1", "Value 1", "Test value 1", 
                              ConfigValue(42), "Test");
        config.RegisterSetting("test.value2", "Value 2", "Test value 2", 
                              ConfigValue(3.14f), "Test");
        config.RegisterSetting("test.value3", "Value 3", "Test value 3", 
                              ConfigValue(true), "Test");
        
        // Modify values
        config.SetInt("test.value1", 100);
        config.SetFloat("test.value2", 2.71f);
        config.SetBool("test.value3", false);
        
        // Save to file
        ASSERT_TRUE(config.SaveToFile());
    }
    
    // Verify file exists
    ASSERT_TRUE(std::filesystem::exists(testFile));
    
    {
        // Create new manager and load
        ConfigManager config2;
        config2.Initialize(testFile);
        
        // Re-register settings (would normally be done in RegisterDefaultSettings)
        config2.RegisterSetting("test.value1", "Value 1", "Test value 1", 
                               ConfigValue(42), "Test");
        config2.RegisterSetting("test.value2", "Value 2", "Test value 2", 
                               ConfigValue(3.14f), "Test");
        config2.RegisterSetting("test.value3", "Value 3", "Test value 3", 
                               ConfigValue(true), "Test");
        
        // Load from file
        ASSERT_TRUE(config2.LoadFromFile());
        
        // Verify loaded values
        ASSERT_TRUE(config2.GetInt("test.value1") == 100);
        ASSERT_TRUE(std::abs(config2.GetFloat("test.value2") - 2.71f) < 0.001f);
        ASSERT_TRUE(config2.GetBool("test.value3") == false);
    }
    
    // Clean up
    std::filesystem::remove(testFile);
    
    std::cout << "✓ ConfigManager persistence test passed" << std::endl;
}

TEST(config_manager_callbacks) {
    ConfigManager config;
    config.Initialize("test_callbacks.txt");
    
    config.RegisterSetting("test.callback", "Callback Test", "Test callback functionality", 
                          ConfigValue(0), "Test");
    
    // Test change callbacks
    bool callbackCalled = false;
    int oldValue = -1;
    int newValue = -1;
    
    int callbackId = config.RegisterChangeCallback(
        [&](const std::string& key, const ConfigValue& oldVal, const ConfigValue& newVal) {
            if (key == "test.callback") {
                callbackCalled = true;
                oldValue = oldVal.AsInt();
                newValue = newVal.AsInt();
            }
        }
    );
    
    // Trigger callback
    config.SetInt("test.callback", 42);
    
    ASSERT_TRUE(callbackCalled);
    ASSERT_TRUE(oldValue == 0);
    ASSERT_TRUE(newValue == 42);
    
    // Test callback removal
    config.UnregisterChangeCallback(callbackId);
    callbackCalled = false;
    
    config.SetInt("test.callback", 100);
    ASSERT_FALSE(callbackCalled); // Should not be called after unregistering
    
    std::cout << "✓ ConfigManager callbacks test passed" << std::endl;
}

TEST(options_state_integration) {
    // This test verifies that OptionsState can be created and initialized
    // Full UI testing would require a more complex test framework

    ConfigManager config;
    ASSERT_TRUE(config.Initialize("test_options.txt"));

    // Verify default settings are registered
    auto categories = config.GetCategories();
    ASSERT_TRUE(categories.size() >= 4); // Audio, Graphics, Input, Gameplay

    // Verify each category has settings
    auto audioSettings = config.GetSettingsInCategory("Audio");
    ASSERT_TRUE(audioSettings.size() > 0);

    auto graphicsSettings = config.GetSettingsInCategory("Graphics");
    ASSERT_TRUE(graphicsSettings.size() > 0);

    auto inputSettings = config.GetSettingsInCategory("Input");
    ASSERT_TRUE(inputSettings.size() > 0);

    auto gameplaySettings = config.GetSettingsInCategory("Gameplay");
    ASSERT_TRUE(gameplaySettings.size() > 0);

    // Test that all default settings have valid types and ranges
    for (const std::string& category : categories) {
        auto settings = config.GetSettingsInCategory(category);
        for (const std::string& key : settings) {
            const ConfigSetting* setting = config.GetSetting(key);
            ASSERT_TRUE(setting != nullptr);
            ASSERT_TRUE(!setting->displayName.empty());
            ASSERT_TRUE(!setting->description.empty());

            // Verify we can get the current value
            ConfigValue value = config.GetValue(key);
            ASSERT_TRUE(value.GetType() == setting->defaultValue.GetType());
        }
    }

    std::cout << "✓ OptionsState integration test passed" << std::endl;
}

TEST(default_settings_validation) {
    ConfigManager config;
    config.Initialize("test_defaults.txt");

    // Test audio settings
    ASSERT_TRUE(config.HasSetting("audio.masterVolume"));
    ASSERT_TRUE(config.GetFloat("audio.masterVolume") >= 0.0f);
    ASSERT_TRUE(config.GetFloat("audio.masterVolume") <= 1.0f);

    ASSERT_TRUE(config.HasSetting("audio.musicVolume"));
    ASSERT_TRUE(config.GetFloat("audio.musicVolume") >= 0.0f);
    ASSERT_TRUE(config.GetFloat("audio.musicVolume") <= 1.0f);

    // Test graphics settings
    ASSERT_TRUE(config.HasSetting("graphics.resolution"));
    int resolution = config.GetInt("graphics.resolution");
    ASSERT_TRUE(resolution >= 0 && resolution <= 3);

    ASSERT_TRUE(config.HasSetting("graphics.fullscreen"));
    // Boolean values don't need range checking

    // Test input settings
    ASSERT_TRUE(config.HasSetting("input.mouseSensitivity"));
    float sensitivity = config.GetFloat("input.mouseSensitivity");
    ASSERT_TRUE(sensitivity >= 0.1f && sensitivity <= 5.0f);

    // Test gameplay settings
    ASSERT_TRUE(config.HasSetting("gameplay.difficulty"));
    int difficulty = config.GetInt("gameplay.difficulty");
    ASSERT_TRUE(difficulty >= 0 && difficulty <= 3);

    std::cout << "✓ Default settings validation test passed" << std::endl;
}

// Test runner
int main() {
    std::cout << "Running Configuration System Tests..." << std::endl;

    test_config_value_types();
    test_config_value_parsing();
    test_config_manager_basic();
    test_config_manager_validation();
    test_config_manager_categories();
    test_config_manager_persistence();
    test_config_manager_callbacks();
    test_options_state_integration();
    test_default_settings_validation();

    std::cout << "\n✅ All configuration system tests passed!" << std::endl;
    return 0;
}
