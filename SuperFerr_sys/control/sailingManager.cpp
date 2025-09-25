//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//***************************************************
// Module: sailingManager.cpp
// Version History:
//   - Version 3.0 - 2025/07/24 (Wenbo Zhang)
//     > Finalized public interface, added pagination and onboard tracking.
//   - Version 3.1 - 2025/07/29 (Vino Jeong)
//     > Updated createSailingViaUI() and integrated new ferry function.
//   - Version 3.1 - 2025/08/02 (Wenbo Zhang)
//     > Fixed delete sailing function.
//   - Version 3.2 - 2025/08/04 (Wenbo Zhang & Vino Jeong)
//     > Improved formatting for the sailing report
//   - Version 3.3 - 2025/08/05 (Wenbo Zhang)
//     > Add updateLaneLengths overload that returns laneUsed ('H'/'L')
//       and accepts laneHint when reversing (lane-accurate restore).
//       Remove extra cin.ignore in list/delete UIs.
//
// Provides UI and logical control for all sailing-related operations,
// including creation, deletion, filtering, and onboard management.
// Interfaces with SailingASM and VehicleASM for data persistence.
//***************************************************

#include "sailingManager.h"
#include <cstring>
#include <iostream>
#include <iomanip>
#include "../entity/reservationASM.h"
#include "../entity/ferryASM.h"

using namespace std;

//--------------------------------------
void SailingManager::initialize() {
    db.initialize();
}

//--------------------------------------
void SailingManager::close() {
    db.shutdown();
}

//--------------------------------------
void SailingManager::deleteAllSailings() {
    int count = db.getRecordCount();
    for (int i = count - 1; i >= 0; --i) {
        db.deleteRecord(i);
    }
    db.flush();
}

//--------------------------------------
void SailingManager::printAllSailings() {
    const int PAGE_SIZE = 5;
    int totalRecords = db.getRecordCount();

    // ⚠️ 移除多余的 cin.ignore，避免需要多按一次回车
    // cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (totalRecords == 0) {
        cout << "\n[Info] No sailings available in the system.\n" << endl;
        string input;
        while (true) {
            cout << "Enter 'q' to return to Main Menu: ";
            getline(cin, input);
            if (input == "q" || input == "Q") {
                cout << "Returning to Main Menu...\n";
                return;
            } else {
                cout << "[Error] Invalid input. Please type 'q' to continue.\n";
            }
        }
    }

    int totalPages = (totalRecords + PAGE_SIZE - 1) / PAGE_SIZE;
    int currentPage = 0;

    while (true) {
        // header
        cout << "\n============================= Sailing Report =============================\n";
        int start = currentPage * PAGE_SIZE;
        int end = min(start + PAGE_SIZE, totalRecords);

        cout << setfill(' ');
        cout << "      ";
        cout << left << setw(12) << "SailingID";
        cout << left << setw(28) << "Ferry Name";
        cout << left << setw(8) << "HRL (m)" << left << setw(8) << "LRL (m)";
        cout << left << setw(8) << "Onboard";

        cout << "\n" << endl; // Spacing

        // Rows
        for (int i = start; i < end; ++i) {
            SailingRecord r;
            if (db.getRecord(i, r)) {
                int onboard = getOnboardVehicleCount(r.date);
                cout << right << setw(4) << (i + 1) << "  " << left << setw(12) << r.date
                     << left << setw(28) << r.ferryName
                     << left << setw(8) << fixed << setprecision(1) << r.highLaneRestLength
                     << left << setw(8) << fixed << setprecision(1) << r.lowLaneRestLength
                     << left << setw(4) << onboard << "\n";
            }
        }

        cout << "==========================================================================\n";
        cout << "[Page " << (currentPage + 1) << " of " << totalPages << "]\n";
        cout << "'n' (next), 'p' (prev), [1~" << totalPages << "] page, 'q' (quit): ";

        string input;
        cout << flush;
        cin >> ws;
        getline(cin, input);

        if (input == "q" || input == "Q") break;
        else if (input == "n" || input == "N") {
            if (currentPage < totalPages - 1) currentPage++;
            else cout << "[Info] Already at the last page.\n";
        }
        else if (input == "p" || input == "P") {
            if (currentPage > 0) currentPage--;
            else cout << "[Info] Already at the first page.\n";
        }
        else {
            bool valid = true;
            for (char c : input) if (!isdigit(static_cast<unsigned char>(c))) valid = false;

            if (valid) {
                int page = stoi(input);
                if (page >= 1 && page <= totalPages) currentPage = page - 1;
                else cout << "[Error] Page out of range.\n";
            } else {
                cout << "[Error] Invalid input. Try 'n', 'p', number, or 'q'.\n";
            }
        }
    }

    cout << "Returning to Main Menu...\n" << endl;
}



//--------------------------------------
int SailingManager::getSailingCount() {
    return db.getRecordCount();
}

//--------------------------------------
bool SailingManager::getSailingByIndex(int index, SailingRecord& out) {
    return db.getRecord(index, out);
}

//--------------------------------------
bool SailingManager::sailingExists(const char* date) {
    int count = db.getRecordCount();
    SailingRecord r;
    for (int i = 0; i < count; ++i) {
        if (db.getRecord(i, r) && strcmp(r.date, date) == 0) return true;
    }
    return false;
}

//--------------------------------------
bool SailingManager::addSailing(const SailingRecord& record) {
    if (sailingExists(record.date)) {
        cout << "Sailing already exists for date: " << record.date << endl;
        return false;
    }
    db.addRecord(record);
    db.flush();
    return true;
}

//--------------------------------------
bool SailingManager::deleteSailingByDate(const char* date) {
    int count = db.getRecordCount();
    SailingRecord r;

    for (int i = 0; i < count; ++i) {
        if (db.getRecord(i, r) && strcmp(r.date, date) == 0) {

            // --- 先：静默清理所有与该航次绑定的预约（包含已 check-in 的） ---
            ReservationASM resASM;
            resASM.initialize();

            // 反复扫描删除，直到没有匹配项（更稳妥，避免漏删）
            while (true) {
                bool deletedOne = false;
                int rc = resASM.getRecordCount();
                for (int j = rc - 1; j >= 0; --j) {
                    ReservationRecord rr = resASM.get(j);
                    if (strcmp(rr.sailingId, date) == 0) {
                        resASM.deleteReservationByIndex(j); // 忽略返回值，继续删
                        deletedOne = true;
                        // 不在这里 break；倒序可安全继续
                    }
                }
                if (!deletedOne) break;
            }

            resASM.shutdown();

            // --- 后：删除该航次本体 ---
            db.deleteRecord(i);
            db.flush();
            return true;
        }
    }
    return false;
}



//--------------------------------------
int SailingManager::getMatchingSailings(float height, float length, SailingRecord* outArray, int maxCount) {
    int total = 0;
    int count = db.getRecordCount();
    SailingRecord r;

    for (int i = 0; i < count && total < maxCount; ++i) {
        if (db.getRecord(i, r)) {
            bool isTall = (height > 2.0f);
            bool canFit = false;

            if (isTall) {
                if (r.highLaneRestLength >= length) canFit = true;
            } else {
                if (r.lowLaneRestLength >= length || r.highLaneRestLength >= length) canFit = true;
            }

            if (canFit) {
                outArray[total++] = r;
            }
        }
    }
    return total;
}

//--------------------------------------
int SailingManager::getSailingsByPage(
    const SailingRecord* matchList, int matchCount,
    int pageNum, int pageSize,
    SailingRecord* outArray
) {
    int start = pageNum * pageSize;
    int end = start + pageSize;
    if (start >= matchCount) return 0;

    int actual = 0;
    for (int i = start; i < end && i < matchCount; ++i) {
        outArray[actual++] = matchList[i];
    }
    return actual;
}

//--------------------------------------
const char* SailingManager::showAvailableAndSelect(
    const SailingRecord* matchList, int matchCount,
    int pageSize
) {
    int currentPage = 0;
    char command;

    while (true) {
        int start = currentPage * pageSize;
        int end = (start + pageSize < matchCount) ? (start + pageSize) : matchCount;

        cout << "\n================== Available Sailings ==================" << endl;
        for (int i = start; i < end; ++i) {
            cout << (i - start + 1) << ". "
                 << matchList[i].date << "\t"
                 << "HRL: " << fixed << setprecision(1) << matchList[i].highLaneRestLength << " m\t"
                 << "LRL: " << matchList[i].lowLaneRestLength << " m" << endl;
        }
        cout << "[Results " << (start + 1) << " to " << end << " of " << matchCount << "]\n";
        cout << "Select [1~" << (end - start)
             << "], or type 'n' (next), 'p' (prev), or 'q' (quit): ";

        cin >> command;

        if (command == 'n') {
            if ((currentPage + 1) * pageSize < matchCount) currentPage++;
        } else if (command == 'p') {
            if (currentPage > 0) currentPage--;
        } else if (command == 'q') {
            return "";
        } else if (isdigit(static_cast<unsigned char>(command))) {
            int selection = command - '0';
            if (selection >= 1 && selection <= (end - start)) {
                return matchList[start + selection - 1].date;
            }
        } else {
            cout << "Invalid input." << endl;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

//--------------------------------------
void SailingManager::createSailingViaUI() {
    SailingRecord record;
    cout << "\n==== Create New Sailing ====" << endl;

    // 输入并验证 Sailing ID
    while (true) {
        cout << "Enter Sailing Date (TTT-DD-HH): ";
        char input[20];
        cin >> ws;
        cin.getline(input, sizeof(input));

        if (isValidSailingId(input, record.date)) break;
        else {
            cout << "Invalid format! Must be TTT-DD-HH where:\n"
                 << "- TTT: 3 letters (A–Z)\n"
                 << "- DD: 01–31\n"
                 << "- HH: 01–24\n\n";
        }
    }

    bool quitMenu = false;
    Ferry selectedFerry;
    // This will select the ferry name to add to the sailing record
    if (!FerryASM::showFerriesAndSelect(&selectedFerry,  &quitMenu)) {
        cout << "Ferry was not assigned." << endl;
        cout << "Press enter to continue." << endl;
        cin.ignore(128, '\n');
        cin.get();

        return;
    }

    strncpy(record.ferryName, selectedFerry.ferryName, NAME_LEN);
    record.ferryName[NAME_LEN - 1] = '\0'; // terminate string
    record.highLaneRestLength = selectedFerry.HCLL;
    record.lowLaneRestLength = selectedFerry.LCLL;

    if (addSailing(record)) {
        cout << "\n-----------------------------------" << endl;
        cout << "Ferry Name:\t\t" << record.ferryName << endl;
        cout << "High Ceiling Lane:\t" << record.highLaneRestLength << endl;
        cout << "Low Ceiling Lane:\t" << record.lowLaneRestLength << endl;
        cout << "-----------------------------------\n" << endl;
    }
    else
        cout << "Failed to create sailing (duplicate date)." << endl;
}

//--------------------------------------
void SailingManager::deleteSailingViaUI() {
    const int pageSize = 5;
    int currentPage = 0;
    string input;

    // ⚠️ 移除多余的 cin.ignore，避免需要多按一次回车
    // cin.ignore(numeric_limits<streamsize>::max(), '\n');

    while (true) {
        int total = db.getRecordCount();
        if (total == 0) {
            cout << "\n[Info] No sailings available in the system.\n" << endl;

            string input2;
            while (true) {
                cout << "Enter 'q' to return to Main Menu: ";
                getline(cin, input2);
                if (input2 == "q" || input2 == "Q") {
                    cout << "Returning to Main Menu...\n";
                    return;
                } else {
                    cout << "[Error] Invalid input. Please type 'q' to continue.\n";
                }
            }
        }

        int start = currentPage * pageSize;
        int end = min(start + pageSize, total);

        cout << "\n==== Delete Sailing ====" << endl;
        for (int i = start; i < end; ++i) {
            SailingRecord r;
            if (db.getRecord(i, r)) {
                cout << (i - start + 1) << ". " << r.date << "\t"
                     << "HRL: " << fixed << setprecision(1) << r.highLaneRestLength << " m\t"
                     << "LRL: " << r.lowLaneRestLength << " m" << endl;
            }
        }

        cout << "\n[Page " << (currentPage + 1) << "] Select 1~" << (end - start)
             << ", 'n'=next, 'p'=prev, 'q'=quit: ";
        cin >> ws;
        getline(cin, input);

        if (input == "n") {
            if ((currentPage + 1) * pageSize < total) currentPage++;
            else cout << "[Info] This is the last page.\n";
        } else if (input == "p") {
            if (currentPage > 0) currentPage--;
            else cout << "[Info] This is the first page.\n";
        } else if (input == "q") {
            cout << "Returning to Main Menu...\n";
            break;
        } else if (input.length() == 1 && isdigit(static_cast<unsigned char>(input[0]))) {
            int selection = input[0] - '0';
            if (selection >= 1 && selection <= (end - start)) {
                SailingRecord r;
                if (db.getRecord(start + selection - 1, r)) {
                    // 显示用户选择的 sailing 信息
                    cout << "\nYou selected to delete sailing:\n";
                    cout << "Date: " << r.date
                         << ", HRL: " << fixed << setprecision(1) << r.highLaneRestLength << " m"
                         << ", LRL: " << r.lowLaneRestLength << " m" << endl;

                    // 确认删除
                    string confirmInput;
                    while (true) {
                        cout << "\n> Confirm delete? [1] Confirm  [2] Cancel: ";
                        getline(cin, confirmInput);

                        if (confirmInput == "1") {
                            if (deleteSailingByDate(r.date)) {
                                cout << "Sailing deleted: " << r.date << endl;
                                if ((currentPage * pageSize) >= db.getRecordCount() && currentPage > 0)
                                    currentPage--;
                            } else {
                                cout << "[Error] Failed to delete sailing.\n";
                            }
                            break;
                        } else if (confirmInput == "2") {
                            cout << "Deletion cancelled.\n";
                            break;
                        } else {
                            cout << "[Error] Invalid input. Please enter 1 to confirm or 2 to cancel.\n";
                        }
                    }
                }
            } else {
                cout << "[Error] Invalid selection number.\n";
            }
        } else {
            cout << "[Error] Invalid input. Please enter a valid number, or 'n'/'p'/'q'.\n";
        }
    }
}

//--------------------------------------
// LEGACY wrapper: keep signature, delegate to new overload
void SailingManager::updateLaneLengths(const char* date, float height, float length, bool isReversing) {
    char lane = updateLaneLengths(date, height, length, isReversing, /*laneHint=*/'\0');
    if (lane == '\0') {
        if (isReversing)
            cout << "Error: Failed to free lane space for sailing " << date << endl;
        // allocate failure信息在新重载里已经打印
    } else if (isReversing) {
        cout << "Freed sailing lane space for " << date << " (lane " << lane << ")\n";
    }
}

//--------------------------------------
// NEW: lane-accurate capacity update
// - allocate: choose lane, deduct and return 'H'/'L'; fail -> '\0'
// - reverse : use laneHint to add back; invalid hint or failure -> '\0'
char SailingManager::updateLaneLengths(const char* date, float height, float length, bool isReversing, char laneHint) {
    if (height <= 0 || height > 9.9f || length <= 0 || length > 99.9f) {
        cout << "Error: Invalid vehicle dimensions. Height must be (0, 9.9], Length must be (0, 99.9]" << endl;
        return '\0';
    }

    int count = db.getRecordCount();
    for (int i = 0; i < count; ++i) {
        SailingRecord r;
        if (!(db.getRecord(i, r) && strcmp(r.date, date) == 0)) continue;

        if (isReversing) {
            // ----- restore capacity to exact lane -----
            if (laneHint != 'H' && laneHint != 'L') {
                cout << "Error: Invalid lane hint when freeing capacity (need 'H' or 'L').\n";
                return '\0';
            }
            if (laneHint == 'H') r.highLaneRestLength += length;
            else                r.lowLaneRestLength  += length;

            db.updateRecord(i, r);
            db.flush();
            return laneHint;
        } else {
            // ----- allocate capacity; choose lane deterministically -----
            bool isTall = (height > 2.0f);
            if (isTall) {
                // tall vehicles must use H
                if (r.highLaneRestLength >= length) {
                    r.highLaneRestLength -= length;
                    db.updateRecord(i, r);
                    db.flush();
                    return 'H';
                } else {
                    cout << "Error: Not enough HRL space for tall vehicle on sailing " << date << endl;
                    return '\0';
                }
            } else {
                // regular: prefer L, fallback H
                if (r.lowLaneRestLength >= length) {
                    r.lowLaneRestLength -= length;
                    db.updateRecord(i, r);
                    db.flush();
                    return 'L';
                } else if (r.highLaneRestLength >= length) {
                    r.highLaneRestLength -= length;
                    db.updateRecord(i, r);
                    db.flush();
                    return 'H';
                } else {
                    cout << "Error: Not enough space for this vehicle on sailing " << date << endl;
                    return '\0';
                }
            }
        }
    }

    cout << "Error: Sailing not found for date " << date << endl;
    return '\0';
}

//--------------------------------------
int SailingManager::getOnboardVehicleCount(const char* sailingID) {
    ReservationASM reservationASM;
    reservationASM.initialize();

    int total = reservationASM.getRecordCount();
    int onboardCount = 0;

    for (int i = 0; i < total; i++) {
        ReservationRecord rec = reservationASM.get(i);
        if (strcmp(rec.sailingId, sailingID) == 0 && rec.isOnboard) {
            onboardCount++;
        }
    }

    reservationASM.shutdown();
    return onboardCount;
}

//--------------------------------------
void SailingManager::updateOnboardCount(const char* date, int delta) {
    int count = db.getRecordCount();
    for (int i = 0; i < count; ++i) {
        SailingRecord r;
        if (db.getRecord(i, r) && strcmp(r.date, date) == 0) {
            r.onboardVehicleCount += delta;
            if (r.onboardVehicleCount < 0) r.onboardVehicleCount = 0;
            db.updateRecord(i, r);
            db.flush();
            return;
        }
    }
    cout << "WARN: Sailing not found for date " << date << endl;
}

//--------------------------------------
bool SailingManager::isValidSailingId(const char* input, char out[DATE_LEN]) {
    if (strlen(input) != 9 || input[3] != '-' || input[6] != '-') return false;

    for (int i = 0; i < 3; ++i) {
        if (!isalpha(static_cast<unsigned char>(input[i]))) return false;
        out[i] = toupper(static_cast<unsigned char>(input[i]));
    }

    out[3] = '-';

    if (!isdigit(static_cast<unsigned char>(input[4])) || !isdigit(static_cast<unsigned char>(input[5]))) return false;
    int day = (input[4] - '0') * 10 + (input[5] - '0');
    if (day < 1 || day > 31) return false;
    out[4] = input[4];
    out[5] = input[5];

    out[6] = '-';

    if (!isdigit(static_cast<unsigned char>(input[7])) || !isdigit(static_cast<unsigned char>(input[8]))) return false;
    int hour = (input[7] - '0') * 10 + (input[8] - '0');
    if (hour < 1 || hour > 24) return false;
    out[7] = input[7];
    out[8] = input[8];

    out[9] = '\0';
    return true;
}
