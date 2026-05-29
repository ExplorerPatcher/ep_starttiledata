#include "pch.h"

#include "GroupsLayoutResolverCallbackListener.h"

using namespace Microsoft::WRL;

CGroupsLayoutResolverCallbackListener::CGroupsLayoutResolverCallbackListener()
{
}

HRESULT CGroupsLayoutResolverCallbackListener::RuntimeClassInitialize(REFGUID groupID, IItemLayoutResolver* resolver)
{
    m_iteratingOverCallbacks = false;
    m_groupID = groupID;
    m_resolver = resolver;

    HRESULT hr = m_resolver->RegisterCallback(this);
    if (SUCCEEDED(hr))
    {
        hr = m_resolver->RegisterInternalCallback(this);
    }

    return hr;
}

HRESULT CGroupsLayoutResolverCallbackListener::RegisterCallback(
    IGroupBoundsChangeNotification* groupBoundsChangeNotificationCallback)
{
    return m_callbacks.AddItem(groupBoundsChangeNotificationCallback, groupBoundsChangeNotificationCallback);
}

HRESULT CGroupsLayoutResolverCallbackListener::UnregisterCallback(
    IGroupBoundsChangeNotification* groupBoundsChangeNotificationCallback)
{
    HRESULT hr;

    if (m_iteratingOverCallbacks)
    {
        hr = m_resolversToUnregister.Add(groupBoundsChangeNotificationCallback);
    }
    else
    {
        hr = _RemoveCallback(groupBoundsChangeNotificationCallback);
    }

    return hr;
}

void CGroupsLayoutResolverCallbackListener::NewItemAddedBegin()
{
    _NotifyNewItemAddedBegin();
    _ProcessPendingUnregisterCallbackCalls();
}

void CGroupsLayoutResolverCallbackListener::NewItemAddedEnd()
{
    _NotifyNewItemAddedEnd();
    _ProcessPendingUnregisterCallbackCalls();
}

void CGroupsLayoutResolverCallbackListener::OnItemsMigrated(IItemLayoutResolver* pDestinationLayoutResolver)
{
    _NotifyItemsMigrated(pDestinationLayoutResolver);
    _ProcessPendingUnregisterCallbackCalls();
}

void CGroupsLayoutResolverCallbackListener::ItemBoundsUpdated(const GUID*, const RECT*, const UINT)
{
    // No-op
}

void CGroupsLayoutResolverCallbackListener::LayoutBoundsUpdated(const RECT)
{
    _NotifyLayoutBoundsChanged();
    _ProcessPendingUnregisterCallbackCalls();
}

void CGroupsLayoutResolverCallbackListener::ItemRemovedPending(REFGUID itemID)
{
    // No-op
}

void CGroupsLayoutResolverCallbackListener::ItemRemoved(REFGUID itemID)
{
    // No-op
}

void CGroupsLayoutResolverCallbackListener::LastItemRemovedPending()
{
    _NotifyGroupEmptiedPending();
    _ProcessPendingUnregisterCallbackCalls();
}

void CGroupsLayoutResolverCallbackListener::LastItemRemoved()
{
    _NotifyGroupEmptied();
}

HRESULT CGroupsLayoutResolverCallbackListener::_RemoveCallback(
    IGroupBoundsChangeNotification* groupBoundsChangeNotificationCallback)
{
    HRESULT hr = m_callbacks.DeleteItem(groupBoundsChangeNotificationCallback);
    if (SUCCEEDED(hr) && m_callbacks.GetItemCount() == 0)
    {
        hr = m_resolver->UnregisterCallback(this);
        if (SUCCEEDED(hr))
        {
            hr = m_resolver->UnregisterInternalCallback(this);
        }
    }

    return hr;
}

void CGroupsLayoutResolverCallbackListener::_NotifyNewItemAddedBegin()
{
    m_iteratingOverCallbacks = true;
    (void)m_callbacks.Enum([](IGroupBoundsChangeNotification*, const ComPtr<IGroupBoundsChangeNotification>& callback) -> BOOL
    {
        callback->NewItemAddedBegin();
        return TRUE;
    });
    m_iteratingOverCallbacks = false;
}

void CGroupsLayoutResolverCallbackListener::_NotifyNewItemAddedEnd()
{
    m_iteratingOverCallbacks = true;
    (void)m_callbacks.Enum([](IGroupBoundsChangeNotification*, const ComPtr<IGroupBoundsChangeNotification>& callback) -> BOOL
    {
        callback->NewItemAddedEnd();
        return TRUE;
    });
    m_iteratingOverCallbacks = false;
}

void CGroupsLayoutResolverCallbackListener::_NotifyItemsMigrated(IItemLayoutResolver* pDestinationLayoutResolver)
{
    m_iteratingOverCallbacks = true;
    (void)m_callbacks.Enum([&](IGroupBoundsChangeNotification*, const ComPtr<IGroupBoundsChangeNotification>& callback) -> BOOL
    {
        callback->OnItemsMigrated(pDestinationLayoutResolver, m_groupID);
        return TRUE;
    });
    m_iteratingOverCallbacks = false;
}

void CGroupsLayoutResolverCallbackListener::_NotifyLayoutBoundsChanged()
{
    m_iteratingOverCallbacks = true;
    (void)m_callbacks.Enum([&](IGroupBoundsChangeNotification*, const ComPtr<IGroupBoundsChangeNotification>& callback) -> BOOL
    {
        callback->GroupBoundsChanged(m_groupID);
        return TRUE;
    });
    m_iteratingOverCallbacks = false;
}

void CGroupsLayoutResolverCallbackListener::_NotifyGroupEmptiedPending()
{
    m_iteratingOverCallbacks = true;
    (void)m_callbacks.Enum([&](IGroupBoundsChangeNotification*, const ComPtr<IGroupBoundsChangeNotification>& callback) -> BOOL
    {
        callback->GroupEmptiedPending(m_groupID);
        return TRUE;
    });
    m_iteratingOverCallbacks = false;
}

void CGroupsLayoutResolverCallbackListener::_NotifyGroupEmptied()
{
    m_iteratingOverCallbacks = true;
    (void)m_callbacks.Enum([&](IGroupBoundsChangeNotification*, const ComPtr<IGroupBoundsChangeNotification>& callback) -> BOOL
    {
        callback->GroupEmptied();
        return TRUE;
    });
    m_iteratingOverCallbacks = false;
}

void CGroupsLayoutResolverCallbackListener::_ProcessPendingUnregisterCallbackCalls()
{
    for (size_t i = 0; i < m_resolversToUnregister.GetSize(); ++i)
    {
        _RemoveCallback(m_resolversToUnregister[i]);
    }
    m_resolversToUnregister.RemoveAll();
}
