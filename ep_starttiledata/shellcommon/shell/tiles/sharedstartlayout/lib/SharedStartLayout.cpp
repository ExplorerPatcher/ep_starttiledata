#include "pch.h"

#include "TileGridMetricsCalculator.h"

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