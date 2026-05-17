#include "pch.h"

#include "GenericTraversalOrder.h"

#include <wrl.h>
#include "../../../shell/inc/Geometry.h"

using namespace Microsoft::WRL;

CGenericTraversalOrder::CGenericTraversalOrder()
{
}

HRESULT CGenericTraversalOrder::RuntimeClassInitialize(IItemLayoutResolver* resolver, int maxGroupWidth)
{
    RETURN_HR_IF(E_INVALIDARG, !resolver); // 12
    m_layoutResolver = resolver;
    m_maxGroupWidth = maxGroupWidth;
    return S_OK;
}

HRESULT CGenericTraversalOrder::GetAdjacent(const LayoutNavigationDirection, const POINT, POINT*, GUID*)
{
    return E_NOTIMPL;
}

HRESULT CGenericTraversalOrder::GetFirst(POINT*, GUID*)
{
    return E_NOTIMPL;
}

HRESULT CGenericTraversalOrder::GetCurrentItemGroup(
    const POINT& startingCell, GUID* itemID, IItemLayoutResolver** groupResolver, POINT* groupRelativeCell)
{
    *groupResolver = nullptr;

    Geometry::CRect groupMargins;
    RETURN_IF_FAILED(m_layoutResolver->GetContainerMargins(&groupMargins)); // 22

    m_layoutResolver->GetMaxCellBounds();

    POINT currentGroupsCell;
    currentGroupsCell.y = startingCell.y;
    currentGroupsCell.x = startingCell.x / (groupMargins.left + groupMargins.right + m_maxGroupWidth);

    RETURN_IF_FAILED(m_layoutResolver->GetItemByCell(currentGroupsCell, itemID)); // 30
    RETURN_IF_FAILED(m_layoutResolver->GetLayoutResolverForContainer(*itemID, groupResolver)); // 31

    POINT groupStartCell;
    RETURN_IF_FAILED(GetActualGroupStartCell(*itemID, &groupStartCell)); // 34

    groupRelativeCell->y = startingCell.y - groupStartCell.y;
    groupRelativeCell->x = startingCell.x - groupStartCell.x;
    return S_OK;
}

HRESULT CGenericTraversalOrder::GetActualGroupStartCell(const GUID& groupID, POINT* cell)
{
    RECT targetBounds;
    RETURN_IF_FAILED(m_layoutResolver->GetItemBounds(groupID, &targetBounds)); // 44

    Geometry::CRect groupMargins;
    RETURN_IF_FAILED(m_layoutResolver->GetContainerMargins(&groupMargins)); // 47

    m_layoutResolver->GetMaxCellBounds();
    cell->y = targetBounds.top;
    cell->x = targetBounds.left * (groupMargins.left + groupMargins.right + m_maxGroupWidth);
    return S_OK;
}
