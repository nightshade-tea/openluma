#include "properties.h"
#include <shlwapi.h>
#include <pathcch.h>

#define _PR_STATUS_MUST_CHECK_ \
    _Success_(return == PR_OK) \
    _Must_inspect_result_

/* Sets the 32-bit flag based on the .ini file. */
static
_PR_STATUS_MUST_CHECK_
PR_STATUS
setIs32Bits(
    _Out_ LPPROPERTIES lpProps
)
{
    if (!lpProps) {
        return PR_ERROR_INVALID_PARAMETERS;
    }

    lpProps->is32Bits = GetPrivateProfileInt(
        INI_DEFAULT_SECTION,
        INI_IS32BITS_KEY,
        INI_IS32BITS_KEY_DEFAULT_VALUE,
        OPENLUMA_INI
    );

    return PR_OK;
}

/* Sets the current process directory path (where openluma.exe is).
 * Used as a base path for resolving relative paths.
 */
static
_PR_STATUS_MUST_CHECK_
PR_STATUS
setCurrentProcDirPath(
    _Out_ LPPROPERTIES lpProps
)
{
    if (!lpProps) {
        return PR_ERROR_INVALID_PARAMETERS;
    }

    if ( !(GetModuleFileName(NULL, lpProps->currentProcDirPath, MAX_PATH)) ) {
        return PR_ERROR_SET_CURRENTPROCDIRPATH;
    }

    if ( PathCchRemoveFileSpec(lpProps->currentProcDirPath, MAX_PATH) != S_OK ) {
        return PR_ERROR_SET_CURRENTPROCDIRPATH;
    }

    return PR_OK;
}

/* Helper function to load and validate paths from .ini file.
 * Converts relative paths to absolute paths and verifies file existence.
 */
static
_PR_STATUS_MUST_CHECK_
PR_STATUS
setPathFromIni(
    _In_ LPPROPERTIES lpProps,
    _In_ LPCWSTR Section,
    _In_ LPCWSTR Key,
    _In_opt_ LPCWSTR DefaultValue,
    _Out_writes_(Size) LPWSTR Destination,
    _In_ DWORD Size,
    _In_ PR_STATUS ErrorCode
)
{
    DWORD dw;
    HRESULT hr;

    if (!lpProps) {
        return PR_ERROR_INVALID_PARAMETERS;
    }

    dw = GetPrivateProfileString(
        Section,
        Key,
        DefaultValue,
        Destination,
        Size,
        OPENLUMA_INI
    );

    if (!dw) {
        return ErrorCode;
    }

    hr = PathCchCombine(
        Destination,
        Size,
        lpProps->currentProcDirPath,
        Destination
    );

    if ( (hr != S_OK) || !(PathFileExists(Destination)) ) {
        return ErrorCode;
    }

    return PR_OK;
}

/* Sets the uplay_r1 DLL path from .ini file. */
static
_PR_STATUS_MUST_CHECK_
PR_STATUS
setUplayR1DllPath(
    _Inout_ LPPROPERTIES lpProps
)
{
    if (!lpProps) {
        return PR_ERROR_INVALID_PARAMETERS;
    }

    return setPathFromIni(
        lpProps,
        INI_DEFAULT_SECTION,
        INI_UPLAYR1DLLPATH_KEY,
        ( (lpProps->is32Bits) ? UPLAY_R1_DLL_32 : UPLAY_R1_DLL_64 ),
        lpProps->uplayR1DllPath,
        MAX_PATH,
        PR_ERROR_SET_UPLAYR1DLLPATH
    );
}

/* Sets the openluma DLL path from .ini file. */
static
_PR_STATUS_MUST_CHECK_
PR_STATUS
setOpenlumaDllPath(
    _Inout_ LPPROPERTIES lpProps
)
{
    if (!lpProps) {
        return PR_ERROR_INVALID_PARAMETERS;
    }

    return setPathFromIni(
        lpProps,
        INI_DEFAULT_SECTION,
        INI_OPENLUMADLLPATH_KEY,
        ( (lpProps->is32Bits) ? OPENLUMA_DLL_32 : OPENLUMA_DLL_64 ),
        lpProps->openlumaDllPath,
        MAX_PATH,
        PR_ERROR_SET_OPENLUMADLLPATH
    );
}

/* Sets the game executable path from .ini file. */
static
_PR_STATUS_MUST_CHECK_
PR_STATUS
setGameExePath(
    _Inout_ LPPROPERTIES lpProps
)
{
    if (!lpProps) {
        return PR_ERROR_INVALID_PARAMETERS;
    }

    return setPathFromIni(
        lpProps,
        INI_DEFAULT_SECTION,
        INI_GAMEEXEPATH_KEY,
        INI_GAMEEXEPATH_KEY_DEFAULT_VALUE,
        lpProps->gameExePath,
        MAX_PATH,
        PR_ERROR_SET_GAMEEXEPATH
    );
}

#define RETURN_ON_ERROR(fn) \
    if ( (status = (fn)) != PR_OK ) { \
        return status; \
    }

/* Read all properties from .ini file. */
static
_PR_STATUS_MUST_CHECK_
PR_STATUS
readIniIntoProps(
    _Inout_ LPPROPERTIES lpProps
)
{
    PR_STATUS status;

    RETURN_ON_ERROR(setIs32Bits(lpProps));
    RETURN_ON_ERROR(setCurrentProcDirPath(lpProps));
    RETURN_ON_ERROR(setUplayR1DllPath(lpProps));
    RETURN_ON_ERROR(setOpenlumaDllPath(lpProps));
    RETURN_ON_ERROR(setGameExePath(lpProps));

    return PR_OK;
}

#undef RETURN_ON_ERROR

_Success_(return == PR_OK)
PR_STATUS
PR_Load(
    _Out_ LPPROPERTIES* plpProps
)
{
    PR_STATUS status;

    if (!plpProps) {
        return PR_ERROR_INVALID_PARAMETERS;
    }

    *plpProps = NULL;

    if ( !(PathFileExists(OPENLUMA_INI)) ) {
        return PR_ERROR_INI_DOESNT_EXIST;
    }

    if ( !(*plpProps = malloc(sizeof(PROPERTIES))) ) {
        return PR_ERROR_MALLOC;
    }

    ZeroMemory(*plpProps, sizeof(PROPERTIES));

    if ( (status = readIniIntoProps(*plpProps)) != PR_OK ) {
        PR_Free(plpProps);
        return status;
    }

    return PR_OK;
}

VOID
PR_Free(
    _Inout_ LPPROPERTIES* plpProps
)
{
    if (plpProps && *plpProps) {
        free(*plpProps);
        *plpProps = NULL;
    }

    return;
}

#define RETURN_CASE_STR(x) \
    case x: \
        return #x;

LPCSTR
PR_StatusToString(
    _In_ PR_STATUS Status
)
{
    switch (Status) {
        RETURN_CASE_STR(PR_OK)
        RETURN_CASE_STR(PR_ERROR_INVALID_PARAMETERS)
        RETURN_CASE_STR(PR_ERROR_INI_DOESNT_EXIST)
        RETURN_CASE_STR(PR_ERROR_MALLOC)
        RETURN_CASE_STR(PR_ERROR_SET_CURRENTPROCDIRPATH)
        RETURN_CASE_STR(PR_ERROR_SET_UPLAYR1DLLPATH)
        RETURN_CASE_STR(PR_ERROR_SET_OPENLUMADLLPATH)
        RETURN_CASE_STR(PR_ERROR_SET_GAMEEXEPATH)
    }

    return "PR_ERROR_UNKNOWN";
}

#undef RETURN_CASE_STR

// TODO: use the defines to refer to filenames and keys.
//       either return LPCWSTR or find a way to convert them using macros!!

LPCSTR
PR_StatusToErrorMsg(
    _In_ PR_STATUS Status
)
{
    switch (Status) {
    case PR_OK:
        return "Function executed successfully";
    case PR_ERROR_INVALID_PARAMETERS:
        return "Function was called with invalid parameters (NULL ptr) !";
    case PR_ERROR_INI_DOESNT_EXIST:
        return "'.\\openluma\\openluma.ini' does not exist !";
    case PR_ERROR_MALLOC:
        return "A memory allocation request failed !";
    case PR_ERROR_SET_CURRENTPROCDIRPATH:
        return "Failed to get current process directory path !";
    case PR_ERROR_SET_UPLAYR1DLLPATH:
        return "Failed to read 'uplayR1Dll' key from .ini ! Check if the option is set and the file exists";
    case PR_ERROR_SET_OPENLUMADLLPATH:
        return "Failed to read 'openlumaDll' key from .ini ! Check if the option is set and the file exists";
    case PR_ERROR_SET_GAMEEXEPATH:
        return "Failed to read 'gameExe' key from .ini ! Check if the option is set and the file exists";
    }

    return "Unknown error !";
}

#undef _PR_STATUS_MUST_CHECK_