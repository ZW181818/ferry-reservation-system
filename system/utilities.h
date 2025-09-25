//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//***************************************************
// utilities.h
// Version: 2.0
// Author: Wenbo Zhang
// Purpose: Provides system-wide startup, shutdown, reset, and backup operations.
// This module offers infrastructure-level support and lifecycle control
// for all major functional modules in the SuperFerry system.
//***************************************************

#ifndef UTILITIES_H
#define UTILITIES_H

//--------------------------------------
// Function: initialize
// Purpose : Initializes core system structures before main control loop.
// Notes   : Must be called before any other module interaction.
//--------------------------------------
void initialize();

//--------------------------------------
// Function: start
// Purpose : Starts all subsystems (UI, ReservationManager, etc.).
// Notes   : Typically called immediately after `initialize()`.
//--------------------------------------
void start();

//--------------------------------------
// Function: shutdown
// Purpose : Performs cleanup, flushes files, releases memory.
// Notes   : Should be called when the main loop ends.
//--------------------------------------
void shutdown();

//--------------------------------------
// Function: reset
// Purpose : Resets in-memory state and runtime structures.
// Notes   : Used in debugging or system reinitialization.
//--------------------------------------
void reset();

#endif // UTILITIES_H
