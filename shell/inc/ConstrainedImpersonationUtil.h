#pragma once

#include "usermgr.h"

class ConstrainedImpersonateLoggedOnUser
{
public:
    ConstrainedImpersonateLoggedOnUser(ABI::Windows::System::IUser* userObject)
        : m_impersonating(false)
        , m_userObject(userObject)
        , m_contextToken(0)
    {
    }

    ConstrainedImpersonateLoggedOnUser(const ConstrainedImpersonateLoggedOnUser&) = delete;

    ~ConstrainedImpersonateLoggedOnUser()
    {
        Revert();
    }

    HRESULT Impersonate();

    void Revert()
    {
        if (m_impersonating)
        {
            FAIL_FAST_IF_WIN32_BOOL_FALSE(RevertToSelf()); // 73
            m_impersonating = false;
        }
    }

    HRESULT DuplicateImpersonationToken(HANDLE* pImpersonationToken)
    {
        *pImpersonationToken = nullptr;

        RETURN_IF_FAILED(GenerateImpersonationToken()); // 86

        if (m_userTokenHandle.is_valid())
        {
            RETURN_IF_WIN32_BOOL_FALSE(DuplicateHandle(
                GetCurrentProcess(), m_userTokenHandle.get(), GetCurrentProcess(), pImpersonationToken, 0, FALSE,
                DUPLICATE_SAME_ACCESS)); // 91
        }

        return S_OK;
    }

    HRESULT GetUserContextToken(UINT64*);

    ConstrainedImpersonateLoggedOnUser& operator=(const ConstrainedImpersonateLoggedOnUser&) = delete;

private:
    HRESULT GenerateImpersonationToken()
    {
        if (m_userObject != nullptr && !m_userTokenHandle.is_valid() /*&& IsUMgrGetConstrainedUserTokenPresent()*/)
        {
            Microsoft::WRL::ComPtr<ABI::Windows::System::Internal::ISignInStateManager> factory;
            RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(
                Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Windows_System_Internal_UserManager).Get(),
                &factory)); // 129

            RETURN_IF_FAILED(factory->GetHandleForUser(m_userObject.Get(), &m_contextToken)); // 131

            RETURN_IF_FAILED(UMgrGetConstrainedUserToken(nullptr, m_contextToken, nullptr, &m_userTokenHandle)); // 133
        }

        return S_OK;
    }

    bool m_impersonating;
    wil::unique_handle m_userTokenHandle;
    Microsoft::WRL::ComPtr<ABI::Windows::System::IUser> m_userObject;
    UINT64 m_contextToken;
};
