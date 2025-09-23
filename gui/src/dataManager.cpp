#include "include/dataManager.hpp"
#include "include/liveStatFetch.hpp"
#include <fstream>
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// File paths
const std::string liveFile  = "liveTotal.json";
const std::string dailyFile = "dailyTotal.json";

// Helper: load JSON from file
json loadJson(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return json::object(); // empty object
    json j;
    file >> j;
    return j;
}

// Helper: save JSON to file
void saveJson(const std::string &filename, const json &j) {
    std::ofstream file(filename);
    file << j.dump(4);
}

// ---------------- Live totals -----------------
int getSavedLiveTotalPower() {
    json j = loadJson(liveFile);
    return j.value("power", 0);
}

int getSavedLiveTotalCurrent() {
    json j = loadJson(liveFile);
    return j.value("current", 0);
}

double getSavedLiveTotalEnergy() {
    json j = loadJson(liveFile);
    return j.value("energy", 0.0);
}

void saveLiveTotalPower(int power) {
    json j = loadJson(liveFile);
    j["power"] = power;
    saveJson(liveFile, j);
}

void saveLiveTotalCurrent(int current) {
    json j = loadJson(liveFile);
    j["current"] = current;
    saveJson(liveFile, j);
}

void saveLiveTotalEnergy(double energy) {
    json j = loadJson(liveFile);
    j["energy"] = energy;
    saveJson(liveFile, j);
}

// ---------------- Refresh count -------------
int getSavedRefreshCount() {
    json j = loadJson(liveFile);
    return j.value("refreshCount", 0);
}

void saveRefreshCount(int refreshCount) {
    json j = loadJson(liveFile);
    j["refreshCount"] = refreshCount;
    saveJson(liveFile, j);
}

// ---------------- Daily totals -----------------
void saveTodayTotal(const StatType &total) {
    json dailyArray = loadJson(dailyFile);
    if (!dailyArray.is_array()) dailyArray = json::array();

    // get current date
    time_t now = time(nullptr);
    tm *ltm = localtime(&now);
    char dateStr[11];
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", ltm);

    json todayEntry = {
        {"date", dateStr},
        {"power", total.power},
        {"current", total.current},
        {"energy", total.energy}
    };

    dailyArray.push_back(todayEntry);

    // Keep only last 30 days
    while (dailyArray.size() > 30) dailyArray.erase(dailyArray.begin());

    saveJson(dailyFile, dailyArray);

    // Reset live totals
    saveLiveTotalPower(0);
    saveLiveTotalCurrent(0);
    saveLiveTotalEnergy(0);
    saveRefreshCount(0);
}

StatType getDailyTotal(int dayIndex) {
    StatType total{};
    json dailyArray = loadJson(dailyFile);
    if (!dailyArray.is_array() || dailyArray.empty()) return total;

    if (dayIndex < 0 || dayIndex >= (int)dailyArray.size()) return total;

    json entry = dailyArray[dailyArray.size() - 1 - dayIndex];
    total.power   = entry.value("power", 0);
    total.current = entry.value("current", 0);
    total.energy  = entry.value("energy", 0.0);

    return total;
}

// ---------------- 30-day aggregates -----------
int get30Power() {
    json dailyArray = loadJson(dailyFile);
    int sum = 0;
    for (auto &entry : dailyArray) sum += entry.value("power", 0);
    return sum;
}

int get30Current() {
    json dailyArray = loadJson(dailyFile);
    int sum = 0;
    for (auto &entry : dailyArray) sum += entry.value("current", 0);
    return sum;
}

double get30Energy() {
    json dailyArray = loadJson(dailyFile);
    double sum = 0.0;
    for (auto &entry : dailyArray) sum += entry.value("energy", 0.0);
    return sum;
}

