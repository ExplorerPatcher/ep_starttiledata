#pragma once

#include "ItemLayoutResolver.h"

enum DISPLACEMENT_DIRECTION
{
    DD_UP = 0,
    DD_DOWN = 1,
    DD_LEFT = 2,
    DD_RIGHT = 3,
    DD_MAX = 4,
};

class DECLSPEC_UUID("f56d4a12-5ab3-438f-850d-e633763c12fc")
CBaseDisplacementHandler
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , IItemLayoutDisplacementHandler
    >
{
    //~ Begin IItemLayoutDisplacementHandler Interface
    STDMETHODIMP SetCellArray(ICellArrayManager* cellArrayManager) override;
    STDMETHODIMP DisplaceItemsFromRect(const Geometry::CRect& targetRect, const Geometry::CRect& previousRect) override;
    //~ End IItemLayoutDisplacementHandler Interface

protected:
    ICellArrayManager* m_cellArrayManager;

    Geometry::CRect _GetAdjacentRect(
        const Geometry::CRect& displaceTargetRect, const Geometry::CRect& displaceBoundingRect,
        DISPLACEMENT_DIRECTION direction) const;
};
