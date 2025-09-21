#include <string>
#include "include/actions.hpp"
using namespace std;

string executeAction(std::string os, std::string project, std::string action) {
  if (os == "LINUX"){
    if (project == "Jarvis") {
      if (action == "Run") {
        system("python3 src/python/jarvis.py");
        return "\nRunning Jarvis";
      }
      else {
        return "\nERROR: Unknown action for project Jarvis";
      }
    }
    else if (project == "Auto Room (Logic Simulation)") {
      if (action == "Compile") {
        system("g++ ./src/logic-simulation.cpp -o build/linux/logic-simulation");
        return "\nCompiling Auto Room (Logic Simulation)";
      }
      else if (action == "Run") {
        return "\nRunning Auto Room (Logic Simulation)";
      }
      else {
        return "\nERROR: Unknown action for project Auto Room (Logic Simulation)";
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

