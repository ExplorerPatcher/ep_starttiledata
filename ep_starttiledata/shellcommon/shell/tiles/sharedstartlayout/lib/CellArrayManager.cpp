#include "pch.h"

#include "CellArrayManager.h"

#include "CellArray.h"

using namespace Microsoft::WRL;

inline const GUID c_emptyCellValue_6 = {};

CCellArrayManager::CCellArrayManager()
    : m_removedItem(c_emptyCellValue_6)
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
            if (_pCellArray->GetCellValue(xIndex, yIndex) != c_emptyCellValue_6)
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
    // rbx
    // zf
    HRESULT v9; // eax
    unsigned int v10; // ebx
    int v11; // eax
    GUID nullGuid; // [rsp+20h] [rbp-38h] OVERLAPPED
    ComPtr<ICommittedCellArrayManager> spCommittedCellArrayManager; // [rsp+20h] [rbp-38h] SPLIT BYREF
    void* retaddr; // [rsp+58h] [rbp+0h]

    ComPtr<ICellArrayManager>* p_spCellArrayManager = &_spCellArrayManager;
    bool v5 = _spCellArrayManager.ptr_ == 0;
    nullGuid = 0;
    if (!v5)
    {
        goto LABEL_12;
    }
    if (_tileID != nullGuid)
    {
    LABEL_12:
        v10 = 0x80070057;
        wil::details::in1diag3::Return_Hr(retaddr, 685u, "shellcommon\\shell\\tiles\\sharedstartlayout\\lib\\cellarraymanager.cpp", 0x80070057);
    }
    else
    {
        _arrayToModify = AutoIgnoredItemArray::Committed;

        ComPtr<ICellArrayManager>::operator=((ComPtr<struct ICellArrayManager>*)&_spCellArrayManager, pCellArrayManager);
        spCommittedCellArrayManager.ptr_ = 0;
        v9 = ComPtr<ICellArrayManager>::As<ICommittedCellArrayManager>(p_spCellArrayManager, (__int64)&spCommittedCellArrayManager);
        v10 = v9;
        if (v9 >= 0)
        {
            v11 = spCommittedCellArrayManager.ptr_->AddIgnoredCommittedItem(spCommittedCellArrayManager.ptr_, tileID);
            if (v11 >= 0)
            {
                _tileID = *tileID;
            }
            else
            {
                wil::details::in1diag3::_Log_Hr(retaddr, 697u, "shellcommon\\shell\\tiles\\sharedstartlayout\\lib\\cellarraymanager.cpp", v11);
            }
            v10 = 0;
        }
        else
        {
            wil::details::in1diag3::Return_Hr(retaddr, 696u, "shellcommon\\shell\\tiles\\sharedstartlayout\\lib\\cellarraymanager.cpp", v9);
        }
        ComPtr<IAssociationElement>::InternalRelease(&spCommittedCellArrayManager);
    }
    return v10;
}
