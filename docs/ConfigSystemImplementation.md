# Config System Implementation Guide

## Overview

This document describes the comprehensive config system implementation for the C++ arcade game engine. The system provides scalable, flexible configuration management with hot-reloading and level-specific overrides.

## ✅ Implemented Features

### 1. **Refactored PlayingState to use GameConfig**
- **Before**: 144+ hardcoded magic numbers throughout PlayingState.cpp
- **After**: All values loaded from configuration files
- **Benefits**: Easy game balancing, no recompilation needed for tweaks

**Key Changes:**
- Player movement speed: `250.0f` → `m_gameConfig->GetPlayerMovementSpeed()`
- Game duration: `30.0f` → `m_gameConfig->GetGameDurationSeconds()`
- Enemy count: `8` → `m_gameConfig->GetEnemyCount()`
- All colors, dimensions, and timing values now configurable

### 2. **Hot-Reloading Support**
- **Automatic**: Checks for file changes every 2 seconds
- **Manual**: Press `R` key to reload configs instantly
- **File Watching**: Tracks modification times of config files
- **Feedback**: Console messages show reload status

**Usage:**
```cpp
// Automatic checking in Update loop
if (m_gameConfig->CheckAndReloadIfModified()) {
    std::cout << "Config reloaded!" << std::endl;
}

// Manual reload
m_gameConfig->ReloadConfigs();
```

### 3. **Level-Specific Config Overrides**
- **Base Config**: `assets/config/gameplay.ini` (default values)
- **Level Configs**: `assets/config/levels/level1.ini`, `level2.ini`, `level3.ini`, `boss.ini`
- **Override System**: Level configs override base values, fall back to base for missing values
- **Dynamic Loading**: Switch levels at runtime with immediate effect

**Level Examples:**
- **Level 1 (Easy)**: Slower enemies, longer time, reduced difficulty
- **Level 3 (Hard)**: Faster enemies, shorter time, increased challenge  
- **Boss Level**: Special mechanics, enhanced audio, dramatic visuals

**Usage:**
```cpp
// Load level-specific config
m_gameConfig->LoadLevelConfig("level3");

// Clear overrides (back to base)
m_gameConfig->ClearLevelOverrides();

// Values automatically use level overrides when available
float speed = m_gameConfig->GetPlayerMovementSpeed(); // Uses level value if set
```

### 4. **Character System Integration**
- **CharacterFactory Integration**: Combines base character templates with config values
- **Dynamic Scaling**: Characters adapt to current level difficulty
- **Config-Aware Creation**: New `CreateConfigAwareCharacter()` method
- **Runtime Modification**: Characters created with level-appropriate stats

**Example Integration:**
```cpp
// Create character with level-appropriate modifications
CreateConfigAwareCharacter("goblin", x, y, difficultyMultiplier);

// Character automatically gets:
// - Health scaled by difficulty
// - Speed from current level config  
// - Size from config values
// - Audio volumes from config
```

## 🎮 In-Game Controls

| Key | Action |
|-----|--------|
| `WASD/Arrows` | Move player |
| `ESC` | Return to menu |
| `R` | Reload configs manually |
| `1` | Load Level 1 (Easy) |
| `2` | Load Level 2 (Normal) |
| `3` | Load Level 3 (Hard) |
| `B` | Load Boss Level |
| `0` | Clear level overrides (Base config) |
| `G` | Spawn config-aware character |

## 📁 File Structure

```
assets/config/
├── gameplay.ini          # Base gameplay settings
├── characters.ini         # Character templates (existing)
└── levels/
    ├── level1.ini        # Easy level overrides
    ├── level2.ini        # Normal level overrides  
    ├── level3.ini        # Hard level overrides
    └── boss.ini          # Boss level overrides
```

## 🔧 Configuration Categories

### Player Settings
- Starting position, movement speed, boundaries
- Animation timing, sprite dimensions

### Game Rules  
- Duration, scoring, debug intervals
- Warning thresholds, time limits

### Enemy Configuration
- Count, spawn patterns, movement speeds
- AI behavior, respawn mechanics

### Visual Settings
- Screen dimensions, colors, HUD layout
- Background parallax, ground rendering

### Audio Settings
- Volume levels for music, SFX, voices
- Sound file mappings

## 🚀 Benefits Achieved

1. **Rapid Iteration**: Change game balance without recompiling
2. **Level Design**: Easy creation of varied gameplay experiences  
3. **Platform Adaptation**: Different configs for mobile vs desktop
4. **Designer Friendly**: Non-programmers can modify game behavior
5. **Debug Support**: Toggle features and adjust timing via config
6. **Modding Support**: Players can customize game behavior

## 🔄 Hot-Reloading Workflow

1. **Edit Config**: Modify any `.ini` file in `assets/config/`
2. **Auto-Reload**: System detects changes within 2 seconds
3. **Manual Reload**: Press `R` for immediate reload
4. **Instant Effect**: Changes apply immediately to running game
5. **Feedback**: Console shows reload status and any errors

## 📊 Performance Impact

- **Minimal Runtime Cost**: Config lookups cached, only reload when changed
- **Memory Efficient**: Configs loaded once, shared across systems
- **File I/O Optimized**: Only reads files when modification time changes

## 🎯 Future Enhancements

- **Config Validation**: Type checking and range validation
- **Config Editor**: GUI tool for editing configurations
- **Network Configs**: Load configs from remote servers
- **Config Profiles**: Save/load different configuration sets
- **Advanced Overrides**: Conditional configs based on platform/performance

## 📝 Example Usage

```cpp
// Initialize config system
GameConfig gameConfig;
gameConfig.LoadConfigs();

// Load specific level
gameConfig.LoadLevelConfig("boss");

// Use config values
float playerSpeed = gameConfig.GetPlayerMovementSpeed();
int enemyCount = gameConfig.GetEnemyCount();
Color hudColor = gameConfig.GetHudBackgroundColor();

// Hot-reload support
if (gameConfig.CheckAndReloadIfModified()) {
    // Configs were reloaded, update game state
    ResetGameState();
}
```

This implementation transforms the game from a hardcoded system to a flexible, configurable engine that supports rapid iteration and easy customization.
