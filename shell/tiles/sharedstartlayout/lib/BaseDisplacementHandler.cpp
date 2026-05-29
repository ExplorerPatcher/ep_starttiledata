#include "pch.h"

#include "BaseDisplacementHandler.h"

HRESULT CBaseDisplacementHandler::SetCellArray(ICellArrayManager* cellArrayManager)
{
    m_cellArrayManager = cellArrayManager;
    return S_OK;
}

HRESULT CBaseDisplacementHandler::DisplaceItemsFromRect(const Geometry::CRect& targetRect, const Geometry::CRect& previousRect)
{
    return E_FAIL;
}

Geometry::CRect CBaseDisplacementHandler::_GetAdjacentRect(
    const Geometry::CRect& displaceTargetRect, const Geometry::CRect& displaceBoundingRect,
    DISPLACEMENT_DIRECTION direction) const
{
    Geometry::CRect result;
    switch (direction)
    {
        case DD_UP:
            result.left = displaceBoundingRect.left;
            result.top = result.bottom = displaceBoundingRect.top - displaceBoundingRect.bottom + displaceTargetRect.top;
            break;

        case DD_DOWN:
            result.left = displaceBoundingRect.left;
            result.top = result.bottom = displaceTargetRect.bottom;
            break;

        case DD_LEFT:
            result.left = displaceBoundingRect.left - displaceBoundingRect.right + displaceTargetRect.left;
            result.top = result.bottom = displaceBoundingRect.top;
            break;

        case DD_RIGHT:
            result.left = displaceTargetRect.right;
            result.top = result.bottom = displaceBoundingRect.top;
            break;
    }

    result.right = result.left + displaceBoundingRect.right - displaceBoundingRect.left;
    result.bottom = result.bottom + displaceBoundingRect.bottom - displaceBoundingRect.top;
    return result;
}
