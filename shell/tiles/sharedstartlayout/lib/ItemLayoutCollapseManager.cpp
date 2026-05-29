#include "pch.h"

#include "ItemLayoutCollapseManager.h"

using namespace Microsoft::WRL;

CItemLayoutCollapseManager::CItemLayoutCollapseManager()
{
}

HRESULT CItemLayoutCollapseManager::Collapse(
    const Geometry::CRect& sourceCells, const Geometry::CRect& targetCells, ICellArrayManager* cellArrayManager)
{
    HRESULT hr = E_FAIL;

    for (size_t handlerIndex = 0; handlerIndex < m_collapseHandlers.GetSize() && FAILED(hr); ++handlerIndex)
    {
        ComPtr<IItemLayoutCollapseHandler> collapseHandler;
        m_collapseHandlers.GetAt(handlerIndex, collapseHandler);
        if (SUCCEEDED(collapseHandler->SetCellArray(cellArrayManager)))
        {
            hr = collapseHandler->Collapse(sourceCells, targetCells);
        }
    }

    return hr;
}

HRESULT CItemLayoutCollapseManager::AddCollapseHandler(IItemLayoutCollapseHandler* collapseHandler)
{
    return m_collapseHandlers.Add(collapseHandler);
}
