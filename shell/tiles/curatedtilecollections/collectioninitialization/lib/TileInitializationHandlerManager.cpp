#include "pch.h"

#include "TileInitializationHandlerManager.h"

#include "InitializationPipelineObjectModel.h"

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
TileInitializationHandlerManager::TileInitializationHandlerManager(const std::shared_ptr<CollectionContext>& context)
    : _context(context)
{
    InitializeKnownHandlers();
}

void TileInitializationHandlerManager::InitializeTile(std::shared_ptr<Internal::LayoutTile> tile)
{
    for (const auto& handler : _knownHandlers)
    {
        if (handler->CanInitializeTile(tile))
        {
            handler->InitializeTile(tile);
        }
    }
}

void TileInitializationHandlerManager::UninitializeTile(std::shared_ptr<Internal::LayoutTile> tile)
{
    for (const auto& handler : _knownHandlers)
    {
        if (handler->CanInitializeTile(tile))
        {
            handler->UninitializeTile(tile);
        }
    }
}

void TileInitializationHandlerManager::InitializeKnownHandlers()
{
    /*wil::com_ptr<IStartLayoutTileInitializationHandler> secondaryTileInitializationHandler;
    THROW_IF_FAILED(Microsoft::WRL::MakeAndInitialize<SecondaryTileInitializationHandler>(
        &secondaryTileInitializationHandler, _context)); // 48
    _knownHandlers.emplace_back(secondaryTileInitializationHandler);

    wil::com_ptr<IStartLayoutTileInitializationHandler> userPinnedAppResolverTileInitializationHandler;
    THROW_IF_FAILED(Microsoft::WRL::MakeAndInitialize<UserPinnedAppResolverTileInitializationHandler>(
        &userPinnedAppResolverTileInitializationHandler, _context)); // 52
    _knownHandlers.emplace_back(userPinnedAppResolverTileInitializationHandler);
    _context->_userPinnedAppResolverTileInitializationHandler = userPinnedAppResolverTileInitializationHandler;

    wil::com_ptr<IStartLayoutTileInitializationHandler> unresolvedLinkPathTileInitializationHandler;
    THROW_IF_FAILED(Microsoft::WRL::MakeAndInitialize<UnresolvedLinkPathTileInitializationHandler>(
        &unresolvedLinkPathTileInitializationHandler)); // 57
    _knownHandlers.emplace_back(unresolvedLinkPathTileInitializationHandler);*/ // Uncomment when needed
}
}
