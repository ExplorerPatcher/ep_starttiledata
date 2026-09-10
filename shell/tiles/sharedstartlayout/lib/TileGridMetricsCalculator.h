#pragma once

#include <shtypes.h>

#include "../../sharedmodel/lib/SharedModelCommon.h"

class TileGridMetricsCalculator final
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , ITileGridMetricsCalculator
    >
{
    const float c_displayWidthPhablet;
    const float c_displayWidthSmallTablet;
    const float c_displayWidthMediumTablet;
    const float c_tileSizeFixedSize;
    const float c_tileSpacing;
    const float c_groupSpacing;
    const float c_minimumAppListWidth;
    const float c_screenDiagonalMediumPhone;

public:
    TileGridMetricsCalculator();
    ~TileGridMetricsCalculator() override;

    HRESULT RuntimeClassInitialize(
        const TileSizingMode tileSizingMode, const float displayWidth, const float displayHeight,
        const float screenDiagonal);

    //~ Begin ITileGridMetricsCalculator Interface
    STDMETHODIMP GetGroupWidthOptions(
        ULONG* groupWidthOptions, ULONG groupWidthOptionsBufferSize, ULONG* groupWidthOptionsWritten,
        ULONG* defaultGroupWidthOption, ULONG* numGroups, ULONG* numGroupsRotated) override;
    STDMETHODIMP GetTileSize(const DEVICE_SCALE_FACTOR scaleFactor, const ULONG groupWidth, float* tileSize) override;
    //~ End ITileGridMetricsCalculator Interface

private:
    TileSizingMode m_tileSizingMode;
    float m_displayWidth;
    float m_displayHeight;
    float m_screenDiagonal;
};