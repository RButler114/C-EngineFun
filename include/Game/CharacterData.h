#pragma once

#include <string>
#include <unordered_map>

struct CharacterStatsData {
    float maxHealth = 100.0f;
    float armor = 0.0f;
    float strength = 10.0f;
    float agility = 10.0f;
    float intelligence = 10.0f;
    float vitality = 10.0f;
};

class CharacterDataRegistry {
public:
    static CharacterDataRegistry& Get();

    // Ensure data is loaded from given config path; no-op if already loaded
    void EnsureLoaded(const std::string& configPath = "assets/config/characters.ini");

    // Get stats for a character template name (e.g., "player", "goblin").
    // Falls back to "player" if not found, and then to defaults.
    CharacterStatsData GetStats(const std::string& templateName) const;

private:
    CharacterDataRegistry() = default;
    void LoadFromConfig(const std::string& configPath);

    bool m_loaded = false;
    std::unordered_map<std::string, CharacterStatsData> m_statsByTemplate;
};

