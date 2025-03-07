#ifndef OPENLUMA_PROPERTIES_H
#define OPENLUMA_PROPERTIES_H

#include <windows.h>

#define OPENLUMA_DIR    L".\\" L"openluma"                   // Base directory for opeluma files
#define OPENLUMA_INI    OPENLUMA_DIR L"\\" L"openluma.ini"   // Configuration file path

#define UPLAY_R1_DLL_32 OPENLUMA_DIR L"\\" L"uplay_r1.dll"   // Default path for uplay_r1.dll
#define UPLAY_R1_DLL_64 OPENLUMA_DIR L"\\" L"uplay_r164.dll" // Default path for uplay_r164.dll
#define OPENLUMA_DLL_32 OPENLUMA_DIR L"\\" L"openluma.dll"   // Default path for openluma.dll
#define OPENLUMA_DLL_64 OPENLUMA_DIR L"\\" L"openluma64.dll" // Default path for openluma64.dll

/* INI sections and keys */
#define INI_DEFAULT_SECTION               L"Openluma"
#define INI_IS32BITS_KEY                  L"is32Bits"
#define INI_IS32BITS_KEY_DEFAULT_VALUE    0
#define INI_UPLAYR1DLLPATH_KEY            L"uplayR1Dll"
#define INI_OPENLUMADLLPATH_KEY           L"openlumaDll"
#define INI_GAMEEXEPATH_KEY               L"gameExe"
#define INI_GAMEEXEPATH_KEY_DEFAULT_VALUE NULL

/* Error codes */
typedef enum _PR_STATUS {
    PR_OK = 0,                       // Success
    PR_ERROR_INVALID_PARAMETERS,     // NULL pointer or invalid parameters
    PR_ERROR_INI_DOESNT_EXIST,       // Configuration file does not exist
    PR_ERROR_MALLOC,                 // Memory allocation failed
    PR_ERROR_SET_CURRENTPROCDIRPATH, // Failed to get current process directory
    PR_ERROR_SET_UPLAYR1DLLPATH,     // Failed to get valid uplay_r1 DLL path
    PR_ERROR_SET_OPENLUMADLLPATH,    // Failed to get valid openluma DLL path
    PR_ERROR_SET_GAMEEXEPATH         // Failed to get valid game executable path
} PR_STATUS;

/* Runtime configuration */
typedef struct _PROPERTIES {
    BOOL is32Bits;                      // Use 32-bit (1) or 64-bit (0) DLLs
    WCHAR currentProcDirPath[MAX_PATH]; // Directory containing openluma.exe
    WCHAR uplayR1DllPath[MAX_PATH];     // Full path to the uplay_r1 DLL
    WCHAR openlumaDllPath[MAX_PATH];    // Full path to the openluma DLL
    WCHAR gameExePath[MAX_PATH];        // Full path to the target game executable
} PROPERTIES, *LPPROPERTIES;

/* Load properties from INI file into new PROPERTIES structure.
 * This function allocates the memory for the PROPERTIES structure that should be freed with PR_Free().
 *
 * @param plpProps - Pointer to receive the allocated PROPERTIES structure
 *
 * @return PR_OK on success, error code otherwise
 */
_Success_(return == PR_OK)
PR_STATUS
PR_Load(
    _Out_ LPPROPERTIES* plpProps
);

/* Free PROPERTIES structure and set it's pointer to NULL.
 *
 * @param plpProps - Pointer to PROPERTIES structure to free
 */
VOID
PR_Free(
    _Inout_ LPPROPERTIES* plpProps
);

/* Convert status code to string representation.
 * The returned string does NOT need to be freed by the caller.
 *
 * @param Status - Status code to convert
 *
 * @return String representation of status code
 */
LPCSTR
PR_StatusToString(
    _In_ PR_STATUS Status
);

/* Convert status code to human-readable error message.
 * The returned string does NOT need to be freed by the caller.
 *
 * @param Status - Status code to convert
 *
 * @return Human-readable error message
 */
LPCSTR
PR_StatusToErrorMsg(
    _In_ PR_STATUS Status
);

#endif // !OPENLUMA_PROPERTIES_H