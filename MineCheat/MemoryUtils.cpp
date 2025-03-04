#include "MemoryUtils.h"
#include <cstring>

DWORD GetProcessIdByWindowName(const char* windowName) {
    HWND windowHandle = FindWindowA(nullptr, windowName);
    if (!windowHandle) return 0;
    DWORD processId;
    GetWindowThreadProcessId(windowHandle, &processId);
    return processId;
}

uintptr_t GetModuleBaseAddress(DWORD processId, const char* moduleName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(moduleEntry);
    uintptr_t moduleBaseAddress = 0;
    if (Module32First(hSnapshot, &moduleEntry)) {
        do {
            if (_stricmp(moduleEntry.szModule, moduleName) == 0) {
                moduleBaseAddress = reinterpret_cast<uintptr_t>(moduleEntry.modBaseAddr);
                break;
            }
        } while (Module32Next(hSnapshot, &moduleEntry));
    }
    CloseHandle(hSnapshot);
    return moduleBaseAddress;
}
