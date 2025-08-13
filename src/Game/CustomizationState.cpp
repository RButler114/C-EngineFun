/**
 * @file CustomizationState.cpp
 * @brief Implementation of character customization screen
 * @author Ryan Butler
 * @date 2025
 */

#include "Game/CustomizationState.h"
#include "Engine/Engine.h"
#include "Engine/Renderer.h"
#include "Engine/BitmapFont.h"
#include "Engine/InputManager.h"
#include "Game/GameStateManager.h"
#include "Engine/ConfigSystem.h"
#include "Engine/AudioManager.h"

#include "Game/PlayingState.h"
#include "Engine/SpriteRenderer.h"

#include "Game/CharacterData.h"
#include "Game/InventoryManager.h"

#include <SDL2/SDL.h>
namespace {
    bool s_limitsLoaded = false;
    int s_minAttr = 1;
    int s_maxAttr = 99;
    int s_totalPoints = 10;
    void EnsureLimitsLoaded(CustomizationManager* mgr) {
        if (s_limitsLoaded) return;
        ConfigManager cfg;
        if (cfg.LoadFromFile("assets/config/customization.ini") && cfg.HasSection("customization_limits")) {
            s_totalPoints = cfg.Get("customization_limits", "total_attribute_points", s_totalPoints).AsInt();
            s_minAttr = cfg.Get("customization_limits", "min_attribute_value", s_minAttr).AsInt();
            s_maxAttr = cfg.Get("customization_limits", "max_attribute_value", s_maxAttr).AsInt();
        }
        s_limitsLoaded = true;
        if (mgr) {
            auto& cz = mgr->GetPlayerCustomization();
            // If default 10, derive remaining from total - current
            if (cz.availablePoints == 10) {
                int baseSum = static_cast<int>(cz.strength + cz.agility + cz.intelligence + cz.vitality);
                cz.availablePoints = std::max(0, s_totalPoints - baseSum);
            }
        }
    }
}

#include <iostream>
#include <sstream>

#include <algorithm>

// Color constants are now defined in the header as static const int values

CustomizationState::CustomizationState()
    : GameState(GameStateType::CUSTOMIZATION, "Customization") {

    // Initialize categories in order (equipment handled later in Pause -> Equip)
    m_categories = {
        CustomizationCategory::BASIC_INFO,
        CustomizationCategory::APPEARANCE,
        CustomizationCategory::ATTRIBUTES
    };
}

void CustomizationState::OnEnter() {
    std::cout << "Entering Customization State" << std::endl;

    // Use the global customization manager instance
    m_customizationManager = std::make_unique<CustomizationManager>();

    // Try to load from config, fall back to defaults if needed
    if (!m_customizationManager->LoadFromConfig("assets/config/customization.ini")) {
        std::cout << "Using default customization options" << std::endl;
    }

    // Also initialize the global instance
    CustomizationManager::GetInstance().InitializeDefaults();

    // Reset UI state
    m_currentMode = UIMode::CATEGORY_SELECTION;
    m_selectedCategoryIndex = 0;
    m_selectedGroupIndex = 0;
    m_selectedOptionIndex = 0;
    m_nameInput = m_customizationManager->GetPlayerCustomization().playerName;
    m_nameInputActive = false;

    // Load current category groups
    SelectCategory(0);

    // Load common UI sounds for consistent UX
    if (GetEngine()->GetAudioManager()) {
        auto* am = GetEngine()->GetAudioManager();
        am->LoadSound("menu_nav", "assets/music/clicking-interface-select-201946.mp3", SoundType::SOUND_EFFECT);
        am->LoadSound("menu_select", "assets/music/select-001-337218.mp3", SoundType::SOUND_EFFECT);
        am->LoadSound("menu_back", "assets/music/select-003-337609.mp3", SoundType::SOUND_EFFECT);
    // Initialize GameConfig for preview animation and sprite sizing
    m_gameConfig = std::make_unique<GameConfig>();
    m_gameConfig->LoadConfigs();
    // Clamp base attributes to configured min/max right away
    EnsureLimitsLoaded(m_customizationManager.get());
    auto& czClamp = m_customizationManager->GetPlayerCustomization();
    auto clampInt = [](float v, int lo, int hi){ return static_cast<float>(std::max(lo, std::min(hi, static_cast<int>(v)))); };
    czClamp.strength = clampInt(czClamp.strength, s_minAttr, s_maxAttr);
    czClamp.agility = clampInt(czClamp.agility, s_minAttr, s_maxAttr);
    czClamp.intelligence = clampInt(czClamp.intelligence, s_minAttr, s_maxAttr);
    czClamp.vitality = clampInt(czClamp.vitality, s_minAttr, s_maxAttr);

    }
}


void CustomizationState::OnExit() {
    std::cout << "Exiting Customization State" << std::endl;
    m_customizationManager.reset();
}

void CustomizationState::Update(float deltaTime) {
    // Update based on current UI mode
    switch (m_currentMode) {
        case UIMode::CATEGORY_SELECTION:
            UpdateCategorySelection();
            break;
        case UIMode::OPTION_SELECTION:
            UpdateOptionSelection();
            break;
        case UIMode::NAME_INPUT:
            UpdateNameInput();
            break;
        case UIMode::ATTRIBUTE_ADJUSTMENT:
            UpdateAttributeAdjustment();
            break;
        case UIMode::CONFIRMATION:
            UpdateConfirmation();
            break;
    }
}

void CustomizationState::Render() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    // Clear with background color
    renderer->Clear(Color(COLOR_BACKGROUND_R, COLOR_BACKGROUND_G, COLOR_BACKGROUND_B, 255));

    // Render title (UI scale aware)
    float uiScale = BitmapFont::GetGlobalScale();
    BitmapFont::DrawText(renderer, "CHARACTER CUSTOMIZATION", (int)(MARGIN*uiScale), (int)(20*uiScale), std::max(1, (int)(3*uiScale)),
                        Color(COLOR_ACCENT_R, COLOR_ACCENT_G, COLOR_ACCENT_B, 255));

    // Render based on current mode
    switch (m_currentMode) {
        case UIMode::CATEGORY_SELECTION:
            RenderCategorySelection();
            break;
        case UIMode::OPTION_SELECTION:
            RenderOptionSelection();
            break;
        case UIMode::NAME_INPUT:
            RenderNameInput();
            break;
        case UIMode::ATTRIBUTE_ADJUSTMENT:
            RenderAttributeAdjustment();
            break;
        case UIMode::CONFIRMATION:
            RenderConfirmation();
            break;
    }

    // Always render character preview and instructions
    RenderCharacterPreview();
    RenderInstructions();
}

void CustomizationState::HandleInput() {
    auto* input = GetInputManager();
    if (!input) return;

    // Global controls - use B for back navigation
    if (input->IsKeyJustPressed(SDL_SCANCODE_B)) {
        if (m_currentMode == UIMode::CATEGORY_SELECTION) {
            ReturnToMenu();
        } else {
            // Go back to previous mode
            if (m_currentMode == UIMode::NAME_INPUT || m_currentMode == UIMode::OPTION_SELECTION) {
                m_currentMode = UIMode::CATEGORY_SELECTION;
            } else if (m_currentMode == UIMode::CONFIRMATION) {
                m_currentMode = UIMode::CATEGORY_SELECTION;
            }
            if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_back", 0.9f);

        }
        return;
    }

    // Escape key only quits from main category selection
    if (input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        if (m_currentMode == UIMode::CATEGORY_SELECTION) {
            ReturnToMenu();
        }
        return;
    }

    // Mode-specific input handling
    switch (m_currentMode) {
        case UIMode::CATEGORY_SELECTION:
            HandleCategoryInput();
            break;
        case UIMode::OPTION_SELECTION:
            HandleOptionInput();
            break;
        case UIMode::NAME_INPUT:
            HandleNameInputKeys();
    // Ensure limits are loaded early
    EnsureLimitsLoaded(m_customizationManager.get());

            break;
        case UIMode::ATTRIBUTE_ADJUSTMENT:
            HandleAttributeInput();
            break;
        case UIMode::CONFIRMATION:
            HandleConfirmationInput();
            break;
    }
}

void CustomizationState::UpdateCategorySelection() {
    // Nothing special to update in category selection mode
}

void CustomizationState::UpdateOptionSelection() {
    // Nothing special to update in option selection mode
}

void CustomizationState::UpdateNameInput() {
    // Update player name from input
    if (!m_nameInput.empty()) {
        m_customizationManager->GetPlayerCustomization().playerName = m_nameInput;
    }
}

void CustomizationState::UpdateAttributeAdjustment() {
    // Nothing special to update in attribute adjustment mode
}

void CustomizationState::UpdateConfirmation() {
    // Nothing special to update in confirmation mode
}

void CustomizationState::RenderCategorySelection() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    int y = TITLE_HEIGHT + MARGIN;

    BitmapFont::DrawText(renderer, "Select Category:", MARGIN, y, 2,
                        Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
    y += LINE_HEIGHT * 2;

    // Render category options
    const char* categoryNames[] = {"Basic Info", "Appearance", "Attributes", "Equipment"};

    for (int i = 0; i < static_cast<int>(m_categories.size()); ++i) {
        if (i == m_selectedCategoryIndex) {
            BitmapFont::DrawText(renderer, std::string("> ") + categoryNames[i], MARGIN, y, 2,
                               Color(COLOR_SELECTED_R, COLOR_SELECTED_G, COLOR_SELECTED_B, 255));
        } else {
            BitmapFont::DrawText(renderer, std::string("  ") + categoryNames[i], MARGIN, y, 2,
                               Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
        }
        y += LINE_HEIGHT;
    }
}

void CustomizationState::RenderOptionSelection() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    int y = TITLE_HEIGHT + MARGIN;

    if (m_currentCategoryGroups.empty()) return;

    // Show current category
    const char* categoryNames[] = {"Basic Info", "Appearance", "Attributes", "Equipment"};
    BitmapFont::DrawText(renderer, categoryNames[m_selectedCategoryIndex], MARGIN, y, 2,
                        Color(COLOR_ACCENT_R, COLOR_ACCENT_G, COLOR_ACCENT_B, 255));
    y += LINE_HEIGHT * 2;

    // Show current group
    if (m_selectedGroupIndex < static_cast<int>(m_currentCategoryGroups.size())) {
        CustomizationGroup* group = m_currentCategoryGroups[m_selectedGroupIndex];

        BitmapFont::DrawText(renderer, group->displayName + ":", MARGIN, y, 2,
                           Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
        y += LINE_HEIGHT;

        // Show options
        if (group->id == "character_class" && !group->options.empty()) {
            // Side-scroll carousel: show 3 at a time, centered on selection, wrap-around
            const int visible = 3;
            int n = static_cast<int>(group->options.size());
            int center = m_selectedOptionIndex % n; if (center < 0) center += n;
            int startOffset = -visible/2; // -1, 0, +1 around center
            int baseX = MARGIN + 60;
            int baseY = y;
            int stepX = 200; // horizontal spacing for 3 items
            int tileW = 160, tileH = 36;
            // Arrows as indicators
            BitmapFont::DrawText(renderer, "<", baseX - 30, baseY + 8, 2, Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 200));
            BitmapFont::DrawText(renderer, ">", baseX + (visible - 1) * stepX + 60, baseY + 8, 2, Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 200));
            for (int k = 0; k < visible; ++k) {
                int rel = startOffset + k;
                int idx = (center + rel) % n; if (idx < 0) idx += n;
                const auto& opt = group->options[idx];
                bool selected = (idx == center);
                int x = baseX + k * stepX;
                // Background tiles: faint for non-selected, stronger for selected
                Rectangle tileRect(x - 10, baseY - 6, tileW, tileH);
                Color box = selected ? Color(60, 60, 100, 200) : Color(40, 40, 60, 120);
                GetRenderer()->DrawRectangle(tileRect, box, true);
                // Label
                std::string label = opt.displayName;
                Color col = selected ? Color(COLOR_SELECTED_R, COLOR_SELECTED_G, COLOR_SELECTED_B, 255)
                                     : Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255);
                BitmapFont::DrawText(renderer, label, x, baseY, 1, col);
            }
            // Description of currently centered job (from customization.ini)
            const auto& sel = group->options[center];
            int textY = baseY + LINE_HEIGHT * 2;
            if (!sel.description.empty()) {
                // Simple word-wrap for description
                int maxChars = 64; // rough wrap width per line
                std::istringstream iss(sel.description);
                std::string word, line;
                auto flushLine = [&](){ if (!line.empty()) { BitmapFont::DrawText(renderer, line, MARGIN + 20, textY, 1, Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255)); textY += LINE_HEIGHT; line.clear(); } };
                while (iss >> word) {
                    std::string next = line.empty()? word : (line + " " + word);
                    if ((int)next.size() > maxChars) { flushLine(); line = word; }
                    else { line = next; }
                }
                flushLine();
            }
            BitmapFont::DrawText(renderer, "Left/Right: Change Job   Up/Down: Switch Group   Enter: Select", MARGIN + 20, textY, 1,
                                 Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
            y = textY + LINE_HEIGHT;
        } else {
            for (int i = 0; i < static_cast<int>(group->options.size()); ++i) {
                const auto& option = group->options[i];
                if (i == m_selectedOptionIndex) {
                    BitmapFont::DrawText(renderer, "> " + option.displayName, MARGIN + 20, y, 1,
                                       Color(COLOR_SELECTED_R, COLOR_SELECTED_G, COLOR_SELECTED_B, 255));
                } else if (i == group->selectedIndex) {
                    BitmapFont::DrawText(renderer, "* " + option.displayName, MARGIN + 20, y, 1,
                                       Color(COLOR_ACCENT_R, COLOR_ACCENT_G, COLOR_ACCENT_B, 255));
                } else {
                    BitmapFont::DrawText(renderer, "  " + option.displayName, MARGIN + 20, y, 1,
                                       Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
                }
                y += LINE_HEIGHT;
            }
        }
    }
}

void CustomizationState::RenderNameInput() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    int y = TITLE_HEIGHT + MARGIN;

    BitmapFont::DrawText(renderer, "Enter Character Name:", MARGIN, y, 2,
                        Color(COLOR_ACCENT_R, COLOR_ACCENT_G, COLOR_ACCENT_B, 255));
    y += LINE_HEIGHT * 2;

    // Show current name with cursor
    std::string displayName = m_nameInput;
    if (m_nameInputActive) {
        displayName += "_";  // Simple cursor
    }

    BitmapFont::DrawText(renderer, displayName, MARGIN, y, 2,
                        Color(COLOR_SELECTED_R, COLOR_SELECTED_G, COLOR_SELECTED_B, 255));
}



void CustomizationState::RenderAttributeAdjustment() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    int y = TITLE_HEIGHT + MARGIN;

    BitmapFont::DrawText(renderer, "Attribute Distribution (Left/Right to adjust, Enter to confirm):", MARGIN, y, 2,
                        Color(COLOR_ACCENT_R, COLOR_ACCENT_G, COLOR_ACCENT_B, 255));
    y += LINE_HEIGHT * 2;

    const auto& cz = m_customizationManager->GetPlayerCustomization();

    int sel = m_selectedOptionIndex; // reuse selection index for which attribute is focused: 0..3

    auto drawAttr = [&](const char* label, int value, bool selected){
        std::string prefix = selected?"> ":"  ";
        BitmapFont::DrawText(renderer, prefix + std::string(label) + ": " + std::to_string(value), MARGIN, y, 1,
                             selected?Color(COLOR_SELECTED_R, COLOR_SELECTED_G, COLOR_SELECTED_B, 255):Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
        y += LINE_HEIGHT;
    };
    // Read limits from config (once)
    static bool s_limitsLoaded = false;
    static int s_minAttr = 1, s_maxAttr = 99;
    static int s_totalPoints = 10;
    if (!s_limitsLoaded) {
        ConfigManager cfg;
        if (cfg.LoadFromFile("assets/config/customization.ini") && cfg.HasSection("customization_limits")) {
            s_totalPoints = cfg.Get("customization_limits", "total_attribute_points", s_totalPoints).AsInt();
            s_minAttr = cfg.Get("customization_limits", "min_attribute_value", s_minAttr).AsInt();
            s_maxAttr = cfg.Get("customization_limits", "max_attribute_value", s_maxAttr).AsInt();
        }
        s_limitsLoaded = true;
        // Set available points if still at default
        auto& czInit = m_customizationManager->GetPlayerCustomization();
        if (czInit.availablePoints == 10) {
            // Derive remaining points from total - current base sum


            int baseSum = static_cast<int>(czInit.strength + czInit.agility + czInit.intelligence + czInit.vitality);
            czInit.availablePoints = std::max(0, s_totalPoints - baseSum);
        }
    }


    drawAttr("Strength", static_cast<int>(cz.strength), sel==0);
    drawAttr("Agility", static_cast<int>(cz.agility), sel==1);
    drawAttr("Intelligence", static_cast<int>(cz.intelligence), sel==2);
    drawAttr("Vitality", static_cast<int>(cz.vitality), sel==3);

    y += LINE_HEIGHT;
    BitmapFont::DrawText(renderer, "Available Points: " + std::to_string(cz.availablePoints), MARGIN, y, 1,
                        Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
}

void CustomizationState::RenderConfirmation() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    int y = TITLE_HEIGHT + MARGIN;



    BitmapFont::DrawText(renderer, "Confirm Character:", MARGIN, y, 2,
                        Color(COLOR_ACCENT_R, COLOR_ACCENT_G, COLOR_ACCENT_B, 255));
    y += LINE_HEIGHT * 2;

    const auto& customization = m_customizationManager->GetPlayerCustomization();

    BitmapFont::DrawText(renderer, "Name: " + customization.playerName, MARGIN, y, 1,
                        Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
    y += LINE_HEIGHT;
    BitmapFont::DrawText(renderer, "Class: " + customization.characterClass, MARGIN, y, 1,
                        Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
    y += LINE_HEIGHT * 2;

    BitmapFont::DrawText(renderer, "Press ENTER to start game", MARGIN, y, 1,
                        Color(COLOR_SELECTED_R, COLOR_SELECTED_G, COLOR_SELECTED_B, 255));
    y += LINE_HEIGHT;
    BitmapFont::DrawText(renderer, "Press B to go back", MARGIN, y, 1,
                        Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
}

void CustomizationState::RenderCharacterPreview() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    // Simple character preview area
    int previewX = SCREEN_WIDTH - 200;
    int previewY = TITLE_HEIGHT + MARGIN;
    int previewWidth = 150;
    int previewHeight = 200;
    // Animated sprite preview using SpriteRenderer and GameConfig
    const auto& czLocal = m_customizationManager->GetPlayerCustomization();
    std::string spritePath = czLocal.spritePath.empty() ? std::string("assets/sprites/little_adventurer.png") : czLocal.spritePath;
    if (!m_gameConfig) { m_gameConfig = std::make_unique<GameConfig>(); m_gameConfig->LoadConfigs(); }
    int spriteW = m_gameConfig->GetAnimationSpriteWidth();
    int spriteH = m_gameConfig->GetAnimationSpriteHeight();
    int totalFrames = m_gameConfig->GetAnimationTotalFrames();
    float spriteScale = m_gameConfig->GetAnimationSpriteScale();
    static float s_animT = 0.0f; s_animT += m_gameConfig->GetApproximateFrameTime();
    int frameIndex = totalFrames > 0 ? static_cast<int>(std::fmod(s_animT * 5.0f, static_cast<float>(totalFrames))) : 0;
    SpriteFrame frame = SpriteRenderer::CreateFrame(frameIndex, spriteW, spriteH, totalFrames);
    SpriteRenderer::RenderSprite(renderer, spritePath,
                                 previewX + (previewWidth - static_cast<int>(spriteW * spriteScale)) / 2,
                                 previewY + (previewHeight - static_cast<int>(spriteH * spriteScale)) / 2,
                                 frame, false, spriteScale);


    // Draw preview background
    renderer->DrawRectangle(Rectangle(previewX, previewY, previewWidth, previewHeight),
                           Color(COLOR_PREVIEW_BG_R, COLOR_PREVIEW_BG_G, COLOR_PREVIEW_BG_B, 255), true);

    // Draw simple character representation
    BitmapFont::DrawText(renderer, "Preview", previewX + 10, previewY + 10, 1,
                        Color(COLOR_ACCENT_R, COLOR_ACCENT_G, COLOR_ACCENT_B, 255));

    // Show name/job and derived HP/MP preview
    const auto& cz = m_customizationManager->GetPlayerCustomization();
    std::string job = cz.characterClass;
    CharacterDataRegistry::Get().EnsureLoaded("assets/config/characters.ini");
    CharacterStatsData base = CharacterDataRegistry::Get().GetStats(job.empty()?"player":job);
    int infoY = previewY + previewHeight + 8;
    BitmapFont::DrawText(renderer, "Name: " + cz.playerName, previewX, infoY, 1, Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
    infoY += LINE_HEIGHT;
    BitmapFont::DrawText(renderer, "Job: " + job, previewX, infoY, 1, Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
    infoY += LINE_HEIGHT;
    BitmapFont::DrawText(renderer, "HP: " + std::to_string((int)base.maxHealth) + "  MP: " + std::to_string((int)base.maxMana), previewX, infoY, 1,
                         Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));

    // TODO: Render actual character sprite based on customization
    renderer->DrawRectangle(Rectangle(previewX + 60, previewY + 80, 30, 60),
                           Color(COLOR_SELECTED_R, COLOR_SELECTED_G, COLOR_SELECTED_B, 255), true);  // Simple body
    renderer->DrawRectangle(Rectangle(previewX + 65, previewY + 60, 20, 20),
                           Color(COLOR_SELECTED_R, COLOR_SELECTED_G, COLOR_SELECTED_B, 255), true);  // Simple head
}

void CustomizationState::RenderInstructions() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    int y = SCREEN_HEIGHT - 80;

    BitmapFont::DrawText(renderer, "Controls: Arrow Keys - Navigate, Enter - Select, B - Back", MARGIN, y, 1,
                        Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
    y += LINE_HEIGHT;
    BitmapFont::DrawText(renderer, "C - Confirm and Start Game, ESC - Return to Menu", MARGIN, y, 1,
                        Color(COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 255));
}

void CustomizationState::HandleCategoryInput() {
    auto* input = GetInputManager();
    if (!input) return;

    static bool upWas=false, downWas=false;
    bool upNow = input->IsKeyPressed(SDL_SCANCODE_UP);
    bool downNow = input->IsKeyPressed(SDL_SCANCODE_DOWN);

    if (upNow && !upWas) {
        m_selectedCategoryIndex = std::max(0, m_selectedCategoryIndex - 1);
        if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
    } else if (downNow && !downWas) {
        m_selectedCategoryIndex = std::min(static_cast<int>(m_categories.size()) - 1, m_selectedCategoryIndex + 1);
        if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
    } else if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN)) {
        SelectCategory(m_selectedCategoryIndex);
        m_currentMode = UIMode::OPTION_SELECTION;
        if (GetEngine()->GetAudioManager()) { GetEngine()->GetAudioManager()->PlaySound("menu_select", 0.9f); }
    } else if (input->IsKeyJustPressed(SDL_SCANCODE_N) && m_categories[m_selectedCategoryIndex] == CustomizationCategory::BASIC_INFO) {
        // Quick access to name input from Basic Info
        m_currentMode = UIMode::NAME_INPUT;
        m_nameInputActive = true;
        if (GetEngine()->GetAudioManager()) { GetEngine()->GetAudioManager()->PlaySound("menu_select", 0.9f); }
    } else if (input->IsKeyJustPressed(SDL_SCANCODE_C)) {
        // Quick confirm - go to confirmation screen
        m_currentMode = UIMode::CONFIRMATION;
    }

    upWas = upNow; downWas = downNow;
}

void CustomizationState::HandleOptionInput() {
    auto* input = GetInputManager();
    if (!input) return;

    if (m_currentCategoryGroups.empty()) return;

    static bool upWas=false, downWas=false, leftWas=false, rightWas=false;
    bool upNow = input->IsKeyPressed(SDL_SCANCODE_UP);
    bool downNow = input->IsKeyPressed(SDL_SCANCODE_DOWN);
    bool leftNow = input->IsKeyPressed(SDL_SCANCODE_LEFT);
    bool rightNow = input->IsKeyPressed(SDL_SCANCODE_RIGHT);

    auto* group = m_currentCategoryGroups[m_selectedGroupIndex];

    if (group && group->id == "character_class") {
        // Carousel navigation: Left/Right cycles options; Up/Down switches group
        if (leftNow && !leftWas) {
            int n = static_cast<int>(group->options.size());
            m_selectedOptionIndex = (m_selectedOptionIndex - 1 + n) % n;
            if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
        } else if (rightNow && !rightWas) {
            int n = static_cast<int>(group->options.size());
            m_selectedOptionIndex = (m_selectedOptionIndex + 1) % n;
            if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
        } else if (upNow && !upWas) {
            m_selectedGroupIndex = std::max(0, m_selectedGroupIndex - 1);
            m_selectedOptionIndex = 0;
            if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
        } else if (downNow && !downWas) {
            m_selectedGroupIndex = std::min(static_cast<int>(m_currentCategoryGroups.size()) - 1, m_selectedGroupIndex + 1);
            m_selectedOptionIndex = 0;
            if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
        } else if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN)) {
            ApplyCurrentSelection();
            if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_select", 0.9f);
        }
    } else {
        if (upNow && !upWas) {
            if (m_selectedGroupIndex < static_cast<int>(m_currentCategoryGroups.size())) {
                m_selectedOptionIndex = std::max(0, m_selectedOptionIndex - 1);
                if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
            }
        } else if (downNow && !downWas) {
            if (m_selectedGroupIndex < static_cast<int>(m_currentCategoryGroups.size())) {
                int maxIdx = std::max(0, static_cast<int>(group->options.size()) - 1);
                m_selectedOptionIndex = std::min(maxIdx, m_selectedOptionIndex + 1);
                if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
            }
        } else if (leftNow && !leftWas) {
            m_selectedGroupIndex = std::max(0, m_selectedGroupIndex - 1);
            m_selectedOptionIndex = 0;
            if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
        } else if (rightNow && !rightWas) {
            m_selectedGroupIndex = std::min(static_cast<int>(m_currentCategoryGroups.size()) - 1, m_selectedGroupIndex + 1);
            m_selectedOptionIndex = 0;
            if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
        } else if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN)) {
            ApplyCurrentSelection();
            if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_select", 0.9f);
        }
    }

    upWas=upNow; downWas=downNow; leftWas=leftNow; rightWas=rightNow;
}

void CustomizationState::HandleNameInputKeys() {
    auto* input = GetInputManager();
    if (!input) return;

    if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN)) {
        m_nameInputActive = false;
        m_currentMode = UIMode::CATEGORY_SELECTION;
        return;
    }

    // Handle text input (simplified)
    if (input->IsKeyJustPressed(SDL_SCANCODE_BACKSPACE) && !m_nameInput.empty()) {
        m_nameInput.pop_back();
    }

    // Add basic character input (A-Z, space)
    for (int scancode = SDL_SCANCODE_A; scancode <= SDL_SCANCODE_Z; ++scancode) {
        if (input->IsKeyJustPressed(static_cast<SDL_Scancode>(scancode))) {
            if (m_nameInput.length() < 20) {  // Limit name length
                char c = 'A' + (scancode - SDL_SCANCODE_A);
                if (!input->IsKeyPressed(SDL_SCANCODE_LSHIFT) && !input->IsKeyPressed(SDL_SCANCODE_RSHIFT)) {
                    c = c - 'A' + 'a';  // Convert to lowercase
                }
                m_nameInput += c;
            }
        }
    }

    if (input->IsKeyJustPressed(SDL_SCANCODE_SPACE) && m_nameInput.length() < 20) {
        m_nameInput += ' ';
    }
}

void CustomizationState::HandleAttributeInput() {
    auto* input = GetInputManager();
    if (!input) return;

    static bool upWas=false, downWas=false, leftWas=false, rightWas=false;
    bool upNow = input->IsKeyPressed(SDL_SCANCODE_UP);
    bool downNow = input->IsKeyPressed(SDL_SCANCODE_DOWN);
    bool leftNow = input->IsKeyPressed(SDL_SCANCODE_LEFT);
    bool rightNow = input->IsKeyPressed(SDL_SCANCODE_RIGHT);

    auto& cz = m_customizationManager->GetPlayerCustomization();

    // Navigate attribute focus (0..3)
    if (upNow && !upWas) {
        m_selectedOptionIndex = std::max(0, m_selectedOptionIndex - 1);
        if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
    } else if (downNow && !downWas) {
        m_selectedOptionIndex = std::min(3, m_selectedOptionIndex + 1);
        if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
    } else if (leftNow && !leftWas) {
        // Enforce min attribute; refund a point if decreased
        int* ptrs[4] = { reinterpret_cast<int*>(&cz.strength), reinterpret_cast<int*>(&cz.agility), reinterpret_cast<int*>(&cz.intelligence), reinterpret_cast<int*>(&cz.vitality) };
        int& v = *ptrs[m_selectedOptionIndex];
        if (v > s_minAttr) { v -= 1; cz.availablePoints += 1; }
        if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
    } else if (rightNow && !rightWas) {
        // Enforce max attribute and available points
        if (cz.availablePoints > 0) {
            int* ptrs[4] = { reinterpret_cast<int*>(&cz.strength), reinterpret_cast<int*>(&cz.agility), reinterpret_cast<int*>(&cz.intelligence), reinterpret_cast<int*>(&cz.vitality) };
            int& v = *ptrs[m_selectedOptionIndex];
            if (v < s_maxAttr) { v += 1; cz.availablePoints -= 1; if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f); }
        }
    } else if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN)) {
        m_currentMode = UIMode::CATEGORY_SELECTION;
        if (GetEngine()->GetAudioManager()) GetEngine()->GetAudioManager()->PlaySound("menu_select", 0.9f);
    }

    upWas=upNow; downWas=downNow; leftWas=leftNow; rightWas=rightNow;
}

void CustomizationState::HandleConfirmationInput() {
    auto* input = GetInputManager();
    if (!input) return;

    if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN)) {
        if (GetEngine()->GetAudioManager()) {
            GetEngine()->GetAudioManager()->LoadSound("game_start", "assets/music/game-start-6104.mp3", SoundType::SOUND_EFFECT);
            GetEngine()->GetAudioManager()->PlaySound("game_start", 1.0f);
        }
        StartGame();
    }
    // Seed starting inventory based on config mapping
    auto& inv = InventoryManager::Get();
    inv.LoadItemsConfig();
    std::string jobId = m_customizationManager->GetPlayerCustomization().characterClass;
    std::string jl;
    jl.resize(jobId.size());
    std::transform(jobId.begin(), jobId.end(), jl.begin(), [](unsigned char c){ return std::tolower(c); });

    ConfigManager cfg;
    if (cfg.LoadFromFile("assets/config/customization.ini") && cfg.HasSection("starting_items")) {
        const auto& sect = cfg.GetSections().at("starting_items").GetAll();
        auto applyLine = [&](const std::string& line){
            // Comma-separated id:qty pairs
            size_t start = 0;
            while (start < line.size()) {
                size_t comma = line.find(',', start);
                std::string token = line.substr(start, comma == std::string::npos ? std::string::npos : comma - start);
                size_t colon = token.find(':');
                if (colon != std::string::npos) {
                    std::string id = token.substr(0, colon);
                    int qty = std::max(1, std::atoi(token.substr(colon + 1).c_str()));
                    inv.AddItem(id, qty);
                }
                if (comma == std::string::npos) break;
                start = comma + 1;
            }
        };

        bool matched = false;
        // First try family keys by substring match
        static const char* families[] = {"psionic","caster","warrior","archer","rogue","support"};
        for (const char* fam : families) {
            if (jl.find(fam) != std::string::npos) {
                auto it = sect.find(fam);
                if (it != sect.end()) { applyLine(it->second.AsString()); matched = true; break; }
            }
        }
        // If still not matched, try exact jobId
        if (!matched) {
            auto it = sect.find(jl);
            if (it != sect.end()) { applyLine(it->second.AsString()); matched = true; }
        }
        // Fallback to default
        if (!matched) {
            auto it = sect.find("default");
            if (it != sect.end()) { applyLine(it->second.AsString()); }
        }
    } else {
        inv.AddItem("potion", 2);
    }

}

void CustomizationState::SelectCategory(int index) {
    if (index < 0 || index >= static_cast<int>(m_categories.size())) return;

    m_selectedCategoryIndex = index;
    m_selectedGroupIndex = 0;
    m_selectedOptionIndex = 0;

    // Load groups for this category
    m_currentCategoryGroups = m_customizationManager->GetGroupsByCategory(m_categories[index]);
}

void CustomizationState::SelectOption(int groupIndex, int optionIndex) {
    if (groupIndex < 0 || groupIndex >= static_cast<int>(m_currentCategoryGroups.size())) return;

    CustomizationGroup* group = m_currentCategoryGroups[groupIndex];
    if (optionIndex < 0 || optionIndex >= static_cast<int>(group->options.size())) return;

    m_customizationManager->ApplyGroupSelection(group->id, optionIndex);
}

void CustomizationState::ApplyCurrentSelection() {
    if (m_selectedGroupIndex < static_cast<int>(m_currentCategoryGroups.size())) {
        SelectOption(m_selectedGroupIndex, m_selectedOptionIndex);
    }
}

void CustomizationState::StartGame() {
    std::cout << "Starting game with customized character..." << std::endl;

    // Apply all customizations to local manager
    m_customizationManager->ApplyCustomizationToPlayerData();

    // Copy customization data to the global instance
    CustomizationManager::GetInstance().GetPlayerCustomization() = m_customizationManager->GetPlayerCustomization();
    // Persist chosen sprite/skin into characters.ini before leaving
    {
        const auto& pc = m_customizationManager->GetPlayerCustomization();
        ConfigManager chars;
        if (chars.LoadFromFile("assets/config/characters.ini")) {
            // write bare filename to match existing convention
            std::string filename = pc.spritePath;
            size_t slash = filename.find_last_of("/\\");
            if (slash != std::string::npos) filename = filename.substr(slash + 1);
            chars.Set("player", "sprite_path", ConfigValue(filename));
            if (chars.SaveToFile("assets/config/characters.ini")) {
                std::cout << "💾 Saved player sprite to characters.ini: " << filename << std::endl;
            } else {
                std::cout << "❌ Failed to save characters.ini" << std::endl;
            }
        } else {
            std::cout << "⚠️ Could not load characters.ini to save sprite selection" << std::endl;
        }
    }

    CustomizationManager::GetInstance().ApplyCustomizationToPlayerData();

    // Validate customization
    if (!m_customizationManager->ValidateCustomization()) {
        std::cout << "Invalid customization - cannot start game" << std::endl;
        return;
    }

    std::cout << "Customization applied globally for: "
              << CustomizationManager::GetInstance().GetPlayerCustomization().playerName << std::endl;

    // Transition to playing state
    if (GetStateManager()) {
        GetStateManager()->ChangeState(GameStateType::PLAYING);
    // Reset run total when starting a fresh game
    if (auto* ps = dynamic_cast<PlayingState*>(GetStateManager()->GetState(GameStateType::PLAYING))) {
        ps->ResetRunTotal();
    }

    }
}

void CustomizationState::ReturnToMenu() {
    std::cout << "Returning to menu..." << std::endl;
    if (GetStateManager()) {
        GetStateManager()->ChangeState(GameStateType::MENU);
    }
}
