//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//***************************************************
// VehicleASM.cpp
// Version: 2.0
// Author: Yanhong Li, Wenbo Zhang
// Purpose: Defines the VehicleASM class and the Vehicle struct.
// This module provides class-based binary file access
// for Vehicle records, supporting fixed-length record
// storage and random access modification.
//***************************************************

#include "vehicleASM.h"
#include <iostream>
#include <cstdio>
#include <cstring>

using namespace std;

//--------------------------------------
// Initialize file stream for read/write
void VehicleASM::initialize() {
    file.open(filename, ios::in | ios::out | ios::binary);
    if (!file) {
        file.clear();
        file.open(filename, ios::out | ios::binary); // create empty file
        file.close();
        file.open(filename, ios::in | ios::out | ios::binary);
    }
    if (!file.good()) {
        cerr << "VehicleASM Error: Could not open file." << endl;
    }
}

void VehicleASM::reset() {
    if (file.is_open()) file.close();

    ofstream resetFile(filename, ios::out | ios::trunc | ios::binary);
    
    if (resetFile.is_open()) resetFile.close();
    else {
        cerr << "Could not reset the Vehicle file." << endl;
        return;
    }

    file.open(filename, ios::in | ios::out | ios::binary);

    if (!file) {
        cerr << "VehicleASM: File could not be reopened." << endl;
    }
}

//--------------------------------------
// Add a vehicle record to end of file
void VehicleASM::addRecord(const Vehicle& record) {
    file.seekp(0, ios::end);
    file.write(reinterpret_cast<const char*>(&record), sizeof(Vehicle));
}

//--------------------------------------
// Get a vehicle record by index
bool VehicleASM::getRecord(int index, Vehicle& outRecord) {
    int offset = index * sizeof(Vehicle);
    file.seekg(offset, ios::beg);
    file.read(reinterpret_cast<char*>(&outRecord), sizeof(Vehicle));
    return file.good();
}

//--------------------------------------
// Update a vehicle record by index
void VehicleASM::updateRecord(int index, const Vehicle& record) {
    int offset = index * sizeof(Vehicle);
    file.seekp(offset, ios::beg);
    file.write(reinterpret_cast<const char*>(&record), sizeof(Vehicle));
}

//--------------------------------------
// Delete a vehicle record by index
void VehicleASM::deleteRecord(int index) {
    int count = getRecordCount();
    if (index < 0 || index >= count) return;

    if (index != count - 1) {
        Vehicle last;
        getRecord(count - 1, last);
        updateRecord(index, last);
    }

    file.flush();
    file.close();
    truncateFile(count - 1);
    file.open(filename, ios::in | ios::out | ios::binary);
}

//--------------------------------------
// Get total number of vehicle records
int VehicleASM::getRecordCount() {
    file.seekg(0, ios::end);
    return file.tellg() / sizeof(Vehicle);
}

//--------------------------------------
// Force flush to disk
void VehicleASM::flush() {
    file.flush();
}

//--------------------------------------
// Close the file stream
void VehicleASM::shutdown() {
    file.close();
}

//--------------------------------------
// Search for vehicle by license plate
Vehicle VehicleASM::getVehicleRecord(const char licensePlate[11]) {
    int count = getRecordCount();
    Vehicle v;

    for (int i = 0; i < count; ++i) {
        getRecord(i, v);
        if (strcmp(v.licensePlate, licensePlate) == 0) {
            return v;
        }
    }

    Vehicle empty = {};
    empty.licensePlate[0] = '\0';
    return empty;
}

//--------------------------------------
// Truncate file to specified number of records
void VehicleASM::truncateFile(int numRecords) {
    fstream oldFile(filename, ios::in | ios::binary);
    fstream newFile("temp.dat", ios::out | ios::binary);

    Vehicle temp;
    for (int i = 0; i < numRecords; ++i) {
        oldFile.read(reinterpret_cast<char*>(&temp), sizeof(Vehicle));
        newFile.write(reinterpret_cast<const char*>(&temp), sizeof(Vehicle));
    }

    oldFile.close();
    newFile.close();

    remove(filename);
    rename("temp.dat", filename);
}
