#ifndef MANAGER_HPP
#define MANAGER_HPP

#include <string>

// Displays the main project menu and returns the name of the chosen project
std::string showMenu();

// Another menu based on first selection, e.g., compile and run precompiled
std::string showProjectMenu(std::string chosenProject);

#endif

