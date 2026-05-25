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
    int yStart = max(rcMaxBounds.bottom - sizeItemCells.cy, rcMaxBounds.top);
    return _CalculateLocationForNewItem(yStart, sizeItemCells, rcNewBounds);
}

HRESULT CPortraitTileCellAssignor::_CalculateLocationForNewItem(
    int yStart, const Geometry::CSize& sizeItemCells, Geometry::CRect* rcNewBounds)
{
    HRESULT hr = E_FAIL;

    Geometry::CSize sizeMaximumCellArray = _spCellArrayManager->GetMaximumCellArrayDimensions();
    Geometry::CRect rcMaxBounds = _spCellArrayManager->GetCurrentCellArrayBounds();
    for (int nYIndex = yStart; nYIndex < rcMaxBounds.bottom && FAILED(hr); ++nYIndex)
    {
        for (int nXIndex = rcMaxBounds.left; nXIndex < sizeMaximumCellArray.cx - sizeItemCells.cx - rcMaxBounds.left + 1 && FAILED(hr); ++nXIndex)
        {
            Geometry::CRect rcPotentialTileSpot(
                nXIndex, nYIndex,
                nXIndex + sizeItemCells.cx, nYIndex + sizeItemCells.cy);
            if (_spCellArrayManager->IsValidRect(rcPotentialTileSpot)
                && _spCellArrayManager->IsRectEmpty(rcPotentialTileSpot))
            {
                hr = S_OK;
                *rcNewBounds = rcPotentialTileSpot;
            }
        }
    }

    if (FAILED(hr))
    {
        rcNewBounds->left = rcMaxBounds.left;
        rcNewBounds->top = rcMaxBounds.bottom;
        rcNewBounds->right = sizeItemCells.cx;
        rcNewBounds->bottom = rcNewBounds->top + sizeItemCells.cy;
        hr = S_OK;
    }

    return hr;
}
