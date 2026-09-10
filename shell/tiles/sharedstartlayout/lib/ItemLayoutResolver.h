#pragma once

#include "ColumnChangeMigrationHandler.h"
#include "ICellArrayManager.h"
#include "ItemLayoutCollapseManager.h"
#include "ItemLayoutDisplacement.h"
#include "SharedStartModel.h"

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

MIDL_INTERFACE("6f13ad3b-7bf5-4ba5-b676-87f3ad61ae04")
IItemLayoutResolverInternal : IUnknown
{
    virtual void STDMETHODCALLTYPE OnItemsMigrated(IItemLayoutResolver*) = 0;
    virtual void STDMETHODCALLTYPE EnableCollapse(BOOL) = 0; ///< @Note: Added in 15063
};

enum class ModificationOperation
{
    Insert = 0,
    Move = 1,
    Remove = 2,
    Resize = 3,
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
    STDMETHODIMP SwapItemsUncommitted(REFGUID itemID1, REFGUID itemID2) override; // @Note: Added in 17134
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
        const long column, REFGUID excludedItemID, POINT* occupiedCell, BOOL* columnUnoccupied) override;
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
    bool m_enableCollapse; // @Note: Added in 15063

    HRESULT _DisplaceItemsFromRect(const Geometry::CRect& targetRect, const Geometry::CRect& previousRect);
    HRESULT _StartBatchingItemBoundsChangeUpdates();
    HRESULT _StopBatchingItemBoundsChangeUpdatesAndNotify();

    static HRESULT s_CreateMigrationHandler(
        const LayoutMigrationOptions& migrationOptions, IItemMigrationHandler** ppMigrationHandler);
};
