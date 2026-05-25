#pragma once

#include "ICellArrayManager.h"

MIDL_INTERFACE("d364071e-0afe-427c-a397-e9e1f734d4dd")
IItemLayoutDisplacementHandler : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE SetCellArray(ICellArrayManager* cellArrayManager) = 0;
    virtual HRESULT STDMETHODCALLTYPE DisplaceItemsFromRect(const Geometry::CRect& targetRect, const Geometry::CRect& previousRect) = 0;
};

class CItemLayoutDisplacement
{
public:
    CItemLayoutDisplacement();

    HRESULT DisplaceItemsFromRect(
        const Geometry::CRect& targetRect, const Geometry::CRect& previousRect, ICellArrayManager* cellArrayManager);
    HRESULT AddDisplacementHandler(IItemLayoutDisplacementHandler* displacementHandler);

private:
    CCoSimpleArray<Microsoft::WRL::ComPtr<IItemLayoutDisplacementHandler>> m_displacementHandlers;
};
