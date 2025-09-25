//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//***************************************************
// Module: ReservationASM.cpp
// Version History:
//   - Version 1.0 - 2025/07/09 (Yanhong Li)
//     > Initial creation of Reservation persistent entity interface.
//   - Version 2.0 - 2025/07/14 (Yu-Chia Chang)
//     > Implement and Change Function name.
//     > Modify writeReservationRecord.
//   - Version 3.0 - 2025/07/17 (Yu-Chia Chang)
//     > Make sure usable when be called by Managers.
//     > Debug: runtime error - infinity loop
//   - Version 4.0 - 2025/07/24 (Wenbo Zhang)
//     > Modify to compile
//   - Version 5.0 - 2025/08/05 (Wenbo Zhang)
//     > Persist laneUsed ('H'/'L'); extend read/write APIs; keep other ops compatible
//
// Purpose:
//   Stores and retrieves reservation records from disk.
//   Used by ReservationManager and Check-in procedures.
//***************************************************

#include "reservationASM.h"
#include <fstream>
#include <iostream>
#include <cstring>

using namespace std;

//--------------------------------------
// Open or create reservation file
void ReservationASM::initialize() {
    file.open(filename, ios::in | ios::out | ios::binary);
    if (!file) {
        file.clear();
        file.open(filename, ios::out | ios::binary);
        file.close();
        file.open(filename, ios::in | ios::out | ios::binary);
    }
    if (!file.good()) {
        cerr << "ReservationASM Error: Could not open file." << endl;
    }
}

//--------------------------------------
// Close file
void ReservationASM::shutdown() {
    file.close();
}

// Reset file
void ReservationASM::reset() {
    if (file.is_open()) file.close();

    ofstream resetFile(filename, ios::out | ios::trunc | ios::binary);
    if (resetFile.is_open()) resetFile.close();
    else {
        cerr << "Could not reset the Reservation file." << endl;
        return;
    }

    file.open(filename, ios::in | ios::out | ios::binary);

    // make sure file is open
    if (!file) {
        cerr << "ReservationASM: File could not be reopened." << endl;
    }
}

//--------------------------------------
// Return total number of reservations
int ReservationASM::getRecordCount() {
    file.clear();
    file.seekg(0, ios::end);
    auto endPos = file.tellg();
    return (endPos > 0) ? static_cast<int>(endPos / sizeof(ReservationRecord)) : 0;
}

//--------------------------------------
// Return reservation by index
ReservationRecord ReservationASM::get(int index) {
    file.clear();
    ReservationRecord record{};
    file.seekg(index * sizeof(ReservationRecord), ios::beg);
    file.read(reinterpret_cast<char*>(&record), sizeof(record));
    return record;
}

//--------------------------------------
// Find first reservation matching license plate
int ReservationASM::findIndexByLicense(const char* plate) {
    file.clear();
    file.seekg(0, ios::beg);
    ReservationRecord record{};
    int idx = 0;
    while (file.read(reinterpret_cast<char*>(&record), sizeof(record))) {
        if (strcmp(record.licensePlate, plate) == 0) {
            return idx;
        }
        ++idx;
    }
    return -1;
}

//--------------------------------------
// Add new reservation to file
bool ReservationASM::writeReservationRecord(const char* licensePlate,
                                            const char* sailingID,
                                            bool isOnboard,
                                            char laneUsed) {
    // Normalize laneUsed
    if (laneUsed != 'H' && laneUsed != 'L') laneUsed = 'L';

    ReservationRecord record{};
    strncpy(record.licensePlate, licensePlate, sizeof(record.licensePlate) - 1);
    strncpy(record.sailingId,    sailingID,    sizeof(record.sailingId)    - 1);
    record.isOnboard = isOnboard;
    record.laneUsed  = laneUsed;

    file.clear();
    file.seekp(0, ios::end);
    file.write(reinterpret_cast<const char*>(&record), sizeof(record));
    file.flush();
    return file.good();
}

//--------------------------------------
// Read reservation and return sailingID + onboard status + laneUsed
bool ReservationASM::readReservationRecord(const char* licensePlate,
                                           char* sailingID,
                                           bool& isOnboard,
                                           char& laneUsed) {
    int idx = findIndexByLicense(licensePlate);
    if (idx < 0) return false;

    ReservationRecord rec = get(idx);
    strncpy(sailingID, rec.sailingId, sizeof(rec.sailingId));
    isOnboard = rec.isOnboard;
    laneUsed  = rec.laneUsed;
    return true;
}

//--------------------------------------
// Mark reservation as onboard by license
bool ReservationASM::checkInReservation(const char* licensePlate) {
    int idx = findIndexByLicense(licensePlate);
    if (idx < 0) return false;

    ReservationRecord record = get(idx);
    record.isOnboard = true;

    file.clear();
    file.seekp(idx * sizeof(record), ios::beg);
    file.write(reinterpret_cast<const char*>(&record), sizeof(record));
    file.flush();
    return file.good();
}

//--------------------------------------
// Delete reservation by license (overwrite with last)
bool ReservationASM::deleteReservationRecord(const char* licensePlate) {
    int count = getRecordCount();
    if (count == 0) return false;

    int target = findIndexByLicense(licensePlate);
    if (target < 0) return false;

    if (target != count - 1) {
        ReservationRecord last = get(count - 1);
        file.clear();
        file.seekp(target * sizeof(last), ios::beg);
        file.write(reinterpret_cast<const char*>(&last), sizeof(last));
    }

    file.flush();
    file.close();
    truncateFile(count - 1);
    file.open(filename, ios::in | ios::out | ios::binary);
    return true;
}

//--------------------------------------
// Remove trailing records by rewriting N records
void ReservationASM::truncateFile(int numRecords) {
    if (numRecords < 0) numRecords = 0;

    fstream oldF(filename, ios::in | ios::binary);
    fstream newF("temp.dat", ios::out | ios::binary);
    ReservationRecord temp{};

    for (int i = 0; i < numRecords && oldF.read(reinterpret_cast<char*>(&temp), sizeof(temp)); ++i) {
        newF.write(reinterpret_cast<const char*>(&temp), sizeof(temp));
    }

    oldF.close();
    newF.close();
    remove(filename);
    rename("temp.dat", filename);
}

//--------------------------------------
// Check if exact reservation exists
bool ReservationASM::existsReservation(const char* licensePlate, const char* sailingID) {
    file.clear();
    file.seekg(0, ios::beg);
    ReservationRecord record{};

    while (file.read(reinterpret_cast<char*>(&record), sizeof(record))) {
        if (strcmp(record.licensePlate, licensePlate) == 0 &&
            strcmp(record.sailingId,    sailingID)    == 0) {
            return true;
        }
    }
    return false;
}

//--------------------------------------
// Find all indexes with matching license
std::vector<int> ReservationASM::findAllIndexesByLicense(const char* plate) {
    file.clear();
    file.seekg(0, ios::beg);

    std::vector<int> indexes;
    ReservationRecord record{};
    int idx = 0;

    while (file.read(reinterpret_cast<char*>(&record), sizeof(record))) {
        if (strcmp(record.licensePlate, plate) == 0) {
            indexes.push_back(idx);
        }
        ++idx;
    }

    return indexes;
}

//--------------------------------------
// Mark reservation as onboard by index
bool ReservationASM::checkInReservationByIndex(int index) {
    int count = getRecordCount();
    if (index < 0 || index >= count) return false;

    ReservationRecord record = get(index);
    record.isOnboard = true;

    file.clear();
    file.seekp(index * sizeof(record), ios::beg);
    file.write(reinterpret_cast<const char*>(&record), sizeof(record));
    file.flush();
    return file.good();
}

//--------------------------------------
// Delete reservation by index
bool ReservationASM::deleteReservationByIndex(int target) {
    int count = getRecordCount();
    if (target < 0 || target >= count) return false;

    if (target != count - 1) {
        ReservationRecord last = get(count - 1);
        file.clear();
        file.seekp(target * sizeof(last), ios::beg);
        file.write(reinterpret_cast<const char*>(&last), sizeof(last));
    }

    file.flush();
    file.close();
    truncateFile(count - 1);
    file.open(filename, ios::in | ios::out | ios::binary);
    return true;
}
