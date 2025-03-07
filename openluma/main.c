#include <stdio.h>
#include <stdarg.h>
#include <minhook.h>
#include "detour.h"
#include "properties.h"

/* Globals for the detour fns */
LPFNNTCREATEUSERPROCESS NT_CREATE_USER_PROCESS_TRAMPOLINE = NULL;
LPFNNTRESUMETHREAD NT_RESUME_THREAD_TRAMPOLINE = NULL;
LPPROPERTIES OPENLUMA_PROPERTIES = NULL;

/* Free allocated memory and uninitialize minhook. */
static
void
cleanup(
    void
)
{
    PR_Free(&OPENLUMA_PROPERTIES);
    MH_Uninitialize();

    return;
}

/* Print message to stderr and exit.
 *
 * @param format - Format string for fprintf()
 * @param ... - Variable arguments for fprintf()
 */
static
__declspec(noreturn)
void
fatal(
    const char *const format,
    ...
)
{
    va_list args;

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    cleanup();

    exit(1);
}

/* Load properties from .ini file and initialize minhook. */
static
void
init(
    void
)
{
    PR_STATUS prstatus;
    MH_STATUS mhstatus;

    if ( (prstatus = PR_Load(&OPENLUMA_PROPERTIES)) != PR_OK ) {
        fatal("PR_Load() failed: %s (%s)\n", PR_StatusToErrorMsg(prstatus), PR_StatusToString(prstatus));
    }

    if ( (mhstatus = MH_Initialize()) != MH_OK ) {
        fatal("MH_Initialize() failed: %s\n", MH_StatusToString(mhstatus));
    }

    return;
}

/* Hook NtCreateUserProcess() and NtResumeThread(). */
static
void
setup_hooks(
    void
)
{
    MH_STATUS mhstatus;
    HMODULE ntdll;
    LPFNNTCREATEUSERPROCESS ntCreateUserProcess;
    LPFNNTRESUMETHREAD ntResumeThread;

    if ( !(ntdll = GetModuleHandle(L"ntdll.dll")) ) {
        fatal("GetModuleHandleA(): Failed to load ntdll.dll\n");
    }

    if ( !(ntCreateUserProcess = (LPFNNTCREATEUSERPROCESS) GetProcAddress(ntdll, "NtCreateUserProcess")) ) {
        fatal("GetProcAddress(): Failed to load NtCreateUserProcess from ntdll.dll\n");
    }

    if ( (mhstatus = MH_CreateHook(ntCreateUserProcess, NtCreateUserProcessDetour, (LPVOID*) &NT_CREATE_USER_PROCESS_TRAMPOLINE)) != MH_OK ) {
        fatal("MH_CreateHook() failed to hook NtCreateUserProcess: %s\n", MH_StatusToString(mhstatus));
    }

    if ( !(ntResumeThread = (LPFNNTRESUMETHREAD) GetProcAddress(ntdll, "NtResumeThread")) ) {
        fatal("GetProcAddress(): Failed to load NtResumeThread from ntdll.dll\n");
    }

    if ( (mhstatus = MH_CreateHook(ntResumeThread, NtResumeThreadDetour, (LPVOID*) &NT_RESUME_THREAD_TRAMPOLINE)) != MH_OK ) {
        fatal("MH_CreateHook() failed to hook NtResumeThread: %s\n", MH_StatusToString(mhstatus));
    }

    if ( (mhstatus = MH_EnableHook(MH_ALL_HOOKS)) != MH_OK ) {
        fatal("MH_EnableHook() failed: %s\n", MH_StatusToString(mhstatus));
    }

    return;
}

/* Start the game executable. */
static
void
launch_game(
    void
)
{
    STARTUPINFO startupInfo;
    PROCESS_INFORMATION procInfo;

    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    ZeroMemory(&procInfo, sizeof(procInfo));

    if ( !CreateProcess(
        OPENLUMA_PROPERTIES->gameExePath,
        NULL,
        NULL,
        NULL,
        0,
        0,
        NULL,
        NULL,
        &startupInfo,
        &procInfo
        )
    ) {
        fatal("CreateProcess(): Failed to start gameExe\n");
    }

    CloseHandle(procInfo.hProcess);
    CloseHandle(procInfo.hThread);

    return;
}

int
main(
    void
)
{
    init();
    setup_hooks();
    launch_game();
    cleanup();

    return 0;
}