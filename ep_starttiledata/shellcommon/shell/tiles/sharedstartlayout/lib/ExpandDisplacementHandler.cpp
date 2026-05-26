#include "pch.h"

#include "ExpandDisplacementHandler.h"

CExpandDisplacementHandler::CExpandDisplacementHandler()
    : m_cellArrayManager(nullptr)
    , m_rowOrColumn(EXPAND_COLLAPSE_DIRECTION_ROW)
{
}

HRESULT CExpandDisplacementHandler::RuntimeClassInitialize(EXPAND_COLLAPSE_DIRECTION rowOrColumn)
{
    m_rowOrColumn = rowOrColumn;
    return S_OK;
}

HRESULT CExpandDisplacementHandler::SetCellArray(ICellArrayManager* cellArrayManager)
{
    m_cellArrayManager = cellArrayManager;
    return S_OK;
}

HRESULT CExpandDisplacementHandler::DisplaceItemsFromRect(const Geometry::CRect& targetRect, const Geometry::CRect& previousRect)
{
    Geometry::CRect arrayDimensions = m_cellArrayManager->GetCurrentCellArrayBounds();

    int shiftBefore;
    if (m_rowOrColumn == EXPAND_COLLAPSE_DIRECTION_ROW)
        shiftBefore = targetRect.bottom - 1;
    else
        shiftBefore = targetRect.right  - 1;

    int shiftBeforeAmount = 0;

    int shiftAfter;
    if (m_rowOrColumn == EXPAND_COLLAPSE_DIRECTION_ROW)
        shiftAfter = targetRect.top  - 1;
    else
        shiftAfter = targetRect.left - 1;

    int shiftAfterAmount;
    if (m_rowOrColumn == EXPAND_COLLAPSE_DIRECTION_ROW)
        shiftAfterAmount = targetRect.GetHeight();
    else
        shiftAfterAmount = targetRect.GetWidth();

    int positionCenterline;
    if (m_rowOrColumn == EXPAND_COLLAPSE_DIRECTION_ROW)
        positionCenterline = targetRect.top  + abs(targetRect.GetHeight() / 2);
    else
        positionCenterline = targetRect.left + abs(targetRect.GetWidth()  / 2);

    bool canSplitOnCenterline;
    if (m_rowOrColumn == EXPAND_COLLAPSE_DIRECTION_ROW)
        canSplitOnCenterline = targetRect.GetHeight() > 1;
    else
        canSplitOnCenterline = targetRect.GetWidth()  > 1;

    CSet<GUID> itemsCollidingWithTargetRect;
    CSet<GUID> itemsBeforeCutline;
    CSet<GUID> itemsAfterCutline;

    if (canSplitOnCenterline && previousRect.GetWidth() > 0 && previousRect.GetHeight() > 0)
    {
        canSplitOnCenterline = previousRect.GetWidth()  == targetRect.GetWidth()
                            && previousRect.GetHeight() == targetRect.GetHeight();
    }

    RETURN_IF_FAILED(m_cellArrayManager->GetItemsInRect(targetRect, &itemsCollidingWithTargetRect)); // 56

    bool enumSuccess = itemsCollidingWithTargetRect.Enumerate([&](GUID itemID) -> bool // @Note: enumSuccess added in 15063, so that Enumerate calls are no longer in a macro body
    {
        Geometry::CRect itemBounds;
        HRESULT hr = m_cellArrayManager->GetItemBounds(itemID, itemBounds);
        if (hr >= 0)
        {
            if (m_rowOrColumn == EXPAND_COLLAPSE_DIRECTION_ROW)
            {
                if (canSplitOnCenterline)
                {
                    canSplitOnCenterline = itemBounds.top == positionCenterline || itemBounds.bottom == positionCenterline;
                }
                if (itemBounds.top - targetRect.top < 0)
                {
                    int shiftAmount = targetRect.top - itemBounds.bottom;
                    shiftBeforeAmount = min(shiftBeforeAmount, shiftAmount);
                }
            }
            else
            {
                if (canSplitOnCenterline)
                {
                    canSplitOnCenterline = itemBounds.left == positionCenterline || itemBounds.right == positionCenterline;
                }
                if (itemBounds.left - targetRect.left < 0)
                {
                    int shiftAmount = targetRect.left - itemBounds.right;
                    shiftBeforeAmount = min(shiftBeforeAmount, shiftAmount);
                }
            }
        }
        return SUCCEEDED(hr);
    });
    RETURN_HR_IF(E_FAIL, !enumSuccess); // 92

    if (canSplitOnCenterline
        && shiftBeforeAmount == 0
        && shiftAfterAmount == (m_rowOrColumn == EXPAND_COLLAPSE_DIRECTION_ROW ? targetRect.GetHeight() : targetRect.GetWidth()))
    {
        Geometry::CPoint coordinateToCheck = m_rowOrColumn == EXPAND_COLLAPSE_DIRECTION_ROW
                                                 ? Geometry::CPoint(0, -1)
                                                 : Geometry::CPoint(-1, 0);
        if (m_cellArrayManager->IsValidCellCoordinate(coordinateToCheck))
        {
            shiftBeforeAmount = shiftAfterAmount - abs(shiftAfterAmount / 2);
            shiftAfterAmount = shiftAfterAmount - shiftBeforeAmount;
            shiftBefore = positionCenterline;
            shiftAfter = positionCenterline - 1;
        }
    }

    if (shiftBeforeAmount != 0)
    {
        RETURN_IF_FAILED(m_cellArrayManager->GetItemsInRect(
            m_rowOrColumn == EXPAND_COLLAPSE_DIRECTION_ROW
                ? Geometry::CRect(arrayDimensions.left, arrayDimensions.top, arrayDimensions.right, shiftBefore)
                : Geometry::CRect(arrayDimensions.left, arrayDimensions.top, shiftBefore, arrayDimensions.bottom),
            &itemsBeforeCutline)); // 116

        enumSuccess = itemsBeforeCutline.Enumerate([&](GUID itemID) -> bool
        {
            HRESULT hr = m_cellArrayManager->AddIgnoredItem(itemID);
            return SUCCEEDED_LOG(hr); // 121 // @Note: Logging inside lambda bodies added in 15063
        });
        RETURN_HR_IF(E_FAIL, !enumSuccess); // 123

        enumSuccess = itemsBeforeCutline.Enumerate([&](GUID itemID) -> bool
        {
            Geometry::CRect itemBounds;
            HRESULT hr = m_cellArrayManager->GetItemBounds(itemID, itemBounds);

            if (SUCCEEDED_LOG(hr)) // 130
            {
                if (m_rowOrColumn == EXPAND_COLLAPSE_DIRECTION_ROW)
                {
                    if (itemBounds.top >= shiftBefore)
                    {
                        return true;
                    }
                    itemBounds.MoveTo(itemBounds.left, itemBounds.top + shiftBeforeAmount);
                }
                else
                {
                    if (itemBounds.left >= shiftBefore)
                    {
                        return true;
                    }
                    itemBounds.MoveTo(itemBounds.left + shiftBeforeAmount, itemBounds.top);
                }
                hr = LOG_IF_FAILED(m_cellArrayManager->MoveItemUncommitted(itemID, itemBounds)); // 148
            }
            return SUCCEEDED(hr);
        });
        RETURN_HR_IF(E_FAIL, !enumSuccess); // 152

        enumSuccess = itemsBeforeCutline.Enumerate([&](GUID itemID) -> bool
        {
            HRESULT hr = m_cellArrayManager->RemoveIgnoredItem(itemID);
            return SUCCEEDED_LOG(hr); // 157
        });
        RETURN_HR_IF(E_FAIL, !enumSuccess); // 159
    }

    if (!m_cellArrayManager->IsRectEmpty(targetRect) && shiftAfterAmount != 0)
    {
        RETURN_IF_FAILED(m_cellArrayManager->GetItemsInRect(
            m_rowOrColumn == EXPAND_COLLAPSE_DIRECTION_ROW
                ? Geometry::CRect(arrayDimensions.left, shiftAfter, arrayDimensions.right, arrayDimensions.bottom)
                : Geometry::CRect(shiftAfter, arrayDimensions.top, arrayDimensions.right, arrayDimensions.bottom),
            &itemsAfterCutline)); // 168

        enumSuccess = itemsAfterCutline.Enumerate([&](GUID itemID) -> bool
        {
            return SUCCEEDED(m_cellArrayManager->AddIgnoredItem(itemID));
        });
        RETURN_HR_IF(E_FAIL, !enumSuccess); // 174

        enumSuccess = itemsAfterCutline.Enumerate([&](GUID itemID) -> bool
        {
            Geometry::CRect itemBounds;
            HRESULT hr = m_cellArrayManager->GetItemBounds(itemID, itemBounds);
            if (SUCCEEDED(hr))
            {
                if (m_rowOrColumn == EXPAND_COLLAPSE_DIRECTION_ROW)
                {
                    if (itemBounds.top <= shiftAfter)
                    {
                        return true;
                    }
                    itemBounds.MoveTo(itemBounds.left, itemBounds.top + shiftAfterAmount);
                }
                else
                {
                    if (itemBounds.left <= shiftAfter)
                    {
                        return true;
                    }
                    itemBounds.MoveTo(itemBounds.left + shiftAfterAmount, itemBounds.top);
                }
                hr = m_cellArrayManager->MoveItemUncommitted(itemID, itemBounds);
            }
            return SUCCEEDED(hr);
        });
        RETURN_HR_IF(E_FAIL, !enumSuccess); // 202

        enumSuccess = itemsAfterCutline.Enumerate([&](GUID itemID) -> bool
        {
            return SUCCEEDED(m_cellArrayManager->RemoveIgnoredItem(itemID));
        });
        RETURN_HR_IF(E_FAIL, !enumSuccess); // 208
    }

    return S_OK;
}
