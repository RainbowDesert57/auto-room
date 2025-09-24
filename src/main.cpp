#include <iostream>
#include <bcm2835.h>
#include <unistd.h>   // for usleep
using namespace std;

int peopleCount = 0;
int blockCount = 0;

// Read ADC value from MCP3008 (channel 0–7)
uint16_t readADC(uint8_t channel) {
    if (channel > 7) return 0;

    char buf[3];
    buf[0] = 1;                           // start bit
    buf[1] = (8 + channel) << 4;          // single-ended + channel
    buf[2] = 0;

    bcm2835_spi_transfern(buf, 3);        // full-duplex SPI transfer

    return ((buf[1] & 3) << 8) | (buf[2] & 0xFF); // combine 10-bit result
}

void lightControl(bool aLaserBlocked, bool bLaserBlocked) {
    cout << "\n--- Debug Output ---\n";
    cout << "Laser A: " << (aLaserBlocked ? "BLOCKED" : "CLEAR") << "\n";
    cout << "Laser B: " << (bLaserBlocked ? "BLOCKED" : "CLEAR") << "\n";

    if (aLaserBlocked) {
        blockCount++;
        cout << "blockCount++ -> " << blockCount << "\n";
    }

    if (bLaserBlocked) {
        blockCount--;
        cout << "blockCount-- -> " << blockCount << "\n";
    }

    // Entry detection
    if (blockCount == 0 && bLaserBlocked && !aLaserBlocked) {
        cout << "Person just ENTERED!\n";
        peopleCount++;
    }
    // Exit detection
    else if (blockCount == 0 && !bLaserBlocked && aLaserBlocked) {
        cout << "Person just EXITED!\n";
        peopleCount--;
    }

    // Clamp people count
    if (peopleCount < 0) peopleCount = 0;

    // Status
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
    if (!bcm2835_spi_begin()) {
        cerr << "Failed to begin SPI.\n";
        return 1;
    }

    // SPI setup for MCP3008
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64); // ~4 MHz

    const uint16_t threshold = 500; // Tune this after testing LDRs
    const uint16_t hysteresis = 50; // Adds stability against noise

    bool prevA = false;
    bool prevB = false;

    while (true) {
        // Read LDR values
        uint16_t aValue = readADC(0);
        uint16_t bValue = readADC(1);

        // Apply hysteresis (prevents rapid flicker)
        bool aLaserBlocked = (aValue < threshold - hysteresis);
        bool bLaserBlocked = (bValue < threshold - hysteresis);

        // Only trigger when state changes
        if (aLaserBlocked != prevA || bLaserBlocked != prevB) {
            lightControl(aLaserBlocked, bLaserBlocked);
            prevA = aLaserBlocked;
            prevB = bLaserBlocked;
        }

        usleep(50000); // 50 ms delay -> ~20 Hz loop
    }

    bcm2835_spi_end();
    bcm2835_close();
    return 0;
}

