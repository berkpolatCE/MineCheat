#include "MemoryUtils.h"
#include "MinefieldMonitor.h"
#include "Overlay.h"
#include <iostream>
#include <Windows.h>

int main() {
    HWND minesweeperWindow = FindWindowA(nullptr, "Minesweeper");
    if (!minesweeperWindow) {
        std::cerr << "Failed to find Minesweeper window" << std::endl;
        return 1;
    }

    DWORD processId;
    GetWindowThreadProcessId(minesweeperWindow, &processId);
    std::cout << "Process ID: " << processId << std::endl;

    uintptr_t baseAddress = GetModuleBaseAddress(processId, "WINMINE.EXE");
    if (!baseAddress) {
        std::cerr << "Failed to get module base address" << std::endl;
        return 1;
    }
    std::cout << "Module base address: 0x" << std::hex << baseAddress << std::dec << std::endl;

    HANDLE processHandle = OpenProcess(PROCESS_VM_READ, FALSE, processId);
    if (!processHandle) {
        std::cerr << "Failed to open process. Error code: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "Starting Minesweeper monitor..." << std::endl;
    std::cout << "1 - Console monitor only" << std::endl;
    std::cout << "2 - Console monitor + Overlay" << std::endl;
    std::cout << "Select option: ";
    int option;
    std::cin >> option;
    std::cin.ignore();
    std::cout << "Press Enter to begin..." << std::endl;
    std::cin.get();

    if (option == 2) {
        struct OverlayParams {
            HWND minesweeperWindow;
            HANDLE processHandle;
            uintptr_t baseAddress;
        };

        auto params = new OverlayParams;
        params->minesweeperWindow = minesweeperWindow;
        params->processHandle = processHandle;
        params->baseAddress = baseAddress;

        HANDLE overlayThread = CreateThread(nullptr, 0, OverlayThreadProc, params, 0, nullptr);
        if (!overlayThread)
            std::cerr << "Failed to create overlay thread" << std::endl;
    }

    try {
        MonitorMineField(processHandle, baseAddress, 100);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown error occurred during monitoring" << std::endl;
    }

    CloseHandle(processHandle);
    return 0;
}
