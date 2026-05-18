#pragma once

#include <Windows.h>

#include "../../../shell/inc/Set.h"
#include "../../../shell/inc/Geometry.h"

struct ICellArrayManagerCallback : public IUnknown
{
    virtual void STDMETHODCALLTYPE ItemBoundsUpdated(const GUID&, const Geometry::CRect&) = 0;
    virtual void STDMETHODCALLTYPE CellArrayBoundsUpdated(const Geometry::CRect&) = 0;
    virtual void STDMETHODCALLTYPE ItemRemovedPending(const GUID&) = 0;
    virtual void STDMETHODCALLTYPE ItemRemoved(const GUID&) = 0;
};

enum SET_ITEM_OPTIONS
{
    SIO_NONE = 0x0,
    SIO_SEND_BOUNDS_UPDATE = 0x1,
};

struct ICellArrayManager : IUnknown
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
    virtual HRESULT STDMETHODCALLTYPE GetItemBounds(const GUID&, Geometry::CRect&) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetBoundingRectForItems(const CSet<GUID>&, Geometry::CRect&) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetLayoutBoundsWithoutItem(const GUID&, RECT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE ContainRectInsideFixedArrayBounds(Geometry::CRect&) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetMaximumCellArrayDimensions(const int, const int) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetItem(const GUID&, Geometry::CRect, SET_ITEM_OPTIONS) = 0;
    virtual HRESULT STDMETHODCALLTYPE RemoveItemUncommitted(const GUID&) = 0;
    virtual HRESULT STDMETHODCALLTYPE InsertEmptyColumn(Geometry::CRect, bool) = 0;
    virtual HRESULT STDMETHODCALLTYPE MoveItemUncommitted(const GUID&, Geometry::CRect) = 0;
    virtual HRESULT STDMETHODCALLTYPE InsertItemUncommitted(const GUID&, const Geometry::CRect) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddIgnoredItem(const GUID&) = 0;
    virtual HRESULT STDMETHODCALLTYPE RemoveIgnoredItem(const GUID&) = 0;
    virtual HRESULT STDMETHODCALLTYPE CommitChanges() = 0;
    virtual HRESULT STDMETHODCALLTYPE AbandonChanges() = 0;
    virtual HRESULT STDMETHODCALLTYPE FixCoordinatesToBeNonNegative() = 0;
};
