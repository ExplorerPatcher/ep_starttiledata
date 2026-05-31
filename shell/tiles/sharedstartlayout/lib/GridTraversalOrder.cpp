#include "pch.h"

#include "GridTraversalOrder.h"

using namespace Microsoft::WRL;

EXTERN_C static const inline GUID c_emptyCellValue = {};

HRESULT CGridTraversalOrder::GetAdjacent(
    const LayoutNavigationDirection direction, const POINT startingCell, POINT* endingCell, GUID* itemID)
{
    GUID currentGroup;
    ComPtr<IItemLayoutResolver> groupResolver;
    POINT groupRelativeCell;
    POINT groupStartCell;
    RECT margins;
    if (m_maxGroupWidth != -1)
    {
        RETURN_IF_FAILED(GetCurrentItemGroup(startingCell, &currentGroup, &groupResolver, &groupRelativeCell)); // 53
        RETURN_IF_FAILED(GetActualGroupStartCell(currentGroup, &groupStartCell)); // 54
        RETURN_IF_FAILED(m_layoutResolver->GetContainerMargins(&margins)); // 55
    }
    else
    {
        currentGroup = {};
        groupResolver = m_layoutResolver;
        groupRelativeCell = startingCell;
        groupStartCell = {};
        margins = {};
    }

    POINT localEndingCell;
    localEndingCell.x = -1;
    localEndingCell.y = -1;
    GUID localItemID = c_emptyCellValue;
    HRESULT hr;
    if (direction == LayoutNavigationDirection_Up)
    {
        hr = GetAdjacentUp(
            groupResolver.Get(), currentGroup, groupStartCell, margins, groupRelativeCell, startingCell, &localEndingCell, &localItemID);
    }
    else if (direction == LayoutNavigationDirection_Down)
    {
        hr = GetAdjacentDown(
            groupResolver.Get(), currentGroup, groupStartCell, margins, groupRelativeCell, startingCell, &localEndingCell, &localItemID);
    }
    else if (direction == LayoutNavigationDirection_Left || direction == LayoutNavigationDirection_Right)
    {
        hr = GetAdjacentHorizontal(
            groupResolver.Get(), currentGroup, groupStartCell, margins, groupRelativeCell, startingCell, direction, &localEndingCell, &localItemID);
    }
    else
    {
        return E_INVALIDARG;
    }

    RETURN_HR_IF(hr, FAILED(hr) && hr != TYPE_E_ELEMENTNOTFOUND); // 77

    *endingCell = localEndingCell;
    *itemID = localItemID;

    return hr;
}

HRESULT CGridTraversalOrder::GetFirst(POINT* endingCell, GUID* itemID)
{
    POINT testCell = { 0, 0 };
    Geometry::CRect resolverBounds;
    GUID localItemID;

    RETURN_IF_FAILED(m_layoutResolver->GetLayoutBounds(&resolverBounds)); // 16
    RETURN_IF_FAILED(m_layoutResolver->GetItemByCell(testCell, &localItemID)); // 17

    while (localItemID == c_emptyCellValue && testCell.x < resolverBounds.GetWidth())
    {
        ++testCell.x;
        RETURN_IF_FAILED(m_layoutResolver->GetItemByCell(testCell, &localItemID)); // 23
    }

    if (localItemID != c_emptyCellValue)
    {
        GetActualGroupStartCell(localItemID, endingCell);
    }

    *itemID = localItemID;
    if (localItemID == c_emptyCellValue)
    {
        return TYPE_E_ELEMENTNOTFOUND;
    }

    return S_OK;
}

HRESULT CGridTraversalOrder::GetAdjacentHorizontal(
    IItemLayoutResolver* groupResolver, REFGUID currentGroup, const POINT& groupStartCell, const RECT& margins,
    const POINT& groupRelativeCell, const POINT& startingCell, LayoutNavigationDirection direction,
    POINT* endingCell, GUID* itemID)
{
    POINT localEndingCell = {};
    GUID localItemID = c_emptyCellValue;

    RECT verticalBounds;
    RETURN_IF_FAILED(GetVerticalBounds(groupResolver, groupStartCell, margins, groupRelativeCell, &verticalBounds)); // 98

    if (groupRelativeCell.y >= margins.top)
    {
        RETURN_IF_FAILED(GetClosestInGroupInVerticalBounds(groupResolver, groupStartCell, verticalBounds, startingCell, direction, &localEndingCell, &localItemID)); // 102
    }

    if (localItemID == c_emptyCellValue)
    {
        HRESULT hr = GetClosestInVerticalBounds(startingCell, verticalBounds, currentGroup, direction, &localEndingCell, &localItemID);
        RETURN_HR_IF(hr, FAILED(hr) && hr != TYPE_E_ELEMENTNOTFOUND); // 110
    }

    RETURN_HR_IF_EXPECTED(TYPE_E_ELEMENTNOTFOUND, localItemID == c_emptyCellValue);

    *endingCell = localEndingCell;
    *itemID = localItemID;

    return S_OK;
}

HRESULT CGridTraversalOrder::GetAdjacentUp(
    IItemLayoutResolver* groupResolver, REFGUID currentGroup, const POINT& groupStartCell, const RECT& margins,
    const POINT& groupRelativeCell, const POINT& startingCell, POINT* endingCell, GUID* itemID)
{
    POINT localEndingCell = {};
    GUID localItemID = c_emptyCellValue;

    POINT relativeCellWithoutMargins;
    relativeCellWithoutMargins.x = groupRelativeCell.x - margins.left;
    relativeCellWithoutMargins.y = groupRelativeCell.y - margins.top;

    if (groupRelativeCell.y < margins.top)
    {
        RETURN_HR_IF_EXPECTED(TYPE_E_ELEMENTNOTFOUND, startingCell.y == groupRelativeCell.y);

        RECT groupBounds;
        RETURN_IF_FAILED(m_layoutResolver->GetItemBounds(currentGroup, &groupBounds)); // 142

        GUID higherGroupID;
        RETURN_IF_FAILED(m_layoutResolver->GetItemByCell({ groupBounds.left, groupBounds.top - 1 }, &higherGroupID)); // 145

        ComPtr<IItemLayoutResolver> higherGroupResolver;
        RETURN_IF_FAILED(m_layoutResolver->GetLayoutResolverForContainer(higherGroupID, &higherGroupResolver)); // 148

        RECT internalBounds;
        RETURN_IF_FAILED(higherGroupResolver->GetLayoutBounds(&internalBounds)); // 151

        POINT higherGroupStartCell;
        RETURN_IF_FAILED(GetActualGroupStartCell(higherGroupID, &higherGroupStartCell)); // 154

        RETURN_IF_FAILED(GetClosestVerticallyInGroup(
            higherGroupResolver.Get(), higherGroupStartCell, margins, relativeCellWithoutMargins,
            internalBounds.bottom - 1, LayoutNavigationDirection_Up, &localEndingCell, &localItemID)); // 156

        if (localItemID == c_emptyCellValue)
        {
            localItemID = higherGroupID;
            localEndingCell.x = startingCell.x;
            localEndingCell.y = groupBounds.top - 1;
        }
    }
    else
    {
        RECT gridBounds;
        bool hasMaxWidth = m_maxGroupWidth != -1; // Assumed variable to remove gotos
        if (!hasMaxWidth)
        {
            RETURN_IF_FAILED(m_layoutResolver->GetLayoutBounds(&gridBounds)); // 171
        }

        if (hasMaxWidth || groupRelativeCell.y < gridBounds.bottom)
        {
            GUID currentItemID;
            RETURN_IF_FAILED(groupResolver->GetItemByCell(relativeCellWithoutMargins, &currentItemID)); // 186
            RECT itemBounds;
            RETURN_IF_FAILED(groupResolver->GetItemBounds(currentItemID, &itemBounds)); // 188

            gridBounds.bottom = itemBounds.top;
            RETURN_HR_IF_EXPECTED(TYPE_E_ELEMENTNOTFOUND, m_maxGroupWidth == -1 && itemBounds.top <= 0);
        }

        RETURN_IF_FAILED(GetClosestVerticallyInGroup(
            groupResolver, groupStartCell, margins, relativeCellWithoutMargins, gridBounds.bottom - 1,
            LayoutNavigationDirection_Up, &localEndingCell, &localItemID)); // 199

        if (localItemID == c_emptyCellValue)
        {
            RETURN_HR_IF_EXPECTED(TYPE_E_ELEMENTNOTFOUND, m_maxGroupWidth == -1);
            localItemID = currentGroup;
            localEndingCell.x = startingCell.x;
            localEndingCell.y = startingCell.y - groupRelativeCell.y;
        }
    }

    *endingCell = localEndingCell;
    *itemID = localItemID;

    return S_OK;
}

HRESULT CGridTraversalOrder::GetAdjacentDown(
    IItemLayoutResolver* groupResolver, REFGUID currentGroup, const POINT& groupStartCell, const RECT& margins,
    const POINT& groupRelativeCell, const POINT& startingCell, POINT* endingCell, GUID* itemID)
{
    POINT localEndingCell = {};
    GUID localItemID = c_emptyCellValue;

    POINT relativeCellWithoutMargins;
    relativeCellWithoutMargins.x = groupRelativeCell.x - margins.left;
    relativeCellWithoutMargins.y = groupRelativeCell.y - margins.top;

    if (groupRelativeCell.y < margins.top)
    {
        RETURN_IF_FAILED(GetClosestVerticallyInGroup(
            groupResolver, groupStartCell, margins, relativeCellWithoutMargins, 0, LayoutNavigationDirection_Down,
            &localEndingCell, &localItemID)); // 239
    }
    else
    {
        GUID currentItemID;
        RETURN_IF_FAILED(groupResolver->GetItemByCell(relativeCellWithoutMargins, &currentItemID)); // 245
        RECT itemBounds;
        RETURN_IF_FAILED(groupResolver->GetItemBounds(currentItemID, &itemBounds)); // 247
        RETURN_IF_FAILED(GetClosestVerticallyInGroup(
            groupResolver, groupStartCell, margins, relativeCellWithoutMargins, itemBounds.bottom,
            LayoutNavigationDirection_Down, &localEndingCell, &localItemID)); // 248
    }

    if (localItemID == c_emptyCellValue)
    {
        RETURN_HR_IF_EXPECTED(TYPE_E_ELEMENTNOTFOUND, m_maxGroupWidth == -1);

        RECT groupBounds;
        RETURN_IF_FAILED(m_layoutResolver->GetItemBounds(currentGroup, &groupBounds)); // 261

        RECT gridBounds;
        RETURN_IF_FAILED(m_layoutResolver->GetLayoutBounds(&gridBounds)); // 264

        RETURN_HR_IF_EXPECTED(TYPE_E_ELEMENTNOTFOUND, groupBounds.bottom == gridBounds.bottom);

        POINT nextGroupStartCell =
        {
            groupBounds.left,
            groupBounds.bottom
        };
        RETURN_IF_FAILED(m_layoutResolver->GetItemByCell(nextGroupStartCell, &localItemID)); // 273

        if (localItemID != c_emptyCellValue)
        {
            localEndingCell = { startingCell.x, groupBounds.bottom };
        }
        else
        {
            return TYPE_E_ELEMENTNOTFOUND;
        }
    }

    *endingCell = localEndingCell;
    *itemID = localItemID;

    return S_OK;
}

HRESULT CGridTraversalOrder::GetClosestVerticallyInGroup(
    IItemLayoutResolver* groupResolver, const POINT& groupStartCell, const RECT& margins, const POINT& startingCell,
    const UINT startingRow, const LayoutNavigationDirection direction, POINT* endingCell, GUID* itemID)
{
    int delta = direction == LayoutNavigationDirection_Down ? 1 : -1;

    Geometry::CRect groupBounds;
    RETURN_IF_FAILED(groupResolver->GetLayoutBounds(&groupBounds)); // 305

    POINT checkStartCell;
    checkStartCell.x = direction == LayoutNavigationDirection_Down ? groupBounds.GetWidth() - 1 : 0;
    checkStartCell.y = static_cast<int>(startingRow);

    POINT closestCell = {};
    GUID closestItemID = c_emptyCellValue;

    POINT checkCell;
    for (checkCell.y = checkStartCell.y; checkCell.y >= 0 && checkCell.y < groupBounds.GetHeight(); checkCell.y += delta)
    {
        for (checkCell.x = checkStartCell.x; checkCell.x >= 0 && checkCell.x < groupBounds.GetWidth(); checkCell.x -= delta)
        {
            GUID foundItemID;
            RETURN_IF_FAILED(groupResolver->GetItemByCell(checkCell, &foundItemID)); // 322
            if (foundItemID != c_emptyCellValue)
            {
                RECT bounds;
                RETURN_IF_FAILED(groupResolver->GetItemBounds(foundItemID, &bounds)); // 326

                if (checkCell.y == (direction == LayoutNavigationDirection_Down ? bounds.top : bounds.bottom - 1))
                {
                    if (closestItemID == c_emptyCellValue || abs(startingCell.x - checkCell.x) < abs(startingCell.x - closestCell.x))
                    {
                        closestItemID = foundItemID;
                        closestCell = checkCell;
                    }
                }
            }
        }

        if (closestItemID != c_emptyCellValue)
        {
            break;
        }
    }

    *endingCell = { groupStartCell.x + closestCell.x + margins.left, groupStartCell.y + closestCell.y + margins.top };
    *itemID = closestItemID;

    return S_OK;
}

HRESULT CGridTraversalOrder::GetClosestInGroupInVerticalBounds(
    IItemLayoutResolver* groupResolver, const POINT& groupStartCell, const RECT& verticalBounds,
    const POINT& startingCell, const LayoutNavigationDirection direction, POINT* endingCell, GUID* itemID)
{
    RECT tempgroupBounds;
    RETURN_IF_FAILED(groupResolver->GetLayoutBounds(&tempgroupBounds)); // 363
    Geometry::CRect groupBounds = tempgroupBounds;

    RECT margins;
    if (m_maxGroupWidth == -1)
    {
        margins = {};
    }
    else
    {
        RETURN_IF_FAILED(m_layoutResolver->GetContainerMargins(&margins)); // 372
    }

    RECT localRelativeBounds;
    RETURN_IF_FAILED(ConvertAbsoluteVerticalBoundsToRelativeVerticalBounds(
        groupStartCell, groupBounds, margins, verticalBounds, &localRelativeBounds)); // Always S_OK

    int delta = direction == LayoutNavigationDirection_Right ? 1 : -1;

    POINT relativeStartingCell;
    ConvertAbsoluteCellToRelativeCell(groupStartCell, margins, startingCell, &relativeStartingCell);

    POINT checkStartCell;
    if (relativeStartingCell.x >= 0 && relativeStartingCell.x < groupBounds.GetWidth())
    {
        if (direction == LayoutNavigationDirection_Right)
        {
            checkStartCell.x = relativeStartingCell.x + delta;
            checkStartCell.y = localRelativeBounds.bottom - 1;
        }
        else
        {
            checkStartCell.x = relativeStartingCell.x + delta;
            checkStartCell.y = localRelativeBounds.top;
        }
    }
    else
    {
        if (direction == LayoutNavigationDirection_Right)
        {
            checkStartCell.x = 0;
            checkStartCell.y = localRelativeBounds.bottom - 1;
        }
        else
        {
            checkStartCell.x = groupBounds.GetWidth() - 1;
            checkStartCell.y = localRelativeBounds.top;
        }
    }

    POINT closestCell = {};
    GUID closestItemID = c_emptyCellValue;
    GUID foundItemID = c_emptyCellValue;

    POINT checkCell;
    for (checkCell.x = checkStartCell.x; checkCell.x >= 0 && checkCell.x < groupBounds.GetWidth(); checkCell.x += delta)
    {
        for (checkCell.y = checkStartCell.y; checkCell.y >= localRelativeBounds.top && checkCell.y < localRelativeBounds.bottom; checkCell.y -= delta)
        {
            RETURN_IF_FAILED(groupResolver->GetItemByCell(checkCell, &foundItemID)); // 407

            if (foundItemID != c_emptyCellValue)
            {
                RECT bounds;
                RETURN_IF_FAILED(groupResolver->GetItemBounds(foundItemID, &bounds)); // 413

                if (checkCell.x == (direction == LayoutNavigationDirection_Right ? bounds.left : bounds.right - 1))
                {
                    if (closestItemID == c_emptyCellValue || abs(relativeStartingCell.y - checkCell.y) < abs(relativeStartingCell.y - closestCell.y))
                    {
                        closestItemID = foundItemID;
                        closestCell = checkCell;
                    }
                }
            }
        }

        if (closestItemID != c_emptyCellValue)
        {
            break;
        }
    }

    *endingCell = { groupStartCell.x + closestCell.x + margins.left, groupStartCell.y + closestCell.y + margins.top };
    *itemID = closestItemID;

    return S_OK;
}

HRESULT CGridTraversalOrder::GetClosestInVerticalBounds(
    const POINT& startingCell, const RECT& verticalBounds, REFGUID currentGroup,
    const LayoutNavigationDirection direction, POINT* endingCell, GUID* itemID)
{
    Geometry::CRect groupBounds;
    RETURN_IF_FAILED(m_layoutResolver->GetItemBounds(currentGroup, &groupBounds)); // 450

    Geometry::CRect gridBounds;
    RETURN_IF_FAILED(m_layoutResolver->GetLayoutBounds(&gridBounds)); // 453

    RECT margins;
    RETURN_IF_FAILED(m_layoutResolver->GetContainerMargins(&margins)); // 456

    int delta = direction == LayoutNavigationDirection_Right ? 1 : -1;

    POINT checkStartCell;
    if (direction == LayoutNavigationDirection_Right)
    {
        checkStartCell.x = groupBounds.right;
        checkStartCell.y = verticalBounds.bottom - 1;
    }
    else
    {
        checkStartCell.x = groupBounds.left - 1;
        checkStartCell.y = verticalBounds.top;
    }
    RETURN_HR_IF_EXPECTED(TYPE_E_ELEMENTNOTFOUND, checkStartCell.x < 0 || checkStartCell.x >= gridBounds.right);

    POINT foundCell = {};
    GUID closestItemID = c_emptyCellValue;
    POINT closestCell = {};
    GUID lastGroupChecked = c_emptyCellValue;

    POINT checkCell;
    for (checkCell.x = checkStartCell.x; checkCell.x >= 0 && checkCell.x < gridBounds.right; checkCell.x += delta)
    {
        GUID topGroupID;
        RETURN_IF_FAILED(m_layoutResolver->GetItemByCell({ checkCell.x, 0 }, &topGroupID)); // 482

        if (topGroupID == c_emptyCellValue)
        {
            continue;
        }

        for (checkCell.y = checkStartCell.y; checkCell.y >= verticalBounds.top && checkCell.y < verticalBounds.bottom; checkCell.y -= delta)
        {
            GUID foundItemID = c_emptyCellValue;

            GUID groupID;
            RETURN_IF_FAILED(m_layoutResolver->GetItemByCell(checkCell, &groupID)); // 492

            if (groupID == c_emptyCellValue)
            {
                continue;
            }

            RETURN_IF_FAILED(m_layoutResolver->GetItemBounds(groupID, &groupBounds)); // 499

            if (margins.top > 0 && checkCell.y == groupBounds.top)
            {
                foundItemID = groupID;
                RETURN_IF_FAILED(GetActualGroupStartCell(foundItemID, &foundCell)); // 506

                if (direction == LayoutNavigationDirection_Left)
                {
                    ComPtr<IItemLayoutResolver> groupResolver;
                    RETURN_IF_FAILED(m_layoutResolver->GetLayoutResolverForContainer(foundItemID, &groupResolver)); // 513

                    Geometry::CRect groupSize;
                    RETURN_IF_FAILED(groupResolver->GetLayoutBounds(&groupSize)); // 516

                    foundCell.x = foundCell.x - 1 + groupSize.right - groupSize.left;
                }
            }
            else if (lastGroupChecked != groupID)
            {
                lastGroupChecked = groupID;

                ComPtr<IItemLayoutResolver> groupResolver;
                RETURN_IF_FAILED(m_layoutResolver->GetLayoutResolverForContainer(groupID, &groupResolver)); // 527

                POINT groupStartCell;
                RETURN_IF_FAILED(GetActualGroupStartCell(groupID, &groupStartCell)); // 530
                RETURN_IF_FAILED(GetClosestInGroupInVerticalBounds(
                    groupResolver.Get(), groupStartCell, verticalBounds, startingCell, direction, &foundCell,
                    &foundItemID)); // 531
            }

            if (foundItemID != c_emptyCellValue
                && (closestItemID == c_emptyCellValue
                    || abs(startingCell.x - foundCell.x) < abs(startingCell.x - closestCell.x)
                    || (foundCell.x == closestCell.x
                        && abs(startingCell.y - foundCell.y) < abs(startingCell.y - closestCell.y))))
            {
                closestItemID = foundItemID;
                closestCell = foundCell;
            }
        }

        if (closestItemID != c_emptyCellValue)
        {
            break;
        }
    }

    *endingCell = closestCell;
    *itemID = closestItemID;

    return S_OK;
}

void CGridTraversalOrder::ConvertAbsoluteCellToRelativeCell( // Argument names and usage assumed
    const POINT& groupStartCell, const RECT& margins, const POINT& absoluteCell, POINT* relativeCell)
{
    relativeCell->x = absoluteCell.x - groupStartCell.x - margins.left;
    relativeCell->y = absoluteCell.y - groupStartCell.y - margins.top;
}

HRESULT CGridTraversalOrder::ConvertAbsoluteVerticalBoundsToRelativeVerticalBounds( // Argument names and usage assumed
    const POINT& groupStartCell, const Geometry::CRect& groupBounds, const RECT& margins,
    const RECT& absoluteBounds, RECT* relativeBounds)
{
    relativeBounds->left   = max(0, absoluteBounds.left - groupStartCell.x - margins.left); // Unused, assumed rhs
    relativeBounds->top    = max(0, absoluteBounds.top  - groupStartCell.y - margins.top);
    relativeBounds->right  = min(groupBounds.GetWidth(),  absoluteBounds.right  - groupStartCell.x - margins.left); // Unused, assumed rhs
    relativeBounds->bottom = min(groupBounds.GetHeight(), absoluteBounds.bottom - groupStartCell.y - margins.top);
    return S_OK;
}

HRESULT CGridTraversalOrder::GetVerticalBounds(
    IItemLayoutResolver* groupResolver, const POINT& groupStartCell, const RECT& margins,
    const POINT& groupRelativeCell, RECT* verticalBounds)
{
    Geometry::CRect localVerticalBounds;

    if (groupRelativeCell.y >= margins.top)
    {
        POINT relativeCellWithoutMargins;
        relativeCellWithoutMargins.x = groupRelativeCell.x - margins.left;
        relativeCellWithoutMargins.y = groupRelativeCell.y - margins.top;

        GUID itemID;
        HRESULT hr = groupResolver->GetItemByCell(relativeCellWithoutMargins, &itemID);
        if (m_maxGroupWidth == -1 && hr == E_FAIL)
        {
            Geometry::CRect groupBounds;
            RETURN_IF_FAILED(groupResolver->GetLayoutBounds(&groupBounds)); // 608

            localVerticalBounds.top = groupBounds.top;
            localVerticalBounds.bottom = groupBounds.bottom;
        }
        else
        {
            RETURN_IF_FAILED(hr); // 615

            RECT itemBounds;
            RETURN_IF_FAILED(groupResolver->GetItemBounds(itemID, &itemBounds)); // 618

            localVerticalBounds.top = itemBounds.top + (groupStartCell.y + margins.top);
            localVerticalBounds.bottom = itemBounds.bottom + (groupStartCell.y + margins.top);
        }
    }
    else
    {
        localVerticalBounds.top = groupStartCell.y;
        localVerticalBounds.bottom = localVerticalBounds.top + margins.top;
    }

    *verticalBounds = localVerticalBounds;

    return S_OK;
}
