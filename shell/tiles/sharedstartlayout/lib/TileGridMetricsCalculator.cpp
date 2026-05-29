#include "pch.h"

#include "TileGridMetricsCalculator.h"

using namespace Microsoft::WRL;

TileGridMetricsCalculator::TileGridMetricsCalculator()
    : c_displayWidthPhablet(400.0f)
    , c_displayWidthSmallTablet(500.0f)
    , c_displayWidthMediumTablet(1280.0f)
    , c_tileSizeFixedSize(48.0f)
    , c_tileSpacing(4.0f)
    , c_groupSpacing(24.0f)
    , c_minimumAppListWidth(204.0f)
    , c_screenDiagonalMediumPhone(4.0f)
{
}

TileGridMetricsCalculator::~TileGridMetricsCalculator()
{
}

HRESULT TileGridMetricsCalculator::RuntimeClassInitialize(
    const TileSizingMode tileSizingMode, const float displayWidth, const float displayHeight,
    const float screenDiagonal)
{
    RETURN_HR_IF(E_INVALIDARG, tileSizingMode == TileSizingMode_FixedSize || tileSizingMode == TileSizingMode_EdgeToEdge); // 19
    RETURN_HR_IF(E_INVALIDARG, displayWidth <= 0.0f); // 20
    RETURN_HR_IF(E_INVALIDARG, displayHeight <= 0.0f); // 21
    RETURN_HR_IF(E_INVALIDARG, screenDiagonal <= 0.0f); // 22

    m_displayWidth = displayWidth;
    m_displayHeight = displayHeight;
    m_screenDiagonal = screenDiagonal;
    return S_OK;
}

HRESULT TileGridMetricsCalculator::GetGroupWidthOptions(
    ULONG* groupWidthOptions, ULONG groupWidthOptionsBufferSize, ULONG* groupWidthOptionsWritten,
    ULONG* defaultGroupWidthOption, ULONG* numGroups, ULONG* numGroupsRotated)
{
    RETURN_HR_IF(E_INVALIDARG, !groupWidthOptions); // 36
    RETURN_HR_IF(E_INVALIDARG, !groupWidthOptionsWritten); // 37
    RETURN_HR_IF(E_INVALIDARG, !defaultGroupWidthOption); // 38
    RETURN_HR_IF(E_INVALIDARG, !numGroups); // 39
    RETURN_HR_IF(E_INVALIDARG, !numGroupsRotated); // 40

    if (m_tileSizingMode == TileSizingMode_FixedSize)
    {
        RETURN_HR_IF(E_BOUNDS, !groupWidthOptionsBufferSize); // 46

        if (c_displayWidthMediumTablet <= (double)m_displayWidth)
        {
            groupWidthOptions[0] = 8;
        }
        else
        {
            groupWidthOptions[0] = 6;
        }

        *defaultGroupWidthOption = groupWidthOptions[0];
        *groupWidthOptionsWritten = 1;

        *numGroups = (int)((m_displayWidth - (float)c_minimumAppListWidth - (float)c_groupSpacing)
            / ((float)(groupWidthOptions[0] - 1)
                * (float)c_tileSpacing + (float)groupWidthOptions[0]
                * (float)c_tileSizeFixedSize + (float)c_groupSpacing));

        *numGroupsRotated = (int)((m_displayHeight - (float)c_minimumAppListWidth - (float)c_groupSpacing)
            / ((float)(groupWidthOptions[0] - 1)
                * (float)c_tileSpacing + (float)groupWidthOptions[0]
                * (float)c_tileSizeFixedSize + (float)c_groupSpacing));
    }
    else
    {
        RETURN_HR_IF(E_BOUNDS, groupWidthOptionsBufferSize < 2); // 74

        if (c_displayWidthPhablet > (double)m_displayWidth)
        {
            groupWidthOptions[0] = 4;
            groupWidthOptions[1] = 6;
        }
        else
        {
            groupWidthOptions[0] = 6;
            groupWidthOptions[1] = 8;
        }

        if (c_displayWidthSmallTablet > (double)m_displayWidth)
        {
            *defaultGroupWidthOption = 8;
        }
        else
        {
            *defaultGroupWidthOption = 6;
        }

        *groupWidthOptionsWritten = 2;
        *numGroups = 1;
        *numGroupsRotated = 1;
    }

    return S_OK;
}

HRESULT TileGridMetricsCalculator::GetTileSize(
    const DEVICE_SCALE_FACTOR scaleFactor, const ULONG groupWidth, float* tileSize)
{
    RETURN_HR_IF(E_INVALIDARG, !groupWidth); // 118
    RETURN_HR_IF(E_INVALIDARG, !tileSize); // 119

    if (m_tileSizingMode == TileSizingMode_FixedSize)
    {
        *tileSize = c_tileSizeFixedSize;
    }
    else
    {
        *tileSize = 100.0f * (float)(int)((float)scaleFactor / 100.0f * (m_displayWidth - (float)(groupWidth + 1)
            * (float)c_tileSpacing) / (float)groupWidth) / (float)scaleFactor;
    }

    return S_OK;
}