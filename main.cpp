//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//***************************************************
// main.cpp
// Version: 2.0
// Author: Wenbo Zhang
// Purpose: Entry point for SuperFerry Reservation System.
// Initializes all subsystems and enters main menu loop.
//***************************************************

#include "ui/mainMenu.h"
#include "system/utilities.h"
#include "control/ferryManager.h"
#include "control/reservationManager.h"
#include "control/sailingManager.h"
#include "entity/ferryASM.h"
#include "entity/reservationASM.h"
#include "entity/sailingASM.h"
#include "entity/vehicleASM.h"

#include <iostream>
using namespace std;

//--------------------------------------
// Function: main
// Purpose : Entry point for entire system execution.
// in  : None
// out : int - exit code (0 = success)
//--------------------------------------
int main() {
    //============================
    //  System Startup
    //============================
    //initialize();   // Load config/data from disk
    start();        // Initialize memory, prepare state

    //============================
    //  Main Menu Loop
    //============================
    bool running = true;
    while (running) {
        running = displayMainMenu();  // 返回 false 则退出系统
        
    }

    //============================
    //  Graceful Exit
    //============================
    // backUp();      // Save data to disk
    // shutDown();    // Release resources, close files

    return 0;
}

