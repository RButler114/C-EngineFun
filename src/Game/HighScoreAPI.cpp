#include "Game/HighScoreAPI.h"
#include "Game/HighScoreManager.h"

namespace HighScoreAPI {
    bool HasAnyScores() {
        return HighScoreManager::HasScores();
    }
}

