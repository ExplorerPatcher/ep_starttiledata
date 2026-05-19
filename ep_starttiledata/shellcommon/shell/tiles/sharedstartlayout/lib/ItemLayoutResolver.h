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
    ~CItemLayoutResolver() override;

    HRESULT RuntimeClassInitialize();

    //~ Begin IItemLayoutResolver Interface
    STDMETHODIMP RegisterCallback(IItemLayoutResolverCallback* callback);
    STDMETHODIMP UnregisterCallback(IItemLayoutResolverCallback* callback);
    STDMETHODIMP RegisterInternalCallback(IItemLayoutResolverInternalCallback* callback);
    STDMETHODIMP UnregisterInternalCallback(IItemLayoutResolverInternalCallback* callback);
    STDMETHODIMP AddNewItem(const GUID& itemID, const SIZE sizeItemCells);
    STDMETHODIMP AddItem(const GUID& itemID, const RECT rcItemBoundsCells);
    STDMETHODIMP InsertItemUncommitted(const GUID& itemID, const RECT rcDestination);
    STDMETHODIMP ResizeItemUncommitted(const GUID& itemID, const SIZE sizeItemCells);
    STDMETHODIMP SwapItemsUncommitted(const GUID& itemID1, const GUID& itemID2); // @Note: Added after 14361
    STDMETHODIMP AddNewContainer(const GUID&, IItemLayoutResolver*);
    STDMETHODIMP AddContainer(const GUID&, IItemLayoutResolver*, const POINT);
    STDMETHODIMP AddSizedContainer(const GUID& containerID, IItemLayoutResolver* pResolver, const RECT destination);
    STDMETHODIMP InsertContainerUncommitted(const GUID&, IItemLayoutResolver*, const POINT);
    STDMETHODIMP GetLayoutResolverForContainer(const GUID&, IItemLayoutResolver**);
    STDMETHODIMP_(BOOL) IsCollapsed();
    STDMETHODIMP Collapse();
    STDMETHODIMP Expand();
    STDMETHODIMP RemoveItemUncommitted(const GUID&);
    STDMETHODIMP MoveItemUncommitted(const GUID&, const POINT);
    STDMETHODIMP_(BOOL) IsEmpty();
    STDMETHODIMP GetItemByCell(const POINT, GUID*);
    STDMETHODIMP GetLayoutBounds(RECT*);
    STDMETHODIMP GetLayoutBoundsWithoutItem(const GUID&, RECT*);
    STDMETHODIMP GetItemBounds(_GUID&, tagRECT*);
    STDMETHODIMP GetContainerSizeWithMargins(const GUID&, SIZE*);
    STDMETHODIMP GetLastOccupiedCellInColumn(const long, GUID&, tagPOINT*, int*);
    STDMETHODIMP SetContainerMargins(const tagRECT);
    STDMETHODIMP GetContainerMargins(tagRECT*);
    STDMETHODIMP SetMaxCellBounds(const int, const int);
    STDMETHODIMP_(SIZE) GetMaxCellBounds();
    STDMETHODIMP MigrateItems(IItemLayoutResolver*, const LayoutMigrationOptions);
    STDMETHODIMP CommitChanges();
    STDMETHODIMP AbandonChanges();
    STDMETHODIMP RepairLayoutUncommitted();
    //~ End IItemLayoutResolver Interface

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
    virtual HRESULT _FindTargetDestinationForNewSize(const GUID&, const SIZE&, Geometry::CRect*) = 0;
    virtual IItemCellAssignor* _GetCellAssignor() = 0;
    virtual HRESULT _PrepareLayoutBeforeOperation(Geometry::CRect&, Geometry::CRect&) = 0;
    virtual HRESULT _CleanupLayoutAfterOperation(Geometry::CRect&, Geometry::CRect&) = 0;
    virtual HRESULT _RepairLayout() = 0;
    virtual HRESULT _ModifyItemUncommittedInternal(
        const GUID& itemID, const RECT& rcDestination, const ModificationOperation operation);
    virtual HRESULT _CommitChangesInternal();

    HRESULT _Collapse(const Geometry::CRect&, const Geometry::CRect&);
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
