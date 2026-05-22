#include "pch.h"

#include "PortraitTileLayoutResolver.h"

#include "AdjacentDisplacementHandler.h"
#include "BaseDisplacementHandler.h"
#include "CompoundDisplacementHandler.h"
#include "EmptyCellDisplacementHandler.h"
#include "ExpandDisplacementHandler.h"
#include "PathCollapseHandler.h"
#include "PortraitTileCellAssignor.h"
#include "PushDownDisplacementHandler.h"

using namespace Microsoft::WRL;

CPortraitTileLayoutResolver::CPortraitTileLayoutResolver()
{
}

HRESULT CPortraitTileLayoutResolver::RuntimeClassInitialize(LAYOUT_RESOLVER_OPTIONS options)
{
    _options = options;
    CItemLayoutResolver::RuntimeClassInitialize();

    RETURN_IF_FAILED(_displacementManager.AddDisplacementHandler(Make<CEmptyCellDisplacementHandler>().Get())); // 23

    CCoSimpleArray<DISPLACEMENT_DIRECTION> rgDirectionPriorities;
    RETURN_IF_FAILED(rgDirectionPriorities.Add(DD_RIGHT)); // 26
    RETURN_IF_FAILED(rgDirectionPriorities.Add(DD_LEFT)); // 27
    RETURN_IF_FAILED(rgDirectionPriorities.Add(DD_DOWN)); // 28
    RETURN_IF_FAILED(rgDirectionPriorities.Add(DD_UP)); // 29
    ComPtr<CAdjacentDisplacementHandler> spAdjacentDisplacementHandler;
    RETURN_IF_FAILED(MakeAndInitialize<CAdjacentDisplacementHandler>(&spAdjacentDisplacementHandler, rgDirectionPriorities, STAOF_PREFER_SHORTEST_DISPLACEMENT | STAOF_DISPLACE_BLOCK_IS_CONTAINED_IN_TARGET | STAOF_PREFER_SWAP_OVER_DIRECTION_PRIORITY)); // 31
    RETURN_IF_FAILED(_displacementManager.AddDisplacementHandler(spAdjacentDisplacementHandler.Get())); // 32

    ComPtr<CCompoundDisplacementHandler> compoundDisplacementHandler;
    RETURN_IF_FAILED(MakeAndInitialize<CCompoundDisplacementHandler>(&compoundDisplacementHandler)); // 36
    RETURN_IF_FAILED(_displacementManager.AddDisplacementHandler(compoundDisplacementHandler.Get())); // 37

    ComPtr<CExpandDisplacementHandler> spExpandDisplacementHandler;
    RETURN_IF_FAILED(MakeAndInitialize<CExpandDisplacementHandler>(&spExpandDisplacementHandler, EXPAND_COLLAPSE_DIRECTION_ROW)); // 41
    RETURN_IF_FAILED(_displacementManager.AddDisplacementHandler(spExpandDisplacementHandler.Get())); // 42

    ComPtr<CPushDownDisplacementHandler> spPushDownDisplacementHandler;
    RETURN_IF_FAILED(MakeAndInitialize<CPushDownDisplacementHandler>(&spPushDownDisplacementHandler)); // 45
    RETURN_IF_FAILED(_displacementManager.AddDisplacementHandler(spPushDownDisplacementHandler.Get())); // 46

    RETURN_IF_FAILED(MakeAndInitialize<CPathCollapseHandler>(&m_pathCollapseHandler, EXPAND_COLLAPSE_DIRECTION_ROW)); // 48
    RETURN_IF_FAILED(_collapseManager.AddCollapseHandler(m_pathCollapseHandler.Get())); // 49

    return S_OK;
}

HRESULT CPortraitTileLayoutResolver::_FindTargetDestinationForNewSize(
    REFGUID itemId, const SIZE& sizeItemCells, Geometry::CRect* rcItemBounds) // @MOD 5x Geometry::CPoint::{ctor} used instead of 2x
{
    HRESULT hr = _spCellArrayManager->GetItemBounds(itemId, *rcItemBounds);
    if (SUCCEEDED(hr) && (rcItemBounds->GetWidth() != sizeItemCells.cx || rcItemBounds->GetHeight() != sizeItemCells.cy))
    {
        hr = _spCellArrayManager->AddIgnoredItem(itemId);

        if (SUCCEEDED(hr))
        {
            bool fFound = false;

            Geometry::CRect rcTentativeBounds = Geometry::CRect(
                Geometry::CPoint(rcItemBounds->left, rcItemBounds->top),
                sizeItemCells);
            if (_IsValidAndEmptyTargetRect(rcTentativeBounds))
            {
                fFound = true;
            }

            if (!fFound && rcItemBounds->GetWidth() < sizeItemCells.cx)
            {
                rcTentativeBounds = Geometry::CRect(
                    Geometry::CPoint(rcItemBounds->right - sizeItemCells.cx, rcItemBounds->top),
                    sizeItemCells);
                if (_IsValidAndEmptyTargetRect(rcTentativeBounds))
                {
                    fFound = true;
                }
            }

            if (!fFound && rcItemBounds->GetHeight() < sizeItemCells.cy)
            {
                rcTentativeBounds = Geometry::CRect(
                    Geometry::CPoint(rcItemBounds->left, rcItemBounds->bottom - sizeItemCells.cy),
                    sizeItemCells);
                if (_IsValidAndEmptyTargetRect(rcTentativeBounds))
                {
                    fFound = true;
                }
            }

            if (!fFound && rcItemBounds->GetWidth() < sizeItemCells.cx && rcItemBounds->GetHeight() < sizeItemCells.cy)
            {
                rcTentativeBounds = Geometry::CRect(
                    Geometry::CPoint(rcItemBounds->right - sizeItemCells.cx, rcItemBounds->bottom - sizeItemCells.cy),
                    sizeItemCells);
                if (_IsValidAndEmptyTargetRect(rcTentativeBounds))
                {
                    fFound = true;
                }
            }

            if (!fFound)
            {
                rcTentativeBounds = Geometry::CRect(
                    Geometry::CPoint(rcItemBounds->left, rcItemBounds->top),
                    sizeItemCells);
            }

            *rcItemBounds = rcTentativeBounds;
        }

        _spCellArrayManager->RemoveIgnoredItem(itemId);
    }

    return hr;
}

IItemCellAssignor* CPortraitTileLayoutResolver::_GetCellAssignor()
{
    if (_spCellAssignor == nullptr)
    {
        _spCellAssignor = Make<CPortraitTileCellAssignor>();
        _spCellAssignor->SetCellArray(_spCellArrayManager.Get());
    }
    return _spCellAssignor.Get();
}

HRESULT CPortraitTileLayoutResolver::_PrepareLayoutBeforeOperation(
    const Geometry::CRect& rcSourceCells, const Geometry::CRect& rcTargetCells)
{
    return _Collapse(rcSourceCells, rcTargetCells);
}

HRESULT CPortraitTileLayoutResolver::_CleanupLayoutAfterOperation(
    const Geometry::CRect& rcSourceCells, const Geometry::CRect& rcTargetCells)
{
    m_lastCleanupSource = rcSourceCells;
    m_lastCleanupTarget = rcTargetCells;

    RETURN_IF_FAILED(_Collapse(rcSourceCells, rcTargetCells)); // 122

    if ((_options & LRO_DISPLACE_INTO_NEGATIVE_SPACE) == 0)
    {
        RETURN_IF_FAILED(_spCellArrayManager->FixCoordinatesToBeNonNegative()); // 126
    }

    return S_OK;
}

HRESULT CPortraitTileLayoutResolver::_RepairLayout()
{
    RETURN_IF_FAILED(_CleanupLayoutAfterOperation(Geometry::CRect(), Geometry::CRect())); // 134
    RETURN_IF_FAILED(_spCellArrayManager->FixCoordinatesToBeNonNegative()); // 135
    return S_OK;
}

HRESULT CPortraitTileLayoutResolver::_ModifyItemUncommittedInternal(
    REFGUID itemID, const RECT& rcDestination, const ModificationOperation operation)
{
    if (operation != ModificationOperation::Remove && operation != ModificationOperation::Resize)
    {
        m_pathCollapseHandler->DisableSmartCollapse();
    }

    HRESULT hr = CItemLayoutResolver::_ModifyItemUncommittedInternal(itemID, rcDestination, operation);

    m_pathCollapseHandler->EnableSmartCollapse();
    return hr;
}

HRESULT CPortraitTileLayoutResolver::_CommitChangesInternal()
{
    RETURN_IF_FAILED(_Collapse(m_lastCleanupSource, m_lastCleanupTarget)); // 160

    m_lastCleanupSource = Geometry::CRect();
    m_lastCleanupTarget = Geometry::CRect();

    return CItemLayoutResolver::_CommitChangesInternal();
}

bool CPortraitTileLayoutResolver::_IsValidAndEmptyTargetRect(const Geometry::CRect& rcRect)
{
    return _spCellArrayManager->IsValidRect(rcRect) && _spCellArrayManager->IsRectEmpty(rcRect);
}
