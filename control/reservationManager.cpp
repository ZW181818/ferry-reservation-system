//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//***************************************************
// Module: ReservationManager.cpp
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
//     > Integrate laneUsed persistence: allocate then write reservation
//       (create: deduct capacity -> write laneUsed; delete: restore by laneUsed)
//       Add rollback on write failure.
//   - Version 5.1 - 2025/08/08 (Assistant)
//     > Add safety check in checkInFlow(): block check-in if sailing was deleted.
//
// Handles user interaction logic for reservation-related commands.
// Delegates validation and storage to ADT and entity layers.
//***************************************************

#include "reservationManager.h"
#include "sailingManager.h"
#include "../entity/sailingASM.h"

#include <iostream>
#include <cstring>
#include <string>
#include <algorithm> // for remove_if
#include <cmath>     // for std::ceil

using namespace std;

namespace {
    // 本地小工具：验证某个 sailingID 是否还存在
    bool sailingStillExists(const char* sailingID) {
        SailingASM s;
        s.initialize();
        int count = s.getRecordCount();
        SailingRecord r{};
        bool found = false;
        for (int i = 0; i < count; ++i) {
            if (s.getRecord(i, r) && strcmp(r.date, sailingID) == 0) {
                found = true;
                break;
            }
        }
        s.shutdown();
        return found;
    }
}

//--------------------------------------
float ReservationManager::calculateFare(
    const Vehicle& v   // in: vehicle data used to determine fare
)
/*
Calculates fare based on height and length:
- Regular (height ≤ 2.0m & length ≤ 7.0m): flat $14
- Oversized height (> 2.0m): $3/meter of length
- Oversized length only: $2/meter of length
Returns the calculated fare as float.
*/
{
    if (v.specialHeight <= 2.0f && v.specialLength <= 7.0f) {
        return 14.0f;
    } else if (v.specialHeight > 2.0f) {
        return v.specialLength * 3.0f;
    } else {
        return v.specialLength * 2.0f;
    }
}

//--------------------------------------
std::string normalizePhoneNumber(
    const std::string& raw  // in: raw phone number input (may include space/dash)
)
/*
Formats raw phone number into a standard format:
- 7 digits  → xxx-xxxx
- 10 digits → xxx-xxx-xxxx
- 11 digits → x-xxx-xxx-xxxx
Returns empty string if not valid.
*/
{
    std::string digits;
    for (char c : raw) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            digits.push_back(c);
        }
    }

    if (digits.length() == 7)
        return digits.substr(0, 3) + "-" + digits.substr(3, 4);
    else if (digits.length() == 10)
        return digits.substr(0, 3) + "-" + digits.substr(3, 3) + "-" + digits.substr(6, 4);
    else if (digits.length() == 11)
        return digits.substr(0, 1) + "-" + digits.substr(1, 3) + "-" + digits.substr(4, 3) + "-" + digits.substr(7, 4);

    return "";
}

//--------------------------------------
bool isValidLicensePlate(
    const std::string& plate  // in: license plate string to validate
)
/*
Checks if license plate is valid:
- Between 1 to 10 characters
- Must consist only of digits or letters (and dash '-')
Returns true if valid, false otherwise.
*/
{
    if (plate.length() < 1 || plate.length() > 10) return false;

    for (char c : plate) {
        if (!std::isdigit(static_cast<unsigned char>(c)) &&
            !std::isalpha(static_cast<unsigned char>(c)) &&
            c != '-') {
            return false;
        }
    }
    return true;
}

//--------------------------------------
void ReservationManager::initializeAll()
/*
Initializes vehicleASM and reservationASM.
Should be called once before accessing reservation operations.
*/
{
    vehicleASM.initialize();
    reservationASM.initialize();
}

//--------------------------------------
void ReservationManager::shutdown()
/*
Closes vehicleASM and reservationASM files.
Should be called before program shutdown.
*/
{
    vehicleASM.shutdown();
    reservationASM.shutdown();
}

//--------------------------------------
void ReservationManager::createFlow(
    SailingManager& sm  // in/out: sailing manager used for matching sailings and updating space
)
/*
Handles full user interaction to create a new reservation:
- Prompts vehicle type and size
- Displays and selects sailings based on dimensions
- Collects license plate and phone number
- Validates and stores vehicle and reservation records
- Deducts lane capacity and persists laneUsed
Cancels and exits cleanly if user aborts at any point.
*/
{
    cout << "-------------------------------------------------------" << endl;
    cout << " Create New Reservation" << endl;
    cout << "-------------------------------------------------------" << endl;

    auto roundUpToOneDecimal = [](float value) -> float {
        return std::ceil(value * 10.0f) / 10.0f;
    };

    int vehicleType = 0;
    float height = 2.0f, length = 7.0f;

    // Vehicle Type
    while (true) {
        cout << "> Enter Vehicle Type\t[1] Regular\t[2] Special : ";
        std::string input;
        std::getline(std::cin >> std::ws, input);

        try {
            vehicleType = std::stoi(input);
        } catch (...) {
            cout << "Invalid input! Please enter number (1 or 2)" << endl;
            continue;
        }

        if (vehicleType == 1 || vehicleType == 2) break;
        else cout << "Invalid input! Must be 1 (Regular) or 2 (Special)" << endl;
    }

    // Vehicle Size
    if (vehicleType == 2) {
        while (true) {
            cout << "> Enter Vehicle Height (2.0m ~ 9.9m) : ";
            std::string input;
            std::getline(std::cin >> std::ws, input);

            try {
                height = std::stof(input);
                height = roundUpToOneDecimal(height);

                if (height > 0.0f && height <= 2.0f) {
                    cout << "(Auto-correct: height adjusted to 2.0m)" << endl;
                    height = 2.0f;
                }
            } catch (...) {
                cout << "Invalid input! Please enter a valid number" << endl;
                continue;
            }

            if (height < 2.0f || height > 9.9f) {
                cout << "Invalid input! Height must be between 2.0 and 9.9 meters" << endl;
            } else break;
        }

        while (true) {
            cout << "> Enter Vehicle Length (7.0m ~ 99.9m) : ";
            std::string input;
            std::getline(std::cin >> std::ws, input);

            if (input.empty()) {
                cout << "Reservation Cancelled" << endl;
                return;
            }

            try {
                length = std::stof(input);
                length = roundUpToOneDecimal(length);

                if (length > 0.0f && length <= 7.0f) {
                    cout << "(Auto-correct: length adjusted to 7.0m)" << endl;
                    length = 7.0f;
                }
            } catch (...) {
                cout << "Invalid input! Please enter a valid number" << endl;
                continue;
            }

            if (length < 7.0f || length > 99.9f) {
                cout << "Invalid input! Length must be between 7.0 and 99.9 meters" << endl;
            } else break;
        }

        if (height <= 2.0f && length <= 7.0f) {
            cout << "Size is within Regular range. Switching to Regular type." << endl;
            vehicleType = 1;
            height = 2.0f;
            length = 7.0f;
        }

    } else {
        height = 2.0f;
        length = 7.0f;
    }

    // Select Available Sailing
    const int MAX_MATCH = 100;
    SailingRecord matchList[MAX_MATCH];
    int matchCount = sm.getMatchingSailings(height, length, matchList, MAX_MATCH);

    if (matchCount == 0) {
        cout << "No available sailings for this vehicle size." << endl;
        return;
    }

    const int PAGE_SIZE = 5;
    const char* selectedSailingId = sm.showAvailableAndSelect(matchList, matchCount, PAGE_SIZE);

    if (strlen(selectedSailingId) == 0) {
        cout << "No sailing selected. Reservation cancelled." << endl;
        return;
    }

    // License Plate
    std::string plateStr;
    while (true) {
        cout << "> Enter Vehicle License Plate (1~10 chars) : ";
        std::getline(std::cin >> std::ws, plateStr);

        if (!isValidLicensePlate(plateStr)) {
            cout << "Invalid license plate! Must be 1~10 chars, only letters (A-Z a-z), digits (0-9), or dash" << endl;
            continue;
        }

        // Transfer all alphabet to uppercase in case duped record
        for (auto &c : plateStr) c = std::toupper(static_cast<unsigned char>(c));
        break;
    }

    char plate[11];
    strncpy(plate, plateStr.c_str(), sizeof(plate) - 1); // transfer back to char[] type in order to save
    plate[sizeof(plate)-1] = '\0';

    // to check if duped
    if (reservationASM.existsReservation(plate, selectedSailingId)) {
        cout << "This license plate already has a reservation for the selected sailing!" << endl;
        cout << "Reservation cancelled." << endl;
        return;
    }

    // Customer Phone
    std::string rawPhone;
    std::string formattedPhone;
    while (true) {
        cout << "> Enter Customer Phone Number: ";
        std::getline(std::cin >> std::ws, rawPhone);

        formattedPhone = normalizePhoneNumber(rawPhone);
        if (formattedPhone.empty()) {
            cout << "Invalid phone number!" << endl;
            cout << "Accepted formats: x-xxx-xxx-xxxx, xxx-xxx-xxxx, xxx-xxxx (spaces and dashes are allowed but not required)." << endl;
            continue;
        }
        break;
    }

    // Final Check
    cout << "\n=== Reservation Summary ===" << endl;
    cout << "License Plate : " << plate << endl;
    cout << "Phone         : " << formattedPhone << endl;
    cout << "Sailing ID    : " << selectedSailingId << endl;
    cout << "Vehicle Type  : " << (vehicleType == 1 ? "Regular" : "Special") << endl;
    if (vehicleType == 2) {
        cout << "Height        : " << height << "m" << endl;
        cout << "Length        : " << length << "m" << endl;
    }
    cout << "===========================" << endl;

    int confirm = 0;
    while (true) {
        cout << "> Select - [1] Submit  [2] Cancel : ";
        std::string input;
        std::getline(std::cin >> std::ws, input);

        try {
            confirm = std::stoi(input);
        } catch (...) {
            confirm = 0; // invalid inputs
        }

        if (confirm == 1 || confirm == 2) break;
        cout << "Invalid choice! Please enter 1 (Confirm and Submit) or 2 (Cancel)" << endl;
    }

    if (confirm == 2) {
        cout << "Reservation cancelled" << endl;
        return;
    }

    // transfer to char[] type to save in vehicle.dat
    char phone[15];
    strncpy(phone, formattedPhone.c_str(), sizeof(phone)-1);
    phone[sizeof(phone)-1] = '\0';

    // Save Vehicle
    Vehicle v{};
    strncpy(v.licensePlate, plate, sizeof(v.licensePlate)-1);
    strncpy(v.customerPhone, phone, sizeof(v.customerPhone)-1);
    v.specialHeight = height;
    v.specialLength = length;
    std::string errMsg;
    bool consistent = checkVehicleConsistency(v, errMsg);

    if (!consistent) {
        cout << "[ERROR] " << errMsg << endl;
        cout << "Reservation cancelled." << endl;
        return;
    }

    // if new vehicle (no exist license plate)
    bool exists = false;
    int total = vehicleASM.getRecordCount();
    Vehicle tmp{};
    for (int i = 0; i < total; i++) {
        vehicleASM.getRecord(i, tmp);
        if (strcmp(tmp.licensePlate, v.licensePlate) == 0) {
            exists = true;
            break;
        }
    }

    if (!exists) {
        vehicleASM.addRecord(v);
        cout << "(Vehicle record saved)" << endl;
    } else {
        cout << "(Vehicle already exists, reuse existing record)" << endl;
    }

    // ===== Decide & Deduct lane (get actual usedLane), then persist reservation with laneUsed =====
    char usedLane = sm.updateLaneLengths(
        selectedSailingId,
        v.specialHeight,
        v.specialLength,
        /*isReversing=*/false,
        /*laneHint=*/'\0'     // ignored in allocate mode; SM decides 'H' or 'L' and returns it
    );

    if (usedLane != 'H' && usedLane != 'L') {
        cout << "[ERROR] Failed to allocate lane space on sailing " << selectedSailingId << ". Reservation cancelled." << endl;
        return;
    }

    if (!reservationASM.writeReservationRecord(plate, selectedSailingId, /*isOnboard=*/false, /*laneUsed=*/usedLane)) {
        cout << "[ERROR] Failed to write reservation to disk. Rolling back lane deduction..." << endl;
        // rollback capacity deduction
        sm.updateLaneLengths(selectedSailingId, v.specialHeight, v.specialLength, /*isReversing=*/true, /*laneHint=*/usedLane);
        return;
    }

    // (保持你项目原有约定：创建预约时就 +1 onboardCount；如果你的口径是 check-in 时再 +1，可以把这行移到 check-in 里)
    sm.updateOnboardCount(selectedSailingId, +1);

    cout << "Reservation Confirmed" << endl;
}

//--------------------------------------
void ReservationManager::deleteFlow(SailingManager& sm)
/*
Handles user interaction for deleting an existing reservation:
- Prompts for license plate
- Lists reservations whose sailings STILL EXIST
- Confirms selection and deletion
- Updates onboard count and frees lane space (lane-accurate)
Also: auto-purges any orphan reservations whose sailing was deleted.
*/
{
    // 防止持有旧句柄：刷新一次
    reservationASM.shutdown();
    reservationASM.initialize();

    cout << "-------------------------------------------------------" << endl;
    cout << " Delete Reservation" << endl;
    cout << "-------------------------------------------------------" << endl;

    char plate[11];
    cout << "> Enter Vehicle License Plate: ";
    cin >> plate;

    // Normalize to uppercase
    for (int i = 0; plate[i]; i++) {
        plate[i] = std::toupper(static_cast<unsigned char>(plate[i]));
    }

    // Find all reservations for the plate
    std::vector<int> indexes = reservationASM.findAllIndexesByLicense(plate);
    if (indexes.empty()) {
        cout << "No reservation found for " << plate << endl;
        return;
    }

    // --- Split into valid (sailing exists) and orphan (sailing deleted) ---
    std::vector<int> validIndexes;
    std::vector<int> orphanIndexes;
    for (int idx : indexes) {
        ReservationRecord rec = reservationASM.get(idx);
        if (sailingStillExists(rec.sailingId)) validIndexes.push_back(idx);
        else orphanIndexes.push_back(idx);
    }

    // --- Auto-purge orphans silently (descending order to avoid index shift) ---
    if (!orphanIndexes.empty()) {
        sort(orphanIndexes.begin(), orphanIndexes.end(), std::greater<int>());
        for (int idx : orphanIndexes) {
            reservationASM.deleteReservationByIndex(idx);
        }
        // 重新加载索引，避免删除后索引错乱
        indexes = reservationASM.findAllIndexesByLicense(plate);
        // 重新构建 validIndexes
        validIndexes.clear();
        for (int idx : indexes) {
            ReservationRecord rec = reservationASM.get(idx);
            if (sailingStillExists(rec.sailingId)) validIndexes.push_back(idx);
        }
    }

    if (validIndexes.empty()) {
        cout << "No valid reservations remain for " << plate
             << " (sailings were deleted and related reservations were purged)." << endl;
        return;
    }

    cout << "\nFound " << validIndexes.size() << " reservation";
    if (validIndexes.size() > 1) cout << "s";
    cout << ":" << endl;

    for (size_t i = 0; i < validIndexes.size(); i++) {
        ReservationRecord rec = reservationASM.get(validIndexes[i]);
        cout << (i + 1) << ". Sailing: " << rec.sailingId
             << ", Onboard: " << (rec.isOnboard ? "Yes" : "No") << endl;
    }

    // User selects reservation to delete
    cout << "\nEnter the number for the reservation you want to delete: ";
    int choice = 0;
    if (!(cin >> choice) || choice <= 0 || choice > (int)validIndexes.size()) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Cancelled" << endl;
        return;
    }

    // Resolve target index after any purges
    int targetIndex = validIndexes[choice - 1];
    ReservationRecord selected = reservationASM.get(targetIndex);

    cout << "\nYou selected:" << endl;
    cout << "Sailing: " << selected.sailingId
         << ", Onboard: " << (selected.isOnboard ? "Yes" : "No") << endl;

    // Confirm deletion
    int confirm = 0;
    while (true) {
        cout << "\n> Confirm\t[1] Delete\t[2] Cancel : ";
        string input;
        cin >> input;
        try { confirm = stoi(input); } catch (...) { confirm = 0; }
        if (confirm == 1 || confirm == 2) break;
        cout << "Invalid choice! Please enter 1 (Delete) or 2 (Cancel)" << endl;
    }

    if (confirm == 2) {
        cout << "Cancelled" << endl;
        return;
    }

    // Lookup vehicle to restore lane capacity accurately
    Vehicle v{};
    bool vehicleFound = false;
    for (int i = 0, n = vehicleASM.getRecordCount(); i < n; i++) {
        Vehicle tmp{};
        if (vehicleASM.getRecord(i, tmp) && strcmp(tmp.licensePlate, selected.licensePlate) == 0) {
            v = tmp;
            vehicleFound = true;
            break;
        }
    }
    if (!vehicleFound) {
        cout << "[WARN] Vehicle info not found; lane space restore may be skipped.\n";
    }

    // Delete & restore counters/capacity
    bool success = reservationASM.deleteReservationByIndex(targetIndex);
    if (!success) {
        cout << "Failed to delete reservation" << endl;
        return;
    }

    cout << "Reservation deleted successfully." << endl;

    // Sailing should exist (we filtered), but double-check to be safe
    if (sailingStillExists(selected.sailingId)) {
        sm.updateOnboardCount(selected.sailingId, -1);

        if (vehicleFound) {
            char lane = selected.laneUsed; // 'H' or 'L'
            if (lane == 'H' || lane == 'L') {
                sm.updateLaneLengths(
                    selected.sailingId,
                    v.specialHeight,
                    v.specialLength,
                    /*isReversing=*/true,
                    /*laneHint=*/lane
                );
                cout << "Freed sailing lane space for " << selected.sailingId
                     << " (lane " << lane << ")\n";
            } else {
                cout << "[WARN] laneUsed invalid; skipped lane restore.\n";
            }
        }
    } else {
        // Sailing was removed between UI and deletion; do not touch counters/lanes.
        cout << "[INFO] Sailing has been deleted meanwhile; counters and lanes not updated." << endl;
    }
}


//--------------------------------------
void ReservationManager::checkInFlow()
/*
Handles check-in process for a vehicle:
- Prompts for license plate
- Filters pending reservations
- Blocks check-in if the sailing was deleted
- Displays vehicle info (type, size, fare)
- Confirms check-in and updates onboard status
Loops until user types '#' to exit.
*/
{
    // 强制刷新 reservationASM，避免持有删除前的旧文件指针/缓存
    reservationASM.shutdown();
    reservationASM.initialize();

    cout << "-------------------------------------------------------" << endl;
    cout << " Check-In (type '#' to return to Main Menu)" << endl;
    cout << "-------------------------------------------------------" << endl;

    while (true) {
        char plate[11];
        cout << "\n> Enter Vehicle License Plate (or '#' to quit): ";
        cin >> plate;

        if (strcmp(plate, "#") == 0) {
            cout << "Returning to Main Menu..." << endl;
            break;
        }

        for (int i = 0; plate[i]; i++) {
            plate[i] = std::toupper(static_cast<unsigned char>(plate[i]));
        }

        // 找到该车牌的所有预约索引
        std::vector<int> indexes = reservationASM.findAllIndexesByLicense(plate);
        if (indexes.empty()) {
            cout << "No reservation found for " << plate << endl;
            continue;
        }

        // 只保留“未登船”且其航次仍存在的预约（过滤掉被删除航次的“孤儿预约”）
        std::vector<int> pendingIndexes;
        for (int idx : indexes) {
            ReservationRecord rec = reservationASM.get(idx);
            if (!rec.isOnboard && sailingStillExists(rec.sailingId)) {
                pendingIndexes.push_back(idx);
            }
        }

        if (pendingIndexes.empty()) {
            cout << "No valid pending reservations for " << plate
                 << " (all checked in or their sailings were deleted)." << endl;
            continue;
        }

        int numResults = static_cast<int>(pendingIndexes.size());
        cout << "\nFound " << numResults << " pending reservation";
        if (numResults > 1) cout << "s";
        cout << ":" << endl;

        for (size_t i = 0; i < pendingIndexes.size(); i++) {
            ReservationRecord rec = reservationASM.get(pendingIndexes[i]);
            cout << (i + 1) << ". Sailing: " << rec.sailingId
                 << ", Onboard: " << (rec.isOnboard ? "Yes" : "No") << endl;
        }

        cout << "\nEnter the number [1 - " << numResults << "] to check in: ";
        int choice = 0;
        if (!(cin >> choice)) {
            // 非整数输入
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid input." << endl;
            continue;
        }

        if (choice <= 0 || choice > numResults) {
            cout << "Check-in cancelled." << endl;
            continue;
        }

        int targetIndex = pendingIndexes[choice - 1];
        ReservationRecord selected = reservationASM.get(targetIndex);

        // 再次保险：如果航次在列表展示后被删除，这里阻断
        if (!sailingStillExists(selected.sailingId)) {
            cout << "[ERROR] Sailing " << selected.sailingId
                 << " has been deleted. This reservation is invalid and cannot be checked in." << endl;
            continue;
        }

        // 读取车辆信息用于展示票价等
        Vehicle vehicleInfo{};
        bool vehicleFound = false;
        int vehicleCount = vehicleASM.getRecordCount();

        for (int i = 0; i < vehicleCount; i++) {
            Vehicle tmp{};
            if (vehicleASM.getRecord(i, tmp)) {
                if (strcmp(tmp.licensePlate, selected.licensePlate) == 0) {
                    vehicleInfo = tmp;
                    vehicleFound = true;
                    break;
                }
            }
        }

        cout << "\nYou selected:\n";
        cout << "Sailing ID:\t" << selected.sailingId << endl;
        cout << "Plate:\t\t" << selected.licensePlate << endl;

        if (!vehicleFound) {
            cout << "[WARNING] Vehicle info could not be found! Cannot show size & fare.\n";
        } else {
            string vType = (vehicleInfo.specialHeight > 2.0f || vehicleInfo.specialLength > 7.0f)
                         ? "Special" : "Regular";

            cout << "Vehicle Type:\t" << vType << endl;
            cout << "Vehicle Height:\t" << vehicleInfo.specialHeight << " m" << endl;
            cout << "Vehicle Length:\t" << vehicleInfo.specialLength << " m" << endl;

            float fare = 0.0f;
            if (vehicleInfo.specialHeight <= 2.0f && vehicleInfo.specialLength <= 7.0f) {
                fare = 14.0f;
            } else if (vehicleInfo.specialHeight > 2.0f) {
                fare = vehicleInfo.specialLength * 3.0f;
            } else {
                fare = vehicleInfo.specialLength * 2.0f;
            }

            cout << "Fare:\t\t$" << fare << endl;
        }

        cout << "Onboard:\t" << (selected.isOnboard ? "Yes" : "No") << endl;

        if (selected.isOnboard) {
            cout << "This reservation is already checked in!" << endl;
            continue;
        }

        int confirm = 0;
        while (true) {
            cout << "\n> Select\t[1] Check-in\t[2] Cancel : ";
            string input;
            cin >> input;

            try {
                confirm = stoi(input);
            } catch (...) {
                confirm = 0;
            }

            if (confirm == 1 || confirm == 2) break;
            cout << "Invalid choice! Please enter 1 (Check-in) or 2 (Cancel)" << endl;
        }

        if (confirm == 2) {
            cout << "Cancelled this vehicle." << endl;
            continue;
        }

        bool success = reservationASM.checkInReservationByIndex(targetIndex);
        if (success) {
            cout << "Vehicle " << plate << " checked in successfully." << endl;
        } else {
            cout << "Failed to check in" << endl;
        }
    }
}

//--------------------------------------
void ReservationManager::listAllReservations()
/*
Displays all reservations in the system:
- Ensures file state is reset
- Shows license plate, sailing ID, and onboard status
*/
{
    reservationASM.shutdown();
    reservationASM.initialize();

    int count = reservationASM.getRecordCount();
    cout << "\n=== Current Reservations ===" << endl;

    if (count == 0) {
        cout << "[INFO] No reservations found." << endl;
        cout << "============================" << endl;
        return;
    }

    for (int idx = 0; idx < count; idx++) {
        ReservationRecord rec = reservationASM.get(idx);
        rec.licensePlate[sizeof(rec.licensePlate)-1] = '\0';
        rec.sailingId[sizeof(rec.sailingId)-1] = '\0';

        cout << (idx + 1) << ". Plate: " << rec.licensePlate
             << ", Sailing: " << rec.sailingId
             << ", Onboard: " << (rec.isOnboard ? "Yes" : "No") << endl;
        // 如需调试 laneUsed，可加：
        // cout << " (Lane: " << rec.laneUsed << ")";
    }

    cout << "============================" << endl;
}

//--------------------------------------
void ReservationManager::listAllVehicles()
/*
Displays all vehicles currently stored:
- Includes plate, phone, height, length
- Identifies vehicle as Regular or Special
*/
{
    cout << "\n=== Current Vehicles ===" << endl;

    int count = vehicleASM.getRecordCount();
    if (count == 0) {
        cout << "[INFO] No vehicles found." << endl;
        return;
    }

    for (int i = 0; i < count; i++) {
        Vehicle v{};
        if (!vehicleASM.getRecord(i, v)) {
            cout << "[WARN] Failed to read vehicle record at index " << i << endl;
            continue;
        }

        cout << (i + 1) << ". Plate: " << v.licensePlate
             << ", Phone: " << v.customerPhone
             << ", Height: " << v.specialHeight
             << ", Length: " << v.specialLength;

        if (v.specialHeight > 2.0f || v.specialLength > 7.0f)
            cout << " [Special]";
        else
            cout << " [Regular]";

        cout << endl;
    }

    cout << "==========================" << endl;
}

//--------------------------------------
bool ReservationManager::checkVehicleConsistency(const Vehicle& newVehicle, std::string& errMsg)
/*
Checks whether a vehicle with the same plate already exists:
- If exists, checks for phone/size consistency
- Returns false and sets error message if conflict
- Returns true if consistent or new
*/
{
    int total = vehicleASM.getRecordCount();
    Vehicle existing{};

    for (int i = 0; i < total; i++) {
        if (!vehicleASM.getRecord(i, existing)) continue;

        if (strcmp(existing.licensePlate, newVehicle.licensePlate) == 0) {
            if (strcmp(existing.customerPhone, newVehicle.customerPhone) != 0) {
                errMsg = "\nPhone number mismatch for plate " + std::string(newVehicle.licensePlate);
                return false;
            }
            if (existing.specialHeight != newVehicle.specialHeight ||
                existing.specialLength != newVehicle.specialLength) {
                errMsg = "\nVehicle size mismatch for plate " + std::string(newVehicle.licensePlate);
                return false;
            }
            return true;
        }
    }

    return true;
}
