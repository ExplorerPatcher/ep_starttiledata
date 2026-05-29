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
        const POINT& startingCell, const UINT startingRow, const LayoutNavigationDirection direction, POINT* endingCell,
        GUID* itemID);
    HRESULT GetClosestInGroupInVerticalBounds(
        IItemLayoutResolver* groupResolver, const POINT& groupStartCell, const RECT& verticalBounds,
        const POINT& startingCell, const LayoutNavigationDirection direction, POINT* endingCell, GUID* itemID);
    HRESULT GetClosestInVerticalBounds(
        const POINT& startingCell, const RECT& verticalBounds, REFGUID currentGroup,
        const LayoutNavigationDirection direction, POINT* endingCell, GUID* itemID);
    void ConvertAbsoluteCellToRelativeCell(
        const POINT& groupStartCell, const RECT& margins, const POINT& absoluteCell, POINT* relativeCell);
    HRESULT ConvertAbsoluteVerticalBoundsToRelativeVerticalBounds(
        const POINT& groupStartCell, const Geometry::CRect& groupBounds, const RECT& margins,
        const RECT& absoluteBounds, RECT* relativeBounds);
    HRESULT GetVerticalBounds(
        IItemLayoutResolver* groupResolver, const POINT& groupStartCell, const RECT& margins,
        const POINT& groupRelativeCell, RECT* verticalBounds);
};
