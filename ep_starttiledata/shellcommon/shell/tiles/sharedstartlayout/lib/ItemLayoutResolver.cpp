#include "pch.h"

#include "ItemLayoutResolver.h"

#include <wil/common.h>
#include <wil/resource.h>

#include "CellArrayManager.h"
#include "ItemLayoutResolverProxy.h"

using namespace Microsoft::WRL;

// @Note
// wil::ScopeExit is the precursor to wil::scope_exit
// e.g:
// auto cleanupOnFailure = wil::ScopeExit([this] { LOG_IF_FAILED(_StopBatchingItemBoundsChangeUpdatesAndNotify()); });
// is the same as
// auto cleanupOnFailure = wil::scope_exit([this] { LOG_IF_FAILED(_StopBatchingItemBoundsChangeUpdatesAndNotify()); });
// in modern code.
// Example taken from CItemLayoutResolver::ResizeItemUncommitted()

inline constexpr GUID c_emptyCellValue = {};

CItemLayoutResolver::CItemLayoutResolver()
    : _options(LRO_DISPLACE_INTO_NEGATIVE_SPACE)
    , m_isCollapsed(false)
    , m_canCollapse(true)
{
}

CItemLayoutResolver::~CItemLayoutResolver()
{
    if (m_itemLayoutResolverProxy != nullptr)
    {
        if (_spCellArrayManager != nullptr)
        {
            _spCellArrayManager->UnregisterCallback(m_itemLayoutResolverProxy.Get());
        }
        m_itemLayoutResolverProxy->Reset(nullptr);
    }
}

HRESULT CItemLayoutResolver::RuntimeClassInitialize()
{
    RETURN_IF_FAILED(MakeAndInitialize<CCellArrayManager>(&_spCellArrayManager)); // 36

    RETURN_IF_FAILED(MakeAndInitialize<ItemLayoutResolverProxy>(&m_itemLayoutResolverProxy, this)); // 38
    RETURN_IF_FAILED(_spCellArrayManager->RegisterCallback(m_itemLayoutResolverProxy.Get())); // 39

    _isBatchingItemBoundsChangeUpdates = false;
    return S_OK;
}

HRESULT CItemLayoutResolver::RegisterCallback(IItemLayoutResolverCallback* callback)
{
    return _htCallbacks.AddItem(callback, callback);
}

HRESULT CItemLayoutResolver::UnregisterCallback(IItemLayoutResolverCallback* callback)
{
    return _htCallbacks.DeleteItem(callback);
}

HRESULT CItemLayoutResolver::RegisterInternalCallback(IItemLayoutResolverInternalCallback* callback)
{
    return _htInternalCallbacks.AddItem(callback, callback);
}

HRESULT CItemLayoutResolver::UnregisterInternalCallback(IItemLayoutResolverInternalCallback* callback)
{
    return _htInternalCallbacks.DeleteItem(callback);
}

HRESULT CItemLayoutResolver::AddNewItem(REFGUID itemID, const SIZE sizeItemCells)
{
    _NotifyNewItemAddedBegin();

    Geometry::CRect rcItemCellBounds;
    HRESULT hr = _GetCellAssignor()->CalculateLocationForNewItem(sizeItemCells, &rcItemCellBounds);
    if (SUCCEEDED(hr))
    {
        hr = _spCellArrayManager->SetItem(itemID, rcItemCellBounds, SIO_SEND_BOUNDS_UPDATE);
    }

    _NotifyNewItemAddedEnd();
    return hr;
}

HRESULT CItemLayoutResolver::AddItem(REFGUID itemID, const RECT rcItemBoundsCells)
{
    return _spCellArrayManager->SetItem(itemID, rcItemBoundsCells, SIO_NONE);
}

HRESULT CItemLayoutResolver::InsertItemUncommitted(REFGUID itemID, const RECT rcDestination)
{
    auto cleanupOnFailure = wil::scope_exit([this]
    {
        LOG_IF_FAILED(_StopBatchingItemBoundsChangeUpdatesAndNotify()); // 251
    });

    RETURN_IF_FAILED(_StartBatchingItemBoundsChangeUpdates()); // 254
    RETURN_IF_FAILED(_spCellArrayManager->AbandonChanges()); // 255
    RETURN_IF_FAILED(_ModifyItemUncommittedInternal(itemID, rcDestination, ModificationOperation::Insert)); // 256
    RETURN_IF_FAILED(_StopBatchingItemBoundsChangeUpdatesAndNotify()); // 257

    cleanupOnFailure.release();
    return S_OK;
}

HRESULT CItemLayoutResolver::ResizeItemUncommitted(REFGUID itemID, const SIZE sizeItemCells)
{
    auto cleanupOnFailure = wil::scope_exit([this]
    {
        LOG_IF_FAILED(_StopBatchingItemBoundsChangeUpdatesAndNotify()); // 283
    });

    RETURN_IF_FAILED(_StartBatchingItemBoundsChangeUpdates()); // 286

    RETURN_IF_FAILED(_spCellArrayManager->AbandonChanges()); // 288

    Geometry::CRect rcItemBounds;

    RETURN_IF_FAILED(_FindTargetDestinationForNewSize(itemID, sizeItemCells, &rcItemBounds)); // 292

    RETURN_IF_FAILED(_ModifyItemUncommittedInternal(itemID, rcItemBounds, ModificationOperation::Resize)); // 294

    RETURN_IF_FAILED(_StopBatchingItemBoundsChangeUpdatesAndNotify()); // 296

    cleanupOnFailure.release();
    return S_OK;
}

HRESULT CItemLayoutResolver::SwapItemsUncommitted(REFGUID itemID1, REFGUID itemID2)
{
    Geometry::CRect rcTileBounds;
    RETURN_IF_FAILED_EXPECTED(_spCellArrayManager->GetItemBounds(itemID1, rcTileBounds));

    auto scopeExit = wil::scope_exit([this]
    {
        LOG_IF_FAILED(_StopBatchingItemBoundsChangeUpdatesAndNotify()); // 269
    });
    RETURN_IF_FAILED(_StartBatchingItemBoundsChangeUpdates()); // 272

    RETURN_IF_FAILED(_spCellArrayManager->AbandonChanges()); // 274
    RETURN_IF_FAILED(_spCellArrayManager->SwapItemsUncommitted(itemID1, itemID2)); // 275

    return S_OK;
}

HRESULT CItemLayoutResolver::AddNewContainer(REFGUID containerID, IItemLayoutResolver* resolver)
{
    return E_NOTIMPL;
}

HRESULT CItemLayoutResolver::AddContainer(REFGUID containerID, IItemLayoutResolver* resolver, const POINT destination)
{
    return E_NOTIMPL;
}

HRESULT CItemLayoutResolver::AddSizedContainer(REFGUID containerID, IItemLayoutResolver* pResolver, const RECT destination)
{
    RETURN_IF_FAILED(_spCellArrayManager->SetItem(containerID, destination, SIO_NONE)); // 304

    auto removeOnFailure = wil::scope_exit([this, containerID]
    {
        _spCellArrayManager->RemoveItemUncommitted(containerID);
        _spCellArrayManager->CommitChanges();
    });

    RETURN_IF_FAILED(m_folderResolvers.AddItem(containerID, pResolver)); // 311
    removeOnFailure.release();
    return S_OK;
}

HRESULT CItemLayoutResolver::InsertContainerUncommitted(REFGUID containerID, IItemLayoutResolver* resolver, const POINT destination)
{
    return E_NOTIMPL;
}

HRESULT CItemLayoutResolver::GetLayoutResolverForContainer(REFGUID containerID, IItemLayoutResolver** ppResolver)
{
    return E_NOTIMPL;
}

BOOL CItemLayoutResolver::IsCollapsed()
{
    return m_isCollapsed != 0;
}

HRESULT CItemLayoutResolver::Collapse()
{
    if (!m_isCollapsed)
    {
        m_isCollapsed = true;

        RECT newLayoutBounds;
        RETURN_IF_FAILED(GetLayoutBounds(&newLayoutBounds)); // 98

        _NotifyLayoutBoundsChange(newLayoutBounds);
    }

    return S_OK;
}

HRESULT CItemLayoutResolver::Expand()
{
    if (m_isCollapsed)
    {
        m_isCollapsed = false;

        RECT newLayoutBounds;
        RETURN_IF_FAILED(GetLayoutBounds(&newLayoutBounds)); // 98

        _NotifyLayoutBoundsChange(newLayoutBounds);
    }

    return S_OK;
}

HRESULT CItemLayoutResolver::RemoveItemUncommitted(REFGUID itemID)
{
    Geometry::CRect itemBounds;
    if (_spCellArrayManager->GetItemBounds(itemID, itemBounds) == S_OK)
    {
        auto cleanupOnFailure = wil::scope_exit([this]
        {
            LOG_IF_FAILED(_StopBatchingItemBoundsChangeUpdatesAndNotify()); // 132
        });

        RETURN_IF_FAILED(_StartBatchingItemBoundsChangeUpdates()); // 135
        RETURN_IF_FAILED(_spCellArrayManager->AbandonChanges()); // 136

        if (_spCellArrayManager->GetItemBounds(itemID, itemBounds) != S_OK)
        {
            itemBounds = Geometry::CRect(
                -1, -1,
                -1, -1);
        }

        RETURN_IF_FAILED(_spCellArrayManager->RemoveItemUncommitted(itemID)); // 145


        RETURN_IF_FAILED(_CleanupLayoutAfterOperation(itemBounds, Geometry::CRect())); // 148


        if (SUCCEEDED(_batchedUpdates.ContainsKey(itemID)))
        {
            LOG_IF_FAILED(_batchedUpdates.DeleteItem(itemID)); // 153
        }

        RETURN_IF_FAILED(_StopBatchingItemBoundsChangeUpdatesAndNotify()); // 156
        cleanupOnFailure.release();
    }

    return S_OK;
}

HRESULT CItemLayoutResolver::MoveItemUncommitted(REFGUID itemID, const POINT destination)
{
    LONG x = destination.x;
    LONG y = destination.y;

    auto cleanupOnFailure = wil::scope_exit([this]
    {
        LOG_IF_FAILED(_StopBatchingItemBoundsChangeUpdatesAndNotify()); // 211
    });

    RETURN_IF_FAILED(_StartBatchingItemBoundsChangeUpdates()); // 214
    RETURN_IF_FAILED(_spCellArrayManager->AbandonChanges()); // 215

    Geometry::CRect itemBounds;
    RETURN_IF_FAILED(_spCellArrayManager->GetItemBounds(itemID, itemBounds)); // 218

    itemBounds.Offset(x - itemBounds.left, y - itemBounds.top);
    RETURN_IF_FAILED(_ModifyItemUncommittedInternal(itemID, itemBounds, ModificationOperation::Move)); // 221
    RETURN_IF_FAILED(CItemLayoutResolver::_StopBatchingItemBoundsChangeUpdatesAndNotify()); // 222

    cleanupOnFailure.release();
    return S_OK;
}

BOOL CItemLayoutResolver::IsEmpty()
{
    return _spCellArrayManager->IsEmpty();
}

HRESULT CItemLayoutResolver::GetItemByCell(const POINT ptCell, GUID* pItemID)
{
    HRESULT hr = E_FAIL;
    *pItemID = c_emptyCellValue;

    if (_spCellArrayManager->IsValidCellCoordinate(ptCell))
    {
        hr = S_OK;
        *pItemID = _spCellArrayManager->GetItemAtCell(ptCell.x, ptCell.y);
    }

    return hr;
}

HRESULT CItemLayoutResolver::GetLayoutBounds(RECT* prcBounds)
{
    if (m_isCollapsed)
    {
        *prcBounds = {};
    }
    else
    {
        *prcBounds = _spCellArrayManager->GetCurrentCellArrayBounds();
    }
    return S_OK;
}

HRESULT CItemLayoutResolver::GetLayoutBoundsWithoutItem(REFGUID itemID, RECT* bounds)
{
    LOG_HR_IF(E_UNEXPECTED, m_isCollapsed); // 403
    return _spCellArrayManager->GetLayoutBoundsWithoutItem(itemID, bounds);
}

HRESULT CItemLayoutResolver::GetItemBounds(REFGUID itemID, RECT* itemBounds)
{
    Geometry::CRect tileBounds;
    RETURN_IF_FAILED_EXPECTED(_spCellArrayManager->GetItemBounds(itemID, tileBounds));

    *itemBounds = tileBounds;
    return S_OK;
}

HRESULT CItemLayoutResolver::GetContainerSizeWithMargins(REFGUID containerID, SIZE* size)
{
    return E_NOTIMPL;
}

HRESULT CItemLayoutResolver::GetLastOccupiedCellInColumn(
    const long column, REFGUID excludedItemID, POINT* occupiedCell, int* columnUnoccupied)
{
    GUID itemID = c_emptyCellValue;

    RECT layoutBounds;
    RETURN_IF_FAILED(GetLayoutBounds(&layoutBounds)); // 231

    POINT checkCell;
    checkCell.x = column;
    for (checkCell.y = layoutBounds.bottom - 1; checkCell.y >= 0; --checkCell.y)
    {
        if (SUCCEEDED(GetItemByCell(checkCell, &itemID)) && itemID != c_emptyCellValue && itemID != excludedItemID)
        {
            break;
        }
    }

    *columnUnoccupied = itemID == c_emptyCellValue;
    *occupiedCell = checkCell;
    return S_OK;
}

HRESULT CItemLayoutResolver::SetContainerMargins(const RECT containerMargins)
{
    return E_NOTIMPL;
}

HRESULT CItemLayoutResolver::GetContainerMargins(RECT* containerMargins)
{
    return E_NOTIMPL;
}

HRESULT CItemLayoutResolver::SetMaxCellBounds(const int nMaxXBounds, const int nMaxYBounds)
{
    HRESULT hr;

    CSet<GUID> rgItemsOutsideBounds;
    if (SUCCEEDED(_spCellArrayManager->GetItemsOutsideOfRect(
        Geometry::CRect(0, 0, nMaxXBounds, nMaxYBounds), &rgItemsOutsideBounds))
        && rgItemsOutsideBounds.GetCount() != 0)
    {
        hr = E_FAIL;
    }
    else
    {
        hr = _spCellArrayManager->SetMaximumCellArrayDimensions(nMaxXBounds, nMaxYBounds);
    }

    return hr;
}

SIZE CItemLayoutResolver::GetMaxCellBounds()
{
    return _spCellArrayManager->GetMaximumCellArrayDimensions();
}

HRESULT CItemLayoutResolver::MigrateItems(
    IItemLayoutResolver* pSourceLayout, const LayoutMigrationOptions migrationOptions)
{
    ComPtr<IItemMigrationHandler> spMigrationHandler;
    RETURN_IF_FAILED(s_CreateMigrationHandler(migrationOptions, &spMigrationHandler)); // 319
    RETURN_IF_FAILED(spMigrationHandler->MigrateItems(static_cast<CItemLayoutResolver*>(pSourceLayout)->_spCellArrayManager.Get(), _spCellArrayManager.Get(), _GetCellAssignor())); // 320

    ComPtr<IItemLayoutResolverInternal> layoutResolverInternal;
    if (SUCCEEDED(pSourceLayout->QueryInterface(layoutResolverInternal.ReleaseAndGetAddressOf())))
    {
        layoutResolverInternal->OnItemsMigrated(this);
    }

    return S_OK;
}

HRESULT CItemLayoutResolver::CommitChanges()
{
    return _CommitChangesInternal();
}

HRESULT CItemLayoutResolver::AbandonChanges()
{
    _StartBatchingItemBoundsChangeUpdates();

    auto cleanupOnFailure = wil::scope_exit([this]
    {
        LOG_IF_FAILED(_StopBatchingItemBoundsChangeUpdatesAndNotify()); // 341
    });

    RETURN_IF_FAILED(_spCellArrayManager->AbandonChanges()); // 344

    cleanupOnFailure.release();
    return S_OK;
}

// 22621 verified.
HRESULT CItemLayoutResolver::RepairLayoutUncommitted()
{
    _StartBatchingItemBoundsChangeUpdates();
    auto cleanupOnFailure = wil::scope_exit([this] // @MOD Use modern wil::scope_exit
    {
        LOG_IF_FAILED(_StopBatchingItemBoundsChangeUpdatesAndNotify()); // 355
    });

    RETURN_IF_FAILED(_RepairLayout()); // 358
    _StopBatchingItemBoundsChangeUpdatesAndNotify();

    cleanupOnFailure.release();
    return S_OK;
}

void CItemLayoutResolver::ItemBoundsUpdated(REFGUID itemID, const Geometry::CRect& rcItemBoundsCells)
{
    return _NotifyItemBoundsChange(itemID, rcItemBoundsCells);
}

void CItemLayoutResolver::CellArrayBoundsUpdated(const Geometry::CRect& rcLayoutBounds)
{
    _NotifyLayoutBoundsChange(rcLayoutBounds);
}

void CItemLayoutResolver::ItemRemovedPending(REFGUID itemID)
{
    _NotifyItemRemovedPending(itemID);

    if (_spCellArrayManager->IsEmpty())
    {
        _NotifyLastItemRemovedPending();
    }
}

void CItemLayoutResolver::ItemRemoved(REFGUID itemID)
{
    _NotifyItemRemoved(itemID);
}

void CItemLayoutResolver::NewItemAddedBegin()
{
    // No-op
}

void CItemLayoutResolver::NewItemAddedEnd()
{
    // No-op
}

void CItemLayoutResolver::OnItemsMigrated(IItemLayoutResolver* pDestinationLayout)
{
    (void)_htInternalCallbacks.Enum([&pDestinationLayout](IItemLayoutResolverInternalCallback*, const ComPtr<IItemLayoutResolverInternalCallback>& callback) -> bool
    {
        callback->OnItemsMigrated(pDestinationLayout);
        return true;
    });
}

void CItemLayoutResolver::OnItemsMigrated(IItemLayoutResolver* pDestinationLayoutResolver, REFGUID groupID)
{
    // No-op
}

HRESULT CItemLayoutResolver::GroupBoundsChanged(REFGUID groupID)
{
    return E_NOTIMPL;
}

void CItemLayoutResolver::GroupEmptiedPending(REFGUID groupID)
{
    // No-op
}

void CItemLayoutResolver::GroupEmptied()
{
    // No-op
}

#include <wil/win32_helpers.h>

// TODO Find 2x CRect::MoveTo inlined here
HRESULT CItemLayoutResolver::GetGutterHitTarget(REFGUID tileID, const RECT targetBounds, POINT* pAdjustedTargetCell)
{
    AutoIgnoredItem ignoredItem;
    ignoredItem.SetIgnoredItem(_spCellArrayManager.Get(), tileID, AutoIgnoredItemArray::Committed);

    POINT targetCell = { targetBounds.left, targetBounds.top };

    ComPtr<ICommittedCellArrayManager> spCommittedCellArrayManager;
    RETURN_IF_FAILED(_spCellArrayManager.As(&spCommittedCellArrayManager)); // 425

    CSet<GUID> overlappingTiles;
    RETURN_IF_FAILED(spCommittedCellArrayManager->GetCommittedItemsInRect(targetBounds, &overlappingTiles)); // 428
    if (!overlappingTiles.GetCount())
    {
        *pAdjustedTargetCell = targetCell;
        return S_OK;
    }

    RECT targetRow = {
        targetBounds.left, targetBounds.top, targetBounds.right, targetBounds.top + 1
    };
    CSet<GUID> tilesInRow;
    RETURN_IF_FAILED(spCommittedCellArrayManager->GetCommittedItemsInRect(targetRow, &tilesInRow)); // 439
    if (!tilesInRow.GetCount())
    {
        *pAdjustedTargetCell = targetCell;
        return S_OK;
    }

    LONG topRow = LONG_MAX;
    LONG bottomRow = LONG_MIN;
    tilesInRow.Enumerate([&spCommittedCellArrayManager, &topRow, &bottomRow](REFGUID value) -> bool
    {
        Geometry::CRect rc;
        if (SUCCEEDED(spCommittedCellArrayManager->GetCommittedItemBounds(value, rc)))
        {
            topRow = min(rc.top, topRow);
            bottomRow = max(rc.bottom - 1, bottomRow);
        }
        return true;
    });

    pAdjustedTargetCell->x = targetBounds.left;
    if (targetBounds.top == topRow)
    {
        pAdjustedTargetCell->y = topRow;
        return S_OK;
    }

    float f1 = (float)(targetBounds.top - topRow);
    float f2 = (float)(int)(bottomRow - topRow) * 0.5f;
    if (f2 > f1)
    {
        pAdjustedTargetCell->y = topRow;
        return S_OK;
    }
    if (f1 > f2)
    {
        pAdjustedTargetCell->y = bottomRow + 1;
        return S_OK;
    }
    if (wil::rect_height(targetBounds) == 1)
    {
        pAdjustedTargetCell->y = topRow;
        return S_OK;
    }

    pAdjustedTargetCell->y = bottomRow + 1;
    while (pAdjustedTargetCell->y != targetBounds.top)
    {
        Geometry::CRect candidateLocation(
            pAdjustedTargetCell->x, pAdjustedTargetCell->y - 1,
            pAdjustedTargetCell->x + wil::rect_width(targetBounds), pAdjustedTargetCell->y);
        candidateLocation.Offset(0, pAdjustedTargetCell->y <= targetBounds.top ? 2 : 0);

        CSet<GUID> tilesInAdjustedTarget;
        RETURN_IF_FAILED(spCommittedCellArrayManager->GetCommittedItemsInRect(candidateLocation, &tilesInAdjustedTarget)); // 494
        if (tilesInAdjustedTarget.GetCount())
        {
            Geometry::CRect adjustedRect;
            adjustedRect.left = pAdjustedTargetCell->x;
            adjustedRect.top = pAdjustedTargetCell->y;
            adjustedRect.right = adjustedRect.left + wil::rect_width(targetBounds);
            adjustedRect.bottom = adjustedRect.top + wil::rect_height(targetBounds);

            CSet<GUID> tilesInOriginalTarget;
            RETURN_IF_FAILED(spCommittedCellArrayManager->GetCommittedItemsInRect(adjustedRect, &tilesInOriginalTarget)); // 502

            bool conflictingTile = false;
            tilesInAdjustedTarget.Enumerate([&tilesInOriginalTarget, &conflictingTile](REFGUID value) -> bool
            {
                bool inOriginalTarget = SUCCEEDED(tilesInOriginalTarget.Contains(value));
                if (!inOriginalTarget)
                {
                    conflictingTile = true;
                }
                return inOriginalTarget;
            });
            if (conflictingTile)
            {
                break;
            }
        }

        *pAdjustedTargetCell = candidateLocation.GetLocation();
    }

    return S_OK;
}

HRESULT CItemLayoutResolver::_ModifyItemUncommittedInternal(
    REFGUID itemID, const RECT& rcDestination, const ModificationOperation operation)
{
    Geometry::CRect rcActualDestination = rcDestination;

    HRESULT hr = _PrepareLayoutBeforeOperation(Geometry::CRect(), Geometry::CRect());
    if (SUCCEEDED(hr))
    {
        Geometry::CRect rcItemBounds;
        if (FAILED(_spCellArrayManager->GetItemBounds(itemID, rcItemBounds)))
        {
            rcItemBounds = Geometry::CRect(-1, -1, -1, -1);
        }

        hr = _spCellArrayManager->AddIgnoredItem(itemID);
        _spCellArrayManager->ContainRectInsideFixedArrayBounds(rcActualDestination);
        if (SUCCEEDED(hr))
        {
            hr = _DisplaceItemsFromRect(rcActualDestination, rcItemBounds);
            if (SUCCEEDED(hr))
            {
                if (operation == ModificationOperation::Insert)
                {
                    hr = _spCellArrayManager->InsertItemUncommitted(itemID, rcActualDestination);
                }
                else
                {
                    hr = _spCellArrayManager->MoveItemUncommitted(itemID, rcActualDestination);
                }
            }
        }

        _spCellArrayManager->RemoveIgnoredItem(itemID);

        if (SUCCEEDED(hr))
        {
            hr =_CleanupLayoutAfterOperation(rcItemBounds, rcActualDestination);
        }
    }

    return hr;
}

HRESULT CItemLayoutResolver::_CommitChangesInternal()
{
    HRESULT hr = _spCellArrayManager->CommitChanges();
    if (SUCCEEDED(hr))
    {
        if (_spCellArrayManager->IsEmpty())
        {
            _NotifyLastItemRemoved();
        }
    }

    return hr;
}

HRESULT CItemLayoutResolver::_Collapse(const Geometry::CRect& rcSourceCells, const Geometry::CRect& rcTargetCells)
{
    return m_canCollapse ? _collapseManager.Collapse(rcSourceCells, rcTargetCells, _spCellArrayManager.Get()) : S_OK;
}

void CItemLayoutResolver::_NotifyNewItemAddedBegin()
{
    (void)_htInternalCallbacks.Enum([](IItemLayoutResolverInternalCallback*, const ComPtr<IItemLayoutResolverInternalCallback>& callback) -> bool
    {
        callback->NewItemAddedBegin();
        return true;
    });
}

void CItemLayoutResolver::_NotifyNewItemAddedEnd()
{
    (void)_htInternalCallbacks.Enum([](IItemLayoutResolverInternalCallback*, const ComPtr<IItemLayoutResolverInternalCallback>& callback) -> bool
    {
        callback->NewItemAddedEnd();
        return true;
    });
}

void CItemLayoutResolver::_NotifyItemBoundsChange(REFGUID itemID, const RECT& rcItemBoundsCells)
{
    if (_isBatchingItemBoundsChangeUpdates)
    {
        _batchedUpdates.SetItem(itemID, rcItemBoundsCells);
    }
    else
    {
        (void)_htCallbacks.Enum([&itemID, &rcItemBoundsCells](IItemLayoutResolverCallback*, const ComPtr<IItemLayoutResolverCallback>& callback) -> bool
        {
            callback->ItemBoundsUpdated(&itemID, &rcItemBoundsCells, 1);
            return true;
        });
    }
}

void CItemLayoutResolver::_NotifyLayoutBoundsChange(const Geometry::CRect& rcLayoutBoundsCells)
{
    (void)_htCallbacks.Enum([&rcLayoutBoundsCells](IItemLayoutResolverCallback*, const ComPtr<IItemLayoutResolverCallback>& callback) -> bool
    {
        callback->LayoutBoundsUpdated(rcLayoutBoundsCells);
        return true;
    });
}

void CItemLayoutResolver::_NotifyItemRemovedPending(REFGUID itemID)
{
    (void)_htCallbacks.Enum([&itemID](IItemLayoutResolverCallback*, const ComPtr<IItemLayoutResolverCallback>& callback) -> bool
    {
        callback->ItemRemovedPending(itemID);
        return true;
    });
}

void CItemLayoutResolver::_NotifyItemRemoved(REFGUID itemID)
{
    (void)_htCallbacks.Enum([&itemID](IItemLayoutResolverCallback*, const ComPtr<IItemLayoutResolverCallback>& callback) -> bool
    {
        callback->ItemRemoved(itemID);
        return true;
    });
}

void CItemLayoutResolver::_NotifyLastItemRemovedPending()
{
    (void)_htCallbacks.Enum([](IItemLayoutResolverCallback*, const ComPtr<IItemLayoutResolverCallback>& callback) -> bool
    {
        callback->LastItemRemovedPending();
        return true;
    });
}

void CItemLayoutResolver::_NotifyLastItemRemoved()
{
    (void)_htCallbacks.Enum([](IItemLayoutResolverCallback*, const ComPtr<IItemLayoutResolverCallback>& callback) -> bool
    {
        callback->LastItemRemoved();
        return true;
    });
}

HRESULT CItemLayoutResolver::_DisplaceItemsFromRect(const Geometry::CRect& targetRect, const Geometry::CRect& previousRect)
{
    return _displacementManager.DisplaceItemsFromRect(targetRect, previousRect, _spCellArrayManager.Get());
}

// 22621 verified.
HRESULT CItemLayoutResolver::_StartBatchingItemBoundsChangeUpdates()
{
    RETURN_HR_IF(E_FAIL, _isBatchingItemBoundsChangeUpdates); // 704
    _isBatchingItemBoundsChangeUpdates = true;
    _batchedUpdates.RemoveAll();
    return S_OK;
}

HRESULT CItemLayoutResolver::_StopBatchingItemBoundsChangeUpdatesAndNotify()
{
    RETURN_HR_IF(E_FAIL, !_isBatchingItemBoundsChangeUpdates);

    _isBatchingItemBoundsChangeUpdates = false;

    CSimpleHashTable<GUID, Geometry::CRect> htUpdatesUnique;
    (void)_batchedUpdates.Enum([&htUpdatesUnique](REFGUID key, const Geometry::CRect& value) -> bool // @Note: htUpdatesUnique added after 14361
    {
        htUpdatesUnique.SetItem(key, value);
        return true;
    });

    (void)htUpdatesUnique.Enum([this](REFGUID key, const Geometry::CRect& value) -> bool
    {
        _NotifyItemBoundsChange(key, value);
        return true;
    });

    return S_OK;
}

HRESULT CItemLayoutResolver::s_CreateMigrationHandler(
    const LayoutMigrationOptions& migrationOptions, IItemMigrationHandler** ppMigrationHandler)
{
    *ppMigrationHandler = nullptr;

    ComPtr<IItemMigrationHandler> migrationHandler;
    switch (migrationOptions.migrationType)
    {
        case LayoutMigrationType_LandscapeToPortrait:
            migrationHandler = Make<CLandscapeToPortraitMigrationHandler>();
            RETURN_IF_NULL_ALLOC(migrationHandler); // 741
            break;

        case LayoutMigrationType_PortraitToLandscape:
            RETURN_HR(E_NOTIMPL); // 744

        case LayoutMigrationType_ColumnChange:
            migrationHandler = Make<CColumnChangeMigrationHandler>();
            RETURN_IF_NULL_ALLOC(migrationHandler); // 748
            break;

        default:
            RETURN_HR(E_NOTIMPL); // 751
    }

    RETURN_IF_FAILED(migrationHandler->SetUnassignedItemId(migrationOptions.unassignedItemId)); // 755

    *ppMigrationHandler = migrationHandler.Detach();
    return S_OK;
}
