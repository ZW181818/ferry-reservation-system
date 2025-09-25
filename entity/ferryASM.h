//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//***************************************************
// Module: ferryASM.h
// Version History:
//   - Version 1.0 - 2025/07/09 (Yanhong Li)
//     > Initial creation of Ferry persistent entity interface.
//   - Version 1.1 - 2025/07/16 (Vino Jeong)
//     > Added ferry class attributes and edited function signatures.
//   - Version 2.0 - 2025/07/20 (Vino Jeong)
//     > Changed class structure and added additional helper functions
// Provides access to binary storage of ferry records including capacity.
// Used by SailingManager when validating new sailings.
//***************************************************

#ifndef FERRY_ASM_H
#define FERRY_ASM_H

#include <iostream>
using namespace std;

//--------------------------------------
// Ferry record structure
struct Ferry {
    char ferryName[26];  // Ferry name - 25 max chars + null
    int HCLL;            // High Ceiling Lane Length
    int LCLL;            // Low Ceiling Lane Length
};

class FerryASM {
private:
    static std::fstream file;

public:
    //--------------------------------------
    static void initialize();
    /*
    Initializes the ferryASM system and opens the binary file.
    */

    //--------------------------------------
    static void shutdown();
    /*
    Shuts down the system and flushes/closes the ferry file.
    */

    //--------------------------------------
    static void reset();
    /*
    Deletes all ferry records and resets the file.
    */

    //--------------------------------------
    static bool writeFerry(
        const char* ferryName,  // in: ferry name
        const int HCLL,         // in: high ceiling lane length
        const int LCLL          // in: low ceiling lane length
    );
    /*
    Stores a new ferry record in the binary file.
    Returns true on success.
    */

    //--------------------------------------
    static bool deleteFerry(
        char* ferryName  // in: ferry name to delete
    );
    /*
    Deletes a ferry record from the binary file by name.
    */

    //--------------------------------------
    static bool ferryExists(
        const char* ferryName  // in: ferry name to check
    );
    /*
    Returns true if ferry name exists in the file.
    */

    //--------------------------------------
    static bool showFerriesAndSelect(
        Ferry* ferry,  // in/out: ferry object to select
        bool* quitMenu    // in: flag for quitting the function
    );
    /*
    Displays a list of ferries and lets user select one.
    Returns true if the operation was successful, false otherwise.
    */

};

#endif // FERRY_ASM_H
