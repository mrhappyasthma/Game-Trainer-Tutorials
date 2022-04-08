#include "Process.h"

#include <stdio.h>
#include <vector>
#include <tlhelp32.h>
#include <Windows.h>

// https://docs.microsoft.com/en-us/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes
DWORD GetProcessID(const wchar_t *processName) {
  // Snapshot all processes.
  HANDLE hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, /*th32ProcessID=*/0);
  if (hProcessSnapshot == INVALID_HANDLE_VALUE) {
    printf("CreateToolhelp32Snapshot failed.\n");
    return 0;
  }

  PROCESSENTRY32 processEntry;
  processEntry.dwSize = sizeof(processEntry);

  // Retrieve the first process information
  if (!Process32First(hProcessSnapshot, &processEntry)) {
    printf("Process32First failed to retrieve process info.\n");
    CloseHandle(hProcessSnapshot);
    return 0;
  }

  // Iterate all app processes looking for `processName` to grab its process ID.
  DWORD processID = 0;
  do {
    if (_wcsicmp(processEntry.szExeFile, processName) == 0) {
      processID = processEntry.th32ProcessID;
      break;
    }
  } while (Process32Next(hProcessSnapshot, &processEntry));

  CloseHandle(hProcessSnapshot);
  return processID;
}

uintptr_t GetModuleBaseAddress(DWORD processID, const wchar_t *moduleName) {
  if (processID == 0) {
    return NULL;
  }

  // Snapshot the modules for the given `processID`.
  HANDLE hModulesSnapshot =
      CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);
  if (hModulesSnapshot == INVALID_HANDLE_VALUE) {
    printf("CreateToolhelp32Snapshot failed.\n");
    return NULL;
  }

  MODULEENTRY32 moduleEntry;
  moduleEntry.dwSize = sizeof(moduleEntry);

  // Retrieve the first module information
  if (!Module32First(hModulesSnapshot, &moduleEntry)) {
    printf("Module32First failed to retrieve process info.\n");
    CloseHandle(hModulesSnapshot);
    return NULL;
  }

  // Iterate all modules looking for `moduleName` to grab its base address.
  BYTE *moduleBaseAddress = 0;
  do {
    if (_wcsicmp(moduleEntry.szModule, moduleName) == 0) {
      moduleBaseAddress = moduleEntry.modBaseAddr;
      break;
    }
  } while (Module32First(hModulesSnapshot, &moduleEntry));

  CloseHandle(hModulesSnapshot);
  return (uintptr_t)moduleBaseAddress;
}

uintptr_t FindDynamicMemoryAllocationAddress(HANDLE hProcess, uintptr_t baseAddress, std::vector<unsigned int> offsets) {
  uintptr_t address = baseAddress;
  for (unsigned int i = 0; i < offsets.size(); i++) {
    BOOL success = ReadProcessMemory(hProcess, (LPCVOID)address, &address, sizeof(address),
                                    /*lpNumberOfBytesRead=*/NULL);
    if (!success) {
      printf("Could not read address (%p) from process (%p)\n", (void*)address, hProcess);
      return NULL;
    }
    address += offsets[i];
  }
  return address;
}