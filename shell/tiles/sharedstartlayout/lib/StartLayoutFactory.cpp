#include "pch.h"

#include <WRL/Module.h>

#include "GenericTraversalOrder.h"
#include "ItemLayoutResolver.h"
#include "TileGridMetricsCalculator.h"
#include "../../sharedmodel/lib/SharedModelCommon.h"

using namespace Microsoft::WRL;

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
