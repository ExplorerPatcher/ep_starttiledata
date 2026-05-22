#pragma once

#include "ItemLayoutResolver.h"

class CGroupsLayoutResolverCallbackListener
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , IItemLayoutResolverInternalCallback
        , IItemLayoutResolverCallback
    >
{
public:
    CGroupsLayoutResolverCallbackListener();

    HRESULT RuntimeClassInitialize(REFGUID groupID, IItemLayoutResolver* resolver);

    HRESULT RegisterCallback(IGroupBoundsChangeNotification* groupBoundsChangeNotificationCallback);
    HRESULT UnregisterCallback(IGroupBoundsChangeNotification* groupBoundsChangeNotificationCallback);

    //~ Begin IItemLayoutResolverInternalCallback Interface
    STDMETHODIMP_(void) NewItemAddedBegin() override;
    STDMETHODIMP_(void) NewItemAddedEnd() override;
    STDMETHODIMP_(void) OnItemsMigrated(IItemLayoutResolver* pDestinationLayoutResolver) override;
    //~ End IItemLayoutResolverInternalCallback Interface

    //~ Begin IItemLayoutResolverCallback Interface
    STDMETHODIMP_(void) ItemBoundsUpdated(const GUID*, const RECT*, const UINT) override;
    STDMETHODIMP_(void) LayoutBoundsUpdated(const RECT) override;
    STDMETHODIMP_(void) ItemRemovedPending(REFGUID itemID) override;
    STDMETHODIMP_(void) ItemRemoved(REFGUID itemID) override;
    STDMETHODIMP_(void) LastItemRemovedPending() override;
    STDMETHODIMP_(void) LastItemRemoved() override;
    //~ End IItemLayoutResolverCallback Interface

private:
    HRESULT _RemoveCallback(IGroupBoundsChangeNotification* groupBoundsChangeNotificationCallback);
    void _NotifyNewItemAddedBegin();
    void _NotifyNewItemAddedEnd();
    void _NotifyItemsMigrated(IItemLayoutResolver* pDestinationLayoutResolver);
    void _NotifyLayoutBoundsChanged();
    void _NotifyGroupEmptiedPending();
    void _NotifyGroupEmptied();
    void _ProcessPendingUnregisterCallbackCalls();

    bool m_iteratingOverCallbacks;
    GUID m_groupID;
    Microsoft::WRL::ComPtr<IItemLayoutResolver> m_resolver;
    CCoSimpleArray<IGroupBoundsChangeNotification*> m_resolversToUnregister;
    CSimpleHashTable<IGroupBoundsChangeNotification*, Microsoft::WRL::ComPtr<IGroupBoundsChangeNotification>> m_callbacks;
};
