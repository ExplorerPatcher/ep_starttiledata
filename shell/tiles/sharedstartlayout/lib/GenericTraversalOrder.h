#pragma once

#include "ItemLayoutResolver.h"

class CGenericTraversalOrder
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , ILayoutTraversalOrder
    >
{
public:
    CGenericTraversalOrder();

    HRESULT RuntimeClassInitialize(IItemLayoutResolver* resolver, int maxGroupWidth);

    //~ Begin ILayoutTraversalOrder Interface
    STDMETHODIMP GetAdjacent(
        const LayoutNavigationDirection direction, const POINT startingCell, POINT* endingCell, GUID* itemID) override;
    STDMETHODIMP GetFirst(POINT* endingCell, GUID* itemID) override;
    //~ End ILayoutTraversalOrder Interface

protected:
    HRESULT GetCurrentItemGroup(
        const POINT& startingCell, GUID* itemID, IItemLayoutResolver** groupResolver, POINT* groupRelativeCell);
    HRESULT GetActualGroupStartCell(REFGUID groupID, POINT* cell);

    Microsoft::WRL::ComPtr<IItemLayoutResolver> m_layoutResolver;
    int m_maxGroupWidth;
};
