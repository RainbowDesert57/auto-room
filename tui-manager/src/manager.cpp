#include <ncurses.h>
#include <string>
#include <vector>
using namespace std;

vector<string> projects = {"Manager", "Auto Room (Logic Simulation)", "Jarvis", "Auto-Room", "TTS", "STT", "Electricity Manager", "Auto-Room GUI"};
string showMenu() {
    int highlight = 0;                // Current highlighted item
    int n_projects = projects.size();
    
    while (true) {
        clear();
        mvprintw(0, 0, "Select a project (Use arrow keys and Enter):");
        
        // Draw menu
        for (int i = 0; i < n_projects; ++i) {
            if (i == highlight) attron(A_REVERSE);
            mvprintw(i + 1, 2, projects[i].c_str());
            if (i == highlight) attroff(A_REVERSE);
        }

        int c = getch();
        switch (c) {
            case KEY_UP:
            case 'k':
                highlight = (highlight - 1 + n_projects) % n_projects;
                break;
            case KEY_DOWN:
            case 'j':
                highlight = (highlight + 1) % n_projects;
                break;
            case 10: // Enter key
                return projects[highlight];
        }
    }
}

string showProjectMenu(string chosenProject) {
    vector<string> actions = {"Compile", "Run"};
    if (chosenProject == "Jarvis" || chosenProject == "TTS" || chosenProject == "STT" || chosenProject == "Auto-Room GUI") {
      return "Run";
    }
    else {
      int highlight = 0;
      int n_actions = actions.size();
  
      while (true) {
          clear();
          mvprintw(0, 0, ("Project: " + chosenProject).c_str());
          mvprintw(1, 0, "Select an action (Use arrow keys and Enter):");
  
          for (int i = 0; i < n_actions; ++i) {
              if (i == highlight) attron(A_REVERSE);
              mvprintw(i + 2, 2, actions[i].c_str());
              if (i == highlight) attroff(A_REVERSE);
          }
  
          int c = getch();
          switch (c) {
              case KEY_UP:
              case 'k':
                  highlight = (highlight - 1 + n_actions) % n_actions;
                  break;
              case KEY_DOWN:
              case 'j':
                  highlight = (highlight + 1) % n_actions;
                  break;
              case 10: // Enter key
                  return actions[highlight];
          }
      }
    }
}

