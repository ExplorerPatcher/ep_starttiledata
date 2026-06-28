#pragma once

#include <Windows.System.h>
#include <Windows.System.Internal.h>

#include "ConstrainedImpersonationUtil.h"
#include "UserModelTokenHelpers.h"

EXTERN_C_START

ROAPI
_Check_return_
HRESULT
WINAPI
RoGetActivationFactoryAsUser(
    _In_ HSTRING activatableClassId,
    _In_ UINT64 userContext,
    _In_ REFIID iid,
    _COM_Outptr_ void** factory
    );

EXTERN_C_END

namespace Windows::Foundation
{
template <typename T>
HRESULT GetActivationFactoryAsUser(HSTRING activatableClassId, ABI::Windows::System::IUser* user, T** factory)
{
    *factory = nullptr;

    if (user != nullptr)
    {
        ConstrainedImpersonateLoggedOnUser impersonator(user);

        wil::unique_handle impersonationHandle;
        RETURN_IF_FAILED(impersonator.DuplicateImpersonationToken(&impersonationHandle)); // 81

        wil::unique_hlocal tokenUser;
        RETURN_IF_FAILED(GetTokenInformationHelper(impersonationHandle.get(), TokenUser, &tokenUser)); // 84

        RETURN_IF_FAILED(AddUserToProcessObject(tokenUser.get())); // 87

        Microsoft::WRL::ComPtr<ABI::Windows::System::Internal::ISignInStateManager> signinMgr;
        RETURN_IF_FAILED(GetActivationFactory(
            Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Windows_System_Internal_UserManager).Get(),
            &signinMgr)); // 90

        UINT64 userContext;
        RETURN_IF_FAILED(signinMgr->GetHandleForUser(user, &userContext)); // 93

        RETURN_IF_FAILED(RoGetActivationFactoryAsUser(activatableClassId, userContext, IID_PPV_ARGS(factory))); // 95
    }
    else
    {
        RETURN_IF_FAILED(RoGetActivationFactory(activatableClassId, IID_PPV_ARGS(factory))); // 99
    }

    return S_OK;
}
}
