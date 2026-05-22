#pragma once

#include "ColumnChangeMigrationHandler.h"

class CPortraitTileCellAssignor
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , IItemCellAssignor
    >
{
public:
    //~ Begin IItemCellAssignor Interface
    STDMETHODIMP SetCellArray(ICellArrayManager* cellArrayManager) override;
    STDMETHODIMP CalculateLocationForNewItem(
        const Geometry::CSize& sizeItemCells, Geometry::CRect* rcNewBounds) override;
    //~ End IItemCellAssignor Interface

protected:
    HRESULT _CalculateLocationForNewItem(
        int yStart, const Geometry::CSize& sizeItemCells, Geometry::CRect* rcNewBounds);

    Microsoft::WRL::ComPtr<ICellArrayManager> _spCellArrayManager;
};
