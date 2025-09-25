//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//***************************************************
// Module: ferryManager.h
// Version History:
//   - Version 2.0 - 2025/07/24 (Wenbo Zhang)
//     > Updated header format and parameter comments.
//   - Version 1.0 - 2025/07/09 (Yanhong Li)
//     > Initial creation of ferry manager interface.
//
// This module provides top-level ferry vessel management features,
// including ferry creation and deletion through user interaction.
// It relies on the FerryASM module to handle data persistence.
//***************************************************

#ifndef FERRYMANAGER_H
#define FERRYMANAGER_H

// using namespace std;

//--------------------------------------
void createFerry();
/*

Prompts the user to input a ferry name and its lane capacity for both
high-ceiling and low-ceiling vehicles. Validates the input and stores
the ferry record using the FerryASM module.

No parameters.
*/

//--------------------------------------
bool deleteFerry();
/*
Lists ferries for the user to reference one for deletion.
Removes ferry from system as long as no active sailings reference it.
*/

#endif // FERRYMANAGER_H
