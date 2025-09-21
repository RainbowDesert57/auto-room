#include "include/os_detect.hpp"

std::string detectOS() {
  #ifdef _WIN32
    return "WINDOWS";
  #elif __APPLE__
    return "MAC";
  #elif __linux__
    return "LINUX";
  #else
    return "UNKNOWN";
  #endif
}

