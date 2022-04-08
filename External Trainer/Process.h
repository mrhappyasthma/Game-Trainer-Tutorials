#pragma once

#include <vector>
#include <Windows.h>

/**
 * Snapshots the current processes running on the system and iterates them
 * to find one matching `processName`. Returns the process ID matching
 * `processName` or 0 if it cannot be found.
 */
DWORD GetProcessID(const wchar_t *processName);

/**
 * Snapshots the modules (64 bit and 32 bit) for the given `processID` and
 * iterates them to find a module matching `moduleName`. Returns the base
 * address of that module or NULL if it cannot be found.
 */
uintptr_t GetModuleBaseAddress(DWORD processID, const wchar_t *moduleName);

/**
 * Traverses nested pointers in a given process, specified by `hProcess`,
 * starting from `baseAddress` and iteratively applying each `offset` using
 * pointer arithmetic. Returns the final address or NULL if any errors occurred.
 */
uintptr_t FindDynamicMemoryAllocationAddress(HANDLE hProcess,
                                             uintptr_t baseAddress,
                                             std::vector<unsigned int> offsets);