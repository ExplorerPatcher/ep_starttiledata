#include "pch.h"

#include "PathCollapseHandler.h"

EXTERN_C static const inline GUID c_emptyCellValue = {};

CPathCollapseHandler::CPathCollapseHandler()
    : m_cellArrayManager(nullptr)
    , m_expandCollapseDirection(EXPAND_COLLAPSE_DIRECTION_ROW)
    , m_smartCollapseEnabled(true)
{
}

HRESULT CPathCollapseHandler::RuntimeClassInitialize(EXPAND_COLLAPSE_DIRECTION expandCollapseDirection)
{
    m_expandCollapseDirection = expandCollapseDirection;
    return S_OK;
}

HRESULT CPathCollapseHandler::SetCellArray(ICellArrayManager* cellArrayManager)
{
    m_cellArrayManager = cellArrayManager;
    return S_OK;
}

HRESULT CPathCollapseHandler::Collapse(const Geometry::CRect& sourceCells, const Geometry::CRect& targetCells)
{
    Geometry::CRect sizeArrayDimensions = m_cellArrayManager->GetCurrentCellArrayBounds();

    Geometry::CRect pathMustInclude = sourceCells;
    Geometry::CRect pathCannotInclude = targetCells;

    int firstLoopMin = m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW
                           ? sizeArrayDimensions.top : sizeArrayDimensions.left;
    int firstLoopMax = m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW
                           ? sizeArrayDimensions.bottom : sizeArrayDimensions.right;
    for (int firstLoop = firstLoopMin; firstLoop < firstLoopMax; ++firstLoop)
    {
        bool empty = false;
        bool startsEmpty = false;

        int secondLoopMin = m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW
                                ? sizeArrayDimensions.left : sizeArrayDimensions.top;
        int secondLoopMax = m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW
                                ? sizeArrayDimensions.right : sizeArrayDimensions.bottom;
        for (int secondLoop = secondLoopMin; secondLoop < secondLoopMax; ++secondLoop)
        {
            GUID itemID = m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW
                              ? m_cellArrayManager->GetItemAtCell(secondLoop, firstLoop)
                              : m_cellArrayManager->GetItemAtCell(firstLoop, secondLoop);
            empty = itemID == c_emptyCellValue;
            if (secondLoop == secondLoopMin && empty)
            {
                startsEmpty = true;
            }
            else if (!empty)
            {
                if (startsEmpty && m_smartCollapseEnabled)
                {
                    if (m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW
                            ? sourceCells.GetHeight()
                            : sourceCells.GetWidth())
                    {
                        CCoSimpleArray<Geometry::CPoint> path;

                        int minFirstLoop = firstLoopMin;
                        int maxFirstLoop = firstLoopMax;

                        int spread = 2;
                        minFirstLoop = max(firstLoop - spread, minFirstLoop);
                        maxFirstLoop = min(firstLoop + spread, maxFirstLoop);

                        for (; spread > 0; --spread)
                        {
                            if (_FindEmptyPath(
                                firstLoop, minFirstLoop, maxFirstLoop, secondLoopMin, spread, pathMustInclude,
                                pathCannotInclude, false, path))
                            {
                                RETURN_IF_FAILED(_CollapseByPath(spread, path)); // 112
                                firstLoop = min(firstLoopMin, firstLoop - spread);
                                firstLoopMax = max(firstLoopMin, firstLoopMax - spread);
                                break;
                            }
                        }
                    }
                }

                break;
            }
        }

        if (empty)
        {
            Geometry::CRect boundsToCollapse = m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW
                ? Geometry::CRect(sizeArrayDimensions.left, firstLoop, sizeArrayDimensions.right, sizeArrayDimensions.bottom)
                : Geometry::CRect(firstLoop, sizeArrayDimensions.top, sizeArrayDimensions.right, sizeArrayDimensions.bottom);
            if (!boundsToCollapse.Intersects(sourceCells))
            {
                RETURN_IF_FAILED(_CollapseByOffset(boundsToCollapse, 1)); // 79
                firstLoop = max(firstLoopMin, firstLoop - 1) - 1;
                firstLoopMax = max(firstLoopMin, firstLoopMax - 1);
            }
        }
    }

    return S_OK;
}

void CPathCollapseHandler::EnableSmartCollapse()
{
    m_smartCollapseEnabled = true;
}

void CPathCollapseHandler::DisableSmartCollapse()
{
    m_smartCollapseEnabled = false;
}

HRESULT CPathCollapseHandler::_CollapseByOffset(const Geometry::CRect& bounds, const int offset)
{
    int firstLoopMin = m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW ? bounds.top : bounds.left;
    int firstLoopMax = m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW ? bounds.bottom : bounds.right;
    for (int firstLoop = firstLoopMin; firstLoop < firstLoopMax; ++firstLoop)
    {
        int secondLoopMin = m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW ? bounds.left : bounds.top;
        int secondLoopMax = m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW ? bounds.right : bounds.bottom;
        for (int secondLoop = secondLoopMin; secondLoop < secondLoopMax; ++secondLoop)
        {
            GUID itemID = m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW
                              ? m_cellArrayManager->GetItemAtCell(secondLoop, firstLoop)
                              : m_cellArrayManager->GetItemAtCell(firstLoop, secondLoop);
            if (itemID != c_emptyCellValue)
            {
                Geometry::CRect itemBounds;
                RETURN_IF_FAILED(m_cellArrayManager->GetItemBounds(itemID, itemBounds)); // 156

                if (m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW)
                {
                    if (itemBounds.top < firstLoop)
                    {
                        continue;
                    }

                    itemBounds.MoveTo(itemBounds.left, itemBounds.top - offset);
                }
                else
                {
                    if (itemBounds.left < firstLoop)
                    {
                        continue;
                    }

                    itemBounds.MoveTo(itemBounds.left - offset, itemBounds.top);
                }

                RETURN_IF_FAILED(m_cellArrayManager->MoveItemUncommitted(itemID, itemBounds)); // 177
            }
        }
    }

    return S_OK;
}

HRESULT CPathCollapseHandler::_CollapseByPath(const int shiftAmount, CCoSimpleArray<Geometry::CPoint>& path)
{
    const Geometry::CRect arrayDimensions = m_cellArrayManager->GetCurrentCellArrayBounds();

    CSet<GUID> _visitedTiles;
    for (int yIndex = arrayDimensions.top; yIndex < arrayDimensions.bottom; ++yIndex)
    {
        for (int xIndex = arrayDimensions.left; xIndex < arrayDimensions.right; ++xIndex)
        {
            GUID itemID = m_cellArrayManager->GetItemAtCell(xIndex, yIndex);
            if (itemID != c_emptyCellValue && !_visitedTiles.Contains(itemID))
            {
                RETURN_IF_FAILED(_visitedTiles.Add(itemID)); // 201
                Geometry::CRect itemBounds;
                RETURN_IF_FAILED(m_cellArrayManager->GetItemBounds(itemID, itemBounds)); // 203

                int firstAxisOfInterest = m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW
                    ? itemBounds.bottom : itemBounds.right;
                int secondAxisOfInterest = m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW
                    ? itemBounds.left : itemBounds.top;

                Geometry::CPoint pathPoint;
                RETURN_IF_FAILED(path.GetAt(secondAxisOfInterest, pathPoint)); // 211

                int pathPointOfInterest = m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW
                    ? pathPoint.y : pathPoint.x;
                if (firstAxisOfInterest <= pathPointOfInterest)
                {
                    continue;
                }

                if (m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW)
                {
                    itemBounds.MoveTo(itemBounds.left, itemBounds.top - shiftAmount);
                }
                else
                {
                    itemBounds.MoveTo(itemBounds.left - shiftAmount, itemBounds.top);
                }

                RETURN_IF_FAILED(m_cellArrayManager->MoveItemUncommitted(itemID, itemBounds)); // 229
            }
        }
    }

    return S_OK;
}

bool CPathCollapseHandler::_FindEmptyPath(
    const int firstAxisStart, const int firstAxisMin, const int firstAxisMax, const int secondAxis,
    const int spread, Geometry::CRect& pathMustInclude, const Geometry::CRect& pathCannotInclude,
    bool haveMetIncludeRequirement, CCoSimpleArray<Geometry::CPoint>& path)
{
    Geometry::CRect sizeArrayDimensions = m_cellArrayManager->GetCurrentCellArrayBounds();
    bool empty = false;

    int secondAxisMax = m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW
                            ? sizeArrayDimensions.GetWidth() : sizeArrayDimensions.GetHeight();
    if (secondAxis >= secondAxisMax)
    {
        if (haveMetIncludeRequirement)
        {
            path.RemoveAll();
            empty = true;
        }
    }
    else
    {
        int localFirstAxisMin = max(firstAxisMin, firstAxisStart - spread);
        int localFirstAxisMax = min(firstAxisMax, firstAxisStart + spread);

        if ((secondAxis == (m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW ? 1 : 0)
                 ? sizeArrayDimensions.left : sizeArrayDimensions.top) != 0)
        {
            localFirstAxisMin = localFirstAxisMax = firstAxisStart;
        }

        for (int firstAxis = localFirstAxisMin; firstAxis <= localFirstAxisMax; ++firstAxis)
        {
            bool completed = false;
            if (m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW)
            {
                if (pathCannotInclude.GetHeight() > 0 && firstAxis >= pathCannotInclude.top && firstAxis < pathCannotInclude.bottom)
                {
                    completed = true;
                }
            }
            else
            {
                if (pathCannotInclude.GetWidth() > 0 && firstAxis >= pathCannotInclude.left && firstAxis < pathCannotInclude.right)
                {
                    completed = true;
                }
            }
            if (completed)
            {
                continue;
            }

            for (int spreadFirstAxis = 0; spreadFirstAxis < spread; ++spreadFirstAxis)
            {
                if (!m_cellArrayManager->IsRectEmpty(m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW
                     ? Geometry::CRect(secondAxis, firstAxis + spreadFirstAxis, secondAxis + 1, firstAxis + spreadFirstAxis + 1)
                     : Geometry::CRect(firstAxis + spreadFirstAxis, secondAxis, firstAxis + spreadFirstAxis + 1, secondAxis + 1)))
                {
                    completed = true;
                    break;
                }
            }
            if (completed)
            {
                continue;
            }

            if (m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW)
            {
                if (!haveMetIncludeRequirement && firstAxis >= pathMustInclude.top && firstAxis <= pathMustInclude.bottom)
                {
                    haveMetIncludeRequirement = true;
                }
            }
            else
            {
                if (!haveMetIncludeRequirement && firstAxis >= pathMustInclude.left && firstAxis <= pathMustInclude.right)
                {
                    haveMetIncludeRequirement = true;
                }
            }

            bool recursionIsDone = _FindEmptyPath(
                firstAxis, firstAxisMin, firstAxisMax, secondAxis + 1, spread, pathMustInclude, pathCannotInclude,
                haveMetIncludeRequirement, path);
            if (recursionIsDone)
            {
                path.InsertAt(m_expandCollapseDirection == EXPAND_COLLAPSE_DIRECTION_ROW
                                  ? Geometry::CPoint(secondAxis, firstAxis)
                                  : Geometry::CPoint(firstAxis, secondAxis), 0);
                empty = recursionIsDone;
                break;
            }
        }
    }

    return empty;
}
