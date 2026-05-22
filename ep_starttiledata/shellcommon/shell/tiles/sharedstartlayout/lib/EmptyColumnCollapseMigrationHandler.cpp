#include "pch.h"

#include "EmptyColumnCollapseMigrationHandler.h"

CEmptyColumnCollapseMigrationHandler::CEmptyColumnCollapseMigrationHandler()
{
}

HRESULT CEmptyColumnCollapseMigrationHandler::MigrateItems(
    ICellArrayManager* pSourceCellArrayManager, ICellArrayManager* pDestinationCellArrayManager,
    IItemCellAssignor* pTileCellAssignor)
{
    // TODO: Implement this function
}

HRESULT CEmptyColumnCollapseMigrationHandler::SetUnassignedItemId(REFGUID unassignedItemId)
{
    return unassignedItemId == GUID_NULL ? S_OK : E_INVALIDARG;
}
