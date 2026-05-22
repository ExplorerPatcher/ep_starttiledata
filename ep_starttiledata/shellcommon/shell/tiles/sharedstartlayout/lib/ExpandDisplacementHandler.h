#pragma once

#include "ItemLayoutResolver.h"

enum EXPAND_COLLAPSE_DIRECTION
{
    EXPAND_COLLAPSE_DIRECTION_ROW = 0,
    EXPAND_COLLAPSE_DIRECTION_COLUMN = 1
};

class CExpandDisplacementHandler
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , IItemLayoutDisplacementHandler
    >
{
public:
    CExpandDisplacementHandler();
    HRESULT RuntimeClassInitialize(EXPAND_COLLAPSE_DIRECTION);

    //~ Begin IItemLayoutDisplacementHandler Interface
    STDMETHODIMP SetCellArray(ICellArrayManager* cellArrayManager) override;
    STDMETHODIMP DisplaceItemsFromRect(const Geometry::CRect& targetRect, const Geometry::CRect& previousRect) override;
    //~ End IItemLayoutDisplacementHandler Interface

private:
    ICellArrayManager* m_cellArrayManager;
    EXPAND_COLLAPSE_DIRECTION m_rowOrColumn;
};
