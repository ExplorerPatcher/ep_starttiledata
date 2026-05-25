#include "pch.h"

#include "LinearTraversalOrder.h"

EXTERN_C static const inline GUID c_emptyCellValue = {};

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
