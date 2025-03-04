#include "Overlay.h"
#include "MinefieldMonitor.h"
#include <gdiplus.h>
#include <iostream>

#pragma comment(lib, "gdiplus.lib")

MinesweeperOverlay::MinesweeperOverlay(HWND minesweeperWnd, HANDLE procHandle, uintptr_t baseAddr)
    : minesweeperWindow(minesweeperWnd), processHandle(procHandle), baseAddress(baseAddr),
    overlayWindow(nullptr), cellSize(16)
{
    mineFieldTopLeft.x = 15;
    mineFieldTopLeft.y = 101;
}

MinesweeperOverlay::~MinesweeperOverlay() {
    if (overlayWindow)
        DestroyWindow(overlayWindow);
}

LRESULT CALLBACK MinesweeperOverlay::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MinesweeperOverlay* pThis = nullptr;
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<MinesweeperOverlay*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }
    else {
        pThis = reinterpret_cast<MinesweeperOverlay*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }
    return pThis ? pThis->HandleMessage(hwnd, uMsg, wParam, lParam) : DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT MinesweeperOverlay::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        HDC memDC = CreateCompatibleDC(hdc);
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        int width = clientRect.right - clientRect.left;
        int height = clientRect.bottom - clientRect.top;
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
        SelectObject(memDC, memBitmap);
        HBRUSH transparentBrush = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(memDC, &clientRect, transparentBrush);
        DeleteObject(transparentBrush);
        HPEN redPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
        HPEN oldPen = static_cast<HPEN>(SelectObject(memDC, redPen));
        for (const auto& pos : minePositions) {
            if (pos.isMine) {
                int x = mineFieldTopLeft.x + pos.x * cellSize;
                int y = mineFieldTopLeft.y + pos.y * cellSize;
                int margin = 2;
                MoveToEx(memDC, x + margin, y + margin, nullptr);
                LineTo(memDC, x + cellSize - margin, y + cellSize - margin);
                MoveToEx(memDC, x + cellSize - margin, y + margin, nullptr);
                LineTo(memDC, x + margin, y + cellSize - margin);
            }
        }
        SelectObject(memDC, oldPen);
        DeleteObject(redPen);
        BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);
        DeleteObject(memBitmap);
        DeleteDC(memDC);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

bool MinesweeperOverlay::Initialize() {
    WNDCLASSEXA wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "MinesweeperOverlayClass";
    if (!RegisterClassExA(&wc))
        return false;

    RECT minesweeperRect;
    if (!GetWindowRect(minesweeperWindow, &minesweeperRect))
        return false;

    overlayWindow = CreateWindowExA(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST,
        "MinesweeperOverlayClass",
        "Minesweeper Overlay",
        WS_POPUP,
        minesweeperRect.left, minesweeperRect.top,
        minesweeperRect.right - minesweeperRect.left,
        minesweeperRect.bottom - minesweeperRect.top,
        nullptr, nullptr, GetModuleHandle(NULL), this
    );
    if (!overlayWindow)
        return false;

    SetLayeredWindowAttributes(overlayWindow, RGB(0, 0, 0), 0, LWA_COLORKEY);
    ShowWindow(overlayWindow, SW_SHOWNOACTIVATE);
    UpdateWindow(overlayWindow);
    return true;
}

void MinesweeperOverlay::UpdatePosition() {
    if (minesweeperWindow && overlayWindow) {
        RECT rect;
        if (GetWindowRect(minesweeperWindow, &rect)) {
            SetWindowPos(overlayWindow, HWND_TOPMOST,
                rect.left, rect.top,
                rect.right - rect.left,
                rect.bottom - rect.top,
                SWP_NOACTIVATE | SWP_SHOWWINDOW);
        }
    }
}

void MinesweeperOverlay::UpdateMines() {
    mineWidth = ReadMemory<BYTE>(processHandle, baseAddress + WIDTH_OFFSET);
    mineHeight = ReadMemory<BYTE>(processHandle, baseAddress + HEIGHT_OFFSET);
    std::vector<std::vector<BYTE>> mineField = GetMineField(processHandle, baseAddress, mineWidth, mineHeight);
    minePositions.clear();
    for (int y = 0; y < mineHeight; y++) {
        for (int x = 0; x < mineWidth; x++) {
            BYTE cellValue = mineField[y][x];
            bool isMine = (cellValue == MINE || cellValue == MINE_FLAGGED || cellValue == MINE_QUESTION);
            minePositions.push_back({ x, y, isMine });
        }
    }
    InvalidateRect(overlayWindow, nullptr, TRUE);
}

DWORD WINAPI OverlayThreadProc(LPVOID lpParam) {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    struct OverlayParams {
        HWND minesweeperWindow;
        HANDLE processHandle;
        uintptr_t baseAddress;
    };

    OverlayParams* params = static_cast<OverlayParams*>(lpParam);
    MinesweeperOverlay overlay(params->minesweeperWindow, params->processHandle, params->baseAddress);
    if (!overlay.Initialize()) {
        delete params;
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return 1;
    }

    MSG msg;
    bool running = true;
    while (running) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        overlay.UpdatePosition();
        overlay.UpdateMines();
        Sleep(100);
        if (!IsWindow(params->minesweeperWindow))
            running = false;
    }
    delete params;
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return 0;
}
