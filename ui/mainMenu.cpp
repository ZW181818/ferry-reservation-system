//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//***************************************************
// mainMenu.cpp
// Version 1.0 
// Author: Vino Jeong - 2025/07/16
// Version: 2.0
// Author: Wenbo Zhang
// Version: 2.1
// Author: Vino Jeong
// Purpose: Header for Main Menu display controller.
// Provides the interface for showing the main system menu
// and capturing user selection for scenario branching.
//***************************************************

#include <iostream>
#include <cstring>
#include <iomanip>
#include "../system/utilities.h"

#include "../control/ferryManager.h"
#include "../control/reservationManager.h"
#include "../control/sailingManager.h"

using namespace std;


//--------------------------------------
// Function: displayMainMenu
// Purpose : Displays the main user menu, validates input,
//           and delegates selected operations.
// in  : None
// out : bool - false if user selects "Exit"
//--------------------------------------

bool displayMainMenu(){

    static ReservationManager rm;
    static SailingManager sm;
    static bool initialized = false;

    if (!initialized) {
        rm.initializeAll();  // ReservationASM + VehicleASM
        sm.initialize();     // SailingASM
        initialized = true;
    }

    // total width of the menu
    const int width = 75;
    char title[10] = "Main Menu";
    bool showMenu = true;
    
    while (showMenu) {
        // int padding = (width - strlen(title)) / 2;
        int option = 0;
        
        cout << endl;
        cout << setfill('-');
        cout << setw(width) << "" << endl;
        cout << setfill(' ');
    
        cout << setw(width/2 + strlen(title)/2 + 1) << title << setw(width/2 - strlen(title)/2 - 1) << "" << endl;
    
        cout << setfill('-');
        cout << setw(width) << "" << endl;
        cout << endl;
        
        // options
        cout << "[1] Create New Reservation" << endl;
        cout << "[2] Check-in Vehicle" << endl;
        cout << "[3] Create / Delete Ferry Vessel" << endl;
        cout << "[4] Create / Delete Sailing" << endl;
        cout << "[5] Print Sailing Report" << endl;
        cout << "[6] Delete Confirmed Reservation" << endl;
        cout << "[7] Reset System" << endl;
        cout << "[8] Exit System" << endl;
    
        cout << setw(width) << "\n" << endl;
    
        cout << "> Select [1 - 8]: ";
    
        while (option < 1 || option > 9) {
            cin >> option;
            if (cin.fail() || option < 1 || option > 9) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid option. Please select a valid menu option [1 - 8]: ";
            }
        }
    
        switch (option) {
            case 1:
                rm.createFlow(sm);
                break;
            case 2:
                rm.checkInFlow();
                break;
            case 3:
                cout << "\n[1] Create Ferry\t[2] Delete Ferry\n" << endl;
                cout << "> Select [1 or 2]: ";
                while (option != 1 && option != 2) {
                    cin >> option;
                    if (cin.fail() || (option != 1 && option != 2)) {
                        cin.clear();
                        cout << "Invalid option. Please select [1] Create Ferry or [2] Delete Ferry:\n";
                    }
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
    
                if (option == 1) {
                    createFerry();
                    option = -1;
                    while (option != 1 && option != 2) {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "[1] Create another ferry\t[2] Back to menu" << endl;
                        cout << "> Select [1 or 2]: ";
                        if(!(cin >> option)) {
                            cin.clear();
                            continue;   
                        }
                        // cout << "option: " << option << endl;
                    }
                    if (option == 1) {
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        createFerry();
                        option = -1;
                    } else if (option == 2) break;

                } else if (option == 2) {
                    if (!deleteFerry()) {
                        cout << "Could not delete the ferry. Please try again." << endl;
                        break;
                    }
                    option = -1;
                    while (option != 1 && option != 2) {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "[1] Delete another ferry\t[2] Back to menu" << endl;
                        cout << "> Select [1 or 2]: ";
                        if(!(cin >> option)) {
                            cin.clear();
                            continue;
                        }
                        if (option == 1) {
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            deleteFerry();
                            option = -1;
                        } else if (option == 2) break;
                    }
                }  
    
                break;
            case 4: 
                cout << '\n';
                cout << "===== Create / Delete Sailing =====" << endl;
                cout << "\n[1] Create Sailing\t[2] Delete Sailing\n" << endl;
            
                cout << "> Select [1 or 2]: ";  // ✅ 添加这一行
            
                while (option != 1 && option != 2) {
                    cin >> option;
                    if (cin.fail() || (option != 1 && option != 2)) {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "Invalid option. Please select [1] Create Sailing or [2] Delete Sailing:\n";
                        cout << "> Select [1 or 2]: ";  // ✅ 再次提示
                    }
                }
            
                if (option == 1) {
                    sm.createSailingViaUI();
                } else if (option == 2) {
                    sm.deleteSailingViaUI();
                }
                break;  
            case 5:
                sm.printAllSailings();
                
                break;
            case 6:
                rm.deleteFlow(sm);
                
                break;
            case 7:
                reset();
                cout << "Resetting database." << endl;
                break;
                
                break;
            case 8:
                shutdown();            
                cout << "Program Exited. Goodbye!" << endl;
                showMenu = false;
                break;
        }

    }
    return false;
}
