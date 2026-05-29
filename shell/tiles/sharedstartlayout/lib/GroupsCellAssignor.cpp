#include "pch.h"

#include "GroupsCellAssignor.h"

HRESULT CGroupsCellAssignor::CalculateLocationForNewItem(const Geometry::CSize& sizeItemCells, Geometry::CRect* newBounds)
{
    return _CalculateLocationForNewItem(_spCellArrayManager->GetCurrentCellArrayBounds().top, sizeItemCells, newBounds);
}
