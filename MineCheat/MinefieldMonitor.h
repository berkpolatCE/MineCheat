#ifndef MINEFIELD_MONITOR_H
#define MINEFIELD_MONITOR_H

#include <vector>
#include <string>
#include <Windows.h>
#include <unordered_map>

// Offsets and constants
constexpr uintptr_t WIDTH_OFFSET = 0x5334;
constexpr uintptr_t HEIGHT_OFFSET = 0x5338;
constexpr uintptr_t FIRST_MINE_OFFSET = 0x5361;
constexpr int ROW_OFFSET = 0x20;

// Cell values
enum CellValues {
    EMPTY_QUESTION = 13,
    EMPTY_FLAGGED = 14,
    NO_MINE = 15,
    PADDING = 16,
    EMPTY_REVEALED = 64,
    REVEALED_1 = 65,
    REVEALED_2 = 66,
    REVEALED_3 = 67,
    REVEALED_4 = 68,
    REVEALED_5 = 69,
    REVEALED_6 = 70,
    REVEALED_7 = 71,
    REVEALED_8 = 72,
    MINE_QUESTION = 141,
    MINE_FLAGGED = 142,
    MINE = 143
};

struct CellPosition {
    int x, y;
    bool isMine;
};

char GetCellDisplayChar(BYTE cellValue);
std::string CreateMineFieldDisplay(const std::vector<std::vector<BYTE>>& mineField, BYTE width, BYTE height);
std::vector<std::vector<BYTE>> GetMineField(HANDLE processHandle, uintptr_t baseAddress, BYTE width, BYTE height);
void MonitorMineField(HANDLE processHandle, uintptr_t baseAddress, int refreshRateMs = 100);

#endif // MINEFIELD_MONITOR_H
