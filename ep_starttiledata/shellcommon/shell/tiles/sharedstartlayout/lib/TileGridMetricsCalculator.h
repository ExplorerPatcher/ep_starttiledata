#pragma once

#include <shtypes.h>
#include <wrl.h>

enum TileSizingMode
{
    TileSizingMode_FixedSize = 0x0,
    TileSizingMode_EdgeToEdge = 0x1,
};

MIDL_INTERFACE("4839e2d8-ff75-4643-9bc6-435ae7c12501")
ITileGridMetricsCalculator : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetGroupWidthOptions(ULONG*, ULONG, ULONG*, ULONG*, ULONG*, ULONG*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetTileSize(const DEVICE_SCALE_FACTOR, const ULONG, float*) = 0;
};

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