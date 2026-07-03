#pragma once

#include "ResultUtils.h"

EXTERN_C inline HRESULT SHGetUIObjectFromFullPIDL(PCIDLIST_ABSOLUTE pidl, HWND hwnd, REFIID riid, void** ppv)
{
    *ppv = nullptr;

    const ITEMID_CHILD* pidlChild;
    IShellFolder* psf;
    HRESULT hr = SHBindToParent(pidl, IID_PPV_ARGS(&psf), &pidlChild);
    if (SUCCEEDED(hr))
    {
        hr = psf->GetUIObjectOf(hwnd, 1, &pidlChild, riid, nullptr, ppv);
        psf->Release();
    }

    return hr;
}

static inline WORD g_cfURL_Storage;
static inline WORD g_cfHIDA_Storage;
/*static inline WORD g_cfFileDescA_Storage;
static inline WORD g_cfFileDescW_Storage;
static inline WORD g_cfOFFSETS_Storage;
static inline WORD g_cfEnterpriseId_Storage;
static inline WORD g_cfAsyncFlag_Storage;*/

inline void _InitClipboardFormats()
{
    // if (!g_cfAsyncFlag_Storage)
    if (!g_cfHIDA_Storage)
    {
        g_cfURL_Storage = RegisterClipboardFormatW(L"UniformResourceLocator");
        g_cfHIDA_Storage = RegisterClipboardFormatW(L"Shell IDList Array");
        /*g_cfFileDescA_Storage = RegisterClipboardFormatW(L"FileGroupDescriptor");
        g_cfFileDescW_Storage = RegisterClipboardFormatW(L"FileGroupDescriptorW");
        g_cfOFFSETS_Storage = RegisterClipboardFormatW(L"Shell Object Offsets");
        g_cfEnterpriseId_Storage = RegisterClipboardFormatW(L"EnterpriseDataProtectionId");
        g_cfAsyncFlag_Storage = RegisterClipboardFormatW(L"AsyncFlag");
        RegisterClipboardFormatW(L"FilePropertyStore");
        RegisterClipboardFormatW(L"FilePropertyStoreByteSize");*/
    }
}

inline void ReleaseStgMediumHGLOBAL(void* pv, STGMEDIUM* pmedium)
{
    if (pmedium->hGlobal && pmedium->tymed == TYMED_HGLOBAL)
    {
#ifdef _DEBUG
        if (pv)
        {
            void* pvT = GlobalLock(pmedium->hGlobal);
            _ASSERT(pvT == pv);
            GlobalUnlock(pmedium->hGlobal);
        }
#endif
        GlobalUnlock(pmedium->hGlobal);
    }
    else
    {
        _ASSERTE(0);
    }

    ReleaseStgMedium(pmedium);
}

inline HRESULT SHGetItemArrayFromDataObjEx(IDataObject* pdtobj, CLIPFORMAT cf, STGMEDIUM* pmedium, CIDA** ppida)
{
    *pmedium = {};
    *ppida = nullptr;

    FORMATETC fmte;
    fmte.cfFormat = cf;
    fmte.ptd = nullptr;
    fmte.dwAspect = DVASPECT_CONTENT;
    fmte.lindex = -1;
    fmte.tymed = TYMED_HGLOBAL;

    HRESULT hr = pdtobj->GetData(&fmte, pmedium);
    if (SUCCEEDED(hr))
    {
        *ppida = (CIDA*)GlobalLock(pmedium->hGlobal);
        if (!*ppida)
        {
            ReleaseStgMedium(pmedium);
            hr = ResultFromKnownLastError();
        }
    }

    return hr;
}

inline HRESULT SHGetItemArrayFromDataObj(IDataObject* pdtobj, STGMEDIUM* pmedium, CIDA** ppida)
{
    _InitClipboardFormats();
    return SHGetItemArrayFromDataObjEx(pdtobj, g_cfHIDA_Storage, pmedium, ppida);
}

#define HIDA_GetPIDLFolder(pida)        (PCIDLIST_ABSOLUTE)(((LPBYTE)pida)+(pida)->aoffset[0])
#define HIDA_GetPIDLItem(pida, i)       (PCUIDLIST_RELATIVE)(((LPBYTE)pida)+(pida)->aoffset[i+1])

inline PIDLIST_ABSOLUTE IDA_ILClone(CIDA* pida, UINT i)
{
    if (i < pida->cidl)
        return ILCombine(HIDA_GetPIDLFolder(pida), HIDA_GetPIDLItem(pida, i));
    return nullptr;
}

inline HRESULT DataObj_GetPath(IDataObject* pdtobj, WCHAR* pszPath, UINT cchPath)
{
    *pszPath = 0;

    FORMATETC fmte;
    fmte.cfFormat = CF_HDROP;
    fmte.ptd = nullptr;
    fmte.dwAspect = DVASPECT_CONTENT;
    fmte.lindex = -1;
    fmte.tymed = TYMED_HGLOBAL;

    STGMEDIUM medium;
    HRESULT hr = pdtobj->GetData(&fmte, &medium);
    if (SUCCEEDED(hr))
    {
        hr = DragQueryFileW((HDROP)medium.hGlobal, 0, pszPath, cchPath) != 0 ? S_OK : E_FAIL;
        ReleaseStgMedium(&medium);
    }

    return hr;
}

inline void* DataObj_GetDataOfType(IDataObject* pdtobj, UINT cfType, STGMEDIUM* pstg)
{
    void* pret = nullptr;

    FORMATETC fmte;
    fmte.cfFormat = (CLIPFORMAT)cfType;
    fmte.ptd = nullptr;
    fmte.dwAspect = DVASPECT_CONTENT;
    fmte.lindex = -1;
    fmte.tymed = TYMED_HGLOBAL;

    if (pdtobj->GetData(&fmte, pstg) == S_OK)
    {
        pret = GlobalLock(pstg->hBitmap);
        if (!pret)
        {
            ReleaseStgMedium(pstg);
        }
    }

    return pret;
}

inline HRESULT DataObj_GetIDList(IDataObject* pdtObj, DATAOBJ_GET_ITEM_FLAGS dwFlags, PIDLIST_ABSOLUTE* ppidlTarget)
{
    *ppidlTarget = nullptr;
    HRESULT hr = E_INVALIDARG;

    if (pdtObj)
    {
        _InitClipboardFormats();

        STGMEDIUM medium;
        CIDA* pida;
        hr = SHGetItemArrayFromDataObj(pdtObj, &medium, &pida);
        if (SUCCEEDED(hr))
        {
            if ((dwFlags & DOGIF_ONLY_IF_ONE) == 0 || pida->cidl == 1)
            {
                *ppidlTarget = IDA_ILClone(pida, 0);
                hr = *ppidlTarget ? S_OK : E_OUTOFMEMORY;
            }
            else
            {
                hr = E_FAIL;
            }
            ReleaseStgMediumHGLOBAL(pida, &medium);
        }
        else
        {
            WCHAR szPath[2084];
            if ((dwFlags & DOGIF_NO_HDROP) == 0)
            {
                hr = DataObj_GetPath(pdtObj, szPath, ARRAYSIZE(szPath));
                if (SUCCEEDED(hr))
                {
                    dwFlags |= DOGIF_NO_URL;
                    hr = SHParseDisplayName(szPath, nullptr, ppidlTarget, 0, nullptr);
                }
            }

            if (FAILED(hr) && (dwFlags & DOGIF_NO_URL) == 0)
            {
                char* pszPath = (char*)DataObj_GetDataOfType(pdtObj, g_cfURL_Storage, &medium);
                if (pszPath)
                {
#ifdef _DEBUG
                    size_t cchPathLen;
                    _ASSERTE(SUCCEEDED(StringCchLengthA(pszPath, ARRAYSIZE(szPath), &cchPathLen)));
#endif
                    SHAnsiToUnicode(pszPath, szPath, ARRAYSIZE(szPath));
                    hr = SHParseDisplayName(szPath, nullptr, ppidlTarget, 0, nullptr);
                    ReleaseStgMediumHGLOBAL(pszPath, &medium);
                }
            }
        }

        if (SUCCEEDED(hr) && (dwFlags & DOGIF_TRAVERSE_LINK) != 0)
        {
            IShellLinkW* psl;
            if (SUCCEEDED(SHGetUIObjectFromFullPIDL(*ppidlTarget, nullptr, IID_PPV_ARGS(&psl))))
            {
                PIDLIST_ABSOLUTE pidlTarget;
                if (SUCCEEDED(psl->GetIDList(&pidlTarget)) && pidlTarget)
                {
                    ILFree(*ppidlTarget);
                    *ppidlTarget = pidlTarget;
                }

                psl->Release();
            }
        }
    }

    return hr;
}
