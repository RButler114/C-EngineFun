#include "include/Game/GameConfig.h"
#include <iostream>

int main() {
    std::cout << "=== Config Test ===" << std::endl;

    GameConfig config;
    if (!config.LoadConfigs()) {
        std::cerr << "Failed to load configs!" << std::endl;
        return 1;
    }

    std::cout << "Config loaded successfully!" << std::endl;
    std::cout << "Sprite Width: " << config.GetAnimationSpriteWidth() << std::endl;
    std::cout << "Sprite Height: " << config.GetAnimationSpriteHeight() << std::endl;
    std::cout << "Total Frames: " << config.GetAnimationTotalFrames() << std::endl;
    std::cout << "Frame Duration: " << config.GetAnimationFrameDuration() << std::endl;
    std::cout << "Player Speed: " << config.GetPlayerMovementSpeed() << std::endl;
    std::cout << "Enemy Count: " << config.GetEnemyCount() << std::endl;

    return 0;
}
