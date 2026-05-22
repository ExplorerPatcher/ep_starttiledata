#pragma once

#include "ItemLayoutResolver.h"

enum EXPAND_COLLAPSE_DIRECTION
{
    EXPAND_COLLAPSE_DIRECTION_ROW = 0,
    EXPAND_COLLAPSE_DIRECTION_COLUMN = 1,
};

class CPathCollapseHandler
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , IItemLayoutCollapseHandler
    >
{
public:
    CPathCollapseHandler();

    HRESULT RuntimeClassInitialize(EXPAND_COLLAPSE_DIRECTION expandCollapseDirection);

    //~ Begin IItemLayoutCollapseHandler Interface
    STDMETHODIMP SetCellArray(ICellArrayManager* cellArrayManager) override;
    STDMETHODIMP Collapse(const Geometry::CRect& sourceCells, const Geometry::CRect& targetCells) override;
    //~ End IItemLayoutCollapseHandler Interface

    void EnableSmartCollapse();
    void DisableSmartCollapse();

protected:
    ICellArrayManager* m_cellArrayManager;

private:
    HRESULT _CollapseByOffset(const Geometry::CRect& bounds, const int offset);
    HRESULT _CollapseByPath(const int shiftAmount, CCoSimpleArray<Geometry::CPoint>& path);
    bool _FindEmptyPath(
        const int firstAxisStart, const int firstAxisMin, const int firstAxisMax, const int secondAxis,
        const int spread, Geometry::CRect& pathMustInclude, const Geometry::CRect& pathCannotInclude,
        bool haveMetIncludeRequirement, CCoSimpleArray<Geometry::CPoint>& path);

    EXPAND_COLLAPSE_DIRECTION m_expandCollapseDirection;
    bool m_smartCollapseEnabled;
};
