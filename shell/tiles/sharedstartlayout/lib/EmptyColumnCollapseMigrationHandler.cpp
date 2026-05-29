#include "pch.h"

#include "EmptyColumnCollapseMigrationHandler.h"

CEmptyColumnCollapseMigrationHandler::CEmptyColumnCollapseMigrationHandler()
{
}

HRESULT CEmptyColumnCollapseMigrationHandler::MigrateItems(
    ICellArrayManager* pSourceCellArrayManager, ICellArrayManager* pDestinationCellArrayManager,
    IItemCellAssignor* pTileCellAssignor)
{
    Geometry::CSize sizeMaxDestinationBounds = pDestinationCellArrayManager->GetMaximumCellArrayDimensions();
    Geometry::CSize sizeMaxSourceBounds = pSourceCellArrayManager->GetMaximumCellArrayDimensions();
    Geometry::CRect sizeCurrentSourceBounds = pSourceCellArrayManager->GetCurrentCellArrayBounds();

    int columnsToCollapse = sizeMaxSourceBounds.cx - sizeMaxDestinationBounds.cx;
    RETURN_HR_IF_EXPECTED(E_INVALIDARG, columnsToCollapse <= 0);

    RETURN_HR_IF(E_INVALIDARG, sizeMaxSourceBounds.cx <= 0); // 17

    CCoSimpleArray<int> amountsToShift;
    amountsToShift.Resize(sizeMaxSourceBounds.cx, 0);

    int numEmptyColumnsFound = 0;
    for (int i = sizeMaxSourceBounds.cx - 1; i >= 0 && numEmptyColumnsFound < columnsToCollapse; --i)
    {
        Geometry::CRect rectForColumn(i, 0, i + 1, sizeCurrentSourceBounds.GetHeight());

        CSet<GUID> itemsInColumn;
        RETURN_IF_FAILED(pSourceCellArrayManager->GetItemsInRect(rectForColumn, &itemsInColumn)); // 29

        if (itemsInColumn.GetCount() == 0)
        {
            for (int j = i + 1; j < sizeMaxSourceBounds.cx; ++j)
            {
                ++amountsToShift[j];
            }
            ++numEmptyColumnsFound;
        }
    }

    RETURN_HR_IF_EXPECTED(E_INVALIDARG, numEmptyColumnsFound != columnsToCollapse);

    for (int i = 0; i < sizeMaxSourceBounds.cx; ++i)
    {
        Geometry::CRect rectForColumn(i, 0, i + 1, sizeCurrentSourceBounds.GetHeight());

        CSet<GUID> itemsInColumn;
        RETURN_IF_FAILED(pSourceCellArrayManager->GetItemsInRect(rectForColumn, &itemsInColumn)); // 52

        HRESULT enumerateResult = S_OK;
        itemsInColumn.Enumerate(
            [i, &enumerateResult, &amountsToShift, &pSourceCellArrayManager, &pDestinationCellArrayManager](REFGUID itemID) -> bool
            {
                Geometry::CRect itemBounds;
                enumerateResult = pSourceCellArrayManager->GetItemBounds(itemID, itemBounds);
                if (SUCCEEDED(enumerateResult) && itemBounds.left == i)
                {
                    Geometry::CRect newBounds(
                        itemBounds.left  - amountsToShift[i], itemBounds.top,
                        itemBounds.right - amountsToShift[i], itemBounds.bottom);
                    enumerateResult = pDestinationCellArrayManager->SetItem(itemID, newBounds, SIO_SEND_BOUNDS_UPDATE);
                }

                return SUCCEEDED(enumerateResult);
            }
        );

        RETURN_IF_FAILED(enumerateResult); // 72
    }

    return S_OK;
}

HRESULT CEmptyColumnCollapseMigrationHandler::SetUnassignedItemId(REFGUID unassignedItemId)
{
    return unassignedItemId == GUID_NULL ? S_OK : E_INVALIDARG;
}
