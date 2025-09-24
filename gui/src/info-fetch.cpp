#include <iostream>
#include <thread>
#include <chrono>
#include <ctime>
#include "include/liveStatFetch.hpp"
#include "include/dataManager.hpp"
using namespace std;


int main () {
    StatType live{}, avg{}, liveTotal{}, dayTotal{};
    bool savedToday = false;
    // load totals and refresh count from storage
    liveTotal.power   = getSavedLiveTotalPower();
    liveTotal.current = getSavedLiveTotalCurrent();
    liveTotal.energy  = getSavedLiveTotalEnergy();
    int refreshCount  = getSavedRefreshCount();

  while (true) {
      live.voltage  = 220; // fixed
      live.current  = getLiveCurrent();
      live.power    = live.current * live.voltage;
  
      // calculate energy increment for this second (Wh)
      double energyIncrement = live.power / 3600.0;
  
      // live.energy = just this tick’s energy
      live.energy = energyIncrement;
  
      // update refresh count
      refreshCount++;
  
      // update totals (only energy accumulates)
      liveTotal.energy  += energyIncrement;
      liveTotal.voltage  = 220;
      liveTotal.power = live.power;
      liveTotal.current = live.current;


      // save totals
      saveLiveTotalEnergy(liveTotal.energy);
      saveLiveTotalCurrent(live.current);
      saveLiveTotalPower(live.power);
      saveRefreshCount(refreshCount);
  
      // calculate averages based on elapsed time
      double elapsed_hours = refreshCount / 3600.0;
      avg.power   = (elapsed_hours > 0) ? (liveTotal.energy / elapsed_hours) : 0;
      avg.current = avg.power / live.voltage;
      avg.energy  = liveTotal.energy / refreshCount;
      avg.voltage = live.voltage;
      saveAvgCurrent(avg.current);
      saveAvgPower(avg.power);
      saveAvgEnergy(avg.energy);
  
      std::time_t now = std::time(nullptr);
      std::tm* localTime = std::localtime(&now);
  
      int hour   = localTime->tm_hour;   // 0-23
      int minute = localTime->tm_min;    // 0-59
  
      // Save daily total at 23:55
      if (hour == 23 && minute == 55) {
        if (!savedToday) {
            saveTodayTotal(liveTotal);  // save today's total once
            liveTotal.energy = 0;       // reset after saving
            liveTotal.current = 0;
            liveTotal.power   = 0;
            refreshCount = 0;
            savedToday = true;
        }
      } else {
        savedToday = false;
      }

      // ---- output ----
      cout << "\n\n\n";
      cout << "avg: " << avg.current*1000 << "mA, " << avg.voltage
           << "V, " << avg.energy <<"Wh, " << avg.power << "W\n";
      cout << "live: " << live.current*1000 << "mA, " << live.voltage
           << "V, " << live.energy << "Wh, " << live.power << "W\n";
      cout << "liveTotal: " << liveTotal.energy << "Wh (accumulated)\n";
      cout << "refreshCount: " << refreshCount << "\n";
  
      // wait 1 second
      this_thread::sleep_for(std::chrono::seconds(1));
  }

return 0;
}

