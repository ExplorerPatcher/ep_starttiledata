#pragma once

#include "ICellArrayManager.h"

MIDL_INTERFACE("c44d7f05-9680-4583-a2de-090cd1645d1a")
IItemMigrationHandler : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE MigrateItems(ICellArrayManager* pSourceCellArrayManager, ICellArrayManager* pDestinationCellArrayManager, IItemCellAssignor* pTileCellAssignor) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetUnassignedItemId(REFGUID unassignedItemId) = 0;
};
