#include "pch.h"

#include "CDSInitialCollectionProvider.h"

#include "TransformerHelpers.h"

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
HRESULT CDSLayoutProvider::RuntimeClassInitialize(const WCHAR* collectionName, const std::shared_ptr<CollectionContext>& context) try
{
    using namespace DataStoreCache::CuratedTileCollectionTransformer;

    std::shared_ptr<CuratedRoot> transformerRoot = TransformerHelpers::GetTransformerRoot(
        CollectionConstants::c_startTileGridCollectionId /*@MOD Force optimized arg, otherwise collectionName*/, context);
    _layoutRoot = std::make_shared<Internal::LayoutRootInternal>(context, transformerRoot);
    return S_OK;
} CATCH_RETURN() // 37

std::shared_ptr<Internal::LayoutRoot> CDSLayoutProvider::GetMatchingDefaultLayout(const SelectionData& selectionData)
{
    return _layoutRoot;
}

const WCHAR* CDSLayoutProvider::GetProviderLoggingName()
{
    return L"CDSLayoutProvider";
}
}

template
HRESULT __declspec(dllexport) Microsoft::WRL::Details::MakeAndInitialize<
    WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::CDSLayoutProvider,
    WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::IInitialCollectionProvider,
    const wchar_t (&)[15],
    const std::shared_ptr<WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::CollectionContext>&
>(
    WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::IInitialCollectionProvider**, const wchar_t (&)[15],
    const std::shared_ptr<WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::CollectionContext>&
);

// Verify the mangled name
__pragma(comment(linker, "/INCLUDE:??$MakeAndInitialize@VCDSLayoutProvider@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@UIInitialCollectionProvider@2345@AEAY0P@$$CBGAEBV?$shared_ptr@UCollectionContext@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@@std@@@Details@WRL@Microsoft@@YAJPEAPEAUIInitialCollectionProvider@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@AEAY0P@$$CBGAEBV?$shared_ptr@UCollectionContext@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@@std@@@Z"))
