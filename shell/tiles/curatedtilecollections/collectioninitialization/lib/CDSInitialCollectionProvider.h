#pragma once

#include "InitializationPipelineObjectModel.h"

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
MIDL_INTERFACE("511a70c8-def9-4797-b01c-e65dbe591eca")
IInitialCollectionProvider : IUnknown
{
    virtual std::shared_ptr<Internal::LayoutRoot> STDMETHODCALLTYPE GetMatchingDefaultLayout(const SelectionData&) = 0;
    virtual const WCHAR* STDMETHODCALLTYPE GetProviderLoggingName() = 0;
};

class CDSLayoutProvider
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , IInitialCollectionProvider
    >
{
public:
    HRESULT RuntimeClassInitialize(const WCHAR* collectionName, const std::shared_ptr<CollectionContext>& context);

    //~ Begin WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::IInitialCollectionProvider Interface
    STDMETHODIMP_(std::shared_ptr<Internal::LayoutRoot>) GetMatchingDefaultLayout(const SelectionData& selectionData) override;
    STDMETHODIMP_(const WCHAR*) GetProviderLoggingName() override;
    //~ End WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::IInitialCollectionProvider Interface

private:
    std::shared_ptr<Internal::LayoutRoot> _layoutRoot;
};
}
