#pragma once

#include <vector>
#include <string>

struct PlayerCustomization; // forward decl to avoid header coupling

class PartyManager {
public:
    struct EquipmentSlots {
        std::string weapon;
        std::string armor;
        std::string accessory;
    };

    struct Member {
        std::string name;
        std::string className; // broad class (warrior/archer/mage/rogue...)
        std::string jobId;     // fine-grained job (e.g., warden)
        int level = 1;
        int hp = 100;
        int maxHp = 100;
        int mp = 30;
        int maxMp = 30;
        std::string portraitPath; // optional; can reuse sprite path
        EquipmentSlots equip;     // simple 3-slot equipment
    };

    static PartyManager& Get();

    void Clear();

    // Initialize a baseline party from player customization if not already present
    void InitializeFromCustomization(const PlayerCustomization& pc);

    // Mutators
    void AddMember(const Member& m);

    // Equipment API
    bool EquipItem(size_t memberIndex, const std::string& slot, const std::string& itemId);
    bool UnequipItem(size_t memberIndex, const std::string& slot);

    // Accessors
    size_t GetMemberCount() const { return m_members.size(); }
    const Member* GetMember(size_t index) const {
        if (index < m_members.size()) return &m_members[index];
        return nullptr;
    }
    Member* GetMemberMutable(size_t index) {
        if (index < m_members.size()) return &m_members[index];
        return nullptr;
    }
    const std::vector<Member>& GetMembers() const { return m_members; }

	    // Derived party state
	    bool IsTotalPartyKO() const {
	        if (m_members.empty()) return false;
	        for (const auto& m : m_members) { if (m.hp > 0) return false; }
	        return true;
	    }



    // Derived stat helpers with equipment bonuses applied
    int GetAttackWithEquipment(size_t memberIndex) const;
    int GetDefenseWithEquipment(size_t memberIndex) const;
    int GetSpeedWithEquipment(size_t memberIndex) const;

private:
    PartyManager() = default;
    std::vector<Member> m_members;
};

