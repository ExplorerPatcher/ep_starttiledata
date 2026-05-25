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
    _EnumerateAllCells([&rcRect, &psetItemsOutsideRect](const int indexY, const int indexX, REFGUID itemID) -> int
    {
        static_assert(false, "Please implement me first!");
        return 0;
    });
    return S_OK;
}

AutoIgnoredItem::AutoIgnoredItem()
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
