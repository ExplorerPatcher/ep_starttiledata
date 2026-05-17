#pragma once

#include <wrl.h>

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
