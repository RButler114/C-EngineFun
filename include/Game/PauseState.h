#pragma once

#include "Game/GameState.h"
#include <vector>
#include <string>

// FF10-style Pause/Party menu overlay pushed on top of PLAYING
class PauseState : public GameState {
public:
    enum class Mode {
        ROOT_MENU,      // Resume / Party / Items / Options / Save / Quit to Menu
        PARTY_VIEW,     // List of current party members
        PARTY_DETAILS,  // Dedicated details view for the selected member
        ITEMS_VIEW,     // Placeholder grid/list (non-functional for now)
        OPTIONS_VIEW,   // Delegates to OptionsState by pushing it
        SAVE_CONFIRM    // Simple confirm/cancel (placeholder)
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

    void DrawItemsPanel();
    void DrawSavePanel();

    // Navigation helpers
    void NavigateUp();
    void NavigateDown();
    void NavigateLeft();
    void NavigateRight();
    void Select();
    void Back();

    // Helpers
    int GetPartySize() const; // Hide future capacity; only returns currently active members
};

