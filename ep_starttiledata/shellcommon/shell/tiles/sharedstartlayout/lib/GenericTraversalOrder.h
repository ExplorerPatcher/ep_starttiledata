#pragma once

#include "ItemLayoutResolver.h"

enum LayoutNavigationDirection
{
    LayoutNavigationDirection_Up = 0x0,
    LayoutNavigationDirection_Down = 0x1,
    LayoutNavigationDirection_Left = 0x2,
    LayoutNavigationDirection_Right = 0x3,
};

MIDL_INTERFACE("0999ed40-ced2-4849-b435-e8cfc9b8752c")
ILayoutTraversalOrder : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetAdjacent(const LayoutNavigationDirection direction, const POINT startingCell, POINT* endingCell, GUID* itemID) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFirst(POINT* endingCell, GUID* itemID) = 0;
};

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
