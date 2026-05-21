#include "pch.h"

#include "CellArray.h"

inline const GUID c_emptyCellValue_7 = {};

CCellArray::CCellArray()
{
}

CCellArray::~CCellArray()
{
}

HRESULT CCellArray::RuntimeClassInitialize()
{
    _rgCellData = new(std::nothrow) GUID[];
    RETURN_IF_NULL_ALLOC(_rgCellData); // 22

    *_rgCellData = c_emptyCellValue_7;
    _ptOrigin = {};
    _rcArrayBounds = {};
    return S_OK;
}

const Geometry::CRect& CCellArray::GetArrayBounds()
{
    return _rcArrayBounds;
}

void CCellArray::FixCoordinatesToBeNonNegative()
{
    _ptOrigin = {};
    _rcArrayBounds.left = 0;
    _rcArrayBounds.top = 0;
    _rcArrayBounds.right = _rcArrayBounds.GetWidth();
    _rcArrayBounds.bottom = _rcArrayBounds.GetHeight();
}

HRESULT CCellArray::RemoveItem(REFGUID itemID)
{
    _EnumerateAllCells([&itemID, this](int indexY, int indexX, REFGUID) -> void
    {
    });
    return S_OK;
}

HRESULT CCellArray::AddIgnoredItem(REFGUID tileID)
{
    return _setIgnoredTiles.Add(tileID);
}

HRESULT CCellArray::RemoveIgnoredItem(REFGUID tileID)
{
    return _setIgnoredTiles.Remove(tileID);
}
