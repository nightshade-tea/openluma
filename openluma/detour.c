#include "detour.h"
#include "dll_injection.h"
#include "properties.h"
#include <stdio.h>

#define STATUS_SUCCESS ((NTSTATUS) 0x0)

extern LPFNNTCREATEUSERPROCESS NT_CREATE_USER_PROCESS_TRAMPOLINE;
extern LPFNNTRESUMETHREAD NT_RESUME_THREAD_TRAMPOLINE;
extern LPPROPERTIES OPENLUMA_PROPERTIES;

HANDLE CHILD_PROCESS_HANDLE = NULL;
HANDLE CHILD_THREAD_HANDLE = NULL;

NTSTATUS
NTAPI
NtCreateUserProcessDetour(
    _Out_ PHANDLE ProcessHandle,
    _Out_ PHANDLE ThreadHandle,
    _In_ ACCESS_MASK ProcessDesiredAccess,
    _In_ ACCESS_MASK ThreadDesiredAccess,
    _In_opt_ PCOBJECT_ATTRIBUTES ProcessObjectAttributes,
    _In_opt_ PCOBJECT_ATTRIBUTES ThreadObjectAttributes,
    _In_ ULONG ProcessFlags,
    _In_ ULONG ThreadFlags,
    _In_opt_ PRTL_USER_PROCESS_PARAMETERS ProcessParameters,
    _Inout_ PPS_CREATE_INFO CreateInfo,
    _In_opt_ PPS_ATTRIBUTE_LIST AttributeList
)
{
    NTSTATUS status;

    status = (*NT_CREATE_USER_PROCESS_TRAMPOLINE)(
        ProcessHandle,
        ThreadHandle,
        ProcessDesiredAccess,
        ThreadDesiredAccess,
        ProcessObjectAttributes,
        ThreadObjectAttributes,
        ProcessFlags,
        ThreadFlags,
        ProcessParameters,
        CreateInfo,
        AttributeList
        );

    if (status == STATUS_SUCCESS && ProcessHandle && ThreadHandle) {
        CHILD_PROCESS_HANDLE = *ProcessHandle;
        CHILD_THREAD_HANDLE = *ThreadHandle;
    }

    return status;
}

NTSTATUS
NTAPI
NtResumeThreadDetour(
    _In_ HANDLE ThreadHandle,
    _Out_opt_ PULONG PreviousSuspendCount
)
{
    if (ThreadHandle && ThreadHandle == CHILD_THREAD_HANDLE) {
        if ( !(InjectDllIntoProcess(CHILD_PROCESS_HANDLE, OPENLUMA_PROPERTIES->uplayR1DllPath)) ) {
            fprintf(stderr, "InjectDllIntoProcess(): Failed to inject UPLAY_R1 dll\n");
            exit(1);
        }

        if ( !(InjectDllIntoProcess(CHILD_PROCESS_HANDLE, OPENLUMA_PROPERTIES->openlumaDllPath)) ) {
            fprintf(stderr, "InjectDllIntoProcess(): Failed to inject OPENLUMA dll\n");
            exit(1);
        }

        CHILD_PROCESS_HANDLE = NULL;
        CHILD_THREAD_HANDLE = NULL;
    }

    return (*NT_RESUME_THREAD_TRAMPOLINE)(
        ThreadHandle,
        PreviousSuspendCount
        );
}

#undef STATUS_SUCCESS