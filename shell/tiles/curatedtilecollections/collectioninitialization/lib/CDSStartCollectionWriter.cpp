#include "pch.h"

#include "CDSStartCollectionWriter.h"

#include "TransformerHelpers.h"
#include "../../../sharedmodel/lib/SharedModelCommon.h"
#include "../../../sharedstartlayout/lib/ItemLayoutResolver.h"

CommonStartTelemetry::LogAllTilesActivity_Dtor_t CommonStartTelemetry::g_pfnLogAllTilesActivity_Dtor;

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
CDSStartCollectionWriter::CDSStartCollectionWriter()
    : _b(false)
    , _columnCount(0)
{
}

HRESULT CDSStartCollectionWriter::RuntimeClassInitialize(
    std::wstring collectionName, bool b, const std::shared_ptr<CollectionContext>& context)
{
    _collectionName = collectionName;
    _b = b;
    _context = context;
    return S_OK;
}

HRESULT CDSStartCollectionWriter::WriteCollection(
    std::shared_ptr<Internal::LayoutRoot> root,
    std::shared_ptr<TileInitializationHandlerManager> tileInitializationHandlerManager,
    CommonStartTelemetry::LogAllTilesActivity activity, wf::IAsyncAction** action)
{
    auto functor = [&]() -> HRESULT
    {
        CommonStartTelemetry::LogAllTilesActivity activityLocal = activity;
        return WriteCollection(root, tileInitializationHandlerManager, activityLocal);
    };

    HRESULT hr = S_OK;

    if (action)
    {
        *action = nullptr;
        TransformerHelpers::BatchAction(_collectionName.c_str(), _context, functor, action);
    }
    else
    {
        hr = functor();
    }

    return hr;
}

bool CDSStartCollectionWriter::CanInitializeTiles()
{
    return true;
}

void CDSStartCollectionWriter::ClearCollection(
    std::vector<std::shared_ptr<Internal::LayoutTile>>& removedTiles, wf::IAsyncAction** action)
{
    if (action)
    {
        TransformerHelpers::BatchAction(_collectionName.c_str(), _context, [&]() -> HRESULT
        {
            ClearCollection(removedTiles);
            return S_OK;
        }, action);
    }
    else
    {
        ClearCollection(removedTiles);
    }
}

bool CDSStartCollectionWriter::IsCollectionEmpty()
{
    using namespace DataStoreCache;
    using namespace DataStoreCache::CuratedTileCollectionTransformer;

    if (!CDSStartCollectionWriter::DoesCollectionExist())
    {
        return true;
    }

    std::shared_ptr<CuratedRoot> root = TransformerHelpers::GetTransformerRoot(
        CollectionConstants::c_startTileGridCollectionId, _context);
    std::unordered_map<GUID, std::shared_ptr<CuratedGroup>, Util::hashGUID> groups = root->GetGroups();
    return groups.empty();
}

bool CDSStartCollectionWriter::DoesCollectionExist()
{
    return TransformerHelpers::HasTransformerRoot(CollectionConstants::c_startTileGridCollectionId, _context);
}

std::wstring CDSStartCollectionWriter::GetCollectionName()
{
    return _collectionName;
}

std::shared_ptr<CollectionContext> CDSStartCollectionWriter::GetContext()
{
    return _context;
}

void CDSStartCollectionWriter::OverrideGroupColumnCount(UINT columnCount)
{
    _columnCount = columnCount;
}

void CDSStartCollectionWriter::ClearCollection(std::vector<std::shared_ptr<Internal::LayoutTile>>& removedTiles)
{
    using namespace DataStoreCache::CuratedTileCollectionTransformer;
    using namespace Internal;

    if (TransformerHelpers::HasTransformerRoot(CollectionConstants::c_startTileGridCollectionId, _context))
    {
        std::shared_ptr<CuratedRoot> transformerRoot = TransformerHelpers::GetTransformerRoot(
            CollectionConstants::c_startTileGridCollectionId, _context);

        for (const auto& groupPair : transformerRoot->GetGroups())
        {
            for (const auto& tilePair : groupPair.second->GetTiles())
            {
                auto it = std::find_if(
                    removedTiles.begin(), removedTiles.end(), [&](const std::shared_ptr<LayoutTile>& item) -> bool
                    {
                        return item->GetUniqueId() == tilePair.first;
                    }
                );
                if (it == removedTiles.end())
                {
                    std::shared_ptr<LayoutTile> tile = std::make_shared<LayoutTileInternal>(tilePair.second, nullptr);
                    removedTiles.emplace_back(tile);
                }
            }

            transformerRoot->RemoveGroup(groupPair.first);
        }
    }
}

HRESULT CDSStartCollectionWriter::WriteCollection(
    std::shared_ptr<Internal::LayoutRoot> root,
    std::shared_ptr<TileInitializationHandlerManager> tileInitializationHandlerManager,
    CommonStartTelemetry::LogAllTilesActivity logAllTilesActivity)
{
    using namespace DataStoreCache::CuratedTileCollectionTransformer;
    using namespace Internal;

    std::vector<std::shared_ptr<LayoutTile>> removedTiles;
    ClearCollection(removedTiles);

    _tileInitializationHandlerManager = tileInitializationHandlerManager;
    // _logAllTilesActivity = logAllTilesActivity;

    /*{
        StartLayoutTelemetry::WritingStartLayoutToStorage writingStartLayoutToStorageTelemetry;
        writingStartLayoutToStorageTelemetry.StartActivity(StartLayoutTelemetryData::LayoutStorageType::Zero);

        _writingStartLayoutToStorageTelemetry = std::move(writingStartLayoutToStorageTelemetry);
    }*/

    /*(Microsoft_Windows_ShellCommon_StartLayoutPopulationEnableBits & 0x1) != 0
        && McTemplateU0z_EventWriteTransfer(
            Microsoft_Windows_StartLayoutPopulation_Provider_Context,
            &DefaultLayoutManager_WriteStartLayoutToStorage_StorageType, L"CDS");*/

    std::shared_ptr<CuratedRoot> transformerRoot = TransformerHelpers::EnsureTransformerRoot(
        _collectionName.c_str(), _context);
    _context->_transformerRoot = transformerRoot;

    {
        std::shared_ptr<ICuratedCollectionBatchCookieImpl> batchCookie = transformerRoot->BeginBatchUpdate();

        transformerRoot->SetGroupCellWidth(root->GetGroupCellWidth());
        if (_columnCount != 0)
        {
            root->SetGroupColumnCount(_columnCount);
            root->SetPreferredGroupColumnCount(_columnCount);
        }
        transformerRoot->SetGroupColumnCount(root->GetGroupColumnCount());
        transformerRoot->SetPreferredColumnCount(root->GetPreferredGroupColumnCount());
        // transformerRoot->SetFullScreenMode(root->GetSetFullScreenMode()); // Was here in 16299
        transformerRoot->SetLayoutCustomizationRestriction(root->GetCustomizationRestriction());
        for (const std::pair<const std::wstring, std::wstring>& pair : root->GetCustomProperties())
        {
            transformerRoot->SetCustomProperty(pair.first, pair.second);
        }

        THROW_IF_FAILED(SharedStartLayout_CreateGroupsLayoutResolver(&_groupsLayoutResolver)); // 84
        THROW_IF_FAILED(_groupsLayoutResolver->SetMaxCellBounds(root->GetPreferredGroupColumnCount(), -1)); // 85

        auto groupsLayoutResolverInternal = _groupsLayoutResolver.query<IItemLayoutResolverInternal>();
        groupsLayoutResolverInternal->EnableCollapse(FALSE);

        // *(LayoutCustomizationRestrictionType *)((char*)this + 648) = root->GetCustomizationRestriction(); // Was here in 16299

        for (const std::shared_ptr<LayoutGroup>& group : root->GetGroupsInLayoutOrder())
        {
            try
            {
                WriteStartGroup(group, transformerRoot);
                /*(Microsoft_Windows_ShellCommon_StartLayoutPopulationEnableBits & 0x1) != 0
                    && McTemplateU0zqq_EventWriteTransfer(
                        Microsoft_Windows_StartLayoutPopulation_Provider_Context,
                        &DefaultLayoutManager_WriteStartLayoutToStorage_GroupWriteSuccess,
                        group->GetDisplayName() != nullptr ? group->GetDisplayName()->c_str() : L"",
                        group->GetLocation().x, group->GetLocation().y);*/
            }
            catch (std::exception& ex)
            {
                (void)ex; /*(Microsoft_Windows_ShellCommon_StartLayoutPopulationEnableBits & 0x2) != 0
                    && McTemplateU0zqqs_EventWriteTransfer(
                        Microsoft_Windows_StartLayoutPopulation_Provider_Context,
                        &DefaultLayoutManager_WriteStartLayoutToStorage_GroupWriteFailure,
                        group->GetDisplayName() != nullptr ? group->GetDisplayName()->c_str() : L"",
                        group->GetLocation().x, group->GetLocation().y, ex.what());*/
            }
        }
    }

    // _writingStartLayoutToStorageTelemetry.Stop();
    return S_OK;
}

void CDSStartCollectionWriter::WriteStartGroup(
    const std::shared_ptr<Internal::LayoutGroup>& group,
    const std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedRoot>& transformerRoot)
{
    using namespace ABI::WindowsInternal::Shell::UnifiedTile;
    using namespace DataStoreCache::CuratedTileCollectionTransformer;

    std::shared_ptr<CuratedGroup> transformerGroup = transformerRoot->CreateGroup();

    if (group->GetDisplayName() != nullptr && !group->GetDisplayName()->empty())
    {
        transformerGroup->SetDisplayName(group->GetDisplayName()->c_str());
    }

    wil::com_ptr<IItemLayoutResolver> portraitLayoutResolver;
    THROW_IF_FAILED(SharedStartLayout_CreatePortraitLayoutResolver(&portraitLayoutResolver)); // 230
    THROW_IF_FAILED(portraitLayoutResolver->SetMaxCellBounds(8, -1)); // 231

    POINT groupLocation = group->GetLocation();
    if (groupLocation.x != -1 && groupLocation.y != -1)
    {
        THROW_IF_FAILED(_groupsLayoutResolver->AddContainer(
            transformerGroup->GetLayoutId(), portraitLayoutResolver.get(), groupLocation)); // 239
        transformerGroup->SetLocation(groupLocation);
    }
    else
    {
        RECT groupBounds = {};
        THROW_IF_FAILED(_groupsLayoutResolver->AddNewContainer(
            transformerGroup->GetLayoutId(), portraitLayoutResolver.get())); // 246
        THROW_IF_FAILED(_groupsLayoutResolver->GetItemBounds(transformerGroup->GetLayoutId(), &groupBounds)); // 247
        transformerGroup->SetLocation({ groupBounds.left, groupBounds.top });
    }

    transformerGroup->SetIsLockedForCustomization(group->GetIsCustomizationLocked());
    for (const std::pair<const std::wstring, std::wstring>& pair : group->GetCustomProperties())
    {
        transformerGroup->SetCustomProperty(pair.first, pair.second);
    }

    for (const std::pair<const GUID, std::shared_ptr<Internal::LayoutFolder>>& pair : *group->GetFolders())
    {
        try
        {
            WriteStartFolder(pair.second, transformerGroup, portraitLayoutResolver.get());

            /*_writingStartLayoutToStorageTelemetry.FolderWriteSuccess(pair.second);
            (Microsoft_Windows_ShellCommon_StartLayoutPopulationEnableBits & 0x1) != 0
                && McTemplateU0zqq_EventWriteTransfer(
                    Microsoft_Windows_StartLayoutPopulation_Provider_Context,
                    &DefaultLayoutManager_WriteStartLayoutToStorage_FolderWriteSuccess,
                    pair.second->GetDisplayName() != nullptr ? pair.second->GetDisplayName()->c_str() : L"",
                    pair.second->GetLocation().x, pair.second->GetLocation().y);*/
        }
        catch (std::exception& ex)
        {
            (void)ex; /*_writingStartLayoutToStorageTelemetry.FolderWriteFailure(pair.second, ex.what());
            (Microsoft_Windows_ShellCommon_StartLayoutPopulationEnableBits & 0x2) != 0
                && McTemplateU0zqq_EventWriteTransfer(
                    Microsoft_Windows_StartLayoutPopulation_Provider_Context,
                    &DefaultLayoutManager_WriteStartLayoutToStorage_FolderWriteFailure,
                    pair.second->GetDisplayName() != nullptr ? pair.second->GetDisplayName()->c_str() : L"",
                    pair.second->GetLocation().x, pair.second->GetLocation().y, ex.what());*/
        }
    }

    for (const std::pair<const GUID, std::shared_ptr<Internal::LayoutTile>>& pair : *group->GetTiles())
    {
        /*wil::unique_hstring serializedIdentifier;
        {
            wil::com_ptr<IUnifiedTileIdentifier> tileIdentifier = pair.second->GetTileIdentifier();
            THROW_IF_FAILED(tileIdentifier->get_SerializedIdentifier(&serializedIdentifier)); // 281
        }*/
        try
        {
            WriteStartTile(pair.second, transformerGroup, portraitLayoutResolver.get());

            /*_writingStartLayoutToStorageTelemetry.TileWriteSuccess(pair.second);
            (Microsoft_Windows_ShellCommon_StartLayoutPopulationEnableBits & 0x1) != 0
                && McTemplateU0z_EventWriteTransfer(
                    Microsoft_Windows_StartLayoutPopulation_Provider_Context,
                    &DefaultLayoutManager_WriteStartLayoutToStorage_TileWriteSuccess,
                    WindowsGetStringRawBuffer(serializedIdentifier.get(), nullptr));
            StartCollectionWriterTelemetryHelper::LogTileTelemetry(
                _logAllTilesActivity, pair.second, GUID_NULL, transformerGroup->GetLayoutId(), false);*/
        }
        catch (std::exception& ex)
        {
            (void)ex; /*_writingStartLayoutToStorageTelemetry.TileWriteFailure(pair.second, ex.what());
            (Microsoft_Windows_ShellCommon_StartLayoutPopulationEnableBits & 0x2) != 0
                && McTemplateU0z_EventWriteTransfer(
                    Microsoft_Windows_StartLayoutPopulation_Provider_Context,
                    &DefaultLayoutManager_WriteStartLayoutToStorage_TileWriteFailure,
                    WindowsGetStringRawBuffer(serializedIdentifier.get(), nullptr), ex.what());
            StartCollectionWriterTelemetryHelper::LogTileTelemetry(
                _logAllTilesActivity, pair.second, GUID_NULL, transformerGroup->GetLayoutId(), true);*/
        }
    }

    /*RECT groupBounds = {};
    THROW_IF_FAILED(_groupsLayoutResolver->GetItemBounds(transformerGroup->GetLayoutId(), &groupBounds)); // 299

    _logAllTilesActivity.GroupLogged(LauncherTelemetry::Group(
        groupBounds, transformerGroup->GetLayoutId(), GUID_NULL, StartCollectionWriterTelemetryHelper::s_loggingMode,
        group->GetDisplayName() != nullptr ? group->GetDisplayName()->length() : 0,
        group->GetIsCustomizationLocked() ? GroupAttributeFlags::None : GroupAttributeFlags::IsUserCustomizable));*/
}

void CDSStartCollectionWriter::WriteStartFolder(
    const std::shared_ptr<Internal::LayoutFolder>& folder,
    const std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup>& transformerGroup,
    IItemLayoutResolver* portraitLayoutResolver)
{
    using namespace DataStoreCache::CuratedTileCollectionTransformer;

    std::shared_ptr<CuratedGroup> transformerFolder = transformerGroup->CreateGroup();

    if (folder->GetDisplayName() != nullptr && !folder->GetDisplayName()->empty())
    {
        transformerFolder->SetDisplayName(folder->GetDisplayName()->c_str());
    }

    THROW_IF_FAILED(portraitLayoutResolver->AddItem(
        transformerFolder->GetLayoutId(), {
            folder->GetLocation().x, folder->GetLocation().y,
            folder->GetLocation().x + folder->GetSize().cx, folder->GetLocation().y + folder->GetSize().cy
        }
    )); // 322

    transformerFolder->SetLocation(folder->GetLocation());
    transformerFolder->SetSize(folder->GetSize());
    transformerFolder->SetIsLockedForCustomization(transformerGroup->GetIsLockedForCustomization());

    wil::com_ptr<IItemLayoutResolver> folderLayoutResolver;
    THROW_IF_FAILED(SharedStartLayout_CreatePortraitLayoutResolver(&folderLayoutResolver)); // 331
    THROW_IF_FAILED(folderLayoutResolver->SetMaxCellBounds(8, -1)); // 332

    for (const std::pair<const GUID, std::shared_ptr<Internal::LayoutTile>>& pair : *folder->GetTiles())
    {
        wil::unique_hstring string;
        THROW_IF_FAILED(pair.second->GetTileIdentifier()->get_SerializedIdentifier(&string)); // 337
        try
        {
            WriteStartTile(pair.second, transformerFolder, folderLayoutResolver.get());

            /*_writingStartLayoutToStorageTelemetry.TileWriteSuccess(pair.second);
            (Microsoft_Windows_ShellCommon_StartLayoutPopulationEnableBits & 0x1) != 0
                && McTemplateU0z_EventWriteTransfer(
                    Microsoft_Windows_StartLayoutPopulation_Provider_Context,
                    &DefaultLayoutManager_WriteStartLayoutToStorage_TileWriteSuccess,
                    WindowsGetStringRawBuffer(string.get(), nullptr));
            StartCollectionWriterTelemetryHelper::LogTileTelemetry(
                _logAllTilesActivity, pair.second, transformerFolder->GetLayoutId(), transformerGroup->GetLayoutId(),
                false);*/
        }
        catch (std::exception& ex)
        {
            (void)ex; /*_writingStartLayoutToStorageTelemetry.TileWriteFailure(pair.second, ex.what());
            (Microsoft_Windows_ShellCommon_StartLayoutPopulationEnableBits & 0x1) != 0
                && McTemplateU0z_EventWriteTransfer(
                    Microsoft_Windows_StartLayoutPopulation_Provider_Context,
                    &DefaultLayoutManager_WriteStartLayoutToStorage_TileWriteFailure,
                    WindowsGetStringRawBuffer(string.get(), nullptr), ex.what());
            StartCollectionWriterTelemetryHelper::LogTileTelemetry(
                _logAllTilesActivity, pair.second, transformerFolder->GetLayoutId(), transformerGroup->GetLayoutId(),
                true);*/
        }
    }

    /*RECT folderBounds = {};
    THROW_IF_FAILED(portraitLayoutResolver->GetItemBounds(transformerFolder->GetLayoutId(), &folderBounds)); // 355

    _logAllTilesActivity.FolderLogged(LauncherTelemetry::Folder(
        folderBounds, GUID_NULL, transformerFolder->GetLayoutId(), StartCollectionWriterTelemetryHelper::s_loggingMode,
        folder->GetDisplayName() != nullptr ? folder->GetDisplayName()->size() : 0));*/
}

void CDSStartCollectionWriter::WriteStartTile(
    const std::shared_ptr<Internal::LayoutTile>& tile,
    const std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup>& transformerGroup,
    IItemLayoutResolver* portraitLayoutResolver)
{
    using namespace DataStoreCache::CuratedTileCollectionTransformer;

    if (_tileInitializationHandlerManager != nullptr)
    {
        // _tileInitializationHandlerManager->InitializeTile(tile); // TODO Implement
    }

    std::shared_ptr<CuratedTile> transformerTile = transformerGroup->CreateTile(tile->GetTileIdentifier().get());

    THROW_IF_FAILED(portraitLayoutResolver->AddItem(
        tile->GetUniqueId(),
        {
            tile->GetLocation().x, tile->GetLocation().y,
            tile->GetLocation().x + tile->GetSize().cx, tile->GetLocation().y + tile->GetSize().cy
        }
    )); // 380

    transformerTile->SetLocation(tile->GetLocation());
    transformerTile->SetSize(tile->GetSize());
    for (const std::pair<const std::wstring, std::wstring>& pair : tile->GetCustomProperties())
    {
        transformerTile->SetCustomProperty(pair.first, pair.second);
    }
}
}

template
HRESULT __declspec(dllexport) Microsoft::WRL::Details::MakeAndInitialize<
    WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::CDSStartCollectionWriter,
    WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICollectionWriter, std::wstring&, bool,
    const std::shared_ptr<WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::CollectionContext>&
>(
    WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICollectionWriter**, std::wstring&, bool&&,
    const std::shared_ptr<WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::CollectionContext>&
);

// Verify the mangled name
__pragma(comment(linker, "/INCLUDE:??$MakeAndInitialize@VCDSStartCollectionWriter@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@UICollectionWriter@2345@AEAV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@_NAEBV?$shared_ptr@UCollectionContext@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@@8@@Details@WRL@Microsoft@@YAJPEAPEAUICollectionWriter@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@AEAV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@$$QEA_NAEBV?$shared_ptr@UCollectionContext@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@@9@@Z"))
