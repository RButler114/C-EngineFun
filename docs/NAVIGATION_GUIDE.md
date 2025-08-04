# Game Navigation Guide

This document explains the improved navigation system implemented in the game to provide consistent and intuitive controls across all screens.

## 🎮 Navigation Philosophy

The new navigation system follows these principles:
- **Consistent controls** across all game states
- **Clear back navigation** without accidentally quitting
- **Intuitive key mappings** that make sense contextually
- **Progressive confirmation** for destructive actions

## 🔑 Universal Controls

### Core Navigation
- **Arrow Keys / WASD**: Navigate through menus and options
- **Enter / Space**: Select/Confirm current option
- **B Key**: Go back to previous screen (universal back button)

### Context-Specific Controls
- **C Key**: Quick confirm/continue (in customization)
- **M Key**: Return to main menu (from gameplay)
- **P Key**: Pause game (from gameplay)

## 📱 Screen-by-Screen Navigation

### Main Menu
- **Arrow Keys**: Navigate between START GAME, OPTIONS, QUIT
- **Enter/Space**: Select highlighted option
- **Escape**: Navigate to QUIT option (press again to confirm)

**Why this change?** Prevents accidental game closure while providing quick access to quit.

### Character Customization
- **Arrow Keys**: Navigate categories and options
- **Enter**: Select category or option
- **B**: Go back to previous screen
- **C**: Quick confirm and start game
- **Escape**: Return to main menu (only from main category view)

**Navigation Flow:**
```
Main Menu → Customization Categories → Option Selection → Confirmation → Gameplay
     ↑              ↑                        ↑              ↑
     B              B                        B              B
```

### Options Menu
- **Arrow Keys**: Navigate settings
- **Left/Right**: Adjust values
- **Enter**: Select/Toggle setting
- **B or Escape**: Return to main menu

### Gameplay
- **Movement**: Arrow Keys / WASD
- **P or Escape**: Pause game
- **M**: Return to main menu
- **R**: Reload configuration
- **1,2,3,B,0**: Level selection shortcuts

### Game Over Screen
- **Enter/Space**: Restart game
- **M or Escape**: Return to main menu
- **Q**: Quick quit

## 🔄 Back Navigation Logic

The **B key** provides consistent back navigation:

1. **Customization Screen**:
   - From option selection → Category selection
   - From category selection → Main menu

2. **Options Screen**:
   - From any option → Main menu

3. **Gameplay**:
   - Not applicable (use M for menu)

## ⚠️ Escape Key Behavior

The escape key behavior has been redesigned for better UX:

### Before (Problematic)
- Escape anywhere = Immediate game quit
- No way to navigate back safely
- Inconsistent behavior across screens

### After (Improved)
- **Main Menu**: Navigate to quit option (requires confirmation)
- **Customization**: Return to menu (only from main view)
- **Options**: Return to menu (alternative to B)
- **Gameplay**: Pause game
- **Game Over**: Return to menu

## 🎯 Benefits of New System

1. **No Accidental Quits**: Escape requires confirmation from main menu
2. **Consistent Back Navigation**: B key works the same everywhere
3. **Clear Visual Feedback**: Instructions shown on each screen
4. **Intuitive Flow**: Natural progression through menus
5. **Accessibility**: Multiple ways to perform common actions

## 🛠️ Implementation Details

### Key Changes Made:
1. Removed automatic quit from `InputManager` on escape
2. Added progressive escape behavior in `MenuState`
3. Implemented B key back navigation in all states
4. Updated all instruction text to reflect new controls
5. Made escape context-aware in each state

### Code Locations:
- `src/Engine/InputManager.cpp`: Removed auto-quit on escape
- `src/Game/MenuState.cpp`: Progressive escape behavior
- `src/Game/CustomizationState.cpp`: B key back navigation
- `src/Game/OptionsState.cpp`: B key alternative
- `src/Game/PlayingState.cpp`: M key for menu return

## 🎮 User Experience

The new navigation system provides:
- **Confidence**: Users won't accidentally quit
- **Consistency**: Same controls work everywhere
- **Clarity**: Clear instructions on every screen
- **Flexibility**: Multiple ways to navigate

This creates a more polished, user-friendly experience that feels professional and intuitive.
