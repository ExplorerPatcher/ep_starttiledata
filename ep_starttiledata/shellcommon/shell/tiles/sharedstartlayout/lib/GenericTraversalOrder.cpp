#include "pch.h"

#include "GenericTraversalOrder.h"

#include <wrl.h>

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

}