#pragma once

#include "SharedStartModel.h"

EXTERN_C STDAPI SharedStartLayout_CreatePortraitLayoutResolver(IItemLayoutResolver** ppLayoutResolver);

EXTERN_C STDAPI SharedStartLayout_CreateDesktopPortraitLayoutResolver(IItemLayoutResolver** ppLayoutResolver);

EXTERN_C STDAPI SharedStartLayout_CreateGroupsLayoutResolver(IItemLayoutResolver** ppLayoutResolver);

EXTERN_C STDAPI SharedStartLayout_CreateTileGridMetricsCalculator(
    TileSizingMode tileSizingMode, const float displayWidth, const float displayHeight, const float screenDiagonal,
    ITileGridMetricsCalculator** tileSizeCalculator);

EXTERN_C STDAPI SharedStartLayout_CreateLayoutTraversalOrder(
    IItemLayoutResolver* layoutResolver, int maxGroupWidth, LayoutOrder order,
    ILayoutTraversalOrder** ppLayoutTraversalOrder);
