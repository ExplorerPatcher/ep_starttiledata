#pragma once

#include "GenericTraversalOrder.h"

class CLinearTraversalOrder : public CGenericTraversalOrder
{
public:
    CLinearTraversalOrder() = default;

    //~ Begin ILayoutTraversalOrder Interface
    HRESULT GetAdjacent(
        const LayoutNavigationDirection direction, const POINT startingCell, POINT* endingCell, GUID* itemID) override;
    HRESULT GetFirst(POINT* endingCell, GUID* itemID) override;
    //~ Begin ILayoutTraversalOrder Interface
};
