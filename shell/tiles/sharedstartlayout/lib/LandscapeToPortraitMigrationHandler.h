#pragma once

#include "IItemCellAssignor.h"
#include "IItemMigrationHandler.h"

class CLandscapeToPortraitMigrationHandler
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
                                          , IItemMigrationHandler
    >
{
public:
    CLandscapeToPortraitMigrationHandler() = default;

    //~ Begin IItemMigrationHandler Interface
    STDMETHODIMP MigrateItems(
        ICellArrayManager* pSourceCellArrayManager, ICellArrayManager* pDestinationCellArrayManager,
        IItemCellAssignor*) override;
    STDMETHODIMP SetUnassignedItemId(REFGUID unassignedItemId) override;
    //~ End IItemMigrationHandler Interface
};
