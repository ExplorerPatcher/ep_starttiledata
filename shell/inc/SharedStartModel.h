#pragma once

#include <shtypes.h>

interface IItemLayoutResolver;

MIDL_INTERFACE("751657ce-45b3-4150-8f86-fda773983ebc")
IItemLayoutResolverCallback : IUnknown
{
    virtual void STDMETHODCALLTYPE ItemBoundsUpdated(const GUID* rgItemIDs, const RECT* rgItemBoundsCells, const UINT uItemCount) = 0;
    virtual void STDMETHODCALLTYPE LayoutBoundsUpdated(const RECT layoutBounds) = 0;
    virtual void STDMETHODCALLTYPE ItemRemovedPending(REFGUID itemID) = 0;
    virtual void STDMETHODCALLTYPE ItemRemoved(REFGUID itemID) = 0;
    virtual void STDMETHODCALLTYPE LastItemRemovedPending() = 0;
    virtual void STDMETHODCALLTYPE LastItemRemoved() = 0;
};

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

// 14361 - 16299        : b86bf953-828b-4f3d-987b-04a5601b23cf
// 17134+               : dd5c8f3a-3ca5-4964-a162-192c9b8fd2bc
MIDL_INTERFACE("dd5c8f3a-3ca5-4964-a162-192c9b8fd2bc")
IItemLayoutResolver : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE RegisterCallback(IItemLayoutResolverCallback* callback) = 0;
    virtual HRESULT STDMETHODCALLTYPE UnregisterCallback(IItemLayoutResolverCallback* callback) = 0;
    virtual HRESULT STDMETHODCALLTYPE RegisterInternalCallback(IItemLayoutResolverInternalCallback* callback) = 0;
    virtual HRESULT STDMETHODCALLTYPE UnregisterInternalCallback(IItemLayoutResolverInternalCallback* callback) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddNewItem(REFGUID itemID, const SIZE sizeItemCells) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddItem(REFGUID itemID, const RECT rcItemBoundsCells) = 0;
    virtual HRESULT STDMETHODCALLTYPE InsertItemUncommitted(REFGUID itemID, const RECT rcDestination) = 0;
    virtual HRESULT STDMETHODCALLTYPE ResizeItemUncommitted(REFGUID itemID, const SIZE sizeItemCells) = 0;
    virtual HRESULT STDMETHODCALLTYPE SwapItemsUncommitted(REFGUID itemID1, REFGUID itemID2) = 0; // @Note: Added in 17134
    virtual HRESULT STDMETHODCALLTYPE AddNewContainer(REFGUID containerID, IItemLayoutResolver* resolver) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddContainer(REFGUID containerID, IItemLayoutResolver* resolver, const POINT destination) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddSizedContainer(REFGUID containerID, IItemLayoutResolver* pResolver, const RECT destination) = 0;
    virtual HRESULT STDMETHODCALLTYPE InsertContainerUncommitted(REFGUID containerID, IItemLayoutResolver* resolver, const POINT destination) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetLayoutResolverForContainer(REFGUID containerID, IItemLayoutResolver** ppResolver) = 0;
    virtual BOOL STDMETHODCALLTYPE IsCollapsed() = 0;
    virtual HRESULT STDMETHODCALLTYPE Collapse() = 0;
    virtual HRESULT STDMETHODCALLTYPE Expand() = 0;
    virtual HRESULT STDMETHODCALLTYPE RemoveItemUncommitted(REFGUID itemID) = 0;
    virtual HRESULT STDMETHODCALLTYPE MoveItemUncommitted(REFGUID itemID, const POINT destination) = 0;
    virtual BOOL STDMETHODCALLTYPE IsEmpty() = 0;
    virtual HRESULT STDMETHODCALLTYPE GetItemByCell(const POINT ptCell, GUID* pItemID) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetLayoutBounds(RECT* prcBounds) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetLayoutBoundsWithoutItem(REFGUID itemID, RECT* bounds) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetItemBounds(REFGUID itemID, RECT* itemBounds) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetContainerSizeWithMargins(REFGUID containerID, SIZE* size) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetLastOccupiedCellInColumn(const long column, REFGUID excludedItemID, POINT* occupiedCell, BOOL* columnUnoccupied) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetContainerMargins(const RECT containerMargins) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetContainerMargins(RECT* containerMargins) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetMaxCellBounds(const int nMaxXBounds, const int nMaxYBounds) = 0;
    virtual SIZE STDMETHODCALLTYPE GetMaxCellBounds() = 0;
    virtual HRESULT STDMETHODCALLTYPE MigrateItems(IItemLayoutResolver* pSourceLayout, const LayoutMigrationOptions migrationOptions) = 0;
    virtual HRESULT STDMETHODCALLTYPE CommitChanges() = 0;
    virtual HRESULT STDMETHODCALLTYPE AbandonChanges() = 0;
    virtual HRESULT STDMETHODCALLTYPE RepairLayoutUncommitted() = 0;
};

MIDL_INTERFACE("aa22a8ff-a704-4ea8-82b5-3c51d9dd3b3c")
ILayoutHitTest : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetGutterHitTarget(REFGUID, const RECT, POINT*) = 0;
};

enum LayoutNavigationDirection
{
    LayoutNavigationDirection_Up = 0x0,
    LayoutNavigationDirection_Down = 0x1,
    LayoutNavigationDirection_Left = 0x2,
    LayoutNavigationDirection_Right = 0x3,
};

MIDL_INTERFACE("0999ed40-ced2-4849-b435-e8cfc9b8752c")
ILayoutTraversalOrder : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetAdjacent(const LayoutNavigationDirection direction, const POINT startingCell, POINT* endingCell, GUID* itemID) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFirst(POINT* endingCell, GUID* itemID) = 0;
};

MIDL_INTERFACE("4839e2d8-ff75-4643-9bc6-435ae7c12501")
ITileGridMetricsCalculator : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetGroupWidthOptions(ULONG* groupWidthOptions, ULONG groupWidthOptionsBufferSize, ULONG* groupWidthOptionsWritten, ULONG* defaultGroupWidthOption, ULONG* numGroups, ULONG* numGroupsRotated) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetTileSize(const DEVICE_SCALE_FACTOR scaleFactor, const ULONG groupWidth, float* tileSize) = 0;
};

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
    virtual HRESULT STDMETHODCALLTYPE CreatePortraitLayoutResolver(IItemLayoutResolver** ppLayoutResolver) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateDesktopPortraitLayoutResolver(IItemLayoutResolver** ppLayoutResolver) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateGroupsLayoutResolver(IItemLayoutResolver** ppLayoutResolver) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateLayoutTraversalOrder(IItemLayoutResolver* layoutResolver, int maxGroupWidth, LayoutOrder order, ILayoutTraversalOrder** ppLayoutTraversalOrder) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateTileGridMetricsCalculator(const TileSizingMode tileSizingMode, const float displayWidth, const float displayHeight, const float screenDiagonal, ITileGridMetricsCalculator** tileSizeCalculator) = 0;
};

class DECLSPEC_UUID("7bd7ab1c-f2c5-60c2-8d00-c2e50336a954")
StartLayoutFactory;

namespace SharedStartModel
{
    inline const SIZE c_size1x1 = { 1, 1 };
    inline const SIZE c_size2x2 = { 2, 2 };
    inline const SIZE c_size2x4 = { 2, 4 };
    inline const SIZE c_size4x2 = { 4, 2 };
    inline const SIZE c_size4x4 = { 4, 4 };
}
