#pragma once

#include "Common.h"

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
MIDL_INTERFACE("ebb3adda-cd0c-4d14-a198-6fb7dcd692e2")
ICuratedTilePrivate : utctc::ICuratedTile
{
    virtual std::shared_ptr<dsct::CuratedTile> STDMETHODCALLTYPE GetTransformerData() = 0;
};

class CuratedTile final
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRt>
        , Microsoft::WRL::FtmBase
        , utctc::ICuratedTile
        , ICuratedTilePrivate
    >
{
    InspectableClass(L"CuratedTileCollections.CuratedTile", BaseTrust);

public:
    HRESULT RuntimeClassInitialize(const std::shared_ptr<dsct::CuratedTile>& transformerTile);

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

    //~ Begin ICuratedTilePrivate Interface
    STDMETHODIMP_(std::shared_ptr<dsct::CuratedTile>) GetTransformerData() override;
    //~ End ICuratedTilePrivate Interface

private:
    std::shared_ptr<dsct::CuratedTile> _transformerTile;
};
}
