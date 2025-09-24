#include <iostream>
#include <bcm2835.h>
#include <unistd.h>   // for usleep

using namespace std;

// GPIO pins for LDR modules
#define LDR_A RPI_V2_GPIO_P1_11  // physical pin 11
#define LDR_B RPI_V2_GPIO_P1_13  // physical pin 13

int peopleCount = 0;
int blockCount = 0;

bool prevA = false;
bool prevB = false;

void lightControl(bool aLaserBlocked, bool bLaserBlocked) {
    cout << "\n--- Debug Output ---\n";
    cout << "Laser A: " << (aLaserBlocked ? "BLOCKED" : "CLEAR") << "\n";
    cout << "Laser B: " << (bLaserBlocked ? "BLOCKED" : "CLEAR") << "\n";

    if (aLaserBlocked) blockCount++;
    if (bLaserBlocked) blockCount--;

    // Detect entry
    if (blockCount == 0 && bLaserBlocked && !aLaserBlocked) {
        cout << "Person just ENTERED!\n";
        peopleCount++;
    }
    // Detect exit
    else if (blockCount == 0 && !bLaserBlocked && aLaserBlocked) {
        cout << "Person just EXITED!\n";
        peopleCount--;
        if (peopleCount < 0) peopleCount = 0;
    }

    // Print current count
    if (peopleCount == 1)
        cout << "There is 1 person in the room.\n";
    else
        cout << "There are " << peopleCount << " people in the room.\n";

    cout << "--------------------\n";
}

int main() {
    if (!bcm2835_init()) {
        cerr << "Failed to initialize bcm2835.\n";
        return 1;
    }

    // Set LDR pins as input
    bcm2835_gpio_fsel(LDR_A, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(LDR_B, BCM2835_GPIO_FSEL_INPT);

    while (true) {
        bool aLaserBlocked = (bcm2835_gpio_lev(LDR_A) == LOW); // LOW if blocked
        bool bLaserBlocked = (bcm2835_gpio_lev(LDR_B) == LOW);

        // Only trigger on state change
        if (aLaserBlocked != prevA || bLaserBlocked != prevB) {
            lightControl(aLaserBlocked, bLaserBlocked);
            prevA = aLaserBlocked;
            prevB = bLaserBlocked;
        }

        usleep(50000); // 50 ms loop (~20 Hz)
    }

    bcm2835_close();
    return 0;
}

