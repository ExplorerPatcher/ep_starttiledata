#pragma once

#include "ICellArrayManager.h"

MIDL_INTERFACE("7ae8c636-b6f2-48db-a6ef-88bb015c01cb")
IItemCellAssignor : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE SetCellArray(ICellArrayManager* cellArrayManager) = 0;
    virtual HRESULT STDMETHODCALLTYPE CalculateLocationForNewItem(const Geometry::CSize& sizeItemCells, Geometry::CRect* rcNewBounds) = 0;
};

MIDL_INTERFACE("c44d7f05-9680-4583-a2de-090cd1645d1a")
IItemMigrationHandler : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE MigrateItems(ICellArrayManager* pSourceCellArrayManager, ICellArrayManager* pDestinationCellArrayManager, IItemCellAssignor* pTileCellAssignor) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetUnassignedItemId(REFGUID unassignedItemId) = 0;
};

class CColumnChangeMigrationHandler
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , IItemMigrationHandler
    >
{
public:
    CColumnChangeMigrationHandler() = default;

    //~ Begin IItemMigrationHandler Interface
    STDMETHODIMP MigrateItems(
        ICellArrayManager* pSourceCellArrayManager, ICellArrayManager* pDestinationCellArrayManager,
        IItemCellAssignor* pTileCellAssignor) override;
    STDMETHODIMP SetUnassignedItemId(REFGUID unassignedItemId) override;
    //~ End IItemMigrationHandler Interface

private:
    HRESULT _ToWiderLayout(ICellArrayManager* pSourceCellArrayManager, ICellArrayManager* pDestinationCellArrayManager);
    HRESULT _ToNarrowerLayout(
        ICellArrayManager* pSourceCellArrayManager, ICellArrayManager* pDestinationCellArrayManager,
        IItemCellAssignor* pTileCellAssignor);
};
