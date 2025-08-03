/**
 * @file test_options_compilation.cpp
 * @brief Compilation test for options system without SDL2 dependencies
 * @author Ryan Butler
 * @date 2025
 */

#include "../include/Engine/ConfigManager.h"
#include <iostream>
#include <cassert>

// Simple test to verify the options system compiles and basic functionality works
int main() {
    std::cout << "Testing Options System Compilation..." << std::endl;
    
    try {
        // Test ConfigValue creation and type safety
        ConfigValue boolVal(true);
        ConfigValue intVal(42);
        ConfigValue floatVal(3.14f);
        ConfigValue stringVal(std::string("test"));
        
        assert(boolVal.GetType() == ConfigType::BOOL);
        assert(intVal.GetType() == ConfigType::INT);
        assert(floatVal.GetType() == ConfigType::FLOAT);
        assert(stringVal.GetType() == ConfigType::STRING);
        
        assert(boolVal.AsBool() == true);
        assert(intVal.AsInt() == 42);
        assert(std::abs(floatVal.AsFloat() - 3.14f) < 0.001f);
        assert(stringVal.AsString() == "test");
        
        std::cout << "✓ ConfigValue creation and type safety works" << std::endl;
        
        // Test ConfigManager creation and basic operations
        ConfigManager config;
        assert(config.Initialize("test_compilation.txt"));
        
        // Test setting registration
        config.RegisterSetting("test.bool", "Test Bool", "A test boolean", 
                              ConfigValue(false), "Test");
        config.RegisterSetting("test.int", "Test Int", "A test integer", 
                              ConfigValue(100), "Test");
        config.RegisterSetting("test.float", "Test Float", "A test float", 
                              ConfigValue(0.5f), "Test");
        
        assert(config.HasSetting("test.bool"));
        assert(config.HasSetting("test.int"));
        assert(config.HasSetting("test.float"));
        
        std::cout << "✓ ConfigManager registration works" << std::endl;
        
        // Test value getting and setting
        assert(config.GetBool("test.bool") == false);
        assert(config.SetBool("test.bool", true));
        assert(config.GetBool("test.bool") == true);
        
        assert(config.GetInt("test.int") == 100);
        assert(config.SetInt("test.int", 200));
        assert(config.GetInt("test.int") == 200);
        
        assert(std::abs(config.GetFloat("test.float") - 0.5f) < 0.001f);
        assert(config.SetFloat("test.float", 0.8f));
        assert(std::abs(config.GetFloat("test.float") - 0.8f) < 0.001f);
        
        std::cout << "✓ ConfigManager get/set operations work" << std::endl;
        
        // Test categories
        auto categories = config.GetCategories();
        assert(categories.size() >= 4); // Should have Audio, Graphics, Input, Gameplay + Test
        
        auto testSettings = config.GetSettingsInCategory("Test");
        assert(testSettings.size() == 3);
        
        std::cout << "✓ ConfigManager categories work" << std::endl;
        
        // Test default settings registration
        auto audioSettings = config.GetSettingsInCategory("Audio");
        assert(audioSettings.size() > 0);
        assert(config.HasSetting("audio.masterVolume"));
        assert(config.HasSetting("audio.musicVolume"));
        
        auto graphicsSettings = config.GetSettingsInCategory("Graphics");
        assert(graphicsSettings.size() > 0);
        assert(config.HasSetting("graphics.resolution"));
        assert(config.HasSetting("graphics.fullscreen"));
        
        std::cout << "✓ Default settings registration works" << std::endl;
        
        // Test validation and clamping
        ConfigSetting volumeSetting("test.volume", "Volume", "Test volume", 
                                   ConfigValue(0.5f), "Test");
        volumeSetting.minValue = ConfigValue(0.0f);
        volumeSetting.maxValue = ConfigValue(1.0f);
        config.RegisterSetting(volumeSetting);
        
        // Test clamping
        assert(config.SetFloat("test.volume", 1.5f)); // Should clamp to 1.0
        assert(std::abs(config.GetFloat("test.volume") - 1.0f) < 0.001f);
        
        assert(config.SetFloat("test.volume", -0.5f)); // Should clamp to 0.0
        assert(std::abs(config.GetFloat("test.volume") - 0.0f) < 0.001f);
        
        std::cout << "✓ Value validation and clamping works" << std::endl;
        
        // Test change callbacks
        bool callbackTriggered = false;
        int callbackId = config.RegisterChangeCallback(
            [&](const std::string& key, const ConfigValue& oldVal, const ConfigValue& newVal) {
                if (key == "test.callback") {
                    callbackTriggered = true;
                }
            }
        );
        
        config.RegisterSetting("test.callback", "Callback Test", "Test callbacks", 
                              ConfigValue(0), "Test");
        config.SetInt("test.callback", 42);
        assert(callbackTriggered);
        
        config.UnregisterChangeCallback(callbackId);
        std::cout << "✓ Change callbacks work" << std::endl;
        
        // Test file operations (basic)
        assert(config.SaveToFile());
        std::cout << "✓ File save works" << std::endl;
        
        // Test reset functionality
        config.SetFloat("audio.masterVolume", 0.3f);
        assert(std::abs(config.GetFloat("audio.masterVolume") - 0.3f) < 0.001f);
        
        assert(config.ResetToDefault("audio.masterVolume"));
        assert(std::abs(config.GetFloat("audio.masterVolume") - 1.0f) < 0.001f); // Default is 1.0
        
        std::cout << "✓ Reset to defaults works" << std::endl;
        
        std::cout << "\n✅ All options system compilation tests passed!" << std::endl;
        std::cout << "The options system is ready for integration with the game engine." << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
}
