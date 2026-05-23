#include "pch.h"

#include "GenericTraversalOrder.h"
#include "GridTraversalOrder.h"
#include "GroupsLayoutResolver.h"
#include "ItemLayoutResolver.h"
#include "LinearTraversalOrder.h"
#include "PortraitTileLayoutResolver.h"
#include "TileGridMetricsCalculator.h"

#include "../../sharedmodel/lib/SharedModelCommon.h"

using namespace Microsoft::WRL;

EXTERN_C STDAPI SharedStartLayout_CreatePortraitLayoutResolver(IItemLayoutResolver** ppLayoutResolver)
{
    *ppLayoutResolver = nullptr;

    ComPtr<IItemLayoutResolver> spLayoutResolver;
    RETURN_IF_FAILED(MakeAndInitialize<CPortraitTileLayoutResolver>(&spLayoutResolver, LRO_DISPLACE_INTO_NEGATIVE_SPACE)); // 17
    *ppLayoutResolver = spLayoutResolver.Detach();
    return S_OK;
}

EXTERN_C STDAPI SharedStartLayout_CreateDesktopPortraitLayoutResolver(IItemLayoutResolver** ppLayoutResolver)
{
    *ppLayoutResolver = nullptr;

    ComPtr<IItemLayoutResolver> spLayoutResolver;
    RETURN_IF_FAILED(MakeAndInitialize<CPortraitTileLayoutResolver>(&spLayoutResolver, LRO_NONE)); // 27
    *ppLayoutResolver = spLayoutResolver.Detach();
    return S_OK;
}

EXTERN_C STDAPI SharedStartLayout_CreateGroupsLayoutResolver(IItemLayoutResolver** ppLayoutResolver)
{
    *ppLayoutResolver = nullptr;

    ComPtr<IItemLayoutResolver> layoutResolver;
    RETURN_IF_FAILED(MakeAndInitialize<CGroupsLayoutResolver>(&layoutResolver)); // 37

    static constexpr RECT c_GroupsLayoutResolverContainerMargins = { 0, 1, 0, 0 };

    RETURN_IF_FAILED(layoutResolver->SetContainerMargins(c_GroupsLayoutResolverContainerMargins)); // 41
    *ppLayoutResolver = layoutResolver.Detach();
    return S_OK;
}

EXTERN_C STDAPI SharedStartLayout_CreateLayoutTraversalOrder(
    IItemLayoutResolver* layoutResolver, int maxGroupWidth, LayoutOrder order,
    ILayoutTraversalOrder** ppLayoutTraversalOrder)
{
    *ppLayoutTraversalOrder = nullptr;

    ComPtr<ILayoutTraversalOrder> spLayoutTraversalOrder;
    switch (order)
    {
        case LayoutOrder_Grid:
            RETURN_IF_FAILED(MakeAndInitialize<CGridTraversalOrder>(&spLayoutTraversalOrder, layoutResolver, maxGroupWidth)); // 56
            break;
        case LayoutOrder_Two:
            RETURN_IF_FAILED(MakeAndInitialize<CGridTraversalOrder>(&spLayoutTraversalOrder, layoutResolver, -1)); // 59
            break;
        case LayoutOrder_Linear:
            RETURN_IF_FAILED(MakeAndInitialize<CLinearTraversalOrder>(&spLayoutTraversalOrder, layoutResolver, maxGroupWidth)); // 62
            break;
    }

    *ppLayoutTraversalOrder = spLayoutTraversalOrder.Detach();
    return S_OK;
}

EXTERN_C STDAPI SharedStartLayout_CreateTileGridMetricsCalculator(
    TileSizingMode tileSizingMode, const float displayWidth, const float displayHeight, const float screenDiagonal,
    ITileGridMetricsCalculator** tileSizeCalculator)
{
    *tileSizeCalculator = nullptr;

    ComPtr<ITileGridMetricsCalculator> spTileGridMetricsCalculator;
    RETURN_IF_FAILED(MakeAndInitialize<TileGridMetricsCalculator>(
        &spTileGridMetricsCalculator, tileSizingMode, displayWidth, displayHeight, screenDiagonal)); // 77
    *tileSizeCalculator = spTileGridMetricsCalculator.Detach();
    return S_OK;
}
