#pragma once

#include "IItemLayoutCollapseHandler.h"

class CItemLayoutCollapseManager
{
public :
    CItemLayoutCollapseManager();

    HRESULT Collapse(
        const Geometry::CRect& sourceCells, const Geometry::CRect& targetCells, ICellArrayManager* cellArrayManager);
    HRESULT AddCollapseHandler(IItemLayoutCollapseHandler* collapseHandler);

private:
    CCoSimpleArray<Microsoft::WRL::ComPtr<IItemLayoutCollapseHandler>> m_collapseHandlers;
};
