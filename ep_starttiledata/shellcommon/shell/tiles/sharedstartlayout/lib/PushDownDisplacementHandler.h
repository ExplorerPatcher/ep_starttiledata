#pragma once

#include "ICellArrayManager.h"
#include "ItemLayoutResolver.h"

class CPushDownDisplacementHandler
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , IItemLayoutDisplacementHandler
    >
{
public:
    CPushDownDisplacementHandler();

    HRESULT RuntimeClassInitialize();

    //~ Begin IItemLayoutDisplacementHandler Interface
    STDMETHODIMP SetCellArray(ICellArrayManager* cellArrayManager) override;
    STDMETHODIMP DisplaceItemsFromRect(const Geometry::CRect& targetRect, const Geometry::CRect& previousRect) override;
    //~ End IItemLayoutDisplacementHandler Interface

private:
    ICellArrayManager* m_cellArrayManager;
};
