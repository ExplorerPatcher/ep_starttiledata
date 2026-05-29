#pragma once

#include "ItemLayoutResolver.h"

class CPortraitTileLayoutResolver : public CItemLayoutResolver
{
public:
    CPortraitTileLayoutResolver();

    HRESULT RuntimeClassInitialize(LAYOUT_RESOLVER_OPTIONS options);

protected:
    //~ Begin CItemLayoutResolver Interface
    HRESULT _FindTargetDestinationForNewSize(REFGUID itemId, const SIZE& sizeItemCells, Geometry::CRect* rcItemBounds) override;
    IItemCellAssignor* _GetCellAssignor() override;
    HRESULT _PrepareLayoutBeforeOperation(const Geometry::CRect& rcSourceCells, const Geometry::CRect& rcTargetCells) override;
    HRESULT _CleanupLayoutAfterOperation(const Geometry::CRect& rcSourceCells, const Geometry::CRect& rcTargetCells) override;
    HRESULT _RepairLayout() override;
    HRESULT _ModifyItemUncommittedInternal(REFGUID itemID, const RECT& rcDestination, const ModificationOperation operation) override;
    HRESULT _CommitChangesInternal() override;
    //~ End CItemLayoutResolver Interface

private:
    bool _IsValidAndEmptyTargetRect(const Geometry::CRect& rcRect);

    Microsoft::WRL::ComPtr<IItemCellAssignor> _spCellAssignor;
    Microsoft::WRL::ComPtr<class CPathCollapseHandler> m_pathCollapseHandler;
    Geometry::CRect m_lastCleanupSource;
    Geometry::CRect m_lastCleanupTarget;
};
