#include "pch.h"

#include "GenericTraversalOrder.h"
#include "ItemLayoutResolver.h"
#include "TileGridMetricsCalculator.h"

#include "../../sharedmodel/lib/SharedModelCommon.h"

using namespace Microsoft::WRL;

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
