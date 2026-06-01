#include "pch.h"

#include "CompoundDisplacementHandler.h"

using namespace Microsoft::WRL;

EXTERN_C static const inline GUID c_emptyCellValue = {};

CCompoundDisplacementHandler::CCompoundDisplacementHandler()
{
}

HRESULT CCompoundDisplacementHandler::SetCellArray(ICellArrayManager* cellArrayManager)
{
    m_cellArrayManager = cellArrayManager;
    return S_OK;
}

HRESULT CCompoundDisplacementHandler::DisplaceItemsFromRect(
    const Geometry::CRect& targetRect, const Geometry::CRect& previousRect)
{
    bool isSizeChange = targetRect.right - targetRect.left != previousRect.right - previousRect.left
        || targetRect.bottom - targetRect.top != previousRect.bottom - previousRect.top;

    CSet<GUID> cellsAtDestination;
    RETURN_IF_FAILED(m_cellArrayManager->GetItemsInRect(targetRect, &cellsAtDestination)); // 35

    //
    //
    //
    if (cellsAtDestination.GetCount() == 1)
    {
        GUID collision;
        RETURN_IF_FAILED(cellsAtDestination.Single(&collision)); // 43
        DisplacementEvaluationSet displaceSet;
        RETURN_IF_FAILED(_TestPosibleMove(collision, targetRect, displaceSet)); // 45

        //
        if (displaceSet.selected && displaceSet.selected->possible)
        {
            RETURN_IF_FAILED(_ApplyPendingMove(collision, displaceSet.selected)); // 50
            displaceSet.Clear();
            return S_OK;
        }
    }
    else
    {
        Geometry::CSize layoutBounds = m_cellArrayManager->GetMaximumCellArrayDimensions();
        if (targetRect.GetWidth() != layoutBounds.cx)
        {
            bool success;
            RETURN_IF_FAILED(_AttemptAdjacentBlockMove(targetRect, previousRect, &cellsAtDestination, isSizeChange, &success)); // 67
            if (success)
            {
                return S_OK;
            }
        }
    }

    int shiftBeforeAmount = 0;
    int shiftAfterAmount = 0;
    int shiftBeforeRow = targetRect.bottom - 1;
    int shiftAfterRow = targetRect.top - 1;
    int positionCenterRow = targetRect.top + targetRect.GetHeight() / 2;
    bool canSplitOnCenterRow = targetRect.GetHeight() > 1 && !isSizeChange;

    cellsAtDestination.Enumerate([&](GUID collision) -> bool
    {
        Geometry::CRect collisionRectangle;
        if (m_cellArrayManager->GetItemBounds(collision, collisionRectangle) == S_OK)
        {
            int yDistance = collisionRectangle.top - targetRect.top;
            if (canSplitOnCenterRow)
            {
                canSplitOnCenterRow = collisionRectangle.top == positionCenterRow || collisionRectangle.bottom == positionCenterRow;
            }
            if (yDistance < 0)
            {
                int shiftAmount = targetRect.top - collisionRectangle.bottom;
                shiftBeforeAmount = min(shiftBeforeAmount, shiftAmount);
            }
            else
            {
                int overlap = targetRect.bottom - collisionRectangle.top;
                if (overlap > 0)
                {
                    shiftAfterAmount = max(shiftAfterAmount, overlap);
                }
            }
        }
        return true;
    });

    if (canSplitOnCenterRow && shiftBeforeAmount == 0 && shiftAfterAmount == targetRect.GetHeight())
    {
        shiftBeforeRow = positionCenterRow;
        shiftAfterRow = positionCenterRow - 1;
        shiftBeforeAmount = shiftAfterAmount / -2;
        shiftAfterAmount -= shiftAfterAmount / 2;
    }

    if (shiftBeforeAmount != 0)
    {
        _ShiftRows(ShiftCriteria::Before, shiftBeforeRow, shiftBeforeAmount);
    }

    if (!m_cellArrayManager->IsRectEmpty(targetRect) && shiftAfterAmount != 0)
    {
        _ShiftRows(ShiftCriteria::After, shiftAfterRow, shiftAfterAmount);
    }

    if (!m_cellArrayManager->IsRectEmpty(targetRect))
    {
        m_cellArrayManager->AbandonChanges();
        RETURN_HR(E_FAIL); // 160
    }

    return S_OK;
}

CCompoundDisplacementHandler::DisplacementEvaluationSet::DisplacementEvaluationSet()
{
    right = CreateRefCountedObj<DisplacementEvaluation>();
    left = CreateRefCountedObj<DisplacementEvaluation>();
    down = CreateRefCountedObj<DisplacementEvaluation>();
    up = CreateRefCountedObj<DisplacementEvaluation>();

    right->direction = DD_RIGHT;
    left->direction = DD_LEFT;
    down->direction = DD_DOWN;
    up->direction = DD_UP;
}

void CCompoundDisplacementHandler::DisplacementEvaluationSet::Clear()
{
    right->chain.RemoveAll();
    left->chain.RemoveAll();
    down->chain.RemoveAll();
    up->chain.RemoveAll();
}

HRESULT CCompoundDisplacementHandler::_ApplyPendingMove(REFGUID collision, RefCountedDisplacementEvaluation& move)
{
    for (size_t i = 0; i < move->chain.GetSize(); ++i)
    {
        ComPtr<CRefCountedObject<PendingCellCoordinates>> pendingCell;
        RETURN_IF_FAILED(move->chain.GetAt(i, pendingCell)); // 175
        RETURN_IF_FAILED(m_cellArrayManager->AddIgnoredItem(pendingCell->cell)); // 176
    }

    for (size_t i = 0; i < move->chain.GetSize(); ++i)
    {
        ComPtr<CRefCountedObject<PendingCellCoordinates>> pendingCell;
        RETURN_IF_FAILED(move->chain.GetAt(i, pendingCell)); // 182
        Geometry::CRect position;
        RETURN_IF_FAILED(m_cellArrayManager->GetItemBounds(pendingCell->cell, position)); // 184
        position.MoveTo(pendingCell->position.x, pendingCell->position.y);
        RETURN_IF_FAILED(m_cellArrayManager->MoveItemUncommitted(pendingCell->cell, position)); // 186
    }

    for (size_t i = 0; i < move->chain.GetSize(); ++i)
    {
        ComPtr<CRefCountedObject<PendingCellCoordinates>> pendingCell;
        RETURN_IF_FAILED(move->chain.GetAt(i, pendingCell)); // 192
        RETURN_IF_FAILED(m_cellArrayManager->RemoveIgnoredItem(pendingCell->cell)); // 193
    }

    Geometry::CRect position;

    RETURN_IF_FAILED(m_cellArrayManager->GetItemBounds(collision, position)); // 198
    position.MoveTo(move->position.x, move->position.y);
    RETURN_IF_FAILED(m_cellArrayManager->AddIgnoredItem(collision)); // 200
    RETURN_IF_FAILED(m_cellArrayManager->MoveItemUncommitted(collision, position)); // 201
    RETURN_IF_FAILED(m_cellArrayManager->RemoveIgnoredItem(collision)); // 202

    return S_OK;
}

HRESULT CCompoundDisplacementHandler::_AttemptAdjacentBlockMove(
    const Geometry::CRect& position, const Geometry::CRect& previousPosition, const CSet<GUID>* cellsAtDestination,
    const bool isSizeChange, bool* success)
{
    Geometry::CPoint collisionTopLeft = position.GetBottomRight();
    Geometry::CPoint collisionBottomRight = position.GetTopLeft();

    *success = false;

    bool completelyContained = true;
    cellsAtDestination->Enumerate([&](GUID collision) -> bool
    {
        Geometry::CRect collisionRectangle;
        if (m_cellArrayManager->GetItemBounds(collision, collisionRectangle) == S_OK)
        {
            collisionTopLeft = Geometry::CPoint(
                min(collisionTopLeft.x, collisionRectangle.left),
                min(collisionTopLeft.y, collisionRectangle.top));
            collisionBottomRight = Geometry::CPoint(
                max(collisionBottomRight.x, collisionRectangle.right),
                max(collisionBottomRight.y, collisionRectangle.bottom));

            if (position != position.Union(collisionRectangle))
            {
                completelyContained = false;
                return false;
            }
        }
        return true;
    });

    if (completelyContained)
    {
        Geometry::CRect emptyAdjacent = _GetAdjacentSpace(
            position, previousPosition, collisionBottomRight, collisionTopLeft, isSizeChange);
        if (emptyAdjacent != position)
        {
            int offsetX = 0;
            int offsetY = 0;
            if (emptyAdjacent.left == position.left)
                offsetY = emptyAdjacent.top - collisionTopLeft.y;
            else
                offsetX = emptyAdjacent.left - collisionTopLeft.x;

            bool enumSuccess = cellsAtDestination->Enumerate([&](GUID itemID) -> bool
            {
                return SUCCEEDED(m_cellArrayManager->AddIgnoredItem(itemID));
            });
            RETURN_HR_IF(E_FAIL, !enumSuccess); // 276

            cellsAtDestination->Enumerate([&](GUID collision) -> bool
            {
                HRESULT hr = S_OK;
                Geometry::CRect collisionRectangle;
                if (m_cellArrayManager->GetItemBounds(collision, collisionRectangle) == S_OK)
                {
                    collisionRectangle.Offset(offsetX, offsetY);
                    hr = m_cellArrayManager->MoveItemUncommitted(collision, collisionRectangle);
                }
                return hr == S_OK;
            });

            enumSuccess = cellsAtDestination->Enumerate([&](GUID itemID) -> bool
            {
                return SUCCEEDED(m_cellArrayManager->RemoveIgnoredItem(itemID));
            });
            RETURN_HR_IF(E_FAIL, !enumSuccess); // 294

            *success = true;
        }
    }

    return S_OK;
}

HRESULT CCompoundDisplacementHandler::_CheckPossibleMove(REFGUID cell, const Geometry::CPoint& target, GUID* pItemID)
{
    Geometry::CRect targetPosition;
    RETURN_IF_FAILED(m_cellArrayManager->GetItemBounds(cell, targetPosition)); // 637

    targetPosition.MoveTo(target.x, target.y);

    if (!m_cellArrayManager->IsValidRect(targetPosition))
    {
        *pItemID = cell;
        return S_OK;
    }

    CSet<GUID> cellsAtDestination;
    RETURN_IF_FAILED(m_cellArrayManager->AddIgnoredItem(cell)); // 648
    RETURN_IF_FAILED(m_cellArrayManager->GetItemsInRect(targetPosition, &cellsAtDestination)); // 649
    RETURN_IF_FAILED(m_cellArrayManager->RemoveIgnoredItem(cell)); // 650

    if (cellsAtDestination.GetCount() != 0)
    {
        if (cellsAtDestination.GetCount() == 1)
        {
            GUID item;
            RETURN_IF_FAILED(cellsAtDestination.Single(&item)); // 657
            *pItemID = item;
        }
        else
        {
            *pItemID = cell;
        }
    }
    else
    {
        *pItemID = c_emptyCellValue;
    }

    return S_OK;
}

Geometry::CRect CCompoundDisplacementHandler::_GetAdjacentSpace(
    const Geometry::CRect& position, const Geometry::CRect& previousPosition,
    const Geometry::CPoint& collisionBottomRight, const Geometry::CPoint& collisionTopLeft, const bool isSizeChange)
{
    Geometry::CSize blockSize(
        collisionBottomRight.x - collisionTopLeft.x,
        collisionBottomRight.y - collisionTopLeft.y);
    Geometry::CRect verticalStart(
        position.left,  collisionTopLeft.y,
        position.right, collisionBottomRight.y);

    Geometry::CRect rightAdjacent = _GetEmptyAdjacentSpace(verticalStart, blockSize, DD_RIGHT);
    bool rightAdjacentIsAvailable = rightAdjacent != verticalStart;

    Geometry::CRect leftAdjacent = _GetEmptyAdjacentSpace(verticalStart, blockSize, DD_LEFT);
    bool leftAdjacentIsAvailable = leftAdjacent != verticalStart;

    Geometry::CRect emptyAdjacent = position;
    if (rightAdjacentIsAvailable)
    {
        if (leftAdjacentIsAvailable
            && previousPosition.GetWidth() > 0
            && previousPosition.GetHeight() > 0
            && rightAdjacent != previousPosition)
        {
            emptyAdjacent = leftAdjacent;
        }
        else
        {
            emptyAdjacent = rightAdjacent;
        }
    }
    else
    {
        if (leftAdjacentIsAvailable)
        {
            emptyAdjacent = leftAdjacent;
        }
        else
        {
            Geometry::CRect horizontalStart(
                collisionTopLeft.x, collisionTopLeft.x + blockSize.cx,
                position.top, position.bottom);

            Geometry::CRect downAdjacent = _GetEmptyAdjacentSpace(horizontalStart, blockSize, DD_DOWN);
            bool downAdjacentIsAvailable = downAdjacent != horizontalStart;

            Geometry::CRect upAdjacent = _GetEmptyAdjacentSpace(horizontalStart, blockSize, DD_UP);
            if (isSizeChange && upAdjacent.top < 0)
            {
                upAdjacent = horizontalStart;
            }
            bool upAdjacentIsAvailable = upAdjacent != horizontalStart;

            if (downAdjacentIsAvailable)
            {
                if (upAdjacentIsAvailable
                    && previousPosition.GetWidth() > 0 && previousPosition.GetHeight() > 0
                    && upAdjacent == previousPosition)
                {
                    emptyAdjacent = upAdjacent;
                }
                else
                {
                    emptyAdjacent = downAdjacent;
                }
            }
            else
            {
                if (upAdjacentIsAvailable)
                {
                    emptyAdjacent = upAdjacent;
                }
            }
        }
    }

    return emptyAdjacent;
}

Geometry::CRect CCompoundDisplacementHandler::_GetEmptyAdjacentSpace(
    const Geometry::CRect& position, const Geometry::CSize& size, const DISPLACEMENT_DIRECTION direction)
{
    Geometry::CRect adjacent;
    switch (direction)
    {
        case DD_RIGHT:
            adjacent = Geometry::CRect(
                position.right, position.top,
                position.right + size.cx, position.top + size.cy);
            break;

        case DD_LEFT:
            adjacent = Geometry::CRect(
                position.left - size.cx, position.top,
                position.left, position.top + size.cy);
            break;

        case DD_DOWN:
            adjacent = Geometry::CRect(
                position.left, position.bottom,
                position.left + size.cx, position.bottom + size.cy);
            break;

        case DD_UP:
            adjacent = Geometry::CRect(
                position.left, position.top - size.cy,
                position.left + size.cx, position.top);
            break;
    }

    return m_cellArrayManager->IsValidRect(adjacent) && m_cellArrayManager->IsRectEmpty(adjacent) ? adjacent : position;
}

inline constexpr int c_nMaxVerticalChainLength = 2;

HRESULT CCompoundDisplacementHandler::_PrepareChain(
    REFGUID cell, const Geometry::CSize& acceptedSize, const DISPLACEMENT_DIRECTION direction,
    const int moveAmount, const int linkCount, CCoSimpleArray<RefCountedPendingCellCoordinates>* pItemsInChain)
{
    Geometry::CRect cellPosition;
    RETURN_IF_FAILED(m_cellArrayManager->GetItemBounds(cell, cellPosition)); // 708

    RETURN_HR_IF_EXPECTED(E_FAIL, (cellPosition.GetWidth() > acceptedSize.cx) || (cellPosition.GetHeight() > acceptedSize.cy));
    if ((direction == DD_UP || direction == DD_DOWN))
    {
        RETURN_HR_IF_EXPECTED(E_FAIL, abs(linkCount * moveAmount) > c_nMaxVerticalChainLength);
    }

    Geometry::CPoint movePosition = _MovePoint(cellPosition.GetTopLeft(), direction, moveAmount);
    RETURN_HR_IF_EXPECTED(E_FAIL, movePosition.y < 0);

    GUID blocking = c_emptyCellValue;
    RETURN_IF_FAILED_EXPECTED(_CheckPossibleMove(cell, movePosition, &blocking));
    RETURN_HR_IF_EXPECTED(E_FAIL, blocking == cell);

    if (blocking == c_emptyCellValue)
    {
        pItemsInChain->RemoveAll();

        ComPtr<CRefCountedObject<PendingCellCoordinates>> coordinates = CreateRefCountedObj<PendingCellCoordinates>();
        coordinates->cell = cell;
        coordinates->position = movePosition;

        RETURN_IF_FAILED(pItemsInChain->Add(coordinates)); // 745
    }
    else
    {
        RETURN_IF_FAILED_EXPECTED(_PrepareChain(
            blocking, acceptedSize, direction, moveAmount, linkCount + 1, pItemsInChain));

        ComPtr<CRefCountedObject<PendingCellCoordinates>> coordinates = CreateRefCountedObj<PendingCellCoordinates>();
        coordinates->cell = cell;
        coordinates->position = movePosition;

        RETURN_IF_FAILED(pItemsInChain->Add(coordinates)); // 756
    }

    return S_OK;
}

HRESULT CCompoundDisplacementHandler::_ShiftRows(const ShiftCriteria criteria, const int y, const int yOffset)
{
    Geometry::CRect arrayDimensions = m_cellArrayManager->GetCurrentCellArrayBounds();

    CSet<GUID> items;
    RETURN_IF_FAILED(m_cellArrayManager->GetItemsInRect(arrayDimensions, &items)); // 434

    bool enumSuccess = items.Enumerate([&](GUID itemID) -> bool
    {
        return SUCCEEDED(m_cellArrayManager->AddIgnoredItem(itemID));
    });
    RETURN_HR_IF(E_FAIL, !enumSuccess); // 440

    enumSuccess = items.Enumerate([&](GUID itemID) -> bool
    {
        Geometry::CRect position;
        HRESULT hr = m_cellArrayManager->GetItemBounds(itemID, position);
        if (SUCCEEDED(hr))
        {
            if (criteria == ShiftCriteria::After)
            {
                if (position.top <= y)
                    return true;
            }
            else
            {
                if (position.top >= y)
                    return true;
            }

            position.Offset(0, yOffset);

            hr = m_cellArrayManager->MoveItemUncommitted(itemID, position);
        }

        return SUCCEEDED(hr);
    });
    RETURN_HR_IF(E_FAIL, !enumSuccess); // 466

    enumSuccess = items.Enumerate([&](GUID itemID) -> bool
    {
        return SUCCEEDED(m_cellArrayManager->RemoveIgnoredItem(itemID));
    });
    RETURN_HR_IF(E_FAIL, !enumSuccess); // 472

    return S_OK;
}

HRESULT CCompoundDisplacementHandler::_TestPosibleMove(
    REFGUID collision, const Geometry::CRect& position, DisplacementEvaluationSet& move)
{
    Geometry::CRect collisionPosition;
    RETURN_IF_FAILED(m_cellArrayManager->GetItemBounds(collision, collisionPosition)); // 483

    move.right->possible = false;
    move.left->possible = false;
    move.down->possible = false;
    move.up->possible = false;

    move.right->distance = position.right - collisionPosition.left;
    move.left->distance = collisionPosition.right - position.left;
    move.down->distance = position.bottom - collisionPosition.top;
    move.up->distance = collisionPosition.bottom - position.top;

    if (collisionPosition.left >= position.left)
    {
        RETURN_IF_FAILED(_TestIfPossibleToMoveObstructionInDirection(collision, collisionPosition, move.right)); // 500
    }

    if (collisionPosition.right <= position.right)
    {
        RETURN_IF_FAILED(_TestIfPossibleToMoveObstructionInDirection(collision, collisionPosition, move.left)); // 505
    }

    if (collisionPosition.top >= position.top)
    {
        RETURN_IF_FAILED(_TestIfPossibleToMoveObstructionInDirection(collision, collisionPosition, move.down)); // 510
    }

    if (collisionPosition.bottom <= position.bottom)
    {
        RETURN_IF_FAILED(_TestIfPossibleToMoveObstructionInDirection(collision, collisionPosition, move.up)); // 515
    }

    if (move.right->possible)
    {
        if (move.left->possible)
        {
            int diff = abs(move.right->distance) - abs(move.left->distance);
            if (diff > 0)
            {
                move.selected = move.left;
            }
            else
            {
                move.selected = move.right;
            }
        }
        else
        {
            move.selected = move.right;
        }
    }
    else if (move.left->possible)
    {
        move.selected = move.left;
    }
    else if (move.down->possible)
    {
        if (move.up->possible)
        {
            int diff = abs(move.down->distance) - abs(move.up->distance);
            if (diff)
            {
                if (diff < 0)
                {
                    move.selected = move.down;
                }
                else
                {
                    move.selected = move.up;
                }
            }
            else
            {
                if (move.up->position.y < 0)
                {
                    move.selected = move.down;
                }
                else
                {
                    move.selected = move.up;
                }
            }
        }
        else
        {
            move.selected = move.down;
        }
    }
    else if (move.up->possible)
    {
        move.selected = move.up;
    }

    return S_OK;
}

HRESULT CCompoundDisplacementHandler::_TestIfPossibleToMoveObstructionInDirection(
    REFGUID itemID, const Geometry::CRect& collisionPosition, RefCountedDisplacementEvaluation& move)
{
    move->position = _MovePoint(collisionPosition.GetTopLeft(), move->direction, move->distance);

    GUID blockingCollision = c_emptyCellValue;
    RETURN_IF_FAILED(_CheckPossibleMove(itemID, move->position, &blockingCollision)); // 586

    move->possible = blockingCollision == c_emptyCellValue;
    move->chain.RemoveAll();
    if (move->possible || blockingCollision == itemID)
    {
        return S_OK;
    }

    HRESULT hr = _PrepareChain(
        blockingCollision, Geometry::CSize(collisionPosition.GetWidth(), collisionPosition.GetHeight()),
        move->direction, move->distance, 1, &move->chain);
    if (hr == S_OK)
    {
        int totalDistance = move->distance * ((int)move->chain.GetSize() + 1);
        move->possible = true;
        move->distance = totalDistance;
    }
    else
    {
        RETURN_HR_IF(E_INVALIDARG, hr != E_FAIL); // 620
    }

    return S_OK;
}

Geometry::CPoint CCompoundDisplacementHandler::_MovePoint(
    const Geometry::CPoint& point, const DISPLACEMENT_DIRECTION direction, const int amount)
{
    Geometry::CPoint p = point;
    switch (direction)
    {
        case DD_RIGHT:
            p.Offset(amount, 0);
            break;
        case DD_LEFT:
            p.Offset(-amount, 0);
            break;
        case DD_DOWN:
            p.Offset(0, amount);
            break;
        case DD_UP:
            p.Offset(0, -amount);
            break;
    }

    return p;
}
