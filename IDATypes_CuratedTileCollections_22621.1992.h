// Parser specific options > [x] No IDA specific extensions

// Include paths: C:\Users\Allison\Dev\Projects\ep_starttiledata\shell\inc
// Compiler args: -std=c++17

// Import "IDATypes_WrlWorkaround.h" after importing this!

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <inspectable.h>

#define __WRL_CONFIGURATION_LEGACY__
/* more WRL includes here */

#include <ppltasks.h>

#include <windows.foundation.collections.h>
#include <windows.system.h>
#include <windows.ui.startscreen.h>
#include "idl/obj/Release/x64/Generated Files/WindowsInternal.Shell.CDSProperties.h"
#include "idl/obj/Release/x64/Generated Files/WindowsInternal.Shell.UnifiedTile.h"
#include "idl/obj/Release/x64/Generated Files/Windows.Internal.UI.StartScreen.h"
#include "idl/obj/Release/x64/Generated Files/Windows.System.Internal.h"

#define RESOURCE_SUPPRESS_STL
#undef _MUTEX_
#undef _SHARED_MUTEX_
#include "packages/Microsoft.Windows.ImplementationLibrary.1.0.260126.7/include/wil/winrt.h" // includes WRL too
#include "packages/Microsoft.Windows.ImplementationLibrary.1.0.260126.7/include/wil/token_helpers.h"

#include "shell/inc/windowscollections.h"

typedef struct _WNF_STATE_NAME
{
    ULONG Data[2];
} WNF_STATE_NAME, *PWNF_STATE_NAME;

typedef struct _WNF_TYPE_ID
{
    GUID TypeId;
} WNF_TYPE_ID, *PWNF_TYPE_ID;

typedef const WNF_TYPE_ID *PCWNF_TYPE_ID;

namespace Windows { namespace Internal
{
    template <typename ElementType>
    class CoTaskMemPolicy
    {
    };

    class LocalMemPolicy
    {
    };

    template <typename Allocator>
    class NativeString
    {
        WCHAR* _pszStringData;
        size_t _cchStringData;
        size_t _cchStringDataCapacity;
    };
} }

typedef Windows::Internal::NativeString<Windows::Internal::CoTaskMemPolicy<WCHAR>> CoTaskMemNativeString;

namespace Windows { namespace Internal
{
    enum TaskApartment
    {
        TaskApartment_MTA,
        TaskApartment_STA,
        TaskApartment_Calling,
        TaskApartment_Any,
        TaskApartment_Synchronous,
        TaskApartment_STAIfSupported,
        TaskApartment_SynchronousUnlessInSTA,
    };

    enum TaskOptions
    {
        TaskOptions_None = 0x0,
        TaskOptions_CreateObject = 0x1,
        TaskOptions_Ordered = 0x2,
        TaskOptions_Marshal = 0x4,
        TaskOptions_AffinityHint = 0x8,
        TaskOptions_BypassLimits = 0x10,
        TaskOptions_Wait = 0x20,
        TaskOptions_WaitWithCalls = 0x40,
        TaskOptions_SyncNesting = 0x80,
        TaskOptions_CancelPending = 0x100,
        TaskOptions_NoWait = 0x200,
        TaskOptions_PriorityMask = 0xF000,
        TaskOptions_BackgroundMode = 0x1000,
        TaskOptions_IdlePriority = 0x2000,
        TaskOptions_LowestPriority = 0x3000,
        TaskOptions_LowerPriority = 0x4000,
        TaskOptions_NormalPriority = 0x5000,
        TaskOptions_HigherPriority = 0x6000,
        TaskOptions_HighestPriority = 0x7000,
        TaskOptions_CriticalPriority = 0x8000,
    };

    class ComTaskPoolHandler
    {
        TaskApartment m_apartment;
        TaskOptions m_options;
        DWORD m_threadId;
    };
} }

template<typename T>
class CTContainer_PolicyUnOwned
{
};

template<typename T>
class CTContainer_PolicyRelease
{
};

class CTContainer_PolicyNewMem
{
};

class CTContainer_PolicyCoTaskMem
{
};

class CTContainer_PolicyLocalMem
{
};

template <typename T>
class CTPolicyCoTaskMem : CTContainer_PolicyCoTaskMem
{
};

template <typename T>
class CTPolicyLocalMem : CTContainer_PolicyLocalMem
{
};

template <typename T>
class CSimpleArrayStandardCompareHelper
{
};

class CSimpleArrayCaseInsensitiveOrdinalStringCompareHelper
{
};

class CSimpleArrayUserDefaultLocaleCaseInsensitiveCompareHelper
{
};

template <typename T>
class CSimpleArrayStandardMergeHelper
{
};

template <
    typename T,
    typename CompareHelper = CSimpleArrayStandardCompareHelper<T>
>
class CTSimpleFixedArray
{
public:
    T* _parray;
    size_t _celem;
};

template <
    typename T,
    size_t MaxSize,
    typename Allocator,
    typename CompareHelper = CSimpleArrayStandardCompareHelper<T>,
    typename MergeHelper = CSimpleArrayStandardMergeHelper<T>
>
class CTSimpleArray : public CTSimpleFixedArray<T, CompareHelper>
{
public:
    T* _parrayT;
    size_t _celemCapacity;
};

template <
    typename T,
    size_t MaxSize = 0xffffffff - 1,
    typename CompareHelper = CSimpleArrayStandardCompareHelper<T>
>
class CCoSimpleArray : public CTSimpleArray<T, MaxSize, CTPolicyCoTaskMem<T>, CompareHelper>
{
};

template <
    typename T,
    size_t MaxSize = 0xffffffff - 1,
    typename CompareHelper = CSimpleArrayStandardCompareHelper<T>
>
class CLocalSimpleArray : public CTSimpleArray<T, MaxSize, CTPolicyLocalMem<T>, CompareHelper>
{
};

template <typename TKey>
class CDefaultHashPolicy
{
};

template <typename TKey>
class CDefaultKeyCompare
{
};

class CDefaultResizePolicy
{
};

class CDefaultRehashPolicy
{
};

class CStringHashPolicy
{
};

class CCaseInsensitiveStringHashPolicy
{
};

template <
    typename TKey,
    typename TValue,
    typename THashPolicy = CDefaultHashPolicy<TKey>,
    typename TKeyCompare = CDefaultKeyCompare<TKey>,
    typename TResizePolicy = CDefaultResizePolicy,
    typename TRehashPolicy = CDefaultRehashPolicy
>
class CSimpleHashTable
{
    class HashBucket
    {
        typedef enum ENTRYSTATE
        {
            EMPTY = 0,
            OCCUPIED = 1,
            FREED = 2
        } ENTRYSTATE;

        ENTRYSTATE _state;
        TKey _key;
        TValue _value;
    };

    UINT _cItems;
    UINT _cBuckets;
    UINT _cFreedBuckets;
    HashBucket* _rgBuckets;
};

template <typename TItem>
class CSet
{
    CSimpleHashTable<TItem, bool>* _pTable;
};

namespace Geometry
{
    struct CPoint : POINT
    {
    };

    struct CRect : RECT
    {
    };

    struct CSize : SIZE
    {
    };
}

template <typename T>
class CRefCountedObject : public IUnknown, public T
{
    ULONG _cRef;
};

#define MICROSOFT_WRL_RUNTIME_CLASS \
    Microsoft::WRL::Details::DontUseNewUseMake DontUseNewUseMake; \
    ULONG refcount_; \

#define MICROSOFT_WRL_IMPLEMENTS_CLASS \
    void* gap[3]; \
    Microsoft::WRL::Details::DontUseNewUseMake DontUseNewUseMake; \
    ULONG refcount_; \

/////

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// std::vector<std::string>;
// std::vector<std::wstring>;

interface IItemLayoutResolver;
interface ILayoutTraversalOrder;
interface ITileGridMetricsCalculator;

enum TileSizingMode
{
    TileSizingMode_FixedSize = 0x0,
    TileSizingMode_EdgeToEdge = 0x1,
};

enum LayoutOrder
{
    LayoutOrder_Grid = 0,
    LayoutOrder_Linear = 1,
    LayoutOrder_Two = 2, // @Note: Added in 15063
};

MIDL_INTERFACE("e329db7a-e2f4-4d74-f1b5-9d75b80a5e46")
IStartLayoutFactory : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE CreatePortraitLayoutResolver(IItemLayoutResolver**) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateDesktopPortraitLayoutResolver(IItemLayoutResolver**) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateGroupsLayoutResolver(IItemLayoutResolver**) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateLayoutTraversalOrder(
        IItemLayoutResolver*, int, LayoutOrder, ILayoutTraversalOrder**) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateTileGridMetricsCalculator(
        const TileSizingMode, const float, const float, const float, ITileGridMetricsCalculator**) = 0;
};

namespace CommonStartTelemetry
{
class LogAllTilesActivity
{
    char data[320]; ///< GE: 344 (+24) bytes
};
}

namespace DataStoreCache::Util
{
struct hashGUID
{
    std::size_t operator()(const GUID& guid) const noexcept;
};
}

namespace DataStoreCache::CloudUtil
{
template <typename T>
struct CloudItemObserverCallback;
}

namespace DataStoreCache::CuratedTileCollectionTransformer
{
/*class ICuratedCollectionBatchCookieImpl
{
public:
    virtual ~ICuratedCollectionBatchCookieImpl();
    virtual Concurrency::task<void> EndBatchUpdate() = 0;
    virtual void SetPreventEndBatchOnDestruction(bool) = 0;
};*/

class ICuratedCollectionBatchCookieImpl
{
public:
    virtual ~ICuratedCollectionBatchCookieImpl();
    virtual Concurrency::task<void>* EndBatchUpdate(Concurrency::task<void>* retstr) = 0;
    virtual void SetPreventEndBatchOnDestruction(bool) = 0;
};

struct CuratedTileChangeInfo;

/*class ICuratedTileImpl
{
public:
    virtual ~ICuratedTileImpl();
    virtual POINT GetLocation() = 0;
    virtual void SetLocation(POINT) = 0;
    virtual std::shared_ptr<ICuratedCollectionBatchCookieImpl> BeginBatchUpdate() = 0;
    virtual GUID GetLayoutId() = 0;
    virtual void SetSize(const SIZE&) = 0;
    virtual SIZE GetSize() = 0;
    virtual wil::com_ptr<WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier> GetTileIdentifier() = 0;
    virtual void SetTileIdentifier(WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
    virtual std::wstring GetCustomProperty(const std::wstring&) = 0;
    virtual bool HasCustomProperty(const std::wstring&) = 0;
    virtual void RemoveCustomProperty(const std::wstring&) = 0;
    virtual void SetCustomProperty(const std::wstring&, const std::wstring&) = 0;
    virtual std::shared_ptr<CloudUtil::CloudItemObserverCallback<CuratedTileChangeInfo>> AddObserver(const std::function<void (const CuratedTileChangeInfo&)>&) = 0;
    virtual bool IsDefaultData() = 0;
    virtual std::wstring GetJSONBlob(UINT) = 0;
};*/

class ICuratedTileImpl
{
public:
    virtual ~ICuratedTileImpl();
    virtual POINT* GetLocation(POINT* retstr) = 0;
    virtual void SetLocation(POINT) = 0;
    virtual std::shared_ptr<ICuratedCollectionBatchCookieImpl>* BeginBatchUpdate(std::shared_ptr<ICuratedCollectionBatchCookieImpl>* retstr) = 0;
    virtual GUID* GetLayoutId(GUID* retstr) = 0;
    virtual void SetSize(const SIZE&) = 0;
    virtual SIZE* GetSize(SIZE* retstr) = 0;
    virtual wil::com_ptr<ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier>* GetTileIdentifier(wil::com_ptr<ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier>* retstr) = 0;
    virtual void SetTileIdentifier(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
    virtual std::wstring* GetCustomProperty(std::wstring* retstr, const std::wstring&) = 0;
    virtual bool HasCustomProperty(const std::wstring&) = 0;
    virtual void RemoveCustomProperty(const std::wstring&) = 0;
    virtual void SetCustomProperty(const std::wstring&, const std::wstring&) = 0;
    virtual std::shared_ptr<CloudUtil::CloudItemObserverCallback<CuratedTileChangeInfo>>* AddObserver(std::shared_ptr<CloudUtil::CloudItemObserverCallback<CuratedTileChangeInfo>>* retstr, const std::function<void (const CuratedTileChangeInfo&)>&) = 0;
    virtual bool IsDefaultData() = 0;
    virtual std::wstring* GetJSONBlob(std::wstring* retstr, UINT) = 0;
};

class CuratedTile
{
public:
    std::shared_ptr<ICuratedTileImpl> _impl;
};

struct CuratedGroupChangeInfo;

class CuratedGroup; // fwd decl

/*class ICuratedGroupImpl
{
public:
    virtual ~ICuratedGroupImpl();
    virtual std::shared_ptr<std::wstring> GetDisplayName() = 0;
    virtual void SetDisplayName(std::wstring) = 0;
    virtual std::shared_ptr<std::wstring> GetLocalizedDisplayNameResource() = 0;
    virtual void SetLocalizedDisplayNameResource(std::wstring) = 0;
    virtual POINT GetLocation() = 0;
    virtual void SetLocation(POINT) = 0;
    virtual bool GetIsLockedForCustomization() = 0;
    virtual void SetIsLockedForCustomization(bool) = 0;
    virtual bool GetIsUserCustomizedDisplayName() = 0;
    virtual void SetIsUserCustomizedDisplayName(bool) = 0;
    virtual SIZE GetSize() = 0;
    virtual void SetSize(SIZE) = 0;
    virtual GUID GetLayoutId() = 0;
    virtual std::shared_ptr<ICuratedCollectionBatchCookieImpl> BeginBatchUpdate() = 0;
    virtual void AddTile(std::shared_ptr<CuratedTile>) = 0;
    virtual std::shared_ptr<CuratedTile> CreateTile(WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
    virtual std::shared_ptr<CuratedTile> CreateTile(const GUID&, WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
    virtual std::unordered_map<GUID, std::shared_ptr<CuratedTile>, Util::hashGUID> GetTiles() = 0;
    virtual void DeleteTile(const GUID&) = 0;
    virtual void RemoveTile(const GUID&) = 0;
    virtual void AddGroup(std::shared_ptr<CuratedGroup>) = 0;
    virtual std::shared_ptr<CuratedGroup> CreateGroup() = 0;
    virtual std::unordered_map<GUID, std::shared_ptr<CuratedGroup>, Util::hashGUID> GetGroups() = 0;
    virtual void DeleteGroup(const GUID&) = 0;
    virtual void RemoveGroup(const GUID&) = 0;
    virtual std::wstring GetCustomProperty(const std::wstring&) = 0;
    virtual bool HasCustomProperty(const std::wstring&) = 0;
    virtual void RemoveCustomProperty(const std::wstring&) = 0;
    virtual void SetCustomProperty(const std::wstring&, const std::wstring&) = 0;
    virtual std::shared_ptr<CloudUtil::CloudItemObserverCallback<CuratedGroupChangeInfo>> AddObserver(const std::function<void (const CuratedGroupChangeInfo&)>&) = 0;
    virtual bool IsDefaultData() = 0;
    virtual std::wstring GetJSONBlob(UINT) = 0;
};*/

class ICuratedGroupImpl
{
public:
    virtual ~ICuratedGroupImpl();
    virtual std::shared_ptr<std::wstring>* GetDisplayName(std::shared_ptr<std::wstring>* retstr) = 0;
    virtual void SetDisplayName(std::wstring) = 0;
    virtual std::shared_ptr<std::wstring>* GetLocalizedDisplayNameResource(std::shared_ptr<std::wstring>* retstr) = 0;
    virtual void SetLocalizedDisplayNameResource(std::wstring) = 0;
    virtual POINT* GetLocation(POINT* retstr) = 0;
    virtual void SetLocation(POINT) = 0;
    virtual bool GetIsLockedForCustomization() = 0;
    virtual void SetIsLockedForCustomization(bool) = 0;
    virtual bool GetIsUserCustomizedDisplayName() = 0;
    virtual void SetIsUserCustomizedDisplayName(bool) = 0;
    virtual SIZE* GetSize(SIZE* retstr) = 0;
    virtual void SetSize(SIZE) = 0;
    virtual GUID* GetLayoutId(GUID* retstr) = 0;
    virtual std::shared_ptr<ICuratedCollectionBatchCookieImpl>* BeginBatchUpdate(std::shared_ptr<ICuratedCollectionBatchCookieImpl>* retstr) = 0;
    virtual void AddTile(std::shared_ptr<CuratedTile>) = 0;
    virtual std::shared_ptr<CuratedTile>* CreateTile(std::shared_ptr<CuratedTile>* retstr, ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
    virtual std::shared_ptr<CuratedTile>* CreateTile(std::shared_ptr<CuratedTile>* retstr, const GUID&, ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
    virtual std::unordered_map<GUID, std::shared_ptr<CuratedTile>, Util::hashGUID>* GetTiles(std::unordered_map<GUID, std::shared_ptr<CuratedTile>, Util::hashGUID>* retstr) = 0;
    virtual void DeleteTile(const GUID&) = 0;
    virtual void RemoveTile(const GUID&) = 0;
    virtual void AddGroup(std::shared_ptr<CuratedGroup>) = 0;
    virtual std::shared_ptr<CuratedGroup>* CreateGroup(std::shared_ptr<CuratedGroup>* retstr) = 0;
    virtual std::unordered_map<GUID, std::shared_ptr<CuratedGroup>, Util::hashGUID>* GetGroups(std::unordered_map<GUID, std::shared_ptr<CuratedGroup>, Util::hashGUID>* retstr) = 0;
    virtual void DeleteGroup(const GUID&) = 0;
    virtual void RemoveGroup(const GUID&) = 0;
    virtual std::wstring* GetCustomProperty(std::wstring* retstr, const std::wstring&) = 0;
    virtual bool HasCustomProperty(const std::wstring&) = 0;
    virtual void RemoveCustomProperty(const std::wstring&) = 0;
    virtual void SetCustomProperty(const std::wstring&, const std::wstring&) = 0;
    virtual std::shared_ptr<CloudUtil::CloudItemObserverCallback<CuratedGroupChangeInfo>>* AddObserver(std::shared_ptr<CloudUtil::CloudItemObserverCallback<CuratedGroupChangeInfo>>* retstr, const std::function<void (const CuratedGroupChangeInfo&)>&) = 0;
    virtual bool IsDefaultData() = 0;
    virtual std::wstring* GetJSONBlob(std::wstring* retstr, UINT) = 0;
};

class CuratedGroup
{
public:
    std::shared_ptr<ICuratedGroupImpl> _impl;
};

std::unordered_map<GUID, std::shared_ptr<CuratedGroup>, Util::hashGUID>;

enum LayoutOverrideType
{
};

enum LayoutCustomizationRestrictionType
{
    LayoutCustomizationRestrictionType_None,
    LayoutCustomizationRestrictionType_OnlySpecifiedGroups,
    LayoutCustomizationRestrictionType_FullLayout,
};

struct CuratedRootChangeInfo;

/*class ICuratedRootImpl
{
public:
    virtual ~ICuratedRootImpl();
    virtual UINT GetLayoutVersion() = 0;
    virtual void SetLayoutVersion(UINT) = 0;
    virtual UINT GetGroupColumnCount() = 0;
    virtual void SetGroupColumnCount(UINT) = 0;
    virtual UINT GetPreferredColumnCount() = 0;
    virtual void SetPreferredColumnCount(UINT) = 0;
    virtual GUID GetLastGroupId() = 0;
    virtual void SetLastGroupId(GUID) = 0;
    virtual LayoutOverrideType GetLayoutOverride() = 0;
    virtual void SetLayoutOverride(LayoutOverrideType) = 0;
    virtual LayoutCustomizationRestrictionType GetLayoutCustomizationRestriction() = 0;
    virtual void SetLayoutCustomizationRestriction(LayoutCustomizationRestrictionType) = 0;
    virtual std::shared_ptr<ICuratedCollectionBatchCookieImpl> BeginBatchUpdate() = 0;
    virtual std::vector<std::shared_ptr<CuratedTile>> GetAllTilesInTree() = 0;
    virtual GUID GetLayoutId() = 0;
    virtual std::shared_ptr<const std::wstring> GetLayoutName() = 0;
    virtual void SetLayoutName(const WCHAR*) = 0;
    virtual UINT GetGroupCellWidth() = 0;
    virtual void SetGroupCellWidth(UINT) = 0;
    virtual FILETIME GetEnterpriseLayoutLastApplied(bool*) = 0;
    virtual void SetEnterpriseLayoutLastApplied(const FILETIME&) = 0;
    virtual FILETIME GetGroupPolicyLayoutFileTimestamp(bool*) = 0;
    virtual void SetGroupPolicyLayoutFileTimestamp(const FILETIME&) = 0;
    virtual void AddTile(std::shared_ptr<CuratedTile>) = 0;
    virtual std::shared_ptr<CuratedTile> CreateTile(WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
    virtual std::shared_ptr<CuratedTile> CreateTile(const GUID&, WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
    virtual std::unordered_map<GUID, std::shared_ptr<CuratedTile>, Util::hashGUID> GetTiles() = 0;
    virtual void DeleteTile(const GUID&) = 0;
    virtual void RemoveTile(const GUID&) = 0;
    virtual size_t GetTileCount() = 0;
    virtual void AddGroup(std::shared_ptr<CuratedGroup>) = 0;
    virtual std::shared_ptr<CuratedGroup> CreateGroup() = 0;
    virtual std::shared_ptr<CuratedGroup> CreateGroup(const GUID&) = 0;
    virtual std::unordered_map<GUID, std::shared_ptr<CuratedGroup>, Util::hashGUID> GetGroups() = 0;
    virtual void DeleteGroup(const GUID&) = 0;
    virtual void RemoveGroup(const GUID&) = 0;
    virtual size_t GetGroupCount() = 0;
    virtual std::wstring GetCustomProperty(const std::wstring&) = 0;
    virtual bool HasCustomProperty(const std::wstring&) = 0;
    virtual void RemoveCustomProperty(const std::wstring&) = 0;
    virtual void SetCustomProperty(const std::wstring&, const std::wstring&) = 0;
    virtual std::shared_ptr<CloudUtil::CloudItemObserverCallback<CuratedRootChangeInfo>> AddObserver(const std::function<void (const CuratedRootChangeInfo&)>&) = 0;
    virtual bool IsDefaultData() = 0;
    virtual std::wstring GetJSONBlob(UINT) = 0;
};*/

class ICuratedRootImpl
{
public:
    virtual ~ICuratedRootImpl();
    virtual UINT GetLayoutVersion() = 0;
    virtual void SetLayoutVersion(UINT) = 0;
    virtual UINT GetGroupColumnCount() = 0;
    virtual void SetGroupColumnCount(UINT) = 0;
    virtual UINT GetPreferredColumnCount() = 0;
    virtual void SetPreferredColumnCount(UINT) = 0;
    virtual GUID* GetLastGroupId(GUID* retstr) = 0;
    virtual void SetLastGroupId(GUID) = 0;
    virtual LayoutOverrideType GetLayoutOverride() = 0;
    virtual void SetLayoutOverride(LayoutOverrideType) = 0;
    virtual LayoutCustomizationRestrictionType GetLayoutCustomizationRestriction() = 0;
    virtual void SetLayoutCustomizationRestriction(LayoutCustomizationRestrictionType) = 0;
    virtual std::shared_ptr<ICuratedCollectionBatchCookieImpl>* BeginBatchUpdate(std::shared_ptr<ICuratedCollectionBatchCookieImpl>* retstr) = 0;
    virtual std::vector<std::shared_ptr<CuratedTile>>* GetAllTilesInTree(std::vector<std::shared_ptr<CuratedTile>>* retstr) = 0;
    virtual GUID GetLayoutId() = 0;
    virtual std::shared_ptr<const std::wstring>* GetLayoutName(std::shared_ptr<const std::wstring>* retstr) = 0;
    virtual void SetLayoutName(const WCHAR*) = 0;
    virtual UINT GetGroupCellWidth() = 0;
    virtual void SetGroupCellWidth(UINT) = 0;
    virtual FILETIME* GetEnterpriseLayoutLastApplied(FILETIME* retstr, bool*) = 0;
    virtual void SetEnterpriseLayoutLastApplied(const FILETIME&) = 0;
    virtual FILETIME* GetGroupPolicyLayoutFileTimestamp(FILETIME* retstr, bool*) = 0;
    virtual void SetGroupPolicyLayoutFileTimestamp(const FILETIME&) = 0;
    virtual void AddTile(std::shared_ptr<CuratedTile>) = 0;
    virtual std::shared_ptr<CuratedTile>* CreateTile(std::shared_ptr<CuratedTile>* retstr, ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
    virtual std::shared_ptr<CuratedTile>* CreateTile(std::shared_ptr<CuratedTile>* retstr, const GUID&, ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
    virtual std::unordered_map<GUID, std::shared_ptr<CuratedTile>, Util::hashGUID>* GetTiles(std::unordered_map<GUID, std::shared_ptr<CuratedTile>, Util::hashGUID>* retstr) = 0;
    virtual void DeleteTile(const GUID&) = 0;
    virtual void RemoveTile(const GUID&) = 0;
    virtual size_t GetTileCount() = 0;
    virtual void AddGroup(std::shared_ptr<CuratedGroup>) = 0;
    virtual std::shared_ptr<CuratedGroup>* CreateGroup(std::shared_ptr<CuratedGroup>* retstr) = 0;
    virtual std::shared_ptr<CuratedGroup>* CreateGroup(std::shared_ptr<CuratedGroup>* retstr, const GUID&) = 0;
    virtual std::unordered_map<GUID, std::shared_ptr<CuratedGroup>, Util::hashGUID>* GetGroups(std::unordered_map<GUID, std::shared_ptr<CuratedGroup>, Util::hashGUID>* retstr) = 0;
    virtual void DeleteGroup(const GUID&) = 0;
    virtual void RemoveGroup(const GUID&) = 0;
    virtual size_t GetGroupCount() = 0;
    virtual std::wstring* GetCustomProperty(std::wstring* retstr, const std::wstring&) = 0;
    virtual bool HasCustomProperty(const std::wstring&) = 0;
    virtual void RemoveCustomProperty(const std::wstring&) = 0;
    virtual void SetCustomProperty(const std::wstring&, const std::wstring&) = 0;
    virtual std::shared_ptr<CloudUtil::CloudItemObserverCallback<CuratedRootChangeInfo>>* AddObserver(std::shared_ptr<CloudUtil::CloudItemObserverCallback<CuratedRootChangeInfo>>* retstr, const std::function<void (const CuratedRootChangeInfo&)>&) = 0;
    virtual bool IsDefaultData() = 0;
    virtual std::wstring* GetJSONBlob(std::wstring* retstr, UINT) = 0;
};

class CuratedRoot
{
public:
    std::shared_ptr<ICuratedRootImpl> _impl;
};

enum CuratedTileCollectionTransformerOptions
{
    CuratedTileCollectionTransformerOptions_0,
};

/*MIDL_INTERFACE("f54aa3a6-565a-487a-ae5b-0e5c3a1388bf")
ICuratedTileCollectionTransformer : IInspectable
{
    virtual std::shared_ptr<CuratedRoot> STDMETHODCALLTYPE CreateNewLayoutRoot(const GUID&, CuratedTileCollectionTransformerOptions) = 0;
    virtual std::shared_ptr<CuratedRoot> STDMETHODCALLTYPE CreateNewLayoutRoot(const WCHAR*, CuratedTileCollectionTransformerOptions) = 0;
    virtual void STDMETHODCALLTYPE DeleteLayoutRoot(const GUID&) = 0;
    virtual void STDMETHODCALLTYPE DeleteLayoutRoot(const WCHAR*) = 0;
    virtual std::shared_ptr<CuratedRoot> STDMETHODCALLTYPE GetCuratedTileCollectionRoot(const GUID&, CuratedTileCollectionTransformerOptions) = 0;
    virtual std::shared_ptr<CuratedRoot> STDMETHODCALLTYPE GetCuratedTileCollectionRoot(const WCHAR*, CuratedTileCollectionTransformerOptions) = 0;
    virtual bool STDMETHODCALLTYPE HasLayoutRoot(const GUID&) = 0;
    virtual bool STDMETHODCALLTYPE HasLayoutRoot(const WCHAR*) = 0;
    virtual std::vector<Microsoft::WRL::Wrappers::HString> STDMETHODCALLTYPE GetAllRoots() = 0;
    virtual void STDMETHODCALLTYPE SetDetectTornContainers(bool) = 0;
    virtual void STDMETHODCALLTYPE PinStartLayoutRoot(const std::shared_ptr<CuratedRoot>&) = 0;
    virtual Concurrency::task<void> STDMETHODCALLTYPE CommitAsyncWithTimerBypass(const WCHAR*) = 0;
    virtual void STDMETHODCALLTYPE SetUseCommitTimer(const WCHAR*, bool) = 0;
};*/

MIDL_INTERFACE("f54aa3a6-565a-487a-ae5b-0e5c3a1388bf")
ICuratedTileCollectionTransformer : IInspectable
{
    virtual std::shared_ptr<CuratedRoot>* STDMETHODCALLTYPE CreateNewLayoutRoot(std::shared_ptr<CuratedRoot>* retstr, const GUID&, CuratedTileCollectionTransformerOptions) = 0;
    virtual std::shared_ptr<CuratedRoot>* STDMETHODCALLTYPE CreateNewLayoutRoot(std::shared_ptr<CuratedRoot>* retstr, const WCHAR*, CuratedTileCollectionTransformerOptions) = 0;
    virtual void STDMETHODCALLTYPE DeleteLayoutRoot(const GUID&) = 0;
    virtual void STDMETHODCALLTYPE DeleteLayoutRoot(const WCHAR*) = 0;
    virtual std::shared_ptr<CuratedRoot>* STDMETHODCALLTYPE GetCuratedTileCollectionRoot(std::shared_ptr<CuratedRoot>* retstr, const GUID&, CuratedTileCollectionTransformerOptions) = 0;
    virtual std::shared_ptr<CuratedRoot>* STDMETHODCALLTYPE GetCuratedTileCollectionRoot(std::shared_ptr<CuratedRoot>* retstr, const WCHAR*, CuratedTileCollectionTransformerOptions) = 0;
    virtual bool STDMETHODCALLTYPE HasLayoutRoot(const GUID&) = 0;
    virtual bool STDMETHODCALLTYPE HasLayoutRoot(const WCHAR*) = 0;
    virtual std::vector<Microsoft::WRL::Wrappers::HString>* STDMETHODCALLTYPE GetAllRoots(std::vector<Microsoft::WRL::Wrappers::HString>* retstr) = 0;
    virtual void STDMETHODCALLTYPE SetDetectTornContainers(bool) = 0;
    virtual void STDMETHODCALLTYPE PinStartLayoutRoot(const std::shared_ptr<CuratedRoot>&) = 0;
    virtual Concurrency::task<void>* STDMETHODCALLTYPE CommitAsyncWithTimerBypass(Concurrency::task<void>* retstr, const WCHAR*) = 0;
    virtual void STDMETHODCALLTYPE SetUseCommitTimer(const WCHAR*, bool) = 0;
};
}

namespace DataStoreCache
{
struct IDataManager
{
};

enum DataStoreCacheInit
{
};

struct LineData
{
};

MIDL_INTERFACE("47ab0a63-152e-49e5-b183-06c1eae6597c")
IDataStoreTransformer : IInspectable
{
    virtual const GUID& STDMETHODCALLTYPE GetId() const = 0;
    virtual void STDMETHODCALLTYPE SetDataManager(IDataManager*) = 0;
    virtual void STDMETHODCALLTYPE SetInitializationStage(DataStoreCacheInit) = 0;
    virtual std::vector<LineData> STDMETHODCALLTYPE DumpData(const GUID&, HSTRING, UINT) = 0;
};
}

namespace Windows::Data
{
struct PlaceholderTile;
struct PlaceholderTileLocal;
}

namespace StartPlaceHolderTelemetry
{
class PlaceholderTileActivated;
}

namespace DataStoreCache::PlaceholderTileTransformer::Internal
{
interface IPlaceholderTileTransformerInternal;
}

namespace DataStoreCache::PlaceholderTileTransformer
{
enum InstallDelayType
{
};

enum AssetManagementPolicyEnum
{
};

enum InstallStateType
{
};

enum InstallReasonType
{
};

class PlaceholderTileImpl
{
};

class PlaceholderTileLocalImpl
{
};

class PlaceholderTile
{
private:
    std::shared_ptr<PlaceholderTileImpl> _impl;
    std::shared_ptr<PlaceholderTileLocalImpl> _localImpl;
};

class IPlaceholderTileBatchCookie
{
};

/*MIDL_INTERFACE("93400fa0-3b2d-413c-9a9c-fb7962988d15")
IPlaceholderTileTransformer : IDataStoreTransformer
{
    virtual std::shared_ptr<IPlaceholderTileBatchCookie> STDMETHODCALLTYPE BeginBatchUpdate() = 0;
    virtual Concurrency::task<void> STDMETHODCALLTYPE EndBatchUpdate(std::shared_ptr<IPlaceholderTileBatchCookie>) = 0;
    virtual std::shared_ptr<PlaceholderTile> STDMETHODCALLTYPE AddTileToCollection(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*, HSTRING) = 0;
    virtual void STDMETHODCALLTYPE RemoveTileFromCollection(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*, HSTRING) = 0;
    virtual std::map<std::wstring, std::shared_ptr<PlaceholderTile>> STDMETHODCALLTYPE GetAllTilesInCollection(HSTRING) = 0;
    virtual std::shared_ptr<PlaceholderTile> STDMETHODCALLTYPE GetTile(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
    virtual std::shared_ptr<PlaceholderTile> STDMETHODCALLTYPE GetTile(const WCHAR*) = 0;
    virtual std::shared_ptr<PlaceholderTile> STDMETHODCALLTYPE TryGetTile(const WCHAR*) = 0;
    virtual bool STDMETHODCALLTYPE HasTile(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
    virtual std::map<std::wstring, std::shared_ptr<PlaceholderTile>> STDMETHODCALLTYPE GetAllTiles() = 0;
    virtual void STDMETHODCALLTYPE DeleteTile(const WCHAR*) = 0;
    virtual bool STDMETHODCALLTYPE HasTile(const WCHAR*) = 0;
    virtual bool STDMETHODCALLTYPE HasPendingCommits() = 0;
};*/

MIDL_INTERFACE("93400fa0-3b2d-413c-9a9c-fb7962988d15")
IPlaceholderTileTransformer : IDataStoreTransformer
{
    virtual std::shared_ptr<IPlaceholderTileBatchCookie>* STDMETHODCALLTYPE BeginBatchUpdate(std::shared_ptr<IPlaceholderTileBatchCookie>* retstr) = 0;
    virtual Concurrency::task<void>* STDMETHODCALLTYPE EndBatchUpdate(Concurrency::task<void>* retstr, std::shared_ptr<IPlaceholderTileBatchCookie>) = 0;
    virtual std::shared_ptr<PlaceholderTile>* STDMETHODCALLTYPE AddTileToCollection(std::shared_ptr<PlaceholderTile>* retstr, ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*, HSTRING) = 0;
    virtual void STDMETHODCALLTYPE RemoveTileFromCollection(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*, HSTRING) = 0;
    virtual std::map<std::wstring, std::shared_ptr<PlaceholderTile>>* STDMETHODCALLTYPE GetAllTilesInCollection(std::map<std::wstring, std::shared_ptr<PlaceholderTile>>* retstr, HSTRING) = 0;
    virtual std::shared_ptr<PlaceholderTile>* STDMETHODCALLTYPE GetTile(std::shared_ptr<PlaceholderTile>* retstr, ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
    virtual std::shared_ptr<PlaceholderTile>* STDMETHODCALLTYPE GetTile(std::shared_ptr<PlaceholderTile>* retstr, const WCHAR*) = 0;
    virtual std::shared_ptr<PlaceholderTile>* STDMETHODCALLTYPE TryGetTile(std::shared_ptr<PlaceholderTile>* retstr, const WCHAR*) = 0;
    virtual bool STDMETHODCALLTYPE HasTile_Identifier(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
    virtual std::map<std::wstring, std::shared_ptr<PlaceholderTile>>* STDMETHODCALLTYPE GetAllTiles(std::map<std::wstring, std::shared_ptr<PlaceholderTile>>* retstr) = 0;
    virtual void STDMETHODCALLTYPE DeleteTile(const WCHAR*) = 0;
    virtual bool STDMETHODCALLTYPE HasTile_String(const WCHAR*) = 0;
    virtual bool STDMETHODCALLTYPE HasPendingCommits() = 0;
};

/*MIDL_INTERFACE("bb5d3c4c-40cb-41c6-8de7-d036a8f90f6c")
Internal::IPlaceholderTileTransformerInternal : IPlaceholderTileTransformer
{
    virtual void STDMETHODCALLTYPE OnItemUpdated(const WCHAR*, const Windows::Data::PlaceholderTileLocal&) = 0;
    virtual void STDMETHODCALLTYPE OnItemUpdated(const WCHAR*, const Windows::Data::PlaceholderTile&) = 0;
    // virtual wil::com_ptr<ABI::Windows::System::IUser> STDMETHODCALLTYPE GetUser() = 0;
    virtual HRESULT STDMETHODCALLTYPE InstallApp(const std::shared_ptr<PlaceholderTile>&, HSTRING, UINT, ABI::Windows::Foundation::Collections::IPropertySet*, StartPlaceHolderTelemetry::PlaceholderTileActivated&) = 0;
    virtual HRESULT STDMETHODCALLTYPE CancelAppInstall(const std::shared_ptr<PlaceholderTile>&, HSTRING) = 0;
    virtual bool STDMETHODCALLTYPE IsAppInstalling(const std::shared_ptr<PlaceholderTile>&) = 0;
};*/

MIDL_INTERFACE("bb5d3c4c-40cb-41c6-8de7-d036a8f90f6c")
Internal::IPlaceholderTileTransformerInternal : IPlaceholderTileTransformer
{
    virtual void STDMETHODCALLTYPE OnItemUpdated(const WCHAR*, const Windows::Data::PlaceholderTileLocal&) = 0;
    virtual void STDMETHODCALLTYPE OnItemUpdated(const WCHAR*, const Windows::Data::PlaceholderTile&) = 0;
    virtual wil::com_ptr<ABI::Windows::System::IUser>* STDMETHODCALLTYPE GetUser(wil::com_ptr<ABI::Windows::System::IUser>* retstr) = 0;
    virtual HRESULT STDMETHODCALLTYPE InstallApp(const std::shared_ptr<PlaceholderTile>&, HSTRING, UINT, ABI::Windows::Foundation::Collections::IPropertySet*, StartPlaceHolderTelemetry::PlaceholderTileActivated&) = 0;
    virtual HRESULT STDMETHODCALLTYPE CancelAppInstall(const std::shared_ptr<PlaceholderTile>&, HSTRING) = 0;
    virtual bool STDMETHODCALLTYPE IsAppInstalling(const std::shared_ptr<PlaceholderTile>&) = 0;
};

std::shared_ptr<PlaceholderTile>; std::_Ref_count_obj2<PlaceholderTile>;
}

struct CuratedTileCollectionTransformerCreationArgs
{
    ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileManager* pUnifiedTileManager;
    ABI::Windows::System::IUser* pUser;
};

MIDL_INTERFACE("ca7bdd1c-19cc-4128-849e-1186ca3381f3")
ITransformerFactory : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetCuratedTileCollectionTransformer(CuratedTileCollectionTransformerCreationArgs, const IID&, void**) = 0;
};

struct hashGUIDCuratedTileCollections
{
    std::size_t operator()(const GUID& guid) const noexcept;
};

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
struct CollectionContext; // fwd decl

enum CollectionOfficeSKU
{
};

struct SelectionData
{
    bool _bRegionCodesSet;
    bool _bGroupCellWidthSet;
    bool _bUnk1Set;
    bool _bSKUsSet;
    bool _bOfficeSKUsSet;
    bool _bPreInstalledAppsEnabledSet;
    bool _bTargetedContentTilesEnabledSet;
    bool _bEducationModeEnabledSet;
    bool _bCommercialDeviceSet;
    std::vector<WCHAR*> _regionCodes; // CoTaskMemAlloc / CoTaskMemFree
    std::vector<ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::CollectionSKU> _collectionSKUs;
    std::vector<CollectionOfficeSKU> _collectionOfficeSKUs;
    UINT _groupCellWidth;
    bool _bUnk1;
    bool _bPreInstalledAppsEnabled;
    bool _bTargetedContentTilesEnabled;
    bool _bEducationModeEnabled;
    bool _bCommercialDevice;
};
}

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::Internal
{
struct hashGUIDStartLayout
{
    std::size_t operator()(const GUID& guid) const noexcept;
};

enum LayoutTileType
{
    LayoutTileType_0,
    LayoutTileType_1,
    LayoutTileType_2,
    LayoutTileType_3,
};

class LayoutTile
{
public:
    /*LayoutTile(
        std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile> transformerTile,
        std::shared_ptr<DataStoreCache::PlaceholderTileTransformer::PlaceholderTile> placeholderTile);*/

private:
    std::map<std::wstring, std::wstring> _customProperties;
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile> _transformerTile;
    std::shared_ptr<DataStoreCache::PlaceholderTileTransformer::PlaceholderTile> _placeholderTile;
    wil::com_ptr<ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier> _tileIdentifier;
    std::shared_ptr<std::wstring> _packageFamilyName;
    std::shared_ptr<std::wstring> _backgroundColor;
    std::shared_ptr<std::wstring> _displayName;
    std::shared_ptr<std::wstring> _arguments;
    std::shared_ptr<std::wstring> _square150x150LogoUri;
    std::shared_ptr<std::wstring> _square310x310LogoUri;
    std::shared_ptr<std::wstring> _square71x71LogoUri;
    std::shared_ptr<std::wstring> _foregroundText;
    std::shared_ptr<std::wstring> _wide310x150LogoUri;
    std::shared_ptr<std::wstring> field_C8;
    std::shared_ptr<std::wstring> field_D8;
    POINT _location = { -1, -1 };
    SIZE _size;
    GUID _uniqueId = GUID_NULL;
    GUID _groupId = GUID_NULL;
    GUID field_118 = GUID_NULL;
    uint32_t field_128 = 0;
    uint16_t field_12C = 0;
    uint8_t field_12E = 0;
    int field_130 = 6;
    LayoutTileType _type = LayoutTileType_0;
};

class LayoutTileInternal : public LayoutTile
{
};

class LayoutFolder
{
    std::map<std::wstring, std::wstring> _customProperties;
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup> _transformerGroup;
    std::shared_ptr<CollectionContext> _context;
    std::shared_ptr<std::unordered_map<GUID, std::shared_ptr<LayoutTile>, hashGUIDStartLayout>> _tiles;
    std::shared_ptr<std::wstring> _displayName;
    std::shared_ptr<std::wstring> _localizedNameTag;
    POINT _location;
    SIZE _size;
    GUID _uniqueId;
};

class LayoutFolderInternal : public LayoutFolder
{
};

class LayoutGroup
{
    std::map<std::wstring, std::wstring> _customProperties;
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup> _transformerGroup;
    POINT _location;
    std::shared_ptr<CollectionContext> _context;
    std::shared_ptr<std::wstring> _displayName;
    std::shared_ptr<std::wstring> _localizedNameTag;
    std::shared_ptr<std::unordered_map<GUID, std::shared_ptr<LayoutTile>, hashGUIDStartLayout>> _tiles;
    std::shared_ptr<std::unordered_map<GUID, std::shared_ptr<LayoutFolder>, hashGUIDStartLayout>> _folders;
    GUID _uniqueId;
    bool _bIsCustomizationLocked;
    bool field_89;
};

class LayoutGroupInternal : public LayoutGroup
{
};

struct LayoutRoot
{
    std::vector<std::shared_ptr<LayoutGroup>> _vector1; ///< ???
    std::vector<std::shared_ptr<LayoutGroup>> _vector2; ///< ???
    std::vector<std::shared_ptr<LayoutGroup>> _vector3; ///< ???
    std::vector<std::shared_ptr<LayoutGroup>> _vector4; ///< ???
    std::vector<std::shared_ptr<LayoutGroup>> _vector5; ///< ???
    SelectionData _selectionData;
    std::map<std::wstring, std::wstring> _customProperties;
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedRoot> _transformerRoot;
    std::vector<GUID> _layoutInitializationGroupIds;
    std::shared_ptr<std::unordered_map<GUID, std::shared_ptr<LayoutGroup>, hashGUIDStartLayout>> _groups;
    UINT _preferredGroupColumnCount;
    UINT _groupColumnCount;
    UINT _groupCellWidth;
    std::shared_ptr<CollectionContext> _context;
    DataStoreCache::CuratedTileCollectionTransformer::LayoutCustomizationRestrictionType _customizationRestriction;
};

class LayoutRootInternal : public LayoutRoot
{
    std::shared_ptr<CollectionContext> _context;
};

std::shared_ptr<const std::wstring>;
std::vector<std::shared_ptr<LayoutGroup>>;
std::shared_ptr<std::unordered_map<GUID, std::shared_ptr<LayoutFolder>, hashGUIDStartLayout>>;
std::_Ref_count_obj2<std::unordered_map<GUID, std::shared_ptr<LayoutFolder>, hashGUIDStartLayout>>;
std::vector<GUID>;
std::pair<const GUID, std::shared_ptr<LayoutTile>>; std::pair<GUID, std::shared_ptr<LayoutTile>>;
std::pair<const GUID, std::shared_ptr<LayoutFolder>>; std::pair<GUID, std::shared_ptr<LayoutFolder>>;
std::pair<const GUID, std::shared_ptr<LayoutGroup>>; std::pair<GUID, std::shared_ptr<LayoutGroup>>;
std::shared_ptr<LayoutTileInternal>; std::_Ref_count_obj2<LayoutTileInternal>;
std::shared_ptr<LayoutFolderInternal>; std::_Ref_count_obj2<LayoutFolderInternal>;
std::shared_ptr<LayoutGroupInternal>; std::_Ref_count_obj2<LayoutGroupInternal>;
std::shared_ptr<LayoutRootInternal>; std::_Ref_count_obj2<LayoutRootInternal>;
std::vector<std::shared_ptr<LayoutTile>>;
std::function<bool (const std::shared_ptr<LayoutTile>&)>;
}

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
MIDL_INTERFACE("dfdbd59d-7e05-41e6-8820-6777608d561f")
IStartLayoutTileInitializationHandler : IUnknown
{
    virtual bool STDMETHODCALLTYPE CanInitializeTile(std::shared_ptr<Internal::LayoutTile> tile) = 0;
    virtual void STDMETHODCALLTYPE InitializeTile(std::shared_ptr<Internal::LayoutTile> tile) = 0;
    virtual void STDMETHODCALLTYPE UninitializeTile(std::shared_ptr<Internal::LayoutTile> tile) = 0;
};

struct CollectionContext
{
    wil::com_ptr<ABI::Windows::System::IUser> _user;
    void* field_8; ///< wil::com_ptr<???>
    uint32_t field_10;
    wil::com_ptr<IStartLayoutTileInitializationHandler> _userPinnedAppResolverTileInitializationHandler;
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedRoot> _transformerRoot;
    void* field_30; ///< wil::com_ptr<???>
};

class TileInitializationHandlerManager
{
    std::vector<wil::com_ptr<IStartLayoutTileInitializationHandler>> _knownHandlers;
    std::shared_ptr<CollectionContext> _context;
};

std::shared_ptr<TileInitializationHandlerManager>; std::_Ref_count_obj2<TileInitializationHandlerManager>;

/*MIDL_INTERFACE("8fea4543-90d5-4ebd-9831-64b31f83e85d")
ICollectionWriter : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE WriteCollection(std::shared_ptr<Internal::LayoutRoot>, std::shared_ptr<TileInitializationHandlerManager>, CommonStartTelemetry::LogAllTilesActivity, Windows::Foundation::IAsyncAction**) = 0;
    virtual bool STDMETHODCALLTYPE CanInitializeTiles() = 0;
    virtual void STDMETHODCALLTYPE ClearCollection(std::vector<std::shared_ptr<Internal::LayoutTile>>&, Windows::Foundation::IAsyncAction**) = 0;
    virtual bool STDMETHODCALLTYPE IsCollectionEmpty() = 0;
    virtual bool STDMETHODCALLTYPE DoesCollectionExist() = 0;
    virtual std::wstring STDMETHODCALLTYPE GetCollectionName() = 0;
    virtual std::shared_ptr<CollectionContext> STDMETHODCALLTYPE GetContext() = 0;
};*/

MIDL_INTERFACE("8fea4543-90d5-4ebd-9831-64b31f83e85d")
ICollectionWriter : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE WriteCollection(std::shared_ptr<Internal::LayoutRoot>, std::shared_ptr<TileInitializationHandlerManager>, CommonStartTelemetry::LogAllTilesActivity, ABI::Windows::Foundation::IAsyncAction**) = 0;
    virtual bool STDMETHODCALLTYPE CanInitializeTiles() = 0;
    virtual void STDMETHODCALLTYPE ClearCollection(std::vector<std::shared_ptr<Internal::LayoutTile>>&, ABI::Windows::Foundation::IAsyncAction**) = 0;
    virtual bool STDMETHODCALLTYPE IsCollectionEmpty() = 0;
    virtual bool STDMETHODCALLTYPE DoesCollectionExist() = 0;
    virtual std::wstring* STDMETHODCALLTYPE GetCollectionName(std::wstring* retstr) = 0;
    virtual std::shared_ptr<CollectionContext>* STDMETHODCALLTYPE GetContext(std::shared_ptr<CollectionContext>* retstr) = 0;
};

MIDL_INTERFACE("2da4f607-aff6-4324-a7c5-9a0ee03d59c9")
IStartCollectionWriter : IUnknown
{
    virtual void STDMETHODCALLTYPE OverrideGroupColumnCount(UINT) = 0;
};

std::shared_ptr<CollectionContext>; std::_Ref_count_obj2<CollectionContext>;
std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup>;
std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedRoot>;
std::shared_ptr<Internal::LayoutFolder>;
std::shared_ptr<Internal::LayoutGroup>;
std::shared_ptr<Internal::LayoutRoot>;
std::shared_ptr<Internal::LayoutTile>;
std::shared_ptr<TileInitializationHandlerManager>;
std::vector<std::shared_ptr<Internal::LayoutTile>>;

struct CDSStartCollectionWriter : ICollectionWriter, IStartCollectionWriter
{
    MICROSOFT_WRL_RUNTIME_CLASS;
    char _logAllTilesActivity[320]; ///< GE: 344 (+24) bytes
    char _writingStartLayoutToStorageTelemetry[320]; ///< GE: 344 (+24) bytes
    wil::com_ptr<IItemLayoutResolver> _groupsLayoutResolver; ///< GE 8474: Removed
    std::shared_ptr<CollectionContext> _context;
    std::shared_ptr<TileInitializationHandlerManager> _tileInitializationHandlerManager;
    bool _b;
    int _columnCount;
    std::wstring _collectionName;
};

/*MIDL_INTERFACE("511a70c8-def9-4797-b01c-e65dbe591eca")
IInitialCollectionProvider : IUnknown
{
    virtual std::shared_ptr<Internal::LayoutRoot> STDMETHODCALLTYPE GetMatchingDefaultLayout(const SelectionData&) = 0;
    virtual const WCHAR* STDMETHODCALLTYPE GetProviderLoggingName() = 0;
};*/

MIDL_INTERFACE("511a70c8-def9-4797-b01c-e65dbe591eca")
IInitialCollectionProvider : IUnknown
{
    virtual std::shared_ptr<Internal::LayoutRoot>* STDMETHODCALLTYPE GetMatchingDefaultLayout(std::shared_ptr<Internal::LayoutRoot>* retstr, const SelectionData&) = 0;
    virtual const WCHAR* STDMETHODCALLTYPE GetProviderLoggingName() = 0;
};

struct CDSLayoutProvider : IInitialCollectionProvider
{
    MICROSOFT_WRL_RUNTIME_CLASS;
    std::shared_ptr<Internal::LayoutRoot> _layoutRoot;
};

MIDL_INTERFACE("20477929-b8fb-43e2-9c9e-a346c98180e1")
ICuratedTileCollectionInternal : ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTileCollection
{
    virtual HRESULT STDMETHODCALLTYPE EnsureTileRegistration() = 0;
    virtual HRESULT STDMETHODCALLTYPE ResurrectTile(std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile> transformerTile, const GUID& tileId) = 0;
    virtual HRESULT STDMETHODCALLTYPE OnTileAddedWithinCollection(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* identifier) = 0;
    virtual HRESULT STDMETHODCALLTYPE OnTileRemovedWithinCollection(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* identifier) = 0;
};

enum CuratedTileCollectionOptionsInternal
{
    CuratedTileCollectionOptionsInternal_None = 0,
    CuratedTileCollectionOptionsInternal_Batched = 0x1,
    CuratedTileCollectionOptionsInternal_Placeholder = 0x2,
};

class CuratedTileCollectionBase
    : public Microsoft::WRL::Implements<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRt>
        , Microsoft::WRL::ChainInterfaces<ICuratedTileCollectionInternal, ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTileCollection>
    >
{
public:
    virtual ~CuratedTileCollectionBase();

private:
    CuratedTileCollectionOptionsInternal _options;
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedRoot> _transformerRoot;
    wil::com_ptr<DataStoreCache::PlaceholderTileTransformer::IPlaceholderTileTransformer> _placeholderTileTransformer;
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::ICuratedCollectionBatchCookieImpl> _batchCookie;
    std::unordered_map<GUID, wil::com_ptr<ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTileGroup>, hashGUIDCuratedTileCollections> _groups;
    std::unordered_map<GUID, wil::com_ptr<ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTile>, hashGUIDCuratedTileCollections> _tiles;
    wil::com_ptr<ABI::Windows::System::IUser> _user;
    UINT64 _userContextToken;
    bool _bInstallPlaceholderTilesOnNextCommit;
    bool _bCommitOnDestroy;
};

std::function<void ()>;

wil::com_ptr<ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTile>;
wil::com_ptr<ABI::Windows::Foundation::Collections::IMapView<GUID, ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTile*>>;
wil::com_ptr<ABI::Windows::Foundation::Collections::IIterable<ABI::Windows::Foundation::Collections::IKeyValuePair<GUID, ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTile*>*>>;
wil::iterable_range<ABI::Windows::Foundation::Collections::IKeyValuePair<GUID, ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTile*>*, wil::err_exception_policy>;

wil::com_ptr<ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTileGroup>;
wil::com_ptr<ABI::Windows::Foundation::Collections::IMapView<GUID, ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTileGroup*>>;
wil::com_ptr<ABI::Windows::Foundation::Collections::IIterable<ABI::Windows::Foundation::Collections::IKeyValuePair<GUID, ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTileGroup*>*>>;
wil::iterable_range<ABI::Windows::Foundation::Collections::IKeyValuePair<GUID, ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTileGroup*>*, wil::err_exception_policy>;

wil::com_ptr<ABI::WindowsInternal::Shell::UnifiedTile::IPackagedUnifiedTileIdentifier>;
wil::com_ptr<IInspectable>;
wil::com_ptr<ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTileCollection>;

Windows::Foundation::Collections::Internal::HashMap<
    GUID,
    ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTile*,
    Windows::Foundation::Collections::Internal::DefaultHash<GUID>,
    Windows::Foundation::Collections::Internal::DefaultEqualityPredicate<GUID>,
    Windows::Foundation::Collections::Internal::DefaultLifetimeTraits<GUID>,
    Windows::Foundation::Collections::Internal::DefaultLifetimeTraits<ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTile*>,
    Windows::Foundation::Collections::Internal::DefaultHashMapOptions<
        GUID,
        ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTile*,
        Windows::Foundation::Collections::Internal::DefaultLifetimeTraits<GUID>
    >
>;

Windows::Foundation::Collections::Internal::HashMap<
    GUID,
    ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTileGroup*,
    Windows::Foundation::Collections::Internal::DefaultHash<GUID>,
    Windows::Foundation::Collections::Internal::DefaultEqualityPredicate<GUID>,
    Windows::Foundation::Collections::Internal::DefaultLifetimeTraits<GUID>,
    Windows::Foundation::Collections::Internal::DefaultLifetimeTraits<ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTileGroup*>,
    Windows::Foundation::Collections::Internal::DefaultHashMapOptions<
        GUID,
        ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTileGroup*,
        Windows::Foundation::Collections::Internal::DefaultLifetimeTraits<GUID>
    >
>;

class CuratedTileCollection /*final*/
    : Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRt>
        , Microsoft::WRL::FtmBase
        , CuratedTileCollectionBase
    >
{
};

enum StartCollectionUpdateOptions
{
    StartCollectionUpdateOptions_None = 0,
    StartCollectionUpdateOptions_ResetGroupPolicyLayoutFileTimestamp = 0x1,
};

MIDL_INTERFACE("cfc51442-aa2d-418b-9a43-98bdbd743347")
IStartTileCollectionUpdater : IInspectable
{
    virtual HRESULT STDMETHODCALLTYPE CheckForUpdateWithOptions(StartCollectionUpdateOptions) = 0;
};

class StartTileCollection /*final*/
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRt>
        , Microsoft::WRL::FtmBase
        , CuratedTileCollectionBase
        , ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::IStartTileCollection
        , IStartTileCollectionUpdater
    >
{
    wil::com_ptr<IStartLayoutFactory> _layoutFactory;
    wil::com_ptr<IItemLayoutResolver> _lastGroupLayoutResolver;
    wil::com_ptr<DataStoreCache::CuratedTileCollectionTransformer::ICuratedTileCollectionTransformer> _transformer;
    ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::TilePinSize _tilePinSize;
    std::vector<std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile>> _tilesPendingUnpin;
    wil::srwlock _tilesPendingUnpinLock;
};

/*MIDL_INTERFACE("ebb3adda-cd0c-4d14-a198-6fb7dcd692e2")
ICuratedTilePrivate : ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTile
{
    virtual std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile> STDMETHODCALLTYPE GetTransformerData() = 0;
};*/

MIDL_INTERFACE("ebb3adda-cd0c-4d14-a198-6fb7dcd692e2")
ICuratedTilePrivate : ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTile
{
    virtual std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile>* STDMETHODCALLTYPE GetTransformerData(std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile>* retstr) = 0;
};

class CuratedTile /*final*/
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRt>
        , Microsoft::WRL::FtmBase
        , ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTile
        , ICuratedTilePrivate
    >
{
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile> _transformerTile;
};

/*MIDL_INTERFACE("6f3e1834-00c0-4e8b-8834-89da30e185e9")
ICuratedTileGroupPrivate : ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTileGroup
{
    virtual HRESULT STDMETHODCALLTYPE AddTile(ICuratedTilePrivate*) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddGroup(ICuratedTileGroupPrivate*) = 0;
    virtual std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup> STDMETHODCALLTYPE GetTransformerData() = 0;
};*/

MIDL_INTERFACE("6f3e1834-00c0-4e8b-8834-89da30e185e9")
ICuratedTileGroupPrivate : ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTileGroup
{
    virtual HRESULT STDMETHODCALLTYPE AddTile(ICuratedTilePrivate*) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddGroup(ICuratedTileGroupPrivate*) = 0;
    virtual std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup>* STDMETHODCALLTYPE GetTransformerData(std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup>* retstr) = 0;
};

class CuratedTileGroup final
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRt>
        , Microsoft::WRL::FtmBase
        , ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTileGroup
        , ICuratedTileGroupPrivate
    >
{
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup> _transformerGroup;
    std::unordered_map<GUID, wil::com_ptr<ICuratedTileGroup>, hashGUIDCuratedTileCollections> _groups;
    std::unordered_map<GUID, wil::com_ptr<ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICuratedTile>, hashGUIDCuratedTileCollections> _tiles;
    wil::com_ptr<IWeakReference> _collectionWeak;
};

wil::com_ptr<ICuratedTilePrivate>;
wil::com_ptr<ICuratedTileGroupPrivate>;

class CollectionInitializationPolicy;

std::vector<std::shared_ptr<CollectionInitializationPolicy>>;

struct ILayoutModificationXMLParser;

MIDL_INTERFACE("88734a32-4faf-4a40-a9d2-21c2341f2b36")
ICollectionInitializationPipeline : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE EnsureCollectionInitialized(bool*, ABI::Windows::Foundation::IAsyncAction**) = 0;
    virtual HRESULT STDMETHODCALLTYPE CheckForUpdate(bool*) = 0;
    virtual HRESULT STDMETHODCALLTYPE ResetCollection(ABI::Windows::Foundation::IAsyncAction**) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetAllowLayoutOptions(int) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetModificationParser(ILayoutModificationXMLParser**) = 0;
};

wil::com_ptr<ICollectionInitializationPipeline>;

MIDL_INTERFACE("7a554b29-b633-4ebf-ad2a-919da582c85e")
IInitialCollectionPostProcessor : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE PostProcessLayout(
        const SelectionData&, std::shared_ptr<Internal::LayoutRoot>&) = 0;
    virtual const WCHAR* STDMETHODCALLTYPE GetPostProcessorLoggingName() = 0;
};

std::vector<wil::com_ptr<IInitialCollectionPostProcessor>>;

std::initializer_list<std::shared_ptr<CollectionInitializationPolicy>>;
std::allocator<std::shared_ptr<CollectionInitializationPolicy>>;

class BaseTileCollectionInitializer
{
public:
    BaseTileCollectionInitializer(ABI::Windows::System::IUser* user);

    virtual void InitializeCollection(const std::wstring&, bool*);
    virtual void CheckForUpdate(const std::wstring&, bool*);
    virtual void ReinitializeCollection(const std::wstring&, ABI::Windows::Foundation::IAsyncAction**);

private:
    wil::com_ptr<ABI::Windows::System::IUser> _user;
    wil::com_ptr<ICollectionWriter> _writer;
    std::shared_ptr<CollectionContext> _context;
};

std::shared_ptr<BaseTileCollectionInitializer> a; std::_Ref_count_obj2<BaseTileCollectionInitializer>;
}

wil::unique_tokeninfo_ptr<TOKEN_USER>;
wil::com_ptr<ABI::Windows::UI::StartScreen::ISecondaryTileStatics>;
wil::com_ptr<ABI::Windows::Internal::UI::StartScreen::ISecondaryTileStaticsPrivate>;
