# Keybinding Configuration System

This document explains the comprehensive keybinding system implemented in the game, which allows players to customize their controls through the options menu.

## 🎯 System Overview

The keybinding system provides:
- **Configurable key mappings** for all game actions
- **Primary and alternative keys** for each action
- **Conflict detection** and validation
- **Configuration file persistence** 
- **Integration with options menu**
- **Easy extensibility** for new actions

## 🏗️ Architecture

### Core Components

1. **KeybindingManager** (`Engine/KeybindingManager.h`)
   - Central management of all key bindings
   - Configuration loading/saving
   - Input validation and conflict detection

2. **GameAction Enum** 
   - Defines all possible game actions
   - Categories: Movement, Menu Navigation, Game Controls, etc.

3. **KeyBinding Struct**
   - Stores primary and alternative keys for each action
   - Display names and descriptions
   - Rebinding permissions

4. **Options Menu Integration**
   - Dedicated keybinding configuration screen
   - Real-time key rebinding interface
   - Visual feedback and conflict warnings

## 🎮 Available Actions

### Movement
- **Move Left/Right/Up/Down**: Character movement
- Default: Arrow Keys + WASD

### Menu Navigation  
- **Menu Up/Down/Left/Right**: Navigate menus
- **Select/Back/Confirm**: Menu interactions
- Default: Arrow Keys + WASD, Enter, B

### Game Controls
- **Pause**: Pause/unpause game
- **Return to Menu**: Go back to main menu
- **Reload Config**: Reload configuration files
- Default: P/Escape, M, R

### Level Selection
- **Level 1/2/3**: Quick level switching
- **Boss Level**: Load boss level
- **Reset Level**: Return to base configuration
- Default: 1/2/3, B, 0

### Customization
- **Confirm Customization**: Quick confirm in character creation
- Default: C

## 📁 Configuration Files

### `assets/config/keybindings.ini`

```ini
[keybindings]
# Each action has primary and alternative key bindings
move_left_primary=Left
move_left_alt=A
move_right_primary=Right
move_right_alt=D
# ... etc

[keybinding_categories]
# Organize actions into categories
movement=Movement
menu=Menu Navigation
game=Game Controls

[ui_settings]
# UI behavior settings
show_conflicts=true
allow_duplicate_bindings=false
require_confirmation=true
```

## 🎛️ Options Menu Usage

### Accessing Keybinding Configuration
1. Go to **Main Menu** → **Options**
2. Select **Keybindings**
3. Navigate with arrow keys
4. Press **Enter** to rebind primary key
5. Press **Right Arrow** to rebind alternative key
6. Press **B** or **Escape** to go back

### Rebinding Keys
1. Select the action you want to rebind
2. Press **Enter** (primary) or **Right** (alternative)
3. Press the new key you want to assign
4. Press **Escape** to cancel rebinding
5. Changes are automatically saved

### Visual Feedback
- **Yellow highlight**: Currently selected action
- **Red text**: Waiting for key input
- **Conflict warnings**: If key is already assigned

## 🔧 Technical Implementation

### Adding New Actions

1. **Add to GameAction enum**:
```cpp
enum class GameAction {
    // ... existing actions
    NEW_ACTION,
    ACTION_COUNT  // Keep this last
};
```

2. **Add to default bindings**:
```cpp
void KeybindingManager::SetupDefaultBindings() {
    // ... existing bindings
    m_bindings[GameAction::NEW_ACTION] = KeyBinding(
        SDL_SCANCODE_X, SDL_SCANCODE_UNKNOWN, 
        "New Action", "Description of new action"
    );
}
```

3. **Add to action names**:
```cpp
void KeybindingManager::InitializeActionNames() {
    // ... existing names
    m_actionNames[GameAction::NEW_ACTION] = "new_action";
    m_actionDisplayNames[GameAction::NEW_ACTION] = "New Action";
}
```

4. **Use in game code**:
```cpp
if (keybindingManager->IsActionJustPressed(GameAction::NEW_ACTION, inputManager)) {
    // Handle the action
}
```

### Integration with Game States

Game states can use the keybinding system instead of hardcoded keys:

```cpp
// Old way (hardcoded)
if (input->IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
    SelectOption();
}

// New way (configurable)
if (keybindingManager->IsActionJustPressed(GameAction::MENU_SELECT, input)) {
    SelectOption();
}
```

## 🛡️ Validation and Safety

### Conflict Detection
- Prevents assigning the same key to multiple actions
- Shows warnings for conflicting assignments
- Allows acceptable conflicts in different contexts

### Key Validation
- Prevents binding system-reserved keys
- Validates key availability
- Ensures at least one key per action

### Configuration Safety
- Automatic backup of working configurations
- Fallback to defaults if config is corrupted
- Validation on load with error reporting

## 🎨 Customization Options

### UI Settings
- **Show Conflicts**: Highlight conflicting key assignments
- **Require Confirmation**: Ask before overwriting existing bindings
- **Group by Category**: Organize actions by type

### Accessibility
- **Sticky Keys**: Support for accessibility features
- **Key Repeat**: Configurable repeat rates
- **Visual Feedback**: Key press confirmation

## 🔄 Configuration Persistence

### Automatic Saving
- Changes saved immediately when modified
- No need to manually save settings
- Persistent across game sessions

### File Format
- Human-readable INI format
- Easy to edit manually if needed
- Version-compatible upgrades

### Backup and Recovery
- Automatic backup of working configurations
- Reset to defaults option
- Individual action reset capability

## 🎯 Benefits

1. **Player Comfort**: Customize controls to personal preference
2. **Accessibility**: Support for different input needs
3. **Flexibility**: Easy to add new actions without code changes
4. **Consistency**: Unified input handling across all game states
5. **Maintainability**: Centralized key management
6. **Extensibility**: Simple to add new features

The keybinding system provides a professional, user-friendly way for players to customize their gaming experience while maintaining clean, maintainable code architecture.
