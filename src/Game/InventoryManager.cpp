#include "Game/InventoryManager.h"
#include "Engine/ConfigSystem.h"
#include <iostream>
#include <algorithm>

InventoryManager& InventoryManager::Get() {
    static InventoryManager instance;
    return instance;
}

bool InventoryManager::LoadItemsConfig(const std::string& path) {
    ConfigManager cfg;
    if (!cfg.LoadFromFile(path)) {
        std::cerr << "⚠️  Inventory: Failed to load items config: " << path << std::endl;
        return false;
    }

    m_defs.clear();
    for (const auto& [sectionName, section] : cfg.GetSections()) {
        if (sectionName == "default") continue;
        ItemDef def;
        def.id = sectionName;
        def.name = cfg.Get(sectionName, "name", sectionName).AsString();
        def.type = cfg.Get(sectionName, "type", "consumable").AsString();
        def.slot = cfg.Get(sectionName, "slot", "").AsString();
        def.description = cfg.Get(sectionName, "description", "").AsString();
        def.healAmount = cfg.Get(sectionName, "heal", 0).AsInt();
        def.mpHealAmount = cfg.Get(sectionName, "mp_heal", 0).AsInt();
        // Optional equipment bonuses
        def.atkBonus = cfg.Get(sectionName, "atk_bonus", 0).AsInt();
        def.defBonus = cfg.Get(sectionName, "def_bonus", 0).AsInt();
        def.spdBonus = cfg.Get(sectionName, "spd_bonus", 0).AsInt();
        m_defs[def.id] = def;
    }

    m_loaded = true;
    SeedIfEmpty();
    std::cout << "✅ Inventory: Loaded " << m_defs.size() << " item defs" << std::endl;
    return true;
}

const InventoryManager::ItemDef* InventoryManager::GetItemDef(const std::string& id) const {
    auto it = m_defs.find(id);
    return it != m_defs.end() ? &it->second : nullptr;
}

void InventoryManager::AddItem(const std::string& id, int qty) {
    EnsureLoaded();
    if (qty <= 0) return;
    auto it = std::find_if(m_items.begin(), m_items.end(), [&](const ItemStack& s){ return s.id == id; });
    if (it == m_items.end()) {
        ItemStack stack; stack.id = id; stack.quantity = qty;
        m_items.push_back(stack);
    } else {
        it->quantity += qty;
    }
}

bool InventoryManager::RemoveItem(const std::string& id, int qty) {
    EnsureLoaded();
    if (qty <= 0) return false;
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        if (it->id == id) {
            if (it->quantity < qty) return false;
            it->quantity -= qty;
            if (it->quantity == 0) m_items.erase(it);
            return true;
        }
    }
    return false;
}

int InventoryManager::GetQuantity(const std::string& id) const {
    for (const auto& s : m_items) if (s.id == id) return s.quantity;
    return 0;
}

void InventoryManager::Clear() {
    m_items.clear();
}

void InventoryManager::EnsureLoaded() {
    if (!m_loaded) LoadItemsConfig();
}

void InventoryManager::SeedIfEmpty() {
    // Give a couple of potions for demo/testing purposes
    if (m_items.empty()) {
        if (m_defs.find("potion") != m_defs.end()) AddItem("potion", 3);
        if (m_defs.find("hi_potion") != m_defs.end()) AddItem("hi_potion", 1);
    }
}

