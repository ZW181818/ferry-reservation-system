//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//***************************************************
// Module: ferryASM.cpp
// Version History:
//   - Version 1.0 - 2025/07/09 (Yanhong Li)
//     > Initial creation of Ferry persistent entity interface.
//   - Version 1.1 - 2025/07/16 (Vino Jeong)
//     > Added ferry class attributes and edited function signatures.
//
// Provides access to binary storage of ferry records including capacity.
// Used by SailingManager when validating new sailings.
//***************************************************

#include "ferryASM.h"
#include "sailingASM.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#define FILE_PATH "ferries.dat"
#define PAGE_LENGTH 5

// fstream FerryASM::file;
// FerryASM ferryManager;

std::fstream FerryASM::file;

void FerryASM::initialize() {
    file.open(FILE_PATH, ios::in | ios::out | ios::binary);
    if (!file.is_open()) {
        file.open(FILE_PATH, ios::out | ios::binary);
        file.close();
        file.open(FILE_PATH, ios::in | ios::out | ios::binary);
    }
}

void FerryASM::shutdown() {
    if (file.is_open()) {
        file.close();
    }
}

void FerryASM::reset() {
    if (file.is_open()) file.close();

    ofstream resetFile(FILE_PATH, ios::out | ios::trunc | ios::binary);
    
    if (resetFile.is_open()) resetFile.close();
    else {
        cerr << "Could not reset the Ferry file." << endl;
        return;
    }

    FerryASM::initialize(); 

}


bool FerryASM::writeFerry(const char* ferryName, const int HCLL, const int LCLL) {
    if (!file.is_open()) {
        cout << "File is not open for writing in FerryASM::writeFerry().\n" << endl;
        return false;
    }
    Ferry newFerry;

    memset(newFerry.ferryName, 0, sizeof(newFerry.ferryName));
    strncpy(newFerry.ferryName, ferryName, sizeof(newFerry.ferryName) - 1);

    newFerry.HCLL = HCLL;
    newFerry.LCLL = LCLL;
    
    file.clear();
    file.seekp(0, ios::end); 
    file.write(reinterpret_cast<const char*>(&newFerry), sizeof(newFerry));
    file.flush();
    // cout << "tellp(): " << file.tellp() << endl;
    if (!file.good()) {
        cout << "File write failed in FerryASM::writeFerry()." << endl;
        return false;
    }
    return true;
}

bool FerryASM::deleteFerry(char* ferryName) {
    // check SailingASM for ferry being assigned
    SailingASM sailingASM;
    
    vector<char*> matches = sailingASM.findSailingsWithFerry(ferryName);
    
    if(!matches.empty()) {

        cout << "\n[WARNING] The ferry is in the following sailing(s):\n" << endl;

        for (char* sailingID : matches) {
            cout << sailingID << endl;
        }

        cin.ignore(128, '\n');
        cout << "\nThe ferry cannot be deleted while it is assigned to a sailing. Press enter to continue." << endl;
        cin.get();

        return false;

    }

    file.clear();
    file.seekg(0, ios::beg);

    std::fstream newFile("temp.dat", ios::out | ios::binary);
    if (!newFile) {
        std::cerr << "Failed to create temp file.\n";
        return false;
    }

    Ferry ferry;
    bool found = false;

    // copy over all records except the one to delete
    while (file.read(reinterpret_cast<char*>(&ferry), sizeof(Ferry))) {
        if (strncmp(ferry.ferryName, ferryName, sizeof(ferry.ferryName)) != 0) {
            newFile.write(reinterpret_cast<char*>(&ferry), sizeof(Ferry));
        } else {
            found = true;
        }
    }

    file.close();
    newFile.close();

    if (!found) {
        // couldn't retrieve ferry to delete
        std::cerr << "Ferry not found: " << ferryName << "\n";
        std::remove("temp.dat");
        file.open(FILE_PATH, ios::in | ios::out | ios::binary); 
        return false;
    }

    std::remove(FILE_PATH);              // Delete original
    std::rename("temp.dat", FILE_PATH); 

    file.open(FILE_PATH, ios::in | ios::out | ios::binary);

    return true;
}


bool FerryASM::ferryExists(const char* ferryName) {
    file.clear();
    file.seekg(0, ios::beg);

    Ferry ferry;
    while (file.read(reinterpret_cast<char*>(&ferry), sizeof(Ferry))) {
        if (strncmp(ferry.ferryName, ferryName, sizeof(ferry.ferryName)) == 0) {
            return true;
        }
    }
    return false; // could not find ferry
}

bool FerryASM::showFerriesAndSelect(Ferry* selectedFerry, bool* quitMenu) {
    
    ifstream inFile(FILE_PATH, ios::binary);
    if (!inFile) {
        std::cerr << "Failed to open ferry file.\n";
    }

    // total ferry count
    inFile.seekg(0, ios::end);
    int totalFerries = inFile.tellg() / sizeof(Ferry);
    if (totalFerries == 0) {
        cout << "\nNo ferries available to show.\n" << endl;
        return false;
    }
    inFile.seekg(0, ios::beg);

    int currentPage = 0;
    char command;

    while (true) {
        int start = currentPage * PAGE_LENGTH;
        int end = min(start + PAGE_LENGTH, totalFerries);

        inFile.seekg(start * sizeof(Ferry), ios::beg);

        cout << "\n" << endl;
        cout << "===================== Available Ferries =====================\n" << endl;
        cout << setfill(' ');
        for (int i = start; i < end; ++i) {
            Ferry ferry;
            inFile.read(reinterpret_cast<char*>(&ferry), sizeof(Ferry));

            cout << right << setw(3) << (i - start + 1) << " ";
            cout << left << setw(28) << ferry.ferryName;

            cout << "HCLL: " << setw(4) << right << (int)ferry.HCLL << " m\t";
            cout << "LCLL: " << setw(4) << right << (int)ferry.LCLL << " m" << endl;
        }


        cout << "\n[Results " << (start + 1) << " to " << end << " of " << totalFerries << "]" << endl;
        cout << "\nSelect a ferry [1 ~ " << (end - start) << "], or type 'n' (next), 'p' (prev), or 'q' (quit): ";
        cin >> command;

        if (command == 'n') {
            if ((currentPage + 1) * PAGE_LENGTH < totalFerries)
                currentPage++;
        } else if (command == 'p') {
            if (currentPage > 0)
                currentPage--;
        } else if (command == 'q') {
            *quitMenu = true;
            return false; // maybe change this
        } else if (isdigit(command)) {
            int selection = command - '0';
            if (selection >= 1 && selection <= (end - start)) {
                // Go to selected record
                int index = start + selection - 1;
                inFile.seekg(index * sizeof(Ferry), ios::beg);
                Ferry selected;
                inFile.read(reinterpret_cast<char*>(&selected), sizeof(Ferry));
                // strncpy(ferryName, selected.ferryName, size);
                *selectedFerry = selected;
                return true;
            } else {
                cout << "Invalid selection." << endl;
            }
        } else {
            cout << "Invalid input.";
        }
    }
}