#include "Game/PartyManager.h"
#include "Game/PlayerCustomization.h"

PartyManager& PartyManager::Get() {
    static PartyManager instance;
    return instance;
}

void PartyManager::Clear() {
    m_members.clear();
}

void PartyManager::InitializeFromCustomization(const PlayerCustomization& pc) {
    if (!m_members.empty()) return; // don't overwrite if already set

    Member player;
    player.name = pc.playerName.empty() ? std::string("Hero") : pc.playerName;
    player.className = pc.characterClass.empty() ? std::string("warrior") : pc.characterClass;
    player.jobId = pc.characterClass; // using same for now
    player.level = 1;
    player.maxHp = 100;
    player.hp = player.maxHp;
    player.portraitPath = pc.spritePath; // may be empty; UI draws placeholder if so

    m_members.push_back(player);
}

void PartyManager::AddMember(const Member& m) {
    m_members.push_back(m);
}

