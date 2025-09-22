#include <string>
#include "include/actions.hpp"
using namespace std;

string executeAction(std::string os, std::string project, std::string action) {
  if (os == "LINUX"){
    if (project == "Jarvis") {
      if (action == "Run") {
        system("python3 src/python/jarvis.py");
        system("./token-check");
        return "\nRan Jarvis";
      }
      else {
        return "\nERROR: Unknown action for project Jarvis";
      }
    }
    else if (project == "Auto Room (Logic Simulation)") {
      if (action == "Compile") {
        system("g++ ./src/logic-simulation.cpp -o build/linux/logic-simulation && x86_64-w64-mingw32-g++ -static-libgcc -static-libstdc++ -o ./build/windows/logic-simulation.exe ./src/logic-simulation.cpp");
        return "\nCompiled Auto Room (Logic Simulation) for both linux and windows";
      }
      else if (action == "Run") {
        system("./build/linux/logic-simulation");
        return "\nRan Auto Room (Logic Simulation)";
      }
      else {
        return "\nERROR: Unknown action for project Auto Room (Logic Simulation)";
      }
    }
    else if (project == "TTS") {
      if (action == "Run") {
        system("python3 src/python/tts.py");
        system("./token-check");
        return "\nRan TTS";
      }
      else {
        return "\nERROR: Unknown action for project TTS";
      }
    }
    else if (project == "STT") {
      if (action == "Run") {
        system("python3 src/python/stt.py");
        system("./token-check");
        return "\nRan STT";
      }
      else {
        return "\nERROR: Unknown action for project STT";
      }
    }
    else if (project == "Auto-Room") {
      if (action == "Compile") {
        system("g++ ./src/main.cpp -o ./build/linux/main -lbcm2835");
        return "\nCompiled Auto-Room\nPlease note that it uses <bcm2835.h> header which is hardware specific\nCan only be compiled on Raspberry PI OS";
      }
      else if (action == "Run") {
        system("./build/linux/main");
        return "\nRan Auto-Room";
      }
      else {
        return "\nERROR: Invalid option for project Auto-Room";
      }
    }
    else {
      return "\nERROR: Unknown Project!";
    }
  }
  else if (os == "WINDOWS") {
    if (project == "Auto Room (Logic Simulation)") {
      if (action == "Compile") {
        system("x86_64-w64-mingw32-g++ -static-libgcc -static-libstdc++ -o ./build/windows/logic-simulation.exe ./src/logic-simulation.cpp");
        return "\nCompiled Succesfully";
      }
      else if (action == "Run") {
        system("./build");
        return "\nRan succesfully";
      }
    }
  }
  else {
    return "\nERROR: Unrecognised OS!\nPlease note that only linux, that too only arch is supported for now";
  }
  return "\nERROR: An unknown error occured!";
}

