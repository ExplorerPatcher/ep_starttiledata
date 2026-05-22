#include "pch.h"

#include "PortraitTileCellAssignor.h"

HRESULT CPortraitTileCellAssignor::SetCellArray(ICellArrayManager* cellArrayManager)
{
    _spCellArrayManager = cellArrayManager;
    return S_OK;
}

HRESULT CPortraitTileCellAssignor::CalculateLocationForNewItem(
    const Geometry::CSize& sizeItemCells, Geometry::CRect* rcNewBounds)
{
    Geometry::CRect rcMaxBounds = _spCellArrayManager->GetCurrentCellArrayBounds();
    int yStart;
    if (rcMaxBounds.bottom - sizeItemCells.cy > rcMaxBounds.top)
    {
        yStart = rcMaxBounds.bottom - sizeItemCells.cy;
    }
    else
    {
        yStart = rcMaxBounds.top;
    }
    return _CalculateLocationForNewItem(yStart, sizeItemCells, rcNewBounds);
}

HRESULT CPortraitTileCellAssignor::_CalculateLocationForNewItem(int yStart, const Geometry::CSize& sizeItemCells, Geometry::CRect* rcNewBounds)
{
    static_assert(false, "Please implement me first!");
    return S_OK;
}
