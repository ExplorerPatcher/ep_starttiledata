#include "pch.h"

#include "CuratedTileCollection.h"

#include <wil/winrt.h>

#include "InternalAsync.h"
#include "TileCollectionInitializers.h"
#include "windowscollections.h"

typedef struct _WNF_STATE_NAME
{
    ULONG Data[2];
} WNF_STATE_NAME, *PWNF_STATE_NAME;

typedef struct _WNF_TYPE_ID
{
    GUID TypeId;
} WNF_TYPE_ID, *PWNF_TYPE_ID;

typedef const WNF_TYPE_ID *PCWNF_TYPE_ID;

extern "C" NTSYSCALLAPI
NTSTATUS NTAPI RtlPublishWnfStateData(
    WNF_STATE_NAME StateName,
    PCWNF_TYPE_ID TypeId,
    const VOID* Buffer,
    ULONG Length,
    const VOID* ExplicitScope);

inline const WNF_STATE_NAME WNF_SHEL_INSTALL_PLACEHOLDER_TILES = { 0xA3BDC075, 0x0D83063E };

struct PlaceholderTileWnf
{
    int operation;
    UINT64 userContextToken;
};

template <typename TFunctor>
bool EnumerateAllTilesInGroupRecursive(utctc::ICuratedTileGroup* group, const TFunctor& functor)
{
    wil::com_ptr<wfc::IMapView<GUID, utctc::ICuratedTile*>> tiles;
    THROW_IF_FAILED(group->GetTiles(&tiles)); // 30

    auto tilesIterable = tiles.query<wfc::IIterable<wfc::IKeyValuePair<GUID, utctc::ICuratedTile*>*>>();

    wil::com_ptr<wfc::IIterator<wfc::IKeyValuePair<GUID, utctc::ICuratedTile*>*>> tilesIterator;
    THROW_IF_FAILED(tilesIterable->First(&tilesIterator)); // 33

    bool bContinue = true;
    wil::com_ptr<wfc::IKeyValuePair<GUID, utctc::ICuratedTile*>> tileCurrent;

    boolean bTileHasCurrent = FALSE;
    THROW_IF_FAILED(tilesIterator->get_HasCurrent(&bTileHasCurrent)); // 39
    while (bTileHasCurrent)
    {
        THROW_IF_FAILED(tilesIterator->get_Current(&tileCurrent)); // 42

        GUID key;
        THROW_IF_FAILED(tileCurrent->get_Key(&key)); // 44

        wil::com_ptr<utctc::ICuratedTile> value;
        THROW_IF_FAILED(tileCurrent->get_Value(&value)); // 46

        bContinue = functor(group, key, value.get());
        if (!bContinue)
        {
            return bContinue;
        }

        THROW_IF_FAILED(tilesIterator->MoveNext(&bTileHasCurrent)); // 52
    }

    wil::com_ptr<wfc::IMapView<GUID, utctc::ICuratedTileGroup*>> groups;
    THROW_IF_FAILED(group->GetGroups(&groups)); // 58

    auto groupsIterable = groups.query<wfc::IIterable<wfc::IKeyValuePair<GUID, utctc::ICuratedTileGroup*>*>>();

    wil::com_ptr<wfc::IIterator<wfc::IKeyValuePair<GUID, utctc::ICuratedTileGroup*>*>> groupsIterator;
    THROW_IF_FAILED(groupsIterable->First(&groupsIterator)); // 62

    wil::com_ptr<wfc::IKeyValuePair<GUID, utctc::ICuratedTileGroup*>> groupCurrent;

    boolean bGroupHasCurrent = FALSE;
    THROW_IF_FAILED(groupsIterator->get_HasCurrent(&bGroupHasCurrent)); // 66
    while (bGroupHasCurrent)
    {
        THROW_IF_FAILED(groupsIterator->get_Current(&groupCurrent)); // 69

        wil::com_ptr<utctc::ICuratedTileGroup> value;
        THROW_IF_FAILED(groupCurrent->get_Value(&value)); // 71

        bContinue = EnumerateAllTilesInGroupRecursive(value.get(), functor);
        if (!bContinue)
        {
            return bContinue; // TODO Change this to break and see disasm
        }

        THROW_IF_FAILED(groupsIterator->MoveNext(&bGroupHasCurrent)); // 77
    }

    return bContinue;
}

template <typename T>
void FindCollectionParentOfTile(GUID tileId, T instance, IInspectable** outCollectionParent)
{
    *outCollectionParent = nullptr;

    wil::com_ptr<wfc::IMapView<GUID, utctc::ICuratedTile*>> tiles;
    THROW_IF_FAILED(instance->GetTiles(&tiles)); // 92

    auto tilesIterable = tiles.query<wfc::IIterable<wfc::IKeyValuePair<GUID, utctc::ICuratedTile*>*>>();

    wil::com_ptr<wfc::IIterator<wfc::IKeyValuePair<GUID, utctc::ICuratedTile*>*>> tilesIterator;
    THROW_IF_FAILED(tilesIterable->First(&tilesIterator)); // 95

    wil::com_ptr<wfc::IKeyValuePair<GUID, utctc::ICuratedTile*>> tileCurrent;

    boolean bHasCurrent = FALSE;
    THROW_IF_FAILED(tilesIterator->get_HasCurrent(&bHasCurrent)); // 99
    while (bHasCurrent)
    {
        THROW_IF_FAILED(tilesIterator->get_Current(&tileCurrent)); // 102

        wil::com_ptr<utctc::ICuratedTile> tileOther;
        THROW_IF_FAILED(tileCurrent->get_Value(&tileOther)); // 104

        GUID tileIdOther = GUID_NULL;
        THROW_IF_FAILED(tileOther->get_UniqueId(&tileIdOther)); // 106
        if (tileId == tileIdOther)
        {
            wil::com_query_to<IInspectable>(instance, outCollectionParent);
        }

        THROW_IF_FAILED(tilesIterator->MoveNext(&bHasCurrent)); // 111
    }

    if (*outCollectionParent == nullptr)
    {
        wil::com_ptr<wfc::IMapView<GUID, utctc::ICuratedTileGroup*>> groups2;
        THROW_IF_FAILED(instance->GetGroups(&groups2)); // 117

        auto groupsIterable2 = groups2.query<wfc::IIterable<wfc::IKeyValuePair<GUID, utctc::ICuratedTileGroup*>*>>();

        wil::com_ptr<wfc::IIterator<wfc::IKeyValuePair<GUID, utctc::ICuratedTileGroup*>*>> groupsIterator2;
        THROW_IF_FAILED(groupsIterable2->First(&groupsIterator2)); // 120

        wil::com_ptr<wfc::IKeyValuePair<GUID, utctc::ICuratedTileGroup*>> groupCurrent2;

        boolean bHasCurrent2 = FALSE;
        THROW_IF_FAILED(groupsIterator2->get_HasCurrent(&bHasCurrent2)); // 124
        while (bHasCurrent2)
        {
            THROW_IF_FAILED(groupsIterator2->get_Current(&groupCurrent2)); // 127

            wil::com_ptr<utctc::ICuratedTileGroup> groupOther;
            THROW_IF_FAILED(groupCurrent2->get_Value(&groupOther)); // 129
            FindCollectionParentOfGroup(tileId, groupOther.get(), outCollectionParent);
            if (*outCollectionParent != nullptr)
            {
                break;
            }

            THROW_IF_FAILED(groupsIterator2->MoveNext(&bHasCurrent2)); // 135
        }
    }
}

template <typename T>
void FindCollectionParentOfGroup(GUID groupId, T instance, IInspectable** outCollectionParent)
{
    *outCollectionParent = nullptr;

    wil::com_ptr<wfc::IMapView<GUID, utctc::ICuratedTileGroup*>> groups;
    THROW_IF_FAILED(instance->GetGroups(&groups)); // 149

    auto groupsIterable = groups.query<wfc::IIterable<wfc::IKeyValuePair<GUID, utctc::ICuratedTileGroup*>*>>();

    wil::com_ptr<wfc::IIterator<wfc::IKeyValuePair<GUID, utctc::ICuratedTileGroup*>*>> groupsIterator;
    THROW_IF_FAILED(groupsIterable->First(&groupsIterator)); // 152

    wil::com_ptr<wfc::IKeyValuePair<GUID, utctc::ICuratedTileGroup*>> groupCurrent;

    boolean bHasCurrent = FALSE;
    THROW_IF_FAILED(groupsIterator->get_HasCurrent(&bHasCurrent)); // 156
    while (bHasCurrent)
    {
        THROW_IF_FAILED(groupsIterator->get_Current(&groupCurrent)); // 159

        wil::com_ptr<utctc::ICuratedTileGroup> groupOther;
        THROW_IF_FAILED(groupCurrent->get_Value(&groupOther)); // 161

        GUID groupIdOther = GUID_NULL;
        THROW_IF_FAILED(groupOther->get_UniqueId(&groupIdOther)); // 163
        if (groupId == groupIdOther)
        {
            wil::com_query_to<IInspectable>(instance, outCollectionParent);
        }

        THROW_IF_FAILED(groupsIterator->MoveNext(&bHasCurrent)); // 168
    }

    if (*outCollectionParent == nullptr)
    {
        wil::com_ptr<wfc::IMapView<GUID, utctc::ICuratedTileGroup*>> groups2;
        THROW_IF_FAILED(instance->GetGroups(&groups2)); // 174

        auto groupsIterable2 = groups2.query<wfc::IIterable<wfc::IKeyValuePair<GUID, utctc::ICuratedTileGroup*>*>>();

        wil::com_ptr<wfc::IIterator<wfc::IKeyValuePair<GUID, utctc::ICuratedTileGroup*>*>> groupsIterator2;
        THROW_IF_FAILED(groupsIterable2->First(&groupsIterator2)); // 177

        wil::com_ptr<wfc::IKeyValuePair<GUID, utctc::ICuratedTileGroup*>> groupCurrent2;

        boolean bHasCurrent2 = FALSE;
        THROW_IF_FAILED(groupsIterator2->get_HasCurrent(&bHasCurrent2)); // 181
        while (bHasCurrent2)
        {
            THROW_IF_FAILED(groupsIterator2->get_Current(&groupCurrent2)); // 184

            wil::com_ptr<utctc::ICuratedTileGroup> groupOther;
            THROW_IF_FAILED(groupCurrent2->get_Value(&groupOther)); // 186
            FindCollectionParentOfGroup(groupId, groupOther.get(), outCollectionParent);
            if (*outCollectionParent != nullptr)
            {
                break;
            }

            THROW_IF_FAILED(groupsIterator2->MoveNext(&bHasCurrent2)); // 192
        }
    }
}

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
HRESULT CuratedTileCollectionBase::get_CollectionName(HSTRING* outResult)
{
    try
    {
        RETURN_HR(WindowsCreateString(
            _transformerRoot->GetLayoutName()->c_str(), _transformerRoot->GetLayoutName()->size(), outResult)); // 202
    } CATCH_RETURN() // 203
}

HRESULT CuratedTileCollectionBase::get_Attributes(utctc::CollectionAttributes* outResult)
{
    return E_NOTIMPL;
}

HRESULT CuratedTileCollectionBase::put_Attributes(utctc::CollectionAttributes attributes)
{
    return E_NOTIMPL;
}

HRESULT CuratedTileCollectionBase::get_Version(UINT* outResult)
{
    *outResult = _transformerRoot->GetLayoutVersion();
    return S_OK;
}

HRESULT CuratedTileCollectionBase::put_Version(UINT version)
{
    _transformerRoot->SetLayoutVersion(version);
    return S_OK;
}

HRESULT CuratedTileCollectionBase::GetGroups(wfc::IMapView<GUID, utctc::ICuratedTileGroup*>** outResult)
{
    *outResult = nullptr;

    wil::com_ptr<Windows::Foundation::Collections::Internal::HashMap<GUID, utctc::ICuratedTileGroup*>> groups;
    Windows::Foundation::Collections::Internal::HashMap<GUID, utctc::ICuratedTileGroup*>::Make(&groups);

    for (const std::pair<const GUID, wil::com_ptr<utctc::ICuratedTileGroup>>& pair : _groups)
    {
        boolean bReplaced;
        RETURN_IF_FAILED(groups->Insert(pair.first, pair.second.get(), &bReplaced)); // 242
    }

    RETURN_HR(groups->GetView(outResult)); // 244
}

HRESULT CuratedTileCollectionBase::GetTiles(wfc::IMapView<GUID, utctc::ICuratedTile*>** outResult)
{
    *outResult = nullptr;

    wil::com_ptr<Windows::Foundation::Collections::Internal::HashMap<GUID, utctc::ICuratedTile*>> tiles;
    Windows::Foundation::Collections::Internal::HashMap<GUID, utctc::ICuratedTile*>::Make(&tiles);

    for (const std::pair<const GUID, wil::com_ptr<utctc::ICuratedTile>>& pair : _tiles)
    {
        boolean bReplaced;
        tiles->Insert(pair.first, pair.second.get(), &bReplaced);
    }

    RETURN_HR(tiles->GetView(outResult)); // 260
}

HRESULT CuratedTileCollectionBase::GetAllTilesInCollection(wfc::IMapView<GUID, utctc::ICuratedTile*>** outResult)
{
    *outResult = nullptr;

    wil::com_ptr<Windows::Foundation::Collections::Internal::HashMap<GUID, utctc::ICuratedTile*>> tiles;
    Windows::Foundation::Collections::Internal::HashMap<GUID, utctc::ICuratedTile*>::Make(&tiles);

    try
    {
        for (const std::pair<const GUID, wil::com_ptr<utctc::ICuratedTile>>& pair : _tiles)
        {
            boolean bReplaced;
            tiles->Insert(pair.first, pair.second.get(), &bReplaced);
        }

        for (const std::pair<const GUID, wil::com_ptr<utctc::ICuratedTileGroup>>& pair : _groups)
        {
            EnumerateAllTilesInGroupRecursive(
                pair.second.get(),
                [tiles = tiles.get()](utctc::ICuratedTileGroup* group, REFGUID key, utctc::ICuratedTile* value) -> bool
                {
                    boolean bReplaced;
                    tiles->Insert(key, value, &bReplaced);
                    return true;
                }
            );
        }

        RETURN_HR(tiles->GetView(outResult)); // 281
    } CATCH_RETURN() // 282
}

HRESULT CuratedTileCollectionBase::DoesCollectionContainTile(
    ut::IUnifiedTileIdentifier* identifier, utctc::ICuratedTile** outTile, boolean* outResult)
{
    *outResult = false;

    try
    {
        *outResult = TryFindTileAndParentGroup(identifier, outTile, nullptr);
        return S_OK;
    } CATCH_RETURN() // 293
}

HRESULT CuratedTileCollectionBase::FindTileAndParentGroup(
    ut::IUnifiedTileIdentifier* identifier, utctc::ICuratedTile** outTile, utctc::ICuratedTileGroup** outGroup,
    boolean* outResult)
{
    *outResult = false;

    try
    {
        *outResult = TryFindTileAndParentGroup(identifier, outTile, outGroup);
        return S_OK;
    }
    CATCH_RETURN() // 305
}

HRESULT CuratedTileCollectionBase::MoveExistingGroupToNewParent(
    utctc::ICuratedTileGroup* existingGroup, utctc::ICuratedTileGroup* newParent)
{
    // wil::ActivityThreadWatcher watcher = TileCollectionTelemetry::WatchCurrentThread("MoveExistingGroupToNewParent");
    try
    {
        GUID groupId;
        RETURN_IF_FAILED(existingGroup->get_UniqueId(&groupId)); // 315

        if (newParent == nullptr)
        {
            RETURN_HR_IF(E_INVALIDARG, _groups.find(groupId) != _groups.end()); // 320
        }
        else
        {
            wil::com_ptr<utctc::ICuratedTileGroup> group;
            if (SUCCEEDED(newParent->GetGroup(groupId, &group)))
            {
                RETURN_HR_IF(E_INVALIDARG, group != nullptr); // 328
            }
        }

        wil::com_ptr<IInspectable> parent;
        FindCollectionParentOfGroup(groupId, this, &parent);
        RETURN_HR_IF(E_INVALIDARG, parent == nullptr); // 338

        auto existingGroupPrivate = wil::com_query<ICuratedTileGroupPrivate>(existingGroup);
        if (newParent == nullptr)
        {
            _transformerRoot->AddGroup(existingGroupPrivate->GetTransformerData());
            _groups[existingGroupPrivate->GetTransformerData()->GetLayoutId()] = existingGroup;
        }
        else
        {
            auto newParentPrivate = wil::com_query<ICuratedTileGroupPrivate>(newParent);
            newParentPrivate->AddGroup(existingGroupPrivate.get());
        }

        // ReSharper disable CppJoinDeclarationAndAssignment
        wil::com_ptr<ICuratedTileCollection> parentCollection;
        wil::com_ptr<utctc::ICuratedTileGroup> parentGroup;
        // ReSharper restore CppJoinDeclarationAndAssignment

        parentCollection = parent.try_query<ICuratedTileCollection>();
        if (parentCollection != nullptr)
        {
            LOG_IF_FAILED(parentCollection->RemoveGroup(groupId)); // 360
        }
        else
        {
            parentGroup = parent.query<utctc::ICuratedTileGroup>();
            LOG_IF_FAILED(parentGroup->RemoveGroup(groupId)); // 365
        }

        return S_OK;
    } CATCH_RETURN() // 369
}

HRESULT CuratedTileCollectionBase::CreateNewGroup(utctc::ICuratedTileGroup** outGroup)
{
    using namespace DataStoreCache::CuratedTileCollectionTransformer;

    // wil::ActivityThreadWatcher watcher = TileCollectionTelemetry::WatchCurrentThread("CreateNewGroup");
    *outGroup = nullptr;

    try
    {
        std::shared_ptr<CuratedGroup> transformerGroup = _transformerRoot->CreateGroup();

        wil::com_ptr<utctc::ICuratedTileGroup> group;
        RETURN_IF_FAILED(MakeAndInitialize<CuratedTileGroup>(&group, transformerGroup, this)); // 378

        _groups[transformerGroup->GetLayoutId()] = group.get();
        group.copy_to(outGroup);

        return S_OK;
    } CATCH_RETURN() // 382
}

HRESULT CuratedTileCollectionBase::GetGroup(GUID groupId, utctc::ICuratedTileGroup** outResult)
{
    *outResult = nullptr;

    auto it = _groups.find(groupId);
    try
    {
        if (it != _groups.end())
        {
            it->second.copy_to(outResult);
        }
        RETURN_HR_IF(E_INVALIDARG, *outResult == nullptr); // 395
        return S_OK;
    } CATCH_RETURN() // 396
}

HRESULT CuratedTileCollectionBase::DeleteGroup(GUID groupId)
{
    // wil::ActivityThreadWatcher watcher = TileCollectionTelemetry::WatchCurrentThread("DeleteGroup");
    return UnparentGroup(groupId, UnparentItemOptions_1);
}

HRESULT CuratedTileCollectionBase::RemoveGroup(GUID groupId)
{
    // wil::ActivityThreadWatcher watcher = TileCollectionTelemetry::WatchCurrentThread("RemoveGroup");
    return UnparentGroup(groupId, UnparentItemOptions_0);
}

HRESULT CuratedTileCollectionBase::MoveExistingTileToNewParent(utctc::ICuratedTile* existingTile, utctc::ICuratedTileGroup* newParent)
{
    // wil::ActivityThreadWatcher watcher = TileCollectionTelemetry::WatchCurrentThread("MoveExistingTileToNewParent");
    try
    {
        GUID tileId;
        RETURN_IF_FAILED(existingTile->get_UniqueId(&tileId)); // 439

        if (newParent == nullptr)
        {
            RETURN_HR_IF(E_INVALIDARG, _tiles.find(tileId) != _tiles.end()); // 444
        }
        else
        {
            wil::com_ptr<utctc::ICuratedTile> tile;
            if (SUCCEEDED(newParent->GetTile(tileId, &tile)))
            {
                RETURN_HR_IF(E_INVALIDARG, tile != nullptr); // 452
            }
        }

        wil::com_ptr<IInspectable> parent;
        FindCollectionParentOfTile(tileId, this, &parent);
        RETURN_HR_IF(E_INVALIDARG, parent == nullptr); // 462

        auto existingTilePrivate = wil::com_query<ICuratedTilePrivate>(existingTile);
        if (newParent == nullptr)
        {
            _transformerRoot->AddTile(existingTilePrivate->GetTransformerData());
            _tiles[existingTilePrivate->GetTransformerData()->GetLayoutId()] = existingTile;
        }
        else
        {
            auto newParentPrivate = wil::com_query<ICuratedTileGroupPrivate>(newParent);
            newParentPrivate->AddTile(existingTilePrivate.get());
        }

        // ReSharper disable CppJoinDeclarationAndAssignment
        wil::com_ptr<ICuratedTileCollection> parentCollection;
        wil::com_ptr<utctc::ICuratedTileGroup> parentGroup;
        // ReSharper restore CppJoinDeclarationAndAssignment

        parentCollection = parent.try_query<ICuratedTileCollection>();
        if (parentCollection != nullptr)
        {
            LOG_IF_FAILED(parentCollection->RemoveTile(tileId)); // 484
        }
        else
        {
            parentGroup = parent.query<utctc::ICuratedTileGroup>();
            LOG_IF_FAILED(parentGroup->RemoveTile(tileId)); // 489
        }

        return S_OK;
    } CATCH_RETURN() // 493
}

HRESULT CuratedTileCollectionBase::AddTile(ut::IUnifiedTileIdentifier* identifier, utctc::ICuratedTile** outResult)
{
    using namespace DataStoreCache::CuratedTileCollectionTransformer;

    // wil::ActivityThreadWatcher watcher = TileCollectionTelemetry::WatchCurrentThread("AddTile");
    *outResult = nullptr;

    try
    {
        std::shared_ptr<CuratedTile> transformerTile = _transformerRoot->CreateTile(identifier);

        wil::com_ptr<utctc::ICuratedTile> tile;
        RETURN_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CuratedTile>(&tile, transformerTile)); // 503

        _tiles[transformerTile->GetLayoutId()] = tile.get();
        LOG_IF_FAILED(OnTileAddedWithinCollection(identifier)); // 509
        tile.copy_to(outResult);

        return S_OK;
    } CATCH_RETURN() // 509
}

HRESULT CuratedTileCollectionBase::AddTileWithId(ut::IUnifiedTileIdentifier* identifier, GUID tileId, utctc::ICuratedTile** outResult)
{
    using namespace DataStoreCache::CuratedTileCollectionTransformer;

    *outResult = nullptr;

    try
    {
        std::shared_ptr<CuratedTile> transformerTile = _transformerRoot->CreateTile(tileId, identifier);

        wil::com_ptr<utctc::ICuratedTile> tile;
        RETURN_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CuratedTile>(&tile, transformerTile)); // 518

        _tiles[transformerTile->GetLayoutId()] = tile.get();
        LOG_IF_FAILED(OnTileAddedWithinCollection(identifier)); // 521
        tile.copy_to(outResult);

        return S_OK;
    } CATCH_RETURN() // 524
}

HRESULT CuratedTileCollectionBase::GetTile(GUID tileId, utctc::ICuratedTile** outResult)
{
    *outResult = nullptr;

    auto it = _tiles.find(tileId);
    try
    {
        if (it != _tiles.end())
        {
            it->second.copy_to(outResult);
        }
        RETURN_HR_IF(E_INVALIDARG, *outResult == nullptr); // 537
        return S_OK;
    } CATCH_RETURN() // 538
}

HRESULT CuratedTileCollectionBase::DeleteTile(GUID tileId)
{
    // wil::ActivityThreadWatcher watcher = TileCollectionTelemetry::WatchCurrentThread("DeleteTile");
    return UnparentTile(tileId, UnparentItemOptions_1);
}

HRESULT CuratedTileCollectionBase::RemoveTile(GUID tileId)
{
    // wil::ActivityThreadWatcher watcher = TileCollectionTelemetry::WatchCurrentThread("RemoveTile");
    return UnparentTile(tileId, UnparentItemOptions_0);
}

HRESULT CuratedTileCollectionBase::Commit()
{
    wil::com_ptr<wf::IAsyncAction> actionIgnored;
    return CommitAsync(&actionIgnored);
}

HRESULT CuratedTileCollectionBase::CommitAsync(wf::IAsyncAction** outResult)
{
    try
    {
        return CommitAsyncInternal(nullptr, outResult);
    } CATCH_RETURN() // 592
}

HRESULT CuratedTileCollectionBase::CommitAsyncWithTimerBypass(wf::IAsyncAction** outResult)
{
    using namespace DataStoreCache::CuratedTileCollectionTransformer;

    try
    {
        RETURN_IF_FAILED(CommitAsyncInternal(nullptr, outResult)); // 597

        auto inner = [this, thisStrong = wil::com_ptr(this)]() -> Concurrency::task<void>
        {
            wil::com_ptr<ICuratedTileCollectionTransformer> transformer;
            CreateCuratedTileCollectionTransformer(nullptr, _user.get(), &transformer);
            return transformer->CommitAsyncWithTimerBypass(_transformerRoot->GetLayoutName()->c_str());
        };

        RETURN_IF_FAILED(wil::run_when_complete_nothrow(*outResult, [inner = std::move(inner)]() -> void
        {
            try
            {
                (void)inner().wait();
            } CATCH_LOG() // 613
        })); // 613

        return S_OK;
    } CATCH_RETURN() // 616
}

HRESULT CuratedTileCollectionBase::ResetToDefault()
{
    using namespace DataStoreCache::CuratedTileCollectionTransformer;

    try
    {
        /*(Microsoft_Windows_ShellCommon_StartLayoutPopulationEnableBits & 0x1) != 0
           && McTemplateU0z_EventWriteTransfer(
               &Microsoft_Windows_StartLayoutPopulation_Provider_Context, &StartLayout_LayoutResetStarted,
               _transformerRoot->GetLayoutName()->c_str());*/

        if (_batchCookie != nullptr)
        {
            _transformerRoot->EndBatchUpdate(_batchCookie);
            _batchCookie = nullptr;
        }

        std::shared_ptr<BaseTileCollectionInitializer> initializer = std::make_shared<BaseTileCollectionInitializer>(_user.get());
        initializer->ReinitializeCollection(_transformerRoot->GetLayoutName()->c_str(), nullptr);

        wil::com_ptr<ICuratedTileCollectionTransformer> transformer;
        CreateCuratedTileCollectionTransformer(nullptr, _user.get(), &transformer);

        _transformerRoot = transformer->GetCuratedTileCollectionRoot(
            _transformerRoot->GetLayoutName()->c_str(), CuratedTileCollectionTransformerOptions_0);

        BeginBatchIfNecessary();
        PopulateFromTransformerData();

        return S_OK;
    } CATCH_RETURN() // 641
}

HRESULT CuratedTileCollectionBase::ResetToDefaultAsync(wf::IAsyncAction** outResult)
{
    return E_NOTIMPL;
}

HRESULT CuratedTileCollectionBase::CheckForUpdate()
{
    using namespace DataStoreCache::CuratedTileCollectionTransformer;

    try
    {
        if (_batchCookie != nullptr)
        {
            _transformerRoot->EndBatchUpdate(_batchCookie);
            _batchCookie = nullptr;
        }

        std::shared_ptr<BaseTileCollectionInitializer> initializer = std::make_shared<BaseTileCollectionInitializer>(_user.get());

        bool bUpdated = false;
        initializer->CheckForUpdate(_transformerRoot->GetLayoutName()->c_str(), &bUpdated);
        if (bUpdated)
        {
            wil::com_ptr<ICuratedTileCollectionTransformer> transformer;
            CreateCuratedTileCollectionTransformer(nullptr, _user.get(), &transformer);

            _transformerRoot = transformer->GetCuratedTileCollectionRoot(
                _transformerRoot->GetLayoutName()->c_str(), CuratedTileCollectionTransformerOptions_0);

            BeginBatchIfNecessary();
            PopulateFromTransformerData();
        }
        else
        {
            BeginBatchIfNecessary();
        }

        return S_OK;
    } CATCH_RETURN() // 680
}

HRESULT CuratedTileCollectionBase::GetCustomProperty(const HSTRING key, HSTRING* outResult)
{
    using namespace DataStoreCache::CuratedTileCollectionTransformer;

    Microsoft::WRL::Wrappers::HString result;
    try
    {
        result.Set(_transformerRoot->GetCustomProperty(WindowsGetStringRawBuffer(key, nullptr)).c_str());
        *outResult = result.Detach();
        return S_OK;
    } CATCH_RETURN() // 689
}

HRESULT CuratedTileCollectionBase::HasCustomProperty(const HSTRING key, boolean* outResult)
{
    try
    {
        *outResult = _transformerRoot->HasCustomProperty(WindowsGetStringRawBuffer(key, nullptr));
        return S_OK;
    } CATCH_RETURN() // 696
}

HRESULT CuratedTileCollectionBase::RemoveCustomProperty(const HSTRING key)
{
    try
    {
        _transformerRoot->RemoveCustomProperty(WindowsGetStringRawBuffer(key, nullptr));
        return S_OK;
    } CATCH_RETURN() // 703
}

HRESULT CuratedTileCollectionBase::SetCustomProperty(const HSTRING key, HSTRING value)
{
    try
    {
        _transformerRoot->SetCustomProperty(
            WindowsGetStringRawBuffer(key, nullptr), WindowsGetStringRawBuffer(value, nullptr));
        return S_OK;
    } CATCH_RETURN() // 712
}

HRESULT CuratedTileCollectionBase::EnsureTileRegistration()
{
    try
    {
        if (!_userContextToken && _user != nullptr)
        {
            _userContextToken = UserHelpers::GetUserContextToken(_user.get());
        }
        THROW_HR_IF(E_UNEXPECTED, !_userContextToken); // 723

        wil::com_ptr<wfc::IMapView<GUID, utctc::ICuratedTile*>> tiles;
        THROW_IF_FAILED(GetAllTilesInCollection(&tiles)); // 727
        auto tilesIterable = tiles.query<wfc::IIterable<wfc::IKeyValuePair<GUID, utctc::ICuratedTile*>*>>();

        auto packageManagerInternal = wil::ActivateInstance<ABI::Windows::Management::Deployment::Internal::IPackageManagerInternal>();

        for (const auto& pair : wil::iterable_range(tilesIterable.get()))
        {
            try
            {
                wil::com_ptr<utctc::ICuratedTile> tile;
                THROW_IF_FAILED(pair->get_Value(&tile)); // 739

                wil::com_ptr<ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier> identifier;
                THROW_IF_FAILED(tile->get_Identifier(&identifier)); // 742

                auto packagedIdentifier = identifier.try_query<ut::IPackagedUnifiedTileIdentifier>();
                if (packagedIdentifier != nullptr)
                {
                    wil::unique_hstring appId;
                    THROW_IF_FAILED(packagedIdentifier->get_AppUserModelId(&appId)); // 748
                    LOG_IF_FAILED(packageManagerInternal->EnsurePackageIsRegisteredByAumidForUser(appId.get(), _userContextToken)); // 749
                }
            } CATCH_LOG() // 752
        }

        return S_OK;
    } CATCH_RETURN() // 755
}

HRESULT CuratedTileCollectionBase::ResurrectTile(
    std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile> transformerTile, const GUID& tileId)
{
    using namespace DataStoreCache::CuratedTileCollectionTransformer;

    wil::com_ptr<ICuratedTileCollectionTransformer> transformer;
    CreateCuratedTileCollectionTransformer(nullptr, _user.get(), &transformer);

    RECT layoutRect = transformerTile->GetLayoutRect();

    wf::Size size;
    size.Width = (float)(layoutRect.right - layoutRect.left);
    size.Height = (float)(layoutRect.bottom - layoutRect.top);

    wf::Point location;
    location.X = (float)layoutRect.left;
    location.Y = (float)layoutRect.top;

    wil::com_ptr<utctc::ICuratedTileGroup> group;
    wil::com_ptr<utctc::ICuratedTile> tile;

    if (SUCCEEDED(GetGroupRecursive(tileId, &group)))
    {
        RETURN_IF_FAILED(group->AddTile(transformerTile->GetTileIdentifier().get(), &tile)); // 777
    }
    else
    {
        RETURN_IF_FAILED(AddTile(transformerTile->GetTileIdentifier().get(), &tile)); // 782
    }

    RETURN_IF_FAILED(tile->put_Location(location)); // 784
    RETURN_IF_FAILED(tile->put_Size(size)); // 785

    return S_OK;
}

HRESULT CuratedTileCollectionBase::OnTileAddedWithinCollection(ut::IUnifiedTileIdentifier* identifier)
{
    static_assert(false); // TODO Requires the type of _9 to be known
}

HRESULT CuratedTileCollectionBase::OnTileRemovedWithinCollection(ut::IUnifiedTileIdentifier* identifier)
{
    static_assert(false); // TODO Requires the type of _9 to be known
}

CuratedTileCollectionBase::CuratedTileCollectionBase()
    : _options(0x1)
    , _userContextToken(0)
    , _bInstallPlaceholderTilesOnNextCommit(false)
    , _bCommitOnDestroy(true)
{
}

CuratedTileCollectionBase::~CuratedTileCollectionBase()
{
    if (_bCommitOnDestroy)
    {
        CuratedTileCollectionBase::Commit();
    }
}

HRESULT CuratedTileCollectionBase::CommitAsyncInternal(std::function<void()>&& callback, wf::IAsyncAction** outAction)
{
    using namespace Windows::Internal;

    HRESULT hr;

    if (_batchCookie != nullptr)
    {
        bool bInstallPlaceholderTiles = _bInstallPlaceholderTilesOnNextCommit;
        _bInstallPlaceholderTilesOnNextCommit = false;

        wil::com_ptr<ABI::Windows::System::IUser> user = _user;
        Concurrency::task<void> task = _transformerRoot->EndBatchUpdate(_batchCookie);

        hr = MakeAsyncAction<Microsoft::WRL::DisableCausality>(
            ComTaskPoolHandler(TaskApartment::MTA, TaskOptions::SyncNesting), outAction, BaseTrust,
            [user, task, bInstallPlaceholderTiles, callback = std::move(callback)]() -> HRESULT
            {
                (void)task.wait();

                if (bInstallPlaceholderTiles)
                {
                    PlaceholderTileWnf data;
                    data.operation = 1;
                    data.userContextToken = UserHelpers::GetUserContextToken(user.get());
                    RtlPublishWnfStateData(WNF_SHEL_INSTALL_PLACEHOLDER_TILES, nullptr, &data, sizeof(data), nullptr);
                }

                callback();
                return S_OK;
            }
        );

        _batchCookie = nullptr; // TODO Check disasm, could it be = nullptr or .reset() ?
    }
    else
    {
        hr = MakeAsyncAction<Microsoft::WRL::DisableCausality>(
            ComTaskPoolHandler(TaskApartment::MTA, TaskOptions::SyncNesting),
            outAction, BaseTrust,
            [callback = std::move(callback)]() -> HRESULT
            {
                callback();
                return S_OK;
            }
        );
    }

    BeginBatchIfNecessary();
    RETURN_IF_FAILED(hr); // 870

    return S_OK;
}

void CuratedTileCollectionBase::BeginBatchIfNecessary()
{
    if (_transformerRoot != nullptr && (_options & 0x1) != 0)
    {
        _batchCookie = _transformerRoot->BeginBatchUpdate();
    }
}

bool CuratedTileCollectionBase::TryFindTileAndParentGroup(
    ut::IUnifiedTileIdentifier* const identifier, utctc::ICuratedTile** outTile, utctc::ICuratedTileGroup** outGroup)
{
    if (outTile != nullptr)
    {
        *outTile = nullptr;
    }
    if (outGroup != nullptr)
    {
        *outGroup = nullptr;
    }

    wil::com_ptr<utctc::ICuratedTile> foundTile;
    wil::com_ptr<utctc::ICuratedTileGroup> foundGroup;

    for (const std::pair<const GUID, wil::com_ptr<utctc::ICuratedTile>>& pair : _tiles)
    {
        wil::com_ptr<ut::IUnifiedTileIdentifier> otherIdentifier;
        THROW_IF_FAILED(pair.second->get_Identifier(&otherIdentifier)); // 987

        boolean bEqual = FALSE;
        THROW_IF_FAILED(identifier->IsEqual(otherIdentifier.get(), &bEqual)); // 990
        if (bEqual)
        {
            foundTile = pair.second.get();
            break;
        }
    }

    for (const std::pair<const GUID, wil::com_ptr<utctc::ICuratedTileGroup>>& pair : _groups)
    {
        if (TryFindTileAndParentGroupInGroup(pair.second.get(), identifier, &foundTile, &foundGroup))
        {
            break;
        }
    }

    bool bFound = foundTile != nullptr;

    if (outTile != nullptr && foundTile != nullptr)
    {
        *outTile = foundTile.detach();
    }
    if (outGroup != nullptr && foundGroup != nullptr)
    {
        *outGroup = foundGroup.detach();
    }

    return bFound;
}

bool CuratedTileCollectionBase::TryFindTileAndParentGroupInGroup(
    utctc::ICuratedTileGroup* const group, ut::IUnifiedTileIdentifier* const identifier,
    utctc::ICuratedTile** outTile, utctc::ICuratedTileGroup** outGroup)
{
    if (outTile != nullptr)
    {
        *outTile = nullptr;
    }
    if (outGroup != nullptr)
    {
        *outGroup = nullptr;
    }

    wil::com_ptr<utctc::ICuratedTile> foundTile;
    wil::com_ptr<utctc::ICuratedTileGroup> foundGroup;

    EnumerateAllTilesInGroupRecursive(
        group,
        [&foundTile, &foundGroup, identifier](utctc::ICuratedTileGroup* group, REFGUID key, utctc::ICuratedTile* value) -> bool
        {
            wil::com_ptr<ut::IUnifiedTileIdentifier> otherIdentifier;
            THROW_IF_FAILED(value->get_Identifier(&otherIdentifier)); // 1036

            boolean bEqual = FALSE;
            THROW_IF_FAILED(identifier->IsEqual(otherIdentifier.get(), &bEqual)); // 1039
            if (bEqual)
            {
                foundTile = value;
                foundGroup = group;
            }

            return bEqual != 0;
        }
    );

    bool bFound = foundTile != nullptr;

    if (outTile != nullptr && foundTile != nullptr)
    {
        *outTile = foundTile.detach();
    }
    if (outGroup != nullptr && foundGroup != nullptr)
    {
        *outGroup = foundGroup.detach();
    }

    return bFound;
}

void CuratedTileCollectionBase::PopulateFromTransformerData()
{
    _groups.clear();
    _tiles.clear();

    for (const auto& pair : _transformerRoot->GetGroups())
    {
        wil::com_ptr<utctc::ICuratedTileGroup> group;
        THROW_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CuratedTileGroup>(&group, pair.second, this)); // 893
        _groups[pair.first] = group.get();
    }

    for (const auto& pair : _transformerRoot->GetTiles())
    {
        wil::com_ptr<utctc::ICuratedTile> tile;
        THROW_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CuratedTile>(&tile, pair.second)); // 900
        _tiles[pair.first] = tile.get();
    }
}

HRESULT CuratedTileCollectionBase::GetGroupRecursive(const GUID& groupId, utctc::ICuratedTileGroup** outGroup)
{
    HRESULT hr = S_OK; // TODO Do the returns like the other overload, and check disasm
    *outGroup = nullptr;

    wil::com_ptr<utctc::ICuratedTileGroup> groupUnused;
    if (FAILED(GetGroup(groupId, outGroup)))
    {
        wil::com_ptr<wfc::IMapView<GUID, utctc::ICuratedTileGroup*>> groups;
        RETURN_IF_FAILED(GetGroups(&groups)); // 958

        for (const auto& pair : wil::iterable_range(groups.query<wfc::IIterable<wfc::IKeyValuePair<GUID, utctc::ICuratedTileGroup*>*>>().get()))
        {
            wil::com_ptr<utctc::ICuratedTileGroup> parentGroup;
            RETURN_IF_FAILED(pair->get_Value(&parentGroup)); // 964
            if (SUCCEEDED(GetGroupRecursive(parentGroup.get(), groupId, outGroup)))
            {
                return hr;
            }
        }

        hr = E_INVALIDARG;
    }

    return hr;
}

HRESULT CuratedTileCollectionBase::GetGroupRecursive(
    utctc::ICuratedTileGroup* parentGroup, const GUID& groupId, utctc::ICuratedTileGroup** outGroup)
{
    *outGroup = nullptr;
    if (SUCCEEDED(parentGroup->GetGroup(groupId, outGroup)))
    {
        return S_OK;
    }

    wil::com_ptr<wfc::IMapView<GUID, utctc::ICuratedTileGroup*>> groups;
    RETURN_IF_FAILED(parentGroup->GetGroups(&groups)); // 929

    auto groupsIterable = groups.query<wfc::IIterable<wfc::IKeyValuePair<GUID, utctc::ICuratedTileGroup*>*>>();
    for (const auto& pair : wil::iterable_range(groupsIterable.get()))
    {
        wil::com_ptr<utctc::ICuratedTileGroup> group;
        RETURN_IF_FAILED(pair->get_Value(&group)); // 935
        if (SUCCEEDED(GetGroupRecursive(group.get(), groupId, outGroup)))
        {
            return S_OK;
        }
    }

    return E_INVALIDARG;
}

HRESULT CuratedTileCollectionBase::UnparentGroup(const GUID& groupId, UnparentItemOptions options)
{
    try
    {
        auto it = _groups.find(groupId);
        if (it != _groups.end())
        {
            _groups.erase(groupId);

            if (options == UnparentItemOptions_0)
            {
                _transformerRoot->RemoveGroup(groupId);
            }
            else
            {
                _transformerRoot->DeleteGroup(groupId);
            }

            return S_OK;
        }
        RETURN_HR(E_INVALIDARG); // 429
    } CATCH_RETURN() // 430
}

HRESULT CuratedTileCollectionBase::UnparentTile(const GUID& tileId, UnparentItemOptions options)
{
    try
    {
        auto it = _tiles.find(tileId);
        if (it != _tiles.end())
        {
            wil::com_ptr<ut::IUnifiedTileIdentifier> identifier;
            THROW_IF_FAILED(it->second->get_Identifier(&identifier)); // 561

            _tiles.erase(tileId);

            if (options == UnparentItemOptions_0)
            {
                _transformerRoot->RemoveTile(tileId);
            }
            else
            {
                _transformerRoot->DeleteTile(tileId);
            }

            LOG_IF_FAILED(OnTileRemovedWithinCollection(identifier.get())); // 575
            return S_OK;
        }
        RETURN_HR(E_INVALIDARG); // 579
    } CATCH_RETURN() // 580
}
}
