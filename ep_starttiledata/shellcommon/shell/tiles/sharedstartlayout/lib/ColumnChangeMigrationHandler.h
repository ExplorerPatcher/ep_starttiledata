#pragma once

#include "IItemCellAssignor.h"
#include "IItemMigrationHandler.h"

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
