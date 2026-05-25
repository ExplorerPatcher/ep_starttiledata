#pragma once

#include "ICellArray.h"
#include "ICellArrayManager.h"

MIDL_INTERFACE("48aa14a7-b0e2-4a6e-8a9e-55e8c7a00533")
ICommittedCellArrayManager : IUnknown
{
    virtual const GUID STDMETHODCALLTYPE GetCommittedItemAtCell(const int, const int) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCommittedItemBounds(REFGUID, Geometry::CRect&) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCommittedItemsInRect(const Geometry::CRect&, CSet<GUID>*) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddIgnoredCommittedItem(REFGUID) = 0;
    virtual HRESULT STDMETHODCALLTYPE RemoveIgnoredCommittedItem(REFGUID) = 0;
};

class CCellArrayManager
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , ICellArrayManager
        , ICommittedCellArrayManager
    >
{
public:
    CCellArrayManager();

    HRESULT RuntimeClassInitialize();

    //~ Begin ICellArrayManager Interface
    STDMETHODIMP RegisterCallback(ICellArrayManagerCallback* pCallback) override;
    STDMETHODIMP UnregisterCallback(ICellArrayManagerCallback* pCallback) override;
    STDMETHODIMP_(const Geometry::CRect&) GetCurrentCellArrayBounds() override;
    STDMETHODIMP_(const Geometry::CSize&) GetMaximumCellArrayDimensions() override;
    STDMETHODIMP_(bool) IsEmpty() override;
    STDMETHODIMP_(bool) IsValidRect(const Geometry::CRect& rcRect) override;
    STDMETHODIMP_(bool) IsValidCellCoordinate(const Geometry::CPoint& ptCell) override;
    STDMETHODIMP_(const GUID) GetItemAtCell(const int cellX, const int cellY) override;
    STDMETHODIMP_(bool) IsRectEmpty(const Geometry::CRect rcRect) override;
    STDMETHODIMP GetItemsInRect(const Geometry::CRect rcRect, CSet<GUID>* psetItemsInRect) override;
    STDMETHODIMP GetItemsOutsideOfRect(const Geometry::CRect rcRect, CSet<GUID>* psetItemsOutsideRect) override;
    STDMETHODIMP GetItemBounds(REFGUID tileID, Geometry::CRect& rcRect) override;
    STDMETHODIMP GetBoundingRectForItems(const CSet<GUID>& setItems, Geometry::CRect& rcBoundingRect) override;
    STDMETHODIMP GetLayoutBoundsWithoutItem(REFGUID itemID, RECT* bounds) override;
    STDMETHODIMP ContainRectInsideFixedArrayBounds(Geometry::CRect& rcRect) override;
    STDMETHODIMP SetMaximumCellArrayDimensions(const int nMaxXBounds, const int nMaxYBounds) override;
    STDMETHODIMP SetItem(REFGUID tileID, Geometry::CRect rcTileBoundsCells, SET_ITEM_OPTIONS options) override;
    STDMETHODIMP RemoveItemUncommitted(REFGUID tileID) override;
    STDMETHODIMP InsertEmptyColumn(Geometry::CRect, bool) override;
    STDMETHODIMP MoveItemUncommitted(REFGUID tileID, Geometry::CRect rcTileBoundsCells) override;
    STDMETHODIMP SwapItemsUncommitted(REFGUID tileID, REFGUID tileIDNew) override; // @Note: Added in 17134
    STDMETHODIMP InsertItemUncommitted(REFGUID tileID, const Geometry::CRect rcTileBoundsCells) override;
    STDMETHODIMP AddIgnoredItem(REFGUID tileID) override;
    STDMETHODIMP RemoveIgnoredItem(REFGUID tileID) override;
    STDMETHODIMP CommitChanges() override;
    STDMETHODIMP AbandonChanges() override;
    STDMETHODIMP FixCoordinatesToBeNonNegative() override;
    //~ End ICellArrayManager Interface

    //~ Begin ICommittedCellArrayManager Interface
    STDMETHODIMP_(const GUID) GetCommittedItemAtCell(const int cellX, const int cellY) override;
    STDMETHODIMP GetCommittedItemBounds(REFGUID tileID, Geometry::CRect& rect) override;
    STDMETHODIMP GetCommittedItemsInRect(const Geometry::CRect& rect, CSet<GUID>* prgItemsInRect) override;
    STDMETHODIMP AddIgnoredCommittedItem(REFGUID tileID) override;
    STDMETHODIMP RemoveIgnoredCommittedItem(REFGUID tileID) override;
    //~ End ICommittedCellArrayManager Interface

    enum class NotifyItemRemoved
    {
        DoNotSendRemoveNotification = 0,
        SendRemoveNotification = 1
    };

private:
    CSimpleHashTable<ICellArrayManagerCallback*, Microsoft::WRL::ComPtr<ICellArrayManagerCallback>> _htCallbacks;
    Microsoft::WRL::ComPtr<ICellArray> _pCellArray;
    Geometry::CSize _sizeMaxBounds;
    CSimpleHashTable<GUID, Geometry::CRect> _htTileBounds;
    Microsoft::WRL::ComPtr<ICellArray> _pCommittedCellArray;
    CSimpleHashTable<GUID, Geometry::CRect> _htCommittedTileBounds;
    GUID m_removedItem;

    template <typename TLambda>
    void _EnumerateAllCells(TLambda lambda);

    HRESULT _GrowCellArrayToFitRect(const Geometry::CRect rcTileBoundsCells);
    void _NotifyTileBoundsChange(REFGUID tileID, const RECT rcTileBoundsCells);
    void _NotifyCellArrayBoundsChange(const Geometry::CRect rcLayoutBoundsCells);
    void _NotifyTileRemovedPending(REFGUID tileID);
    void _NotifyTileRemoved(REFGUID tileID);
    HRESULT _ShrinkInfiniteDimensionEdges();
    HRESULT _ShrinkInfiniteDimensionEdges(const Microsoft::WRL::ComPtr<ICellArray>& cellArray, bool* boundsChanged) const;
    HRESULT _EnsureWorkingWithTemporaryChanges();
    HRESULT _SaveExistingCellArray();
    void _ClearCommittedCellArray();
    HRESULT _RemoveItemInternal(REFGUID itemID, NotifyItemRemoved shouldNotifyItemRemoved);
};

template <typename TLambda>
void CCellArrayManager::_EnumerateAllCells(TLambda lambda)
{
    Geometry::CRect currentArrayBounds = _pCellArray->GetArrayBounds();

    for (int indexY = currentArrayBounds.top; indexY < currentArrayBounds.bottom; ++indexY)
    {
        for (int indexX = currentArrayBounds.left; indexX < currentArrayBounds.right; ++indexX)
        {
            if (!lambda(indexX, indexY, GetItemAtCell(indexX, indexY))) // Assumed return value usage
            {
                break;
            }
        }
    }
}

enum class AutoIgnoredItemArray
{
    Pending = 0,
    Committed = 1
};

class AutoIgnoredItem
{
public:
    AutoIgnoredItem();
    AutoIgnoredItem(const AutoIgnoredItem& other) = delete;
    ~AutoIgnoredItem();

    HRESULT SetIgnoredItem(ICellArrayManager* pCellArrayManager, REFGUID tileID, AutoIgnoredItemArray arrayToModify);

private:
    AutoIgnoredItemArray _arrayToModify;
    Microsoft::WRL::ComPtr<ICellArrayManager> _spCellArrayManager;
    GUID _tileID;
};
