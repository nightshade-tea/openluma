#ifndef OPENLUMA_DLL_INJECTION_H
#define OPENLUMA_DLL_INJECTION_H

#include <windows.h>

#define DLL_LOAD_TIMEOUT 10000 // ms

/* Inject a DLL into a target process.
 *
 * @param ProcessHandle - Handle to the target process
 * @param DllPath - Absolute path to the DLL
 *
 * @return TRUE if the injection succeeds, FALSE otherwise
 */
_Success_(return == TRUE)
BOOL
InjectDllIntoProcess(
    _In_ HANDLE ProcessHandle,
    _In_ LPCWSTR DllPath
);

#endif // !OPENLUMA_DLL_INJECTION_H