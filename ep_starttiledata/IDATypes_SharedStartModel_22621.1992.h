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
	namespace Details
	{
		template <typename T>
		class ComPtrRefBase
		{
			T* ptr_;
		};
		
		template <typename T>
		class ComPtrRef : Details::ComPtrRefBase<T>
		{
		};
	}
	
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

enum SET_ITEM_OPTIONS
{
    SIO_NONE = 0x0,
    SIO_SEND_BOUNDS_UPDATE = 0x1,
};

struct ICellArrayManager : IUnknown
{
};

struct ICellArrayManager_vtbl : IUnknown_vtbl
{
    HRESULT (__stdcall *RegisterCallback)(ICellArrayManager* This, ICellArrayManagerCallback*);
    HRESULT (__stdcall *UnregisterCallback)(ICellArrayManager* This, ICellArrayManagerCallback*);
    const Geometry::CRect& (__stdcall *GetCurrentCellArrayBounds)(ICellArrayManager* This);
    const Geometry::CSize& (__stdcall *GetMaximumCellArrayDimensions)(ICellArrayManager* This);
    bool (__stdcall *IsEmpty)(ICellArrayManager* This);
    bool (__stdcall *IsValidRect)(ICellArrayManager* This, const Geometry::CRect&);
    bool (__stdcall *IsValidCellCoordinate)(ICellArrayManager* This, const Geometry::CPoint&);
    GUID* (__stdcall *GetItemAtCell)(ICellArrayManager* This, GUID* retstr, const int, const int);
    bool (__stdcall *IsRectEmpty)(ICellArrayManager* This, const Geometry::CRect);
    HRESULT (__stdcall *GetItemsInRect)(ICellArrayManager* This, const Geometry::CRect, CSet<GUID>*);
    HRESULT (__stdcall *GetItemsOutsideOfRect)(ICellArrayManager* This, const Geometry::CRect, CSet<GUID>*);
    HRESULT (__stdcall *GetItemBounds)(ICellArrayManager* This, const GUID&, Geometry::CRect&);
    HRESULT (__stdcall *GetBoundingRectForItems)(ICellArrayManager* This, const CSet<GUID>&, Geometry::CRect&);
    HRESULT (__stdcall *GetLayoutBoundsWithoutItem)(ICellArrayManager* This, const GUID&, RECT*);
    HRESULT (__stdcall *ContainRectInsideFixedArrayBounds)(ICellArrayManager* This, Geometry::CRect&);
    HRESULT (__stdcall *SetMaximumCellArrayDimensions)(ICellArrayManager* This, const int, const int);
    HRESULT (__stdcall *SetItem)(ICellArrayManager* This, const GUID&, Geometry::CRect, SET_ITEM_OPTIONS);
    HRESULT (__stdcall *RemoveItemUncommitted)(ICellArrayManager* This, const GUID&);
    HRESULT (__stdcall *InsertEmptyColumn)(ICellArrayManager* This, Geometry::CRect, bool);
    HRESULT (__stdcall *MoveItemUncommitted)(ICellArrayManager* This, const GUID&, Geometry::CRect);
    HRESULT (__stdcall *SwapItemsUncommitted)(ICellArrayManager* This, const GUID&, const GUID&); ///< @Note: Added after 14361
    HRESULT (__stdcall *InsertItemUncommitted)(ICellArrayManager* This, const GUID&, const Geometry::CRect);
    HRESULT (__stdcall *AddIgnoredItem)(ICellArrayManager* This, const GUID&);
    HRESULT (__stdcall *RemoveIgnoredItem)(ICellArrayManager* This, const GUID&);
    HRESULT (__stdcall *CommitChanges)(ICellArrayManager* This);
    HRESULT (__stdcall *AbandonChanges)(ICellArrayManager* This);
    HRESULT (__stdcall *FixCoordinatesToBeNonNegative)(ICellArrayManager* This);
};

struct IItemCellAssignor : IUnknown
{
};

struct IItemCellAssignor_vtbl : IUnknown_vtbl
{
    HRESULT (__stdcall *SetCellArray)(IItemCellAssignor* This, ICellArrayManager*);
    HRESULT (__stdcall *CalculateLocationForNewItem)(IItemCellAssignor* This, const Geometry::CSize&, Geometry::CRect*);
};

struct IItemMigrationHandler : IUnknown
{
};

struct IItemMigrationHandler_vtbl : IUnknown_vtbl
{
    HRESULT (__stdcall *MigrateItems)(IItemMigrationHandler* This, ICellArrayManager*, ICellArrayManager*, IItemCellAssignor*);
    HRESULT (__stdcall *SetUnassignedItemId)(IItemMigrationHandler* This, const GUID&);
};

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
    SIZE* (__stdcall *GetMaxCellBounds)(IItemLayoutResolver* This, SIZE* retstr);
    HRESULT (__stdcall *MigrateItems)(IItemLayoutResolver* This, IItemLayoutResolver*, const LayoutMigrationOptions);
    HRESULT (__stdcall *CommitChanges)(IItemLayoutResolver* This);
    HRESULT (__stdcall *AbandonChanges)(IItemLayoutResolver* This);
    HRESULT (__stdcall *RepairLayoutUncommitted)(IItemLayoutResolver* This);
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

struct IItemLayoutDisplacementHandler : IUnknown
{
};

struct IItemLayoutDisplacementHandler_vtbl : IUnknown_vtbl
{
    HRESULT (__stdcall *SetCellArray)(IItemLayoutDisplacementHandler* This, ICellArrayManager*);
    HRESULT (__stdcall *DisplaceItemsFromRect)(IItemLayoutDisplacementHandler* This, const Geometry::CRect&, const Geometry::CRect&);
};

struct IItemLayoutCollapseHandler : IUnknown
{
};

struct IItemLayoutCollapseHandler_vtbl : IUnknown_vtbl
{
    HRESULT (__stdcall *SetCellArray)(IItemLayoutCollapseHandler* This, ICellArrayManager*);
    HRESULT (__stdcall *Collapse)(IItemLayoutCollapseHandler* This, const Geometry::CRect&, const Geometry::CRect&);
};

class CItemLayoutDisplacement
{
public:
    CItemLayoutDisplacement();

    HRESULT DisplaceItemsFromRect(
        const Geometry::CRect& targetRect, const Geometry::CRect& previousRect, ICellArrayManager* cellArrayManager);
    HRESULT AddDisplacementHandler(IItemLayoutDisplacementHandler* displacementHandler);

private:
    CCoSimpleArray<Microsoft::WRL::ComPtr<IItemLayoutDisplacementHandler>> m_displacementHandlers;
};

class CItemLayoutCollapseManager
{
public :
    CItemLayoutCollapseManager();

    HRESULT Collapse(
        const Geometry::CRect& sourceCells, const Geometry::CRect& targetCells, ICellArrayManager* cellArrayManager);
    HRESULT AddCollapseHandler(IItemLayoutCollapseHandler* collapseHandler);

private:
    CCoSimpleArray<Microsoft::WRL::ComPtr<IItemLayoutCollapseHandler>> m_collapseHandlers;
};

enum LAYOUT_RESOLVER_OPTIONS
{
    LRO_NONE = 0x0,
    LRO_DISPLACE_INTO_NEGATIVE_SPACE = 0x1,
};

class CItemLayoutResolver;

class ItemLayoutResolverProxy
    : ICellArrayManagerCallback
    , IGroupBoundsChangeNotification
{
    Microsoft::WRL::Details::DontUseNewUseMake DontUseNewUseMake;
    ULONG refcount_;

    CItemLayoutResolver* m_callbackNoRef;
};

enum /*class*/ ModificationOperation
{
    ModificationOperation_Insert = 0,
    ModificationOperation_Move = 1,
    ModificationOperation_Remove = 2,
    ModificationOperation_Resize = 3,
};

class CItemLayoutResolver
    : public IItemLayoutResolver
    , public ICellArrayManagerCallback
    , public IGroupBoundsChangeNotification
    , public ILayoutHitTest
    , public IItemLayoutResolverInternal
{
    Microsoft::WRL::Details::DontUseNewUseMake DontUseNewUseMake;
    ULONG refcount_;

    Microsoft::WRL::ComPtr<ICellArrayManager> _spCellArrayManager;
    CItemLayoutDisplacement _displacementManager;
    CItemLayoutCollapseManager _collapseManager;
    LAYOUT_RESOLVER_OPTIONS _options;
    Microsoft::WRL::ComPtr<ItemLayoutResolverProxy> m_itemLayoutResolverProxy;
    CSimpleHashTable<GUID, Microsoft::WRL::ComPtr<IItemLayoutResolver>> m_folderResolvers;
    CSimpleHashTable<IItemLayoutResolverCallback*, Microsoft::WRL::ComPtr<IItemLayoutResolverCallback>> _htCallbacks;
    CSimpleHashTable<IItemLayoutResolverInternalCallback*, Microsoft::WRL::ComPtr<IItemLayoutResolverInternalCallback>> _htInternalCallbacks;
    char _isBatchingItemBoundsChangeUpdates;
    CSimpleHashTable<GUID, Geometry::CRect> _batchedUpdates;
    char m_isCollapsed;
    char m_isUnk1; ///< @Note: Added after 14361
};

enum /*class*/ AutoIgnoredItemArray
{
    AutoIgnoredItemArray_Pending = 0,
    AutoIgnoredItemArray_Committed = 1,
};

class AutoIgnoredItem
{
    AutoIgnoredItemArray _arrayToModify;
    Microsoft::WRL::ComPtr<ICellArrayManager> _spCellArrayManager;
    GUID _tileID;
};

struct CItemLayoutResolver_vtbl : IItemLayoutResolver_vtbl // Make sure after the type
{
    void* (__thiscall *__vecDelDtor)(UINT);

    HRESULT (__thiscall *RuntimeClassInitialize)(CItemLayoutResolver* This);

    HRESULT (__thiscall *_FindTargetDestinationForNewSize)(CItemLayoutResolver* This, const GUID&, const SIZE&, Geometry::CRect*);
    IItemCellAssignor* (__thiscall *_GetCellAssignor)(CItemLayoutResolver* This);
    HRESULT (__thiscall *_PrepareLayoutBeforeOperation)(CItemLayoutResolver* This, const Geometry::CRect&, const Geometry::CRect&);
    HRESULT (__thiscall *_CleanupLayoutAfterOperation)(CItemLayoutResolver* This, const Geometry::CRect&, const Geometry::CRect&);
    HRESULT (__thiscall *_RepairLayout)(CItemLayoutResolver* This);
    HRESULT (__thiscall *_ModifyItemUncommittedInternal)(CItemLayoutResolver* This, const GUID& itemID, const RECT& rcDestination, const ModificationOperation operation);
    HRESULT (__thiscall *_CommitChangesInternal)(CItemLayoutResolver* This);
};

class CEmptyCellDisplacementHandler
    : IItemLayoutDisplacementHandler
{
    Microsoft::WRL::Details::DontUseNewUseMake DontUseNewUseMake;
    ULONG refcount_;

    ICellArrayManager* m_cellArrayManager;
};

enum DISPLACEMENT_DIRECTION
{
    DD_UP = 0,
    DD_DOWN = 1,
    DD_LEFT = 2,
    DD_RIGHT = 3,
    DD_MAX = 4,
};

class CBaseDisplacementHandler
    : IItemLayoutDisplacementHandler
{
    Microsoft::WRL::Details::DontUseNewUseMake DontUseNewUseMake;
    ULONG refcount_;

    ICellArrayManager* m_cellArrayManager;
};

enum SINGLE_TILE_ADJACENT_OPTION_FLAGS
{
    STAOF_NONE = 0,
    STAOF_PREFER_SHORTEST_DISPLACEMENT = 0x1,
    STAOF_DISPLACE_BLOCK_IS_CONTAINED_IN_TARGET = 0x2,
    STAOF_DISPLACE_INTO_NEGATIVE_SPACE = 0x4,
    STAOF_PREFER_SWAP_OVER_DIRECTION_PRIORITY = 0x8,
};

class CAdjacentDisplacementHandler
    : CBaseDisplacementHandler
{
    void* gap[3];

    Microsoft::WRL::Details::DontUseNewUseMake DontUseNewUseMake;
    ULONG refcount_;

    struct DisplaceResult
    {
        bool valid;
        Geometry::CPoint offset;
        int distanceMoved;
        Geometry::CRect displacementDestination;
    };

    CCoSimpleArray<DISPLACEMENT_DIRECTION> m_directionPriorities;
    SINGLE_TILE_ADJACENT_OPTION_FLAGS m_options;
};

enum EXPAND_COLLAPSE_DIRECTION
{
    EXPAND_COLLAPSE_DIRECTION_ROW = 0,
    EXPAND_COLLAPSE_DIRECTION_COLUMN = 1,
};

class CPathCollapseHandler
    : IItemLayoutCollapseHandler
{
    Microsoft::WRL::Details::DontUseNewUseMake DontUseNewUseMake;
    ULONG refcount_;

    ICellArrayManager* m_cellArrayManager;
    EXPAND_COLLAPSE_DIRECTION m_expandCollapseDirection;
    bool m_smartCollapseEnabled;
};

class __cppobj CPortraitTileLayoutResolver : CItemLayoutResolver
{
    Microsoft::WRL::ComPtr<IItemCellAssignor> _spCellAssignor;
    Microsoft::WRL::ComPtr<CPathCollapseHandler> m_pathCollapseHandler;
    Geometry::CRect m_lastCleanupSource;
    Geometry::CRect m_lastCleanupTarget;
};

struct ICellArray : IUnknown
{
};

struct ICellArray_vtbl : IUnknown_vtbl
{
    HRESULT (__stdcall *CloneArray)(ICellArray*, ICellArray**);
    GUID* (__stdcall *GetCellValue)(ICellArray*, GUID* retstr, const int, const int);
    const Geometry::CRect* (__stdcall *GetArrayBounds)(ICellArray*);
    void (__stdcall *GetItemsInRect)(ICellArray*, const Geometry::CRect&, CSet<GUID>*);
    void (__stdcall *FixCoordinatesToBeNonNegative)(ICellArray*);
    HRESULT (__stdcall *SetItem)(ICellArray*, const GUID&, const Geometry::CRect&);
    HRESULT (__stdcall *RemoveItem)(ICellArray*, const GUID&);
    HRESULT (__stdcall *SetArrayBounds)(ICellArray*, const Geometry::CRect&);
    HRESULT (__stdcall *AddIgnoredItem)(ICellArray*, const GUID&);
    HRESULT (__stdcall *RemoveIgnoredItem)(ICellArray*, const GUID&);
};

class CCellArray
	: ICellArray
{
	Microsoft::WRL::Details::DontUseNewUseMake DontUseNewUseMake;
    ULONG refcount_;

	Geometry::CRect _rcArrayBounds;
	GUID* _rgCellData;
	CSet<GUID> _setIgnoredTiles;
	Geometry::CPoint _ptOrigin;
};

struct ICommittedCellArrayManager : IUnknown
{
};

struct ICommittedCellArrayManager_vtbl : IUnknown_vtbl
{
	GUID* (__stdcall *GetCommittedItemAtCell)(ICommittedCellArrayManager*, GUID* retstr, const int, const int);
    HRESULT (__stdcall *GetCommittedItemBounds)(ICommittedCellArrayManager*, const GUID&, const Geometry::CRect&);
    HRESULT (__stdcall *GetCommittedItemsInRect)(ICommittedCellArrayManager*, const Geometry::CRect&, CSet<GUID>*);
    HRESULT (__stdcall *AddIgnoredCommittedItem)(ICommittedCellArrayManager*, const GUID&);
    HRESULT (__stdcall *RemoveIgnoredCommittedItem)(ICommittedCellArrayManager*, const GUID&);
};

class CCellArrayManager
	: ICellArrayManager
	, ICommittedCellArrayManager
{
	Microsoft::WRL::Details::DontUseNewUseMake DontUseNewUseMake;
    ULONG refcount_;

	CSimpleHashTable<ICellArrayManagerCallback*, Microsoft::WRL::ComPtr<ICellArrayManagerCallback>, CDefaultHashPolicy<ICellArrayManagerCallback*>, CDefaultKeyCompare<ICellArrayManagerCallback*>, CDefaultResizePolicy, CDefaultRehashPolicy> _htCallbacks;
	Microsoft::WRL::ComPtr<ICellArray> _pCellArray; ///< Microsoft::WRL::ComPtr<ICellArray
	Geometry::CSize _sizeMaxBounds;
	CSimpleHashTable<GUID, Geometry::CRect, CDefaultHashPolicy<GUID>, CDefaultKeyCompare<GUID>, CDefaultResizePolicy, CDefaultRehashPolicy> _htTileBounds;
	Microsoft::WRL::ComPtr<ICellArray> _pCommittedCellArray; ///< Microsoft::WRL::ComPtr<ICellArray
	CSimpleHashTable<GUID, Geometry::CRect, CDefaultHashPolicy<_GUID>, CDefaultKeyCompare<GUID>, CDefaultResizePolicy, CDefaultRehashPolicy> _htCommittedTileBounds;
	GUID m_removedItem;
};

class CGroupsLayoutResolverCallbackListener
    : public IItemLayoutResolverInternalCallback
    , public IItemLayoutResolverCallback
{
    Microsoft::WRL::Details::DontUseNewUseMake DontUseNewUseMake;
    ULONG refcount_;

    bool m_iteratingOverCallbacks;
    GUID m_groupID;
    Microsoft::WRL::ComPtr<IItemLayoutResolver> m_resolver;
    CCoSimpleArray<IGroupBoundsChangeNotification*> m_resolversToUnregister;
    CSimpleHashTable<IGroupBoundsChangeNotification*, Microsoft::WRL::ComPtr<IGroupBoundsChangeNotification>> m_callbacks;
};

class CGroupsLayoutResolver : public CItemLayoutResolver
{
    struct GroupResolverInternal
    {
        Microsoft::WRL::ComPtr<IItemLayoutResolver> resolver;
        Microsoft::WRL::ComPtr<CGroupsLayoutResolverCallbackListener> resolverCallback;
    };

    /*const*/ int c_groupWidth;
    bool m_newItemBeingAdded;
    GUID m_pendingRemovedGroup;
    RECT m_containerMargins;
    Microsoft::WRL::ComPtr<IItemCellAssignor> m_cellAssignor;
    CSimpleHashTable<GUID, GroupResolverInternal> m_groupResolvers;
};

class CEmptyColumnCollapseMigrationHandler
    : public IItemMigrationHandler
{
    Microsoft::WRL::Details::DontUseNewUseMake DontUseNewUseMake;
    ULONG refcount_;
};
class CPortraitTileCellAssignor
    : IItemCellAssignor
{
    Microsoft::WRL::Details::DontUseNewUseMake DontUseNewUseMake;
    ULONG refcount_;

    Microsoft::WRL::ComPtr<ICellArrayManager> _spCellArrayManager;
};

class CPushDownDisplacementHandler
    : IItemLayoutDisplacementHandler
{
    Microsoft::WRL::Details::DontUseNewUseMake DontUseNewUseMake;
    ULONG refcount_;

    ICellArrayManager* m_cellArrayManager;
};

class CExpandDisplacementHandler
    : IItemLayoutDisplacementHandler
{
    Microsoft::WRL::Details::DontUseNewUseMake DontUseNewUseMake;
    ULONG refcount_;

    ICellArrayManager* m_cellArrayManager;
    EXPAND_COLLAPSE_DIRECTION m_rowOrColumn;
};

template <typename T>
class CRefCountedObject
{
    ULONG _cRef;
};

class CCompoundDisplacementHandler
    : IItemLayoutDisplacementHandler
{
    Microsoft::WRL::Details::DontUseNewUseMake DontUseNewUseMake;
    ULONG refcount_;

    struct PendingCellCoordinates
    {
        GUID cell;
        Geometry::CPoint position;
    };

    using RefCountedPendingCellCoordinates = Microsoft::WRL::ComPtr<CRefCountedObject<PendingCellCoordinates>>;

    class DisplacementEvaluation
    {
    public:
        DISPLACEMENT_DIRECTION direction;
        bool possible;
        int distance;
        CCoSimpleArray<RefCountedPendingCellCoordinates> chain;
        Geometry::CPoint position;
    };

    using RefCountedDisplacementEvaluation = Microsoft::WRL::ComPtr<CRefCountedObject<DisplacementEvaluation>>;

    class DisplacementEvaluationSet
    {
    public:
        RefCountedDisplacementEvaluation right;
        RefCountedDisplacementEvaluation left;
        RefCountedDisplacementEvaluation down;
        RefCountedDisplacementEvaluation up;
        RefCountedDisplacementEvaluation selected;
    };

    enum ShiftCriteria
    {
        Before = 0,
        After = 1
    };

    ICellArrayManager* m_cellArrayManager;
};

enum LayoutNavigationDirection
{
    LayoutNavigationDirection_Up = 0x0,
    LayoutNavigationDirection_Down = 0x1,
    LayoutNavigationDirection_Left = 0x2,
    LayoutNavigationDirection_Right = 0x3,
};

struct ILayoutTraversalOrder : IUnknown
{
};

struct ILayoutTraversalOrder_vtbl : IUnknown_vtbl
{
    HRESULT (__stdcall*GetAdjacent)(
        ILayoutTraversalOrder* This, const LayoutNavigationDirection, const POINT, POINT*, GUID*);
    HRESULT (__stdcall*GetFirst)(ILayoutTraversalOrder* This, POINT*, GUID*);
};

class CGenericTraversalOrder
    : ILayoutTraversalOrder
{
    Microsoft::WRL::Details::DontUseNewUseMake DontUseNewUseMake;
    ULONG refcount_;

    Microsoft::WRL::ComPtr<IItemLayoutResolver> m_layoutResolver;
    int m_maxGroupWidth;
};

class CGridTraversalOrder : CGenericTraversalOrder
{
};

class CLinearTraversalOrder : CGenericTraversalOrder
{
};
