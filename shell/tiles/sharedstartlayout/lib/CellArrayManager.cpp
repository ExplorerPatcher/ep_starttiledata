#include "pch.h"

#include "CellArrayManager.h"

#include "CellArray.h"

using namespace Microsoft::WRL;

EXTERN_C static const inline GUID c_emptyCellValue = {};

CCellArrayManager::CCellArrayManager()
    : m_removedItem(c_emptyCellValue)
{
}

HRESULT CCellArrayManager::RuntimeClassInitialize()
{
    RETURN_IF_FAILED(MakeAndInitialize<CCellArray>(&_pCellArray)); // 19
    _sizeMaxBounds = {};
    return S_OK;
}

HRESULT CCellArrayManager::RegisterCallback(ICellArrayManagerCallback* pCallback)
{
    HRESULT hr = _htCallbacks.AddItem(pCallback, pCallback);
    return hr;
}

HRESULT CCellArrayManager::UnregisterCallback(ICellArrayManagerCallback* pCallback)
{
    return _htCallbacks.DeleteItem(pCallback);
}

const Geometry::CRect& CCellArrayManager::GetCurrentCellArrayBounds()
{
    return _pCellArray->GetArrayBounds();
}

const Geometry::CSize& CCellArrayManager::GetMaximumCellArrayDimensions()
{
    return _sizeMaxBounds;
}

bool CCellArrayManager::IsEmpty()
{
    return _htTileBounds.GetItemCount() == 0;
}

bool CCellArrayManager::IsValidRect(const Geometry::CRect& rcRect)
{
    Geometry::CPoint ptBottomRightOccupiedCell(rcRect.GetBottomRight());
    ptBottomRightOccupiedCell.Offset(-1, -1);
    return IsValidCellCoordinate(rcRect.GetTopLeft()) && IsValidCellCoordinate(ptBottomRightOccupiedCell);
}

bool CCellArrayManager::IsValidCellCoordinate(const Geometry::CPoint& ptCell)
{
    return (_sizeMaxBounds.cx == -1 || ptCell.x >= 0 && ptCell.x < _sizeMaxBounds.cx)
        && (_sizeMaxBounds.cy == -1 || ptCell.y >= 0 && ptCell.y < _sizeMaxBounds.cy);
}

const GUID CCellArrayManager::GetItemAtCell(const int cellX, const int cellY)
{
    return _pCellArray->GetCellValue(cellX, cellY);
}

bool CCellArrayManager::IsRectEmpty(const Geometry::CRect rcRect)
{
    for (int yIndex = rcRect.top; yIndex < rcRect.bottom; ++yIndex)
    {
        for (int xIndex = rcRect.left; xIndex < rcRect.right; ++xIndex)
        {
            if (_pCellArray->GetCellValue(xIndex, yIndex) != c_emptyCellValue)
            {
                return false;
            }
        }
    }

    return true;
}

HRESULT CCellArrayManager::GetItemsInRect(const Geometry::CRect rcRect, CSet<GUID>* psetItemsInRect)
{
    _pCellArray->GetItemsInRect(rcRect, psetItemsInRect);
    return S_OK;
}

HRESULT CCellArrayManager::GetItemsOutsideOfRect(const Geometry::CRect rcRect, CSet<GUID>* psetItemsOutsideRect)
{
    _EnumerateAllCells([&](const int indexX, const int indexY, REFGUID itemID) -> BOOL
    {
        if (itemID != c_emptyCellValue && !rcRect.Contains(Geometry::CPoint(indexX, indexY)))
        {
            if (FAILED(psetItemsOutsideRect->Contains(itemID)))
            {
                psetItemsOutsideRect->Add(itemID);
            }
        }
        return TRUE; // Assumed return value usage
    });
    return S_OK;
}

HRESULT CCellArrayManager::GetItemBounds(REFGUID tileID, Geometry::CRect& rcRect)
{
    return _htTileBounds.GetItem(tileID, rcRect);
}

HRESULT CCellArrayManager::GetBoundingRectForItems(const CSet<GUID>& setItems, Geometry::CRect& rcBoundingRect)
{
    HRESULT hr = S_OK;

    Geometry::CRect rcPrivateBoundingRect;
    setItems.Enumerate([&](REFGUID tileID) -> bool
    {
        Geometry::CRect rcItemBounds;
        hr = GetItemBounds(tileID, rcItemBounds);
        if (SUCCEEDED(hr))
        {
            if (!rcPrivateBoundingRect.IsEmpty())
            {
                rcPrivateBoundingRect = Geometry::CRect(
                    min(rcPrivateBoundingRect.left,   rcItemBounds.left  ),
                    min(rcPrivateBoundingRect.top,    rcItemBounds.top   ),
                    max(rcPrivateBoundingRect.right,  rcItemBounds.right ),
                    max(rcPrivateBoundingRect.bottom, rcItemBounds.bottom));
            }
            else
            {
                rcPrivateBoundingRect = rcItemBounds;
            }
        }
        return SUCCEEDED(hr);
    });

    if (SUCCEEDED(hr))
    {
        rcBoundingRect = rcPrivateBoundingRect;
    }

    return hr;
}

HRESULT CCellArrayManager::GetLayoutBoundsWithoutItem(REFGUID itemID, RECT* bounds)
{
    ComPtr<ICellArray> pTemporaryArray;
    RETURN_IF_FAILED(_pCellArray->CloneArray(&pTemporaryArray)); // 150
    RETURN_IF_FAILED(pTemporaryArray->RemoveItem(itemID)); // 151

    bool boundsChangedUnused;
    RETURN_IF_FAILED(_ShrinkInfiniteDimensionEdges(pTemporaryArray, &boundsChangedUnused)); // 154

    *bounds = pTemporaryArray->GetArrayBounds();

    return S_OK;
}

HRESULT CCellArrayManager::ContainRectInsideFixedArrayBounds(Geometry::CRect& rcRect)
{
    HRESULT hr = S_OK;

    if ((_sizeMaxBounds.cx == -1 || rcRect.GetWidth() <= _sizeMaxBounds.cx)
        && (_sizeMaxBounds.cy == -1 || rcRect.GetHeight() <= _sizeMaxBounds.cy))
    {
        if (_sizeMaxBounds.cx != -1 && rcRect.left < 0)
        {
            rcRect.Offset(-rcRect.left, 0);
        }
        if (_sizeMaxBounds.cy != -1 && rcRect.top < 0)
        {
            rcRect.Offset(0, -rcRect.top);
        }
        if (_sizeMaxBounds.cx != -1 && rcRect.right > _sizeMaxBounds.cx)
        {
            rcRect.Offset(_sizeMaxBounds.cx - rcRect.right, 0);
        }
        if (_sizeMaxBounds.cy != -1 && rcRect.bottom > _sizeMaxBounds.cy)
        {
            rcRect.Offset(0, _sizeMaxBounds.cy - rcRect.bottom);
        }
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

HRESULT CCellArrayManager::SetMaximumCellArrayDimensions(const int nMaxXBounds, const int nMaxYBounds)
{
    _sizeMaxBounds.cx = nMaxXBounds;
    _sizeMaxBounds.cy = nMaxYBounds;
    return S_OK;
}

HRESULT CCellArrayManager::SetItem(REFGUID tileID, Geometry::CRect rcTileBoundsCells, SET_ITEM_OPTIONS options)
{
    RETURN_HR_IF(E_INVALIDARG, !IsValidRect(rcTileBoundsCells)); // 210

    RETURN_IF_FAILED(_GrowCellArrayToFitRect(rcTileBoundsCells)); // 212

    //
    //

    RETURN_IF_FAILED_WITH_EXPECTED(_pCellArray->SetItem(tileID, rcTileBoundsCells), E_INVALIDARG); // 216 // @Note: WITH_EXPECTED added in 18362

    //
    RETURN_IF_FAILED(_htTileBounds.SetItem(tileID, rcTileBoundsCells)); // 219

    if ((options & SIO_SEND_BOUNDS_UPDATE) != 0)
    {
        _NotifyTileBoundsChange(tileID, rcTileBoundsCells);
    }

    return S_OK;
}

HRESULT CCellArrayManager::RemoveItemUncommitted(REFGUID tileID)
{
    RETURN_IF_FAILED(_EnsureWorkingWithTemporaryChanges()); // 231
    RETURN_IF_FAILED(_RemoveItemInternal(tileID, NotifyItemRemoved::SendRemoveNotification)); // 232
    RETURN_IF_FAILED(_ShrinkInfiniteDimensionEdges()); // 233

    m_removedItem = tileID;

    return S_OK;
}

HRESULT CCellArrayManager::InsertEmptyColumn(Geometry::CRect, bool)
{
    RETURN_IF_FAILED(_EnsureWorkingWithTemporaryChanges()); // 264

    return E_NOTIMPL;
}

HRESULT CCellArrayManager::MoveItemUncommitted(REFGUID tileID, Geometry::CRect rcTileBoundsCells)
{
    RETURN_IF_FAILED(_EnsureWorkingWithTemporaryChanges()); // 271

    //
    RETURN_IF_FAILED(_RemoveItemInternal(tileID, NotifyItemRemoved::DoNotSendRemoveNotification)); // 274

    //
    //
    //

    HRESULT hr = SetItem(tileID, rcTileBoundsCells, SIO_SEND_BOUNDS_UPDATE);
    if (FAILED_LOG(hr)) // 281
        RETURN_IF_FAILED(AbandonChanges()); // 282

    return hr;
}

HRESULT CCellArrayManager::SwapItemsUncommitted(REFGUID tileID, REFGUID tileIDNew) // @Note: Added in 17134
{
    RETURN_IF_FAILED(_EnsureWorkingWithTemporaryChanges()); // 243

    Geometry::CRect rcItemBounds;
    RETURN_IF_FAILED(GetItemBounds(tileID, rcItemBounds)); // 246

    RETURN_IF_FAILED(_RemoveItemInternal(tileID, NotifyItemRemoved::DoNotSendRemoveNotification)); // 248

    auto abandonOnFailure = wil::scope_exit([this]
    {
        LOG_IF_FAILED(AbandonChanges()); // 252
    });

    RETURN_IF_FAILED(SetItem(tileIDNew, rcItemBounds, SIO_NONE)); // 255

    abandonOnFailure.release();
    return S_OK;
}

HRESULT CCellArrayManager::InsertItemUncommitted(REFGUID tileID, const Geometry::CRect rcTileBoundsCells)
{
    RETURN_IF_FAILED(_EnsureWorkingWithTemporaryChanges()); // 290

    return SetItem(tileID, rcTileBoundsCells, SIO_SEND_BOUNDS_UPDATE);
}

HRESULT CCellArrayManager::AddIgnoredItem(REFGUID tileID)
{
    return _pCellArray->AddIgnoredItem(tileID);
}

HRESULT CCellArrayManager::RemoveIgnoredItem(REFGUID tileID)
{
    return _pCellArray->RemoveIgnoredItem(tileID);
}

HRESULT CCellArrayManager::CommitChanges()
{
    _ClearCommittedCellArray();

    if (m_removedItem != c_emptyCellValue)
    {
        _NotifyTileRemoved(m_removedItem);
        m_removedItem = c_emptyCellValue;
    }

    RETURN_IF_FAILED(_ShrinkInfiniteDimensionEdges()); // 317

    return S_OK;
}

HRESULT CCellArrayManager::AbandonChanges()
{
    if (_pCommittedCellArray != nullptr)
    {
        CSimpleHashTable<GUID, Geometry::CRect> htBoundsNotificationsToSend;
        Geometry::CRect rcLayoutUpdateToSend;

        _htCommittedTileBounds.Enum([&](REFGUID tileID, Geometry::CRect rcCommittedTileBounds) -> BOOL
        {
            Geometry::CRect rcCurrentTileBounds;
            HRESULT hr = GetItemBounds(tileID, rcCurrentTileBounds);
            if (FAILED(hr) || rcCurrentTileBounds != rcCommittedTileBounds)
            {
                _htTileBounds.SetItem(tileID, rcCommittedTileBounds);
                htBoundsNotificationsToSend.AddItem(tileID, rcCommittedTileBounds);
            }
            return TRUE;
        });

        _htTileBounds.Enum([&](REFGUID tileID, Geometry::CRect) -> BOOL
        {
            if (FAILED(_htCommittedTileBounds.ContainsKey(tileID)))
            {
                _htTileBounds.DeleteItem(tileID);
            }
            return TRUE;
        });

        bool fSendCellArrayBoundsUpdate = _pCommittedCellArray->GetArrayBounds() != _pCellArray->GetArrayBounds();
        if (fSendCellArrayBoundsUpdate)
        {
            rcLayoutUpdateToSend = _pCommittedCellArray->GetArrayBounds();
        }

        _pCellArray.Swap(_pCommittedCellArray);

        htBoundsNotificationsToSend.Enum([&](REFGUID tileID, Geometry::CRect rcCommittedTileBounds) -> BOOL
        {
            if (IsRectEmpty(rcCommittedTileBounds))
            {
                _RemoveItemInternal(tileID, NotifyItemRemoved::SendRemoveNotification);
            }
            else
            {
                _NotifyTileBoundsChange(tileID, rcCommittedTileBounds);
            }
            return TRUE;
        });

        if (fSendCellArrayBoundsUpdate)
        {
            _NotifyCellArrayBoundsChange(rcLayoutUpdateToSend);
        }

        _ClearCommittedCellArray();
        m_removedItem = c_emptyCellValue;
    }

    return S_OK;
}

HRESULT CCellArrayManager::FixCoordinatesToBeNonNegative()
{
    BOOL isSuccesful = TRUE;

    Geometry::CPoint ptOrigin = GetCurrentCellArrayBounds().GetTopLeft();
    if (ptOrigin.x < 0 || ptOrigin.y < 0)
    {
        _pCellArray->FixCoordinatesToBeNonNegative();
        _NotifyCellArrayBoundsChange(GetCurrentCellArrayBounds());
        isSuccesful = _htTileBounds.Enum([&](REFGUID tileID, Geometry::CRect rcTileBounds) -> BOOL
        {
            BOOL isSet = FALSE;
            rcTileBounds.Offset(-ptOrigin.x, -ptOrigin.y);
            if (_htTileBounds.SetItem(tileID, rcTileBounds) == S_OK)
            {
                _NotifyTileBoundsChange(tileID, rcTileBounds);
                isSet = TRUE;
            }
            return isSet;
        });
    }

    return isSuccesful ? S_OK : E_FAIL;
}

const GUID CCellArrayManager::GetCommittedItemAtCell(const int cellX, const int cellY)
{
    ComPtr<ICellArray> pArray = _pCommittedCellArray != nullptr ? _pCommittedCellArray.Get() : _pCellArray.Get();
    return pArray->GetCellValue(cellX, cellY);
}

HRESULT CCellArrayManager::GetCommittedItemBounds(REFGUID tileID, Geometry::CRect& rect)
{
    return (_pCommittedCellArray != nullptr ? _htCommittedTileBounds : _htTileBounds).GetItem(tileID, rect);
}

HRESULT CCellArrayManager::GetCommittedItemsInRect(const Geometry::CRect& rect, CSet<GUID>* prgItemsInRect)
{
    ComPtr<ICellArray> pArray = _pCommittedCellArray != nullptr ? _pCommittedCellArray.Get() : _pCellArray.Get();
    pArray->GetItemsInRect(rect, prgItemsInRect);

    return S_OK;
}

HRESULT CCellArrayManager::AddIgnoredCommittedItem(REFGUID tileID)
{
    ComPtr<ICellArray> pArray = _pCommittedCellArray != nullptr ? _pCommittedCellArray.Get() : _pCellArray.Get();
    RETURN_IF_FAILED(pArray->AddIgnoredItem(tileID)); // 447

    return S_OK;
}

HRESULT CCellArrayManager::RemoveIgnoredCommittedItem(REFGUID tileID)
{
    ComPtr<ICellArray> pArray = _pCommittedCellArray != nullptr ? _pCommittedCellArray.Get() : _pCellArray.Get();
    RETURN_IF_FAILED(pArray->RemoveIgnoredItem(tileID)); // 454

    return S_OK;
}

HRESULT CCellArrayManager::_GrowCellArrayToFitRect(const Geometry::CRect rcTileBoundsCells)
{
    HRESULT hr = S_OK;
    Geometry::CRect rcArrayBounds = _pCellArray->GetArrayBounds();

    Geometry::CPoint ptBottomRightTest(rcTileBoundsCells.GetBottomRight());
    ptBottomRightTest.Offset(-1, -1);
    if (!rcArrayBounds.Contains(rcTileBoundsCells.GetTopLeft()) || !rcArrayBounds.Contains(ptBottomRightTest))
    {
        rcArrayBounds = Geometry::CRect(
            min(rcTileBoundsCells.left,   rcArrayBounds.left  ),
            min(rcTileBoundsCells.top,    rcArrayBounds.top   ),
            max(rcTileBoundsCells.right,  rcArrayBounds.right ),
            max(rcTileBoundsCells.bottom, rcArrayBounds.bottom));
        hr = _pCellArray->SetArrayBounds(rcArrayBounds);
        if (SUCCEEDED(hr))
        {
            _NotifyCellArrayBoundsChange(GetCurrentCellArrayBounds());
        }
    }

    return hr;
}

void CCellArrayManager::_NotifyTileBoundsChange(REFGUID tileID, const RECT rcTileBoundsCells)
{
    _htCallbacks.Enum([&](ICellArrayManagerCallback*, const ComPtr<ICellArrayManagerCallback>& callback) -> BOOL
    {
        callback->ItemBoundsUpdated(tileID, rcTileBoundsCells);
        return true;
    });
}

void CCellArrayManager::_NotifyCellArrayBoundsChange(const Geometry::CRect rcLayoutBoundsCells)
{
    _htCallbacks.Enum([&](ICellArrayManagerCallback*, const ComPtr<ICellArrayManagerCallback>& callback) -> BOOL
    {
        callback->CellArrayBoundsUpdated(rcLayoutBoundsCells);
        return true;
    });
}

void CCellArrayManager::_NotifyTileRemovedPending(REFGUID tileID)
{
    _htCallbacks.Enum([&](ICellArrayManagerCallback*, const ComPtr<ICellArrayManagerCallback>& callback) -> BOOL
    {
        callback->ItemRemovedPending(tileID);
        return true;
    });
}

void CCellArrayManager::_NotifyTileRemoved(REFGUID tileID)
{
    _htCallbacks.Enum([&](ICellArrayManagerCallback*, const ComPtr<ICellArrayManagerCallback>& callback) -> BOOL
    {
        callback->ItemRemoved(tileID);
        return true;
    });
}

HRESULT CCellArrayManager::_ShrinkInfiniteDimensionEdges()
{
    bool boundsChanged;
    RETURN_IF_FAILED(_ShrinkInfiniteDimensionEdges(_pCellArray, &boundsChanged)); // 520
    if (boundsChanged)
    {
        _NotifyCellArrayBoundsChange(GetCurrentCellArrayBounds());
    }

    return S_OK;
}

HRESULT CCellArrayManager::_ShrinkInfiniteDimensionEdges(const ComPtr<ICellArray>& cellArray, bool* boundsChanged) const
{
    HRESULT hr = S_OK;
    const Geometry::CRect currentArray = cellArray->GetArrayBounds();
    *boundsChanged = false;

    int cellX = currentArray.right;
    int cellY = currentArray.bottom;

    bool seenOccupiedTileInRow = false;
    bool seenOccupiedTileInColumn = false;

    BOOL shouldContinue = TRUE;
    for (int indexX = currentArray.right; shouldContinue && indexX > currentArray.left; --indexX)
    {
        for (int i = currentArray.top; i < currentArray.bottom; ++i)
        {
            GUID guidOne = cellArray->GetCellValue(indexX - 1, i);
            if (guidOne != c_emptyCellValue)
            {
                cellX = indexX;
                seenOccupiedTileInRow = true;
                shouldContinue = FALSE;
                break;
            }
        }
    }

    shouldContinue = TRUE;
    for (int indexY = currentArray.bottom; shouldContinue && indexY > currentArray.top; --indexY)
    {
        for (int i = currentArray.left; i < currentArray.right; ++i)
        {
            GUID guidOne = cellArray->GetCellValue(i, indexY - 1);
            if (guidOne != c_emptyCellValue)
            {
                cellY = indexY;
                seenOccupiedTileInColumn = true;
                shouldContinue = FALSE;
                break;
            }
        }
    }

    Geometry::CRect shrunkenArray(
        currentArray.left, currentArray.top, seenOccupiedTileInRow ? cellX : 0, seenOccupiedTileInColumn ? cellY : 0);
    if (currentArray != shrunkenArray)
    {
        hr = cellArray->SetArrayBounds(shrunkenArray);
        if (SUCCEEDED(hr))
        {
            *boundsChanged = true;
        }
    }

    return hr;
}

HRESULT CCellArrayManager::_EnsureWorkingWithTemporaryChanges()
{
    if (_pCommittedCellArray == nullptr)
    {
        _ASSERT(_htCommittedTileBounds.GetItemCount() == 0);
        RETURN_IF_FAILED(_SaveExistingCellArray()); // 603
    }

    return S_OK;
}

HRESULT CCellArrayManager::_SaveExistingCellArray()
{
    RETURN_IF_FAILED(_pCellArray->CloneArray(&_pCommittedCellArray)); // 612

    _htTileBounds.Enum([&](REFGUID tileID, Geometry::CRect rcTileBounds) -> BOOL
    {
        _htCommittedTileBounds.SetItem(tileID, rcTileBounds);
        return TRUE;
    });

    return S_OK;
}

void CCellArrayManager::_ClearCommittedCellArray()
{
    _pCommittedCellArray.Reset();
    _htCommittedTileBounds.RemoveAll();
}

HRESULT CCellArrayManager::_RemoveItemInternal(REFGUID itemID, NotifyItemRemoved shouldNotifyItemRemoved)
{
    RETURN_IF_FAILED(_pCellArray->RemoveItem(itemID)); // 630

    //
    RETURN_IF_FAILED(_htTileBounds.DeleteItem(itemID)); // 633

    if (shouldNotifyItemRemoved == NotifyItemRemoved::SendRemoveNotification)
    {
        _NotifyTileRemovedPending(itemID);
    }

    return S_OK;
}

AutoIgnoredItem::AutoIgnoredItem()
    : _arrayToModify(AutoIgnoredItemArray::Pending)
    , _tileID()
{
}

AutoIgnoredItem::~AutoIgnoredItem()
{
    if (_spCellArrayManager != nullptr)
    {
        if (_arrayToModify == AutoIgnoredItemArray::Pending)
        {
            LOG_IF_FAILED(_spCellArrayManager->RemoveIgnoredItem(_tileID)); // 668
        }
        else if (_arrayToModify == AutoIgnoredItemArray::Committed)
        {
            ComPtr<ICommittedCellArrayManager> spCommittedCellArrayManager;
            if (SUCCEEDED_LOG(_spCellArrayManager.As(&spCommittedCellArrayManager))) // 673
            {
                LOG_IF_FAILED(spCommittedCellArrayManager->RemoveIgnoredCommittedItem(_tileID)); // 675
            }
        }
    }
}

HRESULT AutoIgnoredItem::SetIgnoredItem(ICellArrayManager* pCellArrayManager, REFGUID tileID, AutoIgnoredItemArray arrayToModify)
{
    GUID nullGuid = {};
    RETURN_HR_IF(E_INVALIDARG, _spCellArrayManager != nullptr || _tileID != nullGuid); // 685

    _arrayToModify = arrayToModify;
    _spCellArrayManager = pCellArrayManager;

    ComPtr<ICommittedCellArrayManager> spCommittedCellArrayManager;
    RETURN_IF_FAILED(_spCellArrayManager.As(&spCommittedCellArrayManager)); // 696
    if (SUCCEEDED_LOG(spCommittedCellArrayManager->AddIgnoredCommittedItem(tileID))) // 697
    {
        _tileID = tileID;
    }

    return S_OK;
}
