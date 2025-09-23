#include <cstdlib>
#include <ctime>
using namespace std;

double getLiveCurrent() {
    srand(static_cast<unsigned>(time(nullptr))); // seed once

    int min = 1000;
    int max = 1500;

    int randomValue = min + rand() % (max - min + 1);
    randomValue = randomValue/1000;
    return randomValue;
}
