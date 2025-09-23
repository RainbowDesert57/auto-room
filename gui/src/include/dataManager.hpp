#pragma once
#include "info-fetch.hpp" // if StatType is defined here

// Load today's totals
int getSavedLiveTotalPower();
int getSavedLiveTotalCurrent();
double getSavedLiveTotalEnergy();
int getSavedRefreshCount();

// Save today's totals
void saveLiveTotalPower(int);
void saveLiveTotalCurrent(int);
void saveLiveTotalEnergy(double);
void saveRefreshCount(int);

// Save end-of-day totals to 30-day JSON
void saveTodayTotal(const StatType& liveTotal);

// Get sum of last 30 days for monthly averages
int get30Power();
int get30Current();
double get30Energy();

