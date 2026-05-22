#include "pch.h"

#include "EmptyCellDisplacementHandler.h"

CEmptyCellDisplacementHandler::CEmptyCellDisplacementHandler()
    : m_cellArrayManager(nullptr)
{
}

HRESULT CEmptyCellDisplacementHandler::SetCellArray(ICellArrayManager* cellArrayManager)
{
    m_cellArrayManager = cellArrayManager;
    return S_OK;
}

HRESULT CEmptyCellDisplacementHandler::DisplaceItemsFromRect(
    const Geometry::CRect& targetRect, const Geometry::CRect& previousRect)
{
    if (m_cellArrayManager->IsValidRect(targetRect) && m_cellArrayManager->IsRectEmpty(targetRect))
    {
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}
