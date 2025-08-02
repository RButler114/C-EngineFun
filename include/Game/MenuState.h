#pragma once

#include "GameState.h"
#include <vector>
#include <string>

enum class MenuOption {
    START_GAME,
    OPTIONS,
    QUIT
};

class MenuState : public GameState {
public:
    MenuState();
    
    void OnEnter() override;
    void OnExit() override;
    void Update(float deltaTime) override;
    void Render() override;
    void HandleInput() override;

private:
    std::vector<std::string> m_menuOptions;
    int m_selectedOption;
    float m_blinkTimer;
    bool m_showSelection;
    
    void NavigateUp();
    void NavigateDown();
    void SelectOption();
    void DrawTitle();
    void DrawMenu();
    void DrawBackground();
};
