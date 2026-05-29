#include "pch.h"

#include "ItemLayoutDisplacement.h"

using namespace Microsoft::WRL;

CItemLayoutDisplacement::CItemLayoutDisplacement()
{
}

HRESULT CItemLayoutDisplacement::DisplaceItemsFromRect(
    const Geometry::CRect& targetRect, const Geometry::CRect& previousRect, ICellArrayManager* cellArrayManager)
{
    HRESULT hr = E_FAIL;

    for (size_t handlerIndex = 0; handlerIndex < m_displacementHandlers.GetSize() && FAILED(hr); ++handlerIndex)
    {
        ComPtr<IItemLayoutDisplacementHandler> handler;
        m_displacementHandlers.GetAt(handlerIndex, handler);
        if (SUCCEEDED(handler->SetCellArray(cellArrayManager)))
        {
            hr = handler->DisplaceItemsFromRect(targetRect, previousRect);
        }
    }

    return hr;
}

HRESULT CItemLayoutDisplacement::AddDisplacementHandler(IItemLayoutDisplacementHandler* displacementHandler)
{
    return m_displacementHandlers.Add(displacementHandler);
}
