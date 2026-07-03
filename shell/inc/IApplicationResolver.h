#pragma once

#include <initguid.h>

interface IShellItem;
interface IShellItem2;
interface IShellLinkW;

DEFINE_GUID(CLSID_StartMenuCacheAndAppResolver, 0x660B90C8, 0x73A9, 0x4B58, 0x8C, 0xAE, 0x35, 0x5B, 0x7F, 0x55, 0x34, 0x1B);

typedef enum tagAPP_RESOLVER_ITEM_FILTER_FLAGS
{
    ARIFF_NONE = 0x0,
    ARIFF_REQUIRE_PREVENT_PINNING_NOT_SET = 0x1,
    ARIFF_REQUIRE_PINNABLE = 0x2,
} APP_RESOLVER_ITEM_FILTER_FLAGS;

MIDL_INTERFACE("de25675a-72de-44b4-9373-05170450c140")
IApplicationResolver : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetAppIDForShortcut(IShellItem* psi, WCHAR** ppszAppID) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAppIDForShortcutObject(IShellLinkW* psl, IShellItem* psi, WCHAR** ppszAppID) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAppIDForWindow(HWND hwnd, WCHAR** ppszAppID, BOOL* pfPinningPrevented, BOOL* pfExplicitAppID, BOOL* pfEmbeddedShortcutValid) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAppIDForProcess(DWORD dwProcessID, WCHAR** ppszAppID, BOOL* pfPinningPrevented, BOOL* pfExplicitAppID, BOOL* pfEmbeddedShortcutValid) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetShortcutForProcess(DWORD dwProcessID, IShellItem** ppsi) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetBestShortcutForAppID(const WCHAR* pszAppID, IShellItem** ppsi) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetBestShortcutAndAppIDForAppPath(const WCHAR* pszAppPath, IShellItem** ppsi, WCHAR** ppszAppID) = 0;
    virtual HRESULT STDMETHODCALLTYPE CanPinApp(IShellItem* psi) = 0;
    virtual HRESULT STDMETHODCALLTYPE CanPinAppShortcut(IShellLinkW* psl, IShellItem* psi) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetRelaunchProperties(HWND hwnd, WCHAR** ppszAppID, WCHAR** ppszCmdLine, WCHAR** ppszIconResource, WCHAR** ppszDisplayNameResource, BOOL* pfPinnable) = 0;
    virtual HRESULT STDMETHODCALLTYPE GenerateShortcutFromWindowProperties(HWND hwnd, IShellItem** ppsi) = 0;
    virtual HRESULT STDMETHODCALLTYPE GenerateShortcutFromItemProperties(IShellItem2* psi2, IShellItem** ppsi) = 0;
};

MIDL_INTERFACE("c1b70261-a4cf-4e49-970e-ef941977e448")
IApplicationResolver2_Base : IApplicationResolver
{
    virtual HRESULT STDMETHODCALLTYPE GetLauncherAppIDForItem(IShellItem*, WCHAR**) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetShortcutForAppID(const WCHAR*, IShellItem**) = 0;
};

// 1809
MIDL_INTERFACE("21cbc515-2dde-4d66-8292-ba34bd25094a")
IApplicationResolver2 : IApplicationResolver2_Base
{
    virtual HRESULT STDMETHODCALLTYPE GetLauncherAppIDForItemEx(IShellItem*, APP_RESOLVER_ITEM_FILTER_FLAGS, WCHAR**) = 0;
};
