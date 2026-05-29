#pragma once

#include "ICellArrayManager.h"

MIDL_INTERFACE("16d714e3-2917-489e-81f1-fbd6b8e72742")
IItemLayoutCollapseHandler : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE SetCellArray(ICellArrayManager* cellArrayManager) = 0;
    virtual HRESULT STDMETHODCALLTYPE Collapse(const Geometry::CRect& sourceCells, const Geometry::CRect& targetCells) = 0;
};
