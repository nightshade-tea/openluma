#include "dll_injection.h"
#include <stdio.h>

#define FAIL(...) \
    fprintf(stderr, __VA_ARGS__); \
    goto cleanup;

_Success_(return == TRUE)
BOOL
InjectDllIntoProcess(
    _In_ HANDLE ProcessHandle,
    _In_ LPCWSTR DllPath
)
{
    LPVOID allocatedMemory = NULL;
    HANDLE thread = NULL;
    BOOL status = FALSE;

    SIZE_T dllPathLength;
    SIZE_T allocationSize;
    HMODULE kernel32;
    LPTHREAD_START_ROUTINE loadLibraryW;
    DWORD dw;

    if (!ProcessHandle || !DllPath) {
        FAIL("InjectDllIntoProcess(): Invalid parameters\n")
    }

    if ( !(dllPathLength = (SIZE_T) lstrlen(DllPath)) ) {
        FAIL("lstrlen(): 'DllPath' length is 0\n")
    }

    allocationSize = 2 * (dllPathLength + 1);

    if ( !(allocatedMemory = VirtualAllocEx(ProcessHandle, NULL, allocationSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)) ) {
        FAIL("VirtualAllocEx(): Failed to allocate memory in the target process\n")
    }

    if ( !(WriteProcessMemory(ProcessHandle, allocatedMemory, DllPath, allocationSize, NULL)) ) {
        FAIL("WriteProcessMemory(): Failed to write memory in the target process\n")
    }

    if ( !(kernel32 = GetModuleHandle(L"kernel32.dll")) ) {
        FAIL("GetModuleHandle(): Failed to get handle for 'kernel32.dll'\n")
    }

    if ( !(loadLibraryW = (LPTHREAD_START_ROUTINE) GetProcAddress(kernel32, "LoadLibraryW")) ) {
        FAIL("GetProcAddress(): Failed to get the address for 'LoadLibraryW()' from 'kernel32.dll'\n")
    }

    if ( !(thread = CreateRemoteThread(ProcessHandle, NULL, 0, loadLibraryW, allocatedMemory, 0, NULL)) ) {
        FAIL("CreateRemoteThread(): Failed to create thread in the target process\n")
    }

    if ( (dw = WaitForSingleObject(thread, DLL_LOAD_TIMEOUT)) == WAIT_TIMEOUT ) {
        FAIL("WaitForSingleObject(): Thread timed out after %d ms ( LoadLibraryW() thread )\n", DLL_LOAD_TIMEOUT)
    }

    if ( dw == WAIT_FAILED ) {
        FAIL("WaitForSingleObject(): WAIT_FAILED ( LoadLibraryW() thread )\n")
    }

    status = TRUE;

cleanup:
    if (allocatedMemory) {
        VirtualFreeEx(ProcessHandle, allocatedMemory, 0, MEM_RELEASE);
    }

    if (thread) {
        CloseHandle(thread);
    }

    return status;
}

#undef FAIL