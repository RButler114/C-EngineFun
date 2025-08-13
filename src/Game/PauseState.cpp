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
#include "Game/InventoryManager.h"

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
    m_options = {"Resume", "Party", "Items", "Options", "Quit to Menu"};
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
        case Mode::PARTY_EQUIP:
            DrawEquipPanel();
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

    // If in equip mode, handle equip-specific input first
    if (m_mode == Mode::PARTY_EQUIP) {
        HandleEquipInput();
        return;
    }

    spaceWasPressed = spacePressed;

    // Quick back to menu via M
    // Items panel input
    if (m_mode == Mode::ITEMS_VIEW) {
        HandleItemsInput();
        return;
    }

    if (input->IsKeyJustPressed(SDL_SCANCODE_M)) {
        if (auto* sm = GetStateManager()) {
            sm->ChangeState(GameStateType::MENU);
        }
    }
}

void PauseState::DrawOverlay() {
    auto* r = GetRenderer();
    if (!r) return;

    // Dim the background (logical size)
    int logicalW = 800, logicalH = 600;
    r->DrawRectangle(Rectangle(0, 0, logicalW, logicalH), Color(0, 0, 0, 180), true);

    // Title (UI scale aware)
    float uiScale = BitmapFont::GetGlobalScale();
    std::string title = "PAUSE";
    int titleScale = std::max(1, (int)(3 * uiScale));
    int titleWidth = static_cast<int>(title.size()) * 6 * titleScale;
    int titleX = (int)((logicalW - titleWidth) / 2);
    BitmapFont::DrawText(r, title, titleX, (int)(40 * uiScale), titleScale, Color(255, 255, 0, 255));
}

void PauseState::DrawMenu() {
    auto* r = GetRenderer();
    if (!r) return;

    // Left menu panel like FF10 (UI scale aware)
    float uiScale = BitmapFont::GetGlobalScale();
    int x = (int)(40 * uiScale), y = (int)(120 * uiScale), w = (int)(260 * uiScale), h = (int)(360 * uiScale);
    r->DrawRectangle(Rectangle(x, y, w, h), Color(0, 0, 0, 220), true);
    r->DrawRectangle(Rectangle(x, y, w, h), Color(255, 255, 255, 255), false);

    int labelScale = std::max(1, (int)(2 * uiScale));
    BitmapFont::DrawText(r, "MENU", x + (int)(12*uiScale), y + (int)(10*uiScale), labelScale, Color(255, 255, 0, 255));

    for (size_t i = 0; i < m_options.size(); ++i) {
        int oy = y + (int)(40*uiScale) + static_cast<int>(i) * (int)(24*uiScale);
        bool selected = (static_cast<int>(i) == m_selectedIndex);
        if (selected && m_showSelection) {
            // selection marker
            BitmapFont::DrawText(r, ">", x + (int)(12*uiScale), oy, labelScale, Color(255, 255, 0, 255));
        }
        Color color = selected ? Color(255, 255, 0, 255) : Color(200, 200, 200, 255);
        BitmapFont::DrawText(r, m_options[i], x + (int)(30*uiScale), oy, labelScale, color);
    }
}

void PauseState::DrawPartyPanel() {
    auto* r = GetRenderer();
    if (!r) return;

    // Right info panel with party slots (UI scale aware)
    float uiScale = BitmapFont::GetGlobalScale();
    int x = (int)(320*uiScale), y = (int)(120*uiScale), w = (int)(440*uiScale), h = (int)(360*uiScale);
    r->DrawRectangle(Rectangle(x, y, w, h), Color(0, 0, 0, 220), true);
    r->DrawRectangle(Rectangle(x, y, w, h), Color(255, 255, 255, 255), false);

    int labelScale = std::max(1, (int)(2 * uiScale));
    BitmapFont::DrawText(r, "PARTY", x + (int)(12*uiScale), y + (int)(10*uiScale), labelScale, Color(255, 255, 0, 255));

    // Layout: draw only current party size; no empty placeholders
    int slotX = x + (int)(12*uiScale);
    int slotY = y + (int)(40*uiScale);
    int slotW = w - (int)(24*uiScale);
    int slotH = (int)(40*uiScale);
    int gap = (int)(6*uiScale);

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
        int faceW = (int)(32*uiScale), faceH = (int)(32*uiScale);
        r->DrawRectangle(Rectangle(slotX + (int)(6*uiScale), sy + (slotH - faceH)/2, faceW, faceH), Color(80,80,120,255), true);

        // Text info
        int textX = slotX + (int)(6*uiScale) + faceW + (int)(8*uiScale);
        const auto* m = PartyManager::Get().GetMember(i);
        std::string name = m ? (m->name.empty() ? "Hero" : m->name) : "Member";
        std::string klass = m ? (m->className.empty() ? "warrior" : m->className) : "-";
        int hp = m ? m->hp : 0;
        int maxHp = m ? m->maxHp : 1;
        // HP/MP overview bars
        int barsX = textX + (int)(160*uiScale); // right-aligned bars inside slot
        int barsW = slotX + slotW - (int)(12*uiScale) - barsX;
        int hpY = sy + (int)(6*uiScale), mpY = sy + (int)(22*uiScale);
        // HP
        r->DrawRectangle(Rectangle(barsX, hpY, barsW, (int)(8*uiScale)), Color(60,60,60,255), true);
        int hpFill = (maxHp>0) ? (barsW * std::max(0, std::min(hp, maxHp)) / maxHp) : 0;
        r->DrawRectangle(Rectangle(barsX, hpY, hpFill, (int)(8*uiScale)), Color(0,180,0,255), true);
        // MP
        int mp = m ? m->mp : 0; int maxMp = m ? m->maxMp : 1;
        r->DrawRectangle(Rectangle(barsX, mpY, barsW, (int)(6*uiScale)), Color(60,60,60,255), true);
        int mpFill = (maxMp>0) ? (barsW * std::max(0, std::min(mp, maxMp)) / maxMp) : 0;
        r->DrawRectangle(Rectangle(barsX, mpY, mpFill, (int)(6*uiScale)), Color(0,120,200,255), true);

        BitmapFont::DrawText(r, name, textX, sy + (int)(6*uiScale), labelScale, Color(220,220,220,255));
        BitmapFont::DrawText(r, "Class: " + klass, textX, sy + (int)(22*uiScale), std::max(1, (int)uiScale), Color(180,180,180,255));

        // Right: HP bar
        int barW = (int)(120*uiScale), barH2 = (int)(10*uiScale);
        int barX = slotX + slotW - barW - (int)(10*uiScale);
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
    // MP Bar will be drawn in Stats section below

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
    int atk = static_cast<int>(base.strength) + PartyManager::Get().GetAttackWithEquipment(static_cast<size_t>(m_partySelectedIndex));
    int def = static_cast<int>(base.vitality + base.armor * 100.0f * 0.2f) + PartyManager::Get().GetDefenseWithEquipment(static_cast<size_t>(m_partySelectedIndex));
    int spd = static_cast<int>(base.agility) + PartyManager::Get().GetSpeedWithEquipment(static_cast<size_t>(m_partySelectedIndex));
    BitmapFont::DrawText(r, "ATK: " + std::to_string(atk) + "  DEF: " + std::to_string(def) + "  SPD: " + std::to_string(spd),
                         statsX, statsY, 1, Color(180,180,180,255));

    // Equipment section for selected party member
    // partySize already defined above
}

void PauseState::DrawEquipPanel() {
    auto* r = GetRenderer();
    if (!r) return;

    int x = 320, y = 120, w = 440, h = 360;
    r->DrawRectangle(Rectangle(x, y, w, h), Color(0, 0, 0, 220), true);
    r->DrawRectangle(Rectangle(x, y, w, h), Color(255, 255, 255, 255), false);

    BitmapFont::DrawText(r, "EQUIP", x + 12, y + 10, 2, Color(255, 255, 0, 255));

    int partySize = static_cast<int>(PartyManager::Get().GetMemberCount());
    if (partySize == 0 || m_partySelectedIndex < 0 || m_partySelectedIndex >= partySize) {
        BitmapFont::DrawText(r, "No member.", x + 12, y + 40, 1, Color(200,200,200,255));
        return;
    }

    const auto* mem = PartyManager::Get().GetMember(static_cast<size_t>(m_partySelectedIndex));
    BitmapFont::DrawText(r, mem->name, x + 12, y + 40, 1, Color(230,230,230,255));

    // Slots list
    std::vector<std::string> slots = {"Weapon", "Armor", "Accessory"};
    int sx = x + 12, sy = y + 70;
    for (int i = 0; i < (int)slots.size(); ++i) {
        bool sel = (i == m_equipSlotIndex);
        if (sel) r->DrawRectangle(Rectangle(sx - 2, sy - 2, 180, 18), Color(40,40,80,255), true);
        std::string val = "-";
        if (i == 0) val = mem->equip.weapon.empty()?"-":mem->equip.weapon;
        if (i == 1) val = mem->equip.armor.empty()?"-":mem->equip.armor;
        if (i == 2) val = mem->equip.accessory.empty()?"-":mem->equip.accessory;
        BitmapFont::DrawText(r, slots[i] + ": " + val, sx, sy, 1, sel?Color(255,255,255,255):Color(200,200,200,255));
        sy += 20;
    }

    // Inventory list (right side), filtered by type
    auto& inv = InventoryManager::Get();
    const auto& items = inv.GetAll();
    int ix = x + 210, iy = y + 70;
    int shown = 0;

    // Row 0: Unequip option
    bool selUnequip = (m_equipItemIndex == 0);
    if (selUnequip) r->DrawRectangle(Rectangle(ix - 2, iy - 2, 220, 18), Color(80,40,40,255), true);
    BitmapFont::DrawText(r, "< Unequip >", ix, iy, 1, selUnequip?Color(255,200,200,255):Color(200,180,180,255));
    iy += 18;
    ++shown;

    // Rows 1..N: Filtered equipment items for this slot
    int selectedRow = 1;
    for (size_t i = 0; i < items.size(); ++i) {
        const auto* def = inv.GetItemDef(items[i].id);
        if (!def) continue;
        if (def->type != "equipment") continue;
        // Per-slot filtering
        if (m_equipSlotIndex == 0 && def->slot != "weapon") continue;
        if (m_equipSlotIndex == 1 && def->slot != "armor") continue;
        if (m_equipSlotIndex == 2 && def->slot != "accessory") continue;
        bool isSelected = (selectedRow == m_equipItemIndex);
        if (isSelected) r->DrawRectangle(Rectangle(ix - 2, iy - 2, 220, 18), Color(40,80,40,255), true);
        // Show preview of bonuses
        std::string bonus;
        if (def->atkBonus) bonus += "+ATK " + std::to_string(def->atkBonus) + " ";
        if (def->defBonus) bonus += "+DEF " + std::to_string(def->defBonus) + " ";
        if (def->spdBonus) bonus += "+SPD " + std::to_string(def->spdBonus) + " ";
        BitmapFont::DrawText(r, def->name + " " + bonus + " x" + std::to_string(items[i].quantity), ix, iy, 1, isSelected?Color(255,255,255,255):Color(200,200,200,255));
        iy += 18;
        ++selectedRow;
        ++shown;
        if (shown >= 12) break;
    }

    BitmapFont::DrawText(r, "UP/DOWN: Slot  LEFT/RIGHT: Items  ENTER: Equip/Unequip  B/ESC: Back", x + 12, y + h - 20, 1, Color(180,180,180,255));
}

void PauseState::HandleEquipInput() {
    auto* input = GetInputManager();
    if (!input) return;

    if (input->IsKeyJustPressed(SDL_SCANCODE_UP)) {
        m_equipSlotIndex = (m_equipSlotIndex - 1 + 3) % 3;
    } else if (input->IsKeyJustPressed(SDL_SCANCODE_DOWN)) {
        m_equipSlotIndex = (m_equipSlotIndex + 1) % 3;
    } else if (input->IsKeyJustPressed(SDL_SCANCODE_LEFT)) {
        m_equipItemIndex = std::max(0, m_equipItemIndex - 1);
    } else if (input->IsKeyJustPressed(SDL_SCANCODE_RIGHT)) {
        m_equipItemIndex = m_equipItemIndex + 1; // capped in draw/selection later
    } else if (input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || input->IsKeyJustPressed(SDL_SCANCODE_B)) {
        m_mode = Mode::PARTY_DETAILS;
    } else if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN) || input->IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
        // Equip/Unequip current selection
        int partySize = static_cast<int>(PartyManager::Get().GetMemberCount());
        if (partySize == 0 || m_partySelectedIndex < 0 || m_partySelectedIndex >= partySize) return;
        size_t memberIndex = static_cast<size_t>(m_partySelectedIndex);

        if (m_equipItemIndex == 0) {
            // Unequip selected slot, respecting combat rule
            std::string slot = (m_equipSlotIndex==0?"weapon":(m_equipSlotIndex==1?"armor":"accessory"));
            if (slot != "weapon") {
                if (auto* sm = GetStateManager()) {
                    auto* current = sm->GetCurrentState();
                    if (current && current->GetType() == GameStateType::COMBAT) {
                        return;
                    }
                }
            }
            PartyManager::Get().UnequipItem(memberIndex, slot);
            return;
        }

        auto& inv = InventoryManager::Get();
        const auto& items = inv.GetAll();
        int idx = 1; // account for Unequip row
        for (size_t i = 0; i < items.size(); ++i) {
            const auto* def = inv.GetItemDef(items[i].id);
            if (!def) continue;
            if (def->type != "equipment") continue;
            // Per-slot filtering
            if (m_equipSlotIndex == 0 && def->slot != "weapon") continue;
            if (m_equipSlotIndex == 1 && def->slot != "armor") continue;
            if (m_equipSlotIndex == 2 && def->slot != "accessory") continue;
            if (idx == m_equipItemIndex) {
                // Enforce rule: only weapon changes allowed during combat
                std::string slot = def->slot;
                if (slot != "weapon") {
                    if (auto* sm = GetStateManager()) {
                        auto* current = sm->GetCurrentState();
                        if (current && current->GetType() == GameStateType::COMBAT) {
                            // Disallow
                            return;
                        }
                    }
                }
                PartyManager::Get().EquipItem(memberIndex, slot, def->id);
                break;
            }
            ++idx;
        }
    }
}



void PauseState::DrawItemsPanel() {
    auto* r = GetRenderer();
    if (!r) return;
    int x = 320, y = 120, w = 440, h = 360;
    r->DrawRectangle(Rectangle(x, y, w, h), Color(0, 0, 0, 220), true);
    r->DrawRectangle(Rectangle(x, y, w, h), Color(255, 255, 255, 255), false);
    BitmapFont::DrawText(r, "ITEMS", x + 12, y + 10, 2, Color(255, 255, 0, 255));

    // Focus: consumables only, grouped and paged
    const auto& inv = InventoryManager::Get();
    const auto& items = inv.GetAll();

    // Filter only consumables
    std::vector<const InventoryManager::ItemStack*> consumables;
    consumables.reserve(items.size());
    for (const auto& s : items) {
        const auto* def = inv.GetItemDef(s.id);
        if (def && def->type == "consumable") consumables.push_back(&s);
    }

    if (consumables.empty()) {
        BitmapFont::DrawText(r, "[No - Items]", x + 12, y + 40, 1, Color(200,200,200,255));
        return;
    }

    // Display in two columns for quicker scanning
    int colW = (w - 24) / 2;
    int leftX = x + 12;
    int rightX = x + 12 + colW;
    int rowYLeft = y + 40;
    int rowYRight = y + 40;

    for (size_t i = 0; i < consumables.size(); ++i) {
        const auto* stack = consumables[i];
        const auto* def = inv.GetItemDef(stack->id);
        if (!def) continue;
        std::string label = def->name + " x" + std::to_string(stack->quantity);
        std::string desc = def->description;
        // Short inline effect hint
        if (def->healAmount > 0) desc = "+HP " + std::to_string(def->healAmount) + (desc.empty()?"":" — ") + desc;

        bool left = (i % 2 == 0);
        int cx = left ? leftX : rightX;
        int& ry = left ? rowYLeft : rowYRight;
        BitmapFont::DrawText(r, label, cx, ry, 1, Color(230,230,230,255));
        if (!desc.empty()) {
            BitmapFont::DrawText(r, desc, cx, ry + 14, 1, Color(160,160,160,255));
            ry += 30;
        } else {
            ry += 20;
        }
        if (ry > y + h - 30) break; // simple overflow guard
    }
    // Visual overlay for target selection
    if (m_itemsSelectingTarget) {
        int ox = x + 12, oy = y + 40;
        int partyCount = static_cast<int>(PartyManager::Get().GetMemberCount());
        for (int i = 0, drawn = 0; i < partyCount; ++i) {
            const auto* mem = PartyManager::Get().GetMember(static_cast<size_t>(i));
            if (!mem) continue;
            int rowY = oy + drawn * 20;
            bool sel = (i == m_itemsPartyCursor);
            if (sel) r->DrawRectangle(Rectangle(ox - 4, rowY - 2, w - 24, 18), Color(40,80,40,200), true);
            std::string line = mem->name + "  HP " + std::to_string(mem->hp) + "/" + std::to_string(mem->maxHp) +
                                 "  MP " + std::to_string(mem->mp) + "/" + std::to_string(mem->maxMp);
            BitmapFont::DrawText(r, line, ox, rowY, 1, sel?Color(255,255,255,255):Color(200,200,200,255));
            ++drawn;
        }
        BitmapFont::DrawText(r, "UP/DOWN: Member  ENTER: Use  B/ESC: Cancel", x + 12, y + h - 36, 1, Color(180,180,180,255));
    }

    // Tooltip for no-effect (full HP) when targeting
    if (m_itemsSelectingTarget) {
        auto& inv2 = InventoryManager::Get();
        const auto& items2 = inv2.GetAll();
        std::vector<size_t> idxs;
        for (size_t i = 0; i < items2.size(); ++i) {
            const auto* def = inv2.GetItemDef(items2[i].id);
            if (def && def->type == std::string("consumable") && items2[i].quantity > 0) idxs.push_back(i);
        }
        if (!idxs.empty()) {
            size_t invIdx = idxs[static_cast<size_t>(std::clamp(m_itemsCursor, 0, (int)idxs.size()-1))];
            const auto* def = inv2.GetItemDef(items2[invIdx].id);
            const auto* mem = PartyManager::Get().GetMember(static_cast<size_t>(m_itemsPartyCursor));
            if (def && mem && def->healAmount > 0 && mem->hp >= mem->maxHp) {
                BitmapFont::DrawText(r, "No effect (HP full)", x + 12, y + h - 52, 1, Color(255,120,120,255));
            }
            if (def && mem && def->mpHealAmount > 0 && mem->mp >= mem->maxMp) {
                BitmapFont::DrawText(r, "No effect (MP full)", x + 12, y + h - 52, 1, Color(120,120,255,255));
            }

        }
    }


    BitmapFont::DrawText(r, "(Items shown: consumables; other non-equipment items will appear here later)", x + 12, y + h - 20, 1, Color(160,160,160,255));
}


void PauseState::HandleItemsInput() {
    auto* input = GetInputManager();
    if (!input) return;

    // Build consumables list ad-hoc; no persistence requested
    auto& inv = InventoryManager::Get();
    const auto& items = inv.GetAll();
    std::vector<size_t> indices; indices.reserve(items.size());
    for (size_t i = 0; i < items.size(); ++i) {
        const auto* def = inv.GetItemDef(items[i].id);
        if (def && def->type == std::string("consumable") && items[i].quantity > 0) indices.push_back(i);
    }
    if (indices.empty()) return;

    // Navigation for consumable cursor
    if (!m_itemsSelectingTarget) {
        if (input->IsKeyJustPressed(SDL_SCANCODE_DOWN) || input->IsKeyJustPressed(SDL_SCANCODE_RIGHT)) {
            m_itemsCursor = (m_itemsCursor + 1) % static_cast<int>(indices.size());
        } else if (input->IsKeyJustPressed(SDL_SCANCODE_UP) || input->IsKeyJustPressed(SDL_SCANCODE_LEFT)) {
            m_itemsCursor = (m_itemsCursor - 1 + static_cast<int>(indices.size())) % static_cast<int>(indices.size());
        } else if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN) || input->IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
            // Enter target selection
            m_itemsSelectingTarget = true;
            m_itemsPartyCursor = std::clamp(m_itemsPartyCursor, 0, static_cast<int>(PartyManager::Get().GetMemberCount()) - 1);
        }
    } else {
        // Selecting target party member
    // If item has no effect, don't block but show tooltip (rendered in DrawItemsPanel)
    // Logic kept here only for navigation, actual draw happens in DrawItemsPanel

        int partyCount = static_cast<int>(PartyManager::Get().GetMemberCount());
        if (partyCount <= 0) { m_itemsSelectingTarget = false; return; }
        if (input->IsKeyJustPressed(SDL_SCANCODE_DOWN)) {
            m_itemsPartyCursor = (m_itemsPartyCursor + 1) % partyCount;
        } else if (input->IsKeyJustPressed(SDL_SCANCODE_UP)) {
            m_itemsPartyCursor = (m_itemsPartyCursor - 1 + partyCount) % partyCount;
        } else if (input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || input->IsKeyJustPressed(SDL_SCANCODE_B)) {
            m_itemsSelectingTarget = false;
        } else if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN) || input->IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
            // Use the consumable on selected target
            size_t invIdx = indices[static_cast<size_t>(m_itemsCursor)];
            const auto* def = inv.GetItemDef(items[invIdx].id);
            if (def && def->type == std::string("consumable") && items[invIdx].quantity > 0) {
                // Currently: only healing items supported (healAmount)
                auto* target = PartyManager::Get().GetMemberMutable(static_cast<size_t>(m_itemsPartyCursor));
                if (target) {
                    bool applied = false;
                    if (def->healAmount > 0 && target->hp < target->maxHp) {
                        target->hp = std::min(target->maxHp, target->hp + def->healAmount);
                        applied = true;
                    } else if (def->mpHealAmount > 0 && target->mp < target->maxMp) {
                        target->mp = std::min(target->maxMp, target->mp + def->mpHealAmount);
                        applied = true;
                    }
                    if (applied) {
                        inv.RemoveItem(def->id, 1);
                        if (auto* am = GetEngine()->GetAudioManager()) am->PlaySound("menu_select", 0.9f);
                    }
                }
            }
            // Exit target selection either way (no persistence)
            m_itemsSelectingTarget = false;
        }
    }
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
            case 4: // Quit to Menu
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
        // Enter equip mode from details
        m_mode = Mode::PARTY_EQUIP;
        m_equipSlotIndex = 0;
        m_equipItemIndex = 0;
        return;
    } else if (m_mode == Mode::ITEMS_VIEW) {
        return;
    }
}

void PauseState::Back() {
    if (auto* am = GetEngine()->GetAudioManager()) am->PlaySound("menu_back", 0.9f);
    if (m_mode == Mode::ROOT_MENU) {
        if (GetStateManager()) GetStateManager()->PopState(); // same as resume
    } else if (m_mode == Mode::PARTY_DETAILS) {
        m_mode = Mode::PARTY_VIEW; // back from details to list
    } else if (m_mode == Mode::PARTY_EQUIP) {
        m_mode = Mode::PARTY_DETAILS; // back from equip to details
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

