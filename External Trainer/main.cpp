#include <cwchar>
#include <iostream>
#include <vector>

#include "Process.h"

template <typename T>
std::ostream& operator<<(std::ostream& output, std::vector<T> const& values) {
  for (auto const& value : values) {
    output << value << std::endl;
  }
  return output;
}

int main() { 
  const wchar_t* processName = L"ac_client.exe";
  DWORD processID = GetProcessID(processName);
  if (processID == 0) {
    wprintf(L"Failed to get process ID for process: %ls\n", processName);
    return 1;
  }

  const wchar_t* moduleName = L"ac_client.exe";
  uintptr_t moduleBaseAddress = GetModuleBaseAddress(processID, moduleName);
  if (moduleBaseAddress == NULL) {
    wprintf(L"Failed to get module base address for module %ls in process ID: %lu\n",
            moduleName, processID);
    return 1;
  }

  HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, /*bInheritHandle=*/FALSE, processID);
  if (hProcess == NULL) {
    printf("Failed to open process with ID: %lu\n", processID);
    return 1;
  }

  // NOTE: Offset values were determined by using Cheat Engine to track the 'ammo' value
  // by firing a few times and scanning until we determined the actual address. Then you
  // can iteratively check for 'what writes to this register' (to get the first pointer
  // address) and then checking 'what accesses this value' and scanning up the pointer
  // chain while recording each offset.
  //
  // For example:
  //    Ammo address found to be: 008005D0 (may change)
  //    - Check the value that writes to this. It's just a dereference of 008005D0,
  //      so the offset is 0
  //    - Search to see what address contains this value (e.g. 007F561C)
  //    - 007F5608 + 14, so offset is 14
  //    Repeat until you find a green static address
  //    - Search to see what address contains value 007F5608 (e.g. 008007F4)
  //    - 00800490 + 364, so offset is 364
  //    Repeat again
  //    - Search to see what address contains value 00800490 (e.g. ac_client.exe+18AC00)
  //    - You'll find a few static addresses offset from the EXE. The middle one
  //      happens to be for single+multiplayer, so use that. But it requires trial
  //      and error
  //    
  //    008005D0 + 0 -> 008005D0
  //    [007F5608 + 14] -> 008005D0
  //    [00800490 + 364] -> 007F5608
  //    static address found: ac_client.exe+18AC00 -> 00800490
  uintptr_t initialAddress = moduleBaseAddress + 0x18AC00;  // e.g. ac_client.exe+18AC00
  std::vector<unsigned int> offsets = {0x364, 0x14, 0x0};
  uintptr_t address = FindDynamicMemoryAllocationAddress(hProcess, initialAddress, offsets);
  if (address == NULL) {
    std::cout << "Failed to find dynamic memory address in process ("
              << hProcess
              << ") at module base address(" << moduleBaseAddress
              << ") with the following offsets: " << offsets;
    return 1;
  }

  printf("Success! Found address: %p\n", ((void*)address));

  int value = 0;
  ReadProcessMemory(hProcess, (BYTE *)address, &value, sizeof(value), /*lpNumberOfBytesRead=*/nullptr);
  printf("Current value = %d\n", value);

  int updatedValue = 25;
  WriteProcessMemory(hProcess, (BYTE *)address, &updatedValue, sizeof(updatedValue), /*lpNumberOfBytesWritten=*/nullptr);
  
  ReadProcessMemory(hProcess, (BYTE *)address, &value, sizeof(value), /*lpNumberOfBytesRead=*/nullptr);
  printf("Updated value = %d\n", value);

  return 0;
}