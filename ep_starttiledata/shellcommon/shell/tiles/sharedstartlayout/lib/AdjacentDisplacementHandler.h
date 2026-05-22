#pragma once

#include "BaseDisplacementHandler.h"

enum SINGLE_TILE_ADJACENT_OPTION_FLAGS
{
    STAOF_NONE = 0,
    STAOF_PREFER_SHORTEST_DISPLACEMENT = 0x1,
    STAOF_DISPLACE_BLOCK_IS_CONTAINED_IN_TARGET = 0x2,
    STAOF_DISPLACE_INTO_NEGATIVE_SPACE = 0x4,
    STAOF_PREFER_SWAP_OVER_DIRECTION_PRIORITY = 0x8,
};

DEFINE_ENUM_FLAG_OPERATORS(SINGLE_TILE_ADJACENT_OPTION_FLAGS);

class CAdjacentDisplacementHandler
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , Microsoft::WRL::Implements<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, CBaseDisplacementHandler>
    >
{
public:
    CAdjacentDisplacementHandler();

    HRESULT RuntimeClassInitialize(
        CCoSimpleArray<DISPLACEMENT_DIRECTION>& directionPriorities, SINGLE_TILE_ADJACENT_OPTION_FLAGS options);

    //~ Begin IItemLayoutDisplacementHandler Interface
    STDMETHODIMP DisplaceItemsFromRect(const Geometry::CRect& targetRect, const Geometry::CRect& previousRect) override;
    //~ End IItemLayoutDisplacementHandler Interface

private:
    struct DisplaceResult
    {
        bool valid;
        Geometry::CPoint offset;
        int distanceMoved;
        Geometry::CRect displacementDestination;

        DisplaceResult();
    };

    DisplaceResult _TestDisplaceDirection(
        Geometry::CRect displaceTarget, DISPLACEMENT_DIRECTION direction, const Geometry::CRect& previousRect);
    DisplaceResult _ChoosePreferredResult(
        const CCoSimpleArray<DisplaceResult>& displacementOptions, const Geometry::CRect& previousRect);
    HRESULT _DisplaceTilesByResult(Geometry::CRect target, DisplaceResult displaceResult);
    HRESULT _IgnoreTiles(CSet<GUID>& tilesToIgnore);
    void _StopIgnoringTiles(CSet<GUID>& tilesToStopIgnoring);

    CCoSimpleArray<DISPLACEMENT_DIRECTION> m_directionPriorities;
    SINGLE_TILE_ADJACENT_OPTION_FLAGS m_options;
};
