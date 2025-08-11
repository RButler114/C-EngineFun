# Streamlined C++ Everharvest Voyager V Engine

A focused C++ game engine optimized for classic arcade-style scrolling games, built with SDL2 and featuring a clean Entity-Component-System architecture.

## Features

### Core Engine
- **Cross-platform window management** using SDL2
- **60 FPS game loop** with delta time calculation
- **Input handling** for keyboard controls
- **2D sprite rendering** with animation support
- **Audio system** with sound effects and music
- **Efficient resource management**

### Streamlined ECS
- **Essential components**: Transform, Velocity, Collision, Audio
- **Core systems**: Movement, Collision Detection, Audio
- **Optimized for arcade games** - no unnecessary complexity

### Game Systems
- **State management** (Menu, Playing, Game Over, Options)
- **Sprite animation** with frame-based rendering
- **Scrolling backgrounds** for side-scrolling gameplay
- **Simple collision detection** for arcade mechanics
- **Audio integration** with volume controls

## Quick Start

### Prerequisites
- C++17 compatible compiler
- CMake 3.16 or higher
- SDL2, SDL2_image, SDL2_mixer

### Building

```bash
# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make -j4

# Run the game
./bin/GameEngine
```

## Key Improvements Made

This engine has been streamlined specifically for arcade games:

1. **Removed Redundant Systems**: Eliminated duplicate Scene/GameState management
2. **Focused ECS**: Only essential components and systems for arcade gameplay
3. **Simplified Rendering**: Direct sprite rendering with SpriteRenderer utility
4. **Removed Bloat**: Eliminated unused ConfigManager and excessive documentation
5. **Optimized Build**: Faster compilation and cleaner project structure

## Game Controls

- **Arrow Keys / WASD**: Navigate menus and move player
- **Enter / Space**: Select options
- **Escape**: Back/Quit
- **Left/Right**: Adjust volume in options

## License

This project is licensed under the MIT License.
