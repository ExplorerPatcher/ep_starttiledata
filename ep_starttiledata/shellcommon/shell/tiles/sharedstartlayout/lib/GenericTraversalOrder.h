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
    virtual HRESULT STDMETHODCALLTYPE GetAdjacent(const LayoutNavigationDirection, const POINT, POINT*, GUID*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFirst(POINT*, GUID*) = 0;
};

class CGenericTraversalOrder final
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , ILayoutTraversalOrder
    >
{
public:
    CGenericTraversalOrder();
    HRESULT RuntimeClassInitialize(IItemLayoutResolver* resolver, int maxGroupWidth);

    //~ Begin ILayoutTraversalOrder Interface
    HRESULT GetAdjacent(const LayoutNavigationDirection, const POINT, POINT*, GUID*) override;
    HRESULT GetFirst(POINT*, GUID*) override;
    //~ End ILayoutTraversalOrder Interface

protected:
    HRESULT GetCurrentItemGroup(
        const POINT& startingCell, GUID* itemID, IItemLayoutResolver** groupResolver, POINT* groupRelativeCell);
    HRESULT GetActualGroupStartCell(REFGUID groupID, POINT* cell);

    Microsoft::WRL::ComPtr<IItemLayoutResolver> m_layoutResolver;
    int m_maxGroupWidth;
};
