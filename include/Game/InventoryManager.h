#pragma once

#include <string>
#include <unordered_map>
#include <vector>

// Simple, config-backed inventory system for items
// Loads item definitions from assets/config/items.ini
// and tracks a list of ItemStacks representing the player's inventory.
class InventoryManager {
public:
    struct ItemDef {
        std::string id;
        std::string name;
        std::string type;        // consumable | equipment
        std::string slot;        // weapon | armor | accessory (for equipment)
        std::string description;
        int healAmount = 0;      // HP heal for consumables
        int mpHealAmount = 0;     // MP restore for consumables
        // Optional bonuses for equipment
        int atkBonus = 0;
        int defBonus = 0;
        int spdBonus = 0;
    };

    struct ItemStack {
        std::string id;
        int quantity = 0;
    };

    static InventoryManager& Get();

    // Load item definitions from config file. Safe to call multiple times.
    bool LoadItemsConfig(const std::string& path = "assets/config/items.ini");

    // Definitions
    const ItemDef* GetItemDef(const std::string& id) const;

    // Inventory operations
    void AddItem(const std::string& id, int qty = 1);
    bool RemoveItem(const std::string& id, int qty = 1);
    int GetQuantity(const std::string& id) const;
    const std::vector<ItemStack>& GetAll() const { return m_items; }
    bool HasAny() const { return !m_items.empty(); }

    // Utility
    void Clear();

private:
    InventoryManager() = default;

    // Internal helpers
    void EnsureLoaded();
    void SeedIfEmpty(); // Give the player a couple of basic items on first use for demo

    // Storage
    std::unordered_map<std::string, ItemDef> m_defs;
    std::vector<ItemStack> m_items;
    bool m_loaded = false;
};

