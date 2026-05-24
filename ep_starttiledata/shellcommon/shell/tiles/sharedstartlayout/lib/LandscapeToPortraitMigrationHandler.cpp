#include "pch.h"

#include "LandscapeToPortraitMigrationHandler.h"

HRESULT CLandscapeToPortraitMigrationHandler::MigrateItems(
    ICellArrayManager* pSourceCellArrayManager, ICellArrayManager* pDestinationCellArrayManager, IItemCellAssignor*)
{
    HRESULT hr = S_OK;

    Geometry::CSize sizeMaxDestinationBounds = pDestinationCellArrayManager->GetMaximumCellArrayDimensions();
    if (sizeMaxDestinationBounds.cx == -1)
    {
        hr = E_FAIL;
    }

    CSet<GUID> setAllItems;
    if (SUCCEEDED(hr))
    {
        hr = pSourceCellArrayManager->GetItemsInRect(pSourceCellArrayManager->GetCurrentCellArrayBounds(), &setAllItems);
        if (SUCCEEDED(hr))
        {
            setAllItems.Enumerate([&](REFGUID key) -> bool
            {
                Geometry::CRect rcTileBounds;
                hr = pSourceCellArrayManager->GetItemBounds(key, rcTileBounds);
                if (SUCCEEDED(hr))
                {
                    // @Note: Guessed based on name from pdb
                    Geometry::CRect rcInvertedBounds(
                        sizeMaxDestinationBounds.cx - rcTileBounds.bottom,
                        rcTileBounds.left,
                        sizeMaxDestinationBounds.cx - rcTileBounds.top,
                        rcTileBounds.right
                    );
                    hr = pDestinationCellArrayManager->SetItem(key, rcInvertedBounds, SIO_SEND_BOUNDS_UPDATE);
                }
                return SUCCEEDED(hr);
            });
        }
    }

    return hr;
}

HRESULT CLandscapeToPortraitMigrationHandler::SetUnassignedItemId(REFGUID unassignedItemId)
{
    return unassignedItemId == GUID_NULL ? S_OK : E_INVALIDARG;
}
