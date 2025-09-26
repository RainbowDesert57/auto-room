#include <iostream>
#include <bcm2835.h>
using namespace std;

// GPIO pins (change if wired differently)
#define LDR_A RPI_V2_GPIO_P1_11  // physical pin 11
#define LDR_B RPI_V2_GPIO_P1_13  // physical pin 13

int peopleCount = 0;

// FSM states
enum State {
    IDLE,        // nothing blocked
    A_BLOCKED,   // A was blocked, waiting for B
    B_BLOCKED    // B was blocked, waiting for A
};

State currentState = IDLE;

// --- Update FSM ---
void updateState(bool aLaserBlocked, bool bLaserBlocked) {
    switch (currentState) {
        case IDLE:
            if (aLaserBlocked && !bLaserBlocked) {
                currentState = A_BLOCKED;
                cout << "A triggered → waiting for B...\n";
            }
            else if (bLaserBlocked && !aLaserBlocked) {
                currentState = B_BLOCKED;
                cout << "B triggered → waiting for A...\n";
            }
            break;

        case A_BLOCKED:
            if (bLaserBlocked) {
                // Sequence A → B → Entry
                peopleCount++;
                cout << "✅ Person ENTERED. Count = " << peopleCount << "\n\n";
                currentState = IDLE;
            }
            else if (!aLaserBlocked) {
                // False trigger (no entry)
                currentState = IDLE;
            }
            break;

        case B_BLOCKED:
            if (aLaserBlocked) {
                // Sequence B → A → Exit
                peopleCount--;
                if (peopleCount < 0) {
                    cout << "⚠️  Negative count detected, resetting to 0!\n";
                    peopleCount = 0;
                }
                cout << "🚪 Person EXITED. Count = " << peopleCount << "\n\n";
                currentState = IDLE;
            }
            else if (!bLaserBlocked) {
                // False trigger
                currentState = IDLE;
            }
            break;
    }
}

int main() {
    if (!bcm2835_init()) {
        cerr << "Failed to initialize bcm2835.\n";
        return 1;
    }

    // Configure GPIO as input
    bcm2835_gpio_fsel(LDR_A, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(LDR_B, BCM2835_GPIO_FSEL_INPT);

    cout << "People counter started. Waiting for laser triggers...\n\n";

    while (true) {
        bool aLaserBlocked = (bcm2835_gpio_lev(LDR_A) == LOW); // LDR LOW when blocked
        bool bLaserBlocked = (bcm2835_gpio_lev(LDR_B) == LOW);

        updateState(aLaserBlocked, bLaserBlocked);

        bcm2835_delay(50); // ~20Hz sampling rate (tweak if needed)
    }

    bcm2835_close();
    return 0;
}

