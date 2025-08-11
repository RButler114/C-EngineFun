#include "Game/PauseState.h"
#include "Game/GameStateManager.h"
#include "Game/PlayingState.h"
#include "Game/PartyManager.h"
#include "Game/PlayerCustomization.h"
#include "Game/CharacterData.h"
#include "Engine/Renderer.h"
#include "Engine/InputManager.h"
#include "Engine/BitmapFont.h"
#include "Engine/AudioManager.h"
#include "Engine/Engine.h"
#include "ECS/ECS.h"
#include <algorithm>
#include <iostream>

PauseState::PauseState()
    : GameState(GameStateType::PAUSED, "Pause")
    , m_mode(Mode::ROOT_MENU)
    , m_selectedIndex(0)
    , m_blinkTimer(0.0f)
    , m_showSelection(true)
    , m_skipFirstFrame(true)
    , m_partySelectedIndex(0) {
    m_options = {"Resume", "Party", "Items", "Options", "Save", "Quit to Menu"};
}

void PauseState::OnEnter() {
    std::cout << "Entering Pause State" << std::endl;
    if (auto* am = GetEngine()->GetAudioManager()) {
        am->LoadSound("menu_nav", "assets/music/clicking-interface-select-201946.mp3", SoundType::SOUND_EFFECT);
        am->LoadSound("menu_select", "assets/music/select-001-337218.mp3", SoundType::SOUND_EFFECT);
        am->LoadSound("menu_back", "assets/music/select-003-337609.mp3", SoundType::SOUND_EFFECT);
    }

    // Initialize party manager from current customization if empty
    const auto& pc = CustomizationManager::GetInstance().GetPlayerCustomization();
    PartyManager::Get().InitializeFromCustomization(pc);

    // Ensure character data is loaded for details view
    CharacterDataRegistry::Get().EnsureLoaded("assets/config/characters.ini");
}

void PauseState::OnExit() {
    std::cout << "Exiting Pause State" << std::endl;
}

void PauseState::Update(float deltaTime) {
    // Input is handled via GameStateManager::HandleInput() per frame; do not call here to avoid double-processing
    m_blinkTimer += deltaTime;
    if (m_blinkTimer >= 0.5f) { m_showSelection = !m_showSelection; m_blinkTimer = 0.0f; }
}

void PauseState::Render() {
    DrawOverlay();
    DrawMenu();

    // Right-hand content panel varies by mode
    switch (m_mode) {
        case Mode::ROOT_MENU:
            DrawPartyPanel(); // show party summary by default on the right
            break;
        case Mode::PARTY_VIEW:
            DrawPartyPanel();
            break;
        case Mode::PARTY_DETAILS:
            DrawPartyDetailsPanel();
            break;
        case Mode::ITEMS_VIEW:
            DrawItemsPanel();
            break;
        case Mode::SAVE_CONFIRM:
            DrawSavePanel();
            break;
        case Mode::OPTIONS_VIEW:
            // Options pushes a separate state; no custom drawing here
            DrawPartyPanel();
            break;
    }
}

void PauseState::HandleInput() {
    auto* input = GetInputManager();
    if (!input) return;

    // Avoid consuming the same ESC/P press that opened this pause state
    if (m_skipFirstFrame) {
        m_skipFirstFrame = false;
        return; // ignore all input on the first frame after being pushed
    }

    if (input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || input->IsKeyJustPressed(SDL_SCANCODE_P) || input->IsKeyJustPressed(SDL_SCANCODE_B)) {
        Back();
        return;
    }

    // Manual debouncing similar to MenuState to avoid skipping options
    static bool upWasPressed = false;
    static bool downWasPressed = false;
    static bool enterWasPressed = false;
    static bool spaceWasPressed = false;

    bool upPressed = input->IsKeyPressed(SDL_SCANCODE_UP) || input->IsKeyPressed(SDL_SCANCODE_W);
    bool downPressed = input->IsKeyPressed(SDL_SCANCODE_DOWN) || input->IsKeyPressed(SDL_SCANCODE_S);
    bool leftPressed = input->IsKeyPressed(SDL_SCANCODE_LEFT) || input->IsKeyPressed(SDL_SCANCODE_A);
    bool rightPressed = input->IsKeyPressed(SDL_SCANCODE_RIGHT) || input->IsKeyPressed(SDL_SCANCODE_D);

    if (upPressed && !upWasPressed) {
        NavigateUp();
    }
    if (downPressed && !downWasPressed) {
        NavigateDown();
    }
    if (leftPressed) {
        NavigateLeft();
    }
    if (rightPressed) {
        NavigateRight();
    }

    upWasPressed = upPressed;
    downWasPressed = downPressed;

    bool enterPressed = input->IsKeyPressed(SDL_SCANCODE_RETURN);
    bool spacePressed = input->IsKeyPressed(SDL_SCANCODE_SPACE);

    if ((enterPressed && !enterWasPressed) || (spacePressed && !spaceWasPressed)) {
        Select();
    }

    enterWasPressed = enterPressed;
    spaceWasPressed = spacePressed;

    // Quick back to menu via M
    if (input->IsKeyJustPressed(SDL_SCANCODE_M)) {
        if (auto* sm = GetStateManager()) {
            sm->ChangeState(GameStateType::MENU);
        }
    }
}

void PauseState::DrawOverlay() {
    auto* r = GetRenderer();
    if (!r) return;

    // Dim the background
    r->DrawRectangle(Rectangle(0, 0, 800, 600), Color(0, 0, 0, 180), true);

    // Title
    std::string title = "PAUSE";
    int titleWidth = static_cast<int>(title.size()) * 6 * 3;
    int titleX = (800 - titleWidth) / 2;
    BitmapFont::DrawText(r, title, titleX, 40, 3, Color(255, 255, 0, 255));
}

void PauseState::DrawMenu() {
    auto* r = GetRenderer();
    if (!r) return;

    // Left menu panel like FF10
    int x = 40, y = 120, w = 260, h = 360;
    r->DrawRectangle(Rectangle(x, y, w, h), Color(0, 0, 0, 220), true);
    r->DrawRectangle(Rectangle(x, y, w, h), Color(255, 255, 255, 255), false);

    BitmapFont::DrawText(r, "MENU", x + 12, y + 10, 2, Color(255, 255, 0, 255));

    for (size_t i = 0; i < m_options.size(); ++i) {
        int oy = y + 40 + static_cast<int>(i) * 24;
        bool selected = (static_cast<int>(i) == m_selectedIndex);
        if (selected && m_showSelection) {
            // selection marker
            BitmapFont::DrawText(r, ">", x + 12, oy, 2, Color(255, 255, 0, 255));
        }
        Color color = selected ? Color(255, 255, 0, 255) : Color(200, 200, 200, 255);
        BitmapFont::DrawText(r, m_options[i], x + 30, oy, 2, color);
    }
}

void PauseState::DrawPartyPanel() {
    auto* r = GetRenderer();
    if (!r) return;

    // Right info panel with party slots
    int x = 320, y = 120, w = 440, h = 360;
    r->DrawRectangle(Rectangle(x, y, w, h), Color(0, 0, 0, 220), true);
    r->DrawRectangle(Rectangle(x, y, w, h), Color(255, 255, 255, 255), false);

    BitmapFont::DrawText(r, "PARTY", x + 12, y + 10, 2, Color(255, 255, 0, 255));

    // Layout: draw only current party size; no empty placeholders
    int slotX = x + 12;
    int slotY = y + 40;
    int slotW = w - 24;
    int slotH = 40;
    int gap = 6;

    int partySize = static_cast<int>(PartyManager::Get().GetMemberCount());

    for (int i = 0; i < partySize; ++i) {
        int sy = slotY + i * (slotH + gap);
        bool selected = (m_mode == Mode::PARTY_VIEW && i == m_partySelectedIndex);

        // Background for each slot
        Color fill = Color(0, 0, 0, 140);
        r->DrawRectangle(Rectangle(slotX, sy, slotW, slotH), fill, true);
        r->DrawRectangle(Rectangle(slotX, sy, slotW, slotH), Color(120,120,120,255), false);

        if (selected) {
            // Highlight border when in party view and selected
            r->DrawRectangle(Rectangle(slotX-2, sy-2, slotW+4, slotH+4), Color(255, 255, 0, 255), false);
        }

        // Left: face/icon placeholder
        int faceW = 32, faceH = 32;
        r->DrawRectangle(Rectangle(slotX + 6, sy + (slotH - faceH)/2, faceW, faceH), Color(80,80,120,255), true);

        // Text info
        int textX = slotX + 6 + faceW + 8;
        const auto* m = PartyManager::Get().GetMember(i);
        std::string name = m ? (m->name.empty() ? "Hero" : m->name) : "Member";
        std::string klass = m ? (m->className.empty() ? "warrior" : m->className) : "-";
        int hp = m ? m->hp : 0;
        int maxHp = m ? m->maxHp : 1;
        BitmapFont::DrawText(r, name, textX, sy + 6, 2, Color(220,220,220,255));
        BitmapFont::DrawText(r, "Class: " + klass, textX, sy + 22, 1, Color(180,180,180,255));

        // Right: HP bar
        int barW = 120, barH2 = 10;
        int barX = slotX + slotW - barW - 10;
        int barY = sy + (slotH - barH2)/2;
        r->DrawRectangle(Rectangle(barX, barY, barW, barH2), Color(60,60,60,255), true);
        int hpW = (maxHp > 0) ? (barW * std::max(0, std::min(hp, maxHp)) / maxHp) : 0;
        r->DrawRectangle(Rectangle(barX, barY, hpW, barH2), Color(0,180,0,255), true);
    }

    if (partySize == 0) {
        // If no party members yet, show a neutral message instead of empty slots
        BitmapFont::DrawText(r, "No party members yet.", x + 12, y + 50, 2, Color(200,200,200,255));
        BitmapFont::DrawText(r, "Progress to recruit allies.", x + 12, y + 80, 1, Color(180,180,180,255));
    }

    // Member details sub-panel when in PARTY_VIEW and selection valid
    if (m_mode == Mode::PARTY_VIEW && partySize > 0 && m_partySelectedIndex >= 0 && m_partySelectedIndex < partySize) {
        int dx = x + w - 180; // right-side small panel
        int dy = y + 12;
        int dw = 168; int dh = 96;
        r->DrawRectangle(Rectangle(dx, dy, dw, dh), Color(0,0,0,200), true);
        r->DrawRectangle(Rectangle(dx, dy, dw, dh), Color(255,255,255,255), false);
        const auto* mem = PartyManager::Get().GetMember(static_cast<size_t>(m_partySelectedIndex));
        if (mem) {
            BitmapFont::DrawText(r, mem->name, dx + 8, dy + 8, 2, Color(255,255,0,255));
            BitmapFont::DrawText(r, ("Class: " + mem->className), dx + 8, dy + 30, 1, Color(220,220,220,255));
            std::string lvl = "Lv. " + std::to_string(mem->level);
            BitmapFont::DrawText(r, lvl, dx + 8, dy + 44, 1, Color(200,200,200,255));
            std::string hp = "HP: " + std::to_string(mem->hp) + "/" + std::to_string(mem->maxHp);
            BitmapFont::DrawText(r, hp, dx + 8, dy + 58, 1, Color(200,200,200,255));
        }
    }
}

void PauseState::DrawPartyDetailsPanel() {
    auto* r = GetRenderer();
    if (!r) return;

    // Full right panel dedicated to details
    int x = 320, y = 120, w = 440, h = 360;
    r->DrawRectangle(Rectangle(x, y, w, h), Color(0, 0, 0, 220), true);
    r->DrawRectangle(Rectangle(x, y, w, h), Color(255, 255, 255, 255), false);

    BitmapFont::DrawText(r, "PARTY - DETAILS", x + 12, y + 10, 2, Color(255, 255, 0, 255));

    int partySize = static_cast<int>(PartyManager::Get().GetMemberCount());
    if (partySize == 0 || m_partySelectedIndex < 0 || m_partySelectedIndex >= partySize) {
        BitmapFont::DrawText(r, "No member selected.", x + 12, y + 50, 1, Color(200,200,200,255));
        return;
    }

    const auto* m = PartyManager::Get().GetMember(static_cast<size_t>(m_partySelectedIndex));
    if (!m) return;

    // Portrait placeholder
    int faceW = 64, faceH = 64;
    r->DrawRectangle(Rectangle(x + 12, y + 40, faceW, faceH), Color(80,80,120,255), true);

    // Basic info
    int infoX = x + 12 + faceW + 12;
    BitmapFont::DrawText(r, m->name, infoX, y + 40, 2, Color(230,230,230,255));
    BitmapFont::DrawText(r, ("Class: " + m->className), infoX, y + 64, 1, Color(200,200,200,255));
    BitmapFont::DrawText(r, ("Job: " + m->jobId), infoX, y + 78, 1, Color(200,200,200,255));

    // Stats area
    int statsX = x + 12;
    int statsY = y + 120;
    BitmapFont::DrawText(r, "Stats", statsX, statsY, 2, Color(255,255,0,255));
    statsY += 26;

    // HP Bar
    int barW = 260, barH = 10; int barX = statsX + 80; int barY = statsY + 4;
    r->DrawRectangle(Rectangle(statsX, statsY, 70, 12), Color(0,0,0,0), false);
    BitmapFont::DrawText(r, "HP", statsX, statsY, 1, Color(200,200,200,255));
    r->DrawRectangle(Rectangle(barX, barY, barW, barH), Color(60,60,60,255), true);
    int hpW = (m->maxHp > 0) ? (barW * std::max(0, std::min(m->hp, m->maxHp)) / m->maxHp) : 0;
    r->DrawRectangle(Rectangle(barX, barY, hpW, barH), Color(0,180,0,255), true);

    // Level and derived stats (from character data)
    statsY += 24;
    BitmapFont::DrawText(r, ("Level: " + std::to_string(m->level)), statsX, statsY, 1, Color(200,200,200,255));
    statsY += 16;

    // Pull class/job baseline from CharacterDataRegistry (use jobId if present, else class name)
    std::string key = !m->jobId.empty() ? m->jobId : m->className;
    CharacterStatsData base = CharacterDataRegistry::Get().GetStats(key);
    int atk = static_cast<int>(base.strength);
    int def = static_cast<int>(base.vitality + base.armor * 100.0f * 0.2f); // rough composite
    int spd = static_cast<int>(base.agility);
    BitmapFont::DrawText(r, "ATK: " + std::to_string(atk) + "  DEF: " + std::to_string(def) + "  SPD: " + std::to_string(spd),
                         statsX, statsY, 1, Color(180,180,180,255));

    // Footer hint
    BitmapFont::DrawText(r, "ENTER: Back to list  B/ESC: Back", x + 12, y + h - 20, 1, Color(180,180,180,255));
}

void PauseState::DrawItemsPanel() {
    auto* r = GetRenderer();
    if (!r) return;
    int x = 320, y = 120, w = 440, h = 360;
    r->DrawRectangle(Rectangle(x, y, w, h), Color(0, 0, 0, 220), true);
    r->DrawRectangle(Rectangle(x, y, w, h), Color(255, 255, 255, 255), false);
    BitmapFont::DrawText(r, "ITEMS (placeholder)", x + 12, y + 10, 2, Color(255, 255, 0, 255));
    BitmapFont::DrawText(r, "No items implemented.", x + 12, y + 40, 1, Color(200,200,200,255));
}

void PauseState::DrawSavePanel() {
    auto* r = GetRenderer();
    if (!r) return;
    int x = 320, y = 120, w = 440, h = 360;
    r->DrawRectangle(Rectangle(x, y, w, h), Color(0, 0, 0, 220), true);
    r->DrawRectangle(Rectangle(x, y, w, h), Color(255, 255, 255, 255), false);
    BitmapFont::DrawText(r, "SAVE (placeholder)", x + 12, y + 10, 2, Color(255, 255, 0, 255));
    BitmapFont::DrawText(r, "Save system not yet implemented.", x + 12, y + 40, 1, Color(200,200,200,255));
}

void PauseState::NavigateUp() {
    if (auto* am = GetEngine()->GetAudioManager()) am->PlaySound("menu_nav", 0.8f);
    if (m_mode == Mode::ROOT_MENU) {
        m_selectedIndex = (m_selectedIndex - 1 + static_cast<int>(m_options.size())) % static_cast<int>(m_options.size());
    } else if (m_mode == Mode::PARTY_VIEW) {
        int partySize = static_cast<int>(PartyManager::Get().GetMemberCount());
        if (partySize > 0) {
            m_partySelectedIndex = (m_partySelectedIndex - 1 + partySize) % partySize; // wrap
        }
    }
    m_showSelection = true;
    m_blinkTimer = 0.0f;
}

void PauseState::NavigateDown() {
    if (auto* am = GetEngine()->GetAudioManager()) am->PlaySound("menu_nav", 0.8f);
    if (m_mode == Mode::ROOT_MENU) {
        m_selectedIndex = (m_selectedIndex + 1) % static_cast<int>(m_options.size());
    } else if (m_mode == Mode::PARTY_VIEW) {
        int partySize = static_cast<int>(PartyManager::Get().GetMemberCount());
        if (partySize > 0) {
            m_partySelectedIndex = (m_partySelectedIndex + 1) % partySize;
        }
    }
    m_showSelection = true;
    m_blinkTimer = 0.0f;
}

void PauseState::NavigateLeft() {
    if (m_mode == Mode::PARTY_VIEW) {
        if (auto* am = GetEngine()->GetAudioManager()) am->PlaySound("menu_nav", 0.8f);
        // For single-column layout, left behaves like up
        int partySize = static_cast<int>(PartyManager::Get().GetMemberCount());
        if (partySize > 0) {
            m_partySelectedIndex = (m_partySelectedIndex - 1 + partySize) % partySize;
        }
    }
}

void PauseState::NavigateRight() {
    if (m_mode == Mode::PARTY_VIEW) {
        if (auto* am = GetEngine()->GetAudioManager()) am->PlaySound("menu_nav", 0.8f);
        // For single-column layout, right behaves like down
        int partySize = static_cast<int>(PartyManager::Get().GetMemberCount());
        if (partySize > 0) {
            m_partySelectedIndex = (m_partySelectedIndex + 1) % partySize;
        }
    }
}

void PauseState::Select() {
    if (auto* am = GetEngine()->GetAudioManager()) am->PlaySound("menu_select", 0.9f);

    if (m_mode == Mode::ROOT_MENU) {
        switch (m_selectedIndex) {
            case 0: // Resume
                if (GetStateManager()) GetStateManager()->PopState();
                return;
            case 1: // Party
                m_mode = Mode::PARTY_VIEW;
                return;
            case 2: // Items
                m_mode = Mode::ITEMS_VIEW;
                return;
            case 3: // Options
                if (GetStateManager()) GetStateManager()->PushState(GameStateType::OPTIONS);
                return;
            case 4: // Save
                m_mode = Mode::SAVE_CONFIRM;
                return;
            case 5: // Quit to Menu
                if (GetStateManager()) GetStateManager()->ChangeState(GameStateType::MENU);
                return;
        }
    } else if (m_mode == Mode::PARTY_VIEW) {
        // Enter dedicated details view for selected member (if any)
        int partySize = static_cast<int>(PartyManager::Get().GetMemberCount());
        if (partySize > 0) {
            m_mode = Mode::PARTY_DETAILS;
        }
        return;
    } else if (m_mode == Mode::PARTY_DETAILS) {
        // In details view: Enter returns to the list
        m_mode = Mode::PARTY_VIEW;
        return;
    } else if (m_mode == Mode::ITEMS_VIEW) {
        // Placeholder: no items
        return;
    } else if (m_mode == Mode::SAVE_CONFIRM) {
        // Placeholder: no save system
        return;
    }
}

void PauseState::Back() {
    if (auto* am = GetEngine()->GetAudioManager()) am->PlaySound("menu_back", 0.9f);
    if (m_mode == Mode::ROOT_MENU) {
        if (GetStateManager()) GetStateManager()->PopState(); // same as resume
    } else if (m_mode == Mode::PARTY_DETAILS) {
        m_mode = Mode::PARTY_VIEW; // back from details to list
    } else {
        m_mode = Mode::ROOT_MENU;
    }
}

int PauseState::GetPartySize() const {
    // Currently only the player is in the party; in future this should
    // query a Party/PartySystem or stored roster. Returning 1 hides
    // the future capacity and prevents implying total team size.
    return 1;
}

