//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//***************************************************
// Module: ReservationManager.h
// Version History:
//   - Version 1.0 - 2025/07/09 (Yanhong Li)
//     > Initial creation of reservation manager interface.
//   - Version 2.0 - 2025/07/19 (Yu-Chia Chang)
//     > Implement flows.
//   - Version 3.0 - 2025/07/21 (Yu-Chia Chang)
//     > Modify to run with sailing manager.
//   - Version 4.0 - 2025/07/24 (Wenbo Zhang)
//     > Modify to compile.
//   - Version 5.0 - 2025/08/05 (Wenbo Zhang)
//     > Integrate laneUsed persistence in create/delete flows
//       (createFlow writes laneUsed; deleteFlow restores capacity by lane)
//
// Handles user interaction logic for reservation-related commands.
// Delegates validation and storage to ADT and entity layers.
//***************************************************

#ifndef RESERVATION_MANAGER_H
#define RESERVATION_MANAGER_H

#include "../entity/vehicleASM.h"
#include "../entity/reservationASM.h"

class SailingManager;  // forward declaration

class ReservationManager {
private:
    VehicleASM vehicleASM;
    ReservationASM reservationASM;

public:
    //===============================
    // File Lifecycle Functions
    //===============================

    //--------------------------------------
    void initializeAll();
    /*
    Initializes all related ASM modules (VehicleASM, ReservationASM).
    Must be called before using any reservation functionality.
    */

    //--------------------------------------
    void shutdown();
    /*
    Flushes and closes all open ASM modules.
    Call during system shutdown.
    */

    //===============================
    // Core Functional Flows
    //===============================

    //--------------------------------------
    void createFlow(
        SailingManager& sm       // in: sailing manager for matching and space checks
    );
    /*
    Guides user through full reservation creation process:
    - Input vehicle info
    - Find matching sailing
    - Deduct lane capacity (decide actual lane 'H'/'L')
    - Persist reservation with laneUsed
    */

    //--------------------------------------
    void deleteFlow(
        SailingManager& sm       // in: sailing manager for lane space restore
    );
    /*
    Deletes a confirmed reservation and restores sailing lane capacity
    using the persisted laneUsed from the reservation record.
    */

    //--------------------------------------
    void checkInFlow();
    /*
    Marks a reservation as onboard. Increases onboard count.
    */

    //===============================
    // Debug / Display Utilities
    //===============================

    //--------------------------------------
    void listAllReservations();
    /*
    Prints a list of all current reservation records.
    */

    //--------------------------------------
    void listAllVehicles();
    /*
    Prints all vehicles registered in the system.
    */

    //===============================
    // Tool Methods
    //===============================

    //--------------------------------------
    bool checkVehicleConsistency(
        const Vehicle& newVehicle,   // in: vehicle info to check
        std::string& errMsg          // out: error reason if conflict
    );
    /*
    Ensures no duplicated license plates with mismatched info.
    Returns false with error message if conflict found.
    */

    //--------------------------------------
    float calculateFare(
        const Vehicle& v             // in: vehicle to calculate
    );
    /*
    Returns fare amount based on height and length rules.
    */
};

#endif // RESERVATION_MANAGER_H
