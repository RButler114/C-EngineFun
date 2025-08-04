/**
 * @file GameConfig.cpp
 * @brief Implementation of GameConfig wrapper class
 * @author Ryan Butler
 * @date 2025
 */

#include "Game/GameConfig.h"
#include <iostream>
#include <sys/stat.h>
#include <ctime>

GameConfig::GameConfig()
    : m_gameplayConfig(std::make_unique<ConfigManager>())
    , m_characterConfig(std::make_unique<ConfigManager>())
    , m_levelConfig(std::make_unique<ConfigManager>())
    , m_lastGameplayModTime(0)
    , m_lastCharacterModTime(0)
    , m_lastLevelModTime(0)
    , m_gameplayConfigPath("assets/config/gameplay.ini")
    , m_characterConfigPath("assets/config/characters.ini")
    , m_levelConfigPath("")
    , m_currentLevel("") {
}

bool GameConfig::LoadConfigs() {
    bool success = true;

    // Load gameplay configuration
    if (!m_gameplayConfig->LoadFromFile(m_gameplayConfigPath)) {
        std::cerr << "Warning: Failed to load " << m_gameplayConfigPath << ", using defaults" << std::endl;
        success = false;
    } else {
        m_lastGameplayModTime = GetFileModificationTime(m_gameplayConfigPath);
        std::cout << "Loaded gameplay config from " << m_gameplayConfigPath << std::endl;
    }

    // Load character configuration (existing)
    if (!m_characterConfig->LoadFromFile(m_characterConfigPath)) {
        std::cerr << "Warning: Failed to load " << m_characterConfigPath << ", using defaults" << std::endl;
        success = false;
    } else {
        m_lastCharacterModTime = GetFileModificationTime(m_characterConfigPath);
        std::cout << "Loaded character config from " << m_characterConfigPath << std::endl;
    }

    return success;
}

bool GameConfig::ReloadConfigs() {
    return LoadConfigs();
}

// Player settings
float GameConfig::GetPlayerStartX() const {
    return GetConfigValueFloat("player", "start_x", 100.0f);
}

float GameConfig::GetPlayerStartY() const {
    return GetConfigValueFloat("player", "start_y", 400.0f);
}

float GameConfig::GetPlayerMovementSpeed() const {
    return GetConfigValueFloat("player", "movement_speed", 250.0f);
}

float GameConfig::GetPlayerMovementThreshold() const {
    return GetConfigValueFloat("player", "movement_threshold", 0.1f);
}

float GameConfig::GetPlayerSkyLimit() const {
    return GetConfigValueFloat("player", "sky_limit", 350.0f);
}

float GameConfig::GetPlayerGroundLimit() const {
    return GetConfigValueFloat("player", "ground_limit", 450.0f);
}

float GameConfig::GetPlayerCameraLeftBoundary() const {
    return GetConfigValueFloat("player", "camera_left_boundary", -50.0f);
}

// Camera settings
float GameConfig::GetCameraTargetOffsetX() const {
    return m_gameplayConfig->Get("camera", "target_offset_x", -300.0f).AsFloat();
}

float GameConfig::GetCameraFollowSpeed() const {
    return m_gameplayConfig->Get("camera", "follow_speed", 3.0f).AsFloat();
}

float GameConfig::GetCameraMinX() const {
    return m_gameplayConfig->Get("camera", "min_x", 0.0f).AsFloat();
}

// Game rules
float GameConfig::GetGameDurationSeconds() const {
    return GetConfigValueFloat("game_rules", "game_duration_seconds", 30.0f);
}

float GameConfig::GetLowTimeWarningThreshold() const {
    return GetConfigValueFloat("game_rules", "low_time_warning_threshold", 10.0f);
}

int GameConfig::GetBaseScorePerFrame() const {
    return GetConfigValueInt("game_rules", "base_score_per_frame", 1);
}

int GameConfig::GetMovementBonusScore() const {
    return GetConfigValueInt("game_rules", "movement_bonus_score", 2);
}

float GameConfig::GetDebugOutputInterval() const {
    return GetConfigValueFloat("game_rules", "debug_output_interval", 5.0f);
}

int GameConfig::GetDebugFrameInterval() const {
    return GetConfigValueInt("game_rules", "debug_frame_interval", 600);
}

// Enemy settings
int GameConfig::GetEnemyCount() const {
    return GetConfigValueInt("enemies", "enemy_count", 8);
}

float GameConfig::GetEnemySpawnStartX() const {
    return GetConfigValueFloat("enemies", "spawn_start_x", 400.0f);
}

float GameConfig::GetEnemySpawnSpacingX() const {
    return GetConfigValueFloat("enemies", "spawn_spacing_x", 200.0f);
}

float GameConfig::GetEnemySpawnHeightVariation() const {
    return GetConfigValueFloat("enemies", "spawn_height_variation", 30.0f);
}

float GameConfig::GetEnemyBaseVelocityX() const {
    return GetConfigValueFloat("enemies", "base_velocity_x", -30.0f);
}

float GameConfig::GetEnemyVelocityVariation() const {
    return GetConfigValueFloat("enemies", "velocity_variation", 20.0f);
}

float GameConfig::GetEnemyVerticalMovementRange() const {
    return GetConfigValueFloat("enemies", "vertical_movement_range", 10.0f);
}

float GameConfig::GetEnemyRespawnDistance() const {
    return GetConfigValueFloat("enemies", "respawn_distance", -200.0f);
}

float GameConfig::GetEnemyRespawnOffset() const {
    return GetConfigValueFloat("enemies", "respawn_offset", 1000.0f);
}

int GameConfig::GetEnemyRespawnRandomRange() const {
    return GetConfigValueInt("enemies", "respawn_random_range", 500);
}

int GameConfig::GetEnemyHeightRandomRange() const {
    return GetConfigValueInt("enemies", "height_random_range", 60);
}

int GameConfig::GetEnemyWidth() const {
    return GetConfigValueInt("enemies", "enemy_width", 28);
}

int GameConfig::GetEnemyHeight() const {
    return GetConfigValueInt("enemies", "enemy_height", 44);
}

// Animation settings
float GameConfig::GetAnimationFrameDuration() const {
    return m_gameplayConfig->Get("animation", "frame_duration", 0.15f).AsFloat();
}

int GameConfig::GetAnimationTotalFrames() const {
    return m_gameplayConfig->Get("animation", "total_frames", 3).AsInt();
}

int GameConfig::GetAnimationSpriteWidth() const {
    return m_gameplayConfig->Get("animation", "sprite_width", 18).AsInt();
}

int GameConfig::GetAnimationSpriteHeight() const {
    return m_gameplayConfig->Get("animation", "sprite_height", 48).AsInt();
}

float GameConfig::GetAnimationSpriteScale() const {
    return m_gameplayConfig->Get("animation", "sprite_scale", 1.0f).AsFloat();
}

float GameConfig::GetApproximateFrameTime() const {
    return m_gameplayConfig->Get("animation", "approximate_frame_time", 0.016f).AsFloat();
}

// Audio settings
float GameConfig::GetBackgroundMusicVolume() const {
    return m_gameplayConfig->Get("audio", "background_music_volume", 0.3f).AsFloat();
}

float GameConfig::GetJumpSoundVolume() const {
    return m_gameplayConfig->Get("audio", "jump_sound_volume", 0.8f).AsFloat();
}

float GameConfig::GetCollisionSoundVolume() const {
    return m_gameplayConfig->Get("audio", "collision_sound_volume", 0.6f).AsFloat();
}

// Visual settings
int GameConfig::GetScreenWidth() const {
    return m_gameplayConfig->Get("visual", "screen_width", 800).AsInt();
}

int GameConfig::GetScreenHeight() const {
    return m_gameplayConfig->Get("visual", "screen_height", 600).AsInt();
}

int GameConfig::GetGroundY() const {
    return m_gameplayConfig->Get("visual", "ground_y", 500).AsInt();
}

int GameConfig::GetGroundHeight() const {
    return m_gameplayConfig->Get("visual", "ground_height", 100).AsInt();
}

int GameConfig::GetGroundDetailCount() const {
    return m_gameplayConfig->Get("visual", "ground_detail_count", 10).AsInt();
}

int GameConfig::GetGroundDetailWidth() const {
    return m_gameplayConfig->Get("visual", "ground_detail_width", 40).AsInt();
}

int GameConfig::GetGroundDetailHeight() const {
    return m_gameplayConfig->Get("visual", "ground_detail_height", 10).AsInt();
}

int GameConfig::GetGroundDetailSpacing() const {
    return m_gameplayConfig->Get("visual", "ground_detail_spacing", 80).AsInt();
}

int GameConfig::GetGroundDetailOffset() const {
    return m_gameplayConfig->Get("visual", "ground_detail_offset", 20).AsInt();
}

// Combat settings
float GameConfig::GetBaseAttackDamage() const {
    return GetConfigValueFloat("combat", "base_attack_damage", 15.0f);
}

float GameConfig::GetBaseDefense() const {
    return GetConfigValueFloat("combat", "base_defense", 5.0f);
}

float GameConfig::GetBaseMagicPower() const {
    return GetConfigValueFloat("combat", "base_magic_power", 10.0f);
}

float GameConfig::GetBaseSpeed() const {
    return GetConfigValueFloat("combat", "base_speed", 100.0f);
}

float GameConfig::GetCriticalChance() const {
    return GetConfigValueFloat("combat", "critical_chance", 5.0f);
}

float GameConfig::GetCriticalMultiplier() const {
    return GetConfigValueFloat("combat", "critical_multiplier", 2.0f);
}

float GameConfig::GetFleeSuccessRate() const {
    return GetConfigValueFloat("combat", "flee_success_rate", 75.0f);
}

// Combat timing
float GameConfig::GetTurnStartDelay() const {
    return GetConfigValueFloat("combat", "turn_start_delay", 1.0f);
}

float GameConfig::GetActionExecuteDelay() const {
    return GetConfigValueFloat("combat", "action_execute_delay", 1.0f);
}

float GameConfig::GetTurnEndDelay() const {
    return GetConfigValueFloat("combat", "turn_end_delay", 1.5f);
}

float GameConfig::GetBattleStartDelay() const {
    return GetConfigValueFloat("combat", "battle_start_delay", 2.0f);
}

float GameConfig::GetBattleEndDelay() const {
    return GetConfigValueFloat("combat", "battle_end_delay", 3.0f);
}

// Combat rewards
int GameConfig::GetBaseExperience() const {
    return GetConfigValueInt("combat", "base_experience", 50);
}

int GameConfig::GetBaseGold() const {
    return GetConfigValueInt("combat", "base_gold", 25);
}

int GameConfig::GetExperiencePerEnemyLevel() const {
    return GetConfigValueInt("combat", "experience_per_enemy_level", 10);
}

int GameConfig::GetGoldPerEnemyLevel() const {
    return GetConfigValueInt("combat", "gold_per_enemy_level", 5);
}

// Combat balance
float GameConfig::GetPlayerHealthMultiplier() const {
    return GetConfigValueFloat("combat", "player_health_multiplier", 1.0f);
}

float GameConfig::GetEnemyHealthMultiplier() const {
    return GetConfigValueFloat("combat", "enemy_health_multiplier", 1.0f);
}

float GameConfig::GetDamageVariance() const {
    return GetConfigValueFloat("combat", "damage_variance", 0.2f);
}

float GameConfig::GetAccuracyBase() const {
    return GetConfigValueFloat("combat", "accuracy_base", 85.0f);
}

// Combat audio
float GameConfig::GetCombatMusicVolume() const {
    return GetConfigValueFloat("combat", "combat_music_volume", 0.7f);
}

float GameConfig::GetAttackSoundVolume() const {
    return GetConfigValueFloat("combat", "attack_sound_volume", 0.8f);
}

float GameConfig::GetMagicSoundVolume() const {
    return GetConfigValueFloat("combat", "magic_sound_volume", 0.9f);
}

float GameConfig::GetVictorySoundVolume() const {
    return GetConfigValueFloat("combat", "victory_sound_volume", 0.6f);
}

float GameConfig::GetDefeatSoundVolume() const {
    return GetConfigValueFloat("combat", "defeat_sound_volume", 0.5f);
}

// Helper method for colors
Color GameConfig::GetColorFromConfig(const ConfigManager& config, const std::string& section, 
                                   const std::string& prefix, const Color& defaultColor) const {
    int r = config.Get(section, prefix + "_r", defaultColor.r).AsInt();
    int g = config.Get(section, prefix + "_g", defaultColor.g).AsInt();
    int b = config.Get(section, prefix + "_b", defaultColor.b).AsInt();
    int a = config.Get(section, prefix + "_a", defaultColor.a).AsInt();
    return Color(r, g, b, a);
}

// Color getters
Color GameConfig::GetGroundColor() const {
    return GetColorFromConfig(*m_gameplayConfig, "colors", "ground", Color(100, 150, 50, 255));
}

Color GameConfig::GetGroundDetailColor() const {
    return GetColorFromConfig(*m_gameplayConfig, "colors", "ground_detail", Color(80, 120, 30, 255));
}

Color GameConfig::GetPlayerBodyColor() const {
    return GetColorFromConfig(*m_gameplayConfig, "colors", "player_body", Color(100, 150, 255, 255));
}

Color GameConfig::GetPlayerHeadColor() const {
    return GetColorFromConfig(*m_gameplayConfig, "colors", "player_head", Color(255, 220, 180, 255));
}

Color GameConfig::GetEnemyRedColor() const {
    return GetColorFromConfig(*m_gameplayConfig, "colors", "enemy_red", Color(255, 0, 0, 255));
}

Color GameConfig::GetEnemyOrangeColor() const {
    return GetColorFromConfig(*m_gameplayConfig, "colors", "enemy_orange", Color(255, 100, 0, 255));
}

Color GameConfig::GetEnemyPurpleColor() const {
    return GetColorFromConfig(*m_gameplayConfig, "colors", "enemy_purple", Color(200, 0, 100, 255));
}

Color GameConfig::GetHudBackgroundColor() const {
    return GetColorFromConfig(*m_gameplayConfig, "colors", "hud_background", Color(0, 0, 0, 200));
}

Color GameConfig::GetHudBorderColor() const {
    return GetColorFromConfig(*m_gameplayConfig, "colors", "hud_border", Color(255, 255, 0, 255));
}

Color GameConfig::GetTextNormalColor() const {
    return GetColorFromConfig(*m_gameplayConfig, "colors", "text_normal", Color(255, 255, 255, 255));
}

Color GameConfig::GetTextWarningColor() const {
    return GetColorFromConfig(*m_gameplayConfig, "colors", "text_warning", Color(255, 100, 100, 255));
}

Color GameConfig::GetTextInstructionsColor() const {
    return GetColorFromConfig(*m_gameplayConfig, "colors", "text_instructions", Color(180, 180, 180, 255));
}

Color GameConfig::GetTextUrgentColor() const {
    return GetColorFromConfig(*m_gameplayConfig, "colors", "text_urgent", Color(255, 0, 0, 255));
}

// HUD settings
int GameConfig::GetHudHeight() const {
    return m_gameplayConfig->Get("visual", "hud_height", 60).AsInt();
}

int GameConfig::GetHudBorderHeight() const {
    return m_gameplayConfig->Get("visual", "hud_border_height", 2).AsInt();
}

int GameConfig::GetScoreX() const {
    return m_gameplayConfig->Get("visual", "score_x", 10).AsInt();
}

int GameConfig::GetScoreY() const {
    return m_gameplayConfig->Get("visual", "score_y", 15).AsInt();
}

int GameConfig::GetScoreScale() const {
    return m_gameplayConfig->Get("visual", "score_scale", 2).AsInt();
}

int GameConfig::GetTimerX() const {
    return m_gameplayConfig->Get("visual", "timer_x", 500).AsInt();
}

int GameConfig::GetTimerY() const {
    return m_gameplayConfig->Get("visual", "timer_y", 15).AsInt();
}

int GameConfig::GetTimerScale() const {
    return m_gameplayConfig->Get("visual", "timer_scale", 2).AsInt();
}

int GameConfig::GetInstructionsX() const {
    return m_gameplayConfig->Get("visual", "instructions_x", 10).AsInt();
}

int GameConfig::GetInstructionsY() const {
    return m_gameplayConfig->Get("visual", "instructions_y", 35).AsInt();
}

int GameConfig::GetInstructionsScale() const {
    return m_gameplayConfig->Get("visual", "instructions_scale", 1).AsInt();
}

int GameConfig::GetWarningY() const {
    return m_gameplayConfig->Get("visual", "warning_y", 100).AsInt();
}

int GameConfig::GetWarningScale() const {
    return m_gameplayConfig->Get("visual", "warning_scale", 3).AsInt();
}

// Player fallback rendering
int GameConfig::GetPlayerBodyWidth() const {
    return m_gameplayConfig->Get("visual", "player_body_width", 16).AsInt();
}

int GameConfig::GetPlayerBodyHeight() const {
    return m_gameplayConfig->Get("visual", "player_body_height", 24).AsInt();
}

int GameConfig::GetPlayerBodyOffsetX() const {
    return m_gameplayConfig->Get("visual", "player_body_offset_x", 8).AsInt();
}

int GameConfig::GetPlayerBodyOffsetY() const {
    return m_gameplayConfig->Get("visual", "player_body_offset_y", 16).AsInt();
}

int GameConfig::GetPlayerHeadWidth() const {
    return m_gameplayConfig->Get("visual", "player_head_width", 12).AsInt();
}

int GameConfig::GetPlayerHeadHeight() const {
    return m_gameplayConfig->Get("visual", "player_head_height", 12).AsInt();
}

int GameConfig::GetPlayerHeadOffsetX() const {
    return m_gameplayConfig->Get("visual", "player_head_offset_x", 10).AsInt();
}

int GameConfig::GetPlayerHeadOffsetY() const {
    return m_gameplayConfig->Get("visual", "player_head_offset_y", 4).AsInt();
}

// Background settings
int GameConfig::GetSkyHeight() const {
    return m_gameplayConfig->Get("background", "sky_height", 500).AsInt();
}

int GameConfig::GetSkyGradientStep() const {
    return m_gameplayConfig->Get("background", "sky_gradient_step", 4).AsInt();
}

int GameConfig::GetSkyBaseIntensity() const {
    return m_gameplayConfig->Get("background", "sky_base_intensity", 50).AsInt();
}

int GameConfig::GetSkyIntensityRange() const {
    return m_gameplayConfig->Get("background", "sky_intensity_range", 100).AsInt();
}

float GameConfig::GetMountainParallaxSpeed() const {
    return m_gameplayConfig->Get("background", "mountain_parallax_speed", 0.2f).AsFloat();
}

int GameConfig::GetMountainRepeatWidth() const {
    return m_gameplayConfig->Get("background", "mountain_repeat_width", 1600).AsInt();
}

int GameConfig::GetMountainSpacing() const {
    return m_gameplayConfig->Get("background", "mountain_spacing", 200).AsInt();
}

int GameConfig::GetMountainCountPerGroup() const {
    return m_gameplayConfig->Get("background", "mountain_count_per_group", 5).AsInt();
}

int GameConfig::GetMountainWidth() const {
    return m_gameplayConfig->Get("background", "mountain_width", 60).AsInt();
}

int GameConfig::GetMountainBaseHeight() const {
    return m_gameplayConfig->Get("background", "mountain_base_height", 100).AsInt();
}

int GameConfig::GetMountainHeightVariation() const {
    return m_gameplayConfig->Get("background", "mountain_height_variation", 30).AsInt();
}

int GameConfig::GetMountainSpacingInGroup() const {
    return m_gameplayConfig->Get("background", "mountain_spacing_in_group", 40).AsInt();
}

Color GameConfig::GetMountainColor() const {
    return GetColorFromConfig(*m_gameplayConfig, "background", "mountain", Color(60, 80, 120, 255));
}

float GameConfig::GetBuildingParallaxSpeed() const {
    return m_gameplayConfig->Get("background", "building_parallax_speed", 0.5f).AsFloat();
}

int GameConfig::GetBuildingRepeatWidth() const {
    return m_gameplayConfig->Get("background", "building_repeat_width", 1200).AsInt();
}

int GameConfig::GetBuildingSpacing() const {
    return m_gameplayConfig->Get("background", "building_spacing", 150).AsInt();
}

int GameConfig::GetBuildingCountPerGroup() const {
    return m_gameplayConfig->Get("background", "building_count_per_group", 4).AsInt();
}

int GameConfig::GetBuildingWidth() const {
    return m_gameplayConfig->Get("background", "building_width", 30).AsInt();
}

int GameConfig::GetBuildingBaseHeight() const {
    return m_gameplayConfig->Get("background", "building_base_height", 80).AsInt();
}

int GameConfig::GetBuildingHeightVariation() const {
    return m_gameplayConfig->Get("background", "building_height_variation", 20).AsInt();
}

int GameConfig::GetBuildingSpacingInGroup() const {
    return m_gameplayConfig->Get("background", "building_spacing_in_group", 35).AsInt();
}

Color GameConfig::GetBuildingColor() const {
    return GetColorFromConfig(*m_gameplayConfig, "background", "building", Color(40, 60, 80, 255));
}

int GameConfig::GetWindowCountHorizontal() const {
    return m_gameplayConfig->Get("background", "window_count_horizontal", 3).AsInt();
}

int GameConfig::GetWindowWidth() const {
    return m_gameplayConfig->Get("background", "window_width", 4).AsInt();
}

int GameConfig::GetWindowHeight() const {
    return m_gameplayConfig->Get("background", "window_height", 6).AsInt();
}

int GameConfig::GetWindowSpacingX() const {
    return m_gameplayConfig->Get("background", "window_spacing_x", 8).AsInt();
}

int GameConfig::GetWindowSpacingY() const {
    return m_gameplayConfig->Get("background", "window_spacing_y", 15).AsInt();
}

int GameConfig::GetWindowOffsetX() const {
    return m_gameplayConfig->Get("background", "window_offset_x", 5).AsInt();
}

Color GameConfig::GetWindowColor() const {
    return GetColorFromConfig(*m_gameplayConfig, "background", "window", Color(255, 255, 150, 255));
}

// Hot-reloading implementation
bool GameConfig::CheckAndReloadIfModified() {
    bool reloaded = false;

    // Check gameplay config
    time_t currentGameplayModTime = GetFileModificationTime(m_gameplayConfigPath);
    if (currentGameplayModTime > m_lastGameplayModTime) {
        std::cout << "🔄 Detected changes in " << m_gameplayConfigPath << ", reloading..." << std::endl;
        if (m_gameplayConfig->LoadFromFile(m_gameplayConfigPath)) {
            m_lastGameplayModTime = currentGameplayModTime;
            std::cout << "✅ Successfully reloaded gameplay config!" << std::endl;
            reloaded = true;
        } else {
            std::cerr << "❌ Failed to reload gameplay config!" << std::endl;
        }
    }

    // Check character config
    time_t currentCharacterModTime = GetFileModificationTime(m_characterConfigPath);
    if (currentCharacterModTime > m_lastCharacterModTime) {
        std::cout << "🔄 Detected changes in " << m_characterConfigPath << ", reloading..." << std::endl;
        if (m_characterConfig->LoadFromFile(m_characterConfigPath)) {
            m_lastCharacterModTime = currentCharacterModTime;
            std::cout << "✅ Successfully reloaded character config!" << std::endl;
            reloaded = true;
        } else {
            std::cerr << "❌ Failed to reload character config!" << std::endl;
        }
    }

    return reloaded;
}

time_t GameConfig::GetLastModificationTime() const {
    return std::max(m_lastGameplayModTime, m_lastCharacterModTime);
}

time_t GameConfig::GetFileModificationTime(const std::string& filepath) const {
    struct stat fileInfo;
    if (stat(filepath.c_str(), &fileInfo) == 0) {
        return fileInfo.st_mtime;
    }
    return 0; // File doesn't exist or error
}

// Level-specific config implementation
bool GameConfig::LoadLevelConfig(const std::string& levelName) {
    m_levelConfigPath = "assets/config/levels/" + levelName + ".ini";

    if (m_levelConfig->LoadFromFile(m_levelConfigPath)) {
        m_currentLevel = levelName;
        m_lastLevelModTime = GetFileModificationTime(m_levelConfigPath);
        std::cout << "🎮 Loaded level config: " << levelName << " from " << m_levelConfigPath << std::endl;
        return true;
    } else {
        std::cerr << "⚠️  Failed to load level config: " << m_levelConfigPath << ", using base config" << std::endl;
        ClearLevelOverrides();
        return false;
    }
}

void GameConfig::ClearLevelOverrides() {
    m_currentLevel = "";
    m_levelConfigPath = "";
    m_lastLevelModTime = 0;
    // Clear the level config manager
    m_levelConfig = std::make_unique<ConfigManager>();
    std::cout << "🔄 Cleared level overrides, using base config" << std::endl;
}

const std::string& GameConfig::GetCurrentLevel() const {
    return m_currentLevel;
}
