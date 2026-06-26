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

MIDL_INTERFACE("e329db7a-e2f4-4d74-f1b5-9d75b80a5e46")
IStartLayoutFactory : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE CreatePortraitLayoutResolver(IItemLayoutResolver**) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateDesktopPortraitLayoutResolver(IItemLayoutResolver**) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateGroupsLayoutResolver(IItemLayoutResolver**) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateLayoutTraversalOrder(
        IItemLayoutResolver*, int, LayoutOrder, ILayoutTraversalOrder**) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateTileGridMetricsCalculator(
        const TileSizingMode, const float, const float, const float, ITileGridMetricsCalculator**) = 0;
};

class DECLSPEC_UUID("7bd7ab1c-f2c5-60c2-8d00-c2e50336a954")
StartLayoutFactory;
