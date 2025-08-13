#pragma once

#include "Game/GameState.h"
#include <vector>
#include <string>

// FF10-style Pause/Party menu overlay pushed on top of PLAYING
class PauseState : public GameState {
public:
    enum class Mode {
        ROOT_MENU,      // Resume / Party / Items / Options / Quit to Menu
        PARTY_VIEW,     // List of current party members
        PARTY_DETAILS,  // Dedicated details view for the selected member
        PARTY_EQUIP,    // Equip UI for currently selected member
        ITEMS_VIEW,     // Items inventory
        OPTIONS_VIEW,   // Delegates to OptionsState by pushing it
    };

    PauseState();
    ~PauseState() override = default;

    void OnEnter() override;
    void OnExit() override;

    void Update(float deltaTime) override;
    void Render() override;
    void HandleInput() override;

private:
    // State
    Mode m_mode;

    // Root menu
    std::vector<std::string> m_options;
    int m_selectedIndex;
    float m_blinkTimer;
    bool m_showSelection;
    bool m_skipFirstFrame; // guard to avoid consuming the triggering ESC in the same frame

    // Party view selection
    int m_partySelectedIndex; // 0..6

    // Drawing helpers
    void DrawOverlay();
    void DrawMenu();
    void DrawPartyPanel();
    void DrawPartyDetailsPanel();

    // Equip UI
    void DrawEquipPanel();
    void HandleEquipInput();

    // Equip panel state
    int m_equipSlotIndex = 0; // 0=weapon,1=armor,2=accessory
    int m_equipItemIndex = 0; // index within filtered inventory list


    void DrawItemsPanel();

    // Navigation helpers
    void NavigateUp();
    void NavigateDown();
    void NavigateLeft();
    void NavigateRight();
    void Select();
    void Back();

    // Items interaction
    void HandleItemsInput();

    // Items state (no persistence across openings by design)
    int m_itemsCursor = 0;             // index into filtered consumables
    bool m_itemsSelectingTarget = false;
    int m_itemsPartyCursor = 0;        // target party member index when selecting target

    // Helpers
    int GetPartySize() const; // Hide future capacity; only returns currently active members
};

