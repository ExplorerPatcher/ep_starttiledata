#pragma once

// #include <ntlsa.h>
// #include <ntdef.h>

typedef struct _SESSION_USER_CONTEXT
{
    UINT64 ContextToken;
    ULONG SessionId;
    ULONG Reserved;
} SESSION_USER_CONTEXT, *PSESSION_USER_CONTEXT;

typedef struct _CRED_PROV_CREDENTIAL
{
    ULONG Flags;
    ULONG AuthenticationPackage;
    ULONG Size;
    PVOID Information;
} CRED_PROV_CREDENTIAL, *PCRED_PROV_CREDENTIAL;

EXTERN_C_START

BOOL WINAPI IsInteractiveUserSession(ULONG SessionId);
BOOL WINAPI QueryActiveSession(PULONG pulSessionId);
BOOL WINAPI QueryUserToken(ULONG SessionId, PHANDLE phToken);
BOOL WINAPI RegisterUsertokenForNoWinlogon();
HRESULT WINAPI UMgrChangeSessionActiveShellUser(ULONG sessionId, UINT64 userContext);
HRESULT WINAPI UMgrChangeSessionUserToken(HANDLE hToken);
HRESULT WINAPI UMgrClearDefaultSignInAccount();
HRESULT WINAPI UMgrConnectLocalUser(LPCWSTR localPassword, GUID providerGuid, PBYTE pAuthInfoBuffer, ULONG cbAuthInfoBuffer);
HRESULT WINAPI UMgrDisconnectLocalUser(GUID providerGuid, PBYTE pAuthInfoBuffer, ULONG cbAuthInfoBuffer, LPCWSTR localUsername, LPCWSTR localPassword);
HRESULT WINAPI UMgrEnumerateSessionUsers(PULONG pCount);
HRESULT WINAPI UMgrFreeSessionUsers(PSESSION_USER_CONTEXT pSessionUsers);
HRESULT WINAPI UMgrFreeUserCredentials(PCRED_PROV_CREDENTIAL pCredential);
HRESULT WINAPI UMgrGetCachedCredentials(PISID pUserSid, PCRED_PROV_CREDENTIAL* ppCredentialsCache);
HRESULT WINAPI UMgrGetConstrainedUserToken(HANDLE callerToken, UINT64 userContext, PSECURITY_CAPABILITIES securityCapabilities, PHANDLE phToken);
HRESULT WINAPI UMgrGetDefaultSignInAccount();
HRESULT WINAPI UMgrGetImpersonationTokenForContext(HANDLE callerToken, UINT64 userContext, PHANDLE phToken);
HRESULT WINAPI UMgrGetSessionActiveShellUserToken(ULONG sessionId, PHANDLE phToken);
HRESULT WINAPI UMgrInformFlags(ULONG flags);
HRESULT WINAPI UMgrInformUserLogoff(HANDLE hToken);
HRESULT WINAPI UMgrInformUserLogon(HANDLE hToken);
HRESULT WINAPI UMgrIsAllowedToActivateAsUser(HANDLE callerToken, UINT64 userContext, PBOOLEAN pbAllowed);
HRESULT WINAPI UMgrLaunchShell(PHANDLE phProcess);
HRESULT WINAPI UMgrLaunchShellInfrastructureHost(PSID LogonSid);
/*HRESULT WINAPI UMgrLogonUser(
    PSTRING OriginName, SECURITY_LOGON_TYPE LogonType, ULONG AuthenticationPackage, PVOID AuthenticationInformation,
    ULONG AuthenticationInformationLength, PTOKEN_GROUPS LocalGroups, PTOKEN_SOURCE SourceContext, ULONG uLogonFlags,
    ULONG uNtlmOptionFlags, PBYTE pAddress, ULONG uAddressSize, PVOID* ProfileBuffer, PULONG ProfileBufferLength,
    PLUID LogonId, PVOID* LsaToken, PQUOTA_LIMITS Quotas, PNTSTATUS Status, PNTSTATUS SubStatus);*/
HRESULT WINAPI UMgrOpenProcessHandleForAccess(DWORD desiredAccess, DWORD processId, PHANDLE phProcess);
HRESULT WINAPI UMgrOpenProcessTokenForQuery(DWORD processId, PHANDLE phToken);
HRESULT WINAPI UMgrQueryDefaultAccountToken(PHANDLE phToken);
HRESULT WINAPI UMgrQuerySessionUserToken(ULONG sessionId, PHANDLE phToken);
HRESULT WINAPI UMgrQuerySessionVirtualAccountToken(ULONG sessionId, PHANDLE phToken);
HRESULT WINAPI UMgrQueryUserContext(HANDLE hToken, PUINT64 userContext);
HRESULT WINAPI UMgrQueryUserContextFromName(LPCWSTR username, PUINT64 userContext);
HRESULT WINAPI UMgrQueryUserContextFromSid(LPCWSTR userSid, PUINT64 userContext);
HRESULT WINAPI UMgrQueryUserToken(UINT64 userContext, PHANDLE phToken);
HRESULT WINAPI UMgrQueryUserTokenFromName(LPCWSTR username, PHANDLE phToken);
HRESULT WINAPI UMgrQueryUserTokenFromSid(LPCWSTR userSid, PHANDLE phToken);
HRESULT WINAPI UMgrSetCachedCredentials(PSID pUserSid, PCRED_PROV_CREDENTIAL pCredentialsCache);
HRESULT WINAPI UMgrSetShellInformation(HANDLE hProcess);

EXTERN_C_END
