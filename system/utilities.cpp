//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//***************************************************
// utilities.cpp
// Version: 2.0
// Author: Wenbo Zhang
// Purpose: Provides system-wide startup, shutdown, reset, and backup operations.
// This module offers infrastructure-level support and lifecycle control
// for all major functional modules in the SuperFerry system.
//***************************************************
#include <iostream>
#include "utilities.h"
#include "../entity/ferryASM.h"
#include "../entity/reservationASM.h"
#include "../entity/sailingASM.h"
#include "../entity/vehicleASM.h"

using namespace std;

//--------------------------------------
// Function: start
// Purpose : Starts up all subsystems and initializes resources.
//--------------------------------------
void start() {
    cout << "[System] Startup complete. Resources initialized.\n";

    FerryASM::initialize();

    ReservationASM reservations;
    reservations.initialize();
    
    SailingASM sailings;
    sailings.initialize();
    
    VehicleASM vehicles;
    vehicles.initialize();
}

//--------------------------------------
// Function: shutdown
// Purpose : Gracefully closes all modules and flushes data.
//--------------------------------------
void shutdown() {
    cout << "[System] Shutdown complete. All data saved.\n";

    FerryASM::shutdown();

    ReservationASM reservations;
    reservations.shutdown();

    SailingASM sailings;
    sailings.shutdown();

    VehicleASM vehicles;
    vehicles.shutdown();
}

//--------------------------------------
// Function: reset
// Purpose : Triggers file-level reset for applicable subsystems.
// Notes   : Only FerryASM::reset implemented. Other resets pending.
//--------------------------------------
void reset() {
    cout << "[System] System reset triggered.\n";

    FerryASM::reset();

    ReservationASM reservations;
    reservations.reset();

    SailingASM sailings;
    sailings.reset();

    VehicleASM vehicles;
    vehicles.reset();
}
