#pragma once

#include <winternl.h>
#include <wil/result_macros.h>

#include "ResultUtils.h"
#include "memsafe.h"

enum RUNTIMEBROKER_CALLERIDENTITY_CHECK
{
    RCC_ASSERT_IF_RUNTIMEBROKER,
    RCC_FAIL_IF_RUNTIMEBROKER,
    RCC_ALLOW_IF_RUNTIMEBROKER,
};

MIDL_INTERFACE("68c6a1b9-de39-42c3-8d28-bf40a5126541")
ICallingProcessInfo : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE OpenCallerProcessHandle(DWORD desiredAccess, HANDLE* callerProcessHandle) = 0;
};

enum PROCESS_UICONTEXT
{
    PROCESS_UICONTEXT_DESKTOP = 0,
    PROCESS_UICONTEXT_IMMERSIVE = 1,
    PROCESS_UICONTEXT_IMMERSIVE_BROKER = 2,
    PROCESS_UICONTEXT_IMMERSIVE_BROWSER = 3,
};

enum PROCESS_UI_FLAGS
{
    PROCESS_UIF_NONE = 0,
    PROCESS_UIF_AUTHORING_MODE = 0x1,
    PROCESS_UIF_RESTRICTIONS_DISABLED = 0x2,
};

DEFINE_ENUM_FLAG_OPERATORS(PROCESS_UI_FLAGS);

struct PROCESS_UICONTEXT_INFORMATION
{
    PROCESS_UICONTEXT processUIContext;
    PROCESS_UI_FLAGS flags;
};

STDAPI_(BOOL) GetProcessUIContextInformation(HANDLE hProcess, PROCESS_UICONTEXT_INFORMATION* pProcessUIContextInformation);
STDAPI CapabilityCheck(HANDLE token, const WCHAR* capability, BOOLEAN* result);

// ApplicationIdentityBuffered.cpp
inline HRESULT ParseAppUserModelId(
    const WCHAR* pszAppUserModelId, WCHAR* pszPackageFamilyName, size_t cchPackageFamilyName,
    WCHAR* pszPackageRelativeApplicationId, size_t cchPackageRelativeApplicationId)
{
    if (pszPackageFamilyName && cchPackageFamilyName)
        *pszPackageFamilyName = 0;

    if (pszPackageRelativeApplicationId && cchPackageRelativeApplicationId)
        *pszPackageRelativeApplicationId = 0;

    WCHAR szAppUserModelId[130];
    HRESULT hr = StringCchCopyW(szAppUserModelId, ARRAYSIZE(szAppUserModelId), pszAppUserModelId);
    if (FAILED(hr))
    {
        if (hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
            return hr;
        return E_INVALIDARG;
    }

    WCHAR* delim = wcsrchr(szAppUserModelId, '!');
    if (szAppUserModelId[0] == '!')
        return E_INVALIDARG;
    if (!delim)
        return E_INVALIDARG;

    const WCHAR* pszPackageRelativeApplicationIdOffset = delim + 1;
    if (!*pszPackageRelativeApplicationIdOffset)
        return E_INVALIDARG;

    hr = S_FALSE;
    *delim = 0;
    if (SUCCEEDED(hr) && pszPackageFamilyName)
    {
        hr = StringCchCopyW(pszPackageFamilyName, cchPackageFamilyName, szAppUserModelId);
    }
    if (SUCCEEDED(hr) && pszPackageRelativeApplicationId)
    {
        hr = StringCchCopyW(
            pszPackageRelativeApplicationId, cchPackageRelativeApplicationId, pszPackageRelativeApplicationIdOffset);
    }

    return hr;
}

// ApplicationIdentity.cpp
inline HRESULT ParseAppUserModelId(
    const WCHAR* pszAppUserModelId, PWSTR* ppszPackageFamilyName, WCHAR** ppszPackageRelativeApplicationId)
{
    WCHAR szPackageRelativeApplicationId[65];
    WCHAR szPackageFamilyName[65];

    if (ppszPackageFamilyName)
        *ppszPackageFamilyName = nullptr;
    if (ppszPackageRelativeApplicationId)
        *ppszPackageRelativeApplicationId = nullptr;

    HRESULT hr = ParseAppUserModelId(
        pszAppUserModelId, szPackageFamilyName, ARRAYSIZE(szPackageFamilyName), szPackageRelativeApplicationId,
        ARRAYSIZE(szPackageRelativeApplicationId));
    if (ppszPackageFamilyName && SUCCEEDED(hr))
    {
        hr = CoAllocString(szPackageFamilyName, ppszPackageFamilyName);
    }

    if (ppszPackageRelativeApplicationId && SUCCEEDED(hr))
    {
        hr = CoAllocString(szPackageRelativeApplicationId, ppszPackageRelativeApplicationId);
        if (FAILED(hr))
        {
            if (ppszPackageFamilyName)
            {
                CoTaskMemFree(*ppszPackageFamilyName);
                *ppszPackageFamilyName = nullptr;
            }
        }
    }

    return hr;
}

typedef struct _TOKEN_SECURITY_ATTRIBUTE_FQBN_VALUE
{
    UINT64 Version;
    UNICODE_STRING Name;
} TOKEN_SECURITY_ATTRIBUTE_FQBN_VALUE;

typedef struct _TOKEN_SECURITY_ATTRIBUTE_OCTET_STRING_VALUE
{
    void* pValue;
    ULONG ValueLength;
} TOKEN_SECURITY_ATTRIBUTE_OCTET_STRING_VALUE;

typedef struct _TOKEN_SECURITY_ATTRIBUTE_V1
{
    UNICODE_STRING Name;
    WORD ValueType;
    WORD Reserved;
    ULONG Flags;
    ULONG ValueCount;
    union
    {
        INT64* pInt64;
        UINT64* pUint64;
        UNICODE_STRING* pString;
        TOKEN_SECURITY_ATTRIBUTE_FQBN_VALUE* pFqbn;
        TOKEN_SECURITY_ATTRIBUTE_OCTET_STRING_VALUE* pOctetString;
    } Values;
} TOKEN_SECURITY_ATTRIBUTE_V1;

typedef struct _TOKEN_SECURITY_ATTRIBUTES_INFORMATION
{
    WORD Version;
    WORD Reserved;
    ULONG AttributeCount;
    union
    {
        TOKEN_SECURITY_ATTRIBUTE_V1* pAttributeV1;
    } Attribute;
} TOKEN_SECURITY_ATTRIBUTES_INFORMATION;

EXTERN_C_START

NTSYSAPI NTSTATUS NTAPI NtQueryInformationToken(
    HANDLE TokenHandle,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    PVOID TokenInformation,
    ULONG TokenInformationLength,
    PULONG ReturnLength
);

NTSYSAPI LONG NTAPI RtlCompareUnicodeString(
    PCUNICODE_STRING String1,
    PCUNICODE_STRING String2,
    BOOLEAN CaseInsensitive
);

NTSYSAPI ULONG NTAPI RtlNtStatusToDosErrorNoTeb(NTSTATUS Status);

EXTERN_C_END

namespace ARI
{

template <typename T>
T* Allocate(size_t n)
{
    SIZE_T bytes;
    if (SUCCEEDED(ULongLongToSIZET(n * static_cast<ULONGLONG>(sizeof(T)), &bytes)))
    {
        // return static_cast<T*>(RtlAllocateHeap(NtCurrentPeb()->ProcessHeap, 0, bytes));
        return static_cast<T*>(HeapAlloc(GetProcessHeap(), 0, bytes)); // @MOD Not using Native API
    }
    else
    {
        return nullptr;
    }
}

inline void Free(void* p)
{
    if (p)
    {
        // RtlFreeHeap(NtCurrentPeb()->ProcessHeap, 0, p);
        HeapFree(GetProcessHeap(), 0, p); // @MOD Not using Native API
    }
}

namespace ProcessToken
{
    namespace SysAppId
    {
        inline LSTATUS GetPackageRelativeApplicationId(
            const TOKEN_SECURITY_ATTRIBUTE_V1* sysAppId, UINT lengthAvailable, UINT* length,
            WCHAR* packageRelativeApplicationId)
        {
            const UNICODE_STRING* attributePackageRelativeApplicationId = &sysAppId->Values.pString[1];
            UINT stringLength = attributePackageRelativeApplicationId->Length / sizeof(WCHAR);
            *length = stringLength + 1;
            if (lengthAvailable >= *length)
            {
                memcpy(
                    packageRelativeApplicationId, attributePackageRelativeApplicationId->Buffer,
                    sizeof(WCHAR) * stringLength);
                packageRelativeApplicationId[stringLength] = 0;
                return ERROR_SUCCESS;
            }
            else
            {
                return ERROR_INSUFFICIENT_BUFFER;
            }
        }

        inline LSTATUS GetPackageFamilyName(
            const TOKEN_SECURITY_ATTRIBUTE_V1* sysAppId, UINT lengthAvailable, UINT* length, WCHAR* packageFamilyName)
        {
            UNICODE_STRING* attributePackageFullName = &sysAppId->Values.pString[0];
            UINT attributePackageFullNameLength = attributePackageFullName->Length / sizeof(WCHAR);
            WCHAR* nameEnd = wcschr(attributePackageFullName->Buffer, '_');
            UINT nameLength = static_cast<UINT>(nameEnd + 1 - attributePackageFullName->Buffer);
            WCHAR* publisherId = &attributePackageFullName->Buffer[attributePackageFullNameLength - (sizeof(WCHAR) * 13)];
            *length = nameLength + 13 + 1;
            if (lengthAvailable >= *length)
            {
                memcpy(&packageFamilyName[0], attributePackageFullName->Buffer, sizeof(WCHAR) * nameLength);
                memcpy(&packageFamilyName[nameLength], publisherId, sizeof(WCHAR) * 13);
                packageFamilyName[nameLength + 13] = 0;
                return ERROR_SUCCESS;
            }
            else
            {
                return ERROR_INSUFFICIENT_BUFFER;
            }
        }

        inline LSTATUS GetAppUserModelId(
            const TOKEN_SECURITY_ATTRIBUTE_V1* sysAppId, UINT lengthAvailable, UINT* length, WCHAR* appUserModelId)
        {
            UINT praidLength;
            LSTATUS lastError = GetPackageRelativeApplicationId(sysAppId, 0, &praidLength, nullptr);
            if (lastError == ERROR_INSUFFICIENT_BUFFER)
            {
                UINT familyLength;
                lastError = GetPackageFamilyName(sysAppId, lengthAvailable, &familyLength, appUserModelId);
                if (lastError == ERROR_INSUFFICIENT_BUFFER || lastError == ERROR_SUCCESS)
                {
                    if (lastError != ERROR_INSUFFICIENT_BUFFER && lengthAvailable >= familyLength + praidLength)
                    {
                        --familyLength;
                        appUserModelId[familyLength] = '!';
                        UINT praidLength2;
                        GetPackageRelativeApplicationId(
                            sysAppId, lengthAvailable - familyLength - 1, &praidLength2,
                            &appUserModelId[familyLength + 1]);
                        *length = familyLength + 1 + praidLength;
                        lastError = ERROR_SUCCESS;
                    }
                    else
                    {
                        lastError = ERROR_INSUFFICIENT_BUFFER;
                        *length = familyLength + praidLength;
                    }
                }
            }
            return lastError;
        }

        inline LSTATUS OpenTokenForProcess(HANDLE process, HANDLE* token)
        {
            if (process == GetCurrentProcess())
            {
                *token = GetCurrentProcessToken();
                return ERROR_SUCCESS;
            }
            else
            {
                return OpenProcessToken(process, TOKEN_QUERY, token) ? ERROR_SUCCESS : GetLastError();
            }
        }

        inline LSTATUS Open(
            HANDLE token, TOKEN_SECURITY_ATTRIBUTES_INFORMATION** attributes,
            const TOKEN_SECURITY_ATTRIBUTE_V1** sysAppId, bool* isPackaged)
        {
            // @MOD Not using ARI::AutoPtrAriHeap<TOKEN_SECURITY_ATTRIBUTES_INFORMATION>
            LSTATUS rv;

            ULONG n;
            NTSTATUS status = NtQueryInformationToken(token, TokenSecurityAttributes, nullptr, 0, &n);
            if (status == 0xC0000023) // STATUS_BUFFER_TOO_SMALL
            {
                TOKEN_SECURITY_ATTRIBUTES_INFORMATION* sai = Allocate<TOKEN_SECURITY_ATTRIBUTES_INFORMATION>(n);
                if (sai)
                {
                    memset(sai, 0, n);
                    status = NtQueryInformationToken(token, TokenSecurityAttributes, sai, n, &n);
                    if (NT_SUCCESS(status))
                    {
                        if (sai->AttributeCount)
                        {
                            UNICODE_STRING attributeNameSYSAPPID;
                            RtlInitUnicodeString(&attributeNameSYSAPPID, L"WIN://SYSAPPID");
                            for (ULONG index = 0; index < sai->AttributeCount; ++index)
                            {
                                const TOKEN_SECURITY_ATTRIBUTE_V1* attribute = &sai->Attribute.pAttributeV1[index];
                                if (RtlCompareUnicodeString(&attributeNameSYSAPPID, &attribute->Name, TRUE) == 0)
                                {
                                    *sysAppId = attribute;
                                    *attributes = sai; // sai.Detach()
                                    sai = nullptr;
                                    rv = ERROR_SUCCESS;
                                    Free(sai); // Duplicated to remove gotos (while not using AutoPtr)
                                    return rv;
                                }
                            }
                        }
                        rv = ERROR_NOT_FOUND;
                    }
                    else
                    {
                        rv = RtlNtStatusToDosErrorNoTeb(status);
                    }
                }
                else
                {
                    rv = ERROR_NOT_ENOUGH_MEMORY;
                }
                Free(sai);
                return rv;
            }
            else if (status)
            {
                return RtlNtStatusToDosErrorNoTeb(status);
            }
            else
            {
                return ERROR_INTERNAL_ERROR;
            }
        }
    }

    class AutoSysAppId
    {
    public:
        AutoSysAppId()
            : attributes(nullptr)
            , sysAppId(nullptr)
        {
        }

        ~AutoSysAppId()
        {
            Close();
        }

        LSTATUS Open(HANDLE);

        LSTATUS OpenProcess(HANDLE process)
        {
            Close();

            HANDLE token;
            LSTATUS lastError = SysAppId::OpenTokenForProcess(process, &token);
            if (lastError == ERROR_SUCCESS)
            {
                lastError = SysAppId::Open(token, &attributes, &sysAppId, nullptr /*Optimized*/);
                if (token != GetCurrentProcessToken())
                {
                    CloseHandle(token);
                }
            }

            return lastError;
        }

        LSTATUS OpenIfPackaged(HANDLE);

        LSTATUS OpenProcessIfPackaged(HANDLE);

        LSTATUS Close()
        {
            if (IsOpen())
            {
                Free(attributes);
                attributes = nullptr;
                sysAppId = nullptr;
            }
            return ERROR_SUCCESS;
        }

        bool IsOpen() const { return attributes != nullptr; }

        LSTATUS GetPackageFullName(const UINT lengthAvailable, UINT* length, WCHAR*);

        LSTATUS GetPackageFamilyName(const UINT lengthAvailable, UINT* length, WCHAR*);

        LSTATUS GetAppUserModelId(const UINT lengthAvailable, UINT* length, WCHAR* appUserModelId) const
        {
            return SysAppId::GetAppUserModelId(sysAppId, lengthAvailable, length, appUserModelId);
        }

        LSTATUS GetPackageRelativeApplicationId(const UINT lengthAvailable, UINT* length, WCHAR*);

        // TOKEN_SECURITY_ATTRIBUTES_INFORMATION* GetAttributes();
        // TOKEN_SECURITY_ATTRIBUTES_INFORMATION* GetAttributes();
        // TOKEN_SECURITY_ATTRIBUTE_V1* GetSysAppId();

    private:
        TOKEN_SECURITY_ATTRIBUTES_INFORMATION* attributes;
        const TOKEN_SECURITY_ATTRIBUTE_V1* sysAppId;
    };
}
}

typedef void* HSTATE;

EXTERN_C WINBASEAPI HSTATE WINAPI OpenStateExplicit(HANDLE userToken, const WCHAR* packageFamilyName);
EXTERN_C WINBASEAPI void WINAPI CloseState(HSTATE state);

EXTERN_C WINBASEAPI BOOL WINAPI GetSystemAppDataKey(HSTATE, HKEY*, const WCHAR*, UINT*);

namespace CallerIdentity
{
    static DWORD g_dwRuntimeBrokerProcessId = DWORD_MAX;
    static bool g_fRuntimeBrokerProcessIdInitialize;

    inline void _EnsureRuntimeBrokerPID()
    {
        if (g_fRuntimeBrokerProcessIdInitialize)
            return;

        HANDLE dwProcessId = GetCurrentProcess();
        WCHAR szImageName[260];
        DWORD dwSize = ARRAYSIZE(szImageName);
        if (SUCCEEDED(ResultFromWin32Bool(QueryFullProcessImageNameW(dwProcessId, 0, szImageName, &dwSize))))
        {
            WCHAR szExpandedPath[260];
            if (ExpandEnvironmentStringsW(
                L"%SystemRoot%\\System32\\RuntimeBroker.exe", szExpandedPath, ARRAYSIZE(szExpandedPath)))
            {
                if (CompareStringOrdinal(szImageName, -1, szExpandedPath, -1, TRUE) == CSTR_EQUAL)
                {
                    g_dwRuntimeBrokerProcessId = GetProcessId(dwProcessId);
                }
            }
        }
        g_fRuntimeBrokerProcessIdInitialize = true;
    }

    inline HRESULT GetCallingProcessHandle(
        DWORD dwProcessAccessFlags, RUNTIMEBROKER_CALLERIDENTITY_CHECK runtimeBrokerCheck, HANDLE* phProcess)
    {
        *phProcess = nullptr;

        Microsoft::WRL::ComPtr<ICallingProcessInfo> spCallingProcessInfo;
        HRESULT hr = CoGetCallContext(IID_PPV_ARGS(&spCallingProcessInfo));
        if (FAILED(hr))
        {
            RETURN_HR_IF(hr, hr != RPC_E_CALL_COMPLETE);
            *phProcess = GetCurrentProcess();
        }
        else
        {
            hr = spCallingProcessInfo->OpenCallerProcessHandle(dwProcessAccessFlags, phProcess);
            if (FAILED(hr))
                return hr;
        }

        if (runtimeBrokerCheck == RCC_FAIL_IF_RUNTIMEBROKER)
        {
            _EnsureRuntimeBrokerPID();
            if (GetProcessId(*phProcess) == g_dwRuntimeBrokerProcessId)
            {
                CloseHandle(*phProcess);
                *phProcess = nullptr;
                return E_FAIL;
            }
        }

        return S_OK;
    }

    inline HRESULT GetCallingProcessId(RUNTIMEBROKER_CALLERIDENTITY_CHECK runtimeBrokerCheck, DWORD* pdwProcess)
    {
        wil::unique_handle shProcess; // OG uses CAutoHandle<HANDLE>
        HRESULT hr = GetCallingProcessHandle(PROCESS_QUERY_LIMITED_INFORMATION, runtimeBrokerCheck, shProcess.put());
        if (SUCCEEDED(hr))
        {
            *pdwProcess = GetProcessId(shProcess.get());
        }
        return hr;
    }

    inline HRESULT GetCallingProcessType(PROCESS_UICONTEXT* pProcessUIContext)
    {
        HANDLE hProcess;
        HRESULT hr = GetCallingProcessHandle(PROCESS_QUERY_LIMITED_INFORMATION, RCC_ASSERT_IF_RUNTIMEBROKER, &hProcess);
        if (SUCCEEDED(hr))
        {
            hr = S_OK;
        }

        if (hr == RPC_E_CALL_COMPLETE)
        {
            hProcess = GetCurrentProcess();
            hr = S_OK;
        }

        if (SUCCEEDED(hr))
        {
            PROCESS_UICONTEXT_INFORMATION uicontextInfo;
            if (GetProcessUIContextInformation(hProcess, &uicontextInfo))
            {
                hr = S_OK;
                *pProcessUIContext = uicontextInfo.processUIContext;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
            }

            CloseHandle(hProcess);
        }

        return hr;
    }

    inline HRESULT GetProcessAppId(HANDLE hProcess, WCHAR** ppszAppId)
    {
        *ppszAppId = nullptr;

        ARI::ProcessToken::AutoSysAppId appId;
        HRESULT hr = HRESULT_FROM_WIN32(appId.OpenProcess(hProcess));
        if (hr != E_NOT_SET && hr != E_ACCESSDENIED)
        {
            RETURN_IF_FAILED(hr); // 165

            UINT cchAppId;
            RETURN_HR_IF(E_UNEXPECTED, appId.GetAppUserModelId(0, &cchAppId, nullptr) != ERROR_INSUFFICIENT_BUFFER); // 168

            wil::unique_cotaskmem_string spszAppId; // OG: CMemString<CMemString_PolicyCoTaskMem>
            RETURN_IF_FAILED(CoAllocStringLen(nullptr, cchAppId, &spszAppId)); // 171
            RETURN_IF_WIN32_ERROR(appId.GetAppUserModelId(cchAppId, &cchAppId, spszAppId.get())); // 172

            hr = S_OK;
            *ppszAppId = spszAppId.release();
        }

        return hr;
    }

    inline HRESULT GetCallingProcessAppId(WCHAR** ppszAppId)
    {
        *ppszAppId = nullptr;

        wil::unique_handle shProcess; // OG uses CAutoHandle<HANDLE>
        HRESULT hr = GetCallingProcessHandle(PROCESS_QUERY_LIMITED_INFORMATION, RCC_FAIL_IF_RUNTIMEBROKER, &shProcess);
        if (SUCCEEDED(hr))
        {
            hr = GetProcessAppId(shProcess.get(), ppszAppId);
            if (hr != E_NOT_SET && hr != E_ACCESSDENIED)
            {
                RETURN_IF_FAILED(hr);
                hr = S_OK;
            }
        }

        return hr;
    }

    inline HRESULT GetStateHandle(const WCHAR* pszPackageFamilyName, HSTATE* phState)
    {
        *phState = OpenStateExplicit(GetCurrentThreadEffectiveToken(), pszPackageFamilyName);
        return ResultFromWin32Bool(*phState != nullptr);
    }

    inline HRESULT GetStateRegKey(HANDLE hState, const WCHAR* pszSubKey, REGSAM samDesired, HKEY* phKey)
    {
        *phKey = nullptr;
        HRESULT hr;

        UINT cch = 0;
        if (GetSystemAppDataKey(hState, nullptr, nullptr, &cch))
        {
            hr = E_UNEXPECTED;
        }
        else
        {
            hr = ResultFromKnownLastError();
            if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) && cch)
            {
                wil::unique_cotaskmem_string spszAppKey; // OG: CMemString<CMemString_PolicyCoTaskMem>
                hr = CoAllocStringLen(nullptr, cch, &spszAppKey);
                if (SUCCEEDED(hr))
                {
                    wil::unique_hkey shKeyRoot; // OG: CAutoRegHandle
                    hr = ResultFromWin32Bool(GetSystemAppDataKey(hState, &shKeyRoot, spszAppKey.get(), &cch));
                    if (SUCCEEDED(hr))
                    {
                        if (pszSubKey)
                        {
                            wil::unique_hkey shKeyBase; // OG: CAutoRegHandle
                            hr = HRESULT_FROM_WIN32(RegOpenKeyExW(
                                shKeyRoot.get(), spszAppKey.get(), 0, KEY_READ | KEY_WRITE, &shKeyBase));
                            if (SUCCEEDED(hr))
                            {
                                hr = HRESULT_FROM_WIN32(RegCreateKeyExW(
                                    shKeyBase.get(), pszSubKey, 0, nullptr, 0, samDesired, nullptr, phKey, nullptr));
                            }
                        }
                        else
                        {
                            hr = HRESULT_FROM_WIN32(RegOpenKeyExW(
                                shKeyRoot.get(), spszAppKey.get(), 0, samDesired, phKey));
                        }
                    }
                }
            }
        }

        return hr;
    }

    /*inline HRESULT GetManifestedOrientationPreference(
        const WCHAR* pszAppId, ORIENTATION_PREFERENCE* pOrientationPreference)
    {
        *pOrientationPreference = ORIENTATION_PREFERENCE_NONE;

        CoTaskMemNativeString spszPackageFamilyName;
        HRESULT hr = ParseAppUserModelId(pszAppId, &spszPackageFamilyName, nullptr);
        if (SUCCEEDED(hr))
        {
            HSTATE hState = nullptr;
            hr = GetStateHandle(spszPackageFamilyName.Get(), &hState);
            if (SUCCEEDED(hr))
            {
                wil::unique_hkey shSplashScreenKey;
                hr = GetStateRegKey(hState, L"SplashScreen", KEY_READ, &shSplashScreenKey);
                if (SUCCEEDED(hr))
                {
                    wil::unique_hkey shAppKey;
                    hr = HRESULT_FROM_WIN32(RegOpenKeyExW(shSplashScreenKey.get(), pszAppId, 0, KEY_READ, &shAppKey));
                    if (SUCCEEDED(hr))
                    {
                        DWORD dwType;
                        ORIENTATION_PREFERENCE pref;
                        DWORD cbData = sizeof(pref);
                        hr = HRESULT_FROM_WIN32(RegQueryValueExW(
                            shAppKey.get(), L"Orientation", nullptr, &dwType, (LPBYTE)&pref, &cbData));
                        if (SUCCEEDED(hr))
                        {
                            if (dwType == REG_DWORD)
                            {
                                hr = S_OK;
                                *pOrientationPreference = pref;
                            }
                            else
                            {
                                hr = E_FAIL;
                            }
                        }
                    }
                }

                CloseState(hState);
            }
        }

        return hr;
    }*/

    inline HRESULT GetImpersonationTokenFromProcess(HANDLE hProcess, DWORD dwExtraTokenAccess, HANDLE* phToken)
    {
        *phToken = nullptr;

        wil::unique_handle shPrimaryToken; // OG uses CAutoHandle<HANDLE>
        HRESULT hr = ResultFromWin32Bool(OpenProcessToken(hProcess, dwExtraTokenAccess, &shPrimaryToken));
        if (SUCCEEDED(hr))
        {
            hr = ResultFromWin32Bool(DuplicateTokenEx(
                shPrimaryToken.get(), TOKEN_IMPERSONATE | TOKEN_QUERY, nullptr, SecurityImpersonation,
                TokenImpersonation, phToken));
        }

        return hr;
    }

    // shell/lib/calleridentity/calleridentity_capability.cpp
    inline HRESULT CheckCapabilityFromImpersonationToken(HANDLE impersonationToken, const WCHAR* capabilityName, bool* capabilityPresent)
    {
        *capabilityPresent = false;

        BOOLEAN present;
        RETURN_IF_NTSTATUS_FAILED(CapabilityCheck(impersonationToken, capabilityName, &present)); // 14
        *capabilityPresent = present != 0;

        return S_OK;
    }

    // shell/lib/calleridentity/calleridentity_capability.cpp
    inline HRESULT CheckCapabilityFromProcessHandle(HANDLE process, const WCHAR* capabilityName, bool* capabilityPresent)
    {
        *capabilityPresent = false;

        wil::unique_handle impersonationToken;
        RETURN_IF_FAILED(GetImpersonationTokenFromProcess(
            process, TOKEN_DUPLICATE | TOKEN_IMPERSONATE | TOKEN_QUERY, &impersonationToken)); // 25

        return CheckCapabilityFromImpersonationToken(impersonationToken.get(), capabilityName, capabilityPresent);
    }

    // shell/lib/calleridentity/calleridentity_capability.cpp
    inline HRESULT CheckCallerCapability(const WCHAR* capabilityName, bool* capabilityPresent)
    {
        *capabilityPresent = false;

        wil::unique_handle callingProcessHandle;
        RETURN_IF_FAILED(GetCallingProcessHandle(0, RCC_ASSERT_IF_RUNTIMEBROKER, &callingProcessHandle)); // 35

        return CheckCapabilityFromProcessHandle(callingProcessHandle.get(), capabilityName, capabilityPresent);
    }

    inline bool IsShellExperienceAppId(const WCHAR* callerAppId)
    {
        const WCHAR* const c_rgAllowedCallers[] = {
            L"Microsoft.Windows.ShellExperienceHost_cw5n1h2txyewy!App",
            L"Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy!App",
            L"5b04b775-356b-4aa0-aaf8-6491ffea5602_6f5w9sgpe6vgt!WP",
            L"StartAppVS_xbqbv2ksdy6ng!App",
            L"Microsoft.Windows.Cortana_cw5n1h2txyewy!ppleae38af2e007f4358a809ac99a64a67c1",
            L"Microsoft.Cortana_8wekyb3d8bbwe!ppleae38af2e007f4358a809ac99a64a67c1",
            L"HoloShell_cw5n1h2txyewy!HoloShell",
        };
        for (UINT i = 0; i < ARRAYSIZE(c_rgAllowedCallers); ++i)
        {
            if (CompareStringOrdinal(c_rgAllowedCallers[i], -1, callerAppId, -1, TRUE) == CSTR_EQUAL)
            {
                return true;
            }
        }
        return false;
    }

    // @NOTE: This is the only one with names from 14393 twinui.dll because it's not in 19041. However, 14393 had some
    //        extra logic that called GetCallingProcessHandle and IsProcessAppContainer, so it may have changed.
    inline HRESULT EnsureCallingProcessIsShellExperience()
    {
        bool isCapabilityPresent;
        if (SUCCEEDED(CheckCallerCapability(L"shellExperience", &isCapabilityPresent))
            && isCapabilityPresent)
        {
            return S_OK;
        }

        wil::unique_cotaskmem_string spszCallerAppID;
        HRESULT hr = GetCallingProcessAppId(&spszCallerAppID);
        if (SUCCEEDED(hr))
        {
            hr = IsShellExperienceAppId(spszCallerAppID.get()) ? S_OK : E_ACCESSDENIED;
        }

        return hr;
    }

    inline HRESULT IsProcessAppContainer(HANDLE hProcess, bool* pfAppContainer)
    {
        *pfAppContainer = false;

        wil::unique_handle shToken; // OG uses CAutoHandle<HANDLE>
        HRESULT hr = HRESULT_FROM_WIN32(ARI::ProcessToken::SysAppId::OpenTokenForProcess(hProcess, &shToken));
        if (SUCCEEDED(hr))
        {
            BOOL IsAppContainer = FALSE;
            DWORD cbToken;
            hr = ResultFromWin32Bool(GetTokenInformation(
                shToken.get(), TokenIsAppContainer, &IsAppContainer, sizeof(IsAppContainer), &cbToken));
            if (SUCCEEDED(hr))
            {
                *pfAppContainer = IsAppContainer != 0;
            }
        }

        return hr;
    }
}
