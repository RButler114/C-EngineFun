#pragma once

#include "Game/GameState.h"
#include "Game/GameStateManager.h"

// Helper to check if equipment changes are allowed for a given slot
// Rule: Weapon can be changed anytime; armor/accessory only when NOT in Combat state
inline bool CanChangeEquipmentForSlot(GameStateManager* gsm, const std::string& slot) {
    if (!gsm) return true; // be permissive if not available
    if (slot == "weapon") return true;
    GameState* current = gsm->GetCurrentState();
    if (!current) return true;
    return current->GetType() != GameStateType::COMBAT; // restrict non-weapons in combat
}

