#pragma once

#include "ICellArray.h"

class CCellArray
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , ICellArray
    >
{
public:
    CCellArray();
    ~CCellArray() override;

    HRESULT RuntimeClassInitialize();

    //~ Begin ICellArray Interface
    STDMETHODIMP CloneArray(ICellArray** clonedCellArray) override;
    STDMETHODIMP_(const GUID) GetCellValue(const int nXIndex, const int nYIndex) override;
    STDMETHODIMP_(const Geometry::CRect&) GetArrayBounds() override;
    STDMETHODIMP_(void) GetItemsInRect(const Geometry::CRect& rcRect, CSet<GUID>* psetItemsInRect) override;
    STDMETHODIMP_(void) FixCoordinatesToBeNonNegative() override;
    STDMETHODIMP SetItem(REFGUID itemID, const Geometry::CRect& rcTileBoundsCells) override;
    STDMETHODIMP RemoveItem(REFGUID itemID) override;
    STDMETHODIMP SetArrayBounds(const Geometry::CRect& rcNewArrayBounds) override;
    STDMETHODIMP AddIgnoredItem(REFGUID tileID) override;
    STDMETHODIMP RemoveIgnoredItem(REFGUID tileID) override;
    //~ End ICellArray Interface

private:
    template <typename TLambda>
    void _EnumerateAllCells(TLambda lambda);

    void _EnsureStorage(const Geometry::CRect& a2);
    void _ChangeCellArraySize(const int a2, const int a3);
    void _GetItemsInRect(const Geometry::CRect& a2, CSet<GUID>* a3) const;
    void _SetRectValue(const Geometry::CRect& rcRect, REFGUID itemID);
    const GUID _GetCellValueNoIgnore(const int nXIndex, const int nYIndex) const;
    void _SetCellValue(const int nXIndex, const int nYIndex, REFGUID itemID);
    bool _IsRectEmpty(const Geometry::CRect& rcTileBoundsCells);
    void _RemoveIgnoredItemsFromSet(CSet<GUID>* psetItems) const;
    bool _IsItemInArray(REFGUID itemID);
    HRESULT _SetArraySizeInternal(const Geometry::CSize& sizeArray, const Geometry::CPoint& ptOffset);

    Geometry::CRect _rcArrayBounds;
    GUID* _rgCellData;
    CSet<GUID> _setIgnoredTiles;
    Geometry::CPoint _ptOrigin;
};

template <typename TLambda>
void CCellArray::_EnumerateAllCells(TLambda lambda)
{
    for (int nYIndex = _rcArrayBounds.top; nYIndex < _rcArrayBounds.bottom; ++nYIndex)
    {
        for (int nXIndex = _rcArrayBounds.left; nXIndex < _rcArrayBounds.right; ++nXIndex)
        {
            lambda(nXIndex, nYIndex, _GetCellValueNoIgnore(nXIndex, nYIndex));
        }
    }
}
