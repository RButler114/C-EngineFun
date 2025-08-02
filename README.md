# 🎮 C++ Game Engine with ECS Architecture

A complete C++ game engine featuring Entity-Component-System architecture, SDL2 rendering, and a classic arcade fighter demo game. Built for learning game development fundamentals with professional-grade code documentation.

## 🎯 Features

### Core Engine
- **Cross-platform window management** with SDL2
- **60 FPS game loop** with frame capping and delta time
- **Comprehensive input handling** (keyboard/mouse)
- **Advanced rendering system** (shapes, sprites, parallax backgrounds)
- **Entity-Component-System (ECS)** architecture
- **Professional state management** (Menu, Playing, Game Over)
- **AABB collision detection** with physics response
- **Complete audio system** with SDL_mixer (sound effects, music, 3D audio)
- **Complete Doxygen documentation** (C++ equivalent to JSDoc)

### Game Features
- **Animated start menu** with retro arcade styling and sound effects
- **Side-scrolling gameplay** with smooth camera following
- **Parallax background layers** (mountains, buildings, sky)
- **Player character** with movement and boundaries
- **Multiple enemy types** with varied AI patterns
- **30-second timed gameplay** with countdown and warnings
- **Scoring system** with movement bonuses
- **Game over screen** with restart options
- **Dynamic audio** with background music, jump sounds, and collision effects

## 🚀 Quick Start (30 seconds)

### Option 1: Instant Setup
```bash
# From project root - builds and runs automatically
./run_game.sh
```

### Option 2: Manual Setup
```bash
# 1. Install dependencies (macOS)
brew install sdl2 sdl2_image sdl2_mixer pkg-config cmake doxygen

# 2. Build and run
mkdir -p build && cd build
cmake .. && make
./bin/GameEngine  # Choose option 2 for full game
```

## 🎮 Controls

### Menu Navigation
- **↑/↓ Arrow Keys** or **W/S**: Navigate menu
- **Enter** or **Space**: Select option
- **Escape**: Quit

### In-Game
- **Arrow Keys** or **WASD**: Move player
- **Escape**: Return to menu

## 📚 Documentation

This project uses **Doxygen** for comprehensive code documentation (C++ equivalent to JSDoc):

```bash
# Generate HTML documentation
./generate_docs.sh

# View documentation
open docs/html/index.html
```

### Documentation Features
- **Complete API reference** with class hierarchies
- **Code examples** and usage patterns
- **Cross-referenced** functions and classes
- **Search functionality** for quick navigation
- **Professional HTML output** with diagrams
- **Auto-deployed** to GitHub Pages: [View Live Documentation](https://rbutler114.github.io/C-EngineFun/)

### CI/CD Integration
- **Automated builds** on macOS and Ubuntu
- **Documentation deployment** on every commit to main
- **Code quality checks** with clang-format and cppcheck
- **Automated testing** with comprehensive test suite

#### Menu Navigation
- **Arrow Keys (↑/↓)** or **W/S**: Navigate menu options
- **Enter** or **Space**: Select highlighted option
- **Escape**: Quit game

#### In-Game Controls
- **Arrow Keys** or **WASD**: Move player character
- **Escape**: Return to main menu

## 🎮 How to Play

1. **Start**: Select "START GAME" from the menu
2. **Objective**: Survive and score points for 30 seconds
3. **Movement**: Use WASD or arrow keys to move your green character
4. **Scoring**:
   - Gain points over time
   - Bonus points for active movement
   - Avoid enemies (red/orange/purple characters)
5. **Timer**: Watch the countdown in the top-right corner
6. **Game Over**: When time expires, view your final score and restart

## 📁 Project Structure
```
cppExploration/
├── CMakeLists.txt          # Build configuration
├── README.md              # This documentation
├── TUTORIAL.md            # Learning guide
├── src/                   # Source files
│   ├── main.cpp          # Entry point with game selection
│   ├── Engine/           # Core engine (Window, Renderer, Input)
│   ├── Game/             # Game states (Menu, Playing, GameOver)
│   ├── ECS/              # Entity-Component-System
│   └── Scenes/           # Scene management framework
├── include/              # Header files
│   ├── Engine/          # Engine headers
│   ├── Game/            # Game state headers
│   ├── ECS/             # ECS system headers
│   └── Scenes/          # Scene management headers
├── test/                # Automated testing suite
├── examples/            # Example implementations
├── assets/              # Game assets (future use)
└── build/               # Build output directory
    └── bin/             # Compiled executables
```

## 🧪 Testing & Validation

The project includes comprehensive automated tests to validate all systems:

### Run Automated Tests
```bash
# From the project root directory
cd test
chmod +x run_tests.sh
./run_tests.sh

# Tests include:
# - Basic rendering validation
# - State management verification
# - Input system testing
# - Manual game functionality checks
```

### Individual Test Components
```bash
# Test basic rendering (from build directory)
g++ -std=c++17 -I../include -I/opt/homebrew/include \
    ../test/test_menu_rendering.cpp ../src/Engine/*.cpp \
    -L/opt/homebrew/lib -lSDL2 -lSDL2_image -o test_menu
./test_menu

# Test input system
g++ -std=c++17 -I../include -I/opt/homebrew/include \
    ../test/test_input_system.cpp ../src/Engine/*.cpp \
    -L/opt/homebrew/lib -lSDL2 -lSDL2_image -o test_input
./test_input
```

## 📁 Project Structure

```
cppExploration/
├── README.md              # This documentation
├── CMakeLists.txt          # Build configuration
├── Doxyfile               # Doxygen documentation config
├── generate_docs.sh       # Documentation generation script
├── run_game.sh           # Quick start script
├── src/                  # Source files (documented)
│   ├── main.cpp          # Entry point with game selection
│   ├── Engine/           # Core engine implementation
│   ├── Game/             # Game states implementation
│   ├── ECS/              # Entity-Component-System implementation
│   └── Scenes/           # Scene management implementation
├── include/              # Header files (fully documented)
│   ├── Engine/          # Engine headers
│   ├── Game/            # Game state headers
│   ├── ECS/             # ECS system headers
│   └── Scenes/          # Scene management headers
├── docs/                # Generated documentation
│   └── html/            # HTML documentation files
├── examples/            # Example code and demos
├── test/               # Unit tests and validation
└── assets/             # Game assets (textures, sounds)
```

## 🏗️ Architecture Overview

### Engine Core
```cpp
// Basic Engine Usage
class MyGame : public Engine {
protected:
    void Update(float deltaTime) override {
        // Game logic here
    }
    void Render() override {
        // Rendering here
    }
};
```

### Entity-Component-System
```cpp
// ECS Usage Example
Entity player = entityManager.CreateEntity();
entityManager.AddComponent<PositionComponent>(player, {100.0f, 200.0f});
entityManager.AddComponent<VelocityComponent>(player, {50.0f, 0.0f});

// Systems process entities with specific components
movementSystem.Update(deltaTime);  // Updates all entities with Position + Velocity
```

### Audio System
```cpp
// Audio Manager Usage
AudioManager audioManager;
audioManager.Initialize();

// Load and play sounds
audioManager.LoadSound("jump", "assets/sounds/jump.wav", SoundType::SOUND_EFFECT);
audioManager.PlaySound("jump", 0.8f);  // 80% volume

// Background music
audioManager.LoadMusic("background", "assets/music/background.wav");
audioManager.PlayMusic("background", 0.5f, -1);  // 50% volume, loop infinitely

// ECS Audio Integration
Entity player = entityManager.CreateEntity();
entityManager.AddComponent<AudioComponent>(player, "jump", 0.8f, false, false, true);
entityManager.AddSystem<AudioSystem>(audioManager);

// 3D Positional Audio
audioComp->is3D = true;
audioComp->maxDistance = 500.0f;
audioSystem->SetListenerPosition(playerX, playerY);
```

## 🧪 Testing & Validation

```bash
# Quick validation
echo "1" | ./bin/GameEngine  # Test basic rendering

# Run all tests
cd test && ./run_tests.sh

# Individual component tests
./test/test_basic_rendering
./test/test_input_system
./test/test_menu_rendering
./test/test_state_management
./test/test_audio_system

# Audio system demo
./examples/AudioExample
```

## 🔧 Troubleshooting

### Quick Fixes
```bash
# Clean rebuild
cd build && make clean && cmake .. && make

# Check dependencies
brew list | grep sdl2

# Verify installation
pkg-config --cflags sdl2
```

### Expected Console Output
```
🥊 ARCADE FIGHTER GAME 🥊
Added game state: 1
Added game state: 3
Entering Menu State
🎨 MenuState::Render() called (frame 1)
Engine initialized successfully!
```

**If you see this output, everything is working perfectly!** ✅

## 🎯 Key Learning Concepts

This engine demonstrates professional game development patterns:
- **Game loop architecture** with proper frame timing
- **Entity-Component-System** for scalable game architecture
- **State management** for different game phases
- **Real-time rendering** with SDL2 and custom graphics
- **Input handling patterns** for responsive controls
- **Collision detection** with AABB and physics response
- **Complete code documentation** using Doxygen
---

**🎮 Total Setup Time: ~30 seconds**
**🏗️ Architecture: Professional ECS Game Engine**
**📚 Documentation: Complete Doxygen API Reference**
**✅ Status: Fully Functional and Documented**
