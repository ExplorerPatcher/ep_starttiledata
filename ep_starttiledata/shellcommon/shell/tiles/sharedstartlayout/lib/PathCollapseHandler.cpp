#include "pch.h"

#include "PathCollapseHandler.h"

CPathCollapseHandler::CPathCollapseHandler()
    : m_cellArrayManager(nullptr)
    , m_expandCollapseDirection(EXPAND_COLLAPSE_DIRECTION_ROW)
    , m_smartCollapseEnabled(true)
{
}

HRESULT CPathCollapseHandler::RuntimeClassInitialize(EXPAND_COLLAPSE_DIRECTION expandCollapseDirection)
{
    m_expandCollapseDirection = expandCollapseDirection;
    return S_OK;
}

HRESULT CPathCollapseHandler::SetCellArray(ICellArrayManager* cellArrayManager)
{
    m_cellArrayManager = cellArrayManager;
    return S_OK;
}

HRESULT CPathCollapseHandler::Collapse(const Geometry::CRect& sourceCells, const Geometry::CRect& targetCells)
{
    // TODO: Implement this function
}

void CPathCollapseHandler::EnableSmartCollapse()
{
    m_smartCollapseEnabled = true;
}

void CPathCollapseHandler::DisableSmartCollapse()
{
    m_smartCollapseEnabled = false;
}

HRESULT CPathCollapseHandler::_CollapseByOffset(const Geometry::CRect& bounds, const int offset)
{
    // TODO: Implement this function
}

HRESULT CPathCollapseHandler::_CollapseByPath(const int shiftAmount, CCoSimpleArray<Geometry::CPoint>& path)
{
    // TODO: Implement this function
}

bool CPathCollapseHandler::_FindEmptyPath(
    const int firstAxisStart, const int firstAxisMin, const int firstAxisMax, const int secondAxis,
    const int spread, Geometry::CRect& pathMustInclude, const Geometry::CRect& pathCannotInclude,
    bool haveMetIncludeRequirement, CCoSimpleArray<Geometry::CPoint>& path)
{
    // TODO: Implement this function
}
