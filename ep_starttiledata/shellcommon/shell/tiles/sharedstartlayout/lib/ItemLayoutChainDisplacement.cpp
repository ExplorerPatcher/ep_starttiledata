#include "pch.h"

#include "ItemLayoutChainDisplacement.h"

using namespace Microsoft::WRL;

CItemLayoutChainDisplacement::CItemLayoutChainDisplacement()
{
}

HRESULT CItemLayoutChainDisplacement::RuntimeClassInitialize(
    CSimpleHashTable<UINT, RefCountedDirectionArray>* directionPrioritySets,
    int deltaOfPriorityLengthOverShortestDistance, CHAIN_DISPLACEMENT_OPTION_FLAGS options)
{
    directionPrioritySets->Enum([this](UINT key, RefCountedDirectionArray value) -> bool
    {
        m_directionPrioritySets.AddItem(key, value);
        return true;
    });
    m_deltaOfPriorityLengthOverShortestDistance = deltaOfPriorityLengthOverShortestDistance;
    m_options = options;
    return S_OK;
}

HRESULT CItemLayoutChainDisplacement::DisplaceItemsFromRect(
    const Geometry::CRect& targetRect, const Geometry::CRect& previousRect)
{
    HRESULT hr = E_FAIL;

    Geometry::CRect currentBounds = m_cellArrayManager->GetCurrentCellArrayBounds();
    if (previousRect.GetSize() == targetRect.GetSize() && currentBounds.Contains(targetRect.GetTopLeft()))
    {
        int bestTotalDistance = 0;
        CCoSimpleArray<ComPtr<CRefCountedObject<ChainLink>>> chosenChainLinks;
        for (UINT directionSetIndex = 0; directionSetIndex < m_directionPrioritySets.GetItemCount(); ++directionSetIndex)
        {
            ComPtr<CRefCountedObject<CCoSimpleArray<DISPLACEMENT_DIRECTION>>> directions;
            if (SUCCEEDED(m_directionPrioritySets.GetItem(directionSetIndex, directions)))
            {
                for (size_t directionIndex = 0; directionIndex < directions->GetSize(); ++directionIndex)
                {
                    DISPLACEMENT_DIRECTION direction;
                    if (SUCCEEDED(directions->GetAt(directionIndex, direction)))
                    {
                        CCoSimpleArray<ComPtr<CRefCountedObject<ChainLink>>> chainLinks;
                        int chainDistance = 0;
                        if (SUCCEEDED(_TryToCreateChain(targetRect, direction, &chainLinks, &chainDistance))
                            && (bestTotalDistance == 0 || m_deltaOfPriorityLengthOverShortestDistance + abs(chainDistance) < abs(bestTotalDistance)))
                        {
                            bestTotalDistance = chainDistance;
                            chosenChainLinks.TransferData(&chainLinks);
                            hr = S_OK;
                        }
                    }
                }
            }
            if (SUCCEEDED(hr))
            {
                break;
            }
        }

        if (SUCCEEDED(hr))
        {
            for (size_t chainLinkIndex = 0; chainLinkIndex < chosenChainLinks.GetSize(); ++chainLinkIndex)
            {
                ComPtr<CRefCountedObject<ChainLink>> link;
                chosenChainLinks.GetAt(chainLinkIndex, link);

                link->setTilesToMove.Enumerate([&](REFGUID itemID) -> bool
                {
                    m_cellArrayManager->AddIgnoredItem(itemID);
                    return true;
                });

                link->setTilesToMove.Enumerate([&](REFGUID itemID) -> bool
                {
                    Geometry::CRect itemBounds;
                    if (SUCCEEDED(m_cellArrayManager->GetItemBounds(itemID, itemBounds)))
                    {
                        itemBounds.Offset(link->DisplacementVector.x, link->DisplacementVector.y);
                        m_cellArrayManager->MoveItemUncommitted(itemID, itemBounds);
                    }
                    return true;
                });

                link->setTilesToMove.Enumerate([this](REFGUID itemID) -> bool
                {
                    m_cellArrayManager->RemoveIgnoredItem(itemID);
                    return true;
                });
            }
        }
    }

    return hr;
}

HRESULT CItemLayoutChainDisplacement::_TryToCreateChain(
    Geometry::CRect target, DISPLACEMENT_DIRECTION direction, CCoSimpleArray<RefCountedChainLink>* chainLinks,
    int* totalChainDistance)
{
    CSet<GUID> tilesInTargetRect;
    HRESULT hr = m_cellArrayManager->GetItemsInRect(target, &tilesInTargetRect);
    RETURN_HR_IF_EXPECTED(E_FAIL, (m_options & CDOF_DISPLACE_ONLY_SAME_SIZED_ITEMS) != 0 && tilesInTargetRect.GetCount() > 1);

    Geometry::CRect tilesInTargetBoundingRect;
    if (SUCCEEDED(hr))
    {
        hr = m_cellArrayManager->GetBoundingRectForItems(tilesInTargetRect, tilesInTargetBoundingRect);
    }
    RETURN_HR_IF_EXPECTED(E_FAIL, (m_options & CDOF_DISPLACE_ONLY_SAME_SIZED_ITEMS) != 0 && tilesInTargetBoundingRect.GetSize() != target.GetSize());

    CSet<GUID> tilesToDisplace;
    if (SUCCEEDED(hr))
    {
        hr = m_cellArrayManager->GetItemsInRect(tilesInTargetBoundingRect, &tilesToDisplace);
    }

    CSet<GUID> tilesToRemoveFromDisplacementSet;
    if (SUCCEEDED(hr))
    {
        tilesToDisplace.Enumerate([&](REFGUID itemID) -> bool
        {
            Geometry::CRect tileBounds;
            if (SUCCEEDED(m_cellArrayManager->GetItemBounds(itemID, tileBounds))
                && !tileBounds.Intersects(target)
                && (   direction == DD_LEFT  && tileBounds.left   >= target.right
                    || direction == DD_RIGHT && tileBounds.right  <= target.left
                    || direction == DD_UP    && tileBounds.top    >= target.bottom
                    || direction == DD_DOWN  && tileBounds.bottom <= target.top))
            {
                hr = tilesToRemoveFromDisplacementSet.Add(itemID);
            }
            return SUCCEEDED(hr);
        });
    }

    if (SUCCEEDED(hr))
    {
        tilesToRemoveFromDisplacementSet.Enumerate([&](REFGUID itemID) -> bool
        {
            hr = tilesToDisplace.Remove(itemID);
            return SUCCEEDED(hr);
        });
    }

    if (SUCCEEDED(hr))
    {
        Geometry::CRect adjacentRect = _GetAdjacentRect(target, tilesInTargetBoundingRect, direction);

        POINT displacementVector;
        displacementVector.x = adjacentRect.left - tilesInTargetBoundingRect.left;
        displacementVector.y = adjacentRect.top - tilesInTargetBoundingRect.top;

        CSet<GUID> tilesToIgnore;
        tilesToIgnore.Add(tilesToDisplace);
        tilesToIgnore.Enumerate([&](const GUID itemID) -> bool
        {
            m_cellArrayManager->AddIgnoredItem(itemID);
            return true;
        });

        if (m_cellArrayManager->IsValidRect(adjacentRect)
            && (m_cellArrayManager->IsRectEmpty(adjacentRect)
                || SUCCEEDED(_TryToCreateChain(adjacentRect, direction, chainLinks, totalChainDistance))))
        {
            RefCountedChainLink newLink = CreateRefCountedObj<ChainLink>();
            newLink->setTilesToMove.Swap(&tilesToDisplace);
            newLink->DisplacementVector = displacementVector;
            chainLinks->Add(newLink);

            *totalChainDistance += direction == DD_UP || direction == DD_DOWN ? displacementVector.y : displacementVector.x;
        }
        else
        {
            hr = E_FAIL;
        }

        tilesToIgnore.Enumerate([&](REFGUID itemID) -> bool
        {
            m_cellArrayManager->RemoveIgnoredItem(itemID);
            return true;
        });
    }

    return hr;
}
