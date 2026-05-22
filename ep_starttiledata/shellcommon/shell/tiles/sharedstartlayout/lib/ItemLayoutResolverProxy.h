#pragma once

#include "ItemLayoutResolver.h"

class ItemLayoutResolverProxy
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , ICellArrayManagerCallback
        , IGroupBoundsChangeNotification
    >
{
public:
    ItemLayoutResolverProxy();

    HRESULT RuntimeClassInitialize(CItemLayoutResolver* callback);

    void Reset(CItemLayoutResolver* callback);

    //~ Begin ICellArrayManagerCallback Interface
    STDMETHODIMP_(void) ItemBoundsUpdated(REFGUID itemID, const Geometry::CRect& rcItemBoundsCells) override;
    STDMETHODIMP_(void) CellArrayBoundsUpdated(const Geometry::CRect& rcLayoutBounds) override;
    STDMETHODIMP_(void) ItemRemovedPending(REFGUID itemID) override;
    STDMETHODIMP_(void) ItemRemoved(REFGUID itemID) override;
    //~ End ICellArrayManagerCallback Interface

    //~ Begin IGroupBoundsChangeNotification Interface
    STDMETHODIMP_(void) NewItemAddedBegin() override;
    STDMETHODIMP_(void) NewItemAddedEnd() override;
    STDMETHODIMP_(void) OnItemsMigrated(IItemLayoutResolver* pDestinationLayoutResolver, REFGUID groupID) override;
    STDMETHODIMP GroupBoundsChanged(REFGUID groupID) override;
    STDMETHODIMP_(void) GroupEmptiedPending(REFGUID groupID) override;
    STDMETHODIMP_(void) GroupEmptied() override;
    //~ End IGroupBoundsChangeNotification Interface

private:
    CItemLayoutResolver* m_callbackNoRef;
};

inline ItemLayoutResolverProxy::ItemLayoutResolverProxy()
    : m_callbackNoRef(nullptr)
{
}

inline HRESULT ItemLayoutResolverProxy::RuntimeClassInitialize(CItemLayoutResolver* callback)
{
    m_callbackNoRef = callback;
    return S_OK;
}

inline void ItemLayoutResolverProxy::Reset(CItemLayoutResolver* callback)
{
    m_callbackNoRef = callback;
}

inline void ItemLayoutResolverProxy::ItemBoundsUpdated(REFGUID itemID, const Geometry::CRect& rcItemBoundsCells)
{
    if (m_callbackNoRef)
        m_callbackNoRef->ItemBoundsUpdated(itemID, rcItemBoundsCells);
}

inline void ItemLayoutResolverProxy::CellArrayBoundsUpdated(const Geometry::CRect& rcLayoutBounds)
{
    if (m_callbackNoRef)
        m_callbackNoRef->CellArrayBoundsUpdated(rcLayoutBounds);
}

inline void ItemLayoutResolverProxy::ItemRemovedPending(REFGUID itemID)
{
    if (m_callbackNoRef)
        m_callbackNoRef->ItemRemovedPending(itemID);
}

inline void ItemLayoutResolverProxy::ItemRemoved(REFGUID itemID)
{
    if (m_callbackNoRef)
        m_callbackNoRef->ItemRemoved(itemID);
}

inline void ItemLayoutResolverProxy::NewItemAddedBegin()
{
    if (m_callbackNoRef)
        m_callbackNoRef->NewItemAddedBegin();
}

inline void ItemLayoutResolverProxy::NewItemAddedEnd()
{
    if (m_callbackNoRef)
        m_callbackNoRef->NewItemAddedEnd();
}

inline void ItemLayoutResolverProxy::OnItemsMigrated(IItemLayoutResolver* pDestinationLayoutResolver, REFGUID groupID)
{
    if (m_callbackNoRef)
        m_callbackNoRef->OnItemsMigrated(pDestinationLayoutResolver, groupID);
}

inline HRESULT ItemLayoutResolverProxy::GroupBoundsChanged(REFGUID groupID)
{
    if (m_callbackNoRef)
        return m_callbackNoRef->GroupBoundsChanged(groupID);
    else
        return S_OK;
}

inline void ItemLayoutResolverProxy::GroupEmptiedPending(REFGUID groupID)
{
    if (m_callbackNoRef)
        m_callbackNoRef->GroupEmptiedPending(groupID);
}

inline void ItemLayoutResolverProxy::GroupEmptied()
{
    if (m_callbackNoRef)
        m_callbackNoRef->GroupEmptied();
}
