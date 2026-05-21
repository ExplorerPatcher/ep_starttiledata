#pragma once

#include "../../../shell/inc/Geometry.h"
#include "../../../shell/inc/Set.h"

MIDL_INTERFACE("92a95f12-bd8c-4327-93f9-898f0bb01d61")
ICellArray : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE CloneArray(ICellArray**) = 0;
    virtual const GUID STDMETHODCALLTYPE GetCellValue(const int, const int) = 0;
    virtual const Geometry::CRect& STDMETHODCALLTYPE GetArrayBounds() = 0;
    virtual void STDMETHODCALLTYPE GetItemsInRect(const Geometry::CRect&, CSet<GUID>*) = 0;
    virtual void STDMETHODCALLTYPE FixCoordinatesToBeNonNegative() = 0;
    virtual HRESULT STDMETHODCALLTYPE SetItem(REFGUID, const Geometry::CRect&) = 0;
    virtual HRESULT STDMETHODCALLTYPE RemoveItem(REFGUID) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetArrayBounds(const Geometry::CRect&) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddIgnoredItem(REFGUID) = 0;
    virtual HRESULT STDMETHODCALLTYPE RemoveIgnoredItem(REFGUID) = 0;
};
