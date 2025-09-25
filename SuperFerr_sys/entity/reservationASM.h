//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//***************************************************
// Module: ReservationASM.h
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
//     > Add laneUsed to ReservationRecord (persist actual lane: 'H'/'L')
//     > Extend read/write APIs to include laneUsed
//
// Purpose:
//   Stores and retrieves reservation records from disk.
//   Used by ReservationManager and Check-in procedures.
//***************************************************

#ifndef RESERVATION_ASM_H
#define RESERVATION_ASM_H

#include <fstream>
#include <vector>

//--------------------------------------
// Structure: ReservationRecord
struct ReservationRecord {
    char sailingId[10];       // Format: TTT-DD-HH (9 + '\0')
    char licensePlate[11];    // Max 10 chars + null
    bool isOnboard;           // Check-in status
    char laneUsed;            // 'H' (HRL) or 'L' (LRL). For regular vehicles that fell back to HRL, this will be 'H'.
};

//--------------------------------------
// Class: ReservationASM
class ReservationASM {
private:
    const char* filename = "reservations.dat";
    std::fstream file;

    void truncateFile(int numRecords);

public:
    //======================
    // FI: File Initialization
    void initialize();                  // Open or create reservation file
    void shutdown();                    // Close reservation file
    void reset();                       // Clear the reservation data
    int  getRecordCount();              // Return total reservation count

    //======================
    // MO: Modification Operations
    bool writeReservationRecord(        // Add new reservation
        const char* licensePlate,
        const char* sailingID,
        bool isOnboard = false,
        char laneUsed = 'L'            // 'H' or 'L'. Default 'L' for regular case.
    );

    bool readReservationRecord(         // Read reservation by license plate (first match)
        const char* licensePlate,
        char* sailingID,
        bool& isOnboard,
        char& laneUsed
    );

    bool checkInReservation(            // Mark reservation as onboard
        const char* licensePlate
    );

    bool deleteReservationRecord(       // Delete reservation by license plate
        const char* licensePlate
    );

    //======================
    // Utility Methods
    int findIndexByLicense(const char* plate);                  // Find first match index
    std::vector<int> findAllIndexesByLicense(const char* plate);// Find all match indexes

    ReservationRecord get(int index);                           // Get reservation by index
    bool existsReservation(                                     // Check if reservation exists
        const char* licensePlate,
        const char* sailingID
    );

    bool checkInReservationByIndex(int index);                  // Check-in using index
    bool deleteReservationByIndex(int index);                   // Delete using index
};

#endif // RESERVATION_ASM_H
