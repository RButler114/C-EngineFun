/**
 * @file GameConfig.h
 * @brief Game-specific configuration wrapper for easy access to gameplay settings
 * @author Ryan Butler
 * @date 2025
 */

#pragma once

#include "Engine/ConfigSystem.h"
#include "Engine/Renderer.h"  // For Color struct
#include <string>
#include <memory>

/**
 * @class GameConfig
 * @brief Wrapper around ConfigManager providing typed access to game settings
 * 
 * This class provides a convenient interface for accessing game configuration
 * values with proper types and sensible defaults. It loads multiple config files
 * and provides easy access methods for common game parameters.
 */
class GameConfig {
public:
    GameConfig();
    ~GameConfig() = default;

    /**
     * @brief Load all game configuration files
     * @return true if all configs loaded successfully
     */
    bool LoadConfigs();

    /**
     * @brief Reload configuration files (for hot-reloading)
     * @return true if reload was successful
     */
    bool ReloadConfigs();

    /**
     * @brief Check if config files have been modified and reload if necessary
     * @return true if files were reloaded
     */
    bool CheckAndReloadIfModified();

    /**
     * @brief Get the last modification time of config files
     */
    time_t GetLastModificationTime() const;

    /**
     * @brief Load level-specific config overrides
     * @param levelName Name of the level (e.g., "level1", "level2", "boss")
     * @return true if level config was loaded successfully
     */
    bool LoadLevelConfig(const std::string& levelName);

    /**
     * @brief Clear level-specific overrides and return to base config
     */
    void ClearLevelOverrides();

    /**
     * @brief Get the currently loaded level name
     */
    const std::string& GetCurrentLevel() const;

    // Player settings
    float GetPlayerStartX() const;
    float GetPlayerStartY() const;
    float GetPlayerMovementSpeed() const;
    float GetPlayerMovementThreshold() const;
    float GetPlayerSkyLimit() const;
    float GetPlayerGroundLimit() const;
    float GetPlayerCameraLeftBoundary() const;

    // Camera settings
    float GetCameraTargetOffsetX() const;
    float GetCameraFollowSpeed() const;
    float GetCameraMinX() const;

    // Game rules
    float GetGameDurationSeconds() const;
    float GetLowTimeWarningThreshold() const;
    int GetBaseScorePerFrame() const;
    int GetMovementBonusScore() const;
    float GetDebugOutputInterval() const;
    int GetDebugFrameInterval() const;

    // Enemy settings
    int GetEnemyCount() const;
    float GetEnemySpawnStartX() const;
    float GetEnemySpawnSpacingX() const;
    float GetEnemySpawnHeightVariation() const;
    float GetEnemyBaseVelocityX() const;
    float GetEnemyVelocityVariation() const;
    float GetEnemyVerticalMovementRange() const;
    float GetEnemyRespawnDistance() const;
    float GetEnemyRespawnOffset() const;
    int GetEnemyRespawnRandomRange() const;
    int GetEnemyHeightRandomRange() const;
    int GetEnemyWidth() const;
    int GetEnemyHeight() const;

    // Animation settings
    float GetAnimationFrameDuration() const;
    int GetAnimationTotalFrames() const;
    int GetAnimationSpriteWidth() const;
    int GetAnimationSpriteHeight() const;
    float GetAnimationSpriteScale() const;
    float GetApproximateFrameTime() const;

    // Audio settings
    float GetBackgroundMusicVolume() const;
    float GetJumpSoundVolume() const;
    float GetCollisionSoundVolume() const;

    // Visual settings
    int GetScreenWidth() const;
    int GetScreenHeight() const;
    int GetGroundY() const;
    int GetGroundHeight() const;
    int GetGroundDetailCount() const;
    int GetGroundDetailWidth() const;
    int GetGroundDetailHeight() const;
    int GetGroundDetailSpacing() const;
    int GetGroundDetailOffset() const;

    // HUD settings
    int GetHudHeight() const;
    int GetHudBorderHeight() const;
    int GetScoreX() const;
    int GetScoreY() const;
    int GetScoreScale() const;
    int GetTimerX() const;
    int GetTimerY() const;
    int GetTimerScale() const;
    int GetInstructionsX() const;
    int GetInstructionsY() const;
    int GetInstructionsScale() const;
    int GetWarningY() const;
    int GetWarningScale() const;

    // Player fallback rendering
    int GetPlayerBodyWidth() const;
    int GetPlayerBodyHeight() const;
    int GetPlayerBodyOffsetX() const;
    int GetPlayerBodyOffsetY() const;
    int GetPlayerHeadWidth() const;
    int GetPlayerHeadHeight() const;
    int GetPlayerHeadOffsetX() const;
    int GetPlayerHeadOffsetY() const;

    // Colors
    Color GetGroundColor() const;
    Color GetGroundDetailColor() const;
    Color GetPlayerBodyColor() const;
    Color GetPlayerHeadColor() const;
    Color GetEnemyRedColor() const;
    Color GetEnemyOrangeColor() const;
    Color GetEnemyPurpleColor() const;
    Color GetHudBackgroundColor() const;
    Color GetHudBorderColor() const;
    Color GetTextNormalColor() const;
    Color GetTextWarningColor() const;
    Color GetTextInstructionsColor() const;
    Color GetTextUrgentColor() const;

    // Background settings
    int GetSkyHeight() const;
    int GetSkyGradientStep() const;
    int GetSkyBaseIntensity() const;
    int GetSkyIntensityRange() const;
    float GetMountainParallaxSpeed() const;
    int GetMountainRepeatWidth() const;
    int GetMountainSpacing() const;
    int GetMountainCountPerGroup() const;
    int GetMountainWidth() const;
    int GetMountainBaseHeight() const;
    int GetMountainHeightVariation() const;
    int GetMountainSpacingInGroup() const;
    Color GetMountainColor() const;
    float GetBuildingParallaxSpeed() const;
    int GetBuildingRepeatWidth() const;
    int GetBuildingSpacing() const;
    int GetBuildingCountPerGroup() const;
    int GetBuildingWidth() const;
    int GetBuildingBaseHeight() const;
    int GetBuildingHeightVariation() const;
    int GetBuildingSpacingInGroup() const;
    Color GetBuildingColor() const;
    int GetWindowCountHorizontal() const;
    int GetWindowWidth() const;
    int GetWindowHeight() const;
    int GetWindowSpacingX() const;
    int GetWindowSpacingY() const;
    int GetWindowOffsetX() const;
    Color GetWindowColor() const;

private:
    std::unique_ptr<ConfigManager> m_gameplayConfig;
    std::unique_ptr<ConfigManager> m_characterConfig;
    std::unique_ptr<ConfigManager> m_levelConfig;  // Level-specific overrides

    // Hot-reloading support
    mutable time_t m_lastGameplayModTime;
    mutable time_t m_lastCharacterModTime;
    mutable time_t m_lastLevelModTime;
    std::string m_gameplayConfigPath;
    std::string m_characterConfigPath;
    std::string m_levelConfigPath;
    std::string m_currentLevel;

    /**
     * @brief Helper to create Color from config RGB values
     */
    Color GetColorFromConfig(const ConfigManager& config, const std::string& section,
                           const std::string& prefix, const Color& defaultColor = Color(255, 255, 255, 255)) const;

    /**
     * @brief Get file modification time
     */
    time_t GetFileModificationTime(const std::string& filepath) const;

    /**
     * @brief Get config value with level override support
     * Checks level config first, then gameplay config, then returns default
     */
    template<typename T>
    T GetConfigValue(const std::string& section, const std::string& key, const T& defaultValue) const;

    // Template specializations
    float GetConfigValueFloat(const std::string& section, const std::string& key, float defaultValue) const {
        if (!m_currentLevel.empty() && m_levelConfig->Has(section, key)) {
            return m_levelConfig->Get(section, key, defaultValue).AsFloat();
        }
        return m_gameplayConfig->Get(section, key, defaultValue).AsFloat();
    }

    int GetConfigValueInt(const std::string& section, const std::string& key, int defaultValue) const {
        if (!m_currentLevel.empty() && m_levelConfig->Has(section, key)) {
            return m_levelConfig->Get(section, key, defaultValue).AsInt();
        }
        return m_gameplayConfig->Get(section, key, defaultValue).AsInt();
    }

    bool GetConfigValueBool(const std::string& section, const std::string& key, bool defaultValue) const {
        if (!m_currentLevel.empty() && m_levelConfig->Has(section, key)) {
            return m_levelConfig->Get(section, key, defaultValue).AsBool();
        }
        return m_gameplayConfig->Get(section, key, defaultValue).AsBool();
    }
};
