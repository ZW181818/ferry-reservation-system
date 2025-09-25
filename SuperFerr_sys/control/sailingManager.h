//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//***************************************************
// Module: sailingManager.h
// Version History:
//   - Version 3.0 - 2025/07/24 (Wenbo Zhang)
//     > Finalized public interface, added pagination and onboard tracking.
//   - Version 3.3 - 2025/08/05 (Wenbo Zhang)
//     > Add updateLaneLengths overload that returns laneUsed ('H'/'L')
//       and accepts laneHint when reversing (for lane-accurate restore).
//
// Provides UI and logical control for all sailing-related operations,
// including creation, deletion, filtering, and onboard management.
// Interfaces with SailingASM and VehicleASM for data persistence.
//***************************************************

#ifndef SAILING_MANAGER_H
#define SAILING_MANAGER_H

#include "../entity/sailingASM.h"
#include "../entity/vehicleASM.h"

class SailingManager {
private:
    SailingASM db;

public:
    //--------------------------------------
    void initialize();
    /*
    Initializes internal sailing data source (sailingASM).
    Must be called before any other operation.
    */

    //--------------------------------------
    void deleteAllSailings();
    /*
    Deletes all sailing records from the system.
    Use with caution in administrative contexts only.
    */

    //--------------------------------------
    void printAllSailings();
    /*
    Prints all sailings to the console.
    */

    //--------------------------------------
    int getSailingCount();
    /*
    Returns the number of sailings currently stored.
    */

    //--------------------------------------
    bool getSailingByIndex(
        int index,            // in: index of sailing to retrieve
        SailingRecord& out    // out: retrieved sailing record
    );
    /*
    Retrieves a sailing record at the specified index.
    Returns true if found, false otherwise.
    */

    //--------------------------------------
    void close();
    /*
    Closes the sailingASM file and releases resources.
    */

    //--------------------------------------
    bool sailingExists(
        const char* date  // in: sailing ID (format: TTT-DD-HH)
    );
    /*
    Checks if a sailing with the given ID exists.
    */

    //--------------------------------------
    bool addSailing(
        const SailingRecord& record  // in: new sailing record to add
    );
    /*
    Adds a new sailing to the data store.
    Returns true on success.
    */

    //--------------------------------------
    bool deleteSailingByDate(
        const char* date  // in: sailing ID to delete
    );
    /*
    Deletes a sailing with the specified ID.
    Returns true if deletion was successful.
    */

    //--------------------------------------
    int getMatchingSailings(
        float height,                  // in: required vehicle height
        float length,                  // in: required vehicle length
        SailingRecord* outArray,       // out: array of matching sailings
        int maxCount                   // in: maximum results to return
    );
    /*
    Finds sailings that can accommodate a vehicle with given size.
    Returns the number of matching sailings.
    */

    //--------------------------------------
    bool isValidSailingId(
        const char* input,  // in: user input string
        char out[DATE_LEN]  // out: normalized sailing ID if valid
    );
    /*
    Validates the sailing ID format (TTT-DD-HH).
    Returns true if valid, and fills `out`.
    */

    //--------------------------------------
    void createSailingViaUI();
    /*
    Provides UI flow for user to create a new sailing.
    */

    //--------------------------------------
    void deleteSailingViaUI();
    /*
    Provides UI flow for user to delete a sailing.
    */

    // -------- Capacity Updates (legacy & new) --------

    //--------------------------------------
    void updateLaneLengths(
        const char* date,  // in: sailing ID
        float height,      // in: vehicle height
        float length,      // in: vehicle length
        bool isReversing   // in: true to free capacity, false to allocate
    );
    /*
    LEGACY: Updates lane space based on height rule only.
    Kept for backward compatibility. New code should use the overload below
    to get/restore the exact lane used.
    */

    //--------------------------------------
    char updateLaneLengths(
        const char* date,  // in: sailing ID
        float height,      // in: vehicle height
        float length,      // in: vehicle length
        bool isReversing,  // in: true to free capacity, false to allocate
        char laneHint      // in: when freeing, must be 'H' or 'L'; ignored when allocating
    );
    /*
    NEW: Lane-accurate capacity update.
    - Allocation (isReversing = false): choose an appropriate lane and
      RETURN the actual lane used ('H' or 'L'); on failure return '\0'.
    - Freeing   (isReversing = true): RESTORE capacity to the lane specified by laneHint
      ('H' or 'L'). On success returns laneHint; on failure returns '\0'.
    This is used by ReservationManager to persist/restore laneUsed per reservation.
    */

    //--------------------------------------
    void updateOnboardCount(
        const char* date,  // in: sailing ID
        int delta          // in: +1 to increment, -1 to decrement
    );
    /*
    Adjusts onboard vehicle count for a sailing.
    */

    //--------------------------------------
    int getOnboardVehicleCount(
        const char* sailingID  // in: sailing ID
    );
    /*
    Returns the number of onboard vehicles for a sailing.
    */

    //--------------------------------------
    int getSailingsByPage(
        const SailingRecord* matchList,  // in: filtered sailing list
        int matchCount,                  // in: total number of matches
        int pageNum,                     // in: which page to fetch (0-based)
        int pageSize,                    // in: number per page
        SailingRecord* outArray          // out: array of sailings on that page
    );
    /*
    Extracts a page of sailings from the match list.
    Returns the number of sailings placed in outArray.
    */

    //--------------------------------------
    const char* showAvailableAndSelect(
        const SailingRecord* matchList,  // in: filtered sailing list
        int matchCount,                  // in: number of available matches
        int pageSize                     // in: page size to show
    );
    /*
    Displays paged list of available sailings and prompts user to select one.
    Returns a const char* pointer to the selected sailing ID, or empty if none.
    */
};

#endif  // SAILING_MANAGER_H
