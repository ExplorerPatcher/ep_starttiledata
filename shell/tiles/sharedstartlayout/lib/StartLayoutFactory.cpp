#include "pch.h"

#include <WRL/Module.h>

#include "GenericTraversalOrder.h"
#include "ItemLayoutResolver.h"
#include "TileGridMetricsCalculator.h"
#include "../../sharedmodel/lib/SharedModelCommon.h"

using namespace Microsoft::WRL;

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
StartLayoutFactory final
    : public RuntimeClass<RuntimeClassFlags<ClassicCom>
        , FtmBase
        , IStartLayoutFactory
    >
{
public:
    //~ Begin IStartLayoutFactory Interface
    STDMETHODIMP CreatePortraitLayoutResolver(IItemLayoutResolver** ppLayoutResolver) override;
    STDMETHODIMP CreateDesktopPortraitLayoutResolver(IItemLayoutResolver** ppLayoutResolver) override;
    STDMETHODIMP CreateGroupsLayoutResolver(IItemLayoutResolver** ppLayoutResolver) override;
    STDMETHODIMP CreateLayoutTraversalOrder(
        IItemLayoutResolver* layoutResolver, int maxGroupWidth, LayoutOrder order,
        ILayoutTraversalOrder** ppLayoutTraversalOrder) override;
    STDMETHODIMP CreateTileGridMetricsCalculator(
        const TileSizingMode tileSizingMode, const float displayWidth, const float displayHeight,
        const float screenDiagonal, ITileGridMetricsCalculator** tileSizeCalculator) override;
    //~ End IStartLayoutFactory Interface
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

HRESULT StartLayoutFactory::CreatePortraitLayoutResolver(IItemLayoutResolver** ppLayoutResolver)
{
    RETURN_HR(SharedStartLayout_CreatePortraitLayoutResolver(ppLayoutResolver)); // 18
}

HRESULT StartLayoutFactory::CreateDesktopPortraitLayoutResolver(IItemLayoutResolver** ppLayoutResolver)
{
    RETURN_HR(SharedStartLayout_CreateDesktopPortraitLayoutResolver(ppLayoutResolver)); // 23
}

HRESULT StartLayoutFactory::CreateGroupsLayoutResolver(IItemLayoutResolver** ppLayoutResolver)
{
    RETURN_HR(SharedStartLayout_CreateGroupsLayoutResolver(ppLayoutResolver)); // 28
}

HRESULT StartLayoutFactory::CreateLayoutTraversalOrder(
    IItemLayoutResolver* layoutResolver, int maxGroupWidth, LayoutOrder order,
    ILayoutTraversalOrder** ppLayoutTraversalOrder)
{
    RETURN_HR(SharedStartLayout_CreateLayoutTraversalOrder(
        layoutResolver, maxGroupWidth, order, ppLayoutTraversalOrder)); // 36
}

HRESULT StartLayoutFactory::CreateTileGridMetricsCalculator(
    const TileSizingMode tileSizingMode, const float displayWidth, const float displayHeight,
    const float screenDiagonal, ITileGridMetricsCalculator** tileSizeCalculator)
{
    RETURN_HR(SharedStartLayout_CreateTileGridMetricsCalculator(
        tileSizingMode, displayWidth, displayHeight, screenDiagonal, tileSizeCalculator)); // 45
}

CoCreatableClass(StartLayoutFactory);
