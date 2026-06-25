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

template <typename TFunctor>
bool EnumerateAllTilesInGroupRecursive(utctc::ICuratedTileGroup* group, const TFunctor& functor);

template <typename T>
void FindCollectionParentOfTile(GUID tileId, T instance, IInspectable** outCollectionParent);

template <typename T>
void FindCollectionParentOfGroup(GUID groupId, T instance, IInspectable** outCollectionParent);

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
MIDL_INTERFACE("20477929-b8fb-43e2-9c9e-a346c98180e1")
ICuratedTileCollectionInternal : utctc::ICuratedTileCollection
{
    virtual HRESULT STDMETHODCALLTYPE EnsureTileRegistration() = 0;
    virtual HRESULT STDMETHODCALLTYPE ResurrectTile(std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile> transformerTile, const GUID& tileId) = 0;
    virtual HRESULT STDMETHODCALLTYPE OnTileAddedWithinCollection(ut::IUnifiedTileIdentifier* identifier) = 0;
    virtual HRESULT STDMETHODCALLTYPE OnTileRemovedWithinCollection(ut::IUnifiedTileIdentifier* identifier) = 0;
};

enum UnparentItemOptions
{
    UnparentItemOptions_0,
    UnparentItemOptions_1,
};

enum CuratedTileCollectionOptionsInternal
{
    CuratedTileCollectionOptionsInternal_None = 0,
    CuratedTileCollectionOptionsInternal_1 = 0x1,
};

DEFINE_ENUM_FLAG_OPERATORS(CuratedTileCollectionOptionsInternal);

class CuratedTileCollectionBase
    : public Microsoft::WRL::Implements<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRt>
        , Microsoft::WRL::ChainInterfaces<ICuratedTileCollectionInternal, utctc::ICuratedTileCollection>
    >
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
    STDMETHODIMP GetGroup(GUID groupId, utctc::ICuratedTileGroup** outResult) override;
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

    //~ Begin ICuratedTileCollectionInternal Interface
    STDMETHODIMP EnsureTileRegistration() override;
    STDMETHODIMP ResurrectTile(std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile> transformerTile, const GUID& tileId) override;
    STDMETHODIMP OnTileAddedWithinCollection(ut::IUnifiedTileIdentifier* identifier) override;
    STDMETHODIMP OnTileRemovedWithinCollection(ut::IUnifiedTileIdentifier* identifier) override;
    //~ End ICuratedTileCollectionInternal Interface

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

    CuratedTileCollectionOptionsInternal _options;
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedRoot> _transformerRoot;
    wil::com_ptr<ABI::Windows::System::IUser /*???*/> _9;
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::ICuratedCollectionBatchCookieImpl> _batchCookie;
    std::unordered_map<GUID, wil::com_ptr<utctc::ICuratedTileGroup>, hashGUIDCuratedTileCollections> _groups;
    std::unordered_map<GUID, wil::com_ptr<utctc::ICuratedTile>, hashGUIDCuratedTileCollections> _tiles;
    wil::com_ptr<ABI::Windows::System::IUser> _user;
    UINT64 _userContextToken;
    bool _bInstallPlaceholderTilesOnNextCommit;
    bool _bCommitOnDestroy;
};

MIDL_INTERFACE("ebb3adda-cd0c-4d14-a198-6fb7dcd692e2")
ICuratedTilePrivate : utctc::ICuratedTile
{
    virtual std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile> STDMETHODCALLTYPE GetTransformerData() = 0;
};

MIDL_INTERFACE("6f3e1834-00c0-4e8b-8834-89da30e185e9")
ICuratedTileGroupPrivate : utctc::ICuratedTileGroup
{
    virtual HRESULT STDMETHODCALLTYPE AddTile(ICuratedTilePrivate*) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddGroup(ICuratedTileGroupPrivate*) = 0;
    virtual std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup> STDMETHODCALLTYPE GetTransformerData() = 0;
};
}
