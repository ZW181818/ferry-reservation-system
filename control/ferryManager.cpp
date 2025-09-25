//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//***************************************************
// File: ferryManager.cpp
// Version History:
//   - Version 1.0 - 2025/07/16 (Vino Jeong)
//     > Logic for creating a ferry
//     > Logic for deleting a ferry
//   - Version 2.0 - 2025/07/28 (Vino Jeong)
//     > Integration of showFerriesAndSelect function
//   - Version 3.0 - 2025/08/05 (Wenbo Zhang)
//     > Logic for transfer upper case input
//
// This module handles user-facing ferry vessel creation and deletion logic.
// It validates input and interacts with FerryASM to persist ferry data.
//***************************************************
//***************************************************

#include <iostream>
#include <fstream>
#include <cctype> 

#include "ferryManager.h"
#include "../entity/ferryASM.h"
#define MAX_FERRY_NAME_LENGTH 25
#define MAX_HIGH_CAPACITY 3600
#define MAX_LOW_CAPACITY 3600

using namespace std;

void toUpperCase(char* str) {
    for (int i = 0; str[i]; ++i) {
        str[i] = toupper(str[i]);
    }
}

void createFerry() {
    char ferryName[MAX_FERRY_NAME_LENGTH + 1];
    int HCLL = -1, LCLL = -1, option = 0;

    //============================
    // Step 1: Enter and Validate Ferry Name
    //============================
    while (true) {
        cout << "\nEnter Ferry Name (max " << MAX_FERRY_NAME_LENGTH << " characters): ";
        cin.getline(ferryName, MAX_FERRY_NAME_LENGTH);

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "[Error] The name you entered is too long.\n";
        } else if (strlen(ferryName) == 0) {
            cout << "[Error] The name cannot be empty. Please try again.\n";
        } else {
            toUpperCase(ferryName);
            if (FerryASM::ferryExists(ferryName)) {
                cout << "[Error] A ferry named \"" << ferryName << "\" already exists. Please choose a different name.\n";
            } else {
                break;
            }
        }
    }

    //============================
    // Step 2: Enter HCLL
    //============================
    while (true) {
        cout << "\nEnter High Ceiling Lane Capacity (0 ~ " << MAX_HIGH_CAPACITY << "): ";
        if (!(cin >> HCLL) || HCLL < 0 || HCLL > MAX_HIGH_CAPACITY) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "[Error] Please enter a valid integer between 0 and " << MAX_HIGH_CAPACITY << ".\n";
        } else {
            break;
        }
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    //============================
    // Step 3: Enter LCLL
    //============================
    while (true) {
        cout << "\nEnter Low Ceiling Lane Capacity (0 ~ " << MAX_LOW_CAPACITY << "): ";
        if (!(cin >> LCLL) || LCLL < 0 || LCLL > MAX_LOW_CAPACITY) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "[Error] Please enter a valid integer between 0 and " << MAX_LOW_CAPACITY << ".\n";
        } else {
            break;
        }
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    //============================
    // Step 4: Validate Capacity Combination
    //============================
    if (HCLL == 0 && LCLL == 0) {
        cout << "[Error] At least one of High or Low Ceiling Lane Capacity must be greater than 0.\n";
        cout << "Press enter to continue." << endl;
        return;
    }

    //============================
    // Step 5: Confirm Creation
    //============================
    while (option != 1 && option != 2) {
        cout << "[1] Confirm\t[2] Cancel" << endl;
        cout << "> Select [1 or 2]: ";
        if (!(cin >> option)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            option = 0;
        }
    }

    if (option == 1) {
        if (FerryASM::writeFerry(ferryName, HCLL, LCLL)) {
            cout << "\n--------------------------------------------------" << endl;
            cout << "Ferry Name:\t\t\t" << ferryName << endl;
            cout << "High Ceiling Lane Length:\t" << HCLL << " m" << endl;
            cout << "Low Ceiling Lane Length:\t" << LCLL << " m" << endl;
            cout << "--------------------------------------------------\n" << endl;
            cout << "Ferry record created successfully.\n" << endl;
        } else {
            cout << "[Error] Failed to write ferry record to disk.\n";
        }
    } else {
        cout << "\nFerry creation cancelled.\n";
    }
}


bool deleteFerry() {
    Ferry ferryToDelete;
    bool quitMenu = false;
    bool proceed = true;

    proceed = FerryASM::showFerriesAndSelect(&ferryToDelete, &quitMenu);
    if (quitMenu) return false;

    if (proceed) {
        if (FerryASM::deleteFerry(ferryToDelete.ferryName)) {
            cout << "\nFerry [" << ferryToDelete.ferryName << "] are deleted.\n" << endl;
            return true;
        }
    } else {
        cout << "Press enter to continue." << endl;
        cin.get();
    }
    
    return false;
}