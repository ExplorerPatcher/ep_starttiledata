#pragma once

#include <Windows.h>

#include "../../../shell/inc/Set.h"
#include "../../../shell/inc/Geometry.h"

MIDL_INTERFACE("da5ab758-b158-4490-963c-87916bd8049a")
ICellArrayManagerCallback : IUnknown
{
    virtual void STDMETHODCALLTYPE ItemBoundsUpdated(REFGUID, const Geometry::CRect&) = 0;
    virtual void STDMETHODCALLTYPE CellArrayBoundsUpdated(const Geometry::CRect&) = 0;
    virtual void STDMETHODCALLTYPE ItemRemovedPending(REFGUID) = 0;
    virtual void STDMETHODCALLTYPE ItemRemoved(REFGUID) = 0;
};

enum SET_ITEM_OPTIONS
{
    SIO_NONE = 0x0,
    SIO_SEND_BOUNDS_UPDATE = 0x1,
};

MIDL_INTERFACE("dd5c8f3a-3ca5-4964-a162-192c9b8fd2bc") // @Note: Same GUID as IItemLayoutResolver
ICellArrayManager : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE RegisterCallback(ICellArrayManagerCallback*) = 0;
    virtual HRESULT STDMETHODCALLTYPE UnregisterCallback(ICellArrayManagerCallback*) = 0;
    virtual const Geometry::CRect& STDMETHODCALLTYPE GetCurrentCellArrayBounds() = 0;
    virtual const Geometry::CSize& STDMETHODCALLTYPE GetMaximumCellArrayDimensions() = 0;
    virtual bool STDMETHODCALLTYPE IsEmpty() = 0;
    virtual bool STDMETHODCALLTYPE IsValidRect(const Geometry::CRect&) = 0;
    virtual bool STDMETHODCALLTYPE IsValidCellCoordinate(const Geometry::CPoint&) = 0;
    virtual const GUID STDMETHODCALLTYPE GetItemAtCell(const int, const int) = 0;
    virtual bool STDMETHODCALLTYPE IsRectEmpty(const Geometry::CRect) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetItemsInRect(const Geometry::CRect, CSet<GUID>*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetItemsOutsideOfRect(const Geometry::CRect, CSet<GUID>*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetItemBounds(REFGUID, Geometry::CRect&) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetBoundingRectForItems(const CSet<GUID>&, Geometry::CRect&) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetLayoutBoundsWithoutItem(REFGUID, RECT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE ContainRectInsideFixedArrayBounds(Geometry::CRect&) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetMaximumCellArrayDimensions(const int, const int) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetItem(REFGUID, Geometry::CRect, SET_ITEM_OPTIONS) = 0;
    virtual HRESULT STDMETHODCALLTYPE RemoveItemUncommitted(REFGUID) = 0;
    virtual HRESULT STDMETHODCALLTYPE InsertEmptyColumn(Geometry::CRect, bool) = 0;
    virtual HRESULT STDMETHODCALLTYPE MoveItemUncommitted(REFGUID, Geometry::CRect) = 0;
    virtual HRESULT STDMETHODCALLTYPE InsertItemUncommitted(REFGUID, const Geometry::CRect) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddIgnoredItem(REFGUID) = 0;
    virtual HRESULT STDMETHODCALLTYPE RemoveIgnoredItem(REFGUID) = 0;
    virtual HRESULT STDMETHODCALLTYPE CommitChanges() = 0;
    virtual HRESULT STDMETHODCALLTYPE AbandonChanges() = 0;
    virtual HRESULT STDMETHODCALLTYPE FixCoordinatesToBeNonNegative() = 0;
};
