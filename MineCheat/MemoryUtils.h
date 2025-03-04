#ifndef MEMORY_UTILS_H
#define MEMORY_UTILS_H

#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>

DWORD GetProcessIdByWindowName(const char* windowName);
uintptr_t GetModuleBaseAddress(DWORD processId, const char* moduleName);

template<typename T>
inline T ReadMemory(HANDLE processHandle, uintptr_t address) {
    T value;
    ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(address), &value, sizeof(T), nullptr);
    return value;
}

#endif // MEMORY_UTILS_H
