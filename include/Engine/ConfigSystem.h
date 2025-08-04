/**
 * @file ConfigSystem.h
 * @brief Simple configuration system for data-driven game design
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>



/**
 * @class ConfigValue
 * @brief Represents a configuration value that can be different types
 */
class ConfigValue {
public:
    ConfigValue() = default;
    ConfigValue(const std::string& value) : m_value(value) {}
    ConfigValue(float value) : m_value(std::to_string(value)) {}
    ConfigValue(int value) : m_value(std::to_string(value)) {}
    ConfigValue(bool value) : m_value(value ? "true" : "false") {}
    
    // Conversion operators
    operator std::string() const { return m_value; }
    operator float() const { 
        try { return std::stof(m_value); } 
        catch (...) { return 0.0f; }
    }
    operator int() const { 
        try { return std::stoi(m_value); } 
        catch (...) { return 0; }
    }
    operator bool() const { 
        return m_value == "true" || m_value == "1" || m_value == "yes";
    }
    
    const std::string& AsString() const { return m_value; }
    float AsFloat() const { return *this; }
    int AsInt() const { return *this; }
    bool AsBool() const { return *this; }

private:
    std::string m_value;
};

/**
 * @class ConfigSection
 * @brief Represents a section of configuration values
 */
class ConfigSection {
public:
    void Set(const std::string& key, const ConfigValue& value) {
        m_values[key] = value;
    }
    
    ConfigValue Get(const std::string& key, const ConfigValue& defaultValue = ConfigValue("")) const {
        auto it = m_values.find(key);
        return (it != m_values.end()) ? it->second : defaultValue;
    }
    
    bool Has(const std::string& key) const {
        return m_values.find(key) != m_values.end();
    }
    
    const std::unordered_map<std::string, ConfigValue>& GetAll() const {
        return m_values;
    }

private:
    std::unordered_map<std::string, ConfigValue> m_values;
};

/**
 * @class ConfigManager
 * @brief Manages configuration files and provides easy access to config values
 *
 * Supports simple INI-style configuration files:
 * [section]
 * key=value
 * key2=value2
 */
class ConfigManager {
public:
    ConfigManager() = default;
    
    /**
     * @brief Load configuration from a file
     * @param filename Path to the configuration file
     * @return true if loaded successfully
     */
    bool LoadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file: " << filename << std::endl;
            return false;
        }
        
        std::string line;
        std::string currentSection = "default";
        
        while (std::getline(file, line)) {
            // Remove whitespace
            line = Trim(line);
            
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#' || line[0] == ';') {
                continue;
            }
            
            // Check for section header
            if (line[0] == '[' && line.back() == ']') {
                currentSection = line.substr(1, line.length() - 2);
                continue;
            }
            
            // Parse key=value pairs
            size_t equalPos = line.find('=');
            if (equalPos != std::string::npos) {
                std::string key = Trim(line.substr(0, equalPos));
                std::string value = Trim(line.substr(equalPos + 1));
                
                m_sections[currentSection].Set(key, ConfigValue(value));
            }
        }
        
        file.close();
        return true;
    }
    
    /**
     * @brief Save configuration to a file
     * @param filename Path to save the configuration file
     * @return true if saved successfully
     */
    bool SaveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to create config file: " << filename << std::endl;
            return false;
        }
        
        for (const auto& [sectionName, section] : m_sections) {
            if (sectionName != "default") {
                file << "[" << sectionName << "]\n";
            }
            
            for (const auto& [key, value] : section.GetAll()) {
                file << key << "=" << value.AsString() << "\n";
            }
            
            file << "\n";
        }
        
        file.close();
        return true;
    }
    
    /**
     * @brief Get a configuration value
     * @param section Section name
     * @param key Key name
     * @param defaultValue Default value if not found
     * @return Configuration value
     */
    ConfigValue Get(const std::string& section, const std::string& key, 
                   const ConfigValue& defaultValue = ConfigValue("")) const {
        auto sectionIt = m_sections.find(section);
        if (sectionIt != m_sections.end()) {
            return sectionIt->second.Get(key, defaultValue);
        }
        return defaultValue;
    }
    
    /**
     * @brief Set a configuration value
     * @param section Section name
     * @param key Key name
     * @param value Value to set
     */
    void Set(const std::string& section, const std::string& key, const ConfigValue& value) {
        m_sections[section].Set(key, value);
    }
    
    /**
     * @brief Check if a section exists
     */
    bool HasSection(const std::string& section) const {
        return m_sections.find(section) != m_sections.end();
    }
    
    /**
     * @brief Check if a key exists in a section
     */
    bool Has(const std::string& section, const std::string& key) const {
        auto sectionIt = m_sections.find(section);
        return sectionIt != m_sections.end() && sectionIt->second.Has(key);
    }
    
    /**
     * @brief Get all sections
     */
    const std::unordered_map<std::string, ConfigSection>& GetSections() const {
        return m_sections;
    }

private:
    std::unordered_map<std::string, ConfigSection> m_sections;
    
    std::string Trim(const std::string& str) const {
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        
        size_t end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }
};


