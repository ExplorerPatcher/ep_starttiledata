#pragma once

#include <winnt.h>

EXTERN_C_START

NTSYSAPI
NTSTATUS
NTAPI
RtlGetDaclSecurityDescriptor(
    _In_ PSECURITY_DESCRIPTOR SecurityDescriptor,
    _Out_ PBOOLEAN DaclPresent,
    _Out_ PACL *Dacl,
    _Out_ PBOOLEAN DaclDefaulted
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryInformationAcl(
    _In_ PACL Acl,
    _Out_writes_bytes_(AclInformationLength) PVOID AclInformation,
    _In_ ULONG AclInformationLength,
    _In_ ACL_INFORMATION_CLASS AclInformationClass
    );

NTSYSAPI
ULONG
NTAPI
RtlLengthSid(
    _In_ PSID Sid
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateAcl(
    _Out_writes_bytes_(AclLength) PACL Acl,
    _In_ ULONG AclLength,
    _In_ ULONG AclRevision
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGetAce(
    _In_ PACL Acl,
    _In_ ULONG AceIndex,
    _Outptr_ PVOID *Ace
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAce(
    _Inout_ PACL Acl,
    _In_ ULONG AceRevision,
    _In_ ULONG StartingAceIndex,
    _In_reads_bytes_(AceListLength) PVOID AceList,
    _In_ ULONG AceListLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAccessAllowedAce(
    _Inout_ PACL Acl,
    _In_ ULONG AceRevision,
    _In_ ACCESS_MASK AccessMask,
    _In_ PSID Sid
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetDaclSecurityDescriptor(
    _Inout_ PSECURITY_DESCRIPTOR SecurityDescriptor,
    _In_ BOOLEAN DaclPresent,
    _In_opt_ PACL Dacl,
    _In_opt_ BOOLEAN DaclDefaulted
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateSecurityDescriptor(
    _Out_ PSECURITY_DESCRIPTOR SecurityDescriptor,
    _In_ ULONG Revision
    );

_Kernel_entry_
NTSYSCALLAPI
NTSTATUS
NTAPI
NtQuerySecurityObject(
    _In_ HANDLE Handle,
    _In_ SECURITY_INFORMATION SecurityInformation,
    _Out_writes_bytes_to_opt_(Length, *LengthNeeded) PSECURITY_DESCRIPTOR SecurityDescriptor,
    _In_ ULONG Length,
    _Out_ PULONG LengthNeeded
    );

_Kernel_entry_
NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetSecurityObject(
    _In_ HANDLE Handle,
    _In_ SECURITY_INFORMATION SecurityInformation,
    _In_ PSECURITY_DESCRIPTOR SecurityDescriptor
    );

EXTERN_C_END

inline HRESULT GetTokenInformationHelper(HANDLE token, TOKEN_INFORMATION_CLASS infoClass, void** tokenInfo)
{
    DWORD byteCount;
    RETURN_HR_IF(E_UNEXPECTED, GetTokenInformation(token, TokenUser, nullptr, 0, &byteCount)); // 30

    DWORD lastError = GetLastError();
    if (lastError == ERROR_INSUFFICIENT_BUFFER)
    {
        wil::unique_hlocal tokenInfoInternal;
        tokenInfoInternal.reset(LocalAlloc(0, byteCount));
        RETURN_IF_NULL_ALLOC(tokenInfoInternal); // 17

        RETURN_IF_WIN32_BOOL_FALSE(GetTokenInformation(token, infoClass, tokenInfoInternal.get(), byteCount, &byteCount)); // 19

        *tokenInfo = tokenInfoInternal.release();
    }
    else
    {
        RETURN_IF_WIN32_ERROR(lastError); // 25
    }

    return S_OK;
}

inline HRESULT BuildUserSD(PSECURITY_DESCRIPTOR oldSD, PSID userSid, ACCESS_MASK flags, bool* exists, PSECURITY_DESCRIPTOR newSD)
{
    BOOLEAN daclPresent;
    PACL dacl = nullptr;
    BOOLEAN daclDefaulted;
    RETURN_IF_NTSTATUS_FAILED(RtlGetDaclSecurityDescriptor(oldSD, &daclPresent, &dacl, &daclDefaulted)); // 41

    for (DWORD i = 0; i < dacl->AceCount; ++i)
    {
        ACCESS_ALLOWED_ACE* aceHeader = nullptr;
        if (GetAce(dacl, i, (LPVOID*)&aceHeader) && aceHeader->Header.AceType == ACCESS_ALLOWED_ACE_TYPE
            && EqualSid(&aceHeader->SidStart, userSid) && (flags & aceHeader->Mask) == flags)
        {
            *exists = true;
            return S_OK;
        }
    }

    ACL_SIZE_INFORMATION aclSizeInfo;
    RETURN_IF_NTSTATUS_FAILED(RtlQueryInformationAcl(dacl, &aclSizeInfo, sizeof(aclSizeInfo), AclSizeInformation)); // 61

    ACL_REVISION_INFORMATION aclRevisionInfo;
    RETURN_IF_NTSTATUS_FAILED(RtlQueryInformationAcl(dacl, &aclRevisionInfo, sizeof(aclRevisionInfo), AclRevisionInformation)); // 64

    ULONG daclLength = aclSizeInfo.AclBytesInUse + 4 /*Header*/ + 4 /*Mask*/ + RtlLengthSid(userSid);

    wil::unique_process_heap_ptr<ACL> newDacl;
    newDacl.reset((PACL)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, daclLength));
    RETURN_IF_NULL_ALLOC(newDacl); // 71

    RETURN_IF_NTSTATUS_FAILED(RtlCreateAcl(newDacl.get(), daclLength, aclRevisionInfo.AclRevision)); // 73

    PVOID ace;
    RETURN_IF_NTSTATUS_FAILED(RtlGetAce(dacl, 0, &ace)); // 76
    RETURN_IF_NTSTATUS_FAILED(RtlAddAce(newDacl.get(), aclRevisionInfo.AclRevision, 0, ace, aclSizeInfo.AclBytesInUse - (4 /*Header*/ + 4 /*Mask*/))); // 78
    RETURN_IF_NTSTATUS_FAILED(RtlAddAccessAllowedAce(newDacl.get(), aclRevisionInfo.AclRevision, flags, userSid)); // 81
    RETURN_IF_NTSTATUS_FAILED(RtlSetDaclSecurityDescriptor(newSD, TRUE, newDacl.get(), 0)); // 83

    return S_OK;
}

inline HRESULT AddUserToHandle(HANDLE handle, PSID userSid, ACCESS_MASK flags)
{
    ULONG requiredLength;
    NTSTATUS status = NtQuerySecurityObject(handle, DACL_SECURITY_INFORMATION, nullptr, 0, &requiredLength);
    if (status != (NTSTATUS)0xC0000023L) // STATUS_BUFFER_TOO_SMALL
    {
        RETURN_IF_NTSTATUS_FAILED(status); // 98
        return S_OK;
    }

    wil::unique_process_heap_ptr<SECURITY_DESCRIPTOR> processDesc;
    processDesc.reset((SECURITY_DESCRIPTOR*)HeapAlloc(GetProcessHeap(), 0, requiredLength));
    RETURN_IF_NULL_ALLOC(processDesc); // 102

    RETURN_IF_NTSTATUS_FAILED(NtQuerySecurityObject(handle, DACL_SECURITY_INFORMATION, processDesc.get(), requiredLength, &requiredLength)); // 105

    SECURITY_DESCRIPTOR newSD = {};
    RETURN_IF_NTSTATUS_FAILED(RtlCreateSecurityDescriptor(&newSD, SECURITY_DESCRIPTOR_REVISION)); // 108
    auto daclCleanup = wil::scope_exit([&newSD]() -> void
    {
        if (newSD.Dacl)
        {
            HeapFree(GetProcessHeap(), 0, newSD.Dacl);
        }
    });

    bool exists = false;
    RETURN_IF_FAILED(BuildUserSD(processDesc.get(), userSid, flags, &exists, &newSD)); // 114
    if (!exists)
    {
        RETURN_IF_NTSTATUS_FAILED(NtSetSecurityObject(handle, DACL_SECURITY_INFORMATION, &newSD)); // 121
    }

    return S_OK;
}

inline HRESULT AddUserToProcessObject(PSID userSid)
{
    wil::unique_handle process;
    process.reset(OpenProcess(PROCESS_QUERY_INFORMATION | READ_CONTROL | WRITE_DAC, FALSE, GetCurrentProcessId()));
    RETURN_LAST_ERROR_IF_NULL(process); // 128

    RETURN_IF_FAILED(AddUserToHandle(
        process.get(), userSid, PROCESS_QUERY_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE)); // 130

    wil::unique_handle token;
    RETURN_IF_WIN32_BOOL_FALSE(OpenProcessToken(process.get(), TOKEN_READ | WRITE_DAC, &token)); // 133

    RETURN_IF_FAILED(AddUserToHandle(token.get(), userSid, TOKEN_DUPLICATE | TOKEN_IMPERSONATE | TOKEN_QUERY)); // 135

    return S_OK;
}
