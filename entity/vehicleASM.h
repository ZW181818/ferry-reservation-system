//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//***************************************************
// VehicleASM.h
// Version: 2.0
// Author: Yanhong Li, Wenbo Zhang
// Purpose: Defines the VehicleASM class and the Vehicle struct.
// This module provides class-based binary file access
// for Vehicle records, supporting fixed-length record
// storage and random access modification.
//***************************************************

#ifndef VEHICLE_ASM_H
#define VEHICLE_ASM_H

#include <fstream>

//---------------------------------------------
// Vehicle record structure (fixed length)
struct Vehicle {
    char licensePlate[11];     // e.g., "ABC123"
    char customerPhone[15];    // e.g., "6041234567"
    float specialLength;       // Vehicle length in meters (up to 1 decimal)
    float specialHeight;       // Vehicle height in meters (up to 1 decimal)
};

//---------------------------------------------
// VehicleASM class: manages binary file I/O for Vehicle
class VehicleASM {
private:
    const char* filename = "vehicles.dat";  // Binary file path
    std::fstream file;                      // File stream

public:
    //---------------------------------------------
    // Initialize file stream for read/write access
    // @param (none)
    // @return (none)
    void initialize();

    //---------------------------------------------
    // Clears all the data in the Vehicle file
    // @param (none)
    // @return (none)
    void reset();

    //---------------------------------------------
    // Add a new vehicle record to end of file
    // @param in: record - vehicle data to write
    // @return (none)
    void addRecord(const Vehicle& record);

    //---------------------------------------------
    // Get a vehicle record by index
    // @param in: index - record index (0-based)
    // @param out: outRecord - filled with vehicle data
    // @return true if successful
    bool getRecord(int index, Vehicle& outRecord);

    //---------------------------------------------
    // Update vehicle record at index
    // @param in: index - record index to update
    // @param in: record - updated vehicle data
    // @return (none)
    void updateRecord(int index, const Vehicle& record);

    //---------------------------------------------
    // Delete record at index by overwrite-and-truncate
    // @param in: index - index of record to delete
    // @return (none)
    void deleteRecord(int index);

    //---------------------------------------------
    // Get total number of records
    // @param (none)
    // @return record count
    int getRecordCount();

    //---------------------------------------------
    // Force flush to disk
    // @param (none)
    // @return (none)
    void flush();

    //---------------------------------------------
    // Close the file stream
    // @param (none)
    // @return (none)
    void shutdown();

    //---------------------------------------------
    // Retrieve a vehicle by license plate
    // @param in: licensePlate - plate to search
    // @return matching Vehicle record, or empty struct if not found
    Vehicle getVehicleRecord(const char licensePlate[11]);

private:
    //---------------------------------------------
    // Helper to truncate the binary file to n records
    // @param in: numRecords - number of records to keep
    // @return (none)
    void truncateFile(int numRecords);
};

#endif // VEHICLE_ASM_H
