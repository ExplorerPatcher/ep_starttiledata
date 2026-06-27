#pragma once

#include <sddl.h>
#include <Windows.System.Internal.h>

#include <wil/token_helpers.h>

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
            THROW_HR(UMgrQueryUserContext(0, &userContextToken)); // 66
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
}
