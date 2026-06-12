#pragma once

interface IItemLayoutResolver;
interface ILayoutTraversalOrder;
interface ITileGridMetricsCalculator;

enum TileSizingMode
{
    TileSizingMode_FixedSize = 0x0,
    TileSizingMode_EdgeToEdge = 0x1,
};

enum LayoutOrder
{
    LayoutOrder_Grid = 0,
    LayoutOrder_Linear = 1,
    LayoutOrder_Two = 2, // @Note: Added in 15063
};

EXTERN_C STDAPI SharedStartLayout_CreatePortraitLayoutResolver(IItemLayoutResolver** ppLayoutResolver);

EXTERN_C STDAPI SharedStartLayout_CreateDesktopPortraitLayoutResolver(IItemLayoutResolver** ppLayoutResolver);

EXTERN_C STDAPI SharedStartLayout_CreateGroupsLayoutResolver(IItemLayoutResolver** ppLayoutResolver);

EXTERN_C STDAPI SharedStartLayout_CreateTileGridMetricsCalculator(
    TileSizingMode tileSizingMode, const float displayWidth, const float displayHeight, const float screenDiagonal,
    ITileGridMetricsCalculator** tileSizeCalculator);

EXTERN_C STDAPI SharedStartLayout_CreateLayoutTraversalOrder(
    IItemLayoutResolver* layoutResolver, int maxGroupWidth, LayoutOrder order,
    ILayoutTraversalOrder** ppLayoutTraversalOrder);
