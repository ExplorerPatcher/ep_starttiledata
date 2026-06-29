#pragma once

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
struct CollectionContext;
}

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::Internal
{
class LayoutTile;
}

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
MIDL_INTERFACE("dfdbd59d-7e05-41e6-8820-6777608d561f")
IStartLayoutTileInitializationHandler : IUnknown
{
    virtual bool STDMETHODCALLTYPE CanInitializeTile(std::shared_ptr<Internal::LayoutTile> tile) = 0;
    virtual void STDMETHODCALLTYPE InitializeTile(std::shared_ptr<Internal::LayoutTile> tile) = 0;
    virtual void STDMETHODCALLTYPE UninitializeTile(std::shared_ptr<Internal::LayoutTile> tile) = 0;
};

class TileInitializationHandlerManager
{
public:
    TileInitializationHandlerManager(const std::shared_ptr<CollectionContext>& context);

    void InitializeTile(std::shared_ptr<Internal::LayoutTile> tile);
    void UninitializeTile(std::shared_ptr<Internal::LayoutTile> tile);

private:
    void InitializeKnownHandlers();

    std::vector<wil::com_ptr<IStartLayoutTileInitializationHandler>> _knownHandlers;
    std::shared_ptr<CollectionContext> _context;
};
}
