#pragma once

#include <Windows.h>

class DECLSPEC_UUID("0c9281f9-6da1-4006-8729-de6e6b61581c")
WpnPlatform;

enum WPN_APP_TYPE
{
    APP_UNKNOWN = 0x0,
    APP_IMMERSIVE = 0x10000000,
    APP_DESKTOP = 0x20000000,
    APP_SYSTEM = 0x40000000,
    APP_PHONE_LEGACY = 0x80000000,
    APP_MASK = 0xF0000000,
};

typedef enum __MIDL___MIDL_itf_wpnplatform_0000_0006_0007
{
    TOAST_DISMISS_REASON_OTHER = 0,
    TOAST_ACTIVATED = 1,
} WPN_TOAST_DISMISS_REASON;

typedef enum __MIDL___MIDL_itf_wpnplatform_0000_0017_0001
{
    WPN_TILE_NOTIFICATION_TYPE_NONE = 0,
    WPN_TILE_NOTIFICATION_TYPE_TILE = 1,
    WPN_TILE_NOTIFICATION_TYPE_BADGE = 2,
    WPN_TILE_NOTIFICATION_TYPE_TILE_FLYOUT = 3,
    WPN_TILE_NOTIFICATION_TYPE_MASK = 0xF,
    WPN_TILE_NOTIFICATION_TYPE_F_VALID_EXPIRY = 0x10,
    WPN_TILE_NOTIFICATION_TYPE_F_PREFERRED = 0x20,
} WpnTileNotificationType;

DEFINE_ENUM_FLAG_OPERATORS(WpnTileNotificationType);

typedef enum __MIDL___MIDL_itf_wpnplatform_0000_0017_0002
{
    WPN_IMAGE_REQUEST_TYPE_OVERWRITE = 1,
    WPN_IMAGE_REQUEST_TYPE_APPEND = 2,
} WpnImageRequestType;

typedef struct __MIDL___MIDL_itf_wpnplatform_0000_0017_0003
{
    const WCHAR* AppUserModelId;
    DWORD NotificationId;
    WpnTileNotificationType Type;
    HRESULT ErrorCode;
    UINT64 Timestamp;
    UINT64 Expiry;
    BYTE* Payload;
    UINT PayloadLength;
    WCHAR* Tag;
    GUID MessageId;
} WPN_TILE_NOTIFICATION, *PWPN_TILE_NOTIFICATION;

typedef struct __MIDL___MIDL_itf_wpnplatform_0000_0017_0004
{
    const WCHAR* AppUserModelId;
    DWORD NotificationId;
    DWORD ResourceId;
    const WCHAR* ResourcePath;
    HRESULT ErrorCode;
    DWORD Flag;
} WPN_RESOURCE_NOTIFICATION, *PWPN_RESOURCE_NOTIFICATION;

typedef struct __MIDL___MIDL_itf_wpnplatform_0000_0017_0005
{
    WpnTileNotificationType Type;
    const WCHAR* AppUserModelId;
} WPN_TILE_NOTIFICATION_CLEAR, *PWPN_TILE_NOTIFICATION_CLEAR;

typedef struct __MIDL___MIDL_itf_wpnplatform_0000_0021_0001
{
    const WCHAR* AppUserModelId;
    UINT Flag;
} WPN_TILE_SESSION_CONTROL, *PWPN_TILE_SESSION_CONTROL;

typedef struct __MIDL___MIDL_itf_wpnplatform_0000_0021_0002
{
    const WCHAR* AppUserModelId;
    DWORD NumNotification;
    DWORD NumNotificationIdExcluded;
    DWORD* NotificationIdExcluded;
    UINT Flag;
} WPN_TILE_REQUEST_CONTROL, *PWPN_TILE_REQUEST_CONTROL;

typedef struct __MIDL___MIDL_itf_wpnplatform_0000_0021_0003
{
    DWORD ResourceId;
    DWORD Flag;
    const WCHAR* Url;
} WPN_TILE_REQUEST_RESOURCE_ENTRY, *PWPN_TILE_REQUEST_RESOURCE_ENTRY;

typedef struct __MIDL___MIDL_itf_wpnplatform_0000_0021_0004
{
    const WCHAR* AppUserModelId;
    DWORD NotificationId;
    DWORD Flag;
    DWORD NumResources;
    WPN_TILE_REQUEST_RESOURCE_ENTRY* Resources;
} WPN_TILE_REQUEST_RESOURCE_CONTROL, *PWPN_TILE_REQUEST_RESOURCE_CONTROL;

typedef struct __MIDL___MIDL_itf_wpnplatform_0000_0021_0005
{
    const WCHAR* AppUserModelId;
    DWORD NotificationId;
    WPN_TOAST_DISMISS_REASON Reason;
} WPN_TOAST_REQUEST_CONTROL, *PWPN_TOAST_REQUEST_CONTROL;

interface IWpnAppEndpoint;

interface IWpnPresentationToastSink;

MIDL_INTERFACE("0e467ac1-65f2-48d6-8bf2-375430548a87")
IWpnPresentationEndpoint : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE TileCreateSession(DWORD*) = 0;
    virtual HRESULT STDMETHODCALLTYPE TileCloseSession(DWORD) = 0;
    virtual HRESULT STDMETHODCALLTYPE TileUpdateSession(DWORD, const WPN_TILE_SESSION_CONTROL*, DWORD) = 0;
    virtual HRESULT STDMETHODCALLTYPE TileRequestNotification(const WPN_TILE_REQUEST_CONTROL*, DWORD) = 0;
    virtual HRESULT STDMETHODCALLTYPE TileRequestResource(WpnImageRequestType, const WPN_TILE_REQUEST_RESOURCE_CONTROL*, DWORD) = 0;
    virtual HRESULT STDMETHODCALLTYPE ToastCreateSession(IWpnPresentationToastSink*, DWORD*) = 0;
    virtual HRESULT STDMETHODCALLTYPE ToastCloseSession(DWORD) = 0;
    virtual HRESULT STDMETHODCALLTYPE ToastReportStatus(const WCHAR*, DWORD, DWORD, HRESULT, const WCHAR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE ToastSuppress(const WCHAR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE ToastUnblockAll() = 0;
    virtual HRESULT STDMETHODCALLTYPE ToastRequestAllNotifications(DWORD) = 0;
    virtual HRESULT STDMETHODCALLTYPE ToastClearNotification(const WPN_TOAST_REQUEST_CONTROL*, DWORD) = 0;
    virtual HRESULT STDMETHODCALLTYPE ToastClearAppNotifications(const WCHAR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE ToastClearRollover(const WCHAR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryAppSetting(const WCHAR*, DWORD*) = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryQuietHours(int*, int*) = 0;
    virtual HRESULT STDMETHODCALLTYPE ActivateToastActionTrigger(const WCHAR*, const WCHAR*, const WCHAR**, DWORD) = 0;
};

MIDL_INTERFACE("f655b052-348b-4ab0-947b-a7dafa44d404")
IWpnRegistrationEndpoint : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE RegisterApplication(const WCHAR*, const WCHAR*, UINT, WPN_APP_TYPE) = 0;
    virtual HRESULT STDMETHODCALLTYPE UpdateRegistration(const WCHAR*, const WCHAR*, UINT, WPN_APP_TYPE) = 0;
    virtual HRESULT STDMETHODCALLTYPE UnregisterApplication(const WCHAR*) = 0;
};

enum WPN_SETTING_TYPE
{
    SETTING_TILE = 0x1,
    SETTING_BADGE = 0x2,
    SETTING_TOAST = 0x4,
    SETTING_AUDIO = 0x8,
    SETTING_LOCK_TILE = 0x10,
    SETTING_LOCK_BADGE = 0x20,
    SETTING_LOCK_TOAST = 0x40,
    SETTING_TICKLE = 0x80,
    SETTING_DEFAULTS = 0xB0004F,
    SETTING_MASK = 0xF000FF,
    CAPABLE_CLOUD = 0x100,
    CAPABLE_TILE = 0x200,
    CAPABLE_TOAST = 0x400,
    CAPABLE_INTERNET = 0x800,
    CAPABLE_BADGE = 0x1000,
    CAPABLE_RINGING = 0x2000,
    CAPABLE_TICKLE = 0x4000,
    CAPABLE_STORAGE = 0x8000,
    CAPABLE_MASK = 0x700FF00,
    ROAMINGOVERRIDE_POLLING_BADGE = 0x10000,
    ROAMINGOVERRIDE_POLLING_TILE = 0x20000,
    ROAMINGOVERRIDE_POLLING_TILE_FLYOUT = 0x40000,
    ROAMINGOVERRIDE_MASK = 0x70000,
    SETTING_VOIP_SCREENON = 0x100000,
    SETTING_TOAST_INTERRUPTIVE = 0x200000,
    SETTING_STOP_CLOUD = 0x400000,
    SETTING_LISTENER_ENABLED = 0x800000,
    CAPABLE_STORAGE_TOAST = 0x1000000,
    CAPABLE_PHONE_VOIP = 0x2000000,
    CAPABLE_TILE_FLYOUT = 0x4000000,
    RESERVED_APPTYPE = 0xF0000000,
};

DEFINE_ENUM_FLAG_OPERATORS(WPN_SETTING_TYPE);

enum WPN_SETTING_POLICY
{
    USER_SETTING = 0x0,
    GROUP_POLICY_DISABLE = 0x1,
    ADMIN_DISABLE = 0x2,
    MBB_BANDWIDTH_CAPPED = 0x4,
};

typedef struct __MIDL___MIDL_itf_wpnplatform_0000_0033_0001
{
    WCHAR* AppUserModelId;
    WPN_APP_TYPE AppType;
    int Enabled;
    WCHAR* ParentId;
} WPN_APP_SETTING_ENTRY, *PWPN_APP_SETTING_ENTRY;

typedef struct __MIDL___MIDL_itf_wpnplatform_0000_0033_0002
{
    ULONG NumApps;
    WPN_APP_SETTING_ENTRY* AppList;
} WPN_APP_LIST, *PWPN_APP_LIST;

MIDL_INTERFACE("71538e01-5877-4d2d-ac1a-ff42b6da910d")
IWpnSettingCallback : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE OnSettingChange(int, WPN_SETTING_POLICY) = 0;
};

typedef struct _WPN_FILE_TIME
{
    BOOL IsValid;
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} WPN_FILE_TIME, *PWPN_FILE_TIME;

enum WPN_MBB_DATE_SOURCE
{
    PLATFORM_DEFAULT = 0,
    PROVIDER_SPECIFIED = 1,
};

typedef enum __MIDL___MIDL_itf_wpnplatform_0000_0006_0003
{
    HALF_HOUR = 0,
    HOUR = 1,
    SIX_HOURS = 2,
    TWELVE_HOURS = 3,
    DAILY = 4,
} WpnPeriodicUpdateRecurrence;

typedef struct __MIDL___MIDL_itf_wpnplatform_0000_0027_0001
{
    WCHAR* AppUserModelId;
    BOOL NotificationQueueEnabled;
    BOOL EnableNotificationQueueForSquare150x150;
    BOOL EnableNotificationQueueForWide310x150;
    BOOL EnableNotificationQueueForSquare310x310;
    UINT TileUrlCount;
    WCHAR* TileUrls[5];
    FILETIME TileTimerExpiry;
    WpnPeriodicUpdateRecurrence TileRecurrence;
    WCHAR* BadgeUrl;
    FILETIME BadgeTimerExpiry;
    WpnPeriodicUpdateRecurrence BadgeRecurrence;
} WPN_APP_POLLING_ENTRY, *PWPN_APP_POLLING_ENTRY;

typedef struct __MIDL___MIDL_itf_wpnplatform_0000_0034_0001
{
    ULONG NumApps;
    WPN_APP_POLLING_ENTRY* AppList;
} WPN_POLLING_APP_LIST, *PWPN_POLLING_APP_LIST;

MIDL_INTERFACE("ffffffff-348b-4ab0-947b-a7dafa44d404")
IWpnSettingsEndpoint : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE QueryAppSetting(const WCHAR*, WPN_SETTING_TYPE, int*) = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryGlobalSetting(WPN_SETTING_TYPE, int*, WPN_SETTING_POLICY*) = 0;
    virtual HRESULT STDMETHODCALLTYPE ChangeAppSetting(const WCHAR*, WPN_SETTING_TYPE, int) = 0;
    virtual HRESULT STDMETHODCALLTYPE ChangeGlobalSetting(WPN_SETTING_TYPE, int) = 0;
    virtual HRESULT STDMETHODCALLTYPE PopulateCapableApps(WPN_SETTING_TYPE, WPN_APP_LIST**) = 0;
    virtual HRESULT STDMETHODCALLTYPE PopulateCapableAppsInPackage(WPN_SETTING_TYPE, const WCHAR*, WPN_APP_LIST**) = 0;
    virtual HRESULT STDMETHODCALLTYPE RegisterSettingCallback(WPN_SETTING_TYPE, IWpnSettingCallback*, ULONG*) = 0;
    virtual HRESULT STDMETHODCALLTYPE UnregisterSettingCallback(ULONG) = 0;
    virtual HRESULT STDMETHODCALLTYPE ClearTiles() = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryMobileBroadbandCap(ULONG*) = 0;
    virtual HRESULT STDMETHODCALLTYPE ChangeMobileBroadbandCap(ULONG) = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryMobileBroadbandUsage(UINT64*) = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryMobileBroadbandDates(WPN_FILE_TIME*, WPN_FILE_TIME*, WPN_MBB_DATE_SOURCE*) = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryToastWakeupTime(WPN_FILE_TIME*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetToastWakeupTime(WPN_FILE_TIME) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetPollingAppsInPackage(const WCHAR*, WPN_POLLING_APP_LIST**) = 0;
    virtual HRESULT STDMETHODCALLTYPE ScheduleSettingsSync(const WCHAR*, const WCHAR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE UnblockAllChannels() = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryEnergySaverSetting(int*) = 0;
    virtual HRESULT STDMETHODCALLTYPE ChangeEnergySaverSetting(int) = 0;
};

interface IWpnIdleTaskEndpoint;
interface IWpnTestEndpoint;

MIDL_INTERFACE("df8e9480-ca73-448e-b8f0-da000f581428")
IWpnPlatform : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE CreateAppEndpoint(IWpnAppEndpoint**) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreatePresentationEndpoint(IWpnPresentationEndpoint**) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateRegistrationEndpoint(IWpnRegistrationEndpoint**) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateSettingsEndpoint(IWpnSettingsEndpoint**) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateIdleTaskEndpoint(IWpnIdleTaskEndpoint**) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateTestEndpoint(IWpnTestEndpoint**) = 0;
    virtual HRESULT STDMETHODCALLTYPE Shutdown() = 0;
};

MIDL_INTERFACE("e716b283-6be7-4e6f-a88f-1cde47d5e355")
IWpnPresentationTileSink : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE TileSessionUpdated(DWORD, const WPN_TILE_NOTIFICATION*, DWORD) = 0;
    virtual HRESULT STDMETHODCALLTYPE TileSessionNotificationCleared(DWORD, const WPN_TILE_NOTIFICATION_CLEAR*, DWORD) = 0;
    virtual HRESULT STDMETHODCALLTYPE TileSessionImageDownloadUpdated(DWORD, int) = 0;
    virtual HRESULT STDMETHODCALLTYPE TileRequestNotificationCompleted(const WPN_TILE_NOTIFICATION*, DWORD) = 0;
    virtual HRESULT STDMETHODCALLTYPE TileRequestResourceCompleted(const WPN_RESOURCE_NOTIFICATION*, DWORD) = 0;
};
