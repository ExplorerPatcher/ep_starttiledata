#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <wil/com.h>

#include "DataStoreCache_MoveMe.h"

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
struct CollectionConstants
{
    static constexpr WCHAR c_startTileGridCollectionId[] = L"Start.TileGrid";
};

struct CollectionContext
{
    wil::com_ptr<ABI::Windows::System::IUser> _user;
    wil::com_ptr<IUnknown> field_8; ///< wil::com_ptr<???>
    uint32_t field_10;
    wil::com_ptr<IUnknown> field_18; ///< wil::com_ptr<???>
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedRoot> _transformerRoot;
    wil::com_ptr<IUnknown> field_30; ///< wil::com_ptr<???>
};

enum CollectionSKU
{
};

enum CollectionOfficeSKU
{
};

struct SelectionData
{
    bool _bRegionCodesSet = false;
    bool _bGroupCellWidthSet = false;
    bool _bUnk1Set = false;
    bool _bSKUsSet = false;
    bool _bOfficeSKUsSet = false;
    bool _bPreInstalledAppsEnabledSet = false;
    bool _bTargetedContentTilesEnabledSet = false;
    bool _bEducationModeEnabledSet = false;
    bool _bCommercialDeviceSet = false;
    std::vector<WCHAR*> _regionCodes; // CoTaskMemAlloc / CoTaskMemFree
    std::vector<CollectionSKU> _collectionSKUs;
    std::vector<CollectionOfficeSKU> _collectionOfficeSKUs;
    UINT _groupCellWidth = 0;
    bool _bUnk1 = false;
    bool _bPreInstalledAppsEnabled = false;
    bool _bTargetedContentTilesEnabled = false;
    bool _bEducationModeEnabled = false;
    bool _bCommercialDevice = false;
};
}

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::Internal
{
struct hashGUIDStartLayout
{
    size_t operator()(const GUID& guid) const noexcept
    {
        const uint64_t* p = reinterpret_cast<const uint64_t*>(&guid);
        return std::hash<uint64_t>{}(p[0]) ^ std::hash<uint64_t>{}(p[1]);
    }
};

enum LayoutTileType
{
    LayoutTileType_0,
    LayoutTileType_1,
    LayoutTileType_2,
    LayoutTileType_3,
};

class LayoutTile
{
public:
    LayoutTile();
    LayoutTile(
        std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile> transformerTile,
        std::shared_ptr<DataStoreCache::PlaceholderTileTransformer::PlaceholderTile> placeholderTile);
    LayoutTile(
        ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* tileIdentifier, LayoutTileType type);

    std::shared_ptr<const std::wstring> GetBackgroundColor() const;
    std::map<std::wstring, std::wstring> GetCustomProperties(); // Assumed name and signature
    std::shared_ptr<const std::wstring> GetDisplayName() const;
    GUID GetGroupId() const; // Assumed name and signature
    POINT GetLocation() const;
    SIZE GetSize() const;
    wil::com_ptr<ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier> GetTileIdentifier() const;
    const LayoutTileType GetTileType();
    GUID GetUniqueId();
    bool IsBackedByLayoutTransformer() const; // Assumed name and signature
    void SetArguments(const std::wstring& arguments);
    void SetBackgroundColor(const std::wstring& backgroundColor);
    void SetDisplayName(const std::wstring& displayName);
    void SetForegroundText(const std::wstring& foregroundText);
    void SetGroupId(const GUID& groupId); // Assumed name and signature
    void SetLocation(const POINT& location);
    void SetSize(const SIZE& size);
    void SetSquare150x150LogoUri(const std::wstring& square150x150LogoUri);
    void SetSquare310x310LogoUri(const std::wstring& square310x310LogoUri);
    void SetSquare71x71LogoUri(const std::wstring& square71x71LogoUri);
    void SetWide310x150LogoUri(const std::wstring& wide310x150LogoUri);
    std::wstring ToLoggingString();
    void UpdateTileIdentifier(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* tileIdentifier);

private:
    void EnsurePackageFamilyName(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* tileIdentifier);

    std::map<std::wstring, std::wstring> _customProperties;
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile> _transformerTile;
    std::shared_ptr<DataStoreCache::PlaceholderTileTransformer::PlaceholderTile> _placeholderTile;
    wil::com_ptr<ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier> _tileIdentifier;
    std::shared_ptr<std::wstring> _packageFamilyName;
    std::shared_ptr<std::wstring> _backgroundColor;
    std::shared_ptr<std::wstring> _displayName;
    std::shared_ptr<std::wstring> _arguments;
    std::shared_ptr<std::wstring> _square150x150LogoUri;
    std::shared_ptr<std::wstring> _square310x310LogoUri;
    std::shared_ptr<std::wstring> _square71x71LogoUri;
    std::shared_ptr<std::wstring> _foregroundText;
    std::shared_ptr<std::wstring> _wide310x150LogoUri;
    std::shared_ptr<std::wstring> field_C8;
    std::shared_ptr<std::wstring> field_D8;
    POINT _location = { -1, -1 };
    SIZE _size;
    GUID _uniqueId = GUID_NULL;
    GUID _groupId = GUID_NULL;
    GUID field_118 = GUID_NULL;
    uint32_t field_128 = 0;
    uint16_t field_12C = 0;
    uint8_t field_12E = 0;
    int field_130 = 6;
    LayoutTileType _type = LayoutTileType_0;
};

class LayoutTileInternal : public LayoutTile
{
public:
    LayoutTileInternal(
        std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile> transformerTile,
        std::shared_ptr<DataStoreCache::PlaceholderTileTransformer::PlaceholderTile> placeholderTile);
    LayoutTileInternal(
        ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* tileIdentifier, LayoutTileType type);
};

class LayoutFolder
{
public:
    LayoutFolder(const std::shared_ptr<CollectionContext>& context);
    LayoutFolder(
        const std::shared_ptr<CollectionContext>& context,
        std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup> transformerGroup);

    void AddLayoutInitializationTile(std::shared_ptr<LayoutTile> tile);
    std::shared_ptr<LayoutTile> AddTile(
        ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* tileIdentifier, LayoutTileType type);
    void GetAllTilesRecursive(
        std::vector<std::shared_ptr<LayoutTile>>& tiles,
        const std::function<bool (const std::shared_ptr<LayoutTile>&)>& shouldInclude);
    std::map<std::wstring, std::wstring> GetCustomProperties(); // Assumed name and signature
    std::shared_ptr<const std::wstring> GetDisplayName();
    std::shared_ptr<const std::wstring> GetLocalizedNameTag() const;
    POINT GetLocation() const;
    SIZE GetSize() const;
    std::shared_ptr<const std::unordered_map<GUID, std::shared_ptr<LayoutTile>, hashGUIDStartLayout>> GetTiles(); // Assumed name and signature
    GUID GetUniqueId();
    void RemoveTile(const GUID& tileId);
    void SetDisplayName(const std::wstring& displayName);
    void SetLocation(const POINT& location);

private:
    std::map<std::wstring, std::wstring> _customProperties;
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup> _transformerGroup;
    std::shared_ptr<CollectionContext> _context;
    std::shared_ptr<std::unordered_map<GUID, std::shared_ptr<LayoutTile>, hashGUIDStartLayout>> _tiles;
    std::shared_ptr<std::wstring> _displayName;
    std::shared_ptr<std::wstring> _localizedNameTag;
    POINT _location = { -1, -1 };
    SIZE _size;
    GUID _uniqueId;
};

class LayoutFolderInternal : public LayoutFolder
{
public:
    LayoutFolderInternal(const std::shared_ptr<CollectionContext>& context);
    LayoutFolderInternal(
        const std::shared_ptr<CollectionContext>& context,
        std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup> transformerGroup);
};

class LayoutGroup
{
public:
    static std::shared_ptr<LayoutGroup> CreateLayoutInitializationGroup(
        const std::shared_ptr<CollectionContext>& context);

    LayoutGroup(const std::shared_ptr<CollectionContext>& context);
    LayoutGroup(
        const std::shared_ptr<CollectionContext>& context,
        std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup> transformerGroup);

    std::shared_ptr<LayoutFolder> AddFolder();
    void AddLayoutInitializationTile(std::shared_ptr<LayoutTile> tile);
    std::shared_ptr<LayoutTile> AddTile(
        ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* tileIdentifier, LayoutTileType type);
    void GetAllTilesRecursive(
        std::vector<std::shared_ptr<LayoutTile>>& tiles,
        const std::function<bool (const std::shared_ptr<LayoutTile>&)>& shouldInclude);
    std::map<std::wstring, std::wstring> GetCustomProperties(); // Assumed name and signature
    std::shared_ptr<const std::wstring> GetDisplayName() const;
    std::shared_ptr<const std::unordered_map<GUID, std::shared_ptr<LayoutFolder>, hashGUIDStartLayout>> GetFolders(); // Assumed name and signature
    bool GetIsCustomizationLocked() const;
    std::shared_ptr<const std::wstring> GetLocalizedNameTag() const;
    POINT GetLocation() const;
    std::shared_ptr<const std::unordered_map<GUID, std::shared_ptr<LayoutTile>, hashGUIDStartLayout>> GetTiles(); // Assumed name and signature
    GUID GetUniqueId();
    void RemoveTile(const GUID& tileId);
    void SetDisplayName(const std::wstring& displayName);
    void SetLocalizedNameTag(const std::wstring& localizedNameTag);
    std::wstring ToLoggingString();

private:
    std::map<std::wstring, std::wstring> _customProperties;
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup> _transformerGroup;
    POINT _location = { -1, -1 };
    std::shared_ptr<CollectionContext> _context;
    std::shared_ptr<std::wstring> _displayName;
    std::shared_ptr<std::wstring> _localizedNameTag;
    std::shared_ptr<std::unordered_map<GUID, std::shared_ptr<LayoutTile>, hashGUIDStartLayout>> _tiles;
    std::shared_ptr<std::unordered_map<GUID, std::shared_ptr<LayoutFolder>, hashGUIDStartLayout>> _folders;
    GUID _uniqueId;
    bool _bIsCustomizationLocked = false;
    bool field_89 = false;
};

class LayoutGroupInternal : public LayoutGroup
{
public:
    LayoutGroupInternal(const std::shared_ptr<CollectionContext>& context);
    LayoutGroupInternal(
        const std::shared_ptr<CollectionContext>& context,
        std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup> group);
};

struct LayoutRoot
{
    static std::shared_ptr<LayoutRoot> __declspec(dllexport) CreateLayoutInitializationLayoutRoot(
        const std::shared_ptr<CollectionContext>& context);

    LayoutRoot(const std::shared_ptr<CollectionContext>& context);
    LayoutRoot(
        const std::shared_ptr<CollectionContext>& context,
        std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedRoot> transformerRoot);

    void AddLayoutInitializationGroup(std::shared_ptr<LayoutGroup> group);
    void GetAllTilesRecursive(
        std::vector<std::shared_ptr<LayoutTile>>& tiles,
        const std::function<bool (const std::shared_ptr<LayoutTile>&)>& shouldInclude);
    std::map<std::wstring, std::wstring> GetCustomProperties(); // Assumed name and signature
    DataStoreCache::CuratedTileCollectionTransformer::LayoutCustomizationRestrictionType GetCustomizationRestriction() const;
    int GetGroupCellWidth() const;
    int GetGroupColumnCount() const;
    std::vector<std::shared_ptr<LayoutGroup>> GetGroupsInLayoutOrder() const;
    int GetPreferredGroupColumnCount() const;
    void SetGroupCellWidth(int groupCellWidth);
    void SetGroupColumnCount(int groupColumnCount);
    void SetPreferredGroupColumnCount(int preferredGroupColumnCount);
    void SwapTile(std::shared_ptr<LayoutTile> tileFrom, std::shared_ptr<LayoutTile> tileTo);

protected:
    std::vector<std::shared_ptr<LayoutGroup>> _vector1; ///< ???
    std::vector<std::shared_ptr<LayoutGroup>> _vector2; ///< ???
    std::vector<std::shared_ptr<LayoutGroup>> _vector3; ///< ???
    std::vector<std::shared_ptr<LayoutGroup>> _vector4; ///< ???
    std::vector<std::shared_ptr<LayoutGroup>> _vector5; ///< ???
    SelectionData _selectionData;
    std::map<std::wstring, std::wstring> _customProperties;
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedRoot> _transformerRoot;
    std::vector<GUID> _layoutInitializationGroupIds;
    std::shared_ptr<std::unordered_map<GUID, std::shared_ptr<LayoutGroup>, hashGUIDStartLayout>> _groups;
    UINT _preferredGroupColumnCount = 0;
    UINT _groupColumnCount = 0;
    UINT _groupCellWidth = 0;
    std::shared_ptr<CollectionContext> _context;
    DataStoreCache::CuratedTileCollectionTransformer::LayoutCustomizationRestrictionType _customizationRestriction =
        DataStoreCache::CuratedTileCollectionTransformer::LayoutCustomizationRestrictionType_None;
};

// Hook 3 creation points
class LayoutRootInternal : public LayoutRoot
{
public:
    // Called by LayoutRoot::CreateLayoutInitializationLayoutRoot()
    // - Called by LayoutRoot::CreateLayoutInitializationLayoutRoot()
    // - Called by TDLMigrationInitialCollectionProvider::CreateLayoutFromTDLView()
    // - Called by DefaultLayoutParser::ParseStartLayouts()
    // - Called by Win8LayoutParser::ParseLayoutXml()
    // - Called by AssignedAccessAllowedAllAppListInitialCollectionProvider::RuntimeClassInitialize()
    // -> Hook LayoutRoot::CreateLayoutInitializationLayoutRoot()
    LayoutRootInternal(const std::shared_ptr<CollectionContext>& context);

    // Called by CDSLayoutProvider::RuntimeClassInitialize() with const context
    // - Called by AppendWin8UpgradeTilesPolicy::GetCustomProvider()
    // - Called by StartLayoutCmdlet::ExportStartLayout()
    // -> Hook MakeAndInitialize<CDSLayoutProvider>()
    //
    // Called by PreserveLayoutPostProcessor::RuntimeClassInitialize() with non-const context
    // - Called by CuratedTileCollections::CreatePreserveLayoutPostProcessor()
    //   - Called by GroupPolicyInitializationPolicy::GetPostProcessors()
    //     - Created by CuratedTileCollections::GetStartLayoutInitializationPolicies()
    //       - Called by Create_StartCollectionInitializationPipeline()
    //         - Called by StartTileGridCollectionInitializer::CreateStartCollectionPipeline()
    //           - Created by Create_StartTileGridCollectionInitializer()
    //             - Referenced in CollectionTypesMap
    //               - Used by FindCollectionTypesEntryForCollection()
    //                 - Called by CuratedTileCollectionManager::GetCollectionForCollectionName()
    //                   - Called by CuratedTileCollectionManager::GetCollectionWithOptions()
    //                     -> ICuratedTileCollectionManager API entry point
    //   - Called by MDMPolicyInitializationPolicy::GetPostProcessors()
    // -> Hook std::make_shared<LayoutRootInternal> in PreserveLayoutPostProcessor::RuntimeClassInitialize()
    LayoutRootInternal(
        const std::shared_ptr<CollectionContext>& context,
        std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedRoot> transformerRoot);

private:
    void InitializeStart();
    void InitializeGenericCollection();

    std::shared_ptr<CollectionContext> _context;
};
}
