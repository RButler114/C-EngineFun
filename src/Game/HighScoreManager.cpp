#include "Game/HighScoreManager.h"
#include <ctime>

#include "Engine/ConfigSystem.h"
#include <algorithm>
#include <iostream>


bool HighScoreManager::LoadEntries(std::vector<Entry>& outEntries) {
    outEntries.clear();
    ConfigManager cfg;
    if (!cfg.LoadFromFile(GetFilePath())) {
        return false; // no file yet
    }
    for (int i = 1; i <= 10; ++i) {
        std::string scoreKey = "score" + std::to_string(i);
        if (cfg.Has("scores", scoreKey)) {
            int s = cfg.Get("scores", scoreKey, 0).AsInt();
            std::string nameKey = "name" + std::to_string(i);
            std::string n = cfg.Has("scores", nameKey)
                ? cfg.Get("scores", nameKey, std::string("YOU")).AsString()
                : std::string("YOU");
            std::string dateKey = "date" + std::to_string(i);
            std::string d = cfg.Has("scores", dateKey)
                ? cfg.Get("scores", dateKey, std::string("")).AsString()
                : std::string("");
            outEntries.push_back(Entry{n, s, d});
        }
    }
    std::sort(outEntries.begin(), outEntries.end(), [](const Entry& a, const Entry& b){
        return a.score > b.score;
    });
    if (outEntries.size() > 10) outEntries.resize(10);
    return !outEntries.empty();
}

bool HighScoreManager::SaveEntries(const std::vector<Entry>& entries) {
    ConfigManager cfg;
    int count = std::min<int>(10, entries.size());
    for (int i = 1; i <= count; ++i) {
        cfg.Set("scores", "score" + std::to_string(i), ConfigValue(entries[i-1].score));
        cfg.Set("scores", "name" + std::to_string(i), ConfigValue(entries[i-1].name));
        cfg.Set("scores", "date" + std::to_string(i), ConfigValue(entries[i-1].date));
    }
    return cfg.SaveToFile(GetFilePath());
}


bool HighScoreManager::SubmitEntry(const Entry& entry) {
    std::vector<Entry> entries;
    LoadEntries(entries); // ok if empty
    entries.push_back(entry);
    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b){ return a.score > b.score; });
    if (entries.size() > 10) entries.resize(10);

    // Ensure date present for the newest entry before saving
    auto now = std::time(nullptr);
    char buf[32];
    std::tm* tm = std::localtime(&now);
    if (tm) {
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", tm);
    } else {
        std::snprintf(buf, sizeof(buf), "%ld", static_cast<long>(now));
    }
    if (!entries.empty() && entries.front().date.empty()) entries.front().date = buf;

    bool ok = SaveEntries(entries);
    if (!ok) std::cerr << "Failed to save high score entries" << std::endl;
    return ok;
}

bool HighScoreManager::HasScores() {
    std::vector<Entry> entries;
    return LoadEntries(entries) && !entries.empty();
}

bool HighScoreManager::WouldQualify(int score) {
    std::vector<Entry> entries;
    LoadEntries(entries);
    if (entries.size() < 10) return score > 0; // any positive score qualifies until table filled
    return score > entries.back().score;
}
