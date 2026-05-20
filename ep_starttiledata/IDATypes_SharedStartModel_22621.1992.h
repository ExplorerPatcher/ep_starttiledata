struct IUnknown_vtbl;

#pragma pack(push, 8)
struct IUnknown
{
    struct IUnknown_vtbl* __vftable;
};
#pragma pack(pop)


#pragma pack(push, 8)
struct /*VFT*/ IUnknown_vtbl
{
    HRESULT (__stdcall *QueryInterface)(IUnknown* This, const IID* const riid, void** ppvObject);
    ULONG (__stdcall *AddRef)(IUnknown* This);
    ULONG (__stdcall *Release)(IUnknown* This);
};
#pragma pack(pop)

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

namespace Microsoft { namespace WRL
{
    template <typename T>
    class ComPtr
    {
        T* ptr_;
    };

    namespace Details
    {
        class DontUseNewUseMake
        {
        };
    }
} }

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
        enum ENTRYSTATE
        {
            EMPTY = 0,
            OCCUPIED = 1,
            FREED = 2
        };

        ENTRYSTATE _state;
        TKey _key;
        TValue _value;
    };

    UINT _cItems;
    UINT _cBuckets;
    UINT _cFreedBuckets;
    HashBucket* _rgBuckets;
};

namespace Geometry
{
    struct CRect : RECT
    {
    };
}

struct IItemLayoutResolverCallback : IUnknown
{
};

struct IItemLayoutResolverCallback_vtbl : IUnknown_vtbl
{
    void (__stdcall *ItemBoundsUpdated)(IItemLayoutResolverCallback* This, const GUID*, const RECT*, const UINT);
    void (__stdcall *LayoutBoundsUpdated)(IItemLayoutResolverCallback* This, const RECT);
    void (__stdcall *ItemRemovedPending)(IItemLayoutResolverCallback* This, const GUID&);
    void (__stdcall *ItemRemoved)(IItemLayoutResolverCallback* This, const GUID&);
    void (__stdcall *LastItemRemovedPending)(IItemLayoutResolverCallback* This);
    void (__stdcall *LastItemRemoved)(IItemLayoutResolverCallback* This);
};

struct IItemLayoutResolver;

struct IItemLayoutResolverInternalCallback : IUnknown
{
};

struct IItemLayoutResolverInternalCallback_vtbl : IUnknown_vtbl
{
    void (__stdcall *NewItemAddedBegin)(IItemLayoutResolverInternalCallback* This);
    void (__stdcall *NewItemAddedEnd)(IItemLayoutResolverInternalCallback* This);
    void (__stdcall *OnItemsMigrated)(IItemLayoutResolverInternalCallback* This, IItemLayoutResolver*);
};

enum LayoutMigrationType
{
    LayoutMigrationType_LandscapeToPortrait = 0x0,
    LayoutMigrationType_PortraitToLandscape = 0x1,
    LayoutMigrationType_ColumnChange = 0x2,
};

typedef struct _LayoutMigrationOptions
{
    LayoutMigrationType migrationType;
    GUID unassignedItemId;
} LayoutMigrationOptions;

struct IItemLayoutResolver : IUnknown
{
};

struct IItemLayoutResolver_vtbl : IUnknown_vtbl
{
    HRESULT (__stdcall *RegisterCallback)(IItemLayoutResolver* This, IItemLayoutResolverCallback*);
    HRESULT (__stdcall *UnregisterCallback)(IItemLayoutResolver* This, IItemLayoutResolverCallback*);
    HRESULT (__stdcall *RegisterInternalCallback)(IItemLayoutResolver* This, IItemLayoutResolverInternalCallback*);
    HRESULT (__stdcall *UnregisterInternalCallback)(IItemLayoutResolver* This, IItemLayoutResolverInternalCallback*);
    HRESULT (__stdcall *AddNewItem)(IItemLayoutResolver* This, const GUID&, const SIZE);
    HRESULT (__stdcall *AddItem)(IItemLayoutResolver* This, const GUID&, const RECT);
    HRESULT (__stdcall *InsertItemUncommitted)(IItemLayoutResolver* This, const GUID&, const RECT);
    HRESULT (__stdcall *ResizeItemUncommitted)(IItemLayoutResolver* This, const GUID&, const SIZE);
    HRESULT (__stdcall *SwapItemsUncommitted)(IItemLayoutResolver* This, const GUID&, const GUID&); ///< @Note: Added after 14361
    HRESULT (__stdcall *AddNewContainer)(IItemLayoutResolver* This, const GUID&, IItemLayoutResolver*);
    HRESULT (__stdcall *AddContainer)(IItemLayoutResolver* This, const GUID&, IItemLayoutResolver*, const POINT);
    HRESULT (__stdcall *AddSizedContainer)(IItemLayoutResolver* This, const GUID&, IItemLayoutResolver*, const RECT);
    HRESULT (__stdcall *InsertContainerUncommitted)(IItemLayoutResolver* This, const GUID&, IItemLayoutResolver*, const POINT);
    HRESULT (__stdcall *GetLayoutResolverForContainer)(IItemLayoutResolver* This, const GUID&, IItemLayoutResolver**);
    BOOL (__stdcall *IsCollapsed)(IItemLayoutResolver* This);
    HRESULT (__stdcall *Collapse)(IItemLayoutResolver* This);
    HRESULT (__stdcall *Expand)(IItemLayoutResolver* This);
    HRESULT (__stdcall *RemoveItemUncommitted)(IItemLayoutResolver* This, const GUID&);
    HRESULT (__stdcall *MoveItemUncommitted)(IItemLayoutResolver* This, const GUID&, const POINT);
    BOOL (__stdcall *IsEmpty)(IItemLayoutResolver* This);
    HRESULT (__stdcall *GetItemByCell)(IItemLayoutResolver* This, const POINT, GUID*);
    HRESULT (__stdcall *GetLayoutBounds)(IItemLayoutResolver* This, RECT*);
    HRESULT (__stdcall *GetLayoutBoundsWithoutItem)(IItemLayoutResolver* This, const GUID&, RECT*);
    HRESULT (__stdcall *GetItemBounds)(IItemLayoutResolver* This, const GUID&, RECT*);
    HRESULT (__stdcall *GetContainerSizeWithMargins)(IItemLayoutResolver* This, const GUID&, SIZE*);
    HRESULT (__stdcall *GetLastOccupiedCellInColumn)(IItemLayoutResolver* This, const long, const GUID&, POINT*, int*);
    HRESULT (__stdcall *SetContainerMargins)(IItemLayoutResolver* This, const RECT);
    HRESULT (__stdcall *GetContainerMargins)(IItemLayoutResolver* This, RECT*);
    HRESULT (__stdcall *SetMaxCellBounds)(IItemLayoutResolver* This, const int, const int);
    SIZE (__stdcall *GetMaxCellBounds)(IItemLayoutResolver* This);
    HRESULT (__stdcall *MigrateItems)(IItemLayoutResolver* This, IItemLayoutResolver*, const LayoutMigrationOptions);
    HRESULT (__stdcall *CommitChanges)(IItemLayoutResolver* This);
    HRESULT (__stdcall *AbandonChanges)(IItemLayoutResolver* This);
    HRESULT (__stdcall *RepairLayoutUncommitted)(IItemLayoutResolver* This);
};

struct ICellArrayManagerCallback : IUnknown
{
};

struct ICellArrayManagerCallback_vtbl : IUnknown_vtbl
{
    void (__stdcall *ItemBoundsUpdated)(ICellArrayManagerCallback* This, const GUID&, const Geometry::CRect&);
    void (__stdcall *CellArrayBoundsUpdated)(ICellArrayManagerCallback* This, const Geometry::CRect&);
    void (__stdcall *ItemRemovedPending)(ICellArrayManagerCallback* This, const GUID&);
    void (__stdcall *ItemRemoved)(ICellArrayManagerCallback* This, const GUID&);
};

struct IGroupBoundsChangeNotification : IUnknown
{
};

struct IGroupBoundsChangeNotification_vtbl : IUnknown_vtbl
{
    void (__stdcall *NewItemAddedBegin)(IGroupBoundsChangeNotification* This);
    void (__stdcall *NewItemAddedEnd)(IGroupBoundsChangeNotification* This);
    void (__stdcall *OnItemsMigrated)(IGroupBoundsChangeNotification* This, IItemLayoutResolver*, const GUID&);
    HRESULT (__stdcall *GroupBoundsChanged)(IGroupBoundsChangeNotification* This, const GUID&);
    void (__stdcall *GroupEmptiedPending)(IGroupBoundsChangeNotification* This, const GUID&);
    void (__stdcall *GroupEmptied)(IGroupBoundsChangeNotification* This);
};

struct ILayoutHitTest : IUnknown
{
};

struct ILayoutHitTest_vtbl : IUnknown_vtbl
{
    HRESULT (__stdcall *GetGutterHitTarget)(ILayoutHitTest* This, const GUID&, const RECT, POINT*);
};

struct IItemLayoutResolverInternal : IUnknown
{
};

struct IItemLayoutResolverInternal_vtbl : IUnknown_vtbl
{
    void (__stdcall *OnItemsMigrated)(IItemLayoutResolverInternal* This, IItemLayoutResolver*);
    void (__stdcall *EnableCollapse)(IItemLayoutResolverInternal* This, BOOL); ///< @Note: Added after 14361
};

class CItemLayoutResolver
    : IItemLayoutResolver
    , ICellArrayManagerCallback
    , IGroupBoundsChangeNotification
    , ILayoutHitTest
    , IItemLayoutResolverInternal
{
    Microsoft::WRL::Details::DontUseNewUseMake DontUseNewUseMake;
    ULONG refcount_;
    UINT64 field_30;
    UINT64 field_38;
    UINT64 field_40;
    UINT64 field_48;
    UINT64 field_50;
    UINT64 field_58;
    UINT64 field_60;
    UINT64 field_68;
    UINT64 field_70;
    UINT32 field_78;
    UINT8 gap7C[4];
    UINT64 field_80;
    UINT32 field_88;
    int field_8C;
    int field_90;
    UINT8 gap94[4];
    UINT64 field_98;
    UINT32 field_A0;
    int field_A4;
    int field_A8;
    UINT8 gapAC[4];
    UINT64 field_B0;
    UINT32 field_B8;
    int field_BC;
    int field_C0;
    UINT8 gapC4[4];
    UINT64 field_C8;
    char _isBatchingItemBoundsChangeUpdates;
    UINT8 gapD1[3];
    int field_D4;
    UINT32 _batchedUpdates;
    int field_DC;
    int field_E0;
    UINT8 gapE4[4];
    UINT64 field_E8;
    char m_isCollapsed;
    char field_F1;
};
