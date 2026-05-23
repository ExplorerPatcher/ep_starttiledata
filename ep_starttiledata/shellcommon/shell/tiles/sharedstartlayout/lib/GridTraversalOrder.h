#pragma once

#include "GenericTraversalOrder.h"

class CGridTraversalOrder : public CGenericTraversalOrder
{
public:
    CGridTraversalOrder() = default;

    //~ Begin ILayoutTraversalOrder Interface
    STDMETHODIMP GetAdjacent(
        const LayoutNavigationDirection direction, const POINT startingCell, POINT* endingCell, GUID* itemID) override;
    STDMETHODIMP GetFirst(POINT* endingCell, GUID* itemID) override;
    //~ End ILayoutTraversalOrder Interface

private:
    HRESULT GetAdjacentHorizontal(
        IItemLayoutResolver* groupResolver, REFGUID currentGroup, const POINT& groupStartCell, const RECT& margins,
        const POINT& groupRelativeCell, const POINT& startingCell, LayoutNavigationDirection direction,
        POINT* endingCell, GUID* itemID);
    HRESULT GetAdjacentUp(
        IItemLayoutResolver* groupResolver, REFGUID currentGroup, const POINT& groupStartCell, const RECT& margins,
        const POINT& groupRelativeCell, const POINT& startingCell, POINT* endingCell, GUID* itemID);
    HRESULT GetAdjacentDown(
        IItemLayoutResolver* groupResolver, REFGUID currentGroup, const POINT& groupStartCell, const RECT& margins,
        const POINT& groupRelativeCell, const POINT& startingCell, POINT* endingCell, GUID* itemID);
    HRESULT GetClosestVerticallyInGroup(
        IItemLayoutResolver* groupResolver, const POINT& groupStartCell, const RECT& margins,
        const POINT& startingCell, const unsigned int startingRow, const LayoutNavigationDirection direction,
        POINT* endingCell, GUID* itemID);
    HRESULT GetClosestInGroupInVerticalBounds(
        IItemLayoutResolver* groupResolver, const POINT& groupStartCell, const RECT& verticalBounds,
        const POINT& startingCell, const LayoutNavigationDirection direction, POINT* endingCell, GUID* itemID);
    HRESULT GetClosestInVerticalBounds(
        const POINT& startingCell, const RECT& verticalBounds, REFGUID currentGroup,
        const LayoutNavigationDirection direction, POINT* endingCell, GUID* itemID);
    void ConvertAbsoluteCellToRelativeCell(const POINT&, const RECT&, const POINT&, POINT*);
    HRESULT ConvertAbsoluteVerticalBoundsToRelativeVerticalBounds(
        const POINT&, const Geometry::CRect&, const RECT&, const RECT&, RECT*);
    HRESULT GetVerticalBounds(
        IItemLayoutResolver* groupResolver, const POINT& groupStartCell, const RECT& margins,
        const POINT& groupRelativeCell, RECT* verticalBounds);
};
