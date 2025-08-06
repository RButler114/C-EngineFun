/**
 * @file GameStateManager.cpp
 * @brief Implementation of the game state management system
 * @author Ryan Butler
 * @date 2025
 */

#include "Game/GameStateManager.h"
#include "Engine/Engine.h"
#include <iostream>

/**
 * @brief Constructor - initializes state management system
 *
 * Creates a new GameStateManager connected to the specified engine.
 * Initializes all tracking variables for deferred state changes.
 *
 * @param engine Pointer to main Engine instance (provides access to all systems)
 *
 * @note Engine pointer is stored and passed to all added states
 * @note Deferred change system starts in "no pending changes" state
 * @note State stack starts empty - add states with AddState() and activate with PushState()
 */
GameStateManager::GameStateManager(Engine* engine)
    : m_engine(engine)           // Store engine for passing to states
    , m_pendingChange(false)     // No state change pending initially
    , m_pendingPop(false)        // No state pop pending initially
{
    std::cout << "✅ GameStateManager initialized" << std::endl;
}

/**
 * @brief Destructor - ensures clean shutdown of all states
 *
 * Automatically calls ClearStates() to properly exit the current state
 * and clean up all resources. This follows RAII principles.
 *
 * @note All states are properly exited with OnExit() calls
 * @note Smart pointers automatically handle memory cleanup
 */
GameStateManager::~GameStateManager() {
    ClearStates(); // Proper cleanup via RAII
    std::cout << "✅ GameStateManager shut down" << std::endl;
}

/**
 * @brief Register a new state with the manager
 *
 * Adds a state to the manager's registry and configures it with
 * engine and manager pointers. The state is stored but not activated
 * until PushState() or ChangeState() is called.
 *
 * @param type GameStateType identifier for this state
 * @param state Unique pointer to state object (transfers ownership)
 *
 * @note Manager takes ownership of the state object
 * @note State is automatically configured with engine and manager pointers
 * @note Replaces any existing state with the same type
 * @note State is stored but not activated (use PushState/ChangeState to activate)
 *
 * @example
 * ```cpp
 * manager.AddState(GameStateType::MENU, std::make_unique<MenuState>());
 * manager.AddState(GameStateType::PLAYING, std::make_unique<PlayingState>());
 * // States are registered but not active yet
 * ```
 */
void GameStateManager::AddState(GameStateType type, std::unique_ptr<GameState> state) {
    if (state) {
        // Configure state with access to engine systems
        state->SetEngine(m_engine);        // Provides access to renderer, input, etc.
        state->SetStateManager(this);      // Allows state to request transitions

        // Store state in registry (transfers ownership)
        m_states[type] = std::move(state);

        std::cout << "✅ Registered state: " << static_cast<int>(type)
                  << " (" << m_states[type]->GetName() << ")" << std::endl;
    } else {
        std::cerr << "❌ Attempted to add null state for type: " << static_cast<int>(type) << std::endl;
    }
}

/**
 * @brief Push a state onto the stack (layered/pause functionality)
 *
 * Adds a new state on top of the current state without removing it.
 * The current state is "paused" (stops receiving updates) but remains
 * in memory with all its data intact. Perfect for pause menus, dialogs,
 * or any overlay that should return to the previous state.
 *
 * @param type GameStateType to push onto the stack
 *
 * @note Current state is NOT exited (OnExit not called) - just paused
 * @note New state receives OnEnter() call for initialization
 * @note Does nothing if the specified state type is not registered
 * @note State change is immediate (not deferred like ChangeState)
 *
 * Stack behavior:
 * - Before: [PLAYING] (active)
 * - After:  [PLAYING, PAUSED] (PAUSED active, PLAYING paused)
 *
 * @example
 * ```cpp
 * // Player presses ESC during gameplay
 * manager.PushState(GameStateType::PAUSED);
 * // Now pause menu is active, but game state is preserved underneath
 * ```
 */
void GameStateManager::PushState(GameStateType type) {
    auto it = m_states.find(type);
    if (it != m_states.end()) {
        // IMPORTANT: Don't call OnExit() on current state when pushing
        // This preserves the current state's data (entities, progress, etc.)
        // The state is "paused" but not destroyed

        // Add new state to top of stack
        m_stateStack.push(type);

        // Initialize the new state
        it->second->OnEnter();

        std::cout << "📌 Pushed state: " << it->second->GetName()
                  << " (stack size: " << m_stateStack.size() << ")" << std::endl;
    } else {
        std::cerr << "❌ Cannot push unregistered state type: " << static_cast<int>(type) << std::endl;
    }
}

/**
 * @brief Pop current state and return to previous state
 *
 * Removes the current state from the stack and returns to the previous
 * state. The current state is properly exited (OnExit called) and the
 * previous state resumes without re-initialization.
 *
 * @note Current state receives OnExit() call for cleanup
 * @note Previous state does NOT receive OnEnter() (it was never exited)
 * @note Does nothing if stack is empty (safety check)
 * @note Previous state resumes exactly where it left off
 *
 * Stack behavior:
 * - Before: [PLAYING, PAUSED] (PAUSED active)
 * - After:  [PLAYING] (PLAYING active, resumed)
 *
 * @example
 * ```cpp
 * // Player selects "Resume" from pause menu
 * manager.PopState();
 * // Pause menu is destroyed, gameplay resumes exactly where it left off
 * ```
 */
void GameStateManager::PopState() {
    if (!m_stateStack.empty()) {
        // Get current state for proper cleanup
        auto currentIt = m_states.find(m_stateStack.top());
        if (currentIt != m_states.end()) {
            std::cout << "📤 Popping state: " << currentIt->second->GetName() << std::endl;

            // Properly exit the current state
            currentIt->second->OnExit();
        }

        // Remove current state from stack
        m_stateStack.pop();

        // IMPORTANT: Don't call OnEnter() on the resumed state
        // The underlying state was paused (not exited), so it should
        // resume naturally without re-initialization
        if (!m_stateStack.empty()) {
            auto newCurrentIt = m_states.find(m_stateStack.top());
            if (newCurrentIt != m_states.end()) {
                std::cout << "▶️  Resumed state: " << newCurrentIt->second->GetName()
                          << " (stack size: " << m_stateStack.size() << ")" << std::endl;
            }
        } else {
            std::cout << "📭 State stack is now empty" << std::endl;
        }
    } else {
        std::cout << "⚠️  Cannot pop from empty state stack" << std::endl;
    }
}

/**
 * @brief Request a state change (deferred until safe)
 *
 * Requests a transition to a different state. The change is deferred
 * until the next Update() call to ensure it happens at a safe point
 * in the frame (not during input processing or rendering).
 *
 * @param type GameStateType to change to
 *
 * @note State change is DEFERRED - happens at start of next Update()
 * @note Current state will be properly exited (OnExit called)
 * @note New state will be properly entered (OnEnter called)
 * @note Entire state stack is cleared and replaced with new state
 * @note Safe to call from within state Update(), Render(), or HandleInput()
 *
 * @example
 * ```cpp
 * // In a game state's HandleInput() method:
 * if (inputManager->IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
 *     GetStateManager()->ChangeState(GameStateType::MENU);
 *     // Change will happen at start of next frame
 * }
 * ```
 */
void GameStateManager::ChangeState(GameStateType type) {
    // Mark that a state change is pending
    m_pendingChange = true;
    m_pendingState = type;

    std::cout << "🔄 State change requested: " << static_cast<int>(type)
              << " (will process next frame)" << std::endl;
}

/**
 * @brief Clear all states from the stack and registry
 *
 * Removes all states from the stack and clears the state registry.
 * The current active state is properly exited before removal.
 *
 * @note Current state receives OnExit() call if stack is not empty
 * @note All states are removed from both stack and registry
 * @note After this call, HasStates() returns false
 * @note Typically used during shutdown or complete game reset
 */
void GameStateManager::ClearStates() {
    std::cout << "🧹 Clearing all states..." << std::endl;

    // Pop all states from stack (ensures proper OnExit() calls)
    while (!m_stateStack.empty()) {
        PopState();
    }

    // Clear the state registry (releases all state objects)
    m_states.clear();

    std::cout << "✅ All states cleared" << std::endl;
}

GameState* GameStateManager::GetCurrentState() const {
    if (m_stateStack.empty()) {
        return nullptr;
    }
    
    auto it = m_states.find(m_stateStack.top());
    return (it != m_states.end()) ? it->second.get() : nullptr;
}

void GameStateManager::Update(float deltaTime) {
    ProcessPendingChanges();
    
    GameState* currentState = GetCurrentState();
    if (currentState) {
        currentState->Update(deltaTime);
    }
}

void GameStateManager::Render() {
    GameState* currentState = GetCurrentState();
    if (currentState) {
        currentState->Render();
    }
}

void GameStateManager::HandleInput() {
    GameState* currentState = GetCurrentState();
    if (currentState) {
        currentState->HandleInput();
    }
}

void GameStateManager::ProcessPendingChanges() {
    if (m_pendingChange) {
        // Exit current state
        if (!m_stateStack.empty()) {
            auto currentIt = m_states.find(m_stateStack.top());
            if (currentIt != m_states.end()) {
                currentIt->second->OnExit();
            }
            m_stateStack.pop();
        }
        
        // Enter new state
        auto newIt = m_states.find(m_pendingState);
        if (newIt != m_states.end()) {
            m_stateStack.push(m_pendingState);
            newIt->second->OnEnter();
            std::cout << "Changed to state: " << newIt->second->GetName() << std::endl;
        }
        
        m_pendingChange = false;
    }
    
    if (m_pendingPop) {
        PopState();
        m_pendingPop = false;
    }
}
