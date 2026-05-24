#pragma once

#include "BaseDisplacementHandler.h"

#include "Geometry.h"
#include "RefCountedObject.h"
#include "Set.h"

enum CHAIN_DISPLACEMENT_OPTION_FLAGS
{
    CDOF_NONE = 0x0,
    CDOF_DISPLACE_ONLY_SAME_SIZED_ITEMS = 0x1,
};

class CItemLayoutChainDisplacement
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , Microsoft::WRL::Implements<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
            , CBaseDisplacementHandler
        >
    >
{
public:
    CItemLayoutChainDisplacement();

    HRESULT RuntimeClassInitialize(
        CSimpleHashTable<UINT, RefCountedDirectionArray>* directionPrioritySets,
        int deltaOfPriorityLengthOverShortestDistance, CHAIN_DISPLACEMENT_OPTION_FLAGS options);

    //~ Begin IItemLayoutDisplacementHandler Interface
    STDMETHODIMP DisplaceItemsFromRect(const Geometry::CRect& targetRect, const Geometry::CRect& previousRect) override;
    //~ End IItemLayoutDisplacementHandler Interface

    struct ChainLink
    {
        CSet<GUID> setTilesToMove;
        Geometry::CPoint DisplacementVector;
    };

    using RefCountedChainLink = Microsoft::WRL::ComPtr<CRefCountedObject<ChainLink>>;

private:
    CSimpleHashTable<UINT, RefCountedDirectionArray> m_directionPrioritySets;
    int m_deltaOfPriorityLengthOverShortestDistance;
    CHAIN_DISPLACEMENT_OPTION_FLAGS m_options;

    HRESULT _TryToCreateChain(
        Geometry::CRect target, DISPLACEMENT_DIRECTION direction, CCoSimpleArray<RefCountedChainLink>* chainLinks,
        int* totalChainDistance);
};
