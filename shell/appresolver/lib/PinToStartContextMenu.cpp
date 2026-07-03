#include "pch.h"

#include <shellapi.h>
#include <ShlObj_core.h>
#include <Shlwapi.h>
#include <wrl/module.h>

#include "CallerIdentity.h"
#include "ComTaskPool.h"
#include "dobjutil.h"
#include "IApplicationResolver.h"
#include "WRLObjectWithSite.h"

#include <WindowsInternal.Shell.LauncherPolicy.h>
#include <WindowsInternal.Shell.UnifiedTile.h>

using namespace Microsoft::WRL;

namespace wstor = ABI::Windows::Storage;
namespace launcherpolicy = ABI::WindowsInternal::Shell::LauncherPolicy;
namespace ut = ABI::WindowsInternal::Shell::UnifiedTile;
namespace utctc = ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections;

STDAPI_(DWORD) SHExpandEnvironmentStringsW(const WCHAR* pszIn, WCHAR* pszOut, DWORD cchOut);

struct ICIVERBTOIDMAP
{
    LPCWSTR pszCmd;
    LPCSTR pszCmdA;
    UINT idCmd;
    UINT idsHelpText;
};

const ICIVERBTOIDMAP* _CmdIDToMap(UINT_PTR idCmd, BOOL fUnicode, const ICIVERBTOIDMAP* pmap)
{
    if (IS_INTRESOURCE(idCmd))
    {
        while (pmap->idCmd != -1)
        {
            if (pmap->idCmd == (UINT)idCmd)
            {
                return pmap;
            }
            ++pmap;
        }
    }
    else
    {
        if (fUnicode)
        {
            while (pmap->idCmd != -1)
            {
                if (pmap->pszCmd && StrCmpICW((LPCWSTR)idCmd, pmap->pszCmd) == 0)
                {
                    return pmap;
                }
                ++pmap;
            }
        }
        else
        {
            while (pmap->idCmd != -1)
            {
                if (pmap->pszCmdA && StrCmpICA((LPCSTR)idCmd, pmap->pszCmdA) == 0)
                {
                    return pmap;
                }
                ++pmap;
            }
        }
    }

    return nullptr;
}

HRESULT SHMapICIVerbToCmdID(CMINVOKECOMMANDINFO* pici, const ICIVERBTOIDMAP* pmap, UINT* pid)
{
    HRESULT hr = E_FAIL;
    UINT idCmd = 0; // Assumed initial value

    if (IS_INTRESOURCE(pici->lpVerb))
    {
        idCmd = (WORD)(UINT_PTR)pici->lpVerb;
        hr = S_OK;
    }
    else
    {
        BOOL fUnicode = pici->cbSize >= sizeof(CMINVOKECOMMANDINFOEX)
            && (pici->fMask & CMIC_MASK_UNICODE) != 0
            && ((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW;
        pmap = _CmdIDToMap(
            fUnicode ? (UINT_PTR)((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW : (UINT_PTR)pici->lpVerb, fUnicode, pmap);
        if (pmap)
        {
            idCmd = pmap->idCmd;
            hr = S_OK;
        }
    }

    if (SUCCEEDED(hr))
    {
        *pid = idCmd;
    }

    return hr;
}

HRESULT SHMapCmdIDToVerb(UINT_PTR idCmd, UINT uFlags, const ICIVERBTOIDMAP* pmap, LPSTR pszName, UINT cchMax)
{
    BOOL fSuccess;

    pmap = _CmdIDToMap(idCmd, uFlags & GCS_VERBW, pmap);
    if (uFlags & GCS_VERBW)
    {
        fSuccess = pmap && pmap->pszCmd != nullptr;

        LPCWSTR pszVerb = fSuccess ? pmap->pszCmd : L"";
        StringCchCopyW((wchar_t*)pszName, cchMax, pszVerb);
    }
    else
    {
        fSuccess = pmap && pmap->pszCmdA != nullptr;

        LPCSTR pszVerb = fSuccess ? pmap->pszCmdA : "";
        StringCchCopyA(pszName, cchMax, pszVerb);
    }

    return fSuccess ? S_OK : E_FAIL;
}

EXTERN_C HRESULT SHMapCmdIDToHelpText(
    UINT_PTR idCmd, UINT uFlags, HINSTANCE hinst, const ICIVERBTOIDMAP* pmap, LPSTR pszName, UINT cchMax)
{
    HRESULT hr = E_FAIL;

    pmap = _CmdIDToMap(idCmd, uFlags & GCS_VERBW, pmap);
    if (pmap)
    {
        if (pmap->idsHelpText)
        {
            int iRet;
            if (uFlags & GCS_VERBW)
            {
                iRet = LoadStringW(hinst, pmap->idsHelpText, (LPWSTR)pszName, cchMax);
            }
            else
            {
                iRet = LoadStringA(hinst, pmap->idsHelpText, (LPSTR)pszName, cchMax);
            }
            hr = iRet ? S_OK : E_OUTOFMEMORY;
        }
    }

    if (FAILED(hr))
    {
        if (uFlags & GCS_VERBW)
        {
            StringCchCopyW((LPWSTR)pszName, cchMax, L"");
        }
        else
        {
            StringCchCopyA((LPSTR)pszName, cchMax, "");
        }
    }

    return hr;
}

EXTERN_C HRESULT SHValidateCmdID(UINT_PTR idCmd, UINT uFlags, const ICIVERBTOIDMAP* pmap)
{
    return _CmdIDToMap(idCmd, uFlags & GCS_VERBW, pmap) ? S_OK : S_FALSE;
}

enum class ContextMenuItemAction
{
    Invalid,
    Pin,
    PinUserCreatedItem,
    Unpin,
};

class DECLSPEC_UUID("470c0ebd-5d73-4d58-9ced-e91e22e23282")
StartPinUnpinContextMenu final
    : public RuntimeClass<RuntimeClassFlags<ClassicCom>
        , IContextMenu
        , IShellExtInit
        , CWRLObjectWithSite
    >
{
public:
    // ReSharper disable once CppHidingFunction
    HRESULT RuntimeClassInitialize();

    //~ Begin IContextMenu Interface
    STDMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags) override;
    STDMETHODIMP InvokeCommand(CMINVOKECOMMANDINFO* pici) override;
    STDMETHODIMP GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT* pwReserved, LPSTR pszName, UINT cchMax) override;
    //~ End IContextMenu Interface

    //~ Begin IShellExtInit Interface
    STDMETHODIMP Initialize(PCIDLIST_ABSOLUTE pidlFolder, IDataObject* pDataObj, HKEY hKeyProgID) override;
    //~ End IShellExtInit Interface

private:
    static HRESULT ValidateCallerIdentity();
    static HRESULT IsShellItemBlockedFromPinning(IShellItem* pShellItem, bool* pfBlocked);

    HRESULT QueryContextMenuImpl(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    HRESULT InvokeCommandImpl(CMINVOKECOMMANDINFO* pici);

    HRESULT _RetrieveShellItemForMenu(IShellItem** ppShellItem);
    HRESULT _GetDisplayStringForAction(const ContextMenuItemAction& action, WCHAR* pszDisplay, UINT cchDisplay);
    HRESULT GetLayoutCustomizationRestrictionType(utctc::StartCollectionCustomizationRestrictionType* pRestriction) const;

    wil::com_ptr_nothrow<IDataObject> _spDataObject;
    bool _fUnk = false;
    ContextMenuItemAction _itemAction = ContextMenuItemAction::Invalid;
    wil::com_ptr_nothrow<utctc::ICuratedTileCollectionManager> _spCuratedTileCollectionManager;
    wil::com_ptr_nothrow<launcherpolicy::ILauncherPolicy> _spLauncherPolicy;
    wil::com_ptr_nothrow<ut::IUnifiedTileIdentifier> _spUnifiedTileIdentifier;
    wil::com_ptr_nothrow<ut::IUnifiedTileIdentifierExtractorStatics> _spUnifiedTileIdentifierExtractorStatics;
    wil::com_ptr_nothrow<wstor::IStorageItem> _spStorageItem;
    // wil::com_ptr_nothrow<ABI::WindowsUdk::UI::StartScreen::IStartScreenManagerExtensionStatics> _spStartScreenManagerExtensionStatics;
};

const ICIVERBTOIDMAP c_avidmStartPinUnpin[] =
{
    { L"PinToStartScreen", "PinToStartScreen", 1, 0 },
    { L"UnpinFromStartScreen", "UnpinFromStartScreen", 2, 0 },
    { nullptr, nullptr, (UINT)-1, 0 }
};

// Implementation based on 22621
HRESULT StartPinUnpinContextMenu::RuntimeClassInitialize()
{
    _fUnk = false;

    // _spCuratedTileCollectionManager = ref new utcurated::CuratedTileCollectionManager();
    // _spLauncherPolicy = WindowsInternal::Shell::LauncherPolicy::LauncherPolicy::GetForUser(nullptr);

    // Pre-22000 has this block after the CuratedTileCollectionManager query
    wil::com_ptr_nothrow<launcherpolicy::ILauncherPolicyStatics> spLauncherPolicyStatics;
    RETURN_IF_FAILED(RoGetActivationFactory(
        Wrappers::HStringReference(RuntimeClass_WindowsInternal_Shell_LauncherPolicy_LauncherPolicy).Get(),
        IID_PPV_ARGS(&spLauncherPolicyStatics)));
    RETURN_IF_FAILED(spLauncherPolicyStatics->GetForUser(nullptr, &_spLauncherPolicy));

    /*if (WI_IsFeatureEnabled(Feature_STest03) && StartDocked::ShouldUseStartDocked())
    {
        RETURN_IF_FAILED(StartDocked::GetStartScreenManagerExtensionStatics(&_spStartScreenManagerExtensionStatics));
    }*/

    // if (_spStartScreenManagerExtensionStatics == nullptr)
    {
        wil::com_ptr_nothrow<IInspectable> spCuratedTileCollectionManagerInspectable;
        RETURN_IF_FAILED(RoActivateInstance(
            Wrappers::HStringReference(RuntimeClass_WindowsInternal_Shell_UnifiedTile_CuratedTileCollections_CuratedTileCollectionManager).Get(),
            &spCuratedTileCollectionManagerInspectable));
        RETURN_IF_FAILED(spCuratedTileCollectionManagerInspectable.query_to(&_spCuratedTileCollectionManager));
    }

    return S_OK;
}

HRESULT StartPinUnpinContextMenu::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    using namespace Windows::Internal;

    HRESULT hr = S_OK;
    auto lambda = [this, &hr, hmenu, indexMenu, idCmdFirst, idCmdLast, uFlags]() -> HRESULT
    {
        hr = QueryContextMenuImpl(hmenu, indexMenu, idCmdFirst, idCmdLast, uFlags);
        return hr;
    };
    RETURN_IF_FAILED(ComTaskPool::QueueTask(TaskApartment::STAIfSupported, TaskOptions::WaitWithCalls, lambda)); // 140
    return hr;
}

HRESULT StartPinUnpinContextMenu::InvokeCommand(CMINVOKECOMMANDINFO* pici)
{
    using namespace Windows::Internal;

    RETURN_IF_FAILED_EXPECTED(ValidateCallerIdentity());
    HRESULT hr = S_OK;
    auto lambda = [this, &hr, pici]() -> void
    {
        hr = InvokeCommandImpl(pici);
    };
    RETURN_IF_FAILED(ComTaskPool::QueueTask(TaskApartment::STAIfSupported, TaskOptions::WaitWithCalls, lambda)); // 299
    return hr;
}

HRESULT StartPinUnpinContextMenu::GetCommandString(
    UINT_PTR idCmd, UINT uFlags, UINT* pwReserved, LPSTR pszName, UINT cchMax)
{
    HRESULT hr = E_NOTIMPL;

    switch (uFlags)
    {
        case GCS_VERBA:
        case GCS_VERBW:
        {
            if (_itemAction == ContextMenuItemAction::Unpin)
            {
                if ((uFlags & GCS_VERBW) != 0)
                {
                    RETURN_IF_FAILED(StringCchCopyW((WCHAR*)pszName, cchMax, L"UnpinFromStartScreen")); // 400
                }
                else
                {
                    RETURN_IF_WIN32_BOOL_FALSE(WideCharToMultiByte(CP_ACP, 0, L"UnpinFromStartScreen", -1, pszName, cchMax, nullptr, nullptr)); // 404
                }
                hr = S_OK;
            }
            else
            {
                hr = SHMapCmdIDToVerb(idCmd, uFlags, c_avidmStartPinUnpin, pszName, cchMax);
            }
            break;
        }
        case GCS_HELPTEXTA:
        case GCS_HELPTEXTW:
        {
            hr = SHMapCmdIDToHelpText(idCmd, uFlags, HINST_THISCOMPONENT, c_avidmStartPinUnpin, pszName, cchMax);
            break;
        }
        case GCS_VALIDATEA:
        case GCS_VALIDATEW:
        {
            hr = SHValidateCmdID(idCmd, uFlags, c_avidmStartPinUnpin);
            break;
        }
    }

    return hr;
}

HRESULT StartPinUnpinContextMenu::Initialize(PCIDLIST_ABSOLUTE pidlFolder, IDataObject* pDataObj, HKEY hKeyProgID)
{
    _spDataObject = pDataObj;
    return S_OK;
}

HRESULT StartPinUnpinContextMenu::ValidateCallerIdentity()
{
    wil::unique_handle hProcess;
    RETURN_IF_FAILED(CallerIdentity::GetCallingProcessHandle(
        PROCESS_QUERY_LIMITED_INFORMATION, RCC_ALLOW_IF_RUNTIMEBROKER, &hProcess)); // 566

    WCHAR szProcessPath[260];
    DWORD dwSize = ARRAYSIZE(szProcessPath);
    RETURN_IF_WIN32_BOOL_FALSE(QueryFullProcessImageNameW(hProcess.get(), 0, szProcessPath, &dwSize)); // 572

    WCHAR szExplorer[260];
    RETURN_LAST_ERROR_IF(SHExpandEnvironmentStringsW(L"%SystemRoot%\\explorer.exe", szExplorer, ARRAYSIZE(szExplorer)) == 0); // 575
    if (CompareStringOrdinal(szProcessPath, dwSize, szExplorer, -1, TRUE) == CSTR_EQUAL)
    {
        return S_OK;
    }

    bool fTemp;
    return SUCCEEDED(CallerIdentity::IsProcessAppContainer(hProcess.get(), &fTemp)) && fTemp
        && SUCCEEDED(CallerIdentity::CheckCapabilityFromProcessHandle(hProcess.get(), L"shellExperience", &fTemp)) && fTemp
        ? S_OK : E_ACCESSDENIED;
}

HRESULT StartPinUnpinContextMenu::IsShellItemBlockedFromPinning(IShellItem* pShellItem, bool* pfBlocked)
{
    *pfBlocked = false;

    SFGAOF sfgaof;
    if (pShellItem->GetAttributes(SFGAO_FOLDER, &sfgaof) != S_OK) // SFGAO_VALIDATE
    {
        ComPtr<IApplicationResolver2> spAppResolver2;
        RETURN_IF_FAILED(CoCreateInstance(
            CLSID_StartMenuCacheAndAppResolver, nullptr, CLSCTX_INPROC, IID_PPV_ARGS(&spAppResolver2))); // 265

        wil::unique_cotaskmem_string spszAppId;
        *pfBlocked = spAppResolver2->GetLauncherAppIDForItemEx(
            pShellItem, ARIFF_REQUIRE_PREVENT_PINNING_NOT_SET, &spszAppId) == 0x80040F07;
    }

    return S_OK;
}

enum STORAGEITEM_CREATION_FLAGS
{
    SICF_DEFAULT = 0,
    SICF_DELETE_NOT_ALLOWED_IF_CONTAINER = 0x1,
    SICF_ALLOW_NAMESPACE_JUNCTION = 0x2,
    SICF_ADVANCED_SHAPES_ALLOWED = 0x4,
    SICF_FILTER_ON_MANIFESTED_TYPES = 0x8,
    SICF_FILTER_ON_MANIFESTED_KINDS = 0x10,
    SICF_DONT_CHECK_PATH_FOR_EXISTENCE = 0x20,
    SICF_FORCE_READ_ONLY = 0x40,
    SICF_INHERITS_LOWIL = 0x80,
    SICF_DISABLE_AQS = 0x100,
    SICF_DISABLE_DEEPQUERY = 0x200,
    SICF_NEIGHBORING_FILES_QUERY = 0x400,
    SICF_DEFAULT_ORDER_BY_DATE = 0x800,
    SICF_NO_FOLDER_CREATE = 0x1000,
    SICF_VALIDATED_ACCESS = 0x2000,
    SICF_CONTENT_INDEXER = 0x4000,
    SICF_ENABLE_FOLDER_REPARSE_TAGS = 0x8000,
    SICF_OR_FILTER_OVER_MANIFESTED_INFO = 0x10000,
    SICF_ALLOW_FULL_NAMESPACE = 0x20000,
    SICF_ALLOW_LINKS = 0x40000,
    SICF_ALLOW_NON_STREAM_FILES = 0x80000,
    SICF_FOLDERID_CAPABILITY_BACKED = 0x100000,
    SICF_BROADFS_CAPABILITY_BACKED = 0x200000,
    SICF_INHERITED_FLAGS_MASK = 0x4 | 0x8 | 0x10 | 0x40 | 0x80 | 0x2000 | 0x4000 | 0x10000 | 0x20000 | 0x40000 | 0x80000 | 0x100000 | 0x200000,
};

interface IStorageProviderPropertyHandler;

struct STORAGEITEM_FROM_SHELLITEM_CREATE_OPTIONS
{
    STORAGEITEM_CREATION_FLAGS sicfFlags;
    const WCHAR* packageFamilyName;
    HANDLE processHandle;
    IStorageProviderPropertyHandler* storageProviderPropertyHandler;
};

// HRESULT CreateStorageItemFromShellItem(IShellItem* psi, const STORAGEITEM_FROM_SHELLITEM_CREATE_OPTIONS& options, REFIID riid, void** ppv); // windows.storage.dll ordinal 942

typedef HRESULT (WINAPI *CreateStorageItemFromShellItem_t)(IShellItem* psi, const STORAGEITEM_FROM_SHELLITEM_CREATE_OPTIONS& options, REFIID riid, void** ppv);
HRESULT CreateStorageItemFromShellItem(IShellItem* psi, const STORAGEITEM_FROM_SHELLITEM_CREATE_OPTIONS& options, REFIID riid, void** ppv)
{
    static CreateStorageItemFromShellItem_t fn = nullptr;
    if (!fn)
    {
        HMODULE h = GetModuleHandleW(L"windows.storage.dll");
        if (h)
            fn = (CreateStorageItemFromShellItem_t)GetProcAddress(h, MAKEINTRESOURCEA(942));
        FAIL_FAST_IF_NULL(fn);
    }
    return fn(psi, options, riid, ppv);
}

// Implementation based on 22621
HRESULT StartPinUnpinContextMenu::QueryContextMenuImpl(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    HRESULT hr = S_OK;

    if ((uFlags & CMF_DEFAULTONLY) == 0 && (uFlags & CMF_OPTIMIZEFORINVOKE) == 0)
    {
        utctc::StartCollectionCustomizationRestrictionType restriction;
        LOG_IF_FAILED(GetLayoutCustomizationRestrictionType(&restriction)); // 153
        if (restriction != utctc::StartCollectionCustomizationRestrictionType_FullLayout)
        {
            // AppResolverTelemetry::GetStartPinContextMenuCommand telemetry;
            // telemetry.StartActivity();

            ComPtr<IShellItem> spShellItem;
            RETURN_IF_FAILED(_RetrieveShellItemForMenu(&spShellItem)); // 159

            if (_spStorageItem == nullptr)
            {
                STORAGEITEM_FROM_SHELLITEM_CREATE_OPTIONS options = {};
                RETURN_IF_FAILED(CreateStorageItemFromShellItem(
                    spShellItem.Get(), options, IID_PPV_ARGS(&_spStorageItem))); // 165
            }

            if (_spUnifiedTileIdentifierExtractorStatics == nullptr)
            {
                RETURN_IF_FAILED(RoGetActivationFactory(
                    Wrappers::HStringReference(RuntimeClass_WindowsInternal_Shell_UnifiedTile_UnifiedTileIdentifierExtractor).Get(),
                    IID_PPV_ARGS(&_spUnifiedTileIdentifierExtractorStatics))); // 170
            }

            if (SUCCEEDED(_spUnifiedTileIdentifierExtractorStatics->ExtractFromStorageItem(_spStorageItem.get(), &_spUnifiedTileIdentifier)) && _spUnifiedTileIdentifier != nullptr)
            {
                /*if (WI_IsFeatureEnabled(Feature_STest03) && StartDocked::ShouldUseStartDocked() && _spStartScreenManagerExtensionStatics != nullptr)
                {
                    boolean fPinned = FALSE;
                    RETURN_IF_FAILED(StartDocked::IsPinnedToStart(_spStartScreenManagerExtensionStatics.get(), _spUnifiedTileIdentifier.get(), &fPinned)); // 178
                    _itemAction = fPinned ? ContextMenuItemAction::Unpin : ContextMenuItemAction::Pin;
                }
                else*/
                {
                    ComPtr<utctc::ICuratedTileCollection> spStartTileGridCollection;
                    RETURN_IF_FAILED(_spCuratedTileCollectionManager->GetCollection(
                        Wrappers::HStringReference(L"Start.TileGrid").Get(), &spStartTileGridCollection)); // 185

                    ComPtr<utctc::ICuratedTileGroup> spGroup;
                    boolean fFound = FALSE;
                    RETURN_IF_FAILED(spStartTileGridCollection->FindTileAndParentGroup(_spUnifiedTileIdentifier.get(), nullptr, &spGroup, &fFound)); // 189
                    if (fFound)
                    {
                        if (restriction != utctc::StartCollectionCustomizationRestrictionType_OnlySpecifiedGroups)
                        {
                            _itemAction = ContextMenuItemAction::Unpin;
                        }
                        else
                        {
                            boolean fGroupLocked;
                            if (FAILED_LOG(spGroup->get_LockedForCustomization(&fGroupLocked)) || !fGroupLocked) // 199
                            {
                                _itemAction = ContextMenuItemAction::Unpin;
                            }
                        }
                    }
                    else
                    {
                        _itemAction = ContextMenuItemAction::Pin;
                    }
                }

                if (_itemAction == ContextMenuItemAction::Pin)
                {
                    ut::UnifiedTileIdentifierKind kind;
                    if (FAILED_LOG(_spUnifiedTileIdentifier->get_Kind(&kind)) || kind == ut::UnifiedTileIdentifierKind_Win32)
                    {
                        bool fBlocked;
                        if (SUCCEEDED_LOG(IsShellItemBlockedFromPinning(spShellItem.Get(), &fBlocked)) && fBlocked) // 220
                        {
                            _itemAction = ContextMenuItemAction::Invalid;
                        }
                    }
                }
            }
            else
            {
                bool fBlocked;
                if (FAILED_LOG(IsShellItemBlockedFromPinning(spShellItem.Get(), &fBlocked)) || !fBlocked) // 231
                {
                    _itemAction = ContextMenuItemAction::PinUserCreatedItem;
                }
            }

            if (_itemAction != ContextMenuItemAction::Invalid)
            {
                WCHAR szDisplay[128];
                RETURN_IF_FAILED(_GetDisplayStringForAction(_itemAction, szDisplay, ARRAYSIZE(szDisplay))); // 239
                InsertMenuW(hmenu, indexMenu, MF_BYPOSITION, idCmdFirst + 1, szDisplay);
                // telemetry.Stop(S_OK);
                hr = MAKE_HRESULT(SEVERITY_SUCCESS, 0, 2);
            }
        }
    }

    return hr;
}

class DECLSPEC_UUID("fae517e7-1b31-47b6-840b-14d7d910ad48")
UserPinnedTileManager;

interface IVisualElementEnumerator;
interface ITileManager;
interface ITileInfo;

MIDL_INTERFACE("d041a5b6-3cc5-4ef2-8a18-cbba20e4469e")
IUserPinnedTileManager : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE Initialize(IVisualElementEnumerator*, ITileManager*) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreatePinnedItemFromShellItem(IShellItem*) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreatePinnedItemFromShellLink(IShellLinkW*) = 0;
    virtual HRESULT STDMETHODCALLTYPE DeletePinnedItem(ITileInfo*) = 0;
    virtual HRESULT STDMETHODCALLTYPE DeletePinnedItemByAppId(const WCHAR*) = 0;
};

// Implementation based on 22621
HRESULT StartPinUnpinContextMenu::InvokeCommandImpl(CMINVOKECOMMANDINFO* pici)
{
    UINT idCmd = _itemAction == ContextMenuItemAction::Unpin ? 2 : 1;
    HRESULT hr = pici->lpVerb == MAKEINTRESOURCEA(idCmd) ? S_OK : SHMapICIVerbToCmdID(pici, c_avidmStartPinUnpin, &idCmd);
    if (SUCCEEDED(hr) && (idCmd == 1 || idCmd == 2)
        && _itemAction != ContextMenuItemAction::Invalid
        && _spStorageItem != nullptr
        && _spUnifiedTileIdentifierExtractorStatics != nullptr
        && (_spUnifiedTileIdentifier != nullptr || _itemAction == ContextMenuItemAction::PinUserCreatedItem))
    {
        if (_itemAction == ContextMenuItemAction::PinUserCreatedItem)
        {
            // AppResolverTelemetry::PinUserCreatedItemToStart telemetry;
            // telemetry.StartActivity();

            ComPtr<IUserPinnedTileManager> spUserPinnedTileManager;
            RETURN_IF_FAILED(CoCreateInstance(
                __uuidof(UserPinnedTileManager), nullptr, CLSCTX_INPROC, IID_PPV_ARGS(&spUserPinnedTileManager))); // 326

            ComPtr<IShellItem> spShellItem;
            RETURN_IF_FAILED(_RetrieveShellItemForMenu(&spShellItem)); // 329

            ComPtr<IShellLinkW> spShellLink;
            if (SUCCEEDED(IUnknown_QueryService(_spunkSite.Get(), IID_IShellLinkW, IID_PPV_ARGS(&spShellLink))))
            {
                RETURN_IF_FAILED(spUserPinnedTileManager->CreatePinnedItemFromShellLink(spShellLink.Get())); // 333
            }
            else
            {
                RETURN_IF_FAILED(spUserPinnedTileManager->CreatePinnedItemFromShellItem(spShellItem.Get())); // 337
            }

            hr = S_OK;
        }
        else
        {
            /*if (WI_IsFeatureEnabled(Feature_STest03) && _spStartScreenManagerExtensionStatics != nullptr)
            {
                switch (_itemAction) // @Note: Turned into switch statement due to line number clues
                {
                    case ContextMenuItemAction::Unpin:
                        RETURN_IF_FAILED(StartDocked::UnpinFromStart(_spStartScreenManagerExtensionStatics.get(), _spUnifiedTileIdentifier.get())); // 349
                        return hr;
                    case ContextMenuItemAction::Pin:
                        RETURN_IF_FAILED(StartDocked::PinToStart(_spStartScreenManagerExtensionStatics.get(), _spUnifiedTileIdentifier.get())); // 352
                        return hr;
                    default:
                        RETURN_HR(E_UNEXPECTED); // 355
                }
            }*/

            wil::com_ptr_nothrow<utctc::ICuratedTileCollection> spStartTileGridCollectionCurated;
            RETURN_IF_FAILED(_spCuratedTileCollectionManager->GetCollection(
                Wrappers::HStringReference(L"Start.TileGrid").Get(), &spStartTileGridCollectionCurated)); // 363
            wil::com_ptr_nothrow<utctc::IStartTileCollection> spStartTileGridCollection;
            RETURN_IF_FAILED(spStartTileGridCollectionCurated.query_to(&spStartTileGridCollection)); // 365

            switch (_itemAction) // @Note: Turned into switch statement due to line number clues
            {
                case ContextMenuItemAction::Unpin:
                    RETURN_IF_FAILED(spStartTileGridCollection->UnpinFromStart(_spUnifiedTileIdentifier.get())); // 370
                    RETURN_IF_FAILED(spStartTileGridCollectionCurated->Commit()); // 371
                    break;
                case ContextMenuItemAction::Pin:
                    RETURN_IF_FAILED(spStartTileGridCollection->PinToStart(_spUnifiedTileIdentifier.get(), utctc::TilePinSize_Tile2x2)); // 374
                    RETURN_IF_FAILED(spStartTileGridCollectionCurated->Commit()); // 375
                    break;
                default:
                    RETURN_HR(E_UNEXPECTED); // 378
            }
        }
    }

    return hr;
}

HRESULT StartPinUnpinContextMenu::_RetrieveShellItemForMenu(IShellItem** ppShellItem)
{
    *ppShellItem = nullptr;

    ComPtr<IPersistFile> spPersistFile;
    if (SUCCEEDED(IUnknown_QueryService(_spunkSite.Get(), IID_IShellLinkW, IID_PPV_ARGS(&spPersistFile))))
    {
        ComPtr<IShellLinkW> spShellLink;
        if (SUCCEEDED(spPersistFile.As(&spShellLink)))
        {
            wil::unique_cotaskmem_ptr<ITEMIDLIST_ABSOLUTE> spidlLink;
            if (SUCCEEDED_LOG(spShellLink->GetIDList(wil::out_param(spidlLink)))) // 490
            {
                ComPtr<IShellFolder> spShellFolder;
                PCUITEMID_CHILD pidlRelative;
                if (SUCCEEDED_LOG(SHBindToParent(spidlLink.get(), IID_PPV_ARGS(&spShellFolder), &pidlRelative))) // 494
                {
                    wil::unique_cotaskmem_ptr<ITEMIDLIST_ABSOLUTE> pidl1;
                    if (SUCCEEDED_LOG(SHGetKnownFolderIDList(/*497*/FOLDERID_AppsFolder, KF_FLAG_DONT_VERIFY, nullptr, wil::out_param(pidl1)))
                        && ILIsParent(pidl1.get(), spidlLink.get(), TRUE))
                    {
                        RETURN_IF_FAILED(SHCreateItemFromIDList(spidlLink.get(), IID_PPV_ARGS(ppShellItem))); // 501
                    }
                }
            }
        }

        if (!*ppShellItem)
        {
            /*CoTaskMemNativeString spszPath;
            RETURN_HR_IF(E_FAIL, !(spPersistFile->GetCurFile(&spszPath) == S_OK && spszPath.HasLength())); // 513
            RETURN_IF_FAILED(SHCreateItemFromParsingName(spszPath.Get(), nullptr, IID_PPV_ARGS(ppShellItem))); // 514*/
            wil::unique_cotaskmem_string spszPath; // @MOD Use wil
            RETURN_HR_IF(E_FAIL, !(spPersistFile->GetCurFile(&spszPath) == S_OK && (spszPath.get() && *spszPath.get()))); // 513
            RETURN_IF_FAILED(SHCreateItemFromParsingName(spszPath.get(), nullptr, IID_PPV_ARGS(ppShellItem))); // 514
        }
    }
    else
    {
        wil::unique_cotaskmem_ptr<ITEMIDLIST_ABSOLUTE> spidl;
        RETURN_IF_FAILED(DataObj_GetIDList(_spDataObject.get(), DOGIF_NO_HDROP | DOGIF_ONLY_IF_ONE, wil::out_param(spidl))); // 520
        RETURN_IF_FAILED(SHCreateItemFromIDList(spidl.get(), IID_PPV_ARGS(ppShellItem))); // 521
    }

    return S_OK;
}

HRESULT StartPinUnpinContextMenu::_GetDisplayStringForAction(
    const ContextMenuItemAction& action, WCHAR* pszDisplay, UINT cchDisplay)
{
    /*if (WI_IsFeatureEnabled(Feature_AccessibilityXAMLContextMenu))
    {
        const WCHAR* pszSource = action == ContextMenuItemAction::Unpin
            ? L"@%windir%\\system32\\shell32.dll,-51609"  // Unpi&n from Start
            : L"@%windir%\\system32\\shell32.dll,-51606"; // &Pin to Start
        RETURN_HR(SHLoadIndirectString(pszSource, pszDisplay, cchDisplay, nullptr)); // 540
    }
    else
    {
        const WCHAR* pszSource = action == ContextMenuItemAction::Unpin
            ? L"@%windir%\\system32\\shell32.dll,-51394"  // Un&pin from Start
            : L"@%windir%\\system32\\shell32.dll,-51201"; // &Pin to Start
        RETURN_HR(SHLoadIndirectString(pszSource, pszDisplay, cchDisplay, nullptr)); // 551
    }*/

    // @MOD Don't use feature flags
    const WCHAR* pszSource = action == ContextMenuItemAction::Unpin
        ? L"@%windir%\\system32\\shell32.dll,-51609"  // Unpi&n from Start
        : L"@%windir%\\system32\\shell32.dll,-51606"; // &Pin to Start
    HRESULT hrLoad = SHLoadIndirectString(pszSource, pszDisplay, cchDisplay, nullptr);
    RETURN_HR_IF(hrLoad, FAILED(hrLoad) && hrLoad != E_FAIL); // E_FAIL if the resource does not exist
    if (hrLoad == E_FAIL)
    {
        pszSource = action == ContextMenuItemAction::Unpin
            ? L"@%windir%\\system32\\shell32.dll,-51394"  // Un&pin from Start
            : L"@%windir%\\system32\\shell32.dll,-51201"; // &Pin to Start
        RETURN_IF_FAILED(SHLoadIndirectString(pszSource, pszDisplay, cchDisplay, nullptr));
    }

    return S_OK;
}

HRESULT StartPinUnpinContextMenu::GetLayoutCustomizationRestrictionType(
    utctc::StartCollectionCustomizationRestrictionType* pRestriction) const
{
    *pRestriction = utctc::StartCollectionCustomizationRestrictionType_None;

    /*if (WI_IsFeatureEnabled(Feature_STest03) && _spStartScreenManagerExtensionStatics != nullptr)
    {
        if (WI_IsFeatureEnabled(Feature_NoChangeStart))
        {
            boolean fPolicySet;
            RETURN_IF_FAILED(_spLauncherPolicy->get_IsLauncherCustomizationDisabledPolicySet(&fPolicySet)); // 442
            if (fPolicySet)
            {
                *pRestriction = utcurated::StartCollectionCustomizationRestrictionType_FullLayout;
            }
        }
    }
    else*/
    {
        boolean fPolicySet;
        RETURN_IF_FAILED(_spLauncherPolicy->get_IsLauncherCustomizationDisabledPolicySet(&fPolicySet)); // 454
        if (fPolicySet)
        {
            ComPtr<utctc::ICuratedTileCollection> spStartTileGridCollectionCurated;
            RETURN_IF_FAILED(_spCuratedTileCollectionManager->GetCollection(Wrappers::HStringReference(L"Start.TileGrid").Get(), &spStartTileGridCollectionCurated)); // 458

            ComPtr<utctc::IStartTileCollection> spStartTileGridCollection;
            RETURN_IF_FAILED(spStartTileGridCollectionCurated.As(&spStartTileGridCollection)); // 461
            RETURN_IF_FAILED(spStartTileGridCollection->get_CustomizationRestriction(pRestriction)); // 462

            if (*pRestriction != utctc::StartCollectionCustomizationRestrictionType_OnlySpecifiedGroups)
            {
                *pRestriction = utctc::StartCollectionCustomizationRestrictionType_FullLayout;
            }
        }
    }

    return S_OK;
}

CoCreatableClass(StartPinUnpinContextMenu);
