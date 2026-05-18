#include "pch.h"

#include "ColumnChangeMigrationHandler.h"

HRESULT CColumnChangeMigrationHandler::MigrateItems(
    ICellArrayManager* pSourceCellArrayManager, ICellArrayManager* pDestinationCellArrayManager, IItemCellAssignor* pTileCellAssignor)
{
    Geometry::CSize sizeMaxSourceBounds;
    sizeMaxSourceBounds.cx = pSourceCellArrayManager->GetMaximumCellArrayDimensions().cx;

    Geometry::CSize sizeMaxDestinationBounds;
    sizeMaxDestinationBounds.cx = pDestinationCellArrayManager->GetMaximumCellArrayDimensions().cx;

    if (sizeMaxSourceBounds.cx > sizeMaxDestinationBounds.cx)
    {
        return _ToNarrowerLayout(pSourceCellArrayManager, pDestinationCellArrayManager, pTileCellAssignor);
    }
    return _ToWiderLayout(pSourceCellArrayManager, pDestinationCellArrayManager);
}

HRESULT CColumnChangeMigrationHandler::SetUnassignedItemId(const GUID& unassignedItemId)
{
    if (unassignedItemId == GUID_NULL)
    {
        return S_OK;
    }
    return E_INVALIDARG;
}

HRESULT CColumnChangeMigrationHandler::_ToWiderLayout(
    ICellArrayManager* pSourceCellArrayManager, ICellArrayManager* pDestinationCellArrayManager)
{
    CSet<GUID> setAllItems;
    HRESULT hr = pSourceCellArrayManager->GetItemsInRect(pSourceCellArrayManager->GetCurrentCellArrayBounds(), &setAllItems);
    if (SUCCEEDED(hr))
    {
        (void)setAllItems.Enumerate([&hr, &pSourceCellArrayManager, &pDestinationCellArrayManager](const GUID& key) -> bool
        {
            Geometry::CRect rcTileBounds;
            hr = pSourceCellArrayManager->GetItemBounds(key, rcTileBounds);
            if (SUCCEEDED(hr))
            {
                hr = pDestinationCellArrayManager->SetItem(key, rcTileBounds, SIO_SEND_BOUNDS_UPDATE);
            }
            return SUCCEEDED(hr);
        });
    }

    return hr;
}

HRESULT CColumnChangeMigrationHandler::_ToNarrowerLayout(
    ICellArrayManager* pSourceCellArrayManager, ICellArrayManager* pDestinationCellArrayManager,
    IItemCellAssignor* pTileCellAssignor)
{
    CSet<GUID> setAllItems;
    CSet<GUID> setOverflowedItems;

    HRESULT hr = pSourceCellArrayManager->GetItemsInRect(pSourceCellArrayManager->GetCurrentCellArrayBounds(), &setAllItems);
    if (SUCCEEDED(hr))
    {
        (void)setAllItems.Enumerate([&hr, &pSourceCellArrayManager, &pDestinationCellArrayManager, &setOverflowedItems](const GUID& tileID) -> bool
        {
            Geometry::CRect rcTileBounds;
            hr = pSourceCellArrayManager->GetItemBounds(tileID, rcTileBounds);
            if (SUCCEEDED(hr))
            {
                if (pDestinationCellArrayManager->IsValidRect(rcTileBounds))
                {
                    hr = pDestinationCellArrayManager->SetItem(tileID, rcTileBounds, SIO_SEND_BOUNDS_UPDATE);
                }
                else
                {
                    setOverflowedItems.Add(tileID);
                }
            }
            return SUCCEEDED(hr);
        });
        if (SUCCEEDED(hr))
        {
            (void)setOverflowedItems.Enumerate([&hr, &pSourceCellArrayManager, &pTileCellAssignor, &pDestinationCellArrayManager](const GUID& tileID) -> bool
            {
                if (SUCCEEDED(hr))
                {
                    Geometry::CRect rcTileBounds;
                    Geometry::CRect rcNewTileCellBounds;

                    hr = pSourceCellArrayManager->GetItemBounds(tileID, rcTileBounds);
                    if (SUCCEEDED(hr))
                    {
                        hr = pTileCellAssignor->CalculateLocationForNewItem(rcTileBounds.GetSize(), &rcNewTileCellBounds);
                    }
                    if (SUCCEEDED(hr))
                    {
                        hr = pDestinationCellArrayManager->SetItem(tileID, rcNewTileCellBounds, SIO_SEND_BOUNDS_UPDATE);
                    }
                }
                return SUCCEEDED(hr);
            });
        }
    }

    return hr;
}
