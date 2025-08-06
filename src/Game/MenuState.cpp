/**
 * @file MenuState.cpp
 * @brief Implementation of main menu game state
 * @author Ryan Butler
 * @date 2025
 */

#include "Game/MenuState.h"
#include "Game/GameStateManager.h"
#include "Engine/Renderer.h"
#include "Engine/InputManager.h"
#include "Engine/Engine.h"
#include "Engine/AudioManager.h"
#include "Engine/BitmapFont.h"
#include <iostream>

/**
 * @brief Constructor - initializes menu state with default options
 *
 * Sets up the menu state with:
 * - Default selection (first option)
 * - Blinking animation timer
 * - Menu option text strings
 * - Initial visual state
 *
 * @note Menu options are hardcoded but could be made configurable
 * @note Selection starts at index 0 (first option)
 */
MenuState::MenuState()
    : GameState(GameStateType::MENU, "Menu")
    , m_selectedOption(0)        // Start with first option selected
    , m_blinkTimer(0.0f)         // Initialize blink animation timer
    , m_showSelection(true)      // Start with selection visible
{
    // Initialize menu option text
    // These correspond to MenuOption enum values
    m_menuOptions = {
        "START GAME",   // MenuOption::START_GAME
        "OPTIONS",      // MenuOption::OPTIONS
        "QUIT"          // MenuOption::QUIT
    };
}

/**
 * @brief Initialize menu when becoming active state
 *
 * Called when the menu state becomes active. Sets up:
 * - Reset selection to first option
 * - Reset visual effects and timers
 * - Load menu-specific audio assets
 * - Display rendering information for developers
 *
 * @note Audio loading is optional - menu works without sound
 * @note Visual effects are reset to ensure consistent appearance
 */
void MenuState::OnEnter() {
    std::cout << "🎮 Entering Main Menu" << std::endl;

    // Provide helpful information for developers about the rendering system
    std::cout << "🎨 Text rendering info (for developers):" << std::endl;
    std::cout << "   - Title: Bright yellow blocks with red outlines" << std::endl;
    std::cout << "   - Menu options: White blocks (selected: yellow)" << std::endl;
    std::cout << "   - Instructions: White blocks with black outlines" << std::endl;
    std::cout << "   - This is placeholder rendering until font system is implemented" << std::endl;

    // Reset menu state to defaults
    m_selectedOption = 0;        // Select first option
    m_blinkTimer = 0.0f;         // Reset blink animation
    m_showSelection = true;      // Start with selection visible

    // Load menu-specific audio assets (optional)
    if (GetEngine()->GetAudioManager()) {
        GetEngine()->GetAudioManager()->LoadSound(
            "menu_select",                          // Sound identifier
            "assets/sounds/menu_select.wav",        // File path
            SoundType::SOUND_EFFECT                 // Sound type
        );
        std::cout << "🔊 Menu audio loaded" << std::endl;
    }
}

/**
 * @brief Clean up menu when leaving state
 *
 * Called when transitioning away from menu state.
 * Handles any necessary cleanup (currently minimal).
 *
 * @note Menu state is lightweight - minimal cleanup needed
 * @note Audio resources remain loaded for potential reuse
 */
void MenuState::OnExit() {
    std::cout << "👋 Exiting Main Menu" << std::endl;
}

void MenuState::Update(float deltaTime) {
    HandleInput(); // Handle user input

    // Update selection blink effect
    m_blinkTimer += deltaTime;
    if (m_blinkTimer >= 0.5f) {
        m_showSelection = !m_showSelection;
        m_blinkTimer = 0.0f;
    }

    // Auto-start removed - use manual input (ENTER key) to start game
}

void MenuState::Render() {
    auto* renderer = GetRenderer();
    if (!renderer) {
        std::cout << "❌ MenuState: No renderer available!" << std::endl;
        return;
    }

    static int renderCount = 0;
    renderCount++;
    if (renderCount % 60 == 1) { // Print every 60 frames (about once per second)
        std::cout << "🎨 MenuState::Render() called (frame " << renderCount << ")" << std::endl;
    }

    DrawBackground();
    DrawTitle();
    DrawMenu();
}

void MenuState::HandleInput() {
    auto* input = GetInputManager();
    if (!input) {
        std::cout << "No input manager in MenuState!" << std::endl;
        return;
    }

    // Debug: Check if any keys are pressed (reduced frequency)
    static float debugTimer = 0.0f;
    debugTimer += 0.016f; // Approximate frame time
    if (debugTimer >= 5.0f) {
        std::cout << "Menu active - Current selection: " << m_selectedOption << " (" << m_menuOptions[m_selectedOption] << ")" << std::endl;
        debugTimer = 0.0f;
    }

    // Navigation
    if (input->IsKeyJustPressed(SDL_SCANCODE_UP) || input->IsKeyJustPressed(SDL_SCANCODE_W)) {
        std::cout << "UP key pressed!" << std::endl;
        NavigateUp();
    }
    if (input->IsKeyJustPressed(SDL_SCANCODE_DOWN) || input->IsKeyJustPressed(SDL_SCANCODE_S)) {
        std::cout << "DOWN key pressed!" << std::endl;
        NavigateDown();
    }

    // Selection - using IsKeyPressed with debouncing since IsKeyJustPressed has issues
    static bool enterWasPressed = false;
    static bool spaceWasPressed = false;

    bool enterPressed = input->IsKeyPressed(SDL_SCANCODE_RETURN);
    bool spacePressed = input->IsKeyPressed(SDL_SCANCODE_SPACE);

    if ((enterPressed && !enterWasPressed) || (spacePressed && !spaceWasPressed)) {
        std::cout << "ENTER/SPACE key pressed!" << std::endl;
        SelectOption();
    }

    enterWasPressed = enterPressed;
    spaceWasPressed = spacePressed;

    // Quick quit - but only if QUIT is already selected, otherwise select QUIT
    if (input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        std::cout << "ESCAPE key pressed!" << std::endl;
        if (m_selectedOption == static_cast<int>(MenuOption::QUIT)) {
            // If QUIT is already selected, confirm quit
            GetEngine()->Quit();
        } else {
            // Otherwise, navigate to QUIT option
            m_selectedOption = static_cast<int>(MenuOption::QUIT);
            m_showSelection = true;
            m_blinkTimer = 0.0f;
            std::cout << "Navigate to QUIT option - press ESCAPE again or ENTER to confirm" << std::endl;
        }
    }
}

void MenuState::NavigateUp() {
    m_selectedOption--;
    if (m_selectedOption < 0) {
        m_selectedOption = static_cast<int>(m_menuOptions.size()) - 1;
    }
    m_showSelection = true;
    m_blinkTimer = 0.0f;

    // Play menu navigation sound
    if (GetEngine()->GetAudioManager()) {
        GetEngine()->GetAudioManager()->PlaySound("menu_select", 0.7f);
    }

    std::cout << "🔼 Menu navigation UP - Selected option: " << m_selectedOption << " (" << m_menuOptions[m_selectedOption] << ")" << std::endl;
}

void MenuState::NavigateDown() {
    m_selectedOption++;
    if (m_selectedOption >= static_cast<int>(m_menuOptions.size())) {
        m_selectedOption = 0;
    }
    m_showSelection = true;
    m_blinkTimer = 0.0f;

    // Play menu navigation sound
    if (GetEngine()->GetAudioManager()) {
        GetEngine()->GetAudioManager()->PlaySound("menu_select", 0.7f);
    }

    std::cout << "🔽 Menu navigation DOWN - Selected option: " << m_selectedOption << " (" << m_menuOptions[m_selectedOption] << ")" << std::endl;
}

void MenuState::SelectOption() {
    MenuOption option = static_cast<MenuOption>(m_selectedOption);

    switch (option) {
        case MenuOption::START_GAME:
            std::cout << "Opening character customization..." << std::endl;
            if (GetStateManager()) {
                GetStateManager()->ChangeState(GameStateType::CUSTOMIZATION);
            }
            break;

        case MenuOption::OPTIONS:
            std::cout << "Opening options..." << std::endl;
            if (GetStateManager()) {
                GetStateManager()->ChangeState(GameStateType::OPTIONS);
            }
            break;

        case MenuOption::QUIT:
            std::cout << "Quitting game..." << std::endl;
            GetEngine()->Quit();
            break;
    }
}

void MenuState::DrawBackground() {
    auto* renderer = GetRenderer();

    // Draw a clean gradient background effect
    for (int y = 0; y < 600; y += 4) {
        int intensity = 15 + (y * 35 / 600); // Darker gradient for better text contrast
        Color bgColor(intensity, intensity, intensity + 10, 255); // Slight blue tint
        renderer->DrawRectangle(Rectangle(0, y, 800, 4), bgColor, true);
    }
}

void MenuState::DrawTitle() {
    auto* renderer = GetRenderer();

    // Draw title using bitmap font
    std::string title = "ARCADE FIGHTER";
    int titleWidth = title.length() * 6 * 4; // 6 pixels per char * 4 scale
    int startX = (800 - titleWidth) / 2;
    int titleY = 150;

    static bool debugPrinted = false;
    if (!debugPrinted) {
        std::cout << "🎨 Drawing bitmap title: " << title << " at (" << startX << ", " << titleY << ")" << std::endl;
        debugPrinted = true;
    }

    // Draw title with bitmap font
    BitmapFont::DrawText(renderer, title, startX, titleY, 4, Color(255, 215, 0, 255)); // Gold color, scale 4

    // Clean title area - no test elements needed anymore since text is working!
}

void MenuState::DrawMenu() {
    auto* renderer = GetRenderer();
    
    int menuStartY = 350;
    int menuSpacing = 60;
    
    for (int i = 0; i < static_cast<int>(m_menuOptions.size()); i++) {
        int optionY = menuStartY + i * menuSpacing;
        bool isSelected = (i == m_selectedOption);
        
        // Calculate text width for centering
        int textWidth = m_menuOptions[i].length() * 16;
        int textX = (800 - textWidth) / 2;
        
        // Draw selection indicator
        if (isSelected && m_showSelection) {
            // Draw selection box with more prominent colors
            Rectangle selectionRect(textX - 20, optionY - 5, textWidth + 40, 30);
            renderer->DrawRectangle(selectionRect, Color(255, 255, 0, 150), true); // Bright yellow background
            renderer->DrawRectangle(selectionRect, Color(255, 0, 0, 255), false); // Red outline

            // Draw larger, more visible arrows
            for (int i = 0; i < 3; i++) {
                renderer->DrawLine(textX - 40, optionY + 8 + i, textX - 25, optionY + 8 + i, Color(255, 0, 0, 255));
                renderer->DrawLine(textX + textWidth + 25, optionY + 8 + i, textX + textWidth + 40, optionY + 8 + i, Color(255, 0, 0, 255));
            }
        }
        
        // Draw menu text using bitmap font
        Color textColor = isSelected ? Color(255, 255, 0, 255) : Color(220, 220, 220, 255);
        BitmapFont::DrawText(renderer, m_menuOptions[i], textX, optionY, 2, textColor); // Scale 2
    }
    
    // Draw instructions at bottom using bitmap font
    std::string instructions = "USE ARROW KEYS TO NAVIGATE - ENTER TO SELECT - ESC TO QUIT";
    int instrWidth = instructions.length() * 6 * 1; // 6 pixels per char * 1 scale
    int instrX = (800 - instrWidth) / 2;

    BitmapFont::DrawText(renderer, instructions, instrX, 550, 1, Color(200, 200, 200, 255)); // Gray, scale 1
}
