#pragma once

namespace DataStoreCache::Util
{
struct hashGUID
{
    size_t operator()(const GUID& guid) const noexcept
    {
        const uint64_t* p = reinterpret_cast<const uint64_t*>(&guid);
        return std::hash<uint64_t>{}(p[0]) ^ std::hash<uint64_t>{}(p[1]);
    }
};
}

namespace DataStoreCache::CloudUtil
{
template <typename T>
struct CloudItemObserverCallback;
}

namespace DataStoreCache::CuratedTileCollectionTransformer
{
class ICuratedCollectionBatchCookieImpl
{
public:
    virtual ~ICuratedCollectionBatchCookieImpl();
    virtual Concurrency::task<void> EndBatchUpdate() = 0;
    virtual void SetPreventEndBatchOnDestruction(bool) = 0;
};

struct CuratedTileChangeInfo;

namespace Internal
{
    class DECLSPEC_NOVTABLE ICuratedTileImpl
    {
    public:
        virtual ~ICuratedTileImpl();
        virtual POINT GetLocation() = 0;
        virtual void SetLocation(POINT) = 0;
        virtual std::shared_ptr<ICuratedCollectionBatchCookieImpl> BeginBatchUpdate() = 0;
        virtual GUID GetLayoutId() = 0;
        virtual void SetSize(const SIZE&) = 0;
        virtual SIZE GetSize() = 0;
        virtual wil::com_ptr<ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier> GetTileIdentifier() = 0;
        virtual void SetTileIdentifier(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
        virtual std::wstring GetCustomProperty(const std::wstring&) = 0;
        virtual bool HasCustomProperty(const std::wstring&) = 0;
        virtual void RemoveCustomProperty(const std::wstring&) = 0;
        virtual void SetCustomProperty(const std::wstring&, const std::wstring&) = 0;
        virtual std::shared_ptr<CloudUtil::CloudItemObserverCallback<CuratedTileChangeInfo>> AddObserver(const std::function<void (const CuratedTileChangeInfo&)>&) = 0;
        virtual bool IsDefaultData() = 0;
        virtual std::wstring GetJSONBlob(UINT) = 0;
    };
}

// shell/DataStoreCache/Transformers/CuratedTileCollectionTransformer/inc/CuratedTransformerObjects/CuratedTile.h
class CuratedTile
{
public:
    CuratedTile(std::shared_ptr<Internal::ICuratedTileImpl> impl)
        : _impl(impl)
    {
        FAIL_FAST_IF(_impl == nullptr || _impl.use_count() == 0); // 51
    }


    POINT GetLocation() { return GetImpl()->GetLocation(); }
    void SetLocation(POINT a2) { GetImpl()->SetLocation(a2); }
    std::shared_ptr<ICuratedCollectionBatchCookieImpl> BeginBatchUpdate() { return GetImpl()->BeginBatchUpdate(); }
    GUID GetLayoutId() { return GetImpl()->GetLayoutId(); }
    void SetSize(const SIZE& a2) { GetImpl()->SetSize(a2); }
    SIZE GetSize() { return GetImpl()->GetSize(); }
    wil::com_ptr<ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier> GetTileIdentifier() { return GetImpl()->GetTileIdentifier(); }
    void SetTileIdentifier(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* a2) { GetImpl()->SetTileIdentifier(a2); }
    std::wstring GetCustomProperty(const std::wstring& a2) { return GetImpl()->GetCustomProperty(a2); }
    bool HasCustomProperty(const std::wstring& a2) { return GetImpl()->HasCustomProperty(a2); }
    void RemoveCustomProperty(const std::wstring& a2) { GetImpl()->RemoveCustomProperty(a2); }
    void SetCustomProperty(const std::wstring& a2, const std::wstring& a3) { GetImpl()->SetCustomProperty(a2, a3); }
    std::shared_ptr<CloudUtil::CloudItemObserverCallback<CuratedTileChangeInfo>> AddObserver(const std::function<void (const CuratedTileChangeInfo&)>& a2) { return GetImpl()->AddObserver(a2); }
    bool IsDefaultData() { return GetImpl()->IsDefaultData(); }
    std::wstring GetJSONBlob(UINT a2) { return GetImpl()->GetJSONBlob(a2); }

    RECT GetLayoutRect()
    {
        SIZE size = GetImpl()->GetSize();
        POINT location = GetImpl()->GetLocation();

        RECT layoutRect;
        layoutRect.left = location.x;
        layoutRect.top = location.y;
        layoutRect.right = layoutRect.left + size.cx;
        layoutRect.bottom = layoutRect.top + size.cy;
        return layoutRect;
    }

protected:
    std::shared_ptr<Internal::ICuratedTileImpl> GetImpl()
    {
        FAIL_FAST_IF(_impl == nullptr || _impl.use_count() == 0); // 91
        return _impl;
    }

    std::shared_ptr<Internal::ICuratedTileImpl> _impl;
};

struct CuratedGroupChangeInfo;

class CuratedGroup; // fwd decl

class DECLSPEC_NOVTABLE ICuratedGroupImpl
{
public:
    virtual ~ICuratedGroupImpl();
    virtual std::shared_ptr<std::wstring> GetDisplayName() = 0;
    virtual void SetDisplayName(std::wstring) = 0;
    virtual std::shared_ptr<std::wstring> GetLocalizedDisplayNameResource() = 0;
    virtual void SetLocalizedDisplayNameResource(std::wstring) = 0;
    virtual POINT GetLocation() = 0;
    virtual void SetLocation(POINT) = 0;
    virtual bool GetIsLockedForCustomization() = 0;
    virtual void SetIsLockedForCustomization(bool) = 0;
    virtual bool GetIsUserCustomizedDisplayName() = 0;
    virtual void SetIsUserCustomizedDisplayName(bool) = 0;
    virtual SIZE GetSize() = 0;
    virtual void SetSize(SIZE) = 0;
    virtual GUID GetLayoutId() = 0;
    virtual std::shared_ptr<ICuratedCollectionBatchCookieImpl> BeginBatchUpdate() = 0;
    virtual void AddTile(std::shared_ptr<CuratedTile>) = 0;
    virtual std::shared_ptr<CuratedTile> CreateTile(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
    virtual std::shared_ptr<CuratedTile> CreateTile(const GUID&, ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
    virtual std::unordered_map<GUID, std::shared_ptr<CuratedTile>, Util::hashGUID> GetTiles() = 0;
    virtual void DeleteTile(const GUID&) = 0;
    virtual void RemoveTile(const GUID&) = 0;
    virtual void AddGroup(std::shared_ptr<CuratedGroup>) = 0;
    virtual std::shared_ptr<CuratedGroup> CreateGroup() = 0;
    virtual std::unordered_map<GUID, std::shared_ptr<CuratedGroup>, Util::hashGUID> GetGroups() = 0;
    virtual void DeleteGroup(const GUID&) = 0;
    virtual void RemoveGroup(const GUID&) = 0;
    virtual std::wstring GetCustomProperty(const std::wstring&) = 0;
    virtual bool HasCustomProperty(const std::wstring&) = 0;
    virtual void RemoveCustomProperty(const std::wstring&) = 0;
    virtual void SetCustomProperty(const std::wstring&, const std::wstring&) = 0;
    virtual std::shared_ptr<CloudUtil::CloudItemObserverCallback<CuratedGroupChangeInfo>> AddObserver(const std::function<void (const CuratedGroupChangeInfo&)>&) = 0;
    virtual bool IsDefaultData() = 0;
    virtual std::wstring GetJSONBlob(UINT) = 0;
};

class CuratedGroup
{
public:
    CuratedGroup(std::shared_ptr<ICuratedGroupImpl> impl)
        : _impl(impl)
    {
    }

    std::shared_ptr<const std::wstring> GetDisplayName() { return _impl->GetDisplayName(); }
    void SetDisplayName(std::wstring a2) { _impl->SetDisplayName(a2); }
    std::shared_ptr<const std::wstring> GetLocalizedDisplayNameResource() { return _impl->GetLocalizedDisplayNameResource(); }
    void SetLocalizedDisplayNameResource(std::wstring a2) { _impl->SetLocalizedDisplayNameResource(a2); }
    POINT GetLocation() { return _impl->GetLocation(); }
    void SetLocation(POINT a2) { _impl->SetLocation(a2); }
    bool GetIsLockedForCustomization() { return _impl->GetIsLockedForCustomization(); }
    void SetIsLockedForCustomization(bool a2) { _impl->SetIsLockedForCustomization(a2); }
    bool GetIsUserCustomizedDisplayName() { return _impl->GetIsUserCustomizedDisplayName(); }
    void SetIsUserCustomizedDisplayName(bool a2) { _impl->SetIsUserCustomizedDisplayName(a2); }
    SIZE GetSize() { return _impl->GetSize(); }
    void SetSize(SIZE a2) { _impl->SetSize(a2); }
    GUID GetLayoutId() { return _impl->GetLayoutId(); }
    std::shared_ptr<ICuratedCollectionBatchCookieImpl> BeginBatchUpdate() { return _impl->BeginBatchUpdate(); }
    void AddTile(std::shared_ptr<CuratedTile> a2) { _impl->AddTile(a2); }
    std::shared_ptr<CuratedTile> CreateTile(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* a2) { return _impl->CreateTile(a2); }
    std::shared_ptr<CuratedTile> CreateTile(const GUID& a2, ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* a3) { return _impl->CreateTile(a2, a3); }
    std::unordered_map<GUID, std::shared_ptr<CuratedTile>, Util::hashGUID> GetTiles() { return _impl->GetTiles(); }
    void DeleteTile(const GUID& a2) { _impl->DeleteTile(a2); }
    void RemoveTile(const GUID& a2) { _impl->RemoveTile(a2); }
    void AddGroup(std::shared_ptr<CuratedGroup> a2) { _impl->AddGroup(a2); }
    std::shared_ptr<CuratedGroup> CreateGroup() { return _impl->CreateGroup(); }
    std::unordered_map<GUID, std::shared_ptr<CuratedGroup>, Util::hashGUID> GetGroups() { return _impl->GetGroups(); }
    void DeleteGroup(const GUID& a2) { _impl->DeleteGroup(a2); }
    void RemoveGroup(const GUID& a2) { _impl->RemoveGroup(a2); }
    std::wstring GetCustomProperty(const std::wstring& a2) { return _impl->GetCustomProperty(a2); }
    bool HasCustomProperty(const std::wstring& a2) { return _impl->HasCustomProperty(a2); }
    void RemoveCustomProperty(const std::wstring& a2) { _impl->RemoveCustomProperty(a2); }
    void SetCustomProperty(const std::wstring& a2, const std::wstring& a3) { _impl->SetCustomProperty(a2, a3); }
    std::shared_ptr<CloudUtil::CloudItemObserverCallback<CuratedGroupChangeInfo>> AddObserver(const std::function<void (const CuratedGroupChangeInfo&)>& a2) { return _impl->AddObserver(a2); }
    bool IsDefaultData() { return _impl->IsDefaultData(); }
    std::wstring GetJSONBlob(UINT a2) { return _impl->GetJSONBlob(a2); }

protected:
    std::shared_ptr<ICuratedGroupImpl> _impl;
};

enum LayoutOverrideType
{
};

enum LayoutCustomizationRestrictionType
{
    LayoutCustomizationRestrictionType_None,
    LayoutCustomizationRestrictionType_OnlySpecifiedGroups,
    LayoutCustomizationRestrictionType_FullLayout,
};

struct CuratedRootChangeInfo;

class DECLSPEC_NOVTABLE ICuratedRootImpl
{
public:
    virtual ~ICuratedRootImpl();
    virtual UINT GetLayoutVersion() = 0;
    virtual void SetLayoutVersion(UINT) = 0;
    virtual UINT GetGroupColumnCount() = 0;
    virtual void SetGroupColumnCount(UINT) = 0;
    virtual UINT GetPreferredColumnCount() = 0;
    virtual void SetPreferredColumnCount(UINT) = 0;
    virtual GUID GetLastGroupId() = 0;
    virtual void SetLastGroupId(GUID) = 0;
    virtual LayoutOverrideType GetLayoutOverride() = 0;
    virtual void SetLayoutOverride(LayoutOverrideType) = 0;
    virtual LayoutCustomizationRestrictionType GetLayoutCustomizationRestriction() = 0;
    virtual void SetLayoutCustomizationRestriction(LayoutCustomizationRestrictionType) = 0;
    virtual std::shared_ptr<ICuratedCollectionBatchCookieImpl> BeginBatchUpdate() = 0;
    virtual std::vector<std::shared_ptr<CuratedTile>> GetAllTilesInTree() = 0;
    virtual GUID GetLayoutId() = 0;
    virtual std::shared_ptr<const std::wstring> GetLayoutName() = 0;
    virtual void SetLayoutName(const WCHAR*) = 0;
    virtual UINT GetGroupCellWidth() = 0;
    virtual void SetGroupCellWidth(UINT) = 0;
    virtual FILETIME GetEnterpriseLayoutLastApplied(bool*) = 0;
    virtual void SetEnterpriseLayoutLastApplied(const FILETIME&) = 0;
    virtual FILETIME GetGroupPolicyLayoutFileTimestamp(bool*) = 0;
    virtual void SetGroupPolicyLayoutFileTimestamp(const FILETIME&) = 0;
    virtual void AddTile(std::shared_ptr<CuratedTile>) = 0;
    virtual std::shared_ptr<CuratedTile> CreateTile(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
    virtual std::shared_ptr<CuratedTile> CreateTile(const GUID&, ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
    virtual std::unordered_map<GUID, std::shared_ptr<CuratedTile>, Util::hashGUID> GetTiles() = 0;
    virtual void DeleteTile(const GUID&) = 0;
    virtual void RemoveTile(const GUID&) = 0;
    virtual size_t GetTileCount() = 0;
    virtual void AddGroup(std::shared_ptr<CuratedGroup>) = 0;
    virtual std::shared_ptr<CuratedGroup> CreateGroup() = 0;
    virtual std::shared_ptr<CuratedGroup> CreateGroup(const GUID&) = 0;
    virtual std::unordered_map<GUID, std::shared_ptr<CuratedGroup>, Util::hashGUID> GetGroups() = 0;
    virtual void DeleteGroup(const GUID&) = 0;
    virtual void RemoveGroup(const GUID&) = 0;
    virtual size_t GetGroupCount() = 0;
    virtual std::wstring GetCustomProperty(const std::wstring&) = 0;
    virtual bool HasCustomProperty(const std::wstring&) = 0;
    virtual void RemoveCustomProperty(const std::wstring&) = 0;
    virtual void SetCustomProperty(const std::wstring&, const std::wstring&) = 0;
    virtual std::shared_ptr<CloudUtil::CloudItemObserverCallback<CuratedRootChangeInfo>> AddObserver(const std::function<void (const CuratedRootChangeInfo&)>&) = 0;
    virtual bool IsDefaultData() = 0;
    virtual std::wstring GetJSONBlob(UINT) = 0;
};

class CuratedRoot
{
public:
    CuratedRoot(std::shared_ptr<ICuratedRootImpl> impl)
        : _impl(impl)
    {
    }

    UINT GetLayoutVersion() { return _impl->GetLayoutVersion(); }
    void SetLayoutVersion(UINT a2) { _impl->SetLayoutVersion(a2); }
    UINT GetGroupColumnCount() { return _impl->GetGroupColumnCount(); }
    void SetGroupColumnCount(UINT a2) { _impl->SetGroupColumnCount(a2); }
    UINT GetPreferredColumnCount() { return _impl->GetPreferredColumnCount(); }
    void SetPreferredColumnCount(UINT a2) { _impl->SetPreferredColumnCount(a2); }
    GUID GetLastGroupId() { return _impl->GetLastGroupId(); }
    void SetLastGroupId(GUID a2) { _impl->SetLastGroupId(a2); }
    LayoutOverrideType GetLayoutOverride() { return _impl->GetLayoutOverride(); }
    void SetLayoutOverride(LayoutOverrideType a2) { _impl->SetLayoutOverride(a2); }
    LayoutCustomizationRestrictionType GetLayoutCustomizationRestriction() { return _impl->GetLayoutCustomizationRestriction(); }
    void SetLayoutCustomizationRestriction(LayoutCustomizationRestrictionType a2) { _impl->SetLayoutCustomizationRestriction(a2); }
    std::shared_ptr<ICuratedCollectionBatchCookieImpl> BeginBatchUpdate() { return _impl->BeginBatchUpdate(); }
    Concurrency::task<void> EndBatchUpdate(std::shared_ptr<ICuratedCollectionBatchCookieImpl> cookie) { return cookie->EndBatchUpdate(); }
    std::vector<std::shared_ptr<CuratedTile>> GetAllTilesInTree() { return _impl->GetAllTilesInTree(); }
    GUID GetLayoutId() { return _impl->GetLayoutId(); }
    std::shared_ptr<const std::wstring> GetLayoutName() { return _impl->GetLayoutName(); }
    void SetLayoutName(const WCHAR* a2) { _impl->SetLayoutName(a2); }
    UINT GetGroupCellWidth() { return _impl->GetGroupCellWidth(); }
    void SetGroupCellWidth(UINT a2) { _impl->SetGroupCellWidth(a2); }
    FILETIME GetEnterpriseLayoutLastApplied(bool* a2) { return _impl->GetEnterpriseLayoutLastApplied(a2); }
    void SetEnterpriseLayoutLastApplied(const FILETIME& a2) { _impl->SetEnterpriseLayoutLastApplied(a2); }
    FILETIME GetGroupPolicyLayoutFileTimestamp(bool* a2) { return _impl->GetGroupPolicyLayoutFileTimestamp(a2); }
    void SetGroupPolicyLayoutFileTimestamp(const FILETIME& a2) { _impl->SetGroupPolicyLayoutFileTimestamp(a2); }
    void AddTile(std::shared_ptr<CuratedTile> a2) { _impl->AddTile(a2); }
    std::shared_ptr<CuratedTile> CreateTile(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* a2) { return _impl->CreateTile(a2); }
    std::shared_ptr<CuratedTile> CreateTile(const GUID& a2, ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* a3) { return _impl->CreateTile(a2, a3); }
    std::unordered_map<GUID, std::shared_ptr<CuratedTile>, Util::hashGUID> GetTiles() { return _impl->GetTiles(); }
    void DeleteTile(const GUID& a2) { _impl->DeleteTile(a2); }
    void RemoveTile(const GUID& a2) { _impl->RemoveTile(a2); }
    size_t GetTileCount() { return _impl->GetTileCount(); }
    void AddGroup(std::shared_ptr<CuratedGroup> a2) { _impl->AddGroup(a2); }
    std::shared_ptr<CuratedGroup> CreateGroup() { return _impl->CreateGroup(); }
    std::shared_ptr<CuratedGroup> CreateGroup(const GUID& a2) { return _impl->CreateGroup(a2); }
    std::unordered_map<GUID, std::shared_ptr<CuratedGroup>, Util::hashGUID> GetGroups() { return _impl->GetGroups(); }
    void DeleteGroup(const GUID& a2) { _impl->DeleteGroup(a2); }
    void RemoveGroup(const GUID& a2) { _impl->RemoveGroup(a2); }
    size_t GetGroupCount() { return _impl->GetGroupCount(); }
    std::wstring GetCustomProperty(const std::wstring& a2) { return _impl->GetCustomProperty(a2); }
    bool HasCustomProperty(const std::wstring& a2) { return _impl->HasCustomProperty(a2); }
    void RemoveCustomProperty(const std::wstring& a2) { _impl->RemoveCustomProperty(a2); }
    void SetCustomProperty(const std::wstring& a2, const std::wstring& a3) { _impl->SetCustomProperty(a2, a3); }
    std::shared_ptr<CloudUtil::CloudItemObserverCallback<CuratedRootChangeInfo>> AddObserver(const std::function<void (const CuratedRootChangeInfo&)>& a2) { return _impl->AddObserver(a2); }
    bool IsDefaultData() { return _impl->IsDefaultData(); }
    std::wstring GetJSONBlob(UINT a2) { return _impl->GetJSONBlob(a2); }

protected:
    std::shared_ptr<ICuratedRootImpl> _impl;
};

enum CuratedTileCollectionTransformerOptions
{
    CuratedTileCollectionTransformerOptions_0,
};

MIDL_INTERFACE("f54aa3a6-565a-487a-ae5b-0e5c3a1388bf")
ICuratedTileCollectionTransformer : IInspectable
{
    virtual std::shared_ptr<CuratedRoot> STDMETHODCALLTYPE CreateNewLayoutRoot(const GUID&, CuratedTileCollectionTransformerOptions) = 0;
    virtual std::shared_ptr<CuratedRoot> STDMETHODCALLTYPE CreateNewLayoutRoot(const WCHAR*, CuratedTileCollectionTransformerOptions) = 0;
    virtual void STDMETHODCALLTYPE DeleteLayoutRoot(const GUID&) = 0;
    virtual void STDMETHODCALLTYPE DeleteLayoutRoot(const WCHAR*) = 0;
    virtual std::shared_ptr<CuratedRoot> STDMETHODCALLTYPE GetCuratedTileCollectionRoot(const GUID&, CuratedTileCollectionTransformerOptions) = 0;
    virtual std::shared_ptr<CuratedRoot> STDMETHODCALLTYPE GetCuratedTileCollectionRoot(const WCHAR*, CuratedTileCollectionTransformerOptions) = 0;
    virtual bool STDMETHODCALLTYPE HasLayoutRoot(const GUID&) = 0;
    virtual bool STDMETHODCALLTYPE HasLayoutRoot(const WCHAR*) = 0;
    virtual std::vector<Microsoft::WRL::Wrappers::HString> STDMETHODCALLTYPE GetAllRoots() = 0;
    virtual void STDMETHODCALLTYPE SetDetectTornContainers(bool) = 0;
    virtual void STDMETHODCALLTYPE PinStartLayoutRoot(const std::shared_ptr<CuratedRoot>&) = 0;
    virtual Concurrency::task<void> STDMETHODCALLTYPE CommitAsyncWithTimerBypass(const WCHAR*) = 0;
    virtual void STDMETHODCALLTYPE SetUseCommitTimer(const WCHAR*, bool) = 0;
};

void CreateCuratedTileCollectionTransformer(
    ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileManager* pUnifiedTileManager,
    ABI::Windows::System::IUser* pUser, ICuratedTileCollectionTransformer** ppTransformer);

class CollectionDoesNotExistException : public wil::ResultException
{
public:
    CollectionDoesNotExistException()
        : ResultException(E_NOT_SET)
    {
    }
};
}

namespace DataStoreCache
{
struct IDataManager
{
};

enum DataStoreCacheInit
{
};

struct LineData
{
};

MIDL_INTERFACE("47ab0a63-152e-49e5-b183-06c1eae6597c")
IDataStoreTransformer : IUnknown
{
    virtual const GUID& STDMETHODCALLTYPE GetId() const = 0;
    virtual void STDMETHODCALLTYPE SetDataManager(IDataManager*) = 0;
    virtual void STDMETHODCALLTYPE SetInitializationStage(DataStoreCacheInit) = 0;
    virtual std::vector<LineData> STDMETHODCALLTYPE DumpData(const GUID&, HSTRING, UINT) = 0;
};
}

namespace Windows::Data
{
struct PlaceholderTile;
struct PlaceholderTileLocal;
}

namespace StartPlaceHolderTelemetry
{
class PlaceholderTileActivated;
}

namespace DataStoreCache::PlaceholderTileTransformer::Internal
{
interface IPlaceholderTileTransformerInternal;
}

namespace DataStoreCache::PlaceholderTileTransformer
{
enum InstallDelayType
{
};

enum AssetManagementPolicyEnum
{
};

enum InstallStateType
{
};

enum InstallReasonType
{
};

class PlaceholderTileImpl
{
public:
    PlaceholderTileImpl(
        Internal::IPlaceholderTileTransformerInternal*, const Windows::Data::PlaceholderTile&);

    void DumpData(std::vector<LineData>&, UINT);
    std::shared_ptr<const std::wstring> GetArguments();
    std::shared_ptr<const std::wstring> GetBackgroundColor();
    std::shared_ptr<const std::wstring> GetForegroundText();
    bool GetHas310x150Logo();
    bool GetHas310x310Logo();
    InstallDelayType GetInstallDelay();
    bool GetIsPrimary();
    bool GetIsVisibleInAppList();
    std::shared_ptr<const std::wstring> GetPackageFamilyName();
    std::shared_ptr<const std::wstring> GetProductId();
    std::shared_ptr<const std::wstring> GetResolvedDisplayName();
    bool GetShowNameOnSquare150x150Logo();
    bool GetShowNameOnSquare310x310Logo();
    bool GetShowNameOnWide310x150Logo();
    std::shared_ptr<const std::wstring> GetSkuId();
    std::shared_ptr<const std::wstring> GetStoreCampaignId();
    wil::com_ptr<ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier> GetTileIdentifier();
    UINT64 GetVisualsChangedHash();
    bool HasCollectionReference(const WCHAR*);
    bool HasPrimaryCollectionReference();
    bool HasSecondaryCollectionReference();
    void RemoveCollectionReference(const WCHAR*);
    void SetBackgroundColor(std::wstring);
    void SetForegroundText(std::wstring);
    void SetInstallDelay(InstallDelayType);
    void SetPackageFamilyName(std::wstring);
    void SetProductId(std::wstring);
    void SetResolvedDisplayName(std::wstring);
    void SetSkuId(std::wstring);
    void SetStoreCampaignId(std::wstring);

protected:
    void OnItemUpdated(const Windows::Data::PlaceholderTile&);
};

class PlaceholderTileLocalImpl
{
public:
    PlaceholderTileLocalImpl(
        Internal::IPlaceholderTileTransformerInternal*, const Windows::Data::PlaceholderTileLocal&, HSTRING);

    void DumpData(std::vector<LineData>&, UINT);
    AssetManagementPolicyEnum GetAssetManagementPolicy();
    std::shared_ptr<const std::wstring> GetAppsLogo44x44Uri();
    InstallReasonType GetInstallReason();
    InstallStateType GetInstallState();
    bool GetIsPostProcessingCompleted();
    std::shared_ptr<const std::wstring> GetPackageFullName();
    std::shared_ptr<const std::wstring> GetSquare150x150LogoUri();
    std::shared_ptr<const std::wstring> GetSquare310x310LogoUri();
    std::shared_ptr<const std::wstring> GetSquare71x71LogoUri();
    std::shared_ptr<const std::wstring> GetWide310x150LogoUri();
    void ReleaseAssets();
    void SetAppsLogo44x44Uri(std::wstring);
    void SetIsPostProcessingCompleted(bool);
    void SetPackageFullName(std::wstring);
    void SetSquare150x150LogoUri(std::wstring);
    void SetSquare310x310LogoUri(std::wstring);
    void SetSquare71x71LogoUri(std::wstring);
    void SetWide310x150LogoUri(std::wstring);

protected:
    void OnItemUpdated(const Windows::Data::PlaceholderTileLocal&);
};

class PlaceholderTile
{
public:
    // ctor & dtor unknown

    std::shared_ptr<const std::wstring> GetBackgroundColor() { return _impl->GetBackgroundColor(); }
    std::shared_ptr<const std::wstring> GetResolvedDisplayName() { return _impl->GetResolvedDisplayName(); }
    void SetAppsLogo44x44Uri(std::wstring appsLogo44x44Uri) { _localImpl->SetAppsLogo44x44Uri(appsLogo44x44Uri); }
    void SetBackgroundColor(std::wstring backgroundColor) { _impl->SetBackgroundColor(backgroundColor); }
    void SetForegroundText(std::wstring foregroundText) { _impl->SetForegroundText(foregroundText); }
    void SetPackageFamilyName(std::wstring packageFamilyName) { _impl->SetPackageFamilyName(packageFamilyName); }
    void SetProductId(std::wstring productId) { _impl->SetProductId(productId); }
    void SetResolvedDisplayName(std::wstring resolvedDisplayName) { _impl->SetResolvedDisplayName(resolvedDisplayName); }
    void SetSkuId(std::wstring skuId) { _impl->SetSkuId(skuId); }
    void SetSquare150x150LogoUri(std::wstring square150x150LogoUri) { _localImpl->SetSquare150x150LogoUri(square150x150LogoUri); }
    void SetSquare310x310LogoUri(std::wstring square310x310LogoUri) { _localImpl->SetSquare310x310LogoUri(square310x310LogoUri); }
    void SetSquare71x71LogoUri(std::wstring square71x71LogoUri) { _localImpl->SetSquare71x71LogoUri(square71x71LogoUri); }
    void SetStoreCampaignId(std::wstring storeCampaignId) { _impl->SetStoreCampaignId(storeCampaignId); }
    void SetWide310x150LogoUri(std::wstring wide310x150LogoUri) { _localImpl->SetWide310x150LogoUri(wide310x150LogoUri); }

private:
    std::shared_ptr<PlaceholderTileImpl> _impl;
    std::shared_ptr<PlaceholderTileLocalImpl> _localImpl;
};

class IPlaceholderTileBatchCookie
{
};

MIDL_INTERFACE("93400fa0-3b2d-413c-9a9c-fb7962988d15")
IPlaceholderTileTransformer : IDataStoreTransformer
{
    virtual std::shared_ptr<IPlaceholderTileBatchCookie> STDMETHODCALLTYPE BeginBatchUpdate() = 0;
    virtual Concurrency::task<void> STDMETHODCALLTYPE EndBatchUpdate(std::shared_ptr<IPlaceholderTileBatchCookie>) = 0;
    virtual std::shared_ptr<PlaceholderTile> STDMETHODCALLTYPE AddTileToCollection(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*, HSTRING) = 0;
    virtual void STDMETHODCALLTYPE RemoveTileFromCollection(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*, HSTRING) = 0;
    virtual std::map<std::wstring, std::shared_ptr<PlaceholderTile>> STDMETHODCALLTYPE GetAllTilesInCollection(HSTRING) = 0;
    virtual std::shared_ptr<PlaceholderTile> STDMETHODCALLTYPE GetTile(const WCHAR*) = 0;
    virtual std::shared_ptr<PlaceholderTile> STDMETHODCALLTYPE GetTile(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
    virtual std::shared_ptr<PlaceholderTile> STDMETHODCALLTYPE TryGetTile(const WCHAR*) = 0;
    virtual bool STDMETHODCALLTYPE HasTile(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier*) = 0;
    virtual std::map<std::wstring, std::shared_ptr<PlaceholderTile>> STDMETHODCALLTYPE GetAllTiles() = 0;
    virtual void STDMETHODCALLTYPE DeleteTile(const WCHAR*) = 0;
    virtual bool STDMETHODCALLTYPE HasTile(const WCHAR*) = 0;
    virtual bool STDMETHODCALLTYPE HasPendingCommits() = 0;
};

MIDL_INTERFACE("bb5d3c4c-40cb-41c6-8de7-d036a8f90f6c")
Internal::IPlaceholderTileTransformerInternal : IPlaceholderTileTransformer
{
    virtual void STDMETHODCALLTYPE OnItemUpdated(const WCHAR*, const Windows::Data::PlaceholderTileLocal&) = 0;
    virtual void STDMETHODCALLTYPE OnItemUpdated(const WCHAR*, const Windows::Data::PlaceholderTile&) = 0;
    virtual wil::com_ptr<ABI::Windows::System::IUser> STDMETHODCALLTYPE GetUser() = 0;
    virtual HRESULT STDMETHODCALLTYPE InstallApp(const std::shared_ptr<PlaceholderTile>&, HSTRING, UINT, ABI::Windows::Foundation::Collections::IPropertySet*, StartPlaceHolderTelemetry::PlaceholderTileActivated&) = 0;
    virtual HRESULT STDMETHODCALLTYPE CancelAppInstall(const std::shared_ptr<PlaceholderTile>&, HSTRING) = 0;
    virtual bool STDMETHODCALLTYPE IsAppInstalling(const std::shared_ptr<PlaceholderTile>&) = 0;
};
}

struct CuratedTileCollectionTransformerCreationArgs
{
    ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileManager* pUnifiedTileManager;
    ABI::Windows::System::IUser* pUser;
};

MIDL_INTERFACE("ca7bdd1c-19cc-4128-849e-1186ca3381f3")
ITransformerFactory : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetCuratedTileCollectionTransformer(CuratedTileCollectionTransformerCreationArgs, REFIID, void**) = 0;
};

class DECLSPEC_UUID("7fa999a5-502f-4a8b-932e-5c84fd8254ab")
TransformerFactory;

// shell/datastorecache/transformers/curatedtilecollectiontransformer_onestruct/lib/CuratedTileCollectionTransformer.cpp
inline void DataStoreCache::CuratedTileCollectionTransformer::CreateCuratedTileCollectionTransformer(
    ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileManager* pUnifiedTileManager,
    ABI::Windows::System::IUser* pUser, ICuratedTileCollectionTransformer** ppTransformer)
{
    *ppTransformer = nullptr;

    Microsoft::WRL::ComPtr<ITransformerFactory> spTransformerFactory;
    THROW_IF_FAILED(CoCreateInstance(
        __uuidof(TransformerFactory), nullptr, CLSCTX_INPROC, IID_PPV_ARGS(&spTransformerFactory))); // 630
    THROW_IF_FAILED(spTransformerFactory->GetCuratedTileCollectionTransformer(
        { pUnifiedTileManager, pUser }, IID_PPV_ARGS(ppTransformer))); // 631
}
