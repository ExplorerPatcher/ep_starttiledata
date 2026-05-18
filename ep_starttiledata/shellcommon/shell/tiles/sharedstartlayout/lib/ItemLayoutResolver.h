#pragma once

#include <wrl.h>
#include <wil/resource.h>

#include "ColumnChangeMigrationHandler.h"
#include "ICellArrayManager.h"

// @TODO: We will probably need these later.
interface IItemLayoutResolverCallback;
interface IItemLayoutResolverInternalCallback;

enum class LayoutMigrationType
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

// 14361 - ?            : b86bf953-828b-4f3d-987b-04a5601b23cf
// 19041 (at least)+    : dd5c8f3a-3ca5-4964-a162-192c9b8fd2bc
MIDL_INTERFACE("dd5c8f3a-3ca5-4964-a162-192c9b8fd2bc")
IItemLayoutResolver : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE RegisterCallback(IItemLayoutResolverCallback*) = 0;
    virtual HRESULT STDMETHODCALLTYPE UnregisterCallback(IItemLayoutResolverCallback*) = 0;
    virtual HRESULT STDMETHODCALLTYPE RegisterInternalCallback(IItemLayoutResolverInternalCallback*) = 0;
    virtual HRESULT STDMETHODCALLTYPE UnregisterInternalCallback(IItemLayoutResolverInternalCallback*) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddNewItem(const GUID&, const SIZE) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddItem(const GUID&, const RECT) = 0;
    virtual HRESULT STDMETHODCALLTYPE InsertItemUncommitted(const GUID&, const RECT) = 0;
    virtual HRESULT STDMETHODCALLTYPE ResizeItemUncommitted(const GUID&, const SIZE) = 0;
    virtual HRESULT STDMETHODCALLTYPE SwapItemsUncommitted(const GUID&, const GUID&) = 0; // @Note: Added after 14361
    virtual HRESULT STDMETHODCALLTYPE AddNewContainer(const GUID&, IItemLayoutResolver*) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddContainer(const GUID&, IItemLayoutResolver*, const POINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddSizedContainer(const GUID&, IItemLayoutResolver*, const RECT) = 0;
    virtual HRESULT STDMETHODCALLTYPE InsertContainerUncommitted(const GUID&, IItemLayoutResolver*, const POINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetLayoutResolverForContainer(const GUID&, IItemLayoutResolver**) = 0;
    virtual BOOL STDMETHODCALLTYPE IsCollapsed() = 0;
    virtual HRESULT STDMETHODCALLTYPE Collapse() = 0;
    virtual HRESULT STDMETHODCALLTYPE Expand() = 0;
    virtual HRESULT STDMETHODCALLTYPE RemoveItemUncommitted(const GUID&) = 0;
    virtual HRESULT STDMETHODCALLTYPE MoveItemUncommitted(const GUID&, const POINT) = 0;
    virtual BOOL STDMETHODCALLTYPE IsEmpty() = 0;
    virtual HRESULT STDMETHODCALLTYPE GetItemByCell(const POINT, GUID*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetLayoutBounds(RECT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetLayoutBoundsWithoutItem(const GUID&, RECT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetItemBounds(const GUID&, RECT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetContainerSizeWithMargins(const GUID&, SIZE*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetLastOccupiedCellInColumn(const long, const GUID&, POINT*, int*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetContainerMargins(const RECT) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetContainerMargins(RECT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetMaxCellBounds(const int, const int) = 0;
    virtual SIZE STDMETHODCALLTYPE GetMaxCellBounds() = 0;
    virtual HRESULT STDMETHODCALLTYPE MigrateItems(IItemLayoutResolver*, const LayoutMigrationOptions) = 0;
    virtual HRESULT STDMETHODCALLTYPE CommitChanges() = 0;
    virtual HRESULT STDMETHODCALLTYPE AbandonChanges() = 0;
    virtual HRESULT STDMETHODCALLTYPE RepairLayoutUncommitted() = 0;
};

enum LAYOUT_RESOLVER_OPTIONS
{
    LRO_NONE = 0x0,
    LRO_DISPLACE_INTO_NEGATIVE_SPACE = 0x1,
};

class CItemLayoutResolver
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , IItemLayoutResolver
        , ICellArrayManagerCallback
        , IGroupBoundsChangeNotification
        , ILayoutHitTest
        , IItemLayoutResolverInternal
    >
{
public:
    CItemLayoutResolver();
    ~CItemLayoutResolver();

    HRESULT RuntimeClassInitialize();

    HRESULT RegisterCallback(IItemLayoutResolverCallback*);
    HRESULT UnregisterCallback(IItemLayoutResolverCallback*);
    HRESULT RegisterInternalCallback(IItemLayoutResolverInternalCallback*);
    HRESULT UnregisterInternalCallback(IItemLayoutResolverInternalCallback*);
    HRESULT AddNewItem(const GUID&, const SIZE);
    HRESULT AddItem(const GUID&, const RECT);
    HRESULT InsertItemUncommitted(const GUID&, const RECT);
    HRESULT ResizeItemUncommitted(const GUID& itemID, const SIZE sizeItemCells);
    HRESULT AddNewContainer(_GUID&, IItemLayoutResolver*);
    HRESULT AddContainer(_GUID&, IItemLayoutResolver*, const tagPOINT);
    HRESULT AddSizedContainer(_GUID&, IItemLayoutResolver*, const tagRECT);
    HRESULT InsertContainerUncommitted(_GUID&, IItemLayoutResolver*, const tagPOINT);
    HRESULT GetLayoutResolverForContainer(_GUID&, IItemLayoutResolver**);
    BOOL IsCollapsed();
    HRESULT Collapse();
    HRESULT Expand();
    HRESULT RemoveItemUncommitted(_GUID&);
    HRESULT MoveItemUncommitted(_GUID&, const tagPOINT);
    BOOL IsEmpty();
    HRESULT GetItemByCell(const tagPOINT, _GUID*);
    HRESULT GetLayoutBounds(tagRECT*);
    HRESULT GetLayoutBoundsWithoutItem(_GUID&, tagRECT*);
    HRESULT GetItemBounds(_GUID&, tagRECT*);
    HRESULT GetContainerSizeWithMargins(_GUID&, tagSIZE*);
    HRESULT GetLastOccupiedCellInColumn(const long, _GUID&, tagPOINT*, int*);
    HRESULT SetContainerMargins(const tagRECT);
    HRESULT GetContainerMargins(tagRECT*);
    HRESULT SetMaxCellBounds(const int, const int);
    SIZE GetMaxCellBounds();
    HRESULT MigrateItems(IItemLayoutResolver*, const _LayoutMigrationOptions);
    HRESULT CommitChanges();
    HRESULT AbandonChanges();
    HRESULT RepairLayoutUncommitted();

    void ItemBoundsUpdated(_GUID&, Geometry::CRect&);
    void CellArrayBoundsUpdated(Geometry::CRect&);
    void ItemRemovedPending(_GUID&);
    void ItemRemoved(_GUID&);
    void NewItemAddedBegin();
    void NewItemAddedEnd();
    void OnItemsMigrated(IItemLayoutResolver*);
    void OnItemsMigrated(IItemLayoutResolver*, _GUID&);
    HRESULT GroupBoundsChanged(_GUID&);
    void GroupEmptiedPending(_GUID&);
    void GroupEmptied();
    HRESULT GetGutterHitTarget(_GUID&, const tagRECT, tagPOINT*);

protected:
    HRESULT _FindTargetDestinationForNewSize(_GUID&, tagSIZE&, Geometry::CRect*);
    IItemCellAssignor* _GetCellAssignor();
    HRESULT _PrepareLayoutBeforeOperation(Geometry::CRect&, Geometry::CRect&);
    HRESULT _CleanupLayoutAfterOperation(Geometry::CRect&, Geometry::CRect&);
    HRESULT _RepairLayout();
    HRESULT _ModifyItemUncommittedInternal(_GUID&, tagRECT&, const ModificationOperation);
    HRESULT _CommitChangesInternal();
    HRESULT _Collapse(Geometry::CRect&, Geometry::CRect&);
    void _NotifyNewItemAddedBegin();
    void _NotifyNewItemAddedEnd();
    void _NotifyItemBoundsChange(_GUID&, tagRECT&);
    void _NotifyLayoutBoundsChange(Geometry::CRect&);
    void _NotifyItemRemovedPending(_GUID&);
    void _NotifyItemRemoved(_GUID&);
    void _NotifyLastItemRemovedPending();
    void _NotifyLastItemRemoved();

    Microsoft::WRL::ComPtr<ICellArrayManager> _spCellArrayManager;
    CItemLayoutDisplacement _displacementManager;
    CItemLayoutCollapseManager _collapseManager;
    LAYOUT_RESOLVER_OPTIONS _options;
    Microsoft::WRL::ComPtr<ItemLayoutResolverProxy> m_itemLayoutResolverProxy;
    CSimpleHashTable<GUID, Microsoft::WRL::ComPtr<IItemLayoutResolver>> m_folderResolvers;

private:
    CSimpleHashTable<IItemLayoutResolverCallback*, Microsoft::WRL::ComPtr<IItemLayoutResolverCallback>> _htCallbacks;
    CSimpleHashTable<IItemLayoutResolverInternalCallback*, Microsoft::WRL::ComPtr<IItemLayoutResolverInternalCallback>> _htInternalCallbacks;
    bool _isBatchingItemBoundsChangeUpdates;
    CSimpleHashTable<GUID, Geometry::CRect> _batchedUpdates;
    bool m_isCollapsed;

    HRESULT _DisplaceItemsFromRect(const Geometry::CRect&, const Geometry::CRect&);
    HRESULT _StartBatchingItemBoundsChangeUpdates();
    HRESULT _StopBatchingItemBoundsChangeUpdatesAndNotify();

    HRESULT s_CreateMigrationHandler(
        const LayoutMigrationOptions& migrationOptions, IItemMigrationHandler** ppMigrationHandler);
};
