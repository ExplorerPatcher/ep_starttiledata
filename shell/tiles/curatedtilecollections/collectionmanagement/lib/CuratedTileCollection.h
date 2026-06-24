#pragma once

#include <Windows.Foundation.h>
#include <Windows.UI.Core.h>

#include <WindowsInternal.Shell.UnifiedTile.h>

#include "../../collectioninitialization/lib/DataStoreCache_MoveMe.h"

namespace wf = ABI::Windows::Foundation;
namespace wfc = ABI::Windows::Foundation::Collections;
namespace ut = ABI::WindowsInternal::Shell::UnifiedTile;
namespace utctc = ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections;

struct hashGUIDCuratedTileCollections
{
    size_t operator()(const GUID& guid) const noexcept
    {
        const uint64_t* p = reinterpret_cast<const uint64_t*>(&guid);
        return std::hash<uint64_t>{}(p[0]) ^ std::hash<uint64_t>{}(p[1]);
    }
};

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
enum UnparentItemOptions
{
    UnparentItemOptions_0,
    UnparentItemOptions_1,
};

class CuratedTileCollectionBase
    : public Microsoft::WRL::Implements<utctc::ICuratedTileCollection>
{
public:
    //~ Begin utctc::ICuratedTileCollection Interface
    STDMETHODIMP get_CollectionName(HSTRING* outResult) override;
    STDMETHODIMP get_Attributes(utctc::CollectionAttributes* outResult) override;
    STDMETHODIMP put_Attributes(utctc::CollectionAttributes attributes) override;
    STDMETHODIMP get_Version(UINT* outResult) override;
    STDMETHODIMP put_Version(UINT version) override;
    STDMETHODIMP GetGroups(wfc::IMapView<GUID, utctc::ICuratedTileGroup*>** outResult) override;
    STDMETHODIMP GetTiles(wfc::IMapView<GUID, utctc::ICuratedTile*>** outResult) override;
    STDMETHODIMP GetAllTilesInCollection(wfc::IMapView<GUID, utctc::ICuratedTile*>** outResult) override;
    STDMETHODIMP DoesCollectionContainTile(
        ut::IUnifiedTileIdentifier* identifier, utctc::ICuratedTile** outTile, boolean* outResult) override;
    STDMETHODIMP FindTileAndParentGroup(
        ut::IUnifiedTileIdentifier* identifier, utctc::ICuratedTile** outTile, utctc::ICuratedTileGroup** outGroup,
        boolean* outResult) override;
    STDMETHODIMP MoveExistingGroupToNewParent(utctc::ICuratedTileGroup* existingGroup, utctc::ICuratedTileGroup* newParent) override;
    STDMETHODIMP CreateNewGroup(utctc::ICuratedTileGroup** outGroup) override;
    STDMETHODIMP GetGroup(GUID groupId, utctc::ICuratedTileGroup** outGroup) override;
    STDMETHODIMP DeleteGroup(GUID groupId) override;
    STDMETHODIMP RemoveGroup(GUID groupId) override;
    STDMETHODIMP MoveExistingTileToNewParent(utctc::ICuratedTile* existingTile, utctc::ICuratedTileGroup* newParent) override;
    STDMETHODIMP AddTile(ut::IUnifiedTileIdentifier* identifier, utctc::ICuratedTile** outResult) override;
    STDMETHODIMP AddTileWithId(ut::IUnifiedTileIdentifier* identifier, GUID tileId, utctc::ICuratedTile** outResult) override;
    STDMETHODIMP GetTile(GUID tileId, utctc::ICuratedTile** outResult) override;
    STDMETHODIMP DeleteTile(GUID tileId) override;
    STDMETHODIMP RemoveTile(GUID tileId) override;
    STDMETHODIMP Commit() override;
    STDMETHODIMP CommitAsync(wf::IAsyncAction** outResult) override;
    STDMETHODIMP CommitAsyncWithTimerBypass(wf::IAsyncAction** outResult) override;
    STDMETHODIMP ResetToDefault() override;
    STDMETHODIMP ResetToDefaultAsync(wf::IAsyncAction** outResult) override;
    STDMETHODIMP CheckForUpdate() override;
    STDMETHODIMP GetCustomProperty(const HSTRING key, HSTRING* outResult) override;
    STDMETHODIMP HasCustomProperty(const HSTRING key, boolean* outResult) override;
    STDMETHODIMP RemoveCustomProperty(const HSTRING key) override;
    STDMETHODIMP SetCustomProperty(const HSTRING key, HSTRING value) override;
    //~ End utctc::ICuratedTileCollection Interface

    virtual HRESULT EnsureTileRegistration();
    virtual HRESULT ResurrectTile(std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile> transformerTile, const GUID& tileId);
    virtual HRESULT OnTileAddedWithinCollection(ut::IUnifiedTileIdentifier* identifier);
    virtual HRESULT OnTileRemovedWithinCollection(ut::IUnifiedTileIdentifier* identifier);

    CuratedTileCollectionBase();

    virtual ~CuratedTileCollectionBase();

protected:
    HRESULT CommitAsyncInternal(std::function<void ()>&& callback, wf::IAsyncAction** outAction);
    void BeginBatchIfNecessary();
    bool TryFindTileAndParentGroup(
        ut::IUnifiedTileIdentifier* const identifier, utctc::ICuratedTile** outTile, utctc::ICuratedTileGroup** outGroup);
    static bool TryFindTileAndParentGroupInGroup(
        utctc::ICuratedTileGroup* const group, ut::IUnifiedTileIdentifier* const identifier,
        utctc::ICuratedTile** outTile, utctc::ICuratedTileGroup** outGroup);
    void PopulateFromTransformerData();
    HRESULT GetGroupRecursive(const GUID& groupId, utctc::ICuratedTileGroup** outGroup);
    HRESULT GetGroupRecursive(utctc::ICuratedTileGroup* parentGroup, const GUID& groupId, utctc::ICuratedTileGroup** outGroup);
    HRESULT UnparentGroup(const GUID& groupId, UnparentItemOptions options);
    HRESULT UnparentTile(const GUID& tileId, UnparentItemOptions options);

private:
    uint32_t _1;
    uint32_t _2;
    uint32_t _3;
    uint32_t _4;
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedRoot> _transformerRoot;
    wil::com_ptr<ABI::Windows::System::IUser /*???*/> _9;
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::ICuratedCollectionBatchCookieImpl> _batchCookie;
    std::unordered_map<GUID, wil::com_ptr<utctc::ICuratedTileGroup>, hashGUIDCuratedTileCollections> _15;
    std::unordered_map<GUID, wil::com_ptr<utctc::ICuratedTile>, hashGUIDCuratedTileCollections> _35;
    wil::com_ptr<ABI::Windows::System::IUser> _user;
    ULONGLONG _userContextToken;
    bool _bInstallPlaceholderTilesOnNextCommit;
    bool _commitOnDestroy;
};
}

// void FindCollectionParentOfGroup<WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::CuratedTileCollectionBase*>(GUID, WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::CuratedTileCollectionBase*, IInspectable**);
// void FindCollectionParentOfTile<WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::CuratedTileCollectionBase*>(GUID, WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::CuratedTileCollectionBase*, IInspectable**);
