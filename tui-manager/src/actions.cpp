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
        system("g++ ./src/logic-simulation.cpp -o build/linux/logic-simulation");
        return "\nCompiled Auto Room (Logic Simulation)";
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

    else {
      return "\nERROR: Unknown Project!";
    }
  }
  else {
    return "\nERROR: Unrecognised OS!\nPlease note that only linux, that too only arch is supported for now";
  }
  return "\nERROR: An unknown error occured!";
}

