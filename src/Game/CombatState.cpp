#include "Game/CombatState.h"
#include "Engine/Engine.h"
#include "Engine/Renderer.h"
#include "Engine/InputManager.h"
#include "Engine/AudioManager.h"
#include "Engine/BitmapFont.h"
#include "ECS/Component.h"
#include "ECS/CombatSystems.h"
#include "Game/PlayingState.h"
#include "Engine/SpriteRenderer.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <fstream>
#include <sstream>
#include <cmath>

CombatState::CombatState()
    : GameState(GameStateType::COMBAT, "Combat")
    , m_gameConfig(std::make_unique<GameConfig>())
    , m_currentPhase(CombatPhase::BATTLE_START)
    , m_currentTurnIndex(0)
    , m_phaseTimer(0.0f)
    , m_selectedAction(CombatAction::ATTACK)
    , m_selectedActionIndex(0)
    , m_experienceGained(0)
    , m_goldGained(0)
    , m_playerVictory(false)
    , m_playerFled(false)
    , m_returnX(0.0f)
    , m_returnY(0.0f)
    , m_messageTimer(0.0f)
    , m_showActionMenu(false) {

    // Load configuration
    if (!m_gameConfig->LoadConfigs()) {
        std::cerr << "Warning: Failed to load combat configs, using defaults" << std::endl;
    }

    // Initialize available actions
    m_availableActions = {
        CombatAction::ATTACK,
        CombatAction::DEFEND,
        CombatAction::MAGIC,
        CombatAction::ITEM,
        CombatAction::FLEE
    };
}

void CombatState::OnEnter() {
    std::cout << "Entering Combat State" << std::endl;

    // Don't create a new entity manager - we'll use the existing entities from PlayingState
    // The entities belong to the PlayingState's EntityManager
    m_entityManager = nullptr;

    // Combat systems will be handled directly in the CombatState for simplicity
    // This avoids the EntityManager mismatch issue

    // Load combat audio (safely)
    if (GetEngine() && GetEngine()->GetAudioManager()) {
        auto* audioManager = GetEngine()->GetAudioManager();

        // Try to load combat sounds (don't crash if files don't exist)
        try {
            audioManager->LoadSound("combat_attack", "assets/sounds/attack.wav", SoundType::SOUND_EFFECT);
            audioManager->LoadSound("combat_magic", "assets/sounds/magic.wav", SoundType::SOUND_EFFECT);
            audioManager->LoadSound("combat_victory", "assets/sounds/victory.wav", SoundType::SOUND_EFFECT);
            audioManager->LoadSound("combat_defeat", "assets/sounds/defeat.wav", SoundType::SOUND_EFFECT);
            audioManager->LoadMusic("combat_music", "assets/music/combat.wav");

            // Start combat music
            audioManager->PlayMusic("combat_music", m_gameConfig->GetCombatMusicVolume(), -1);
        } catch (...) {
            std::cout << "Warning: Could not load combat audio files" << std::endl;
        }
    }

    // Combat events will be handled directly in the CombatState methods

    // Reset combat state
    m_currentPhase = CombatPhase::BATTLE_START;
    m_phaseTimer = 0.0f;
    m_currentTurnIndex = 0;
    m_selectedActionIndex = 0;
    m_showActionMenu = false;

    // Show battle start message
    ShowMessage("Battle begins!", 2.0f);

    // Initialize with dummy participants for now (will be set properly when combat is triggered)
    // This prevents crashes when no participants are set
    if (m_participants.empty()) {
        std::cout << "Warning: No combat participants set, using dummy data" << std::endl;
        // Add a dummy player and enemy to prevent crashes
        Entity dummyPlayer;
        Entity dummyEnemy;
        m_participants.emplace_back(dummyPlayer, true, 100.0f);
        m_participants.emplace_back(dummyEnemy, false, 80.0f);
    }

    // Calculate turn order
    CalculateTurnOrder();
}

void CombatState::OnExit() {
    std::cout << "Exiting Combat State" << std::endl;

    // Stop combat music and restore background music
    if (GetEngine()->GetAudioManager()) {
        auto* audioManager = GetEngine()->GetAudioManager();
        audioManager->StopMusic();
    // Log player sprite path for combat as well
    {
        const std::string path = m_gameConfig->GetPlayerSpritePath();
        std::cout << "🎨 [Combat] Player sprite resolved from config: " << path << std::endl;
    }


        // Restart background music if available
        audioManager->PlayMusic("background", m_gameConfig->GetBackgroundMusicVolume(), -1);
    }

    // Clean up combat entities
    m_entityManager.reset();
    m_participants.clear();
}

void CombatState::Update(float deltaTime) {
    // Update message timer
    if (m_messageTimer > 0.0f) {
        m_messageTimer -= deltaTime;
    }

    // Update phase timer
    m_phaseTimer += deltaTime;

    // Update based on current phase
    switch (m_currentPhase) {
        case CombatPhase::BATTLE_START:
            UpdateBattleStart(deltaTime);
            break;
        case CombatPhase::TURN_START:
            UpdateTurnStart(deltaTime);
            break;
        case CombatPhase::ACTION_SELECT:
            UpdateActionSelect(deltaTime);
            break;
        case CombatPhase::ACTION_EXECUTE:
            UpdateActionExecute(deltaTime);
            break;
        case CombatPhase::TURN_END:
            UpdateTurnEnd(deltaTime);
            break;
        case CombatPhase::BATTLE_END:
            UpdateBattleEnd(deltaTime);
            break;
        case CombatPhase::TRANSITION_OUT:
            UpdateTransitionOut(deltaTime);
            break;
    }

    // Combat systems are handled directly in the phase updates above
    // No separate EntityManager update needed
}

void CombatState::Render() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    // Clear screen with dark background for combat
    renderer->Clear(Color(20, 20, 40, 255));

    // Render combat UI
    RenderCombatUI();

    // Render participants
    RenderParticipants();

    // Render health bars
    RenderHealthBars();

    // Render action menu if active
    if (m_showActionMenu) {
        RenderActionMenu();
    }

    // Render current message
    if (m_messageTimer > 0.0f) {
        RenderMessage();
    }

    // Render turn indicator
    RenderTurnIndicator();
}

void CombatState::HandleInput() {
    auto* input = GetInputManager();
    if (!input) return;

    // Handle input based on current phase
    switch (m_currentPhase) {
        case CombatPhase::ACTION_SELECT:
            HandleActionSelection();
            break;
        case CombatPhase::BATTLE_END:
            // Any key to continue
            if (input->IsKeyJustPressed(SDL_SCANCODE_SPACE) ||
                input->IsKeyJustPressed(SDL_SCANCODE_RETURN) ||
                input->IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
                m_currentPhase = CombatPhase::TRANSITION_OUT;
                m_phaseTimer = 0.0f;
            }
            break;
        default:
            // Skip phases with space or enter
            if (input->IsKeyJustPressed(SDL_SCANCODE_SPACE) ||
                input->IsKeyJustPressed(SDL_SCANCODE_RETURN)) {
                m_phaseTimer = 999.0f; // Force phase advance
            }
            break;
    }
}

void CombatState::InitializeCombat(Entity player, const std::vector<Entity>& enemies) {
    m_participants.clear();

    // Add player with higher health
    if (player.IsValid()) {
        float playerInitiative = 100.0f; // Base initiative
        CombatParticipant playerParticipant(player, true, playerInitiative);
        playerParticipant.maxHealth = 120.0f;
        playerParticipant.currentHealth = 120.0f;
        m_participants.push_back(playerParticipant);
    }

    // Add enemies with varied health
    for (const auto& enemy : enemies) {
        if (enemy.IsValid()) {
            float enemyInitiative = 80.0f + (rand() % 40); // 80-120 initiative
            CombatParticipant enemyParticipant(enemy, false, enemyInitiative);
            enemyParticipant.maxHealth = 60.0f + (rand() % 40); // 60-100 health
            enemyParticipant.currentHealth = enemyParticipant.maxHealth;
            m_participants.push_back(enemyParticipant);
        }
    }

    std::cout << "Combat initialized with " << m_participants.size() << " participants" << std::endl;

    // Recalculate turn order with the new participants
    CalculateTurnOrder();
}

// Phase update methods (to be implemented)
void CombatState::UpdateBattleStart([[maybe_unused]] float deltaTime) {
    if (m_phaseTimer >= 2.0f) {
        m_currentPhase = CombatPhase::TURN_START;
        m_phaseTimer = 0.0f;
    }
}

void CombatState::UpdateTurnStart([[maybe_unused]] float deltaTime) {
    if (m_phaseTimer >= 1.0f) {
        if (static_cast<size_t>(m_currentTurnIndex) < m_participants.size()) {
            const auto& participant = m_participants[m_currentTurnIndex];
            if (participant.isPlayer) {
                m_currentPhase = CombatPhase::ACTION_SELECT;
                m_showActionMenu = true;
                UpdateAvailableActions();
            } else {
                m_currentPhase = CombatPhase::ACTION_EXECUTE;
            }
        }
        m_phaseTimer = 0.0f;
    }
}

void CombatState::UpdateActionSelect([[maybe_unused]] float deltaTime) {
    // Wait for player input - handled in HandleInput
}

void CombatState::UpdateActionExecute([[maybe_unused]] float deltaTime) {
    if (m_phaseTimer >= 1.0f) {
        const auto& participant = m_participants[m_currentTurnIndex];

        if (participant.isPlayer) {
            ProcessPlayerAction();
        } else {
            ProcessEnemyAction(participant);
        }

        m_currentPhase = CombatPhase::TURN_END;
        m_phaseTimer = 0.0f;
    }
}

void CombatState::UpdateTurnEnd([[maybe_unused]] float deltaTime) {
    if (m_phaseTimer >= 1.5f) {
        if (IsBattleOver()) {
            m_currentPhase = CombatPhase::BATTLE_END;
            CalculateBattleRewards();
        } else {
            AdvanceToNextTurn();
            m_currentPhase = CombatPhase::TURN_START;
        }
        m_phaseTimer = 0.0f;
    }
}

void CombatState::UpdateBattleEnd([[maybe_unused]] float deltaTime) {
    // Show instruction to continue after a short delay
    if (m_phaseTimer >= 2.0f && m_messageTimer <= 0.0f) {
        ShowMessage("Press SPACE or ENTER to continue...", 999.0f); // Long duration
    }
}

void CombatState::UpdateTransitionOut([[maybe_unused]] float deltaTime) {
    if (m_phaseTimer >= 1.0f) {
        // Return to playing state
        std::cout << "Combat ending - returning to playing state..." << std::endl;
        if (auto* stateMgr = GetStateManager()) {
            // Before popping, adjust player position in the underlying PlayingState
            auto* underlying = stateMgr->GetCurrentState(); // Currently CombatState
            // Pop will resume underlying state (PlayingState) next
            stateMgr->PopState();
            auto* resumed = stateMgr->GetCurrentState();
            if (resumed && resumed->GetType() == GameStateType::PLAYING) {
                auto* playing = dynamic_cast<PlayingState*>(resumed);
                if (playing) {
                    playing->HandlePostCombatReturn();
                }
            }
        } else {
            std::cout << "ERROR: No state manager available for combat exit!" << std::endl;
        }
    }
}

// Utility methods
void CombatState::ShowMessage(const std::string& message, float duration) {
    m_currentMessage = message;
    m_messageTimer = duration;
}

void CombatState::CalculateTurnOrder() {
    // Sort participants by initiative (highest first)
    std::sort(m_participants.begin(), m_participants.end(),
        [](const CombatParticipant& a, const CombatParticipant& b) {
            return a.initiative > b.initiative;
        });

    // Assign turn order
    for (size_t i = 0; i < m_participants.size(); ++i) {
        m_participants[i].turnOrder = static_cast<int>(i);
    }

    m_currentTurnIndex = 0;
}

bool CombatState::IsBattleOver() const {
    return AreAllEnemiesDead() || IsPlayerDead() || m_playerFled;
}

bool CombatState::AreAllEnemiesDead() const {
    for (const auto& participant : m_participants) {
        if (!participant.isPlayer && participant.isAlive) {
            return false;
        }
    }
    return true;
}

bool CombatState::IsPlayerDead() const {
    for (const auto& participant : m_participants) {
        if (participant.isPlayer && !participant.isAlive) {
            return true;
        }
    }
    return false;
}

void CombatState::AdvanceToNextTurn() {
    m_currentTurnIndex = (m_currentTurnIndex + 1) % m_participants.size();
    m_showActionMenu = false;
}

void CombatState::UpdateAvailableActions() {
    // For now, all actions are available
    // TODO: Check MP for magic, items in inventory, etc.
}

void CombatState::CalculateBattleRewards() {
    if (AreAllEnemiesDead()) {
        m_playerVictory = true;
        m_experienceGained = 50; // Base XP
        m_goldGained = 25; // Base gold
        ShowMessage("Victory! Gained " + std::to_string(m_experienceGained) + " XP!", 3.0f);
        std::cout << "BATTLE WON - Player victory!" << std::endl;
    } else if (IsPlayerDead()) {
        m_playerVictory = false;
        ShowMessage("Defeat...", 3.0f);
        std::cout << "BATTLE LOST - Player defeated!" << std::endl;
    } else if (m_playerFled) {
        ShowMessage("Fled from battle!", 2.0f);
        std::cout << "BATTLE FLED - Player escaped!" << std::endl;
    }
}

void CombatState::HandleCombatEvent(const CombatEvent& event) {
    auto* audioManager = GetEngine()->GetAudioManager();

    switch (event.type) {
        case CombatEvent::Type::TURN_START:
            // Handle turn start
            if (!event.message.empty()) {
                ShowMessage(event.message, 1.5f);
            }
            break;

        case CombatEvent::Type::ACTION_EXECUTED:
            // Handle action execution and play appropriate sound
            if (!event.message.empty()) {
                ShowMessage(event.message, 2.0f);

                // Play sound based on action type
                if (audioManager) {
                    if (event.message.find("Attack") != std::string::npos) {
                        audioManager->PlaySound("combat_attack", m_gameConfig->GetAttackSoundVolume());
                    } else if (event.message.find("Magic") != std::string::npos) {
                        audioManager->PlaySound("combat_magic", m_gameConfig->GetMagicSoundVolume());
                    }
                }
            }
            break;

        case CombatEvent::Type::DAMAGE_DEALT:
            // Handle damage dealt
            if (!event.message.empty()) {
                ShowMessage(event.message, 1.5f);

                // Play attack sound for damage
                if (audioManager) {
                    audioManager->PlaySound("combat_attack", m_gameConfig->GetAttackSoundVolume());
                }
            }
            break;

        case CombatEvent::Type::ENTITY_DEFEATED:
            // Handle entity defeat
            if (!event.message.empty()) {
                ShowMessage(event.message, 2.0f);
            }
            break;

        case CombatEvent::Type::BATTLE_END:
            // Handle battle end with victory/defeat sound
            m_currentPhase = CombatPhase::BATTLE_END;
            m_phaseTimer = 0.0f;

            if (audioManager) {
                if (m_playerVictory) {
                    audioManager->PlaySound("combat_victory", m_gameConfig->GetVictorySoundVolume());
                } else {
                    audioManager->PlaySound("combat_defeat", m_gameConfig->GetDefeatSoundVolume());
                }
            }
            break;
    }
}

// Combat action processing
void CombatState::ProcessPlayerAction() {
    const auto& participant = m_participants[m_currentTurnIndex];

    switch (m_selectedAction) {
        case CombatAction::ATTACK:
            // Find first alive enemy
            for (const auto& enemy : m_participants) {
                if (!enemy.isPlayer && enemy.isAlive) {
                    ExecuteAttack(participant.entity, enemy.entity);
                    break;
                }
            }
            break;
        case CombatAction::DEFEND:
            ExecuteDefend(participant.entity);
            break;
        case CombatAction::MAGIC:
            // TODO: Implement magic system
            ShowMessage("Magic not yet implemented!", 1.5f);
            break;
        case CombatAction::ITEM:
            // TODO: Implement item system
            ShowMessage("Items not yet implemented!", 1.5f);
            break;
        case CombatAction::FLEE:
            if (AttemptFlee()) {
                m_playerFled = true;
                ShowMessage("Successfully fled!", 2.0f);
            } else {
                ShowMessage("Couldn't escape!", 1.5f);
            }
            break;
    }
}

void CombatState::ProcessEnemyAction(const CombatParticipant& participant) {
    // Simple AI: always attack player
    for (const auto& player : m_participants) {
        if (player.isPlayer && player.isAlive) {
            ExecuteAttack(participant.entity, player.entity);
            break;
        }
    }
}

void CombatState::ExecuteAttack([[maybe_unused]] Entity attacker, Entity target) {
    // Basic attack calculation
    int damage = 15 + (rand() % 10); // 15-25 damage

    // Apply damage to target participant
    for (auto& participant : m_participants) {
        if (participant.entity.GetID() == target.GetID()) {
            participant.currentHealth -= damage;
            if (participant.currentHealth <= 0) {
                participant.currentHealth = 0;
                participant.isAlive = false;
            }

            std::string targetName = participant.isPlayer ? "Player" : "Enemy";
            ShowMessage(targetName + " takes " + std::to_string(damage) + " damage!", 1.5f);

            if (!participant.isAlive) {
                ShowMessage(targetName + " is defeated!", 2.0f);
            }
            break;
        }
    }
}

void CombatState::ExecuteDefend([[maybe_unused]] Entity defender) {
    ShowMessage("Defending! Damage reduced next turn.", 1.5f);
    // TODO: Apply defense buff
}

void CombatState::ExecuteMagic([[maybe_unused]] Entity caster, [[maybe_unused]] Entity target) {
    // TODO: Implement magic system
}

bool CombatState::AttemptFlee() {
    // 75% chance to flee successfully
    return (rand() % 100) < 75;
}

// Input handling
void CombatState::HandleActionSelection() {
    auto* input = GetInputManager();
    if (!input) return;

    // Navigate action menu
    if (input->IsKeyJustPressed(SDL_SCANCODE_UP) || input->IsKeyJustPressed(SDL_SCANCODE_W)) {
        m_selectedActionIndex = (m_selectedActionIndex - 1 + m_availableActions.size()) % m_availableActions.size();
        m_selectedAction = m_availableActions[m_selectedActionIndex];
    }
    if (input->IsKeyJustPressed(SDL_SCANCODE_DOWN) || input->IsKeyJustPressed(SDL_SCANCODE_S)) {
        m_selectedActionIndex = (m_selectedActionIndex + 1) % m_availableActions.size();
        m_selectedAction = m_availableActions[m_selectedActionIndex];
    }

    // Select action
    if (input->IsKeyJustPressed(SDL_SCANCODE_RETURN) || input->IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
        m_currentPhase = CombatPhase::ACTION_EXECUTE;
        m_showActionMenu = false;
        m_phaseTimer = 0.0f;
    }
}

bool CombatState::CanUseMagic([[maybe_unused]] Entity entity) const {
    // TODO: Check if entity has MP and magic abilities
    return true;
}

bool CombatState::CanUseItems([[maybe_unused]] Entity entity) const {
    // TODO: Check if entity has items in inventory
    return true;
}

// Rendering methods
void CombatState::RenderCombatUI() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    // Render combat title at top
    std::string title = "COMBAT";
    int titleWidth = title.length() * 6 * 3; // 6 pixels per char * 3 scale
    int titleX = (800 - titleWidth) / 2;
    int titleY = 10; // Moved higher

    BitmapFont::DrawText(renderer, title, titleX, titleY, 3, Color(255, 255, 0, 255)); // Yellow title

    // Render phase indicator below title
    std::string phaseText = "BATTLE START";
    switch (m_currentPhase) {
        case CombatPhase::BATTLE_START: phaseText = "BATTLE START"; break;
        case CombatPhase::TURN_START: phaseText = "TURN START"; break;
        case CombatPhase::ACTION_SELECT: phaseText = "SELECT ACTION"; break;
        case CombatPhase::ACTION_EXECUTE: phaseText = "EXECUTING"; break;
        case CombatPhase::TURN_END: phaseText = "TURN END"; break;
        case CombatPhase::BATTLE_END: phaseText = "BATTLE END"; break;
        case CombatPhase::TRANSITION_OUT: phaseText = "ENDING"; break;
    }

    int phaseWidth = phaseText.length() * 6 * 1; // 6 pixels per char * 1 scale
    int phaseX = (800 - phaseWidth) / 2;
    int phaseY = 50; // Moved closer to title

    BitmapFont::DrawText(renderer, phaseText, phaseX, phaseY, 1, Color(200, 200, 200, 255)); // Gray phase text
}

void CombatState::RenderParticipants() {
    auto* renderer = GetRenderer();
    if (!renderer || m_participants.empty()) return;

    // Render player on left center
    int playerX = 200;
    int playerY = 350;

    // Render enemies on right center
    int enemyStartX = 450;
    int enemyY = 320;
    int enemySpacing = 60;

    int enemyIndex = 0;
    for (size_t i = 0; i < m_participants.size(); ++i) {
        const auto& participant = m_participants[i];

        if (participant.isPlayer) {
            // Render player sprite
            int spriteW = m_gameConfig->GetAnimationSpriteWidth();
            int spriteH = m_gameConfig->GetAnimationSpriteHeight();
            int totalFrames = m_gameConfig->GetAnimationTotalFrames();
            float spriteScale = m_gameConfig->GetAnimationSpriteScale();

            // Simple idle animation cycle based on phase timer
            int frameIndex = static_cast<int>(std::fmod(m_phaseTimer * 5.0f, static_cast<float>(totalFrames)));
            SpriteFrame frame = SpriteRenderer::CreateFrame(frameIndex, spriteW, spriteH, totalFrames);
            const std::string playerSpritePath = m_gameConfig->GetPlayerSpritePath();
            // Player should face right on the left side of the screen
            SpriteRenderer::RenderSprite(renderer, playerSpritePath,
                                         playerX, playerY, frame, /*flipHorizontal=*/false, spriteScale);
        } else {
            // Render enemy using frog sprite
            int x = enemyStartX;
            int y = enemyY + enemyIndex * enemySpacing; // Offset for multiple enemies
            static std::shared_ptr<Texture> frog;
            static std::vector<SpriteFrame> idle;
            static bool loaded = false;
            if (!loaded) {
                frog = renderer->LoadTexture("assets/sprites/enemies/frog/frog.png");
                std::ifstream in("assets/sprites/enemies/frog/frog.spritepos");
                if (in) {
                    std::string line;
                    while (std::getline(in, line)) {
                        if (line.rfind("idle", 0) == 0) {
                            auto colon = line.find(':');
                            if (colon != std::string::npos) {
                                std::istringstream iss(line.substr(colon + 1));
                                int fx, fy, fw, fh;
                                if (iss >> fx >> fy >> fw >> fh) idle.emplace_back(fx, fy, fw, fh);
                            }
                        }
                    }
                }
                loaded = true;
            }
            if (frog && !idle.empty()) {
                int fi = static_cast<int>(std::fmod(m_phaseTimer * 5.0f, static_cast<float>(idle.size())));
                const SpriteFrame& f = idle[fi];
                Rectangle src(f.x, f.y, f.width, f.height);
                Rectangle dest(x, y, 28, 44);
                // Enemies on right should face left in combat as well
                renderer->DrawTexture(frog, src, dest, true, false);
            } else {
                Rectangle enemyRect(x, y, 28, 44);
                renderer->DrawRectangle(enemyRect, Color(255, 100, 100, 255), true);
            }
            enemyIndex++;
        }
    }
}

void CombatState::RenderActionMenu() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    // Render action menu background (bottom left, adjusted for screen size)
    int menuX = 20;
    int menuY = 450;  // Moved up to ensure it fits on screen
    int menuWidth = 180; // Reduced width
    int menuHeight = 120; // Increased height for better spacing

    Rectangle menuBg(menuX, menuY, menuWidth, menuHeight);
    renderer->DrawRectangle(menuBg, Color(0, 0, 0, 220), true);

    Rectangle menuBorder(menuX, menuY, menuWidth, menuHeight);
    renderer->DrawRectangle(menuBorder, Color(255, 255, 255, 255), false);

    // Render title
    BitmapFont::DrawText(renderer, "ACTIONS", menuX + 10, menuY + 10, 2, Color(255, 255, 0, 255));

    // Render action options
    const std::vector<std::string> actionNames = {
        "ATTACK", "DEFEND", "MAGIC", "ITEM", "FLEE"
    };

    for (size_t i = 0; i < actionNames.size(); ++i) {
        int optionY = menuY + 30 + i * 18; // Better spacing
        bool isSelected = (i == static_cast<size_t>(m_selectedActionIndex));

        Color textColor = isSelected ? Color(255, 255, 0, 255) : Color(200, 200, 200, 255);
        std::string prefix = isSelected ? "> " : "  ";

        BitmapFont::DrawText(renderer, prefix + actionNames[i], menuX + 10, optionY, 1, textColor);
    }

    // Removed duplicate action rendering loop to prevent visual artifacts
}

void CombatState::RenderHealthBars() {
    auto* renderer = GetRenderer();
    if (!renderer || m_participants.empty()) return;

    // Render health bars for all participants
    for (size_t i = 0; i < m_participants.size(); ++i) {
        const auto& participant = m_participants[i];

        // Skip invalid entities
        if (!participant.entity.IsValid()) continue;

        // Use actual participant health values
        float currentHealth = participant.currentHealth;
        float maxHealth = participant.maxHealth;

        int barWidth = 120;
        int barHeight = 12;
        int barX, barY;

        if (participant.isPlayer) {
            barX = 50;  // Player health bar on left
            barY = 200; // Moved down to avoid overlap
        } else {
            barX = 550; // Enemy health bars on right
            barY = 200 + (i - 1) * 60; // Tighter spacing for enemies
        }

        // Calculate health percentage
        float healthPercent = maxHealth > 0 ? currentHealth / maxHealth : 1.0f;
        int healthBarWidth = static_cast<int>(barWidth * healthPercent);

        // Background
        Rectangle bgRect(barX, barY, barWidth, barHeight);
        renderer->DrawRectangle(bgRect, Color(60, 60, 60, 255), true);

        // Health bar with color based on health level
        Color healthColor;
        if (healthPercent > 0.6f) {
            healthColor = Color(0, 255, 0, 255); // Green
        } else if (healthPercent > 0.3f) {
            healthColor = Color(255, 255, 0, 255); // Yellow
        } else {
            healthColor = Color(255, 0, 0, 255); // Red
        }
        Rectangle healthRect(barX, barY, healthBarWidth, barHeight);
        renderer->DrawRectangle(healthRect, healthColor, true);

        // Border
        Rectangle borderRect(barX, barY, barWidth, barHeight);
        renderer->DrawRectangle(borderRect, Color(255, 255, 255, 255), false);

        // Health text background
        Rectangle textBg(barX, barY - 15, barWidth, 12);
        renderer->DrawRectangle(textBg, Color(0, 0, 0, 180), true);

        // Health text
        std::string healthText = std::to_string(static_cast<int>(currentHealth)) + "/" + std::to_string(static_cast<int>(maxHealth));
        BitmapFont::DrawText(renderer, healthText, barX + barWidth + 10, barY - 2, 1, Color(255, 255, 255, 255));

        // Character label
        std::string label = participant.isPlayer ? "PLAYER" : "ENEMY";
        BitmapFont::DrawText(renderer, label, barX, barY - 15, 1, Color(200, 200, 200, 255));
    }
}

void CombatState::RenderMessage() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    if (m_messageTimer <= 0.0f || m_currentMessage.empty()) return;

    // Render message background (moved to center area)
    int msgX = 200;
    int msgY = 120; // Moved down to avoid turn indicator
    int msgWidth = 400;
    int msgHeight = 50;

    Rectangle msgBg(msgX, msgY, msgWidth, msgHeight);
    renderer->DrawRectangle(msgBg, Color(0, 0, 0, 180), true);

    Rectangle msgBorder(msgX, msgY, msgWidth, msgHeight);
    renderer->DrawRectangle(msgBorder, Color(255, 255, 255, 255), false);

    // Render the actual message text
    int textX = msgX + 10;
    int textY = msgY + 20;
    BitmapFont::DrawText(renderer, m_currentMessage, textX, textY, 1, Color(255, 255, 255, 255));
}

void CombatState::RenderTurnIndicator() {
    auto* renderer = GetRenderer();
    if (!renderer) return;

    if (static_cast<size_t>(m_currentTurnIndex) >= m_participants.size()) return;

    const auto& currentParticipant = m_participants[m_currentTurnIndex];

    // Render turn indicator text (moved to avoid overlap with messages)
    std::string turnText = currentParticipant.isPlayer ? "PLAYER'S TURN" : "ENEMY'S TURN";
    int textWidth = turnText.length() * 6 * 2; // 6 pixels per char * 2 scale
    int textX = (800 - textWidth) / 2;
    int textY = 75; // Moved higher to avoid message overlap

    Color turnColor = currentParticipant.isPlayer ? Color(0, 255, 0, 255) : Color(255, 100, 100, 255);
    BitmapFont::DrawText(renderer, turnText, textX, textY, 2, turnColor);

    // Render arrow indicator above current participant
    int indicatorSize = 16;
    int indicatorX, indicatorY;

    if (currentParticipant.isPlayer) {
        indicatorX = 200 + 16 - indicatorSize / 2;
        indicatorY = 330; // Above player
    } else {
        indicatorX = 450 + 14 - indicatorSize / 2;
        indicatorY = 300 + (m_currentTurnIndex - 1) * 60; // Above enemies
    }

    // Animated turn indicator
    float pulse = sin(m_phaseTimer * 4.0f) * 0.3f + 0.7f;
    Uint8 alpha = static_cast<Uint8>(255 * pulse);
    Rectangle indicator(indicatorX, indicatorY, indicatorSize, indicatorSize);
    renderer->DrawRectangle(indicator, Color(255, 255, 0, alpha), true);
}
