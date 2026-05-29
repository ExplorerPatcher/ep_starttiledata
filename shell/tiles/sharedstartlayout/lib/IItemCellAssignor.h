#pragma once

#include "ICellArrayManager.h"

MIDL_INTERFACE("7ae8c636-b6f2-48db-a6ef-88bb015c01cb")
IItemCellAssignor : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE SetCellArray(ICellArrayManager* cellArrayManager) = 0;
    virtual HRESULT STDMETHODCALLTYPE CalculateLocationForNewItem(const Geometry::CSize& sizeItemCells, Geometry::CRect* rcNewBounds) = 0;
};