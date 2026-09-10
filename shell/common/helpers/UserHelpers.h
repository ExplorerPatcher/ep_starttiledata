#pragma once

#include <sddl.h>
#include <Windows.System.Internal.h>

#include <wil/token_helpers.h>

#include "CallerIdentity.h"
#include "usermgr.h"

#if (NTDDI_VERSION >= NTDDI_WIN10_TH2)
_IRQL_requires_max_(PASSIVE_LEVEL)
_Must_inspect_result_
NTSYSAPI
DWORD
NTAPI
RtlGetCurrentServiceSessionId (
    VOID
    );
#endif // NTDDI_VERSION >= NTDDI_WIN10_TH2

#if (NTDDI_VERSION >= NTDDI_WIN10_RS1)
_IRQL_requires_max_(PASSIVE_LEVEL)
_Must_inspect_result_
NTSYSAPI
BOOLEAN
NTAPI
RtlIsMultiSessionSku (
    VOID
    );
#endif // NTDDI_VERSION >= NTDDI_WIN10_RS1

inline BOOL IsUMgrGetImpersonationTokenForContextPresent()
{
    return TRUE; // Don't care
}

namespace UserHelpers
{
inline UINT64 GetUserContextToken(ABI::Windows::System::IUser* user)
{
    UINT64 userContextToken = 0;

    if (user != nullptr)
    {
        wil::com_ptr<ABI::Windows::System::Internal::ISignInStateManager> signInStateManager;
        THROW_IF_FAILED(RoGetActivationFactory(
            Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Windows_System_Internal_UserManager).Get(),
            IID_PPV_ARGS(&signInStateManager))); // 76
        THROW_IF_FAILED(signInStateManager->GetHandleForUser(user, &userContextToken)); // 77
    }
    else
    {
        if (IsUMgrGetImpersonationTokenForContextPresent())
        {
            THROW_HR(UMgrQueryUserContext(nullptr, &userContextToken)); // 66
        }
        else
        {
            THROW_HR(E_NOTIMPL); // 70
        }
    }

    return userContextToken;
}

inline wil::unique_handle GetUserTokenHandle(ABI::Windows::System::IUser* user)
{
    wil::unique_handle userTokenHandle;

    if (!IsUMgrGetImpersonationTokenForContextPresent() || user == nullptr)
    {
        HRESULT hr = wil::open_current_access_token_nothrow(&userTokenHandle);
        if (hr == HRESULT_FROM_WIN32(ERROR_NO_TOKEN))
        {
            hr = wil::open_current_access_token_nothrow(&userTokenHandle);
        }
        THROW_IF_FAILED(hr); // 97
    }
    else
    {
        THROW_IF_FAILED(UMgrQueryUserToken(GetUserContextToken(user), &userTokenHandle)); // 102
    }

    return userTokenHandle;
}

inline wil::unique_tokeninfo_ptr<TOKEN_USER> GetUserToken(ABI::Windows::System::IUser* user)
{
    return wil::get_token_information<TOKEN_USER>(GetUserTokenHandle(user).get());
}

inline wil::unique_hlocal_string GetUserSidString(ABI::Windows::System::IUser* user)
{
    wil::unique_hlocal_string sidString;
    THROW_IF_WIN32_BOOL_FALSE(ConvertSidToStringSidW(GetUserToken(user)->User.Sid, wil::out_param(sidString))); // 115

    return sidString;
}

inline wil::unique_hlocal_string GetUserSidString(UINT64 userContextToken)
{
    wil::unique_handle userTokenHandle;
    THROW_IF_FAILED(UMgrQueryUserToken(userContextToken, &userTokenHandle)); // 122

    wil::unique_hlocal_string sidString;
    THROW_IF_WIN32_BOOL_FALSE(ConvertSidToStringSidW(wil::get_token_information<TOKEN_USER>(userTokenHandle.get())->User.Sid, wil::out_param(sidString))); // 125

    return sidString;
}

inline bool IsSystemAccount(HANDLE accessToken)
{
    wil::unique_tokeninfo_ptr<TOKEN_USER> tokenUser = wil::get_token_information<TOKEN_USER>(accessToken);
    return IsWellKnownSid(tokenUser->User.Sid, WinLocalSystemSid)
        || IsWellKnownSid(tokenUser->User.Sid, WinLocalServiceSid)
        || IsWellKnownSid(tokenUser->User.Sid, WinNetworkServiceSid)
        || IsWellKnownSid(tokenUser->User.Sid, WinAccountDefaultSystemManagedSid);
}

inline bool IsThreadEffectiveToken(HANDLE accessToken)
{
    wil::unique_tokeninfo_ptr<TOKEN_USER> tokenUserThreadEffectiveToken =
        wil::get_token_information<TOKEN_USER>(GetCurrentThreadEffectiveToken());
    wil::unique_tokeninfo_ptr<TOKEN_USER> tokenUser = wil::get_token_information<TOKEN_USER>(accessToken);
    return EqualSid(tokenUserThreadEffectiveToken->User.Sid, tokenUser->User.Sid);
}

class Impersonator
{
public:
    Impersonator(ABI::Windows::System::IUser* user)
    {
        Initialize(user);
    }

    wil::unique_token_reverter Impersonate()
    {
        FAIL_FAST_IF_NULL(_userHandle); // 216
        if (IsThreadEffectiveToken(_userHandle.get()))
        {
            wil::unique_token_reverter tokenHandle;
            tokenHandle.reset(INVALID_HANDLE_VALUE);
            return tokenHandle;
        }
        else
        {
            static bool s_disabledPredefinedCache;
            if (!s_disabledPredefinedCache)
            {
                THROW_IF_WIN32_ERROR(RegDisablePredefinedCacheEx()); // 227
                s_disabledPredefinedCache = true;
            }

            wil::unique_handle tokenHandle;
            THROW_LAST_ERROR_IF(!OpenThreadToken(GetCurrentThread(), TOKEN_ALL_ACCESS, true, tokenHandle.put())
                && GetLastError() != ERROR_NO_TOKEN); // 237
            THROW_IF_WIN32_BOOL_FALSE(ImpersonateLoggedOnUser(_userHandle.get())); // 242
            return wil::unique_token_reverter(tokenHandle.release());
        }
    }

private:
    void Initialize(ABI::Windows::System::IUser* user)
    {
        _user = user;

        if (user != nullptr)
        {
            bool isAppContainer;
            THROW_IF_FAILED(CallerIdentity::IsProcessAppContainer(GetCurrentProcess(), &isAppContainer)); // 270
            wil::unique_handle accessTokenHandle = wil::open_current_access_token();

            wil::com_ptr<ABI::Windows::System::Internal::ISignInStateManager> signInStateManager;
            THROW_IF_FAILED(RoGetActivationFactory(
                Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Windows_System_Internal_UserManager).Get(),
                IID_PPV_ARGS(&signInStateManager))); // 274
            THROW_IF_FAILED(signInStateManager->GetHandleForUser(user, &_userContextToken)); // 275

            if (!IsSystemAccount(accessTokenHandle.get()))
            {
                UINT64 userContextToken;
                THROW_IF_FAILED(UMgrQueryUserContext(nullptr, &userContextToken)); // 283
                if (userContextToken == _userContextToken)
                {
                    _userHandle = std::move(accessTokenHandle);
                }
            }

            if (!_userHandle.is_valid())
            {
                if (isAppContainer)
                {
                    FAIL_FAST_IF(!IsUMgrGetImpersonationTokenForContextPresent()); // 294
                    THROW_IF_FAILED(UMgrGetConstrainedUserToken(
                        nullptr, _userContextToken, nullptr, wil::out_param(_userHandle))); // 295
                }
                else
                {
                    DWORD sessionId;
                    THROW_IF_WIN32_BOOL_FALSE(ProcessIdToSessionId(GetCurrentProcessId(), &sessionId)); // 301
                    if (RtlIsMultiSessionSku() && sessionId == RtlGetCurrentServiceSessionId())
                    {
                        wil::unique_handle userTokenHandle;
                        THROW_IF_FAILED(UMgrQueryUserToken(_userContextToken, wil::out_param(userTokenHandle))); // 308
                        THROW_IF_WIN32_BOOL_FALSE(DuplicateTokenEx(
                            userTokenHandle.get(), MAXIMUM_ALLOWED, nullptr, SecurityImpersonation, TokenImpersonation,
                            wil::out_param(_userHandle))); // 309
                    }
                    else
                    {
                        wil::unique_handle userTokenHandle;
                        FAIL_FAST_IF(!IsUMgrGetImpersonationTokenForContextPresent()); // 321
                        THROW_IF_FAILED(UMgrQuerySessionUserToken(sessionId, wil::out_param(userTokenHandle))); // 322

                        wil::unique_handle impersonationTokenHandle;
                        THROW_IF_WIN32_BOOL_FALSE(DuplicateTokenEx(
                            userTokenHandle.get(), MAXIMUM_ALLOWED, nullptr, SecurityImpersonation, TokenImpersonation,
                            wil::out_param(impersonationTokenHandle))); // 325

                        FAIL_FAST_IF(!IsUMgrGetImpersonationTokenForContextPresent()); // 326
                        THROW_IF_FAILED(UMgrGetImpersonationTokenForContext(
                            impersonationTokenHandle.get(), _userContextToken, wil::out_param(_userHandle))); // 327
                    }
                }
            }
        }
        else
        {
            THROW_IF_FAILED(UMgrQueryUserContext(nullptr, &_userContextToken)); // 264

            wil::unique_handle accessTokenHandle = wil::open_current_access_token();
            _userHandle = std::move(accessTokenHandle);
        }
    }

    wil::com_ptr<ABI::Windows::System::IUser> _user;
    wil::unique_handle _userHandle;
    UINT64 _userContextToken;
};
}
