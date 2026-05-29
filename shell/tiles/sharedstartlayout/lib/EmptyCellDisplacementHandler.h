#pragma once

#include "ItemLayoutResolver.h"

class CEmptyCellDisplacementHandler
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , IItemLayoutDisplacementHandler
    >
{
public:
    CEmptyCellDisplacementHandler();

    //~ Begin IItemLayoutDisplacementHandler Interface
    STDMETHODIMP SetCellArray(ICellArrayManager* cellArrayManager) override;
    STDMETHODIMP DisplaceItemsFromRect(const Geometry::CRect& targetRect, const Geometry::CRect& previousRect) override;
    //~ End IItemLayoutDisplacementHandler Interface

private:
    ICellArrayManager* m_cellArrayManager;
};
