//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//***************************************************
// SailingASM.h
// Version: 2.0
// Author: Wenbo Zhang
// Purpose: Header file for SailingASM class.
// Provides binary record-based file access for fixed-length SailingRecord.
//***************************************************

#ifndef SAILING_ASM_H
#define SAILING_ASM_H

#include <fstream>

//--------------------------------------
// Constants for record field lengths
const int DATE_LEN = 10;   // "TTT-DD-HH" + '\0' = 9 + 1
const int NAME_LEN = 26;   // Ferry name: varchar(1–25) + null

//--------------------------------------
// SailingRecord structure
struct SailingRecord {
    char date[DATE_LEN];              // Primary key: e.g., "ABC-17-08"
    char ferryName[NAME_LEN];         // Ferry name (max 25 chars)
    float highLaneRestLength;         // Remaining high lane length
    float lowLaneRestLength;          // Remaining low lane length
    int onboardVehicleCount;          // Vehicles marked as onboard
};

//--------------------------------------
// Binary file access class
class SailingASM {
private:
    std::fstream file;
    const char* filename = "sailings.dat";

public:
    //--------------------------------------
    // Initializes file stream for read/write access
    void initialize();

    //--------------------------------------
    // Closes file stream
    void shutdown();

    //--------------------------------------
    // Resets data to empty file
    void reset();

    //--------------------------------------
    // Forces file buffer to flush
    void flush();

    //--------------------------------------
    // Adds a new sailing record
    // Parameters:
    //   in  record - sailing information to write
    void addRecord(const SailingRecord& record);

    //--------------------------------------
    // Reads a sailing record by index
    // Parameters:
    //   in  index     - zero-based index in file
    //   out outRecord - output record to fill
    // Returns: true if record read successfully
    bool getRecord(int index, SailingRecord& outRecord);

    //--------------------------------------
    // Updates sailing record at given index
    // Parameters:
    //   in index   - position in file to update
    //   in record  - updated sailing info
    void updateRecord(int index, const SailingRecord& record);

    //--------------------------------------
    // Deletes record by index using overwrite strategy
    // Parameters:
    //   in index - index of record to delete
    void deleteRecord(int index);

    //--------------------------------------
    // Returns total number of sailing records in file
    int getRecordCount();

    //--------------------------------------
    // Searches if a ferry name is present in any record
    // and returns the IDs for the sailings that the ferry is assigned to
    // Parameters:
    //   in ferryName - name of ferry to search for
    // Returns: true if found
    std::vector<char*> findSailingsWithFerry(char* ferryName);

private:
    //--------------------------------------
    // Truncates file to hold only numRecords
    // Parameters:
    //   in numRecords - number of records to retain
    void truncateFile(int numRecords);
};

#endif
