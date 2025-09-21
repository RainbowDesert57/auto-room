#include <ncurses.h>
#include <string>
#include "include/os_detect.hpp"
#include "include/manager.hpp"
#include "include/actions.hpp"
#include <iostream>
using namespace std;
int main() {
    // Initialize ncurses
    initscr();          // Start curses mode
    cbreak();           // Disable line buffering
    noecho();           // Don't echo input
    keypad(stdscr, TRUE); // Enable arrow keys

    // Get terminal size
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // Detect OS
    string os = detectOS();

    // Display a title
    string title = "=== TUI Manager ===";
    mvprintw(1, (cols - title.size()) / 2, "%s", title.c_str());

    // Display detected OS
    string os_info = "Detected OS: " + os;
    mvprintw(3, 2, "%s", os_info.c_str());

    // Display menu options
    string chosenProject = showMenu();
    string action = showProjectMenu(chosenProject);
    
    // Cleanup ncurses
    endwin();

    // Execute the option
    string executeMessage = executeAction (os, chosenProject, action);
    cout << "\nYou chose to " << action << " " << chosenProject;
    cout << executeMessage;
    return 0;
}

