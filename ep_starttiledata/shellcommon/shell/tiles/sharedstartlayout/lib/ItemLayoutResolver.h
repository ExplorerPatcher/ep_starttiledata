#pragma once

#include <wrl.h>
#include <wil/resource.h>

#include "ColumnChangeMigrationHandler.h"
#include "ICellArrayManager.h"
#include "IItemLayoutCollapseHandler.h"

MIDL_INTERFACE("751657ce-45b3-4150-8f86-fda773983ebc")
IItemLayoutResolverCallback : IUnknown
{
    virtual void STDMETHODCALLTYPE ItemBoundsUpdated(const GUID*, const RECT*, const UINT) = 0;
    virtual void STDMETHODCALLTYPE LayoutBoundsUpdated(const RECT) = 0;
    virtual void STDMETHODCALLTYPE ItemRemovedPending(REFGUID itemID) = 0;
    virtual void STDMETHODCALLTYPE ItemRemoved(REFGUID itemID) = 0;
    virtual void STDMETHODCALLTYPE LastItemRemovedPending() = 0;
    virtual void STDMETHODCALLTYPE LastItemRemoved() = 0;
};

interface IItemLayoutResolver;

MIDL_INTERFACE("c20f0b4b-91e4-47a6-a785-c0d735c7b6c6")
IItemLayoutResolverInternalCallback : IUnknown
{
    virtual void STDMETHODCALLTYPE NewItemAddedBegin() = 0;
    virtual void STDMETHODCALLTYPE NewItemAddedEnd() = 0;
    virtual void STDMETHODCALLTYPE OnItemsMigrated(IItemLayoutResolver* pDestinationLayoutResolver) = 0;
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

// 14361 - ?            : b86bf953-828b-4f3d-987b-04a5601b23cf
// 19041 (at least)+    : dd5c8f3a-3ca5-4964-a162-192c9b8fd2bc
MIDL_INTERFACE("dd5c8f3a-3ca5-4964-a162-192c9b8fd2bc")
IItemLayoutResolver : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE RegisterCallback(IItemLayoutResolverCallback*) = 0;
    virtual HRESULT STDMETHODCALLTYPE UnregisterCallback(IItemLayoutResolverCallback*) = 0;
    virtual HRESULT STDMETHODCALLTYPE RegisterInternalCallback(IItemLayoutResolverInternalCallback*) = 0;
    virtual HRESULT STDMETHODCALLTYPE UnregisterInternalCallback(IItemLayoutResolverInternalCallback*) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddNewItem(REFGUID, const SIZE) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddItem(REFGUID, const RECT) = 0;
    virtual HRESULT STDMETHODCALLTYPE InsertItemUncommitted(REFGUID, const RECT) = 0;
    virtual HRESULT STDMETHODCALLTYPE ResizeItemUncommitted(REFGUID, const SIZE) = 0;
    virtual HRESULT STDMETHODCALLTYPE SwapItemsUncommitted(REFGUID, REFGUID) = 0; // @Note: Added after 14361
    virtual HRESULT STDMETHODCALLTYPE AddNewContainer(REFGUID, IItemLayoutResolver*) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddContainer(REFGUID, IItemLayoutResolver*, const POINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddSizedContainer(REFGUID, IItemLayoutResolver*, const RECT) = 0;
    virtual HRESULT STDMETHODCALLTYPE InsertContainerUncommitted(REFGUID, IItemLayoutResolver*, const POINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetLayoutResolverForContainer(REFGUID, IItemLayoutResolver**) = 0;
    virtual BOOL STDMETHODCALLTYPE IsCollapsed() = 0;
    virtual HRESULT STDMETHODCALLTYPE Collapse() = 0;
    virtual HRESULT STDMETHODCALLTYPE Expand() = 0;
    virtual HRESULT STDMETHODCALLTYPE RemoveItemUncommitted(REFGUID) = 0;
    virtual HRESULT STDMETHODCALLTYPE MoveItemUncommitted(REFGUID, const POINT) = 0;
    virtual BOOL STDMETHODCALLTYPE IsEmpty() = 0;
    virtual HRESULT STDMETHODCALLTYPE GetItemByCell(const POINT, GUID*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetLayoutBounds(RECT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetLayoutBoundsWithoutItem(REFGUID, RECT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetItemBounds(REFGUID, RECT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetContainerSizeWithMargins(REFGUID, SIZE*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetLastOccupiedCellInColumn(const long, REFGUID, POINT*, int*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetContainerMargins(const RECT) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetContainerMargins(RECT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetMaxCellBounds(const int, const int) = 0;
    virtual SIZE STDMETHODCALLTYPE GetMaxCellBounds() = 0;
    virtual HRESULT STDMETHODCALLTYPE MigrateItems(IItemLayoutResolver*, const LayoutMigrationOptions) = 0;
    virtual HRESULT STDMETHODCALLTYPE CommitChanges() = 0;
    virtual HRESULT STDMETHODCALLTYPE AbandonChanges() = 0;
    virtual HRESULT STDMETHODCALLTYPE RepairLayoutUncommitted() = 0;
};

MIDL_INTERFACE("7749f118-6f2a-4fb7-b84d-eef688cbd0e6")
IGroupBoundsChangeNotification : IUnknown
{
    virtual void STDMETHODCALLTYPE NewItemAddedBegin() = 0;
    virtual void STDMETHODCALLTYPE NewItemAddedEnd() = 0;
    virtual void STDMETHODCALLTYPE OnItemsMigrated(IItemLayoutResolver*, REFGUID) = 0;
    virtual HRESULT STDMETHODCALLTYPE GroupBoundsChanged(REFGUID) = 0;
    virtual void STDMETHODCALLTYPE GroupEmptiedPending(REFGUID) = 0;
    virtual void STDMETHODCALLTYPE GroupEmptied() = 0;
};

MIDL_INTERFACE("aa22a8ff-a704-4ea8-82b5-3c51d9dd3b3c")
ILayoutHitTest : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetGutterHitTarget(REFGUID, const RECT, POINT*) = 0;
};

MIDL_INTERFACE("6f13ad3b-7bf5-4ba5-b676-87f3ad61ae04")
IItemLayoutResolverInternal : IUnknown
{
    virtual void STDMETHODCALLTYPE OnItemsMigrated(IItemLayoutResolver*) = 0;
    virtual void STDMETHODCALLTYPE EnableCollapse(BOOL) = 0; ///< @Note: Added after 14361
};

enum class ModificationOperation
{
    Insert = 0,
    Move = 1,
    Remove = 2,
    Resize = 3,
};

MIDL_INTERFACE("d364071e-0afe-427c-a397-e9e1f734d4dd")
IItemLayoutDisplacementHandler : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE SetCellArray(ICellArrayManager* cellArrayManager) = 0;
    virtual HRESULT STDMETHODCALLTYPE DisplaceItemsFromRect(const Geometry::CRect& targetRect, const Geometry::CRect& previousRect) = 0;
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
    LRO_NONE = 0,
    LRO_DISPLACE_INTO_NEGATIVE_SPACE = 0x1,
};

DEFINE_ENUM_FLAG_OPERATORS(LAYOUT_RESOLVER_OPTIONS);

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

    virtual HRESULT RuntimeClassInitialize();

    //~ Begin IItemLayoutResolver Interface
    STDMETHODIMP RegisterCallback(IItemLayoutResolverCallback* callback) override;
    STDMETHODIMP UnregisterCallback(IItemLayoutResolverCallback* callback) override;
    STDMETHODIMP RegisterInternalCallback(IItemLayoutResolverInternalCallback* callback) override;
    STDMETHODIMP UnregisterInternalCallback(IItemLayoutResolverInternalCallback* callback) override;
    STDMETHODIMP AddNewItem(REFGUID itemID, const SIZE sizeItemCells) override;
    STDMETHODIMP AddItem(REFGUID itemID, const RECT rcItemBoundsCells) override;
    STDMETHODIMP InsertItemUncommitted(REFGUID itemID, const RECT rcDestination) override;
    STDMETHODIMP ResizeItemUncommitted(REFGUID itemID, const SIZE sizeItemCells) override;
    STDMETHODIMP SwapItemsUncommitted(REFGUID itemID1, REFGUID itemID2) override; // @Note: Added after 14361
    STDMETHODIMP AddNewContainer(REFGUID containerID, IItemLayoutResolver* resolver) override;
    STDMETHODIMP AddContainer(REFGUID containerID, IItemLayoutResolver* resolver, const POINT destination) override;
    STDMETHODIMP AddSizedContainer(REFGUID containerID, IItemLayoutResolver* pResolver, const RECT destination) override;
    STDMETHODIMP InsertContainerUncommitted(REFGUID containerID, IItemLayoutResolver* resolver, const POINT destination) override;
    STDMETHODIMP GetLayoutResolverForContainer(REFGUID containerID, IItemLayoutResolver** ppResolver) override;
    STDMETHODIMP_(BOOL) IsCollapsed() override;
    STDMETHODIMP Collapse() override;
    STDMETHODIMP Expand() override;
    STDMETHODIMP RemoveItemUncommitted(REFGUID itemID) override;
    STDMETHODIMP MoveItemUncommitted(REFGUID itemID, const POINT destination) override;
    STDMETHODIMP_(BOOL) IsEmpty() override;
    STDMETHODIMP GetItemByCell(const POINT ptCell, GUID* pItemID) override;
    STDMETHODIMP GetLayoutBounds(RECT* prcBounds) override;
    STDMETHODIMP GetLayoutBoundsWithoutItem(REFGUID itemID, RECT* bounds) override;
    STDMETHODIMP GetItemBounds(REFGUID itemID, RECT* itemBounds) override;
    STDMETHODIMP GetContainerSizeWithMargins(REFGUID containerID, SIZE* size) override;
    STDMETHODIMP GetLastOccupiedCellInColumn(
        const long column, REFGUID excludedItemID, POINT* occupiedCell, int* columnUnoccupied) override;
    STDMETHODIMP SetContainerMargins(const RECT containerMargins) override;
    STDMETHODIMP GetContainerMargins(RECT* containerMargins) override;
    STDMETHODIMP SetMaxCellBounds(const int nMaxXBounds, const int nMaxYBounds) override;
    STDMETHODIMP_(SIZE) GetMaxCellBounds() override;
    STDMETHODIMP MigrateItems(IItemLayoutResolver* pSourceLayout, const LayoutMigrationOptions migrationOptions) override;
    STDMETHODIMP CommitChanges() override;
    STDMETHODIMP AbandonChanges() override;
    STDMETHODIMP RepairLayoutUncommitted() override;
    //~ End IItemLayoutResolver Interface

    //~ Begin ICellArrayManagerCallback Interface
    STDMETHODIMP_(void) ItemBoundsUpdated(REFGUID itemID, const Geometry::CRect& rcItemBoundsCells) override;
    STDMETHODIMP_(void) CellArrayBoundsUpdated(const Geometry::CRect& rcLayoutBounds) override;
    STDMETHODIMP_(void) ItemRemovedPending(REFGUID itemID) override;
    STDMETHODIMP_(void) ItemRemoved(REFGUID itemID) override;
    //~ End ICellArrayManagerCallback Interface

    //~ Begin IGroupBoundsChangeNotification Interface
    STDMETHODIMP_(void) NewItemAddedBegin() override;
    STDMETHODIMP_(void) NewItemAddedEnd() override;
    STDMETHODIMP_(void) OnItemsMigrated(IItemLayoutResolver* pDestinationLayoutResolver, REFGUID groupID) override;
    STDMETHODIMP GroupBoundsChanged(REFGUID groupID) override;
    STDMETHODIMP_(void) GroupEmptiedPending(REFGUID groupID) override;
    STDMETHODIMP_(void) GroupEmptied() override;
    //~ End IGroupBoundsChangeNotification Interface

    //~ Begin ILayoutHitTest Interface
    STDMETHODIMP GetGutterHitTarget(REFGUID tileID, const RECT targetBounds, POINT* pAdjustedTargetCell) override;
    //~ End ILayoutHitTest Interface

    //~ Begin IItemLayoutResolverInternal Interface
    STDMETHODIMP_(void) OnItemsMigrated(IItemLayoutResolver* pDestinationLayout) override;
    STDMETHODIMP_(void) EnableCollapse(BOOL enableCollapse) override;
    //~ End IItemLayoutResolverInternal Interface

protected:
    virtual HRESULT _FindTargetDestinationForNewSize(
        REFGUID itemId, const SIZE& sizeItemCells, Geometry::CRect* rcItemBounds) = 0;
    virtual IItemCellAssignor* _GetCellAssignor() = 0;
    virtual HRESULT _PrepareLayoutBeforeOperation(
        const Geometry::CRect& rcSourceCells, const Geometry::CRect& rcTargetCells) = 0;
    virtual HRESULT _CleanupLayoutAfterOperation(
        const Geometry::CRect& rcSourceCells, const Geometry::CRect& rcTargetCells) = 0;
    virtual HRESULT _RepairLayout() = 0;
    virtual HRESULT _ModifyItemUncommittedInternal(
        REFGUID itemID, const RECT& rcDestination, const ModificationOperation operation);
    virtual HRESULT _CommitChangesInternal();

    HRESULT _Collapse(const Geometry::CRect& rcSourceCells, const Geometry::CRect& rcTargetCells);
    void _NotifyNewItemAddedBegin();
    void _NotifyNewItemAddedEnd();
    void _NotifyItemBoundsChange(REFGUID itemID, const RECT& rcItemBoundsCells);
    void _NotifyLayoutBoundsChange(const Geometry::CRect& rcLayoutBoundsCells);
    void _NotifyItemRemovedPending(REFGUID itemID);
    void _NotifyItemRemoved(REFGUID itemID);
    void _NotifyLastItemRemovedPending();
    void _NotifyLastItemRemoved();

    Microsoft::WRL::ComPtr<ICellArrayManager> _spCellArrayManager;
    CItemLayoutDisplacement _displacementManager;
    CItemLayoutCollapseManager _collapseManager;
    LAYOUT_RESOLVER_OPTIONS _options;
    Microsoft::WRL::ComPtr<class ItemLayoutResolverProxy> m_itemLayoutResolverProxy;
    CSimpleHashTable<GUID, Microsoft::WRL::ComPtr<IItemLayoutResolver>> m_folderResolvers;

private:
    CSimpleHashTable<IItemLayoutResolverCallback*, Microsoft::WRL::ComPtr<IItemLayoutResolverCallback>> _htCallbacks;
    CSimpleHashTable<IItemLayoutResolverInternalCallback*, Microsoft::WRL::ComPtr<IItemLayoutResolverInternalCallback>> _htInternalCallbacks;
    bool _isBatchingItemBoundsChangeUpdates;
    CSimpleHashTable<GUID, Geometry::CRect> _batchedUpdates;
    bool m_isCollapsed;
    bool m_enableCollapse; // @Note: Added after 14361

    HRESULT _DisplaceItemsFromRect(const Geometry::CRect& targetRect, const Geometry::CRect& previousRect);
    HRESULT _StartBatchingItemBoundsChangeUpdates();
    HRESULT _StopBatchingItemBoundsChangeUpdatesAndNotify();

    static HRESULT s_CreateMigrationHandler(
        const LayoutMigrationOptions& migrationOptions, IItemMigrationHandler** ppMigrationHandler);
};
