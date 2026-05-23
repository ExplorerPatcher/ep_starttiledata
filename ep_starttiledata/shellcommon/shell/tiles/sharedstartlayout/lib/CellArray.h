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
    STDMETHODIMP SetItem(REFGUID itemID, Geometry::CRect& rcTileBoundsCells);
    STDMETHODIMP RemoveItem(REFGUID itemID) override;
    STDMETHODIMP SetArrayBounds(const Geometry::CRect& rcNewArrayBounds) override;
    STDMETHODIMP AddIgnoredItem(REFGUID tileID) override;
    STDMETHODIMP RemoveIgnoredItem(REFGUID tileID) override;
    //~ End ICellArray Interface

private:
    template <typename TLambda>
    void _EnumerateAllCells(TLambda lambda);

    void _EnsureStorage(Geometry::CRect&);
    void _ChangeCellArraySize(const int, const int);
    void _GetItemsInRect(Geometry::CRect&, CSet<_GUID>*);
    void _SetRectValue(Geometry::CRect&, _GUID&);
    const GUID _GetCellValueNoIgnore(const int nXIndex, const int nYIndex);
    void _SetCellValue(const int, const int, GUID&);
    bool _IsRectEmpty(Geometry::CRect&);
    void _RemoveIgnoredItemsFromSet(CSet<GUID>*);
    bool _IsItemInArray(REFGUID itemID);
    HRESULT _SetArraySizeInternal(const Geometry::CSize& sizeArray, const Geometry::CPoint& ptOffset);

    Geometry::CRect _rcArrayBounds;
    GUID* _rgCellData;
    CSet<GUID> _setIgnoredTiles;
    Geometry::CPoint _ptOrigin;
};
