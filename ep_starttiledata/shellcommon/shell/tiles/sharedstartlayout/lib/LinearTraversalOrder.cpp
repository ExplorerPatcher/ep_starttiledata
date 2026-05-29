#include "pch.h"

#include "LinearTraversalOrder.h"

EXTERN_C static const inline GUID c_emptyCellValue = {};

HRESULT CLinearTraversalOrder::GetAdjacent(
    const LayoutNavigationDirection direction, const POINT startingCell, POINT* endingCell, GUID* itemID)
{
    RETURN_HR_IF_EXPECTED(E_INVALIDARG, direction != LayoutNavigationDirection_Left && direction != LayoutNavigationDirection_Right);

    RECT bounds;
    RETURN_IF_FAILED(m_layoutResolver->GetLayoutBounds(&bounds)); // 40

    GUID localID = c_emptyCellValue;
    int delta = direction == LayoutNavigationDirection_Right ? 1 : -1;

    POINT checkCell;
    checkCell.x = startingCell.x + delta;
    checkCell.y = startingCell.y;
    while (checkCell.y < bounds.bottom - bounds.top && checkCell.y >= 0)
    {
        while (checkCell.x < bounds.right - bounds.left && checkCell.x >= 0)
        {
            GUID tempID;
            RETURN_IF_FAILED(m_layoutResolver->GetItemByCell(checkCell, &tempID)); // 53

            if (tempID != c_emptyCellValue)
            {
                RECT itemBounds;
                RETURN_IF_FAILED(m_layoutResolver->GetItemBounds(tempID, &itemBounds)); // 57

                if (itemBounds.left == checkCell.x && itemBounds.top == checkCell.y)
                {
                    localID = tempID;
                    break;
                }
            }

            checkCell.x += delta;
        }

        if (localID == c_emptyCellValue)
        {
            checkCell.x = direction == LayoutNavigationDirection_Right ? 0 : bounds.right - bounds.left - 1;
            checkCell.y += delta;
            continue;
        }

        break;
    }

    *endingCell = checkCell;
    *itemID = localID;

    return localID == c_emptyCellValue ? TYPE_E_ELEMENTNOTFOUND : S_OK;
}

HRESULT CLinearTraversalOrder::GetFirst(POINT* endingCell, GUID* itemID)
{
    GUID localItemID = c_emptyCellValue;
    RETURN_IF_FAILED(m_layoutResolver->GetItemByCell({}, &localItemID)); // 16

    POINT localEndingCell = {};
    HRESULT hr = S_OK;
    if (localItemID == c_emptyCellValue)
        hr = GetAdjacent(LayoutNavigationDirection_Right, {}, &localEndingCell, &localItemID);

    RETURN_HR_IF(hr, FAILED(hr) && hr != TYPE_E_ELEMENTNOTFOUND); // 23
    *itemID = localItemID;
    *endingCell = localEndingCell;
    return hr;
}
