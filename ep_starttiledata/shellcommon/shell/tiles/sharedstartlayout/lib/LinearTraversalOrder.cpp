#include "pch.h"

#include "LinearTraversalOrder.h"

EXTERN_C static const inline GUID c_emptyCellValue = {};

HRESULT CLinearTraversalOrder::GetFirst(POINT* endingCell, GUID* itemID)
{
    POINT v10; // x19
    POINT localEndingCell; // [xsp+0h] [xbp-20h] BYREF
    GUID localItemID; // [xsp+8h] [xbp-18h] BYREF
    wil::details::in1diag3* v13; // [xsp+28h] [xbp+8h]

    localItemID = c_emptyCellValue;
    int v7 = this->m_layoutResolver->GetItemByCell(0, &localItemID);
    if (v7 < 0)
    {
        wil::details::in1diag3::Return_Hr(v13, 16u, "shellcommon\\shell\\tiles\\sharedstartlayout\\lib\\lineartraversalorder.cpp", v7);
        return v7;
    }

    localEndingCell = 0;
    HRESULT v9 = 0;
    v10 = 0;
    if (!memcmp_0(&localItemID, &c_emptyCellValue, 0x10u))
    {
        v9 = this->GetAdjacent(LayoutNavigationDirection_Right, 0, &localEndingCell, &localItemID);
        if ((v9 + 0x80000000 & 0x80000000) == 0 && v9 != TYPE_E_ELEMENTNOTFOUND)
        {
            wil::details::in1diag3::Return_Hr(
                v13,
                23u,
                "shellcommon\\shell\\tiles\\sharedstartlayout\\lib\\lineartraversalorder.cpp",
                v9);
            return v9;
        }
        v10 = localEndingCell;
    }

    *itemID = localItemID;
    *endingCell = v10;
    return v9;
}
