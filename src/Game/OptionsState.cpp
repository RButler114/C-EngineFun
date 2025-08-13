/**
#include "Engine/ConfigSystem.h"
#include "Engine/Window.h"

 * @file OptionsState.cpp
 * @brief Simple options menu implementation for arcade games
 * @author Ryan Butler
 * @date 2025
 */

#include "Engine/ConfigSystem.h"
#include "Engine/Window.h"

#include "Game/OptionsState.h"
#include "Game/GameStateManager.h"
#include "Engine/Renderer.h"
#include "Engine/InputManager.h"
#include "Engine/Engine.h"
#include "Engine/AudioManager.h"
#include "Engine/BitmapFont.h"
#include "Engine/KeybindingManager.h"
#include <iostream>
#include <algorithm>
#include <cmath>
static void LoadVisualSnapshot(bool& fs, bool& vsync, int& w, int& h, int& fps) {
    ConfigManager cfg; if (cfg.LoadFromFile("assets/config/gameplay.ini")) {
        fs = cfg.Get("visual","fullscreen", false).AsBool();
        vsync = cfg.Get("visual","vsync", true).AsBool();
        w = cfg.Get("visual","screen_width", 800).AsInt();
        h = cfg.Get("visual","screen_height", 600).AsInt();
        fps = cfg.Get("visual","fps_limit", 60).AsInt();
    }
}


OptionsState::OptionsState()
    : GameState(GameStateType::OPTIONS, "Options")
    , m_selectedOption(0)
    , m_blinkTimer(0.0f)
    , m_showSelection(true)
    , m_musicVolume(0.5f)
    , m_soundVolume(0.7f)
    , m_resolutions{}
    , m_resolutionIndex(0)
    , m_fullscreen(false)
    , m_keybindingManager(std::make_unique<KeybindingManager>())
    , m_inKeybindingMode(false)
    , m_selectedKeybinding(0)
    , m_waitingForKey(false)
    , m_keyToRebind(GameAction::ACTION_COUNT)
    , m_rebindingPrimary(true)
    , m_vsync(true)
    , m_fpsOptions{0, 30, 60, 120, 144}
    , m_fpsIndex(2)

    , m_keybindingScrollOffset(0)
    , m_maxVisibleKeybindings(0) {

    InitializeOptions();
}

OptionsState::~OptionsState() = default;

void OptionsState::OnEnter() {
    std::cout << "Entering Options State" << std::endl;

    // Get current audio settings
    if (GetEngine()->GetAudioManager()) {
        m_musicVolume = GetEngine()->GetAudioManager()->GetMusicVolume();
        m_soundVolume = GetEngine()->GetAudioManager()->GetCategoryVolume(SoundType::SOUND_EFFECT);
    }

    // Load keybindings
    if (!m_keybindingManager->LoadFromConfig("assets/config/keybindings.ini")) {
        std::cout << "Using default keybindings" << std::endl;
    }

    // Get configurable actions
    m_configurableActions = m_keybindingManager->GetConfigurableActions();

    // Calculate layout
    CalculateKeybindingLayout();

    // Reset selection
    m_selectedOption = 0;
    m_selectedKeybinding = 0;
    m_keybindingScrollOffset = 0;
    m_inKeybindingMode = false;
    m_waitingForKey = false;
    m_blinkTimer = 0.0f;
    m_showSelection = true;

    // Load menu UI sounds
    if (GetEngine()->GetAudioManager()) {
        auto* am = GetEngine()->GetAudioManager();
        am->LoadSound("menu_nav", "assets/music/clicking-interface-select-201946.mp3", SoundType::SOUND_EFFECT);
        am->LoadSound("menu_select", "assets/music/select-001-337218.mp3", SoundType::SOUND_EFFECT);
        am->LoadSound("menu_back", "assets/music/select-003-337609.mp3", SoundType::SOUND_EFFECT);
    }
}

void OptionsState::OnExit() {
    std::cout << "Exiting Options State" << std::endl;
}

void OptionsState::Update(float deltaTime) {
    HandleInput();

    // Update selection blink effect
    m_blinkTimer += deltaTime;
    if (m_blinkTimer >= 0.5f) {
    // Auto-revert timer for pending video changes
    if (m_videoAwaitingConfirm) {
        m_videoRevertTimer -= deltaTime;
        if (m_videoRevertTimer <= 0.0f) {
            RevertVideoChanges();
        }
    }

        m_showSelection = !m_showSelection;
        m_blinkTimer = 0.0f;
    }
}

void OptionsState::Render() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    DrawBackground();
    DrawTitle();

    if (m_inKeybindingMode) {
        DrawKeybindings();
    } else {
        DrawOptions();
    }

    DrawInstructions();
}

void OptionsState::HandleInput() {
    auto* input = GetInputManager();
    if (!input) return;

    if (m_inKeybindingMode) {
        HandleKeybindingInput();
    } else {
        // Navigation - using IsKeyPressed with debouncing since IsKeyJustPressed has issues
        static bool upWasPressed = false;
    // Section jump shortcuts: PageUp/PageDown
    if (input->IsKeyJustPressed(SDL_SCANCODE_PAGEUP)) {
        for (int i = m_selectedOption - 1; i >= 0; --i) {
            if (m_optionTypes[i] == OptionType::HEADER_VIDEO || m_optionTypes[i] == OptionType::HEADER_AUDIO || m_optionTypes[i] == OptionType::HEADER_CONTROLS) {
                m_selectedOption = i;
                break;
            }
        }
    }
    if (input->IsKeyJustPressed(SDL_SCANCODE_PAGEDOWN)) {
        for (int i = m_selectedOption + 1; i < GetOptionCount(); ++i) {
            if (m_optionTypes[i] == OptionType::HEADER_VIDEO || m_optionTypes[i] == OptionType::HEADER_AUDIO || m_optionTypes[i] == OptionType::HEADER_CONTROLS) {
                m_selectedOption = i;
                break;
            }
        }
    }

        static bool downWasPressed = false;
        static bool wWasPressed = false;
        static bool sWasPressed = false;

        bool upPressed = input->IsKeyPressed(SDL_SCANCODE_UP);
        bool downPressed = input->IsKeyPressed(SDL_SCANCODE_DOWN);
        bool wPressed = input->IsKeyPressed(SDL_SCANCODE_W);
        bool sPressed = input->IsKeyPressed(SDL_SCANCODE_S);

        if ((upPressed && !upWasPressed) || (wPressed && !wWasPressed)) {
            NavigateUp();
        }
        if ((downPressed && !downWasPressed) || (sPressed && !sWasPressed)) {
            NavigateDown();
        }

        upWasPressed = upPressed;
        downWasPressed = downPressed;
        wWasPressed = wPressed;
        sWasPressed = sPressed;

        // Value adjustment - using IsKeyPressed with debouncing since IsKeyJustPressed has issues
        static bool leftWasPressed = false;
        static bool rightWasPressed = false;
        static bool aWasPressed = false;
        static bool dWasPressed = false;

        bool leftPressed = input->IsKeyPressed(SDL_SCANCODE_LEFT);
        bool rightPressed = input->IsKeyPressed(SDL_SCANCODE_RIGHT);
        bool aPressed = input->IsKeyPressed(SDL_SCANCODE_A);
        bool dPressed = input->IsKeyPressed(SDL_SCANCODE_D);

        if ((leftPressed && !leftWasPressed) || (aPressed && !aWasPressed)) {
            AdjustLeft();
        }
        if ((rightPressed && !rightWasPressed) || (dPressed && !dWasPressed)) {
            AdjustRight();
        }

        leftWasPressed = leftPressed;
        rightWasPressed = rightPressed;
        aWasPressed = aPressed;
        dWasPressed = dPressed;

        // Selection - using IsKeyPressed with debouncing since IsKeyJustPressed has issues
        static bool enterWasPressed = false;
        static bool spaceWasPressed = false;

        bool enterPressed = input->IsKeyPressed(SDL_SCANCODE_RETURN);
        bool spacePressed = input->IsKeyPressed(SDL_SCANCODE_SPACE);

        if ((enterPressed && !enterWasPressed) || (spacePressed && !spaceWasPressed)) {
            SelectOption();
        }

        enterWasPressed = enterPressed;
        spaceWasPressed = spacePressed;

        // Back - use B key for consistent navigation
        static bool bWasPressed = false;
        static bool escapeWasPressed = false;

        bool bPressed = input->IsKeyPressed(SDL_SCANCODE_B);
        bool escapePressed = input->IsKeyPressed(SDL_SCANCODE_ESCAPE);

        if ((bPressed && !bWasPressed) || (escapePressed && !escapeWasPressed)) {
            GoBack();
        }

        bWasPressed = bPressed;
        escapeWasPressed = escapePressed;

    }
}

void OptionsState::NavigateUp() {
    // UP key should move to visually higher option (lower index)
    m_selectedOption--;
    if (m_selectedOption < 0) {
        m_selectedOption = GetOptionCount() - 1;
    }
    m_showSelection = true;
    m_blinkTimer = 0.0f;

    if (GetEngine()->GetAudioManager()) {
        GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
    }
    // Keep selected option within visible window
    if (auto* r = GetRenderer()) {
        int lw=0, lh=0; r->GetLogicalSize(lw, lh);
        int spacing = (int)(60 * BitmapFont::GetGlobalScale());
        int maxVisible = std::max(1, (lh - (int)(200*BitmapFont::GetGlobalScale()) - (int)(120*BitmapFont::GetGlobalScale())) / spacing);
        if (m_selectedOption < m_optionsScrollOffset) m_optionsScrollOffset = m_selectedOption;
        if (m_selectedOption >= m_optionsScrollOffset + maxVisible)
            m_optionsScrollOffset = m_selectedOption - maxVisible + 1;
        if (m_optionsScrollOffset < 0) m_optionsScrollOffset = 0;
        if (m_optionsScrollOffset > std::max(0, GetOptionCount() - maxVisible))
            m_optionsScrollOffset = std::max(0, GetOptionCount() - maxVisible);
    }
}

void OptionsState::NavigateDown() {
    // DOWN key should move to visually lower option (higher index)
    m_selectedOption++;
    if (m_selectedOption >= GetOptionCount()) {
        m_selectedOption = 0;
    }
    m_showSelection = true;
    m_blinkTimer = 0.0f;

    if (GetEngine()->GetAudioManager()) {
        GetEngine()->GetAudioManager()->PlaySound("menu_nav", 0.7f);
    }
}

void OptionsState::AdjustLeft() {
    auto* audioManager = GetEngine()->GetAudioManager();
    if (!audioManager) return;

    if (m_selectedOption >= 0 && m_selectedOption < GetOptionCount()) {
        // Skip headers
        if (m_optionTypes[m_selectedOption] == OptionType::HEADER_VIDEO ||
            m_optionTypes[m_selectedOption] == OptionType::HEADER_AUDIO ||
            m_optionTypes[m_selectedOption] == OptionType::HEADER_CONTROLS) {
            return;
        }

    // Keep selected option within visible window
    if (auto* r = GetRenderer()) {
        int lw=0, lh=0; r->GetLogicalSize(lw, lh);
        int spacing = (int)(60 * BitmapFont::GetGlobalScale());
        int maxVisible = std::max(1, (lh - (int)(200*BitmapFont::GetGlobalScale()) - (int)(120*BitmapFont::GetGlobalScale())) / spacing);
        if (m_selectedOption < m_optionsScrollOffset) m_optionsScrollOffset = m_selectedOption;
        if (m_selectedOption >= m_optionsScrollOffset + maxVisible)
            m_optionsScrollOffset = m_selectedOption - maxVisible + 1;
        if (m_optionsScrollOffset < 0) m_optionsScrollOffset = 0;
        if (m_optionsScrollOffset > std::max(0, GetOptionCount() - maxVisible))
            m_optionsScrollOffset = std::max(0, GetOptionCount() - maxVisible);
    }

        OptionType type = m_optionTypes[m_selectedOption];

        switch (type) {
            case OptionType::RESOLUTION:
                if (!m_resolutions.empty()) {
                    int n = static_cast<int>(m_resolutions.size());
                    m_resolutionIndex = (m_resolutionIndex - 1 + n) % n;
                }
                break;
            case OptionType::FULLSCREEN:
                m_fullscreen = !m_fullscreen;
                break;
            case OptionType::VSYNC:
                m_vsync = !m_vsync;
                break;
            case OptionType::FPS_LIMIT:
                if (!m_fpsOptions.empty()) {
                    int n = static_cast<int>(m_fpsOptions.size());
                    m_fpsIndex = (m_fpsIndex - 1 + n) % n;
                }
                break;
            case OptionType::VOLUME_MUSIC:
                m_musicVolume = std::max(0.0f, m_musicVolume - 0.1f);
                audioManager->SetMusicVolume(m_musicVolume);
                break;
            case OptionType::VOLUME_SOUND:
                m_soundVolume = std::max(0.0f, m_soundVolume - 0.1f);
                audioManager->SetCategoryVolume(SoundType::SOUND_EFFECT, m_soundVolume);
                audioManager->PlaySound("menu_select", m_soundVolume);
                break;
            default:
    // Keep selected option within visible window
    if (auto* r = GetRenderer()) {
        int lw=0, lh=0; r->GetLogicalSize(lw, lh);
        int spacing = (int)(60 * BitmapFont::GetGlobalScale());
        int maxVisible = std::max(1, (lh - (int)(200*BitmapFont::GetGlobalScale()) - (int)(120*BitmapFont::GetGlobalScale())) / spacing);
        if (m_selectedOption < m_optionsScrollOffset) m_optionsScrollOffset = m_selectedOption;
        if (m_selectedOption >= m_optionsScrollOffset + maxVisible)
            m_optionsScrollOffset = m_selectedOption - maxVisible + 1;
        if (m_optionsScrollOffset < 0) m_optionsScrollOffset = 0;
        if (m_optionsScrollOffset > std::max(0, GetOptionCount() - maxVisible))
            m_optionsScrollOffset = std::max(0, GetOptionCount() - maxVisible);
    }

                break;
        }
    }
}

void OptionsState::AdjustRight() {
    auto* audioManager = GetEngine()->GetAudioManager();
    if (!audioManager) return;

    if (m_selectedOption >= 0 && m_selectedOption < GetOptionCount()) {
        // Skip headers
        if (m_optionTypes[m_selectedOption] == OptionType::HEADER_VIDEO ||
            m_optionTypes[m_selectedOption] == OptionType::HEADER_AUDIO ||
            m_optionTypes[m_selectedOption] == OptionType::HEADER_CONTROLS) {
            return;
        }

        OptionType type = m_optionTypes[m_selectedOption];

        switch (type) {
            case OptionType::RESOLUTION:
                if (!m_resolutions.empty()) {
                    int n = static_cast<int>(m_resolutions.size());
                    m_resolutionIndex = (m_resolutionIndex + 1) % n;
                }
                break;
            case OptionType::FULLSCREEN:
                m_fullscreen = !m_fullscreen;
                break;
            case OptionType::VSYNC:
                m_vsync = !m_vsync;
                break;
            case OptionType::FPS_LIMIT:
                if (!m_fpsOptions.empty()) {
                    int n = static_cast<int>(m_fpsOptions.size());
                    m_fpsIndex = (m_fpsIndex + 1) % n;
                }
                break;
            case OptionType::VOLUME_MUSIC:
                m_musicVolume = std::min(1.0f, m_musicVolume + 0.1f);
                audioManager->SetMusicVolume(m_musicVolume);
                break;
            case OptionType::VOLUME_SOUND:
                m_soundVolume = std::min(1.0f, m_soundVolume + 0.1f);
                audioManager->SetCategoryVolume(SoundType::SOUND_EFFECT, m_soundVolume);
                audioManager->PlaySound("menu_select", m_soundVolume);
                break;
            default:
                break;
        }
    }
}

void OptionsState::SelectOption() {
    // Ensure we have valid bounds
    if (m_selectedOption >= 0 && m_selectedOption < GetOptionCount()) {

        OptionType type = m_optionTypes[m_selectedOption];

        // Skip headers - no action on ENTER
        if (type == OptionType::HEADER_VIDEO || type == OptionType::HEADER_AUDIO || type == OptionType::HEADER_CONTROLS) {
            return;
        }

        switch (type) {
            case OptionType::KEYBINDINGS:
                EnterKeybindingMode();
                break;
            case OptionType::BACK_TO_MENU:
                GoBack();
                break;
            case OptionType::RESOLUTION:
            case OptionType::FULLSCREEN:
            case OptionType::VSYNC:
            case OptionType::FPS_LIMIT:
                // Mark pending; apply after user confirms
                m_videoPendingChanges = true;
                break;
            case OptionType::APPLY_CHANGES:
                ApplyVideoChanges();
                break;
            case OptionType::CANCEL_CHANGES:
                RevertVideoChanges();
                break;
            case OptionType::CONFIRM_CHANGES:
                // Finalize applied changes by cancelling the auto-revert window
                m_videoAwaitingConfirm = false;
                m_videoRevertTimer = 0.0f;
                m_hasPrevSnapshot = false; // snapshot no longer needed
                break;
            case OptionType::STANDARDIZE:
                StandardizeVideoSettings();
                break;
            case OptionType::VOLUME_MUSIC:
                if (GetEngine()->GetAudioManager()) {
                    GetEngine()->GetAudioManager()->PlaySound("menu_select", 0.7f);
                }
                break;
            case OptionType::VOLUME_SOUND:
                if (GetEngine()->GetAudioManager()) {
                    GetEngine()->GetAudioManager()->PlaySound("menu_select", m_soundVolume);
                }
                break;
            default:
                break;
        }
    }
}

void OptionsState::GoBack() {
    if (GetEngine()->GetAudioManager()) {
        GetEngine()->GetAudioManager()->PlaySound("menu_back", 0.9f);
    }
    if (GetStateManager()) {
        GetStateManager()->PopState();
    }
}

void OptionsState::DrawBackground() {

    auto* renderer = GetRenderer();

    // Simple gradient background (based on logical size)
    int logicalW = 800, logicalH = 600;
    // We can draw within logical coordinates; background lines up to H
    for (int y = 0; y < logicalH; y++) {
        int intensity = 20 + (y * 30) / logicalH;
        Color bgColor(intensity, intensity, intensity + 10, 255);
        renderer->DrawLine(0, y, logicalW, y, bgColor);
    }
}

void OptionsState::DrawTitle() {
    auto* renderer = GetRenderer();

    float uiScale = BitmapFont::GetGlobalScale();
    // Title
    BitmapFont::DrawText(renderer, "OPTIONS", (int)(320*uiScale), (int)(100*uiScale), (int)std::max(1.0f, 3*uiScale), Color(255, 255, 100, 255));
}

void OptionsState::DrawOptions() {
    auto* renderer = GetRenderer();

    // Scale/layout by UI scale and logical size
    float uiScale = BitmapFont::GetGlobalScale();
    int lw=0, lh=0; renderer->GetLogicalSize(lw, lh);
    int spacing = (int)(60 * uiScale);

    // Compute visible range with scrolling
    int optionCount = GetOptionCount();
    int maxVisible = std::max(1, (lh - (int)(200*uiScale) - (int)(120*uiScale)) / spacing);
    m_maxVisibleOptions = maxVisible;
    m_optionsScrollOffset = std::min(m_optionsScrollOffset, std::max(0, optionCount - maxVisible));

    int blockHeight = std::min(optionCount, maxVisible) * spacing;
    int startY = (lh - blockHeight) / 2; // center vertically
    int startX = (int)(std::max(20.0f, (lw - 600*uiScale) / 2)); // center-ish horizontally with margin

    for (int i = 0; i < optionCount; i++) {
        int visibleIndex = i - m_optionsScrollOffset;
        if (visibleIndex < 0 || visibleIndex >= maxVisible) continue;
        int y = startY + visibleIndex * spacing;
        bool isSelected = (i == m_selectedOption);

        Color textColor = isSelected && m_showSelection ?
            Color(255, 255, 100, 255) : Color(200, 200, 200, 255);

        std::string displayText = m_options[i];
        OptionType type = m_optionTypes[i];

        // Headers: style differently and skip value brackets
        bool isHeader = (type == OptionType::HEADER_VIDEO || type == OptionType::HEADER_AUDIO || type == OptionType::HEADER_CONTROLS);
        int textScale = isHeader ? 2 : 2;
        Color headerColor(180, 220, 255, 255);

        if (!isHeader) {
            // Add value display for non-header options
            switch (type) {
                case OptionType::VOLUME_MUSIC:
                    displayText += GetVolumeDisplayText(m_musicVolume);
                    break;
                case OptionType::VOLUME_SOUND:
                    displayText += GetVolumeDisplayText(m_soundVolume);
                    break;
                case OptionType::RESOLUTION:
                    if (!m_resolutions.empty()) {
                        const auto& r = m_resolutions[std::clamp(m_resolutionIndex, 0, (int)m_resolutions.size()-1)];
                        displayText += "  [" + std::to_string(r.w) + "x" + std::to_string(r.h) + "]";
                        if (r.w == 800 && r.h == 600) { displayText += "  (Standard)"; }
                    }
                    break;
                case OptionType::FULLSCREEN:
                    displayText += m_fullscreen ? "  [On]" : "  [Off]";
                    break;
                case OptionType::VSYNC:
                    displayText += m_vsync ? "  [On]" : "  [Off]";
                    break;
                case OptionType::FPS_LIMIT:
                    if (!m_fpsOptions.empty()) {
                        int fps = m_fpsOptions[std::clamp(m_fpsIndex, 0, (int)m_fpsOptions.size()-1)];
                        displayText += fps > 0 ? ("  [" + std::to_string(fps) + "]") : "  [Unlimited]";
                    }
                    break;
                default:
                    break;
            }
        }

        // Draw
        if (isHeader) {
            BitmapFont::DrawText(renderer, displayText, startX - 20, y, textScale, headerColor);
        } else {
            BitmapFont::DrawText(renderer, displayText, startX, y, textScale, textColor);
        }

        // Draw selection indicator only on interactive
        if (!isHeader && isSelected && m_showSelection) {
            BitmapFont::DrawText(renderer, ">", 170, y, textScale, Color(255, 255, 100, 255));
        }
    }
}

void OptionsState::DrawInstructions() {
    auto* renderer = GetRenderer();

    float uiScale = BitmapFont::GetGlobalScale();
    int instructionY = (int)(500 * uiScale);  // Start position for instructions

    // Row-specific tips
    std::string tip;
    if (m_selectedOption >= 0 && m_selectedOption < GetOptionCount()) {
        switch (m_optionTypes[m_selectedOption]) {
            case OptionType::RESOLUTION:
                tip = "Left/Right: Change resolution  Enter: Apply"; break;
            case OptionType::FULLSCREEN:
                tip = "Enter: Toggle fullscreen"; break;
            case OptionType::VSYNC:
                tip = "Enter: Toggle VSync (recreates renderer)"; break;
            case OptionType::FPS_LIMIT:
                tip = "Left/Right: Change FPS cap  Enter: Apply"; break;
            case OptionType::APPLY_CHANGES:
                tip = "Enter: Apply pending video changes (starts confirm timer)"; break;
            case OptionType::CANCEL_CHANGES:
                tip = "Enter: Cancel pending or revert applied changes"; break;
            case OptionType::CONFIRM_CHANGES:
                tip = "Enter: Confirm changes and cancel auto-revert"; break;
            case OptionType::STANDARDIZE:
                tip = "Enter: Snap to 800x600, VSync On, 60 FPS (then Apply)"; break;
            case OptionType::VOLUME_MUSIC:
                tip = "Left/Right: Adjust music volume"; break;
            case OptionType::VOLUME_SOUND:
                tip = "Left/Right: Adjust SFX volume"; break;
            case OptionType::KEYBINDINGS:
                tip = "Enter: Open keybindings"; break;
            default:
                break;
        }

        // Show apply/cancel/confirm hint when pending or awaiting confirm
        if (m_videoPendingChanges || m_videoAwaitingConfirm) {
            std::string applyTip = m_videoAwaitingConfirm ?
                ("Changes applied. Confirm within " + std::to_string((int)std::ceil(m_videoRevertTimer)) + "s or they will revert.") :
                "Pending changes: Select Apply to commit or Cancel to discard.";
            BitmapFont::DrawText(renderer, applyTip, (int)(30*uiScale), instructionY + (int)(100*uiScale), std::max(1, (int)uiScale), Color(255, 200, 120, 255));
        }
    }

    if (!tip.empty()) {
        BitmapFont::DrawText(renderer, tip, (int)(30*uiScale), instructionY + (int)(80*uiScale), std::max(1, (int)uiScale), Color(180, 180, 255, 255));
    }

    // Show apply/cancel hint when pending or awaiting confirm
    if (m_videoPendingChanges || m_videoAwaitingConfirm) {
        std::string applyTip = m_videoAwaitingConfirm ?
            ("Changes applied. Confirm within " + std::to_string((int)std::ceil(m_videoRevertTimer)) + "s or they will revert.") :
            "Pending changes: Select Apply to commit or Cancel to discard.";
        BitmapFont::DrawText(renderer, applyTip, (int)(30*uiScale), instructionY + (int)(100*uiScale), std::max(1, (int)uiScale), Color(255, 200, 120, 255));
    }

    if (m_inKeybindingMode) {
        if (m_waitingForKey) {
            BitmapFont::DrawText(renderer, "PRESS A KEY TO BIND (ESC TO CANCEL)", (int)(30*uiScale), instructionY, std::max(1, (int)uiScale), Color(255, 255, 100, 255));
        } else {
            BitmapFont::DrawText(renderer, "UP/DOWN: Navigate  ENTER: Rebind Primary  RIGHT: Rebind Alt", (int)(30*uiScale), instructionY, std::max(1, (int)uiScale), Color(150, 150, 150, 255));
            BitmapFont::DrawText(renderer, "B/ESC: Back to Options", (int)(30*uiScale), instructionY + (int)(20*uiScale), std::max(1, (int)uiScale), Color(150, 150, 150, 255));

            // Show scroll hint if needed
            if (m_configurableActions.size() > static_cast<size_t>(m_maxVisibleKeybindings)) {
                BitmapFont::DrawText(renderer, "Use UP/DOWN to scroll through all keybindings", (int)(30*uiScale), instructionY + (int)(40*uiScale), std::max(1, (int)uiScale), Color(100, 100, 100, 255));
            }
        }
    } else {
        // Main options menu instructions
        BitmapFont::DrawText(renderer, "UP/DOWN: Navigate between options", (int)(30*uiScale), instructionY, std::max(1, (int)uiScale), Color(150, 150, 150, 255));
        BitmapFont::DrawText(renderer, "LEFT/RIGHT: Adjust settings", (int)(30*uiScale), instructionY + (int)(20*uiScale), std::max(1, (int)uiScale), Color(150, 150, 150, 255));
        BitmapFont::DrawText(renderer, "ENTER: Apply (Resolution/Fullscreen/VSync/FPS)", (int)(30*uiScale), instructionY + (int)(40*uiScale), std::max(1, (int)uiScale), Color(150, 150, 150, 255));
        BitmapFont::DrawText(renderer, "B/ESC: Back to Main Menu", (int)(30*uiScale), instructionY + (int)(60*uiScale), std::max(1, (int)uiScale), Color(150, 150, 150, 255));
    }
}

void OptionsState::InitializeVideoOptions() {
    // Populate common resolutions
    m_resolutions = {
        {800, 600}, {1024, 768}, {1280, 720}, {1366, 768}, {1600, 900}, {1920, 1080}, {2560, 1440}
    };

    // Load saved (or current) settings from gameplay.ini
    ConfigManager cfg;

    if (cfg.LoadFromFile("assets/config/gameplay.ini")) {
        int w = cfg.Get("visual", "screen_width", 800).AsInt();
        int h = cfg.Get("visual", "screen_height", 600).AsInt();

        bool fs = cfg.Get("visual", "fullscreen", false).AsBool();
        m_fullscreen = fs;
        // Find matching resolution in list, else append
        bool found = false;
        for (size_t i = 0; i < m_resolutions.size(); ++i) {
            if (m_resolutions[i].w == w && m_resolutions[i].h == h) { m_resolutionIndex = static_cast<int>(i); found = true; break; }
        }
        if (!found) { m_resolutions.push_back({w, h}); m_resolutionIndex = static_cast<int>(m_resolutions.size() - 1); }
    }
    // Load VSync and FPS from config
    {
    m_videoAwaitingConfirm = false;
    m_videoRevertTimer = 0.0f;

        ConfigManager cfg2;
        if (cfg2.LoadFromFile("assets/config/gameplay.ini")) {
            m_vsync = cfg2.Get("visual", "vsync", true).AsBool();
            int fps = cfg2.Get("visual", "fps_limit", 60).AsInt(); // 0 = Unlimited
            m_fpsIndex = 0;
            for (size_t i = 0; i < m_fpsOptions.size(); ++i) {
                if (m_fpsOptions[i] == fps) { m_fpsIndex = static_cast<int>(i); break; }
            }
        }
    }
}
void OptionsState::ApplyVideoChanges() {
    // Take snapshot before applying to allow revert
    LoadVisualSnapshot(m_prevFullscreen, m_prevVsync, m_prevWidth, m_prevHeight, m_prevFps);
    m_hasPrevSnapshot = true;

    SaveVideoSettings();
    if (auto* eng = GetEngine()) {
        eng->RecreateRendererFromConfig();
        int fps = m_fpsOptions[std::clamp(m_fpsIndex, 0, (int)m_fpsOptions.size()-1)];
        eng->SetTargetFPS(fps <= 0 ? 0 : fps);
    }
    m_videoPendingChanges = false;
    m_videoAwaitingConfirm = true;
    m_videoRevertTimer = 10.0f; // 10-second confirm window
}

void OptionsState::RevertVideoChanges() {
    if (m_hasPrevSnapshot) {
        // Restore previous settings and apply
        // Update in-memory values
        // Restore resolution index to match previous width/height
        int idx = 0;
        for (size_t i = 0; i < m_resolutions.size(); ++i) {
            if (m_resolutions[i].w == m_prevWidth && m_resolutions[i].h == m_prevHeight) { idx = (int)i; break; }
        }
        m_resolutionIndex = idx;
        m_fullscreen = m_prevFullscreen;
        m_vsync = m_prevVsync;
        // map fps to index
        for (size_t i = 0; i < m_fpsOptions.size(); ++i) {
            if (m_fpsOptions[i] == m_prevFps) { m_fpsIndex = (int)i; break; }
        }
        SaveVideoSettings();
        if (auto* eng = GetEngine()) {
            eng->RecreateRendererFromConfig();
            eng->SetTargetFPS(m_prevFps <= 0 ? 0 : m_prevFps);
        }
    } else {
        // Fallback: reload from config
        InitializeVideoOptions();
    }
    m_videoPendingChanges = false;
    m_videoAwaitingConfirm = false;
    m_videoRevertTimer = 0.0f;
}

void OptionsState::StandardizeVideoSettings() {
    // Set to baseline values
    // 800x600, fullscreen off, vsync on, fps 60
    int idx = 0;
    for (size_t i = 0; i < m_resolutions.size(); ++i) {
        if (m_resolutions[i].w == 800 && m_resolutions[i].h == 600) { idx = (int)i; break; }
    }
    // Take snapshot of currently applied settings for revert window
    LoadVisualSnapshot(m_prevFullscreen, m_prevVsync, m_prevWidth, m_prevHeight, m_prevFps);
    m_hasPrevSnapshot = true;

    m_resolutionIndex = idx;
    m_fullscreen = false;
    m_vsync = true;
    // find 60 in presets
    for (size_t i = 0; i < m_fpsOptions.size(); ++i) {
        if (m_fpsOptions[i] == 60) { m_fpsIndex = (int)i; break; }
    }
    m_videoPendingChanges = true;
}



void OptionsState::SaveVideoSettings() {
    // Persist to gameplay.ini
    ConfigManager cfg;
    cfg.LoadFromFile("assets/config/gameplay.ini");
    const auto& res = m_resolutions[std::clamp(m_resolutionIndex, 0, (int)m_resolutions.size()-1)];
    cfg.Set("visual", "screen_width", res.w);
    cfg.Set("visual", "screen_height", res.h);
    cfg.Set("visual", "fullscreen", m_fullscreen);
    cfg.Set("visual", "vsync", m_vsync);
    cfg.Set("visual", "fps_limit", m_fpsOptions[std::clamp(m_fpsIndex, 0, (int)m_fpsOptions.size()-1)]);
    cfg.SaveToFile("assets/config/gameplay.ini");

    // Apply immediately to window
    if (auto* win = GetEngine()->GetWindow()) {
        win->SetFullscreen(m_fullscreen);
        if (!m_fullscreen) win->SetSize(res.w, res.h);
    }
}

void OptionsState::InitializeOptions() {
    m_options.clear();
    m_optionTypes.clear();

    // Grouped sections with headers for clarity
    InitializeVideoOptions();
    m_options.push_back("Video");
    m_optionTypes.push_back(OptionType::HEADER_VIDEO);
    m_options.push_back("Resolution");
    m_optionTypes.push_back(OptionType::RESOLUTION);
    m_options.push_back("Fullscreen");
    m_optionTypes.push_back(OptionType::FULLSCREEN);
    m_options.push_back("VSync");
    m_optionTypes.push_back(OptionType::VSYNC);
    m_options.push_back("FPS Limit");
    m_optionTypes.push_back(OptionType::FPS_LIMIT);
    // Apply/Cancel/Confirm and Standardize controls
    m_options.push_back("Apply");
    m_optionTypes.push_back(OptionType::APPLY_CHANGES);
    m_options.push_back("Cancel");
    m_optionTypes.push_back(OptionType::CANCEL_CHANGES);
    m_options.push_back("Confirm");
    m_optionTypes.push_back(OptionType::CONFIRM_CHANGES);
    m_options.push_back("Standardize (800x600, VSync On, 60 FPS)");
    m_optionTypes.push_back(OptionType::STANDARDIZE);

    m_options.push_back("Audio");
    m_optionTypes.push_back(OptionType::HEADER_AUDIO);
    m_options.push_back("Music Volume");
    m_optionTypes.push_back(OptionType::VOLUME_MUSIC);
    m_options.push_back("Sound Volume");
    m_optionTypes.push_back(OptionType::VOLUME_SOUND);

    m_options.push_back("Controls");
    m_optionTypes.push_back(OptionType::HEADER_CONTROLS);
    m_options.push_back("Keybindings");
    m_optionTypes.push_back(OptionType::KEYBINDINGS);

    // Back
    m_options.push_back("Back to Menu");
    m_optionTypes.push_back(OptionType::BACK_TO_MENU);

    // Ensure vectors are in sync
    if (m_options.size() != m_optionTypes.size()) {
        std::cerr << "CRITICAL ERROR: Options vectors out of sync after initialization!" << std::endl;
        std::cerr << "  m_options.size(): " << m_options.size() << std::endl;
        std::cerr << "  m_optionTypes.size(): " << m_optionTypes.size() << std::endl;
    }

    // Ensure we start with a valid selection
    if (m_selectedOption < 0 || m_selectedOption >= GetOptionCount()) {
        m_selectedOption = 0;
    }
}

int OptionsState::GetOptionCount() const {
    // Use the smaller of the two sizes to ensure we never go out of bounds
    return static_cast<int>(std::min(m_options.size(), m_optionTypes.size()));
}



void OptionsState::EnterKeybindingMode() {
    m_inKeybindingMode = true;
    m_selectedKeybinding = 0;
    m_waitingForKey = false;
    std::cout << "Entered keybinding configuration mode" << std::endl;
}

void OptionsState::ExitKeybindingMode() {
    m_inKeybindingMode = false;
    m_waitingForKey = false;
    SaveKeybindings();
    std::cout << "Exited keybinding configuration mode" << std::endl;
}

void OptionsState::HandleKeybindingInput() {
    auto* input = GetInputManager();
    if (!input) return;

    if (m_waitingForKey) {
        // Handle key rebinding
        if (input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
            CancelKeyRebinding();
            return;
        }

        // Check for any key press
        for (int scancode = 0; scancode < SDL_NUM_SCANCODES; ++scancode) {
            if (input->IsKeyJustPressed(static_cast<SDL_Scancode>(scancode))) {
                ApplyKeyRebinding(static_cast<SDL_Scancode>(scancode));
                return;
            }
        }
    } else {
        HandleKeybindingNavigation();
    }
}

void OptionsState::HandleKeybindingNavigation() {
    auto* input = GetInputManager();
    if (!input) return;

    // Navigation
    if (input->IsKeyJustPressed(SDL_SCANCODE_UP) || input->IsKeyJustPressed(SDL_SCANCODE_W)) {
        m_selectedKeybinding = std::max(0, m_selectedKeybinding - 1);
        UpdateKeybindingScroll();
    }
    if (input->IsKeyJustPressed(SDL_SCANCODE_DOWN) || input->IsKeyJustPressed(SDL_SCANCODE_S)) {
        m_selectedKeybinding = std::min(static_cast<int>(m_configurableActions.size()) - 1, m_selectedKeybinding + 1);
        UpdateKeybindingScroll();
    }

    // Select action to rebind
    if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN) || input->IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
        if (m_selectedKeybinding >= 0 && m_selectedKeybinding < static_cast<int>(m_configurableActions.size())) {
            GameAction action = m_configurableActions[m_selectedKeybinding];
            StartKeyRebinding(action, true); // Start with primary key
        }
    }

    // Alternative key rebinding (right arrow)
    if (input->IsKeyJustPressed(SDL_SCANCODE_RIGHT) || input->IsKeyJustPressed(SDL_SCANCODE_D)) {
        if (m_selectedKeybinding >= 0 && m_selectedKeybinding < static_cast<int>(m_configurableActions.size())) {
            GameAction action = m_configurableActions[m_selectedKeybinding];
            StartKeyRebinding(action, false); // Start with alternative key
        }
    }

    // Back
    if (input->IsKeyJustPressed(SDL_SCANCODE_B) || input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        ExitKeybindingMode();
    }
}

void OptionsState::StartKeyRebinding(GameAction action, bool primary) {
    m_waitingForKey = true;
    m_keyToRebind = action;
    m_rebindingPrimary = primary;

    std::cout << "Waiting for key input for " << m_keybindingManager->GetActionDisplayName(action)
              << " (" << (primary ? "primary" : "alternative") << ")" << std::endl;
}

void OptionsState::CancelKeyRebinding() {
    m_waitingForKey = false;
    m_keyToRebind = GameAction::ACTION_COUNT;
    std::cout << "Key rebinding cancelled" << std::endl;
}

void OptionsState::ApplyKeyRebinding(SDL_Scancode key) {
    if (m_keyToRebind == GameAction::ACTION_COUNT) return;

    // Validate the key binding
    if (!m_keybindingManager->ValidateBinding(m_keyToRebind, key)) {
        std::cout << "Invalid key binding: " << m_keybindingManager->GetKeyName(key) << std::endl;
        CancelKeyRebinding();
        return;
    }

    // Apply the binding
    bool success = false;
    if (m_rebindingPrimary) {
        success = m_keybindingManager->SetPrimaryKey(m_keyToRebind, key);
    } else {
        success = m_keybindingManager->SetAlternativeKey(m_keyToRebind, key);
    }

    if (success) {
        std::cout << "Successfully bound " << m_keybindingManager->GetKeyName(key)
                  << " to " << m_keybindingManager->GetActionDisplayName(m_keyToRebind) << std::endl;
    } else {
        std::cout << "Failed to bind key" << std::endl;
    }

    m_waitingForKey = false;
    m_keyToRebind = GameAction::ACTION_COUNT;
}

void OptionsState::DrawKeybindings() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    // Layout constants
    const int titleY = 80;
    const int startY = 120;
    const int lineHeight = 22;
    const int leftMargin = 30;
    const int actionColumnWidth = 220;
    const int bindingColumnX = leftMargin + actionColumnWidth;
    const int statusColumnX = bindingColumnX + 200;

    // Draw title
    BitmapFont::DrawText(renderer, "KEYBINDING CONFIGURATION", leftMargin, titleY, 2, Color(255, 255, 100, 255));

    // Draw column headers
    BitmapFont::DrawText(renderer, "Action", leftMargin, startY - 25, 1, Color(150, 150, 150, 255));
    BitmapFont::DrawText(renderer, "Keys", bindingColumnX, startY - 25, 1, Color(150, 150, 150, 255));

    // Draw scroll indicator if needed
    if (m_configurableActions.size() > static_cast<size_t>(m_maxVisibleKeybindings)) {
        int totalActions = static_cast<int>(m_configurableActions.size());
        int currentPage = (m_selectedKeybinding / m_maxVisibleKeybindings) + 1;
        int totalPages = (totalActions + m_maxVisibleKeybindings - 1) / m_maxVisibleKeybindings;

        std::string scrollInfo = "Page " + std::to_string(currentPage) + "/" + std::to_string(totalPages);
        BitmapFont::DrawText(renderer, scrollInfo, 600, titleY, 1, Color(150, 150, 150, 255));
    }

    // Draw visible keybindings
    int visibleCount = 0;
    for (size_t i = 0; i < m_configurableActions.size() && visibleCount < m_maxVisibleKeybindings; ++i) {
        int actionIndex = static_cast<int>(i);

        // Skip items that are scrolled out of view
        if (actionIndex < m_keybindingScrollOffset) continue;

        int y = startY + visibleCount * lineHeight;
        bool isSelected = (actionIndex == m_selectedKeybinding);

        GameAction action = m_configurableActions[i];
        const KeyBinding& binding = m_keybindingManager->GetBinding(action);

        Color textColor = isSelected && m_showSelection ?
            Color(255, 255, 100, 255) : Color(200, 200, 200, 255);

        // Selection indicator
        if (isSelected && m_showSelection) {
            BitmapFont::DrawText(renderer, ">", leftMargin - 20, y, 1, Color(255, 255, 100, 255));
        }

        // Action name (truncated if too long)
        std::string actionText = m_keybindingManager->GetActionDisplayName(action);
        if (actionText.length() > 25) {
            actionText = actionText.substr(0, 22) + "...";
        }
        BitmapFont::DrawText(renderer, actionText, leftMargin, y, 1, textColor);

        // Key bindings (formatted for better readability)
        std::string bindingText = GetKeybindingDisplayText(binding);
        if (bindingText.length() > 20) {
            bindingText = bindingText.substr(0, 17) + "...";
        }
        BitmapFont::DrawText(renderer, bindingText, bindingColumnX, y, 1, textColor);

        // Status indicator
        if (m_waitingForKey && m_keyToRebind == action) {
            std::string waitText = m_rebindingPrimary ? "[PRIMARY]" : "[ALT]";
            BitmapFont::DrawText(renderer, waitText, statusColumnX, y, 1, Color(255, 100, 100, 255));
        } else if (isSelected) {
            BitmapFont::DrawText(renderer, "ENTER:Primary  RIGHT:Alt", statusColumnX, y, 1, Color(100, 255, 100, 255));
        }

        visibleCount++;
    }

    // Draw separator line
    int separatorY = startY + m_maxVisibleKeybindings * lineHeight + 10;
    for (int x = leftMargin; x < 700; x += 10) {
        BitmapFont::DrawText(renderer, "-", x, separatorY, 1, Color(100, 100, 100, 255));
    }
}

void OptionsState::SaveKeybindings() {
    if (m_keybindingManager->SaveToConfig("assets/config/keybindings.ini")) {
        std::cout << "Keybindings saved successfully" << std::endl;
    } else {
        std::cout << "Failed to save keybindings" << std::endl;
    }
}

std::string OptionsState::GetVolumeDisplayText(float volume) const {
    int volumePercent = static_cast<int>(volume * 100);
    return ": " + std::to_string(volumePercent) + "%";
}

std::string OptionsState::GetKeybindingDisplayText(const KeyBinding& binding) const {
    std::string text;

    // Get primary key name
    std::string primaryName;
    if (binding.primaryKey != SDL_SCANCODE_UNKNOWN) {
        primaryName = m_keybindingManager->GetKeyName(binding.primaryKey);
        // Shorten common key names for better display
        if (primaryName == "Left") primaryName = "←";
        else if (primaryName == "Right") primaryName = "→";
        else if (primaryName == "Up") primaryName = "↑";
        else if (primaryName == "Down") primaryName = "↓";
        else if (primaryName == "Return") primaryName = "Enter";
        else if (primaryName == "Space") primaryName = "Space";
    } else {
        primaryName = "None";
    }

    text += primaryName;

    // Get alternative key name
    if (binding.alternativeKey != SDL_SCANCODE_UNKNOWN) {
        std::string altName = m_keybindingManager->GetKeyName(binding.alternativeKey);
        // Shorten common key names
        if (altName == "Left") altName = "←";
        else if (altName == "Right") altName = "→";
        else if (altName == "Up") altName = "↑";
        else if (altName == "Down") altName = "↓";
        else if (altName == "Return") altName = "Enter";
        else if (altName == "Space") altName = "Space";

        text += " / " + altName;
    }

    return text;
}

void OptionsState::CalculateKeybindingLayout() {
    // Calculate how many keybindings can fit on screen
    const int titleHeight = 120;  // Space for title and headers
    const int instructionHeight = 80;  // Space for instructions at bottom
    const int lineHeight = 22;
    const int screenHeight = 600;  // Assuming 600px screen height

    int availableHeight = screenHeight - titleHeight - instructionHeight;
    m_maxVisibleKeybindings = std::max(1, availableHeight / lineHeight);

    std::cout << "Calculated max visible keybindings: " << m_maxVisibleKeybindings << std::endl;
}

void OptionsState::UpdateKeybindingScroll() {
    if (m_configurableActions.empty() || m_maxVisibleKeybindings <= 0) return;

    // Ensure selected item is visible
    if (m_selectedKeybinding < m_keybindingScrollOffset) {
        // Scroll up
        m_keybindingScrollOffset = m_selectedKeybinding;
    } else if (m_selectedKeybinding >= m_keybindingScrollOffset + m_maxVisibleKeybindings) {
        // Scroll down
        m_keybindingScrollOffset = m_selectedKeybinding - m_maxVisibleKeybindings + 1;
    }

    // Clamp scroll offset
    int maxOffset = std::max(0, static_cast<int>(m_configurableActions.size()) - m_maxVisibleKeybindings);
    m_keybindingScrollOffset = std::max(0, std::min(m_keybindingScrollOffset, maxOffset));
}

bool OptionsState::IsKeybindingVisible(int index) const {
    return index >= m_keybindingScrollOffset &&
           index < m_keybindingScrollOffset + m_maxVisibleKeybindings;
}