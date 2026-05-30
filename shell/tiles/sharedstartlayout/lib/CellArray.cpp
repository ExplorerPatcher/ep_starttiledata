#include "pch.h"

#include "CellArray.h"

using namespace Microsoft::WRL;

EXTERN_C static const inline GUID c_emptyCellValue = {};

CCellArray::CCellArray()
{
}

CCellArray::~CCellArray()
{
    delete[] _rgCellData;
}

HRESULT CCellArray::RuntimeClassInitialize()
{
    _rgCellData = new(std::nothrow) GUID[1];
    RETURN_IF_NULL_ALLOC(_rgCellData); // 22

    _rgCellData[0] = c_emptyCellValue;
    _ptOrigin = Geometry::CPoint();
    _rcArrayBounds = Geometry::CRect();
    return S_OK;
}

HRESULT CCellArray::CloneArray(ICellArray** clonedCellArray)
{
    ComPtr<CCellArray> temporaryArray;
    RETURN_IF_FAILED(MakeAndInitialize<CCellArray>(&temporaryArray)); // 33

    Geometry::CRect rcExistingArrayBounds = GetArrayBounds();
    RETURN_IF_FAILED(temporaryArray->SetArrayBounds(rcExistingArrayBounds)); // 36

    for (int nYIndex = rcExistingArrayBounds.top; nYIndex < rcExistingArrayBounds.bottom; ++nYIndex)
    {
        for (int nXIndex = rcExistingArrayBounds.left; nXIndex < rcExistingArrayBounds.right; ++nXIndex)
        {
            temporaryArray->_SetCellValue(nXIndex, nYIndex, _GetCellValueNoIgnore(nXIndex, nYIndex));
        }
    }

    RETURN_IF_FAILED(temporaryArray.CopyTo(clonedCellArray)); // 46

    return S_OK;
}

const GUID CCellArray::GetCellValue(const int nXIndex, const int nYIndex)
{
    GUID itemID = c_emptyCellValue;

    if (_rcArrayBounds.Contains(Geometry::CPoint(nXIndex, nYIndex)))
    {
        itemID = _GetCellValueNoIgnore(nXIndex, nYIndex);
    }

    if (SUCCEEDED(_setIgnoredTiles.Contains(itemID)))
    {
        itemID = c_emptyCellValue;
    }

    return itemID;
}

const Geometry::CRect& CCellArray::GetArrayBounds()
{
    return _rcArrayBounds;
}

void CCellArray::GetItemsInRect(const Geometry::CRect& rcRect, CSet<GUID>* psetItemsInRect)
{
    for (int yIndex = rcRect.top; yIndex < rcRect.bottom; ++yIndex)
    {
        for (int xIndex = rcRect.left; xIndex < rcRect.right; ++xIndex)
        {
            GUID cellValue = GetCellValue(xIndex, yIndex);
            if (c_emptyCellValue != cellValue && FAILED(psetItemsInRect->Contains(cellValue)))
            {
                psetItemsInRect->Add(cellValue);
            }
        }
    }
}

void CCellArray::FixCoordinatesToBeNonNegative()
{
    _ptOrigin = Geometry::CPoint();
    _rcArrayBounds = Geometry::CRect(0, 0, _rcArrayBounds.GetWidth(), _rcArrayBounds.GetHeight());
}

HRESULT CCellArray::SetItem(REFGUID itemID, const Geometry::CRect& rcTileBoundsCells)
{
    HRESULT hr = S_OK;

    if (_IsRectEmpty(rcTileBoundsCells) && !_IsItemInArray(itemID))
    {
        Geometry::CPoint ptBottomRightOccupiedCell(rcTileBoundsCells.GetBottomRight());
        ptBottomRightOccupiedCell.Offset(-1, -1);
        if (_rcArrayBounds.Contains(rcTileBoundsCells.GetTopLeft()) && _rcArrayBounds.Contains(ptBottomRightOccupiedCell))
        {
            _SetRectValue(rcTileBoundsCells, itemID);
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

HRESULT CCellArray::RemoveItem(REFGUID itemID)
{
    _EnumerateAllCells([&](int indexX, int indexY, REFGUID enumeratedItemID) -> void
    {
        if (enumeratedItemID == itemID)
        {
            _SetCellValue(indexX, indexY, c_emptyCellValue);
        }
    });
    return S_OK;
}

HRESULT CCellArray::SetArrayBounds(const Geometry::CRect& rcNewArrayBounds)
{
    Geometry::CPoint ptOffset;
    if (rcNewArrayBounds.left < _rcArrayBounds.left || rcNewArrayBounds.top < _rcArrayBounds.top)
    {
        ptOffset.x = abs(rcNewArrayBounds.left - _rcArrayBounds.left);
        ptOffset.y = abs(rcNewArrayBounds.top - _rcArrayBounds.top);
    }

    HRESULT hr = _SetArraySizeInternal(rcNewArrayBounds.GetSize(), ptOffset);
    if (hr == S_OK)
    {
        _ptOrigin.Offset(ptOffset.x, ptOffset.y);
    }

    return hr;
}

HRESULT CCellArray::AddIgnoredItem(REFGUID tileID)
{
    return _setIgnoredTiles.Add(tileID);
}

HRESULT CCellArray::RemoveIgnoredItem(REFGUID tileID)
{
    return _setIgnoredTiles.Remove(tileID);
}

void CCellArray::_EnsureStorage(const Geometry::CRect& a2)
{
    // Function body unknown
}

void CCellArray::_ChangeCellArraySize(const int a2, const int a3)
{
    // Function body unknown
}

void CCellArray::_GetItemsInRect(const Geometry::CRect& a2, CSet<GUID>* a3) const
{
    // Function body unknown
}

void CCellArray::_SetRectValue(const Geometry::CRect& rcRect, REFGUID itemID)
{
    for (int nYIndex = rcRect.top; nYIndex < rcRect.bottom; ++nYIndex)
    {
        for (int nXIndex = rcRect.left; nXIndex < rcRect.right; ++nXIndex)
        {
            _SetCellValue(nXIndex, nYIndex, itemID);
        }
    }
}

const GUID CCellArray::_GetCellValueNoIgnore(const int nXIndex, const int nYIndex) const
{
    GUID itemID = c_emptyCellValue;

    if (_rcArrayBounds.Contains(Geometry::CPoint(nXIndex, nYIndex)))
    {
        int nNormalizedXIndex = (_ptOrigin.x + nXIndex);
        int nNormalizedYIndex = (_ptOrigin.y + nYIndex) * (_rcArrayBounds.right - _rcArrayBounds.left);
        itemID = _rgCellData[nNormalizedXIndex + nNormalizedYIndex];
    }

    return itemID;
}

void CCellArray::_SetCellValue(const int nXIndex, const int nYIndex, REFGUID itemID)
{
    int nNormalizedXIndex = (_ptOrigin.x + nXIndex);
    int nNormalizedYIndex = (_ptOrigin.y + nYIndex) * (_rcArrayBounds.right - _rcArrayBounds.left);
    _rgCellData[nNormalizedXIndex + nNormalizedYIndex] = itemID;
}

bool CCellArray::_IsRectEmpty(const Geometry::CRect& rcTileBoundsCells)
{
    CSet<GUID> itemsInRect;
    GetItemsInRect(rcTileBoundsCells, &itemsInRect);
    _RemoveIgnoredItemsFromSet(&itemsInRect);
    return itemsInRect.GetCount() == 0;
}

void CCellArray::_RemoveIgnoredItemsFromSet(CSet<GUID>* psetItems) const
{
    _setIgnoredTiles.Enumerate([&](REFGUID itemID) -> bool
    {
        if (SUCCEEDED(psetItems->Contains(itemID)))
        {
            psetItems->Remove(itemID);
        }
        return true;
    });
}

bool CCellArray::_IsItemInArray(REFGUID itemID)
{
    bool fItemInArray = false;
    _EnumerateAllCells([&](int, int, REFGUID enumeratedItemID) -> void
    {
        if (enumeratedItemID == itemID)
        {
            fItemInArray = true;
        }
    });
    return fItemInArray;
}

HRESULT CCellArray::_SetArraySizeInternal(const Geometry::CSize& sizeArray, const Geometry::CPoint& ptOffset)
{
    HRESULT hr = S_OK;

    if (ptOffset.x >= 0 && ptOffset.y >= 0)
    {
        GUID* _rgOriginalCellData = _rgCellData;

        Geometry::CRect rcOriginalArrayBounds = _rcArrayBounds;
        _rcArrayBounds.Offset(-ptOffset.x, -ptOffset.y);
        _rcArrayBounds.right = _rcArrayBounds.left + sizeArray.cx;
        _rcArrayBounds.bottom = _rcArrayBounds.top + sizeArray.cy;

        int nOriginalWidth = rcOriginalArrayBounds.GetWidth();

        _rgCellData = new(std::nothrow) GUID[sizeArray.cx * sizeArray.cy];
        RETURN_IF_NULL_ALLOC(_rgCellData); // 252

        for (int i = 0; i < sizeArray.cx * sizeArray.cy; ++i)
        {
            _rgCellData[i] = c_emptyCellValue;
        }

        for (int nOriginalYIndex = rcOriginalArrayBounds.top; nOriginalYIndex < rcOriginalArrayBounds.bottom; ++nOriginalYIndex)
        {
            for (int nOriginalXIndex = rcOriginalArrayBounds.left; nOriginalXIndex < rcOriginalArrayBounds.right; ++nOriginalXIndex)
            {
                int nNormalizedXIndex = (_ptOrigin.x + nOriginalXIndex);
                int nNormalizedYIndex = (_ptOrigin.y + nOriginalYIndex) * nOriginalWidth;
                int i = nNormalizedXIndex + nNormalizedYIndex;
                if (_rcArrayBounds.Contains(Geometry::CPoint(nOriginalXIndex, nOriginalYIndex)))
                {
                    _SetCellValue(ptOffset.x + nOriginalXIndex, ptOffset.y + nOriginalYIndex, _rgOriginalCellData[i]);
                }
                else if (_rgOriginalCellData[i] != c_emptyCellValue)
                {
                    hr = E_FAIL;
                }
            }
        }

        delete[] _rgOriginalCellData;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}
