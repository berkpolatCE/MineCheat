#ifndef OVERLAY_H
#define OVERLAY_H

#include <Windows.h>
#include <vector>
#include "MinefieldMonitor.h"
#include "MemoryUtils.h"
#include <gdiplus.h>

class MinesweeperOverlay {
public:
    MinesweeperOverlay(HWND minesweeperWnd, HANDLE procHandle, uintptr_t baseAddr);
    ~MinesweeperOverlay();
    bool Initialize();
    void UpdatePosition();
    void UpdateMines();

private:
    HWND minesweeperWindow;
    HWND overlayWindow;
    HANDLE processHandle;
    uintptr_t baseAddress;
    std::vector<CellPosition> minePositions;
    int cellSize;
    BYTE mineWidth, mineHeight;
    POINT mineFieldTopLeft;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

DWORD WINAPI OverlayThreadProc(LPVOID lpParam);

#endif // OVERLAY_H
