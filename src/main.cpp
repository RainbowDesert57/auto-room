#include <iostream>
#include <bcm2835.h>
using namespace std;

int peopleCount = 0;
int blockCount = 0;

// Function to read a channel (0–7) from MCP3008
uint16_t readADC(uint8_t channel) {
    char buf[3];
    buf[0] = 1;                    // start bit
    buf[1] = (8 + channel) << 4;   // single-ended mode + channel number
    buf[2] = 0;                     // dummy byte

    bcm2835_spi_transfern(buf, 3);  // send request and receive result

    // Combine the 10-bit result from buf[1] and buf[2]
    return ((buf[1] & 3) << 8) | (buf[2] & 0xFF);
}

void lightControl(bool &aLaserBlocked, bool &bLaserBlocked) {

          cout << "\nDebug output\n";
            if (aLaserBlocked) {
              cout << "Laser 1 is blocked\n\n";
            }
            else if (!aLaserBlocked) {
              cout << "Laser 1 is not blocked\n\n";
            }
            
            if (bLaserBlocked) {
              cout << "Laser 2 is blocked\n\n";
            }
            else if (!bLaserBlocked) {
              cout << "Laser 2 is not blocked\n\n";
           }


            if (aLaserBlocked) {
              blockCount++;
              cout << "\nblockCount++ triggered\n";
              cout << "\nblockCount = " << blockCount << "\n\n";
            }
            
            if (bLaserBlocked) {
              blockCount--;
              cout<< "\nblockCount-- triggered\n";
              cout << "\nblockCount = " << blockCount <<"\n\n";
            }

            if (blockCount == 0 && bLaserBlocked && !aLaserBlocked) {
              cout << "Person just entered!\n";
              peopleCount++;
            }
            else if (blockCount == 0 && !bLaserBlocked && aLaserBlocked) {
              cout << "Person just exitted!\n";
              peopleCount--;
            }

            if (peopleCount == 1){
              cout << "There is 1 person in the room\n\n"; //This is done for better grammar in the output :D
            }
            else if (peopleCount <= 0) {
              cout << "There are no people in the room\n\n";
              peopleCount = 0; //Just in case it goes negative...
            }
            else {
              cout<<"There are " <<peopleCount <<" people in the room\n\n";
            }

}
int main() {
    bool aLaserBlocked;
    bool bLaserBlocked;
    static bool prevA = false;
    static bool prevB = false;
    char input;
    
    if (!bcm2835_init()) return 1;
    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536);

    const uint16_t threshold = 500; // Adjust after testing the LDRs

    
    while (true) {
      // Read LDR values from MCP3008 channels
      uint16_t aValue = readADC(0); // Channel 0 -> Laser A
      uint16_t bValue = readADC(1); // Channel 1 -> Laser B

      aLaserBlocked = (aValue < threshold);
      bLaserBlocked = (bValue < threshold);

      if (aLaserBlocked != prevA) {
        lightControl(aLaserBlocked, bLaserBlocked);
        prevA=aLaserBlocked;
        prevB=bLaserBlocked;
      }
      if (bLaserBlocked != prevB) {
        lightControl(aLaserBlocked, bLaserBlocked);
        prevB=bLaserBlocked;
        prevA=aLaserBlocked;
      }

    }
return 0;
}
