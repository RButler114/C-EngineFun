#pragma once

#include <vector>
#include <string>

// Simple persistent high score table stored in assets/config/highscores.ini
class HighScoreManager {
public:
    struct Entry {
        std::string name; // 1-3 initials (or up to a few chars)
        int score{0};
        std::string date; // formatted date/time string
    };

    // Submit a score with initials; keeps top 10 descending
    static bool SubmitEntry(const Entry& entry);

    // Backward-compat helper (kept to avoid breaking existing includes)
    static bool SubmitScore(int score) { return SubmitEntry({"YOU", score, ""}); }

    // Load entries descending (highest first)
    static bool LoadEntries(std::vector<Entry>& outEntries);

    // Save entries (assumed descending, truncated to 10)
    static bool SaveEntries(const std::vector<Entry>& entries);

    // Returns true if there is at least one stored score
    static bool HasScores();

    // Determine if a score would qualify for top 10 (without inserting)
    static bool WouldQualify(int score);

    // Config file path
    static const char* GetFilePath() { return "assets/config/highscores.ini"; }
};

