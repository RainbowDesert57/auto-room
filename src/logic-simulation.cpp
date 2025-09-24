#include <iostream>
using namespace std;

int peopleCount = 0;
int blockCount = 0;

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
            else if (peopleCount == 0) {
              cout << "There are no people in the room\n\n";
            }
            else if (peopleCount < 0) {
              cout << "PLEASE RECALIBRATE!!\n\n";
              cout << "There are " << peopleCount << " people in the room\n" << "Resetting to 0\n\n";
              peopleCount = 0;
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
    
    //Temporaty code till we dont have raspberry pi
    
    while (true) {

      cout << "Is laser 1 blocked? (y/n): ";
      cin >> input;
      if (input == 'y') {
        aLaserBlocked = true;
      }
      else if (input == 'n') {
        aLaserBlocked = false;
      }

      cout << "Is laser 2 blocked? (y/n): ";
      cin >> input;
      if (input == 'y') {
        bLaserBlocked = true;
      }
      else if (input == 'n') {
        bLaserBlocked = false;
      }

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


   // Practical concept to call lightControl() once any change in laser state is detected
   // take two bools
   // bool prevA = false;
   // bool prevB = false;
   // and other two bools
   // 
   // bool aLaserBlocked;
   // bool bLaserBlocked;
   //
   // once any laser is blocked (a or b) update the bool to false or true accordingly
   // now,
   //
   // if (aLaserBlocked != prevA) {
   //   lightControl(aLaserBlocked, bLaserBlocked);
   //   prevA=aLaserBlocked;
   //   prevB=bLaserBlocked;
   // }
   // else if (bLaserBlocked != prevB) {
   //   lightControl(aLaserBlocked, bLaserBlocked);
   //   prevB=bLaserBlocked;
   //   prevA=aLaserBlocked;
   // }
   // this will call lightControl on any update!!
   return 0;
}
