#include "pch.h"

#include "ItemLayoutChainDisplacement.h"

using namespace Microsoft::WRL;

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
