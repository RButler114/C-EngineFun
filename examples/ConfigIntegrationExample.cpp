/**
 * @file ConfigIntegrationExample.cpp
 * @brief Example showing how to integrate GameConfig with the existing character system
 * @author Ryan Butler
 * @date 2025
 */

#include "Game/GameConfig.h"
#include "Game/CharacterFactory.h"
#include "ECS/EntityManager.h"
#include <iostream>

/**
 * @brief Example demonstrating GameConfig integration with CharacterFactory
 * 
 * This example shows how to:
 * 1. Use GameConfig for level-specific character modifications
 * 2. Combine base character templates with level overrides
 * 3. Create dynamic character variations based on current level
 */
void DemonstrateConfigCharacterIntegration() {
    std::cout << "=== GameConfig + CharacterFactory Integration Example ===" << std::endl;
    
    // Initialize systems
    GameConfig gameConfig;
    CharacterFactory characterFactory;
    EntityManager entityManager;
    
    // Load base configurations
    gameConfig.LoadConfigs();
    characterFactory.LoadFromConfig("assets/config/characters.ini");
    
    std::cout << "\n1. Base Character Creation:" << std::endl;
    
    // Create a base goblin enemy
    Entity baseGoblin = characterFactory.CreateCharacter(entityManager, "goblin", 100.0f, 200.0f);
    std::cout << "Created base goblin at (100, 200)" << std::endl;
    
    std::cout << "\n2. Level-Specific Character Modifications:" << std::endl;
    
    // Load Level 1 (Easy) - characters should be weaker
    gameConfig.LoadLevelConfig("level1");
    std::cout << "Loaded Level 1 config" << std::endl;
    
    // Create a level-modified goblin
    Entity level1Goblin = characterFactory.CreateCharacter(entityManager, "goblin", 300.0f, 200.0f);
    
    // Apply level-specific modifications using GameConfig
    if (auto* health = entityManager.GetComponent<HealthComponent>(level1Goblin)) {
        // Reduce health for easier level
        float healthMultiplier = 0.7f; // Could come from level config
        health->maxHealth *= healthMultiplier;
        health->currentHealth = health->maxHealth;
        std::cout << "Level 1 goblin health reduced to: " << health->maxHealth << std::endl;
    }
    
    if (auto* velocity = entityManager.GetComponent<VelocityComponent>(level1Goblin)) {
        // Use level-specific enemy speed
        float levelSpeed = gameConfig.GetEnemyBaseVelocityX();
        velocity->vx = levelSpeed;
        std::cout << "Level 1 goblin speed set to: " << levelSpeed << std::endl;
    }
    
    // Load Level 3 (Hard) - characters should be stronger
    gameConfig.LoadLevelConfig("level3");
    std::cout << "\nLoaded Level 3 config" << std::endl;
    
    Entity level3Goblin = characterFactory.CreateCharacter(entityManager, "goblin", 500.0f, 200.0f);
    
    // Apply hard level modifications
    if (auto* health = entityManager.GetComponent<HealthComponent>(level3Goblin)) {
        // Increase health for harder level
        float healthMultiplier = 1.5f; // Could come from level config
        health->maxHealth *= healthMultiplier;
        health->currentHealth = health->maxHealth;
        std::cout << "Level 3 goblin health increased to: " << health->maxHealth << std::endl;
    }
    
    if (auto* velocity = entityManager.GetComponent<VelocityComponent>(level3Goblin)) {
        // Use level-specific enemy speed (faster)
        float levelSpeed = gameConfig.GetEnemyBaseVelocityX();
        velocity->vx = levelSpeed;
        std::cout << "Level 3 goblin speed set to: " << levelSpeed << std::endl;
    }
    
    std::cout << "\n3. Boss Level Character Creation:" << std::endl;
    
    // Load Boss level
    gameConfig.LoadLevelConfig("boss");
    std::cout << "Loaded Boss level config" << std::endl;
    
    // Create boss character with special modifications
    Entity bossGoblin = characterFactory.CreateCharacter(entityManager, "goblin", 700.0f, 200.0f);
    
    if (auto* health = entityManager.GetComponent<HealthComponent>(bossGoblin)) {
        // Boss-level health boost
        health->maxHealth *= 3.0f;
        health->currentHealth = health->maxHealth;
        std::cout << "Boss goblin health set to: " << health->maxHealth << std::endl;
    }
    
    if (auto* render = entityManager.GetComponent<RenderComponent>(bossGoblin)) {
        // Make boss larger using config values
        render->width = gameConfig.GetEnemyWidth();
        render->height = gameConfig.GetEnemyHeight();
        // Boss-specific color (could be from config)
        render->r = 150; render->g = 0; render->b = 0; // Dark red
        std::cout << "Boss goblin size: " << render->width << "x" << render->height << std::endl;
    }
    
    std::cout << "\n4. Dynamic Character Scaling Based on Game Time:" << std::endl;
    
    // Example: Scale character difficulty based on game progression
    float gameTime = 15.0f; // Simulate 15 seconds into the game
    float maxGameTime = gameConfig.GetGameDurationSeconds();
    float difficultyScale = 1.0f + (gameTime / maxGameTime) * 0.5f; // Up to 50% harder
    
    Entity scaledGoblin = characterFactory.CreateCharacter(entityManager, "goblin", 900.0f, 200.0f);
    if (auto* health = entityManager.GetComponent<HealthComponent>(scaledGoblin)) {
        health->maxHealth *= difficultyScale;
        health->currentHealth = health->maxHealth;
        std::cout << "Time-scaled goblin health: " << health->maxHealth 
                  << " (scale: " << difficultyScale << ")" << std::endl;
    }
    
    std::cout << "\n5. Config-Driven Character Variants:" << std::endl;
    
    // Create different character variants based on config
    for (int i = 0; i < gameConfig.GetEnemyCount(); i++) {
        float x = gameConfig.GetEnemySpawnStartX() + i * gameConfig.GetEnemySpawnSpacingX();
        float y = gameConfig.GetPlayerStartY();
        
        Entity variant = characterFactory.CreateCharacter(entityManager, "goblin", x, y);
        
        // Apply variant-specific modifications
        if (auto* velocity = entityManager.GetComponent<VelocityComponent>(variant)) {
            float baseSpeed = gameConfig.GetEnemyBaseVelocityX();
            float variation = gameConfig.GetEnemyVelocityVariation();
            velocity->vx = baseSpeed - (i % 3) * variation;
        }
        
        if (auto* render = entityManager.GetComponent<RenderComponent>(variant)) {
            // Different colors for variants (could be from config)
            switch (i % 3) {
                case 0: render->r = 255; render->g = 0; render->b = 0; break;     // Red
                case 1: render->r = 255; render->g = 100; render->b = 0; break;   // Orange  
                case 2: render->r = 200; render->g = 0; render->b = 100; break;   // Purple
            }
        }
        
        std::cout << "Created variant " << i << " at (" << x << ", " << y << ")" << std::endl;
    }
    
    std::cout << "\n=== Integration Complete ===" << std::endl;
    std::cout << "This example shows how GameConfig can enhance the CharacterFactory" << std::endl;
    std::cout << "by providing level-specific modifications and dynamic scaling." << std::endl;
}

/**
 * @brief Example of creating a config-aware character creation helper
 */
class ConfigAwareCharacterFactory {
private:
    CharacterFactory& m_characterFactory;
    GameConfig& m_gameConfig;
    
public:
    ConfigAwareCharacterFactory(CharacterFactory& factory, GameConfig& config)
        : m_characterFactory(factory), m_gameConfig(config) {}
    
    /**
     * @brief Create a character with level-appropriate modifications
     */
    Entity CreateLevelCharacter(EntityManager& entityManager, const std::string& characterType, 
                               float x, float y, float difficultyMultiplier = 1.0f) {
        
        Entity character = m_characterFactory.CreateCharacter(entityManager, characterType, x, y);
        
        // Apply level-specific modifications
        ApplyLevelModifications(entityManager, character, difficultyMultiplier);
        
        return character;
    }
    
private:
    void ApplyLevelModifications(EntityManager& entityManager, Entity character, float difficultyMultiplier) {
        // Health scaling
        if (auto* health = entityManager.GetComponent<HealthComponent>(character)) {
            health->maxHealth *= difficultyMultiplier;
            health->currentHealth = health->maxHealth;
        }
        
        // Speed from config
        if (auto* velocity = entityManager.GetComponent<VelocityComponent>(character)) {
            velocity->vx = m_gameConfig.GetEnemyBaseVelocityX() * difficultyMultiplier;
        }
        
        // Size from config
        if (auto* render = entityManager.GetComponent<RenderComponent>(character)) {
            render->width = m_gameConfig.GetEnemyWidth();
            render->height = m_gameConfig.GetEnemyHeight();
        }
        
        // Audio volume from config
        if (auto* audio = entityManager.GetComponent<AudioComponent>(character)) {
            audio->volume = m_gameConfig.GetCollisionSoundVolume();
        }
    }
};

int main() {
    DemonstrateConfigCharacterIntegration();
    return 0;
}
