#include "pch.h"

#include "AdjacentDisplacementHandler.h"

CAdjacentDisplacementHandler::CAdjacentDisplacementHandler()
    : m_options(STAOF_NONE)
{
}

HRESULT CAdjacentDisplacementHandler::RuntimeClassInitialize(
    CCoSimpleArray<DISPLACEMENT_DIRECTION>& directionPriorities, SINGLE_TILE_ADJACENT_OPTION_FLAGS options)
{
    m_options = options;
    m_directionPriorities.Merge(directionPriorities, SAMF_SORTED | SAMF_UNION);
    return S_OK;
}

HRESULT CAdjacentDisplacementHandler::DisplaceItemsFromRect(
    const Geometry::CRect& targetRect, const Geometry::CRect& previousRect)
{
    HRESULT hr = E_FAIL;

    CCoSimpleArray<DisplaceResult> displaceOptions;
    // for (size_t directionIndex = 0; directionIndex < m_directionPriorities.GetSize(); ++directionIndex)
    for (const DISPLACEMENT_DIRECTION& direction : m_directionPriorities) // @MOD Optimize
    {
        // DISPLACEMENT_DIRECTION direction;
        // if (SUCCEEDED(m_directionPriorities.GetAt(directionIndex, direction)))
        {
            displaceOptions.Add(_TestDisplaceDirection(targetRect, direction, previousRect));
        }
    }

    DisplaceResult preferredResult = _ChoosePreferredResult(displaceOptions, previousRect);
    if (preferredResult.valid)
    {
        hr = _DisplaceTilesByResult(targetRect, preferredResult);
    }

    return hr;
}

CAdjacentDisplacementHandler::DisplaceResult::DisplaceResult()
    : valid(false)
{
}

CAdjacentDisplacementHandler::DisplaceResult CAdjacentDisplacementHandler::_TestDisplaceDirection(
    Geometry::CRect displaceTarget, DISPLACEMENT_DIRECTION direction, const Geometry::CRect& previousRect)
{
    DisplaceResult displaceResult;

    CSet<GUID> itemsInRect;
    HRESULT hr = m_cellArrayManager->GetItemsInRect(displaceTarget, &itemsInRect);

    Geometry::CRect boundingRect;
    if (SUCCEEDED(hr))
    {
        hr = m_cellArrayManager->GetBoundingRectForItems(itemsInRect, boundingRect);
    }

    if (direction == DD_UP)
    {
        hr = boundingRect.bottom <= displaceTarget.bottom ? S_OK : E_FAIL;
    }
    else if (direction == DD_DOWN)
    {
        hr = boundingRect.top >= displaceTarget.top ? S_OK : E_FAIL;
    }
    else if (direction == DD_LEFT)
    {
        hr = boundingRect.right <= displaceTarget.right ? S_OK : E_FAIL;
    }
    else if (direction == DD_RIGHT)
    {
        hr = boundingRect.left >= displaceTarget.left ? S_OK : E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        displaceResult.displacementDestination = _GetAdjacentRect(displaceTarget, boundingRect, direction);

        Geometry::CRect occupyingRow = previousRect;
        occupyingRow.left = 0;
        occupyingRow.right = m_cellArrayManager->GetMaximumCellArrayDimensions().cx;

        CSet<GUID> otherItemsInRow;
        if (FAILED(m_cellArrayManager->GetItemsInRect(occupyingRow, &otherItemsInRow))
            || otherItemsInRow.GetCount() != 0
            || !occupyingRow.Contains(displaceResult.displacementDestination.GetTopLeft())
            || !occupyingRow.Contains(Geometry::CPoint(
                displaceResult.displacementDestination.bottom - 1, displaceResult.displacementDestination.right - 1)))
        {
            if ((m_options & STAOF_DISPLACE_BLOCK_IS_CONTAINED_IN_TARGET) != 0 && itemsInRect.GetCount() > 1)
            {
                if (displaceTarget.Contains(boundingRect.GetTopLeft())
                    && displaceTarget.Contains(Geometry::CPoint(boundingRect.right - 1, boundingRect.bottom - 1)))
                {
                    hr = S_OK;
                }
                else
                {
                    hr = E_FAIL;
                }
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        hr = _IgnoreTiles(itemsInRect);
    }
    if (SUCCEEDED(hr))
    {
        displaceResult.offset = Geometry::CPoint(displaceResult.displacementDestination.left - boundingRect.left, displaceResult.displacementDestination.top - boundingRect.top);
        displaceResult.distanceMoved = abs(abs(displaceResult.offset.x) > abs(displaceResult.offset.y) ? displaceResult.offset.x : displaceResult.offset.y);
        displaceResult.valid = m_cellArrayManager->IsValidRect(displaceResult.displacementDestination)
            && m_cellArrayManager->IsRectEmpty(displaceResult.displacementDestination)
            && ((displaceResult.displacementDestination.left >= 0 && displaceResult.displacementDestination.top >= 0)
                || (m_options & STAOF_DISPLACE_INTO_NEGATIVE_SPACE) != 0);
    }

    _StopIgnoringTiles(itemsInRect);
    return displaceResult;
}

CAdjacentDisplacementHandler::DisplaceResult CAdjacentDisplacementHandler::_ChoosePreferredResult(
    const CCoSimpleArray<DisplaceResult>& displacementOptions, const Geometry::CRect& previousRect)
{
    DisplaceResult preferredResult;
    // for (size_t optionIndex = 0; optionIndex < displacementOptions.GetSize(); ++optionIndex)
    for (const DisplaceResult& result : displacementOptions) // @MOD Optimize
    {
        // DisplaceResult result = displacementOptions[optionIndex];
        if (result.valid)
        {
            if ((m_options & STAOF_PREFER_SWAP_OVER_DIRECTION_PRIORITY) != 0
                && previousRect.GetWidth() > 0 && previousRect.GetHeight() > 0
                && result.displacementDestination == previousRect)
            {
                preferredResult = result;
                break;
            }

            if ((m_options & STAOF_PREFER_SHORTEST_DISPLACEMENT) != 0)
            {
                if (!preferredResult.valid || result.distanceMoved < preferredResult.distanceMoved)
                {
                    preferredResult = result;
                }
            }
            else
            {
                if (!preferredResult.valid)
                {
                    preferredResult = result;
                }
            }
        }
    }

    return preferredResult;
}

HRESULT CAdjacentDisplacementHandler::_DisplaceTilesByResult(Geometry::CRect target, DisplaceResult displaceResult)
{
    CSet<GUID> itemsInRect;
    HRESULT hr = m_cellArrayManager->GetItemsInRect(target, &itemsInRect);
    if (SUCCEEDED(hr))
    {
        hr = _IgnoreTiles(itemsInRect);
    }
    if (SUCCEEDED(hr))
    {
        itemsInRect.Enumerate([&hr, this, &displaceResult](REFGUID itemID) -> bool
        {
            Geometry::CRect itemBounds;
            if (SUCCEEDED(hr))
            {
                hr = m_cellArrayManager->GetItemBounds(itemID, itemBounds);
            }
            if (SUCCEEDED(hr))
            {
                itemBounds.Offset(displaceResult.offset.x, displaceResult.offset.y);
                hr = m_cellArrayManager->MoveItemUncommitted(itemID, itemBounds);
            }
            return SUCCEEDED(hr);
        });
    }

    _StopIgnoringTiles(itemsInRect);
    return hr;
}

HRESULT CAdjacentDisplacementHandler::_IgnoreTiles(CSet<GUID>& tilesToIgnore)
{
    HRESULT hr = S_OK;
    tilesToIgnore.Enumerate([&hr, this](REFGUID itemID) -> bool
    {
        hr = m_cellArrayManager->AddIgnoredItem(itemID);
        return SUCCEEDED(hr);
    });
    return hr;
}

void CAdjacentDisplacementHandler::_StopIgnoringTiles(CSet<GUID>& tilesToStopIgnoring)
{
    tilesToStopIgnoring.Enumerate([this](REFGUID itemID) -> bool
    {
        m_cellArrayManager->RemoveIgnoredItem(itemID);
        return true;
    });
}
