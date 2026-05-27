#include "pch.h"

#include "PushDownDisplacementHandler.h"

CPushDownDisplacementHandler::CPushDownDisplacementHandler()
    : m_cellArrayManager(nullptr)
{
}

HRESULT CPushDownDisplacementHandler::RuntimeClassInitialize()
{
    return S_OK; // @Note: Couldn't find a body for this??
}

HRESULT CPushDownDisplacementHandler::SetCellArray(ICellArrayManager* cellArrayManager)
{
    m_cellArrayManager = cellArrayManager;
    return S_OK;
}

HRESULT CPushDownDisplacementHandler::DisplaceItemsFromRect(const Geometry::CRect& targetRect, const Geometry::CRect& previousRect)
{
    Geometry::CRect arrayDimensions = m_cellArrayManager->GetCurrentCellArrayBounds();

    int pushDownAmount = 0;

    CSet<GUID> itemsCollidingWithTargetRect;
    CSet<GUID> itemsToMove;

    RETURN_IF_FAILED(m_cellArrayManager->GetItemsInRect(targetRect, &itemsCollidingWithTargetRect)); // 37

    RETURN_IF_FAILED(itemsCollidingWithTargetRect.Enumerate([this, &pushDownAmount, targetRect](GUID itemID) -> bool // 47
    {
        Geometry::CRect itemBounds;
        HRESULT hr = m_cellArrayManager->GetItemBounds(itemID, itemBounds);
        if (SUCCEEDED(hr))
        {
            if (pushDownAmount <= targetRect.bottom - itemBounds.top )
            {
                pushDownAmount = targetRect.bottom - itemBounds.top;
            }
        }
        return SUCCEEDED(hr);
    }));

    if (pushDownAmount != 0)
    {
        RETURN_IF_FAILED(m_cellArrayManager->GetItemsInRect(
            Geometry::CRect(arrayDimensions.left, targetRect.top, arrayDimensions.right, arrayDimensions.bottom),
            &itemsToMove)); // 52

        RETURN_IF_FAILED(itemsToMove.Enumerate([this](GUID itemID) -> bool // 58
        {
            return SUCCEEDED(m_cellArrayManager->AddIgnoredItem(itemID));
        }));

        RETURN_IF_FAILED(itemsToMove.Enumerate([this, &pushDownAmount](GUID itemID) -> bool // 71
        {
            Geometry::CRect itemBounds;
            HRESULT hr = m_cellArrayManager->GetItemBounds(itemID, itemBounds);
            if (SUCCEEDED(hr))
            {
                // inlined Geometry::CRect::MoveTo()
                itemBounds.bottom += pushDownAmount;
                itemBounds.top += pushDownAmount;
                hr = m_cellArrayManager->MoveItemUncommitted(itemID, itemBounds);
            }
            return SUCCEEDED(hr);
        }));

        RETURN_IF_FAILED(itemsToMove.Enumerate([this](GUID itemID) -> bool // 77
        {
            return SUCCEEDED(m_cellArrayManager->RemoveIgnoredItem(itemID));
        }));
    }

    return S_OK;
}
