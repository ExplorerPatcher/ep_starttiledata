#pragma once

#include "ItemLayoutResolver.h"
#include "PathCollapseHandler.h"

class CExpandDisplacementHandler
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , IItemLayoutDisplacementHandler
    >
{
public:
    CExpandDisplacementHandler();

    HRESULT RuntimeClassInitialize(EXPAND_COLLAPSE_DIRECTION rowOrColumn);

    //~ Begin IItemLayoutDisplacementHandler Interface
    STDMETHODIMP SetCellArray(ICellArrayManager* cellArrayManager) override;
    STDMETHODIMP DisplaceItemsFromRect(const Geometry::CRect& targetRect, const Geometry::CRect& previousRect) override;
    //~ End IItemLayoutDisplacementHandler Interface

private:
    ICellArrayManager* m_cellArrayManager;
    EXPAND_COLLAPSE_DIRECTION m_rowOrColumn;
};
