#!/bin/bash

# 🎮 Everharvest Voyager V Game - Quick Start Script
# ==========================================

echo "🎮 Everharvest Voyager V GAME ENGINE"
echo "=============================="

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}❌ Error: Please run this script from the project root directory${NC}"
    echo "Expected to find CMakeLists.txt in current directory"
    exit 1
fi

# Check dependencies
echo -e "${BLUE}🔍 Checking dependencies...${NC}"

if ! command -v brew &> /dev/null; then
    echo -e "${RED}❌ Homebrew not found. Please install Homebrew first:${NC}"
    echo "/bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
    exit 1
fi

if ! brew list sdl2 &> /dev/null; then
    echo -e "${YELLOW}⚠️  SDL2 not found. Installing dependencies...${NC}"
    brew install sdl2 sdl2_image pkg-config cmake
else
    echo -e "${GREEN}✅ SDL2 dependencies found${NC}"
fi

# Build the project
echo -e "${BLUE}🔨 Building project...${NC}"

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

if ! cmake .. ; then
    echo -e "${RED}❌ CMake configuration failed${NC}"
    exit 1
fi

if ! make ; then
    echo -e "${RED}❌ Build failed${NC}"
    exit 1
fi

echo -e "${GREEN}✅ Build successful!${NC}"

# Check if executable exists
if [ ! -f "bin/GameEngine" ]; then
    echo -e "${RED}❌ Game executable not found at bin/GameEngine${NC}"
    exit 1
fi

echo -e "${GREEN}✅ Game executable ready${NC}"

# Run the game
echo ""
echo -e "${BLUE}🚀 Starting Everharvest Voyager V Game...${NC}"
echo ""
echo "Game Options:"
echo "  1. Simple Test (basic rendering validation)"
echo "  2. Everharvest Voyager V (full game experience)"
echo ""
echo -e "${YELLOW}Recommendation: Choose option 2 for the complete game!${NC}"
echo ""

# Start the game from the bin directory so assets are found correctly
cd bin
./GameEngine

echo ""
echo -e "${GREEN}🎮 Thanks for playing Everharvest Voyager V!${NC}"
