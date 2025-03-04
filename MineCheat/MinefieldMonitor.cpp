#define NOMINMAX

#include "MinefieldMonitor.h"
#include "MemoryUtils.h"
#include <iostream>
#include <chrono>
#include <algorithm>

char GetCellDisplayChar(BYTE cellValue) {
    static const std::unordered_map<BYTE, char> cellMap = {
        {MINE, '*'},
        {MINE_FLAGGED, 'F'},
        {MINE_QUESTION, '?'},
        {NO_MINE, '.'},
        {EMPTY_FLAGGED, 'F'},
        {EMPTY_QUESTION, '?'},
        {PADDING, ' '},
        {EMPTY_REVEALED, ' '},
        {REVEALED_1, '1'},
        {REVEALED_2, '2'},
        {REVEALED_3, '3'},
        {REVEALED_4, '4'},
        {REVEALED_5, '5'},
        {REVEALED_6, '6'},
        {REVEALED_7, '7'},
        {REVEALED_8, '8'}
    };
    auto it = cellMap.find(cellValue);
    return (it != cellMap.end()) ? it->second : 'X';
}

std::string CreateMineFieldDisplay(const std::vector<std::vector<BYTE>>& mineField, BYTE width, BYTE height) {
    std::string result;
    result.reserve(width * height + 150);
    result += "Minesweeper Monitor - Live View\nWidth: " + std::to_string(width) +
        ", Height: " + std::to_string(height) + "\n\nMinesweeper Board:\n";
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            result.push_back(GetCellDisplayChar(mineField[y][x]));
        }
        result.push_back('\n');
    }
    return result;
}

std::vector<std::vector<BYTE>> GetMineField(HANDLE processHandle, uintptr_t baseAddress, BYTE width, BYTE height) {
    const uintptr_t firstMineAddress = baseAddress + FIRST_MINE_OFFSET;
    std::vector<std::vector<BYTE>> mineField(height, std::vector<BYTE>(width));
    for (int y = 0; y < height; y++) {
        uintptr_t rowAddress = firstMineAddress + (y * ROW_OFFSET);
        if (width <= 32) {
            ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(rowAddress),
                mineField[y].data(), width, nullptr);
        }
        else {
            for (int x = 0; x < width; x++) {
                mineField[y][x] = ReadMemory<BYTE>(processHandle, rowAddress + x);
            }
        }
    }
    return mineField;
}

static void ClearScreen() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
        return;
    COORD topLeft = { 0, 0 };
    DWORD length = csbi.dwSize.X * csbi.dwSize.Y, written;
    FillConsoleOutputCharacterA(hConsole, ' ', length, topLeft, &written);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, length, topLeft, &written);
    SetConsoleCursorPosition(hConsole, topLeft);
}

void MonitorMineField(HANDLE processHandle, uintptr_t baseAddress, int refreshRateMs) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    CONSOLE_CURSOR_INFO originalCursorInfo = cursorInfo;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
    ClearScreen();

    std::string lastDisplayString;
    auto lastRefreshTime = std::chrono::steady_clock::now();
    BYTE lastWidth = 0, lastHeight = 0;
    std::vector<std::vector<BYTE>> mineField;
    int maxHeightDisplayed = 0;

    while (true) {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            currentTime - lastRefreshTime).count();

        if (elapsedMs >= refreshRateMs) {
            BYTE currentWidth = ReadMemory<BYTE>(processHandle, baseAddress + WIDTH_OFFSET);
            BYTE currentHeight = ReadMemory<BYTE>(processHandle, baseAddress + HEIGHT_OFFSET);
            bool sizeChanged = (currentWidth != lastWidth || currentHeight != lastHeight);
            mineField = GetMineField(processHandle, baseAddress, currentWidth, currentHeight);
            std::string currentDisplayString = CreateMineFieldDisplay(mineField, currentWidth, currentHeight);

            int currentDisplayHeight = 0;
            for (char c : currentDisplayString)
                if (c == '\n') currentDisplayHeight++;
            currentDisplayHeight++;

            maxHeightDisplayed = std::max(maxHeightDisplayed, currentDisplayHeight);

            if (sizeChanged && (currentHeight < lastHeight || currentWidth < lastWidth))
                ClearScreen();
            else
                SetConsoleCursorPosition(hConsole, { 0, 0 });

            std::cout << currentDisplayString;
            if (sizeChanged && currentDisplayHeight < maxHeightDisplayed) {
                CONSOLE_SCREEN_BUFFER_INFO csbi;
                GetConsoleScreenBufferInfo(hConsole, &csbi);
                for (int i = currentDisplayHeight; i < maxHeightDisplayed; i++) {
                    COORD linePos = { 0, static_cast<SHORT>(csbi.dwCursorPosition.Y - currentDisplayHeight + i) };
                    SetConsoleCursorPosition(hConsole, linePos);
                    CONSOLE_SCREEN_BUFFER_INFO csbiWidth;
                    GetConsoleScreenBufferInfo(hConsole, &csbiWidth);
                    std::string spaces(csbiWidth.dwSize.X, ' ');
                    std::cout << spaces;
                }
                SetConsoleCursorPosition(hConsole, csbi.dwCursorPosition);
            }
            std::cout << std::flush;
            lastDisplayString = std::move(currentDisplayString);
            lastWidth = currentWidth;
            lastHeight = currentHeight;
            lastRefreshTime = currentTime;
        }
        Sleep(10);
    }
    SetConsoleCursorInfo(hConsole, &originalCursorInfo);
}
