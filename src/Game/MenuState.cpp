#include "Game/MenuState.h"
#include "Game/GameStateManager.h"
#include "Engine/Renderer.h"
#include "Engine/InputManager.h"
#include "Engine/Engine.h"
#include "Engine/AudioManager.h"
#include "Engine/BitmapFont.h"
#include <iostream>

MenuState::MenuState()
    : GameState(GameStateType::MENU, "Menu")
    , m_selectedOption(0)
    , m_blinkTimer(0.0f)
    , m_showSelection(true) {

    m_menuOptions = {
        "START GAME",
        "OPTIONS",
        "QUIT"
    };
}

void MenuState::OnEnter() {
    std::cout << "Entering Menu State" << std::endl;
    std::cout << "🎨 Text will be rendered as colored rectangles:" << std::endl;
    std::cout << "   - Title: Bright yellow blocks with red outlines" << std::endl;
    std::cout << "   - Menu options: White blocks (selected: yellow)" << std::endl;
    std::cout << "   - Instructions: White blocks with black outlines" << std::endl;
    m_selectedOption = 0;
    m_blinkTimer = 0.0f;
    m_showSelection = true;

    // Load menu sounds
    if (GetEngine()->GetAudioManager()) {
        GetEngine()->GetAudioManager()->LoadSound("menu_select", "assets/sounds/menu_select.wav", SoundType::SOUND_EFFECT);
    }
}

void MenuState::OnExit() {
    std::cout << "Exiting Menu State" << std::endl;
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

    // Quick quit
    if (input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        std::cout << "ESCAPE key pressed!" << std::endl;
        GetEngine()->Quit();
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
            std::cout << "Starting game..." << std::endl;
            if (GetStateManager()) {
                GetStateManager()->ChangeState(GameStateType::PLAYING);
            }
            break;

        case MenuOption::OPTIONS:
            std::cout << "Opening options..." << std::endl;
            // TODO: Change to options state when implemented
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
