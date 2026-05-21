#pragma once

#include "ICellArrayManager.h"

struct IItemCellAssignor : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE SetCellArray(ICellArrayManager*) = 0;
    virtual HRESULT STDMETHODCALLTYPE CalculateLocationForNewItem(const Geometry::CSize&, Geometry::CRect*) = 0;
};

struct IItemMigrationHandler : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE MigrateItems(ICellArrayManager*, ICellArrayManager*, IItemCellAssignor*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetUnassignedItemId(REFGUID) = 0;
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
