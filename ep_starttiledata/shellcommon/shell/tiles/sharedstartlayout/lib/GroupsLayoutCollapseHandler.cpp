#include "pch.h"

#include "GroupsLayoutCollapseHandler.h"

EXTERN_C static const inline GUID c_emptyCellValue = {};

CGroupsLayoutCollapseHandler::CGroupsLayoutCollapseHandler()
    : m_expandCollapseDirection(EXPAND_COLLAPSE_DIRECTION_ROW)
{
}

HRESULT CGroupsLayoutCollapseHandler::RuntimeClassInitialize(EXPAND_COLLAPSE_DIRECTION expandCollapseDirection)
{
    m_expandCollapseDirection = expandCollapseDirection;
    return S_OK;
}

HRESULT CGroupsLayoutCollapseHandler::SetCellArray(ICellArrayManager* cellArrayManager)
{
    m_cellArrayManager = cellArrayManager;
    return S_OK;
}

HRESULT CGroupsLayoutCollapseHandler::Collapse(const Geometry::CRect&, const Geometry::CRect&)
{
    HRESULT hr = S_OK;

    Geometry::CSize sizeCurrentArray = m_cellArrayManager->GetCurrentCellArrayBounds().GetSize();

    CSimpleHashTable<GUID, Geometry::CRect> itemsToCompact;
    CCoSimpleArray<GUID> compactedItems;

    for (int column = 0; SUCCEEDED(hr) && column < sizeCurrentArray.cx; ++column)
    {
        int delta = 0;
        for (int row = 0; SUCCEEDED(hr) && row < sizeCurrentArray.cy; ++row)
        {
            GUID itemID = m_cellArrayManager->GetItemAtCell(column, row);
            if (itemID == c_emptyCellValue)
            {
                ++delta;
            }
            else if (delta > 0 && FAILED(itemsToCompact.ContainsKey(itemID)))
            {
                Geometry::CRect itemBounds;
                if (SUCCEEDED(m_cellArrayManager->GetItemBounds(itemID, itemBounds)))
                {
                    Geometry::CRect newBounds(
                        itemBounds.left,  itemBounds.top - delta,
                        itemBounds.right, itemBounds.bottom - delta);
                    hr = itemsToCompact.AddItem(itemID, newBounds);
                    if (SUCCEEDED(hr))
                    {
                        hr = compactedItems.Add(itemID);
                    }
                }
            }
        }
    }

    for (size_t index = 0; SUCCEEDED(hr) && index < compactedItems.GetSize(); ++index)
    {
        GUID itemID = c_emptyCellValue;
        hr = compactedItems.GetAt(index, itemID);
        if (SUCCEEDED(hr))
        {
            Geometry::CRect itemBounds;
            hr = itemsToCompact.GetItem(itemID, itemBounds);
            if (SUCCEEDED(hr))
            {
                hr = m_cellArrayManager->MoveItemUncommitted(itemID, itemBounds);
            }
        }
    }

    return hr;
}
