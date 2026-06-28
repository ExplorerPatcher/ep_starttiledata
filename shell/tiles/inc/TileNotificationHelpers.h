#pragma once

#include "wpnplatform.h"

WINOLEAPI
CoCreateInstanceAsUser(
    _In_ REFCLSID rclsid,
    _In_opt_ LPUNKNOWN pUnkOuter,
    _In_ DWORD dwClsContext,
    _In_ UINT64 userContext,
    _In_ REFIID riid,
    _COM_Outptr_ _At_(*ppv, _Post_readable_size_(_Inexpressible_(varies))) LPVOID  FAR * ppv
    );

namespace TileNotificationHelpers
{
enum class NotificationType
{
    Tile, // Actual values unknown
};

inline HRESULT GetWpnSettingsEndpoint(UINT64 userContext, IWpnSettingsEndpoint** outEndpoint)
{
    *outEndpoint = nullptr;

    Microsoft::WRL::ComPtr<IWpnPlatform> wpnPlatform;
    RETURN_IF_FAILED(CoCreateInstanceAsUser(
        __uuidof(WpnPlatform), nullptr, CLSCTX_LOCAL_SERVER | CLSCTX_NO_CODE_DOWNLOAD, userContext,
        IID_PPV_ARGS(&wpnPlatform))); // 20

    RETURN_HR(wpnPlatform->CreateSettingsEndpoint(outEndpoint)); // 22
}

inline HRESULT EnableNotifications(
    ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* identifier, UINT64 userContextToken, bool bEnable,
    NotificationType type = NotificationType::Tile)
{
    using namespace ABI::WindowsInternal::Shell::UnifiedTile;

    UnifiedTileIdentifierKind kind;
    RETURN_IF_FAILED(identifier->get_Kind(&kind)); // 28
    if (kind == UnifiedTileIdentifierKind_Packaged)
    {
        Microsoft::WRL::ComPtr<IWpnSettingsEndpoint> settingsEndpoint;
        RETURN_IF_FAILED(GetWpnSettingsEndpoint(userContextToken, &settingsEndpoint)); // 33

        Microsoft::WRL::Wrappers::HString notificationId;
        RETURN_IF_FAILED(identifier->get_NotificationId(notificationId.ReleaseAndGetAddressOf())); // 36

        HRESULT hr = settingsEndpoint->ChangeAppSetting(notificationId.GetRawBuffer(nullptr), SETTING_TILE, bEnable);

        if (FAILED(hr))
        {
            RETURN_HR_IF(hr, hr != E_NOT_SET); // 42
        }
    }

    return S_OK;
}

inline HRESULT AreNotificationsEnabled(
    ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* identifier, UINT64 userContext,
    NotificationType type, bool* bOutEnabled)
{
    using namespace ABI::WindowsInternal::Shell::UnifiedTile;

    *bOutEnabled = false;

    UnifiedTileIdentifierKind kind;
    RETURN_IF_FAILED(identifier->get_Kind(&kind)); // 62
    if (kind == UnifiedTileIdentifierKind_Packaged)
    {
        Microsoft::WRL::ComPtr<IWpnSettingsEndpoint> settingsEndpoint;
        RETURN_IF_FAILED(GetWpnSettingsEndpoint(userContext, &settingsEndpoint)); // 67

        Microsoft::WRL::Wrappers::HString notificationId;
        RETURN_IF_FAILED(identifier->get_NotificationId(notificationId.ReleaseAndGetAddressOf())); // 70

        BOOL bEnabled = FALSE;
        HRESULT hr = settingsEndpoint->QueryAppSetting(notificationId.GetRawBuffer(nullptr), SETTING_TILE, &bEnabled);

        if (FAILED(hr))
        {
            RETURN_HR_IF(hr, hr != E_NOT_SET); // 77
        }

        *bOutEnabled = bEnabled != 0;
    }

    return S_OK;
}
}
