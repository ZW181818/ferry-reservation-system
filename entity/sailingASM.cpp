//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//***************************************************
// SailingASM.cpp
// Version: 2.0
// Author: Wenbo Zhang
// Purpose: Header file for SailingASM class.
// Provides binary record-based file access for fixed-length SailingRecord.
//***************************************************

#include "sailingASM.h"
#include <iostream>
#include <cstdio>
#include <cstring>

using namespace std;

//-------------------------------------------------------------
// Initializes the binary file for sailing records
void SailingASM::initialize() {
    file.open(filename, ios::in | ios::out | ios::binary);
    if (!file.is_open()) {
        // file.clear();
        file.open(filename, ios::out | ios::binary);  // Create empty file
        file.close();
        file.open(filename, ios::in | ios::out | ios::binary);
    }

    if (!file.good()) {
        cerr << "SailingASM Error: Could not open file." << endl;
    }
}

void SailingASM::reset() {
    if (file.is_open()) file.close();

    ofstream resetFile(filename, ios::out | ios::trunc | ios::binary);
    
    if (resetFile.is_open()) resetFile.close();
    else {
        cerr << "Could not reset the Sailing file." << endl;
        return;
    }

    file.open(filename, ios::in | ios::out | ios::binary);

    // ensure file is open
    if (!file) {
        cerr << "SailingASM: File could not be reopened." << endl;
    }
}

//-------------------------------------------------------------
// Adds a new record to end of the file
void SailingASM::addRecord(const SailingRecord& record) {
    file.clear();
    // cout << "\nRecord Details in SailingASM::addRecord" << endl;
    // cout << "Ferry:\t" << record.ferryName << endl;
    // cout << "HRL:\t" << record.highLaneRestLength << endl;
    // cout << "LRL:\t" << record.lowLaneRestLength << endl;
    file.seekp(0, ios::end);
    file.clear();
    file.write(reinterpret_cast<const char*>(&record), sizeof(SailingRecord));
    file.flush();
    if (!file) {
        cerr << "[ERROR] Failed to write the record in addRecord()." << endl;
    } else {
        cout << "Sailing record written successfully." << endl;
    }
}

//-------------------------------------------------------------
// Retrieves a record by index (0-based)
// Returns true if read is successful
bool SailingASM::getRecord(int index, SailingRecord& outRecord) {
    int offset = index * static_cast<int>(sizeof(SailingRecord));
    file.seekg(offset, ios::beg);
    file.read(reinterpret_cast<char*>(&outRecord), sizeof(SailingRecord));
    return file.good();  // Explicit boolean result
}

//-------------------------------------------------------------
// Updates an existing record at given index
void SailingASM::updateRecord(int index, const SailingRecord& record) {
    int offset = index * static_cast<int>(sizeof(SailingRecord));
    file.seekp(offset, ios::beg);
    file.write(reinterpret_cast<const char*>(&record), sizeof(SailingRecord));
}

//-------------------------------------------------------------
// Deletes record at given index using overwrite-and-truncate strategy
void SailingASM::deleteRecord(int index) {
    int count = getRecordCount();
    if (index < 0 || index >= count) return;

    // If not last record, overwrite with last record
    if (index != count - 1) {
        SailingRecord last;
        getRecord(count - 1, last);
        updateRecord(index, last);
    }

    file.flush();
    file.close();
    truncateFile(count - 1);
    file.open(filename, ios::in | ios::out | ios::binary);
}

//-------------------------------------------------------------
// Returns number of records in the file
int SailingASM::getRecordCount() {
    file.clear();
    if (!file.is_open() || !file.good()) {
        cout << "[ERROR] Problem in getRecordCount()." << endl;
        return 0;
    }
    file.seekg(0, ios::end);
    // cout << "tellg(): " << file.tellg() << endl;
    return static_cast<int>(file.tellg() / sizeof(SailingRecord));
}

//-------------------------------------------------------------
// Flushes the file stream to disk
void SailingASM::flush() {
    file.flush();
}

//-------------------------------------------------------------
// Closes the file stream
void SailingASM::shutdown() {
    file.close();
}

//-------------------------------------------------------------
// Truncates file to hold only numRecords (used in delete)
void SailingASM::truncateFile(int numRecords) {
    fstream oldFile(filename, ios::in | ios::binary);
    fstream newFile("temp.dat", ios::out | ios::binary);

    SailingRecord temp;
    for (int i = 0; i < numRecords; ++i) {
        oldFile.read(reinterpret_cast<char*>(&temp), sizeof(SailingRecord));
        newFile.write(reinterpret_cast<const char*>(&temp), sizeof(SailingRecord));
    }

    oldFile.close();
    newFile.close();

    remove(filename);
    rename("temp.dat", filename);
}

//-------------------------------------------------------------
// Checks whether a given ferry name is used in any sailing
std::vector<char*> SailingASM::findSailingsWithFerry(char* ferryName) {
    vector<char*> results;

    ifstream sailings(filename, ios::binary);
    if (!sailings) {
        return results;
    }

    SailingRecord sailing;

    while (sailings.read(reinterpret_cast<char*>(&sailing), sizeof(SailingRecord))) {
        if (strncmp(sailing.ferryName, ferryName, sizeof(sailing.ferryName)) == 0) {
            char* sailingID = new char[strlen(sailing.date) + 1];
            strcpy(sailingID, sailing.date);
            results.push_back(sailingID);
        }
    }

    return results;
}
