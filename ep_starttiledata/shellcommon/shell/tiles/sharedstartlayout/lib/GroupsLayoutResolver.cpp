#include "pch.h"

#include "GroupsLayoutResolver.h"

#include "AdjacentDisplacementHandler.h"
#include "EmptyCellDisplacementHandler.h"
#include "EmptyColumnCollapseMigrationHandler.h"
#include "ExpandDisplacementHandler.h"
#include "GroupsLayoutCollapseHandler.h"
#include "GroupsLayoutResolverCallbackListener.h"
#include "ItemLayoutChainDisplacement.h"
#include "ItemLayoutResolverProxy.h"
#include "RefCountedObject.h"

using namespace Microsoft::WRL;

inline constexpr GUID c_emptyCellValue = {};

CGroupsLayoutResolver::CGroupsLayoutResolver()
    : c_groupWidth(1)
    , m_newItemBeingAdded(false)
{
}

CGroupsLayoutResolver::~CGroupsLayoutResolver()
{
    m_groupResolvers.Enum([this](GUID key, GroupResolverInternal& value) -> BOOL
    {
        LOG_IF_FAILED(_UnregisterFromCallback(key));
        return TRUE;
    });
}

HRESULT CGroupsLayoutResolver::RuntimeClassInitialize()
{
    CItemLayoutResolver::RuntimeClassInitialize();

    m_newItemBeingAdded = false;
    m_pendingRemovedGroup = c_emptyCellValue;
    m_containerMargins = {};

    RETURN_IF_FAILED(_displacementManager.AddDisplacementHandler(Make<CEmptyCellDisplacementHandler>().Get())); // 35

    CCoSimpleArray<DISPLACEMENT_DIRECTION> directionPriorities;
    //
    //
    RETURN_IF_FAILED(directionPriorities.Add(DD_UP)); // 40
    RETURN_IF_FAILED(directionPriorities.Add(DD_DOWN)); // 41

    //
    ComPtr<CAdjacentDisplacementHandler> adjacentDisplacementHandler;
    RETURN_IF_FAILED(MakeAndInitialize<CAdjacentDisplacementHandler>(
        &adjacentDisplacementHandler, directionPriorities,
        STAOF_PREFER_SHORTEST_DISPLACEMENT | STAOF_DISPLACE_BLOCK_IS_CONTAINED_IN_TARGET)); // 47
    RETURN_IF_FAILED(_displacementManager.AddDisplacementHandler(adjacentDisplacementHandler.Get())); // 48

    CSimpleHashTable<UINT, RefCountedDirectionArray> chainDisplacementDirectionPriorities;
    RefCountedDirectionArray firstDirectionPriorities = CreateRefCountedObj<CCoSimpleArray<DISPLACEMENT_DIRECTION>>();
    RETURN_IF_FAILED(firstDirectionPriorities->Add(DD_UP)); // 54
    RETURN_IF_FAILED(firstDirectionPriorities->Add(DD_DOWN)); // 55
    RETURN_IF_FAILED(chainDisplacementDirectionPriorities.AddItem(0, firstDirectionPriorities)); // 56

    //
    ComPtr<CItemLayoutChainDisplacement> chainDisplacementHandler;
    RETURN_IF_FAILED(MakeAndInitialize<CItemLayoutChainDisplacement>(&chainDisplacementHandler, &chainDisplacementDirectionPriorities, 0, CDOF_NONE)); // 60
    RETURN_IF_FAILED(_displacementManager.AddDisplacementHandler(chainDisplacementHandler.Get())); // 61

    ComPtr<CExpandDisplacementHandler> expandDisplacementHandler;
    RETURN_IF_FAILED(MakeAndInitialize<CExpandDisplacementHandler>(&expandDisplacementHandler, EXPAND_COLLAPSE_DIRECTION_ROW)); // 64
    RETURN_IF_FAILED(_displacementManager.AddDisplacementHandler(expandDisplacementHandler.Get())); // 65

    ComPtr<CGroupsLayoutCollapseHandler> spGroupsCollapseHandler;
    RETURN_IF_FAILED(MakeAndInitialize<CGroupsLayoutCollapseHandler>(&spGroupsCollapseHandler, EXPAND_COLLAPSE_DIRECTION_ROW)); // 68
    RETURN_IF_FAILED(_collapseManager.AddCollapseHandler(spGroupsCollapseHandler.Get())); // 69

    return S_OK;
}

HRESULT CGroupsLayoutResolver::ResizeItemUncommitted(REFGUID itemID, const SIZE sizeItemCells)
{
    return E_NOTIMPL;
}

HRESULT CGroupsLayoutResolver::AddNewContainer(REFGUID containerID, IItemLayoutResolver* resolver)
{
    Geometry::CSize groupSize;
    HRESULT hr = _GetGroupSizeWithMarginFromLayoutResolver(resolver, &groupSize);
    if (SUCCEEDED(hr))
    {
        groupSize.cx = max(m_containerMargins.right - m_containerMargins.left, groupSize.cx);
        groupSize.cy = max(m_containerMargins.bottom - m_containerMargins.top, groupSize.cy);
        hr = AddNewItem(containerID, groupSize);
        if (SUCCEEDED(hr))
        {
            hr = _RegisterForCallbacksWithSubresolver(resolver, containerID);
        }
    }

    return hr;
}

HRESULT CGroupsLayoutResolver::AddContainer(REFGUID containerID, IItemLayoutResolver* resolver, const POINT destination)
{
    Geometry::CRect groupRect;
    HRESULT hr = _PositionGroupAtPoint(resolver, destination, &groupRect);
    if (SUCCEEDED(hr))
    {
        hr = AddItem(containerID, groupRect);
        if (SUCCEEDED(hr))
        {
            hr = _RegisterForCallbacksWithSubresolver(resolver, containerID);
        }
    }

    return hr;
}

HRESULT CGroupsLayoutResolver::AddSizedContainer(
    REFGUID containerID, IItemLayoutResolver* pResolver, const RECT destination)
{
    return E_NOTIMPL;
}

HRESULT CGroupsLayoutResolver::InsertContainerUncommitted(
    REFGUID containerID, IItemLayoutResolver* resolver, const POINT destination)
{
    Geometry::CRect groupRect;
    HRESULT hr = _PositionGroupAtPoint(resolver, destination, &groupRect);
    if (SUCCEEDED(hr))
    {
        hr = InsertItemUncommitted(containerID, groupRect);
        if (SUCCEEDED(hr))
        {
            hr = _RegisterForCallbacksWithSubresolver(resolver, containerID);
        }
    }

    return hr;
}

HRESULT CGroupsLayoutResolver::GetLayoutResolverForContainer(REFGUID containerID, IItemLayoutResolver** ppResolver)
{
    *ppResolver = nullptr;

    GroupResolverInternal groupResolverData;
    RETURN_IF_FAILED(m_groupResolvers.GetItem(containerID, groupResolverData)); // 145
    return groupResolverData.resolver.CopyTo(ppResolver);
}

HRESULT CGroupsLayoutResolver::RemoveItemUncommitted(REFGUID itemID)
{
    HRESULT hr = CItemLayoutResolver::RemoveItemUncommitted(itemID);
    if (SUCCEEDED(hr))
    {
        m_pendingRemovedGroup = itemID;
    }
    else if (hr == TYPE_E_ELEMENTNOTFOUND)
    {
        hr = _UnregisterFromCallback(itemID);
    }

    return hr;
}

HRESULT CGroupsLayoutResolver::SetContainerMargins(const RECT containerMargins)
{
    m_containerMargins = containerMargins;
    return S_OK;
}

HRESULT CGroupsLayoutResolver::GetContainerMargins(RECT* containerMargins)
{
    *containerMargins = m_containerMargins;
    return S_OK;
}

HRESULT CGroupsLayoutResolver::MigrateItems(
    IItemLayoutResolver* pSourceLayout, const LayoutMigrationOptions migrationOptions)
{
    switch (migrationOptions.migrationType)
    {
        case LayoutMigrationType_ColumnChange:
        {
            ComPtr<CEmptyColumnCollapseMigrationHandler> migrationHandler;
            MakeAndInitialize<CEmptyColumnCollapseMigrationHandler>(&migrationHandler);

            CGroupsLayoutResolver* pSourceGroupsLayoutResolver = static_cast<CGroupsLayoutResolver*>(pSourceLayout);

            if (SUCCEEDED(migrationHandler->MigrateItems(
                pSourceGroupsLayoutResolver->_spCellArrayManager.Get(), _spCellArrayManager.Get(), _GetCellAssignor())))
            {
                EnumerateGroupsInLayoutOrder(pSourceGroupsLayoutResolver->_spCellArrayManager.Get(),
                    [&](REFGUID groupID) -> HRESULT
                    {
                        ComPtr<IItemLayoutResolver> groupResolver;
                        RETURN_IF_FAILED(pSourceGroupsLayoutResolver->GetLayoutResolverForContainer(groupID, &groupResolver)); // 242
                        RETURN_IF_FAILED(_RegisterForCallbacksWithSubresolver(groupResolver.Get(), groupID)); // 243
                        return S_OK;
                    }
                );
                return S_OK;
            }

            ComPtr<IItemLayoutResolver> unassignedGroupResolver;
            EnumerateGroupsInLayoutOrder(pSourceGroupsLayoutResolver->_spCellArrayManager.Get(),
                [&migrationOptions, &pSourceGroupsLayoutResolver, &unassignedGroupResolver, this](REFGUID groupID) -> HRESULT
                {
                    ComPtr<IItemLayoutResolver> groupResolver;
                    if (migrationOptions.unassignedItemId == groupID)
                    {
                        RETURN_IF_FAILED(pSourceGroupsLayoutResolver->GetLayoutResolverForContainer(groupID, &unassignedGroupResolver)); // 257
                    }
                    else
                    {
                        RETURN_IF_FAILED(pSourceGroupsLayoutResolver->GetLayoutResolverForContainer(groupID, &groupResolver)); // 261
                        RETURN_IF_FAILED(AddNewContainer(groupID, groupResolver.Get())); // 262
                    }
                    return S_OK;
                }
            );
            if (unassignedGroupResolver != nullptr)
            {
                RETURN_IF_FAILED(AddNewContainer(migrationOptions.unassignedItemId, unassignedGroupResolver.Get())); // 269
            }
            break;
        }
        default:
        {
            RETURN_HR(E_FAIL); // 275
        }
    }

    return S_OK;
}

HRESULT CGroupsLayoutResolver::GetContainerSizeWithMargins(REFGUID containerID, SIZE* size)
{
    ComPtr<IItemLayoutResolver> groupResolver;
    RETURN_IF_FAILED(GetLayoutResolverForContainer(containerID, &groupResolver)); // 183

    RECT containerBounds;
    RETURN_IF_FAILED(groupResolver->GetLayoutBounds(&containerBounds)); // 186

    size->cx = m_containerMargins.left + groupResolver->GetMaxCellBounds().cx + m_containerMargins.right;
    size->cy = m_containerMargins.top + (containerBounds.bottom - containerBounds.top) + m_containerMargins.bottom;

    return S_OK;
}

HRESULT CGroupsLayoutResolver::CommitChanges()
{
    m_pendingRemovedGroup = c_emptyCellValue;
    return _CommitChangesInternal();
}

void CGroupsLayoutResolver::NewItemAddedBegin()
{
    m_newItemBeingAdded = true;
}

void CGroupsLayoutResolver::NewItemAddedEnd()
{
    m_newItemBeingAdded = false;
}

void CGroupsLayoutResolver::OnItemsMigrated(IItemLayoutResolver* pDestinationLayoutResolver, REFGUID groupID)
{
    LOG_IF_FAILED(m_groupResolvers.DeleteItem(groupID)); // 297
    LOG_IF_FAILED(_RegisterForCallbacksWithSubresolver(pDestinationLayoutResolver, groupID)); // 298
}

HRESULT CGroupsLayoutResolver::GroupBoundsChanged(REFGUID groupID)
{
    RETURN_IF_FAILED_EXPECTED(_EnsureConsistencyWithResolverBounds(groupID));
    return S_OK;
}

void CGroupsLayoutResolver::GroupEmptiedPending(REFGUID groupID)
{
    RemoveItemUncommitted(groupID);
}

void CGroupsLayoutResolver::GroupEmptied()
{
    if (m_pendingRemovedGroup != c_emptyCellValue)
    {
        _UnregisterFromCallback(m_pendingRemovedGroup);
        m_pendingRemovedGroup = c_emptyCellValue;
    }

    CommitChanges();
}

void CGroupsLayoutResolver::ItemRemoved(REFGUID itemID)
{
    CItemLayoutResolver::ItemRemoved(itemID);
    _UnregisterFromCallback(itemID);
}

bool CGroupsLayoutResolver::IsGroupPendingRemoval()
{
    // TODO: Implement this function
}

HRESULT CGroupsLayoutResolver::_FindTargetDestinationForNewSize(
    REFGUID itemId, const SIZE& sizeItemCells, Geometry::CRect* rcItemBounds)
{
    return E_NOTIMPL;
}

IItemCellAssignor* CGroupsLayoutResolver::_GetCellAssignor()
{
    if (m_cellAssignor == nullptr)
    {
        m_cellAssignor = Make<CGroupsCellAssignor>();
        m_cellAssignor->SetCellArray(_spCellArrayManager.Get());
    }
    return m_cellAssignor.Get();
}

HRESULT CGroupsLayoutResolver::_PrepareLayoutBeforeOperation(
    const Geometry::CRect& rcSourceCells, const Geometry::CRect& rcTargetCells)
{
    return _Collapse(Geometry::CRect(), Geometry::CRect());
}

HRESULT CGroupsLayoutResolver::_CleanupLayoutAfterOperation(
    const Geometry::CRect& rcSourceCells, const Geometry::CRect& rcTargetCells)
{
    return _Collapse(Geometry::CRect(), Geometry::CRect());
}

HRESULT CGroupsLayoutResolver::_RepairLayout()
{
    HRESULT hr = _CleanupLayoutAfterOperation(Geometry::CRect(), Geometry::CRect());
    if (hr == S_OK)
    {
        hr = _spCellArrayManager->FixCoordinatesToBeNonNegative();
    }

    return hr;
}

HRESULT CGroupsLayoutResolver::_RegisterForCallbacksWithSubresolver(IItemLayoutResolver* resolver, REFGUID containerID)
{
    ComPtr<CGroupsLayoutResolverCallbackListener> groupLayoutResolverCallback;
    HRESULT hr = MakeAndInitialize<CGroupsLayoutResolverCallbackListener>(&groupLayoutResolverCallback, containerID, resolver);
    if (SUCCEEDED(hr))
    {
        hr = groupLayoutResolverCallback->RegisterCallback(m_itemLayoutResolverProxy.Get());
    }
    if (SUCCEEDED(hr))
    {
        GroupResolverInternal groupResolverInternal = { resolver, groupLayoutResolverCallback.Get() };
        hr = m_groupResolvers.SetItem(containerID, groupResolverInternal);
    }

    return hr;
}

HRESULT CGroupsLayoutResolver::_UnregisterFromCallback(REFGUID groupID)
{
    GroupResolverInternal groupResolverInternal;
    HRESULT hr = m_groupResolvers.GetItem(groupID, groupResolverInternal);
    if (SUCCEEDED(hr))
    {
        ComPtr<CGroupsLayoutResolverCallbackListener> resolverCallback = groupResolverInternal.resolverCallback;
        hr = resolverCallback->UnregisterCallback(m_itemLayoutResolverProxy.Get());
    }
    if (SUCCEEDED(hr))
    {
        hr = m_groupResolvers.DeleteItem(groupID);
    }

    return hr;
}

HRESULT CGroupsLayoutResolver::_GetGroupSizeWithMarginFromLayoutResolver(
    IItemLayoutResolver* resolver, Geometry::CSize* size)
{
    Geometry::CRect containerBounds;
    RETURN_IF_FAILED(resolver->GetLayoutBounds(&containerBounds)); // 419

    size->cx = c_groupWidth;
    size->cy = m_containerMargins.top + (containerBounds.bottom - containerBounds.top) + m_containerMargins.bottom;

    return S_OK;
}

HRESULT CGroupsLayoutResolver::_PositionGroupAtPoint(
    IItemLayoutResolver* resolver, const Geometry::CPoint& destination, Geometry::CRect* rect)
{
    Geometry::CSize containerSize;
    HRESULT hr = _GetGroupSizeWithMarginFromLayoutResolver(resolver, &containerSize);
    if (SUCCEEDED(hr))
    {
        rect->left = destination.x;
        rect->right = rect->left + c_groupWidth;
        rect->top = destination.y;
        rect->bottom = rect->top + containerSize.cy;
    }

    return hr;
}

HRESULT CGroupsLayoutResolver::_GetResolverBoundsWithMargins(REFGUID itemID, Geometry::CRect* bounds)
{
    GroupResolverInternal groupResolverInternal;
    RETURN_IF_FAILED(m_groupResolvers.GetItem(itemID, groupResolverInternal)); // 448

    //

    Geometry::CRect containerBounds;
    RETURN_IF_FAILED(groupResolverInternal.resolver->GetLayoutBounds(&containerBounds)); // 453

    //

    Geometry::CSize containerSize;
    RETURN_IF_FAILED(_GetGroupSizeWithMarginFromLayoutResolver(groupResolverInternal.resolver.Get(), &containerSize)); // 458

    Geometry::CRect currentPosition;
    RETURN_IF_FAILED(_spCellArrayManager->GetItemBounds(itemID, currentPosition)); // 461

    bounds->left = currentPosition.left;
    bounds->right = currentPosition.right;
    bounds->top = currentPosition.top + containerBounds.top;
    bounds->bottom = bounds->top + containerSize.cy;

    return S_OK;
}

HRESULT CGroupsLayoutResolver::_EnsureConsistencyWithResolverBounds(REFGUID groupID)
{
    GroupResolverInternal groupResolverInternal;
    RETURN_IF_FAILED(m_groupResolvers.GetItem(groupID, groupResolverInternal)); // 475

    Geometry::CSize groupSize;
    RETURN_IF_FAILED(_GetGroupSizeWithMarginFromLayoutResolver(groupResolverInternal.resolver.Get(), &groupSize)); // 478

    if (groupSize.cy == m_containerMargins.top + m_containerMargins.bottom && groupResolverInternal.resolver->IsEmpty())
    {
        return E_FAIL;
    }

    if (m_pendingRemovedGroup == groupID || m_newItemBeingAdded)
    {
        RETURN_IF_FAILED(AbandonChanges()); // 496
    }

    Geometry::CRect groupBounds;
    RETURN_IF_FAILED(_GetResolverBoundsWithMargins(groupID, &groupBounds)); // 504
    RETURN_IF_FAILED(_ModifyItemUncommittedInternal(groupID, groupBounds, ModificationOperation::Move)); // 505

    if (m_pendingRemovedGroup == c_emptyCellValue || m_newItemBeingAdded)
    {
        RETURN_IF_FAILED(CommitChanges()); // 512
    }

    return S_OK;
}

template <typename T>
HRESULT CGroupsLayoutResolver::EnumerateGroupsInLayoutOrder(ICellArrayManager* pCellArrayManager, T lambda)
{
    CSet<GUID> setProcessedItems;

    Geometry::CRect currentArrayBounds = pCellArrayManager->GetCurrentCellArrayBounds();
    for (int indexY = currentArrayBounds.top; indexY < currentArrayBounds.bottom; ++indexY)
    {
        for (int indexX = currentArrayBounds.left; indexX < currentArrayBounds.right; ++indexX)
        {
            GUID itemID = pCellArrayManager->GetItemAtCell(indexX, indexY);
            if (itemID != c_emptyCellValue && setProcessedItems.Contains(itemID) != S_OK)
            {
                setProcessedItems.Add(itemID);
                RETURN_IF_FAILED(lambda(itemID)); // 220
            }
        }
    }

    return S_OK;
}
