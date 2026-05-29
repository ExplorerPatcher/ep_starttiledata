#pragma once

#include "PortraitTileCellAssignor.h"

class CGroupsCellAssignor : public CPortraitTileCellAssignor
{
public:
    //~ Begin IItemCellAssignor Interface
    STDMETHODIMP CalculateLocationForNewItem(const Geometry::CSize& sizeItemCells, Geometry::CRect* newBounds) override;
    //~ End IItemCellAssignor Interface
};
