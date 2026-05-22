#include "pch.h"

#include "GroupsLayoutResolverCallbackListener.h"

CGroupsLayoutResolverCallbackListener::CGroupsLayoutResolverCallbackListener()
{
    // TODO: Implement this function
}

HRESULT CGroupsLayoutResolverCallbackListener::RuntimeClassInitialize(REFGUID groupID, IItemLayoutResolver* resolver)
{
    // TODO: Implement this function
}

HRESULT CGroupsLayoutResolverCallbackListener::RegisterCallback(
    IGroupBoundsChangeNotification* groupBoundsChangeNotificationCallback)
{
    // TODO: Implement this function
}

HRESULT CGroupsLayoutResolverCallbackListener::UnregisterCallback(
    IGroupBoundsChangeNotification* groupBoundsChangeNotificationCallback)
{
    // TODO: Implement this function
}

void CGroupsLayoutResolverCallbackListener::NewItemAddedBegin()
{
    // TODO: Implement this function
}

void CGroupsLayoutResolverCallbackListener::NewItemAddedEnd()
{
    // TODO: Implement this function
}

void CGroupsLayoutResolverCallbackListener::OnItemsMigrated(IItemLayoutResolver* pDestinationLayoutResolver)
{
    // TODO: Implement this function
}

void CGroupsLayoutResolverCallbackListener::ItemBoundsUpdated(const GUID*, const RECT*, const UINT)
{
    // TODO: Implement this function
}

void CGroupsLayoutResolverCallbackListener::LayoutBoundsUpdated(const RECT)
{
    // TODO: Implement this function
}

void CGroupsLayoutResolverCallbackListener::ItemRemovedPending(REFGUID itemID)
{
    // TODO: Implement this function
}

void CGroupsLayoutResolverCallbackListener::ItemRemoved(REFGUID itemID)
{
    // TODO: Implement this function
}

void CGroupsLayoutResolverCallbackListener::LastItemRemovedPending()
{
    // TODO: Implement this function
}

void CGroupsLayoutResolverCallbackListener::LastItemRemoved()
{
    // TODO: Implement this function
}

HRESULT CGroupsLayoutResolverCallbackListener::_RemoveCallback(
    IGroupBoundsChangeNotification* groupBoundsChangeNotificationCallback)
{
    // TODO: Implement this function
}

void CGroupsLayoutResolverCallbackListener::_NotifyNewItemAddedBegin()
{
    // TODO: Implement this function
}

void CGroupsLayoutResolverCallbackListener::_NotifyNewItemAddedEnd()
{
    // TODO: Implement this function
}

void CGroupsLayoutResolverCallbackListener::_NotifyItemsMigrated(IItemLayoutResolver* pDestinationLayoutResolver)
{
    // TODO: Implement this function
}

void CGroupsLayoutResolverCallbackListener::_NotifyLayoutBoundsChanged()
{
    // TODO: Implement this function
}

void CGroupsLayoutResolverCallbackListener::_NotifyGroupEmptiedPending()
{
    // TODO: Implement this function
}

void CGroupsLayoutResolverCallbackListener::_NotifyGroupEmptied()
{
    // TODO: Implement this function
}

void CGroupsLayoutResolverCallbackListener::_ProcessPendingUnregisterCallbackCalls()
{
    // TODO: Implement this function
}
