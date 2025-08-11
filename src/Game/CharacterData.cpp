#include "Game/CharacterData.h"
#include "Engine/ConfigSystem.h"

CharacterDataRegistry& CharacterDataRegistry::Get() {
    static CharacterDataRegistry inst;
    return inst;
}

void CharacterDataRegistry::EnsureLoaded(const std::string& configPath) {
    if (m_loaded) return;
    LoadFromConfig(configPath);
    m_loaded = true;
}

void CharacterDataRegistry::LoadFromConfig(const std::string& configPath) {
    ConfigManager cfg;
    if (!cfg.LoadFromFile(configPath)) {
        return; // keep defaults
    }
    for (const auto& [sectionName, section] : cfg.GetSections()) {
        // filter out non-character sections
        if (sectionName == "balance" || sectionName == "world" || sectionName == "audio" || sectionName == "graphics" || sectionName == "default")
            continue;
        CharacterStatsData data;
        data.maxHealth = cfg.Get(sectionName, "max_health", 100.0f).AsFloat();
        data.armor = cfg.Get(sectionName, "armor", 0.0f).AsFloat();
        data.strength = cfg.Get(sectionName, "strength", 10.0f).AsFloat();
        data.agility = cfg.Get(sectionName, "agility", 10.0f).AsFloat();
        data.intelligence = cfg.Get(sectionName, "intelligence", 10.0f).AsFloat();
        data.vitality = cfg.Get(sectionName, "vitality", 10.0f).AsFloat();
        m_statsByTemplate[sectionName] = data;
    }
}

CharacterStatsData CharacterDataRegistry::GetStats(const std::string& templateName) const {
    auto it = m_statsByTemplate.find(templateName);
    if (it != m_statsByTemplate.end()) return it->second;
    // fallback
    auto itPlayer = m_statsByTemplate.find("player");
    if (itPlayer != m_statsByTemplate.end()) return itPlayer->second;
    return CharacterStatsData{};
}

