#!/bin/bash

echo "🧪 GAME ENGINE AUTOMATED TEST SUITE"
echo "===================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test results
TESTS_PASSED=0
TESTS_FAILED=0
TOTAL_TESTS=0

# Function to run a test
run_test() {
    local test_name="$1"
    local test_executable="$2"
    local timeout_seconds="$3"
    
    echo ""
    echo -e "${BLUE}🔍 Running: $test_name${NC}"
    echo "----------------------------------------"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    # Compile the test
    echo "Compiling $test_executable..."
    cd ../build
    
    # Create test executable
    g++ -std=c++17 -I../include \
        ../test/$test_executable.cpp \
        ../src/Engine/*.cpp \
        ../src/Game/*.cpp \
        ../src/ECS/*.cpp \
        ../src/Scenes/*.cpp \
        -lSDL2 -lSDL2_image \
        -o test_$test_executable
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}❌ COMPILATION FAILED for $test_name${NC}"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return 1
    fi
    
    echo "✅ Compilation successful"
    
    # Run the test with timeout
    echo "Running test (timeout: ${timeout_seconds}s)..."
    timeout ${timeout_seconds}s ./test_$test_executable
    
    local exit_code=$?
    
    if [ $exit_code -eq 0 ]; then
        echo -e "${GREEN}✅ PASSED: $test_name${NC}"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    elif [ $exit_code -eq 124 ]; then
        echo -e "${YELLOW}⏰ TIMEOUT: $test_name (may need manual verification)${NC}"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    else
        echo -e "${RED}❌ FAILED: $test_name (exit code: $exit_code)${NC}"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    
    # Clean up
    rm -f test_$test_executable
    cd ../test
}

# Function to run manual verification test
run_manual_test() {
    local test_name="$1"
    local instructions="$2"
    
    echo ""
    echo -e "${BLUE}🔍 Manual Test: $test_name${NC}"
    echo "----------------------------------------"
    echo -e "${YELLOW}$instructions${NC}"
    echo ""
    echo "Press 'y' if the test passed, 'n' if it failed, or 's' to skip:"
    read -n 1 -r response
    echo ""
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    case $response in
        [Yy])
            echo -e "${GREEN}✅ MANUAL PASS: $test_name${NC}"
            TESTS_PASSED=$((TESTS_PASSED + 1))
            ;;
        [Nn])
            echo -e "${RED}❌ MANUAL FAIL: $test_name${NC}"
            TESTS_FAILED=$((TESTS_FAILED + 1))
            ;;
        *)
            echo -e "${YELLOW}⏭️  SKIPPED: $test_name${NC}"
            TOTAL_TESTS=$((TOTAL_TESTS - 1))
            ;;
    esac
}

# Start testing
echo "Starting automated test suite..."
echo "Tests will run automatically with timeouts."
echo ""

# Make sure we're in the test directory
cd "$(dirname "$0")"

# Test 1: Basic Rendering
run_test "Basic Rendering System" "test_basic_rendering" 10

# Test 2: State Management
run_test "State Management System" "test_state_management" 15

# Test 3: Input System (this one might need manual verification)
echo ""
echo -e "${YELLOW}⚠️  Next test requires manual interaction${NC}"
echo "The input test will open a window. Please interact with it as instructed."
echo "Press Enter to continue or 's' to skip..."
read -n 1 -r skip_input
echo ""

if [[ ! $skip_input =~ ^[Ss]$ ]]; then
    run_test "Input System" "test_input_system" 15
else
    echo -e "${YELLOW}⏭️  SKIPPED: Input System Test${NC}"
fi

# Manual verification tests
echo ""
echo -e "${BLUE}🔍 Manual Verification Tests${NC}"
echo "These tests require you to run the actual game and verify functionality."

# Test the actual game
run_manual_test "Main Game Menu Navigation" \
"Run: cd ../build && echo '2' | ./bin/GameEngine
You should see:
1. A window opens with 'ARCADE FIGHTER' title
2. Menu options: START GAME, OPTIONS, QUIT
3. You can navigate with arrow keys
4. Selection highlights change
Did this work correctly?"

run_manual_test "Game State Transitions" \
"From the menu, press Enter on START GAME
You should see:
1. Transition to playing state
2. Green player character visible
3. Red enemy characters visible
4. Scrolling background
5. HUD with score and timer
Did this work correctly?"

run_manual_test "Player Movement and Camera" \
"In the playing state, use WASD or arrow keys
You should see:
1. Green player moves smoothly
2. Camera follows player
3. Background scrolls
4. Player stays within bounds
Did this work correctly?"

run_manual_test "Game Timer and Game Over" \
"Wait for the 30-second timer to expire
You should see:
1. Timer counts down in HUD
2. Warning appears when < 10 seconds
3. Game Over screen appears
4. Final score displayed
5. Options to restart or return to menu
Did this work correctly?"

# Final results
echo ""
echo "🏁 TEST SUITE COMPLETE"
echo "======================"
echo -e "Tests Passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests Failed: ${RED}$TESTS_FAILED${NC}"
echo -e "Total Tests:  $TOTAL_TESTS"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}🎉 ALL TESTS PASSED! Game engine is working correctly.${NC}"
    exit 0
else
    echo -e "${RED}❌ Some tests failed. Check the output above for details.${NC}"
    exit 1
fi
