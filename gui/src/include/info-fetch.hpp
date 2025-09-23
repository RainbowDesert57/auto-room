#pragma once

struct StatType {
    int power = 0;       // instantaneous power (W)
    double current = 0;     // instantaneous current (A)
    int voltage = 0;     // instantaneous voltage (V)
    double energy = 0;   // accumulated energy (Wh)
};

