#pragma once

class GlobalInterfaceTable
{
public:
    static const UINT c_InvalidCookie = 0;

    static HRESULT Marshal(IUnknown* punk, REFGUID riid, DWORD* pdwCookie)
    {
        *pdwCookie = c_InvalidCookie;
        HRESULT hr = s_Retrieve();
        if (SUCCEEDED(hr))
            hr = s_pGlobalInterfaceTable->RegisterInterfaceInGlobal(punk, riid, pdwCookie);
        return hr;
    }

    static HRESULT Unmarshal(DWORD dwCookie, REFGUID riid, void** ppv)
    {
        *ppv = nullptr;
        HRESULT hr = s_Retrieve();
        if (SUCCEEDED(hr))
            hr = s_pGlobalInterfaceTable->GetInterfaceFromGlobal(dwCookie, riid, ppv);
        return hr;
    }

    static void Revoke(DWORD dwCookie)
    {
        if (dwCookie != c_InvalidCookie)
        {
            HRESULT hr = s_Retrieve();
            if (SUCCEEDED(hr))
                s_pGlobalInterfaceTable->RevokeInterfaceFromGlobal(dwCookie);
        }
    }

private:
    static HRESULT s_Retrieve()
    {
        HRESULT hr = S_OK;
        if (!s_pGlobalInterfaceTable)
        {
            Microsoft::WRL::ComPtr<IGlobalInterfaceTable> spGit;
            hr = CoCreateInstance(CLSID_StdGlobalInterfaceTable, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&spGit));
            if (SUCCEEDED(hr))
            {
                *(IGlobalInterfaceTable**)std::addressof(spGit) = InterlockedCompareExchangePointer((void**)&GlobalInterfaceTable::s_pGlobalInterfaceTable, spGit.Get(), nullptr) != nullptr ? spGit.Get() : nullptr;
            }
        }
        return hr;
    }

    inline static IGlobalInterfaceTable* s_pGlobalInterfaceTable = nullptr;
};
