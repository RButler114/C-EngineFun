#include "Game/PartyManager.h"
#include "Game/CharacterData.h"

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
    // Avoid generic fallback; use jobId if present, else 'player' template
    player.className = pc.characterClass.empty() ? std::string("player") : pc.characterClass;
    player.jobId = pc.characterClass; // jobId == class selection
    player.level = 1;
    player.maxHp = 100;
    player.hp = player.maxHp;
    player.maxMp = 30;
    player.mp = player.maxMp;
    player.portraitPath = pc.spritePath; // may be empty; UI draws placeholder if so
    // Initialize HP/MP from character data baselines
    CharacterStatsData base = CharacterDataRegistry::Get().GetStats(player.jobId.empty()?player.className:player.jobId);
    player.maxHp = static_cast<int>(base.maxHealth);
    player.hp = player.maxHp;
    player.maxMp = static_cast<int>(base.maxMana);
    player.mp = player.maxMp;


    m_members.push_back(player);
}

#include "Game/EquipmentRules.h"
#include "Game/GameStateManager.h"

bool PartyManager::EquipItem(size_t memberIndex, const std::string& slot, const std::string& itemId) {
    Member* m = GetMemberMutable(memberIndex);
    if (!m) return false;
    if (slot == "weapon") m->equip.weapon = itemId;
    else if (slot == "armor") m->equip.armor = itemId;
    else if (slot == "accessory") m->equip.accessory = itemId;
    else return false;
    return true;
}

bool PartyManager::UnequipItem(size_t memberIndex, const std::string& slot) {
    Member* m = GetMemberMutable(memberIndex);
    if (!m) return false;
    if (slot == "weapon") m->equip.weapon.clear();
    else if (slot == "armor") m->equip.armor.clear();
    else if (slot == "accessory") m->equip.accessory.clear();
    else return false;
    return true;
}

#include "Game/InventoryManager.h"

static inline const InventoryManager::ItemDef* GetDef(const std::string& id) {
    return InventoryManager::Get().GetItemDef(id);
}

int PartyManager::GetAttackWithEquipment(size_t memberIndex) const {
    if (memberIndex >= m_members.size()) return 0;
    int atk = 0; // base to be combined with class stats in UI/calc sites
    const auto& eq = m_members[memberIndex].equip;
    if (!eq.weapon.empty()) { if (auto* d = GetDef(eq.weapon)) atk += d->atkBonus; }
    if (!eq.armor.empty()) { if (auto* d = GetDef(eq.armor)) atk += d->atkBonus; }
    if (!eq.accessory.empty()) { if (auto* d = GetDef(eq.accessory)) atk += d->atkBonus; }
    return atk;
}

int PartyManager::GetDefenseWithEquipment(size_t memberIndex) const {
    if (memberIndex >= m_members.size()) return 0;
    int df = 0;
    const auto& eq = m_members[memberIndex].equip;
    if (!eq.weapon.empty()) { if (auto* d = GetDef(eq.weapon)) df += d->defBonus; }
    if (!eq.armor.empty()) { if (auto* d = GetDef(eq.armor)) df += d->defBonus; }
    if (!eq.accessory.empty()) { if (auto* d = GetDef(eq.accessory)) df += d->defBonus; }
    return df;
}

int PartyManager::GetSpeedWithEquipment(size_t memberIndex) const {
    if (memberIndex >= m_members.size()) return 0;
    int spd = 0;
    const auto& eq = m_members[memberIndex].equip;
    if (!eq.weapon.empty()) { if (auto* d = GetDef(eq.weapon)) spd += d->spdBonus; }
    if (!eq.armor.empty()) { if (auto* d = GetDef(eq.armor)) spd += d->spdBonus; }
    if (!eq.accessory.empty()) { if (auto* d = GetDef(eq.accessory)) spd += d->spdBonus; }
    return spd;
}



void PartyManager::AddMember(const Member& m) {
    m_members.push_back(m);
}

