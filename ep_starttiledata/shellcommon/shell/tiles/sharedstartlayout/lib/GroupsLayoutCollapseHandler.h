#pragma once

#include "IItemLayoutCollapseHandler.h"
#include "PathCollapseHandler.h"

class CGroupsLayoutCollapseHandler
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , IItemLayoutCollapseHandler
    >
{
public:
    CGroupsLayoutCollapseHandler();

    HRESULT RuntimeClassInitialize(EXPAND_COLLAPSE_DIRECTION);

    //~ Begin IItemLayoutCollapseHandler Interface
    STDMETHODIMP SetCellArray(ICellArrayManager* cellArrayManager) override;
    STDMETHODIMP Collapse(const Geometry::CRect&, const Geometry::CRect&) override;
    //~ End IItemLayoutCollapseHandler Interface

protected:
    Microsoft::WRL::ComPtr<ICellArrayManager> m_cellArrayManager;

private:
    EXPAND_COLLAPSE_DIRECTION m_expandCollapseDirection;
};
