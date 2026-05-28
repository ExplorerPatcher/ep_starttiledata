#include "pch.h"

#include "PushDownDisplacementHandler.h"

CPushDownDisplacementHandler::CPushDownDisplacementHandler()
    : m_cellArrayManager(nullptr)
{
}

HRESULT CPushDownDisplacementHandler::RuntimeClassInitialize()
{
    return S_OK;
}

HRESULT CPushDownDisplacementHandler::SetCellArray(ICellArrayManager* cellArrayManager)
{
    m_cellArrayManager = cellArrayManager;
    return S_OK;
}

HRESULT CPushDownDisplacementHandler::DisplaceItemsFromRect(
    const Geometry::CRect& targetRect, const Geometry::CRect& previousRect)
{
    Geometry::CRect arrayDimensions = m_cellArrayManager->GetCurrentCellArrayBounds();

    int pushDownAmount = 0;

    CSet<GUID> itemsCollidingWithTargetRect;
    CSet<GUID> itemsToMove;

    RETURN_IF_FAILED(m_cellArrayManager->GetItemsInRect(targetRect, &itemsCollidingWithTargetRect)); // 37

    bool enumSuccess = itemsCollidingWithTargetRect.Enumerate([&](GUID itemID) -> bool
    {
        Geometry::CRect itemBounds;
        HRESULT hr = m_cellArrayManager->GetItemBounds(itemID, itemBounds);
        if (SUCCEEDED(hr))
            pushDownAmount = max(pushDownAmount, targetRect.bottom - itemBounds.top);
        return SUCCEEDED(hr);
    });
    RETURN_HR_IF(E_FAIL, !enumSuccess); // 47

    if (pushDownAmount != 0)
    {
        RETURN_IF_FAILED(m_cellArrayManager->GetItemsInRect(
            Geometry::CRect(arrayDimensions.left, targetRect.top, arrayDimensions.right, arrayDimensions.bottom),
            &itemsToMove)); // 52

        enumSuccess = itemsToMove.Enumerate([&](GUID itemID) -> bool
        {
            return SUCCEEDED(m_cellArrayManager->AddIgnoredItem(itemID));
        });
        RETURN_HR_IF(E_FAIL, !enumSuccess); // 58

        enumSuccess = itemsToMove.Enumerate([&](GUID itemID) -> bool
        {
            Geometry::CRect itemBounds;
            HRESULT hr = m_cellArrayManager->GetItemBounds(itemID, itemBounds);
            if (SUCCEEDED(hr))
            {
                itemBounds.MoveTo(itemBounds.left, itemBounds.top + pushDownAmount);
                hr = m_cellArrayManager->MoveItemUncommitted(itemID, itemBounds);
            }
            return SUCCEEDED(hr);
        });
        RETURN_HR_IF(E_FAIL, !enumSuccess); // 71

        enumSuccess = itemsToMove.Enumerate([&](GUID itemID) -> bool
        {
            return SUCCEEDED(m_cellArrayManager->RemoveIgnoredItem(itemID));
        });
        RETURN_HR_IF(E_FAIL, !enumSuccess); // 77
    }

    return S_OK;
}
