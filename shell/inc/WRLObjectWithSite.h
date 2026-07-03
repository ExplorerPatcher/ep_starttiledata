#pragma once

#include <ocidl.h>
#include <wrl/implements.h>

#include "GlobalInterfaceTable.h"

class CWRLObjectWithSite
    : public Microsoft::WRL::Implements<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , IObjectWithSite
    >
{
public:
    //~ Begin IObjectWithSite Interface
    STDMETHODIMP SetSite(IUnknown* punkSite) override
    {
        _spunkSite = punkSite;
        return S_OK;
    }

    STDMETHODIMP GetSite(REFIID riid, void** ppvSite) override
    {
        *ppvSite = nullptr;
        return _spunkSite.Get() ? _spunkSite.CopyTo(riid, ppvSite) : E_FAIL;
    }
    //~ End IObjectWithSite Interface

protected:
    Microsoft::WRL::ComPtr<IUnknown> _spunkSite;
};

class CWRLObjectWithGITSite
    : public Microsoft::WRL::Implements<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , IObjectWithSite
    >
{
public:
    CWRLObjectWithGITSite();

    //~ Begin IObjectWithSite Interface
    STDMETHODIMP SetSite(IUnknown* pUnkSite) override;
    STDMETHODIMP GetSite(REFIID riid, void** ppvSite) override;
    //~ End IObjectWithSite Interface

protected:
    ~CWRLObjectWithGITSite();

    template<typename T> Microsoft::WRL::ComPtr<T> Site();
    Microsoft::WRL::ComPtr<IUnknown> SiteUnk();

    RTL_SRWLOCK _lock;
    DWORD _dwGITCookie;
    Microsoft::WRL::ComPtr<IUnknown> _spunkDirectSite;
};

inline CWRLObjectWithGITSite::CWRLObjectWithGITSite()
    : _lock()
    , _dwGITCookie(GlobalInterfaceTable::c_InvalidCookie)
{
}

inline bool IsAgile(IUnknown* punk)
{
    Microsoft::WRL::ComPtr<IAgileObject> spAgileObject;
    return !punk || SUCCEEDED(punk->QueryInterface(IID_PPV_ARGS(&spAgileObject)));
}

inline HRESULT CWRLObjectWithGITSite::SetSite(IUnknown* pUnkSite)
{
    HRESULT hr;

    if (pUnkSite)
    {
        if (IsAgile(this) && !IsAgile(pUnkSite))
        {
            DWORD dwNewGITCookie;
            hr = GlobalInterfaceTable::Marshal(pUnkSite, __uuidof(IUnknown), &dwNewGITCookie);
            if (SUCCEEDED(hr))
            {
                AcquireSRWLockExclusive(&_lock);
                DWORD dwOldCookie = _dwGITCookie;
                _dwGITCookie = dwNewGITCookie;
                ReleaseSRWLockExclusive(&_lock);
                if (dwOldCookie != GlobalInterfaceTable::c_InvalidCookie)
                    GlobalInterfaceTable::Revoke(dwOldCookie);
            }
        }
        else
        {
            AcquireSRWLockExclusive(&_lock);
            _spunkDirectSite = pUnkSite;
            ReleaseSRWLockExclusive(&_lock);
            hr = S_OK;
        }
    }
    else
    {
        AcquireSRWLockExclusive(&_lock);
        Microsoft::WRL::ComPtr<IUnknown> spunkDirectSite = std::move(_spunkDirectSite);
        DWORD dwOldCookie = _dwGITCookie;
        _dwGITCookie = GlobalInterfaceTable::c_InvalidCookie;
        ReleaseSRWLockExclusive(&_lock);
        if (dwOldCookie != GlobalInterfaceTable::c_InvalidCookie)
            GlobalInterfaceTable::Revoke(dwOldCookie);
        else
            spunkDirectSite.Reset();
        hr = S_OK;
    }

    return hr;
}

inline HRESULT CWRLObjectWithGITSite::GetSite(REFIID riid, void** ppvSite)
{
    *ppvSite = nullptr;
    AcquireSRWLockShared(&_lock);

    HRESULT hr;
    DWORD dwCookie;
    if (_spunkDirectSite.Get())
    {
        hr = _spunkDirectSite->QueryInterface(riid, ppvSite);
        dwCookie = 0;
    }
    else
    {
        hr = E_FAIL;
        dwCookie = _dwGITCookie;
    }

    ReleaseSRWLockShared(&_lock);

    if (dwCookie)
        hr = GlobalInterfaceTable::Unmarshal(dwCookie, riid, ppvSite);

    return hr;
}

inline CWRLObjectWithGITSite::~CWRLObjectWithGITSite()
{
    if (_dwGITCookie != GlobalInterfaceTable::c_InvalidCookie)
    {
        GlobalInterfaceTable::Revoke(_dwGITCookie);
        _dwGITCookie = GlobalInterfaceTable::c_InvalidCookie;
    }
}

template <typename T>
Microsoft::WRL::ComPtr<T> CWRLObjectWithGITSite::Site()
{
    Microsoft::WRL::ComPtr<T> spSite;
    GetSite(IID_PPV_ARGS(&spSite));
    return spSite;
}

inline Microsoft::WRL::ComPtr<IUnknown> CWRLObjectWithGITSite::SiteUnk()
{
    return Site<IUnknown>();
}
