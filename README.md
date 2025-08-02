# 🎮 Arcade Fighter Game Engine

A complete C++ game engine featuring a classic side-scrolling arcade fighter game, built for learning game development fundamentals.

## 🎯 Features

### Core Engine
- **Cross-platform window management** with SDL2
- **60 FPS game loop** with frame capping and delta time
- **Comprehensive input handling** (keyboard/mouse)
- **Advanced rendering system** (shapes, sprites, parallax backgrounds)
- **Entity-Component-System (ECS)** architecture
- **Professional state management** (Menu, Playing, Game Over)
- **AABB collision detection** with physics response

### Game Features
- **Animated start menu** with retro arcade styling
- **Side-scrolling gameplay** with smooth camera following
- **Parallax background layers** (mountains, buildings, sky)
- **Player character** with movement and boundaries
- **Multiple enemy types** with varied AI patterns
- **30-second timed gameplay** with countdown and warnings
- **Scoring system** with movement bonuses
- **Game over screen** with restart options

## 🛠️ Prerequisites

### Required Software
- **CMake 3.16+**
- **C++17 compatible compiler** (Clang/GCC)
- **SDL2 and SDL2_image libraries**
- **pkg-config** (for dependency management)

### macOS Installation
```bash
# Install dependencies using Homebrew
brew install sdl2 sdl2_image pkg-config cmake

# Verify installation
sdl2-config --version
pkg-config --modversion sdl2
```

## 🚀 Quick Start

### 1. Clone and Build
```bash
# Navigate to your development directory
cd /path/to/your/projects

# If you already have the project, navigate to it
cd cppExploration

# Create build directory and compile
mkdir -p build
cd build
cmake ..
make

# Verify build success
ls bin/GameEngine  # Should show the executable
```

### 2. Run the Game
```bash
# From the build directory
./bin/GameEngine

# You'll see a menu:
# 1. Simple Test (basic rendering validation)
# 2. Arcade Fighter (full game experience)

# Choose option 2 for the complete game
```

### 3. Game Controls

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

## 🔧 Troubleshooting

### Common Issues

#### "SDL2/SDL.h not found"
```bash
# Reinstall SDL2 dependencies
brew uninstall sdl2 sdl2_image
brew install sdl2 sdl2_image pkg-config

# Verify installation paths
brew --prefix sdl2
pkg-config --cflags sdl2
```

#### "Game window appears blank"
1. **Check window focus**: Click on the game window
2. **Verify rendering**: Run the simple test first (`option 1`)
3. **Check console output**: Look for rendering debug messages
4. **Test basic rendering**: Run `test/test_menu_rendering.cpp`

#### "Input not responding"
1. **Ensure window has focus**: Click on the game window
2. **Check console**: Look for "Menu input check" messages
3. **Test input system**: Run `test/test_input_system.cpp`
4. **Try different keys**: Use both arrow keys and WASD

#### "Build fails"
```bash
# Clean and rebuild
cd build
make clean
rm -rf *
cmake ..
make

# Check dependencies
brew list | grep sdl2
pkg-config --exists sdl2 && echo "SDL2 found" || echo "SDL2 missing"
```

## 🎓 Learning Resources

### Documentation
- **README.md**: This file - complete setup and usage guide
- **TUTORIAL.md**: Step-by-step learning guide for game development
- **test/**: Automated testing suite with validation scripts

### Code Examples
- **examples/ECSExample.cpp**: Complete ECS demo with collision detection
- **src/main.cpp**: Main game implementation with state management
- **test/test_*.cpp**: Individual component tests and validation

### Architecture Overview
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

// ECS System Usage
auto entityManager = std::make_unique<EntityManager>();
entityManager->AddSystem<MovementSystem>();
entityManager->AddSystem<RenderSystem>(renderer);

Entity player = entityManager->CreateEntity();
entityManager->AddComponent<TransformComponent>(player, 100.0f, 100.0f);
entityManager->AddComponent<VelocityComponent>(player, 0.0f, 0.0f);
```

## 🎯 Learning Objectives

This engine demonstrates:
- **Game loop architecture** with proper frame timing
- **Real-time rendering** with SDL2 and custom graphics
- **Input handling patterns** for responsive controls
- **Entity-Component-System** for scalable game architecture
- **State management** for different game phases
- **Collision detection** with AABB and physics response
- **Camera systems** for side-scrolling gameplay
- **Professional game structure** with proper separation of concerns

## 🚀 Next Steps

### Extend the Game
1. **Add sound effects** using SDL2_mixer
2. **Implement sprite animations** for characters
3. **Create more enemy types** with different behaviors
4. **Add power-ups and collectibles**
5. **Implement multiple levels** with different backgrounds
6. **Add particle effects** for visual polish

### Learn More
1. **Study the ECS architecture** in `src/ECS/`
2. **Examine state management** in `src/Game/`
3. **Understand rendering pipeline** in `src/Engine/`
4. **Run automated tests** to see validation techniques
5. **Read TUTORIAL.md** for step-by-step learning

---

## 📞 Support

If you encounter issues:
1. **Check the troubleshooting section** above
2. **Run the automated tests** to validate your setup
3. **Examine console output** for debug information
4. **Try the simple rendering test** first to isolate issues

**The game engine is fully functional and tested!** 🎮✨
