#pragma once

#include "Common.h"

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
interface ICuratedTilePrivate;

MIDL_INTERFACE("6f3e1834-00c0-4e8b-8834-89da30e185e9")
ICuratedTileGroupPrivate : utctc::ICuratedTileGroup
{
    virtual HRESULT STDMETHODCALLTYPE AddTile(ICuratedTilePrivate*) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddGroup(ICuratedTileGroupPrivate*) = 0;
    virtual std::shared_ptr<dsct::CuratedGroup> STDMETHODCALLTYPE GetTransformerData() = 0;
};

class CuratedTileGroup final
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRt>
        , Microsoft::WRL::FtmBase
        , utctc::ICuratedTileGroup
        , ICuratedTileGroupPrivate
    >
{
    InspectableClass(L"CuratedTileCollections.CuratedTileGroup", BaseTrust);

public:
    HRESULT RuntimeClassInitialize(
        std::shared_ptr<dsct::CuratedGroup> transformerGroup, utctc::ICuratedTileCollection* collection);

    //~ Begin utctc::ICuratedTileGroup Interface
    STDMETHODIMP get_UniqueId(GUID* value) override;
    STDMETHODIMP get_DisplayName(HSTRING* value) override;
    STDMETHODIMP put_DisplayName(HSTRING value) override;
    STDMETHODIMP get_Attributes(utctc::GroupAttributes* value) override;
    STDMETHODIMP put_Attributes(utctc::GroupAttributes value) override;
    STDMETHODIMP get_Location(wf::Point* value) override;
    STDMETHODIMP put_Location(wf::Point value) override;
    STDMETHODIMP get_Size(wf::Size* value) override;
    STDMETHODIMP put_Size(wf::Size value) override;
    STDMETHODIMP get_LockedForCustomization(boolean* value) override;
    STDMETHODIMP put_LockedForCustomization(boolean value) override;
    STDMETHODIMP GetGroups(wfc::IMapView<GUID, ICuratedTileGroup*>** result) override;
    STDMETHODIMP GetTiles(wfc::IMapView<GUID, utctc::ICuratedTile*>** result) override;
    STDMETHODIMP CreateNewGroup(ICuratedTileGroup** result) override;
    STDMETHODIMP GetGroup(GUID groupId, ICuratedTileGroup** result) override;
    STDMETHODIMP DeleteGroup(GUID groupId) override;
    STDMETHODIMP RemoveGroup(GUID groupId) override;
    STDMETHODIMP AddTile(ut::IUnifiedTileIdentifier* identifier, utctc::ICuratedTile** result) override;
    STDMETHODIMP AddTileWithId(ut::IUnifiedTileIdentifier* identifier, GUID tileId, utctc::ICuratedTile** result) override;
    STDMETHODIMP GetTile(GUID tileId, utctc::ICuratedTile** result) override;
    STDMETHODIMP DeleteTile(GUID tileId) override;
    STDMETHODIMP RemoveTile(GUID tileId) override;
    STDMETHODIMP GetCustomProperty(HSTRING key, HSTRING* result) override;
    STDMETHODIMP HasCustomProperty(HSTRING key, boolean* result) override;
    STDMETHODIMP RemoveCustomProperty(HSTRING key) override;
    STDMETHODIMP SetCustomProperty(HSTRING key, HSTRING value) override;
    //~ End utctc::ICuratedTileGroup Interface

    //~ Begin ICuratedTileGroupPrivate Interface
    STDMETHODIMP AddTile(ICuratedTilePrivate* tile) override;
    STDMETHODIMP AddGroup(ICuratedTileGroupPrivate* group) override;
    STDMETHODIMP_(std::shared_ptr<dsct::CuratedGroup>) GetTransformerData() override;
    //~ End ICuratedTileGroupPrivate Interface

private:
    void PopulateFromTransformerData();
    HRESULT UnparentTile(const GUID& tileId, UnparentItemOptions options);
    HRESULT UnparentGroup(const GUID& groupId, UnparentItemOptions options);

    std::shared_ptr<dsct::CuratedGroup> _transformerGroup;
    std::unordered_map<GUID, wil::com_ptr<ICuratedTileGroup>, hashGUIDCuratedTileCollections> _groups;
    std::unordered_map<GUID, wil::com_ptr<utctc::ICuratedTile>, hashGUIDCuratedTileCollections> _tiles;
    wil::com_ptr<IWeakReference> _collectionWeak;
};
}
