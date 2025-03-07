#ifndef OPENLUMA_DETOUR_H
#define OPENLUMA_DETOUR_H

#include <windows.h>

/* typedef undocumented types as LPVOID */
typedef LPVOID PCOBJECT_ATTRIBUTES;
typedef LPVOID PRTL_USER_PROCESS_PARAMETERS;
typedef LPVOID PPS_CREATE_INFO;
typedef LPVOID PPS_ATTRIBUTE_LIST;

/* Function pointer types for the trampoline functions */
typedef NTSTATUS (NTAPI* LPFNNTCREATEUSERPROCESS)(
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
    );

typedef NTSTATUS(NTAPI* LPFNNTRESUMETHREAD)(
    _In_ HANDLE ThreadHandle,
    _Out_opt_ PULONG PreviousSuspendCount
    );

/* Detour function for ntdll's NtCreateUserProcess().
 * Captures handles of newly created processes.
 */
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
);

/* Detour function for ntdll's NtResumeThread().
 * Injects DLLs into the target process before allowing the thread to resume execution.
 */
NTSTATUS
NTAPI
NtResumeThreadDetour(
    _In_ HANDLE ThreadHandle,
    _Out_opt_ PULONG PreviousSuspendCount
);

#endif // !OPENLUMA_DETOUR_H