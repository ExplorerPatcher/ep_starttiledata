#pragma once

#include "../../collectioninitialization/lib/DataStoreCache_MoveMe.h"

namespace wf = ABI::Windows::Foundation;
namespace ut = ABI::WindowsInternal::Shell::UnifiedTile;
namespace utctc = ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections;

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
class CuratedTile final
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRt>
        , utctc::ICuratedTile
    >
{
    InspectableClass(L"CuratedTileCollections.CuratedTile", BaseTrust);

public:
    HRESULT RuntimeClassInitialize(
        const std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile>& transformerTile);

    //~ Begin utctc::ICuratedTile Interface
    STDMETHODIMP get_Identifier(ut::IUnifiedTileIdentifier** value) override;
    STDMETHODIMP get_UniqueId(GUID* value) override;
    STDMETHODIMP get_Location(wf::Point* value) override;
    STDMETHODIMP put_Location(wf::Point value) override;
    STDMETHODIMP get_Size(wf::Size* value) override;
    STDMETHODIMP put_Size(wf::Size value) override;
    STDMETHODIMP GetCustomProperty(HSTRING key, HSTRING* result) override;
    STDMETHODIMP HasCustomProperty(HSTRING key, boolean* result) override;
    STDMETHODIMP RemoveCustomProperty(HSTRING key) override;
    STDMETHODIMP SetCustomProperty(HSTRING key, HSTRING value) override;
    //~ End utctc::ICuratedTile Interface

private:
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile> _transformerTile;
};
}
