#pragma once

#include "GenericTraversalOrder.h"

class CLinearTraversalOrder : public CGenericTraversalOrder
{
public:
    CLinearTraversalOrder() = default;

    //~ Begin ILayoutTraversalOrder Interface
    STDMETHODIMP GetAdjacent(
        const LayoutNavigationDirection direction, const POINT startingCell, POINT* endingCell, GUID* itemID) override;
    STDMETHODIMP GetFirst(POINT* endingCell, GUID* itemID) override;
    //~ Begin ILayoutTraversalOrder Interface
};
