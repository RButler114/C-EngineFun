# Options System Documentation

## Overview

The Options System provides a comprehensive, type-safe configuration management solution for game settings with hierarchical menu interface, real-time validation, file persistence, and seamless game integration.

## Core Components

- **ConfigManager** - Central configuration management with type-safe values
- **OptionsState** - Interactive menu with category navigation and real-time preview
- **ConfigValue** - Type-safe value container (bool, int, float, string, enum)
- **ConfigSetting** - Setting definition with validation and metadata

## Quick Start

```cpp
// Initialize and use configuration
ConfigManager config;
config.Initialize("config.txt");

// Get/Set values with type safety
float volume = config.GetFloat("audio.masterVolume");
config.SetFloat("audio.masterVolume", 0.8f);
config.SaveToFile();

// Navigate to options from menu
stateManager->ChangeState(GameStateType::OPTIONS);
```

## Available Settings

### Audio
- Master/Music/SFX/Voice/Ambient volumes (0.0-1.0)
- Global mute toggle

### Graphics
- Resolution (800x600, 1024x768, 1280x720, 1920x1080)
- Fullscreen, VSync, frame rate limiting, FPS display

### Input
- Mouse sensitivity (0.1-5.0), Y-axis inversion
- Key repeat delay and rate

### Gameplay
- Difficulty (Easy, Normal, Hard, Expert)
- Auto-save settings, tutorials, pause behavior

## Navigation Controls

### Menu Navigation
- **UP/DOWN**: Navigate categories/settings
- **LEFT/RIGHT**: Adjust values
- **ENTER**: Select/confirm
- **ESCAPE**: Go back/cancel
- **TAB**: Switch categories

### UI Elements
- **Sliders**: Numeric values with real-time preview
- **Toggles**: Boolean ON/OFF switches
- **Dropdowns**: Enumerated options (resolution, difficulty)
- **Visual indicators**: Restart requirements (*), unsaved changes

## Configuration File Format

Settings are stored in human-readable text format:
```
# Audio Settings
audio.masterVolume=1.0
audio.musicVolume=0.7

# Graphics Settings
graphics.resolution=2
graphics.fullscreen=false
```

## Integration

```cpp
// 1. Include headers
#include "Engine/ConfigManager.h"
#include "Game/OptionsState.h"

// 2. Register state
stateManager->AddState(GameStateType::OPTIONS, std::make_unique<OptionsState>());

// 3. Custom settings
config.RegisterSetting("custom.setting", "Display Name", "Description",
                      ConfigValue(defaultVal), "Category");

// 4. Real-time callbacks
config.RegisterChangeCallback([](const std::string& key, const ConfigValue& oldVal, const ConfigValue& newVal) {
    if (key == "audio.masterVolume") {
        audioManager->SetMasterVolume(newVal.AsFloat());
    }
});
```

## Testing

Run configuration system tests:
```bash
cd test
g++ -std=c++17 -I../include test_options_compilation.cpp ../src/Engine/ConfigManager.cpp -o test_config
./test_config
```

## Troubleshooting

**Settings not persisting**: Check file permissions and ensure `SaveToFile()` is called
**Invalid values**: Verify min/max ranges and validation functions
**UI not responding**: Check input manager integration and state transitions
