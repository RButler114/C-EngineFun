#pragma once

#include <vector>
#include <string>

struct PlayerCustomization; // forward decl to avoid header coupling

class PartyManager {
public:
    struct Member {
        std::string name;
        std::string className; // broad class (warrior/archer/mage/rogue...)
        std::string jobId;     // fine-grained job (e.g., warden)
        int level = 1;
        int hp = 100;
        int maxHp = 100;
        std::string portraitPath; // optional; can reuse sprite path
    };

    static PartyManager& Get();

    void Clear();

    // Initialize a baseline party from player customization if not already present
    void InitializeFromCustomization(const PlayerCustomization& pc);

    // Mutators
    void AddMember(const Member& m);

    // Accessors
    size_t GetMemberCount() const { return m_members.size(); }
    const Member* GetMember(size_t index) const {
        if (index < m_members.size()) return &m_members[index];
        return nullptr;
    }
    const std::vector<Member>& GetMembers() const { return m_members; }

private:
    PartyManager() = default;
    std::vector<Member> m_members;
};

