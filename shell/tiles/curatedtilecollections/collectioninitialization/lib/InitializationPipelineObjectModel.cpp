#include "pch.h"

#include "InitializationPipelineObjectModel.h"

#include <appmodel.h>

#if !NUKE_SHAREDSTARTLAYOUT
#include "../../../sharedstartlayout/lib/GenericTraversalOrder.h"
#include "../../../sharedstartlayout/lib/ItemLayoutResolver.h"
#include "../../../sharedmodel/lib/SharedModelCommon.h"
#endif

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
bool IsPrimaryTile(ABI::WindowsInternal::Shell::UnifiedTile::IPackagedUnifiedTileIdentifier* packagedTileIdentifier)
{
    using namespace Microsoft::WRL::Wrappers;

    wil::unique_hstring appUserModelId;
    packagedTileIdentifier->get_AppUserModelId(&appUserModelId);

    wil::unique_hstring tileId;
    packagedTileIdentifier->get_TileId(&tileId);

    WCHAR packageFamilyName[65];
    UINT32 packageFamilyNameLength = ARRAYSIZE(packageFamilyName);
    WCHAR packageRelativeApplicationId[66];
    UINT32 packageRelativeApplicationIdLength = ARRAYSIZE(packageRelativeApplicationId);
    THROW_HR_IF(E_INVALIDARG, ParseApplicationUserModelId(
        WindowsGetStringRawBuffer(appUserModelId.get(), nullptr), &packageFamilyNameLength, packageFamilyName,
        &packageRelativeApplicationIdLength, packageRelativeApplicationId) != ERROR_SUCCESS); // 80

    INT32 compareTileIdIsNull;
    INT32 compareTileIdIsRelativeApplicationId;
    THROW_IF_FAILED(WindowsCompareStringOrdinal(tileId.get(), nullptr, &compareTileIdIsNull)); // 84
    THROW_IF_FAILED(WindowsCompareStringOrdinal(
        tileId.get(), HStringReference(packageRelativeApplicationId).Get(), &compareTileIdIsRelativeApplicationId)); // 85

    return compareTileIdIsNull == 0 || compareTileIdIsRelativeApplicationId == 0;
}
}

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::Internal
{
LayoutTile::LayoutTile()
{
    THROW_IF_FAILED(CoCreateGuid(&_uniqueId)); // 426
}

LayoutTile::LayoutTile(
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile> transformerTile,
    std::shared_ptr<DataStoreCache::PlaceholderTileTransformer::PlaceholderTile> placeholderTile)
{
    _transformerTile = transformerTile;
    _placeholderTile = placeholderTile;
}

LayoutTile::LayoutTile(
    ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* tileIdentifier, LayoutTileType type
)
    : _type(type)
{
    THROW_IF_FAILED(CoCreateGuid(&_uniqueId)); // 438
}

std::shared_ptr<const std::wstring> LayoutTile::GetBackgroundColor() const
{
    if (_placeholderTile != nullptr)
    {
        FAIL_FAST_HR(E_NOTIMPL); // TODO return _placeholderTile->GetBackgroundColor();
    }
    else
    {
        return _backgroundColor;
    }
}

std::map<std::wstring, std::wstring> LayoutTile::GetCustomProperties()
{
    return _customProperties;
}

std::shared_ptr<const std::wstring> LayoutTile::GetDisplayName() const
{
    if (_placeholderTile != nullptr)
    {
        FAIL_FAST_HR(E_NOTIMPL); // TODO return _placeholderTile->GetResolvedDisplayName();
    }
    else
    {
        return _displayName;
    }
}

GUID LayoutTile::GetGroupId() const
{
    return _groupId;
}

POINT LayoutTile::GetLocation() const
{
    if (_transformerTile != nullptr)
    {
        return _transformerTile->GetLocation();
    }
    else
    {
        return _location;
    }
}

SIZE LayoutTile::GetSize() const
{
    if (_transformerTile != nullptr)
    {
        return _transformerTile->GetSize();
    }
    else
    {
        return _size;
    }
}

wil::com_ptr<ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier> LayoutTile::GetTileIdentifier() const
{
    if (_transformerTile != nullptr)
    {
        return _transformerTile->GetTileIdentifier();
    }
    else
    {
        return _tileIdentifier;
    }
}

const LayoutTileType LayoutTile::GetTileType()
{
    using namespace ABI::WindowsInternal::Shell::UnifiedTile;

    LayoutTileType type;

    if (_transformerTile != nullptr)
    {
        UnifiedTileIdentifierKind kind;
        THROW_IF_FAILED(GetTileIdentifier()->get_Kind(&kind)); // 60
        switch (kind)
        {
            case UnifiedTileIdentifierKind_Packaged:
            {
                auto packagedTileIdentifier = GetTileIdentifier().query<IPackagedUnifiedTileIdentifier>();
                type = IsPrimaryTile(packagedTileIdentifier.get()) ? LayoutTileType_0 : LayoutTileType_2;
                break;
            }
            case UnifiedTileIdentifierKind_Win32:
            {
                type = LayoutTileType_1;
                break;
            }
            case UnifiedTileIdentifierKind_TargetedContent:
            {
                type = LayoutTileType_3;
                break;
            }
            default:
            {
                THROW_HR(E_UNEXPECTED); // 86
            }
        }
    }
    else
    {
        type = _type;
    }

    return type;
}

GUID LayoutTile::GetUniqueId()
{
    if (_transformerTile != nullptr)
    {
        return _transformerTile->GetLayoutId();
    }
    else
    {
        return _uniqueId;
    }
}

bool LayoutTile::IsBackedByLayoutTransformer() const
{
    return _transformerTile != nullptr;
}

void LayoutTile::SetArguments(const std::wstring& arguments)
{
    _arguments = std::make_shared<std::wstring>(arguments);
}

void LayoutTile::SetBackgroundColor(const std::wstring& backgroundColor)
{
    if (_placeholderTile != nullptr)
    {
        FAIL_FAST_HR(E_NOTIMPL); // TODO _placeholderTile->SetBackgroundColor(backgroundColor);
    }
    else
    {
        _backgroundColor = std::make_shared<std::wstring>(backgroundColor);
    }
}

void LayoutTile::SetDisplayName(const std::wstring& displayName)
{
    if (_placeholderTile != nullptr)
    {
        FAIL_FAST_HR(E_NOTIMPL); // TODO _placeholderTile->SetResolvedDisplayName(displayName);
    }
    else
    {
        _displayName = std::make_shared<std::wstring>(displayName);
    }
}

void LayoutTile::SetForegroundText(const std::wstring& foregroundText)
{
    _foregroundText = std::make_shared<std::wstring>(foregroundText);
}

void LayoutTile::SetGroupId(const GUID& groupId)
{
    _groupId = groupId;
}

void LayoutTile::SetLocation(const POINT& location)
{
    if (_transformerTile != nullptr)
    {
        _transformerTile->SetLocation(location);
    }
    else
    {
        _location = location;
    }
}

void LayoutTile::SetSize(const SIZE& size)
{
    if (_transformerTile != nullptr)
    {
        _transformerTile->SetSize(size);
    }
    else
    {
        _size = size;
    }
}

void LayoutTile::SetSquare150x150LogoUri(const std::wstring& square150x150LogoUri)
{
    _square150x150LogoUri = std::make_shared<std::wstring>(square150x150LogoUri);
}

void LayoutTile::SetSquare310x310LogoUri(const std::wstring& square310x310LogoUri)
{
    _square310x310LogoUri = std::make_shared<std::wstring>(square310x310LogoUri);
}

void LayoutTile::SetSquare71x71LogoUri(const std::wstring& square71x71LogoUri)
{
    _square71x71LogoUri = std::make_shared<std::wstring>(square71x71LogoUri);
}

void LayoutTile::SetWide310x150LogoUri(const std::wstring& wide310x150LogoUri)
{
    _wide310x150LogoUri = std::make_shared<std::wstring>(wide310x150LogoUri);
}

std::wstring LayoutTile::ToLoggingString()
{
    using namespace ABI::WindowsInternal::Shell::UnifiedTile;

    std::wstring str(L"Tile:\n");

    str.append(L"Identifier: ");
    wil::unique_hstring string;
    THROW_IF_FAILED(GetTileIdentifier()->get_SerializedIdentifier(&string)); // 417
    str.append(WindowsGetStringRawBuffer(string.get(), nullptr));

    str.append(L"\n");
    return str;
}

void LayoutTile::UpdateTileIdentifier(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* tileIdentifier)
{
    THROW_HR_IF_MSG(
        E_INVALIDARG, _transformerTile != nullptr,
        "It's not possible to update the identifier on a tile backe by the layout transformer."); // 44

    _tileIdentifier = tileIdentifier;
    EnsurePackageFamilyName(_tileIdentifier.get());
}

void LayoutTile::EnsurePackageFamilyName(ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* tileIdentifier)
{
    using namespace ABI::WindowsInternal::Shell::UnifiedTile;

    UnifiedTileIdentifierKind kind;
    THROW_IF_FAILED(tileIdentifier->get_Kind(&kind)); // 455
    if (kind == UnifiedTileIdentifierKind_Packaged)
    {
        auto packagedTileIdentifier = GetTileIdentifier().query<IPackagedUnifiedTileIdentifier>();

        wil::unique_hstring string;
        THROW_IF_FAILED(packagedTileIdentifier->get_AppUserModelId(&string)); // 460

        WCHAR packageFamilyName[65];
        UINT32 packageFamilyNameLength = ARRAYSIZE(packageFamilyName);
        WCHAR packageRelativeApplicationId[66];
        UINT32 packageRelativeApplicationIdLength = ARRAYSIZE(packageRelativeApplicationId);
        THROW_HR_IF(E_INVALIDARG, ParseApplicationUserModelId(
            WindowsGetStringRawBuffer(string.get(), nullptr), &packageFamilyNameLength, packageFamilyName,
            &packageRelativeApplicationIdLength, packageRelativeApplicationId) != ERROR_SUCCESS); // 473

        _packageFamilyName = std::make_shared<std::wstring>(packageFamilyName);
    }
}

LayoutTileInternal::LayoutTileInternal(
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile> transformerTile,
    std::shared_ptr<DataStoreCache::PlaceholderTileTransformer::PlaceholderTile> placeholderTile
)
    : LayoutTile(transformerTile, placeholderTile)
{
}

LayoutTileInternal::LayoutTileInternal(
    ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* tileIdentifier, LayoutTileType type
)
    : LayoutTile(tileIdentifier, type)
{
}

LayoutFolder::LayoutFolder(const std::shared_ptr<CollectionContext>& context)
    : _context(context)
{
    THROW_IF_FAILED(CoCreateGuid(&_uniqueId));
    _tiles = std::make_shared<std::unordered_map<GUID, std::shared_ptr<LayoutTile>, hashGUIDStartLayout>>();
}

LayoutFolder::LayoutFolder(
    const std::shared_ptr<CollectionContext>& context,
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup> transformerGroup
)
    : _context(context)
{
    using namespace DataStoreCache::CuratedTileCollectionTransformer;

    _tiles = std::make_shared<std::unordered_map<GUID, std::shared_ptr<LayoutTile>, hashGUIDStartLayout>>();
    _transformerGroup = transformerGroup;

    for (std::pair<const GUID, std::shared_ptr<CuratedTile>> tilePair : transformerGroup->GetTiles())
    {
        std::shared_ptr<LayoutTile> tile = std::make_shared<LayoutTileInternal>(tilePair.second, nullptr);
        _tiles->emplace(std::make_pair(tile->GetUniqueId(), tile));
    }
}

void LayoutFolder::AddLayoutInitializationTile(std::shared_ptr<LayoutTile> tile)
{
    THROW_HR_IF(E_INVALIDARG, tile != nullptr && tile->IsBackedByLayoutTransformer()); // 560

    tile->SetGroupId(_uniqueId);
    _tiles->emplace(std::make_pair(tile->GetUniqueId(), tile));
}

std::shared_ptr<LayoutTile> LayoutFolder::AddTile(
    ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* tileIdentifier, LayoutTileType type)
{
    std::shared_ptr<LayoutTile> tile;
    if (_transformerGroup != nullptr)
    {
        tile = std::make_shared<LayoutTileInternal>(_transformerGroup->CreateTile(tileIdentifier), nullptr);
    }
    else
    {
        tile = std::make_shared<LayoutTileInternal>(tileIdentifier, type);
    }

    _tiles->emplace(std::make_pair(tile->GetUniqueId(), tile));
    return tile;
}

void LayoutFolder::GetAllTilesRecursive(
    std::vector<std::shared_ptr<LayoutTile>>& tiles,
    const std::function<bool(const std::shared_ptr<LayoutTile>&)>& shouldInclude)
{
    for (const std::pair<const GUID, std::shared_ptr<LayoutTile>>& pair : *_tiles)
    {
        if (!shouldInclude || shouldInclude(pair.second))
        {
            tiles.emplace_back(pair.second);
        }
    }
}

std::map<std::wstring, std::wstring> LayoutFolder::GetCustomProperties()
{
    return _customProperties;
}

std::shared_ptr<const std::wstring> LayoutFolder::GetDisplayName()
{
    if (_transformerGroup != nullptr)
    {
        return _transformerGroup->GetDisplayName();
    }
    else
    {
        return _displayName;
    }
}

std::shared_ptr<const std::wstring> LayoutFolder::GetLocalizedNameTag() const
{
    if (_transformerGroup != nullptr)
    {
        return _transformerGroup->GetLocalizedDisplayNameResource();
    }
    else
    {
        return _localizedNameTag;
    }
}

POINT LayoutFolder::GetLocation() const
{
    if (_transformerGroup != nullptr)
    {
        return _transformerGroup->GetLocation();
    }
    else
    {
        return _location;
    }
}

SIZE LayoutFolder::GetSize() const
{
    if (_transformerGroup != nullptr)
    {
        return _transformerGroup->GetSize();
    }
    else
    {
        return _size;
    }
}

std::shared_ptr<const std::unordered_map<GUID, std::shared_ptr<LayoutTile>, hashGUIDStartLayout>> LayoutFolder::GetTiles()
{
    return _tiles;
}

GUID LayoutFolder::GetUniqueId()
{
    if (_transformerGroup != nullptr)
    {
        return _transformerGroup->GetLayoutId();
    }
    else
    {
        return _uniqueId;
    }
}

void LayoutFolder::RemoveTile(const GUID& tileId)
{
    if (_transformerGroup != nullptr)
    {
        _transformerGroup->RemoveTile(tileId);
    }
    _tiles->erase(tileId);
}

void LayoutFolder::SetDisplayName(const std::wstring& displayName)
{
    _displayName = std::make_shared<std::wstring>(displayName);
}

void LayoutFolder::SetLocation(const POINT& location)
{
    if (_transformerGroup != nullptr)
    {
        _transformerGroup->SetLocation(location);
    }
    else
    {
        _location = location;
    }
}

LayoutFolderInternal::LayoutFolderInternal(const std::shared_ptr<CollectionContext>& context)
    : LayoutFolder(context)
{
}

LayoutFolderInternal::LayoutFolderInternal(
    const std::shared_ptr<CollectionContext>& context,
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup> transformerGroup
)
    : LayoutFolder(context, transformerGroup)
{
}

std::shared_ptr<LayoutGroup> LayoutGroup::CreateLayoutInitializationGroup(
    const std::shared_ptr<CollectionContext>& context)
{
    return std::make_shared<LayoutGroupInternal>(context);
}

LayoutGroup::LayoutGroup(const std::shared_ptr<CollectionContext>& context)
    : _context(context)
{
    THROW_IF_FAILED(CoCreateGuid(&_uniqueId)); // 870
    _tiles = std::make_shared<std::unordered_map<GUID, std::shared_ptr<LayoutTile>, hashGUIDStartLayout>>();
    _folders = std::make_shared<std::unordered_map<GUID, std::shared_ptr<LayoutFolder>, hashGUIDStartLayout>>();
}

LayoutGroup::LayoutGroup(
    const std::shared_ptr<CollectionContext>& context,
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup> transformerGroup
)
    : _context(context)
{
    using namespace DataStoreCache::CuratedTileCollectionTransformer;

    _tiles = std::make_shared<std::unordered_map<GUID, std::shared_ptr<LayoutTile>, hashGUIDStartLayout>>();
    _folders = std::make_shared<std::unordered_map<GUID, std::shared_ptr<LayoutFolder>, hashGUIDStartLayout>>();

    _transformerGroup = transformerGroup;

    for (std::pair<const GUID, std::shared_ptr<CuratedTile>> tilePair : transformerGroup->GetTiles())
    {
        std::shared_ptr<LayoutTile> tile = std::make_shared<LayoutTileInternal>(tilePair.second, nullptr);
        _tiles->emplace(std::make_pair(tile->GetUniqueId(), tile));
    }

    for (std::pair<const GUID, std::shared_ptr<CuratedGroup>> groupPair : transformerGroup->GetGroups())
    {
        std::shared_ptr<LayoutFolderInternal> folder = std::make_shared<LayoutFolderInternal>(_context, groupPair.second);
        _folders->emplace(std::make_pair(folder->GetUniqueId(), folder));
    }
}

std::shared_ptr<LayoutFolder> LayoutGroup::AddFolder()
{
    std::shared_ptr<LayoutFolder> folder;
    if (_transformerGroup == nullptr)
    {
        folder = std::make_shared<LayoutFolderInternal>(_context);
    }

    _folders->emplace(std::make_pair(folder->GetUniqueId(), folder));
    return folder;
}

void LayoutGroup::AddLayoutInitializationTile(std::shared_ptr<LayoutTile> tile)
{
    THROW_HR_IF(E_INVALIDARG, tile != nullptr && tile->IsBackedByLayoutTransformer()); // 789

    tile->SetGroupId(_uniqueId);
    _tiles->emplace(std::make_pair(tile->GetUniqueId(), tile));
}

std::shared_ptr<LayoutTile> LayoutGroup::AddTile(
    ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* tileIdentifier, LayoutTileType type)
{
    using namespace DataStoreCache::CuratedTileCollectionTransformer;

    std::shared_ptr<LayoutTile> tile;
    if (_transformerGroup != nullptr)
    {
        std::shared_ptr<CuratedTile> transformerTile = _transformerGroup->CreateTile(tileIdentifier);
        tile = std::make_shared<LayoutTileInternal>(transformerTile, nullptr);
    }
    else
    {
        tile = std::make_shared<LayoutTileInternal>(tileIdentifier, type);
    }

    tile->SetGroupId(_uniqueId);
    _tiles->emplace(std::make_pair(tile->GetUniqueId(), tile));
    return tile;
}

void LayoutGroup::GetAllTilesRecursive(
    std::vector<std::shared_ptr<LayoutTile>>& tiles,
    const std::function<bool(const std::shared_ptr<LayoutTile>&)>& shouldInclude)
{
    for (const std::pair<const GUID, std::shared_ptr<LayoutFolder>>& pair : *_folders)
    {
        pair.second->GetAllTilesRecursive(tiles, shouldInclude);
    }

    for (const std::pair<const GUID, std::shared_ptr<LayoutTile>>& pair : *_tiles)
    {
        if (!shouldInclude || shouldInclude(pair.second))
        {
            tiles.emplace_back(pair.second);
        }
    }
}

std::map<std::wstring, std::wstring> LayoutGroup::GetCustomProperties()
{
    return _customProperties;
}

std::shared_ptr<const std::wstring> LayoutGroup::GetDisplayName() const
{
    if (_transformerGroup != nullptr)
    {
        return _transformerGroup->GetDisplayName();
    }
    else
    {
        return _displayName;
    }
}

std::shared_ptr<const std::unordered_map<GUID, std::shared_ptr<LayoutFolder>, hashGUIDStartLayout>> LayoutGroup::GetFolders()
{
    return _folders;
}

bool LayoutGroup::GetIsCustomizationLocked() const
{
    if (_transformerGroup != nullptr)
    {
        return _transformerGroup->GetIsLockedForCustomization();
    }
    else
    {
        return _bIsCustomizationLocked;
    }
}

std::shared_ptr<const std::wstring> LayoutGroup::GetLocalizedNameTag() const
{
    if (_transformerGroup != nullptr)
    {
        return _transformerGroup->GetLocalizedDisplayNameResource();
    }
    else
    {
        return _localizedNameTag;
    }
}

POINT LayoutGroup::GetLocation() const
{
    if (_transformerGroup != nullptr && !field_89)
    {
        return _transformerGroup->GetLocation();
    }
    else
    {
        return _location;
    }
}

std::shared_ptr<const std::unordered_map<GUID, std::shared_ptr<LayoutTile>, hashGUIDStartLayout>> LayoutGroup::GetTiles()
{
    return _tiles;
}

GUID LayoutGroup::GetUniqueId()
{
    if (_transformerGroup != nullptr)
    {
        return _transformerGroup->GetLayoutId();
    }
    else
    {
        return _uniqueId;
    }
}

void LayoutGroup::RemoveTile(const GUID& tileId)
{
    if (_transformerGroup != nullptr)
    {
        _transformerGroup->RemoveTile(tileId);
    }
    _tiles->erase(tileId);
}

void LayoutGroup::SetDisplayName(const std::wstring& displayName)
{
    _displayName = std::make_shared<std::wstring>(displayName);
}

void LayoutGroup::SetLocalizedNameTag(const std::wstring& localizedNameTag)
{
    _localizedNameTag = std::make_shared<std::wstring>(localizedNameTag);
}

std::wstring LayoutGroup::ToLoggingString()
{
    std::wstring str(L"Group:\n");

    str.append(L"Name: ");
    if (_displayName != nullptr)
    {
        str.append(_displayName->c_str());
    }
    str.append(L"\n");

    OLECHAR szGuid[39];
    if (StringFromGUID2(_uniqueId, szGuid, ARRAYSIZE(szGuid)) > 0)
    {
        str.append(L"Collection Id: ");
        str.append(szGuid, wcslen(szGuid));
        str.append(L"\n");
    }

    return str;
}

LayoutGroupInternal::LayoutGroupInternal(const std::shared_ptr<CollectionContext>& context)
    : LayoutGroup(context)
{
}

LayoutGroupInternal::LayoutGroupInternal(
    const std::shared_ptr<CollectionContext>& context,
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup> group
)
    : LayoutGroup(context, group)
{
}

LayoutRoot::LayoutRoot(const std::shared_ptr<CollectionContext>& context)
{
    _context = context;
    _groups = std::make_shared<std::unordered_map<GUID, std::shared_ptr<LayoutGroup>, hashGUIDStartLayout>>();
}

LayoutRoot::LayoutRoot(
    const std::shared_ptr<CollectionContext>& context,
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedRoot> transformerRoot)
{
    _context = context;
    _groups = std::make_shared<std::unordered_map<GUID, std::shared_ptr<LayoutGroup>, hashGUIDStartLayout>>();
    _transformerRoot = transformerRoot;
}

void LayoutRoot::AddLayoutInitializationGroup(std::shared_ptr<LayoutGroup> group)
{
    _groups->emplace(std::make_pair(group->GetUniqueId(), group));
    _layoutInitializationGroupIds.emplace_back(group->GetUniqueId());
}

void LayoutRoot::GetAllTilesRecursive(
    std::vector<std::shared_ptr<LayoutTile>>& tiles,
    const std::function<bool (const std::shared_ptr<LayoutTile>&)>& shouldInclude)
{
    for (const std::pair<const GUID, std::shared_ptr<LayoutGroup>>& pair : *_groups)
    {
        pair.second->GetAllTilesRecursive(tiles, shouldInclude);
    }
}

std::map<std::wstring, std::wstring> LayoutRoot::GetCustomProperties()
{
    return _customProperties;
}

DataStoreCache::CuratedTileCollectionTransformer::LayoutCustomizationRestrictionType LayoutRoot::GetCustomizationRestriction() const
{
    if (_transformerRoot != nullptr)
    {
        return _transformerRoot->GetLayoutCustomizationRestriction();
    }
    else
    {
        return _customizationRestriction;
    }
}

int LayoutRoot::GetGroupCellWidth() const
{
    if (_transformerRoot != nullptr)
    {
        return _transformerRoot->GetGroupCellWidth();
    }
    else
    {
        return _groupCellWidth;
    }
}

int LayoutRoot::GetGroupColumnCount() const
{
    if (_transformerRoot != nullptr)
    {
        return _transformerRoot->GetGroupColumnCount();
    }
    else
    {
        return _groupColumnCount;
    }
}

std::vector<std::shared_ptr<LayoutGroup>> LayoutRoot::GetGroupsInLayoutOrder() const
{
    std::vector<std::shared_ptr<LayoutGroup>> groups;

    for (const GUID& groupId : _layoutInitializationGroupIds)
    {
        groups.emplace_back(_groups->at(groupId));
    }

    return groups;
}

int LayoutRoot::GetPreferredGroupColumnCount() const
{
    if (_transformerRoot != nullptr)
    {
        return _transformerRoot->GetPreferredColumnCount();
    }
    else
    {
        return _preferredGroupColumnCount;
    }
}

void LayoutRoot::SetGroupCellWidth(int groupCellWidth)
{
    if (_transformerRoot != nullptr)
    {
        _transformerRoot->SetGroupCellWidth(groupCellWidth);
    }
    else
    {
        _groupCellWidth = groupCellWidth;
    }
}

void LayoutRoot::SetGroupColumnCount(int groupColumnCount)
{
    if (_transformerRoot != nullptr)
    {
        _transformerRoot->SetGroupColumnCount(groupColumnCount);
    }
    else
    {
        _groupColumnCount = groupColumnCount;
    }
}

void LayoutRoot::SetPreferredGroupColumnCount(int preferredGroupColumnCount)
{
    if (_transformerRoot != nullptr)
    {
        _transformerRoot->SetPreferredColumnCount(preferredGroupColumnCount);
    }
    else
    {
        _preferredGroupColumnCount = preferredGroupColumnCount;
    }
}

void LayoutRoot::SwapTile(std::shared_ptr<LayoutTile> tileFrom, std::shared_ptr<LayoutTile> tileTo)
{
    bool bNotFound = true;
    GUID groupId = tileFrom->GetGroupId();

    auto copyLocationAndSize = [&]() -> void
    {
        tileTo->SetLocation(tileFrom->GetLocation());
        tileTo->SetSize(tileFrom->GetSize());
    };

    std::unordered_map<GUID, std::shared_ptr<LayoutGroup>, hashGUIDStartLayout>::iterator itGroup = _groups->find(groupId);
    if (itGroup != _groups->end())
    {
        copyLocationAndSize();
        itGroup->second->RemoveTile(tileFrom->GetUniqueId());
        itGroup->second->AddLayoutInitializationTile(tileTo);

        bNotFound = false;
    }
    else
    {
        for (const std::pair<const GUID, std::shared_ptr<LayoutGroup>>& groupPair : *_groups)
        {
            auto itFolder = groupPair.second->GetFolders()->find(groupId);
            if (*itFolder == *groupPair.second->GetFolders()->end()) // @Note: This comparison of dereferenced values is sus
            {
                continue;
            }

            copyLocationAndSize();
            itFolder->second->RemoveTile(tileFrom->GetUniqueId());
            itFolder->second->AddLayoutInitializationTile(tileTo);

            bNotFound = false;
            break;
        }
    }

    THROW_HR_IF(E_UNEXPECTED, bNotFound); // 1093
}

LayoutRootInternal::LayoutRootInternal(const std::shared_ptr<CollectionContext>& context)
    : LayoutRoot(context)
    , _context(context)
{
}

LayoutRootInternal::LayoutRootInternal(
    const std::shared_ptr<CollectionContext>& context,
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedRoot> transformerRoot
)
    : LayoutRoot(context, transformerRoot)
    , _context(context)
{
    if (CompareStringOrdinal(
        transformerRoot->GetLayoutName()->c_str(), -1, CollectionConstants::c_startTileGridCollectionId, -1,
        TRUE) == CSTR_EQUAL)
    {
        InitializeStart();
    }
    else
    {
        InitializeGenericCollection();
    }
}

void LayoutRootInternal::InitializeStart()
{
    using namespace DataStoreCache::CuratedTileCollectionTransformer;

#if !NUKE_SHAREDSTARTLAYOUT
    wil::com_ptr<IItemLayoutResolver> groupsLayoutResolver;
    THROW_IF_FAILED(SharedStartLayout_CreateGroupsLayoutResolver(&groupsLayoutResolver)); // 1133

    THROW_IF_FAILED(groupsLayoutResolver->SetMaxCellBounds(
        _transformerRoot->GetPreferredColumnCount() != 0 ? _transformerRoot->GetPreferredColumnCount() : 1, -1)); // 1137
#endif

    for (std::pair<const GUID, std::shared_ptr<CuratedGroup>> groupIt : _transformerRoot->GetGroups())
    {
        std::shared_ptr<LayoutGroup> layoutGroup = std::make_shared<LayoutGroupInternal>(_context, groupIt.second);
        _groups->emplace(std::make_pair(layoutGroup->GetUniqueId(), layoutGroup));

#if !NUKE_SHAREDSTARTLAYOUT
        wil::com_ptr<IItemLayoutResolver> portraitLayoutResolver;
        THROW_IF_FAILED(SharedStartLayout_CreatePortraitLayoutResolver(&portraitLayoutResolver)); // 1145
        THROW_IF_FAILED(portraitLayoutResolver->SetMaxCellBounds(_transformerRoot->GetGroupCellWidth(), -1)); // 1146
        THROW_IF_FAILED(groupsLayoutResolver->InsertContainerUncommitted(layoutGroup->GetUniqueId(), portraitLayoutResolver.get(), layoutGroup->GetLocation())); // 1147

        for (const std::pair<const GUID, std::shared_ptr<CuratedTile>>& tileIt : groupIt.second->GetTiles())
        {
            THROW_IF_FAILED(portraitLayoutResolver->AddItem(tileIt.first, tileIt.second->GetLayoutRect())); // 1151
        }
#endif
    }

#if NUKE_SHAREDSTARTLAYOUT
    std::vector<std::pair<GUID, POINT>> tileIdsAndLocations;
    for (const std::pair<const GUID, std::shared_ptr<LayoutGroup>>& pair : *_groups)
    {
        tileIdsAndLocations.emplace_back(std::make_pair(pair.first, pair.second->GetLocation()));
    }
    std::sort(tileIdsAndLocations.begin(), tileIdsAndLocations.end(), [](const auto& a, const auto& b)
    {
        if (a.second.y != b.second.y)
            return a.second.y < b.second.y;
        return a.second.x < b.second.x;
    });

    for (const std::pair<GUID, POINT>& pair : tileIdsAndLocations)
    {
        _layoutInitializationGroupIds.emplace_back(pair.first);
    }
#else
    THROW_IF_FAILED(groupsLayoutResolver->CommitChanges()); // 1156

    wil::com_ptr<ILayoutTraversalOrder> layoutTraversalOrder;
    THROW_IF_FAILED(SharedStartLayout_CreateLayoutTraversalOrder(
        groupsLayoutResolver.get(), _transformerRoot->GetGroupCellWidth(), LayoutOrder_Linear, &layoutTraversalOrder)); // 1159

    POINT endingCell;
    GUID itemID;
    HRESULT hr = layoutTraversalOrder->GetFirst(&endingCell, &itemID);
    THROW_HR_IF(hr, FAILED(hr) && hr != TYPE_E_ELEMENTNOTFOUND); // 1166
    if (itemID != GUID_NULL)
    {
        _layoutInitializationGroupIds.emplace_back(itemID);

        POINT nextEndingCell;
        while (SUCCEEDED(layoutTraversalOrder->GetAdjacent(LayoutNavigationDirection_Right, endingCell, &nextEndingCell, &itemID)))
        {
            _layoutInitializationGroupIds.emplace_back(itemID);
            endingCell = nextEndingCell;
        }
    }
#endif
}

void LayoutRootInternal::InitializeGenericCollection()
{
    using namespace DataStoreCache::CuratedTileCollectionTransformer;

    for (std::pair<const GUID, std::shared_ptr<CuratedGroup>> groupIt : _transformerRoot->GetGroups())
    {
        std::shared_ptr<LayoutGroupInternal> layoutGroup = std::make_shared<LayoutGroupInternal>(_context, groupIt.second);
        _groups->emplace(std::make_pair(layoutGroup->GetUniqueId(), layoutGroup));
    }
}

std::shared_ptr<LayoutRoot> LayoutRoot::CreateLayoutInitializationLayoutRoot(
    const std::shared_ptr<CollectionContext>& context)
{
    return std::make_shared<LayoutRootInternal>(context);
}

// Verify the mangled name
__pragma(comment(linker, "/INCLUDE:?CreateLayoutInitializationLayoutRoot@LayoutRoot@Internal@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@SA?AV?$shared_ptr@ULayoutRoot@Internal@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@@std@@AEBV?$shared_ptr@UCollectionContext@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@@8@@Z"))
}

template
std::shared_ptr<WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::Internal::LayoutRootInternal>
__declspec(dllexport) std::make_shared<
    WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::Internal::LayoutRootInternal,
    std::shared_ptr<WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::CollectionContext>&,
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedRoot>
>(
    std::shared_ptr<WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::CollectionContext>&,
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedRoot>&&
);
