#pragma once

#include "ColumnChangeMigrationHandler.h"

class CEmptyColumnCollapseMigrationHandler
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , IItemMigrationHandler
    >
{
public:
    CEmptyColumnCollapseMigrationHandler();

    //~ Begin IItemMigrationHandler Interface
    STDMETHODIMP MigrateItems(
        ICellArrayManager* pSourceCellArrayManager, ICellArrayManager* pDestinationCellArrayManager,
        IItemCellAssignor* pTileCellAssignor) override;
    STDMETHODIMP SetUnassignedItemId(REFGUID unassignedItemId) override;
    //~ End IItemMigrationHandler Interface
};
