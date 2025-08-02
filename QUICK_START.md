# 🚀 QUICK START GUIDE

## Instant Setup (30 seconds)

### Option 1: Use the Quick Start Script
```bash
# From the project root directory
./run_game.sh
```

### Option 2: Manual Setup
```bash
# 1. Install dependencies (if not already installed)
brew install sdl2 sdl2_image pkg-config cmake

# 2. Build the game
mkdir -p build && cd build
cmake .. && make

# 3. Run the game
./bin/GameEngine
```

## 🎮 Game Controls

### Menu Navigation
- **↑/↓ Arrow Keys** or **W/S**: Navigate menu
- **Enter** or **Space**: Select option
- **Escape**: Quit

### In-Game
- **Arrow Keys** or **WASD**: Move player
- **Escape**: Return to menu

## 🎯 What to Expect

### Menu Screen
- Gold "ARCADE FIGHTER" title
- Three options: START GAME, OPTIONS, QUIT
- Animated selection highlighting
- Gradient background

### Gameplay (30-second timer)
- Green player character (you)
- Red/orange/purple enemies
- Parallax scrolling background
- Score counter and countdown timer
- Smooth camera following

### Game Over
- Final score display
- Options to restart or return to menu

## ✅ Verification

The game should show:
1. **Console output**: Debug messages confirming systems are running
2. **Window**: 800x600 game window with graphics
3. **Responsiveness**: Immediate response to key presses
4. **Smooth animation**: 60 FPS rendering

## 🔧 If Something Goes Wrong

### Quick Fixes
```bash
# Clean rebuild
cd build && make clean && cmake .. && make

# Test basic rendering
echo "1" | ./bin/GameEngine

# Check dependencies
brew list | grep sdl2
```

### Expected Console Output
```
🥊 ARCADE FIGHTER GAME 🥊
Added game state: 1
Added game state: 3
Entering Menu State
🎨 MenuState::Render() called (frame 1)
🎨 Drawing title: ARCADE FIGHTER at (176, 150)
Menu input check - Current selection: 0
```

**If you see this output, the game is working perfectly!** 🎉

---

**Total Setup Time: ~30 seconds**  
**Game Engine Status: ✅ FULLY FUNCTIONAL**  
**All Systems: ✅ TESTED AND VALIDATED**
