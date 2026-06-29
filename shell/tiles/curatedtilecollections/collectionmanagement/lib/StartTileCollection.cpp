#include "pch.h"

#include "StartTileCollection.h"

#include "CallerIdentity.h"
#include "CuratedTile.h"
#include "CuratedTileGroup.h"
#include "TileCollectionInitializers.h"
#include "usermodelptc.h"
#include "../../../inc/SecondaryTileHelpers.h"
#include "../../../inc/TileNotificationHelpers.h"
#include "../../../../common/helpers/UserHelpers.h"

#if !NUKE_SHAREDSTARTLAYOUT
#include "../../../sharedmodel/lib/SharedModelCommon.h"
#include "../../../sharedstartlayout/lib/ItemLayoutResolver.h"
#endif

namespace cdsp = ABI::WindowsInternal::Shell::CDSProperties;
namespace wrl = Microsoft::WRL;
namespace wrlw = Microsoft::WRL::Wrappers;

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
typedef std::shared_ptr<BaseTileCollectionInitializer> (*Create_StartTileGridCollectionInitializer_t)(ABI::Windows::System::IUser*);
EXTERN_C __declspec(dllexport) Create_StartTileGridCollectionInitializer_t g_pfnCreate_StartTileGridCollectionInitializer = nullptr;
#define Create_StartTileGridCollectionInitializer g_pfnCreate_StartTileGridCollectionInitializer

StartTileCollection::StartTileCollection()
{
}

StartTileCollection::~StartTileCollection()
{
    if (!_tilesPendingUnpin.empty())
    {
        wil::com_ptr<wf::IAsyncAction> actionIgnored;
        StartTileCollection::CommitAsync(&actionIgnored);
        _bCommitOnDestroy = false;
    }
}

HRESULT StartTileCollection::RuntimeClassInitialize(CuratedTileCollectionOptionsInternal options, ABI::Windows::System::IUser* user)
{
    _user = user;
    _options = options;

    /*if (Feature_EnableStartOnAFCOnWCOS)
    {
        RETURN_IF_FAILED(PrefetchStartData(user)); // 123
    }*/ // @MOD Disable this

    CreateCuratedTileCollectionTransformer(nullptr, _user.get(), &_transformer);
    _transformerRoot = _transformer->GetCuratedTileCollectionRoot(
        L"Start.TileGrid", dsct::CuratedTileCollectionTransformerOptions_0);

    PopulateFromTransformerData();
    BeginBatchIfNecessary();

    return S_OK;
}

HRESULT StartTileCollection::CommitAsync(wf::IAsyncAction** outResult)
{
    try
    {
        std::vector<std::shared_ptr<dsct::CuratedTile>> tilesPendingUnpin{ nullptr };
        {
            auto lock = _tilesPendingUnpinLock.lock_exclusive();
            tilesPendingUnpin = std::move(_tilesPendingUnpin);
        }
        return CommitAsyncInternal([tilesPendingUnpin = std::move(tilesPendingUnpin), user = wil::com_ptr(_user.get())]() -> void
        {
            for (const std::shared_ptr<dsct::CuratedTile>& pendingTile : tilesPendingUnpin)
            {
                wil::com_ptr<ut::IPackagedUnifiedTileIdentifier> packagedIdentifier;
                pendingTile->GetTileIdentifier().query_to(&packagedIdentifier);
                SecondaryTileHelpers::FindAndRemoveSecondaryTile(packagedIdentifier.get(), user.get());
            }
        }, outResult);
    } CATCH_RETURN() // 245
}

HRESULT StartTileCollection::ResetToDefault()
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

    std::shared_ptr<BaseTileCollectionInitializer> initializer = Create_StartTileGridCollectionInitializer(_user.get());
    initializer->ReinitializeCollection(L"Start.TileGrid", nullptr);

    _transformerRoot =_transformer->GetCuratedTileCollectionRoot(
        L"Start.TileGrid", dsct::CuratedTileCollectionTransformerOptions_0);

    BeginBatchIfNecessary();
    PopulateFromTransformerData();

    return S_OK;
}

HRESULT StartTileCollection::ResetToDefaultAsync(wf::IAsyncAction** outResult)
{
    /*(Microsoft_Windows_ShellCommon_StartLayoutPopulationEnableBits & 0x1) != 0
        && McTemplateU0z_EventWriteTransfer(
            &Microsoft_Windows_StartLayoutPopulation_Provider_Context, &StartLayout_LayoutResetStarted,
            _transformerRoot->GetLayoutName()->c_str());*/

    if (_batchCookie != nullptr)
    {
        (void)_transformerRoot->EndBatchUpdate(_batchCookie).wait();
        _batchCookie = nullptr;
    }

    std::shared_ptr<BaseTileCollectionInitializer> initializer = Create_StartTileGridCollectionInitializer(_user.get());
    initializer->ReinitializeCollection(L"Start.TileGrid", outResult);

    _transformerRoot =_transformer->GetCuratedTileCollectionRoot(
        L"Start.TileGrid", dsct::CuratedTileCollectionTransformerOptions_0);

    BeginBatchIfNecessary();
    PopulateFromTransformerData();

    return S_OK;
}

HRESULT StartTileCollection::CheckForUpdate()
{
    return CheckForUpdateWithOptions(StartCollectionUpdateOptions_None);
}

HRESULT StartTileCollection::ResurrectTile(std::shared_ptr<dsct::CuratedTile> transformerTile, const GUID& tileId)
{
    try
    {
        wil::com_ptr<utctc::ICuratedTileGroup> group;
        HRESULT hrGetGroup = GetGroupRecursive(tileId, &group);
        if (FAILED(hrGetGroup))
        {
            LOG_IF_FAILED_WITH_EXPECTED(hrGetGroup, E_INVALIDARG); // 534
            CreateNewLastGroup(&tileId, &group);
        }

        RECT bounds = transformerTile->GetLayoutRect();

        wf::Size size;
        size.Width = (float)(bounds.right - bounds.left);
        size.Height = (float)(bounds.bottom - bounds.top);

#if NUKE_SHAREDSTARTLAYOUT
        wf::Point location = { (float)bounds.left, (float)bounds.top };
#else
        wf::Point location;
        wil::com_ptr<IItemLayoutResolver> groupLayoutResolver = CreateLayoutResolverForGroup(group.get());
        if (SUCCEEDED(groupLayoutResolver->AddItem(transformerTile->GetLayoutId(), transformerTile->GetLayoutRect())))
        {
            location = { (float)bounds.left, (float)bounds.top };
        }
        else
        {
            location = { -1.0f, -1.0f };
        }
#endif

        wil::com_ptr<utctc::ICuratedTile> tile;
        RETURN_IF_FAILED(group->AddTileWithId(
            transformerTile->GetTileIdentifier().get(), transformerTile->GetLayoutId(), &tile)); // 558
        RETURN_IF_FAILED(tile->put_Location(location)); // 559
        RETURN_IF_FAILED(tile->put_Size(size)); // 560

        return S_OK;
    } CATCH_RETURN() // 563
}

HRESULT StartTileCollection::PinToStart(ut::IUnifiedTileIdentifier* identifier, utctc::TilePinSize size)
{
    try
    {
        if (!TryFindTileAndParentGroup(identifier, nullptr, nullptr))
        {
            _tilePinSize = size;

            boolean bIsSingleGroupModeEnabled = TRUE;
            wil::com_ptr<utctc::ICuratedCollectionSelectionHelpers> selectionHelpers;
            RETURN_IF_FAILED(Windows::Foundation::GetActivationFactoryAsUser(
                wrlw::HStringReference(RuntimeClass_WindowsInternal_Shell_UnifiedTile_CuratedTileCollections_CuratedCollectionSelectionHelpers).Get(),
                _user.get(), &selectionHelpers)); // 263
            if (FAILED_LOG(selectionHelpers->get_IsSingleGroupModeEnabled(&bIsSingleGroupModeEnabled))) // 264
            {
                bIsSingleGroupModeEnabled = TRUE;
            }

            wil::com_ptr<utctc::ICuratedTileGroup> targetGroup = FindTargetGroup(bIsSingleGroupModeEnabled != 0);
            PinTileToGroup(identifier, targetGroup.get(), nullptr);
        }

        return S_OK;
    } CATCH_RETURN() // 274
}

HRESULT StartTileCollection::PinToStartAtLocation(
    ut::IUnifiedTileIdentifier* identifier, utctc::ICuratedTileGroup* group, wf::Point location, wf::Size size)
{
    try
    {
        wil::com_ptr<utctc::ICuratedTile> foundTile;
        wil::com_ptr<utctc::ICuratedTileGroup> foundGroup;

        if (TryFindTileAndParentGroup(identifier, &foundTile, &foundGroup))
        {
            THROW_HR_IF(E_UNEXPECTED, foundGroup == nullptr); // 291

            GUID groupIdOne;
            THROW_IF_FAILED(foundGroup->get_UniqueId(&groupIdOne)); // 294 // @Note: Possible bug in OG, shouldn't a1 be group instead of foundGroup?

            GUID groupIdOther;
            THROW_IF_FAILED(foundGroup->get_UniqueId(&groupIdOther)); // 297

            THROW_HR_IF(HRESULT_FROM_WIN32(ERROR_ALREADY_ASSIGNED), groupIdOne != groupIdOther); // 299
        }
        else
        {
            RECT bounds;
            bounds.left = (int)location.X;
            bounds.top = (int)location.Y;
            bounds.right = (int)(location.X + size.Width);
            bounds.bottom = (int)(location.Y + size.Height);
            PinTileToGroup(identifier, group, &bounds);
        }

        return S_OK;
    } CATCH_RETURN() // 313
}

HRESULT StartTileCollection::UnpinFromStart(ut::IUnifiedTileIdentifier* identifier)
{
    try
    {
        wil::com_ptr<utctc::ICuratedTile> foundTile;
        wil::com_ptr<utctc::ICuratedTileGroup> foundGroup;

        THROW_HR_IF(E_NOT_SET, !TryFindTileAndParentGroup(identifier, &foundTile, &foundGroup)); // 320
        THROW_HR_IF(E_UNEXPECTED, foundGroup == nullptr); // 321

        UnpinFromStartInternal(foundTile.get(), foundGroup.get());

        wrl::ComPtr<wfc::IMapView<GUID, utctc::ICuratedTileGroup*>> groupsIterable;
        UINT numGroups;
        wrl::ComPtr<wfc::IMapView<GUID, utctc::ICuratedTile*>> tilesIterable;
        UINT numTiles;
        if ((SUCCEEDED(foundGroup->GetGroups(&groupsIterable))
                && SUCCEEDED(groupsIterable->get_Size(&numGroups)) && numGroups == 0)
            && (SUCCEEDED(foundGroup->GetTiles(&tilesIterable))
                && SUCCEEDED(tilesIterable->get_Size(&numTiles)) && numTiles == 0))
        {
            GUID groupId;
            THROW_IF_FAILED(foundGroup->get_UniqueId(&groupId)); // 336
            DeleteGroup(groupId);
        }

        return S_OK;
    } CATCH_RETURN() // 343
}

HRESULT StartTileCollection::ReplaceTinyOrMediumTile(
    ut::IUnifiedTileIdentifier* identifier, ut::IUnifiedTileIdentifier* identifier2)
{
    try
    {
        THROW_HR_IF(HRESULT_FROM_WIN32(ERROR_ALREADY_ASSIGNED), TryFindTileAndParentGroup(identifier2, nullptr, nullptr)); // 351

        wil::com_ptr<utctc::ICuratedTile> tile;
        wil::com_ptr<utctc::ICuratedTileGroup> group;

        THROW_HR_IF(E_NOT_SET, !TryFindTileAndParentGroup(identifier, &tile, &group)); // 356
        THROW_HR_IF(E_UNEXPECTED, group == nullptr); // 357

        wf::Point location;
        THROW_IF_FAILED(tile->get_Location(&location)); // 360

        wf::Size size;
        THROW_IF_FAILED(tile->get_Size(&size)); // 363

        RETURN_HR_IF(E_INVALIDARG, size.Width > 2.0f || size.Height > 2.0f || size.Width != size.Height); // 368

        UnpinFromStartInternal(tile.get(), group.get());
        PinToStartAtLocation(identifier2, group.get(), location, size);

        return S_OK;
    } CATCH_RETURN() // 375
}

HRESULT StartTileCollection::get_LastGroupId(GUID* outResult)
{
    try
    {
        *outResult = _transformerRoot->GetLastGroupId();
        return S_OK;
    } CATCH_RETURN() // 382
}

HRESULT StartTileCollection::put_LastGroupId(GUID value)
{
    try
    {
        _transformerRoot->SetLastGroupId(value);
        return S_OK;
    } CATCH_RETURN() // 389
}

HRESULT StartTileCollection::get_CustomizationRestriction(utctc::StartCollectionCustomizationRestrictionType* outResult)
{
    *outResult = utctc::StartCollectionCustomizationRestrictionType_None;
    try
    {
        switch (_transformerRoot->GetLayoutCustomizationRestriction())
        {
            case dsct::LayoutCustomizationRestrictionType_None:
                *outResult = utctc::StartCollectionCustomizationRestrictionType_None;
                break;

            case dsct::LayoutCustomizationRestrictionType_OnlySpecifiedGroups:
                *outResult = utctc::StartCollectionCustomizationRestrictionType_OnlySpecifiedGroups;
                break;

            case dsct::LayoutCustomizationRestrictionType_FullLayout:
                *outResult = utctc::StartCollectionCustomizationRestrictionType_FullLayout;
                break;
        }

        return S_OK;
    } CATCH_RETURN() // 410
}

HRESULT StartTileCollection::put_CustomizationRestriction(utctc::StartCollectionCustomizationRestrictionType value)
{
    dsct::LayoutCustomizationRestrictionType restriction;
    switch (value)
    {
        case utctc::StartCollectionCustomizationRestrictionType_None:
            restriction = dsct::LayoutCustomizationRestrictionType_None;
            break;

        case utctc::StartCollectionCustomizationRestrictionType_OnlySpecifiedGroups:
            restriction = dsct::LayoutCustomizationRestrictionType_OnlySpecifiedGroups;
            break;

        case utctc::StartCollectionCustomizationRestrictionType_FullLayout:
            restriction = dsct::LayoutCustomizationRestrictionType_FullLayout;
            break;

        default:
            return E_INVALIDARG;
    }
    try
    {
        _transformerRoot->SetLayoutCustomizationRestriction(restriction);
        return S_OK;
    } CATCH_RETURN() // 433
}

HRESULT StartTileCollection::get_GroupCellWidth(UINT* outResult)
{
    *outResult = 0;
    try
    {
        *outResult = _transformerRoot->GetGroupCellWidth();
        return S_OK;
    } CATCH_RETURN() // 441
}

HRESULT StartTileCollection::put_GroupCellWidth(UINT value)
{
    try
    {
        _transformerRoot->SetGroupCellWidth(value);
        return S_OK;
    } CATCH_RETURN() // 448
}

HRESULT StartTileCollection::get_PreferredColumnCount(UINT* outResult)
{
    *outResult = 0;
    try
    {
        *outResult = _transformerRoot->GetPreferredColumnCount();
        return S_OK;
    } CATCH_RETURN() // 456
}

HRESULT StartTileCollection::put_PreferredColumnCount(UINT value)
{
    try
    {
        _transformerRoot->SetPreferredColumnCount(value);
        return S_OK;
    } CATCH_RETURN() // 463
}

HRESULT StartTileCollection::get_CurrentColumnCount(UINT* outResult)
{
    *outResult = 0;
    try
    {
        *outResult = _transformerRoot->GetGroupColumnCount();
        return S_OK;
    } CATCH_RETURN() // 471
}

HRESULT StartTileCollection::put_CurrentColumnCount(UINT value)
{
    try
    {
        _transformerRoot->SetGroupColumnCount(value);
        return S_OK;
    } CATCH_RETURN() // 478
}

HRESULT StartTileCollection::CheckForUpdateWithOptions(StartCollectionUpdateOptions options)
{
    try
    {
        if ((options & StartCollectionUpdateOptions_ResetGroupPolicyLayoutFileTimestamp) != 0)
        {
            FILETIME timestamp = {};
            _transformerRoot->SetGroupPolicyLayoutFileTimestamp(timestamp);
        }

        if (_batchCookie != nullptr)
        {
            _transformerRoot->EndBatchUpdate(_batchCookie);
            _batchCookie = nullptr;
        }

        std::shared_ptr<BaseTileCollectionInitializer> initializer = Create_StartTileGridCollectionInitializer(_user.get());

        bool bUpdated = false;
        initializer->CheckForUpdate(_transformerRoot->GetLayoutName()->c_str(), &bUpdated);
        if (bUpdated)
        {
            _transformerRoot = _transformer->GetCuratedTileCollectionRoot(
                _transformerRoot->GetLayoutName()->c_str(), dsct::CuratedTileCollectionTransformerOptions_0);
            _batchCookie = _transformerRoot->BeginBatchUpdate();
            PopulateFromTransformerData();
        }
        else
        {
            _batchCookie = _transformerRoot->BeginBatchUpdate();
        }

        _transformer->PinStartLayoutRoot(_transformerRoot);

        return S_OK;
    } CATCH_RETURN() // 522
}

#if !NUKE_SHAREDSTARTLAYOUT
void StartTileCollection::EnsureLayoutFactory()
{
    if (_layoutFactory == nullptr)
    {
        THROW_IF_FAILED(CoCreateInstance(
            __uuidof(StartLayoutFactory), nullptr, CLSCTX_INPROC, IID_PPV_ARGS(&_layoutFactory))); // 569
    }
}
#endif

wil::com_ptr<utctc::ICuratedTileGroup> StartTileCollection::FindTargetGroup(bool bSingleGroupMode)
{
    wil::com_ptr<utctc::ICuratedTileGroup> targetGroup;

    auto it = _groups.find(_transformerRoot->GetLastGroupId());
    if (it != _groups.end())
    {
        targetGroup = it->second.get();
    }

    if (targetGroup == nullptr)
    {
        wf::Point endmostLocation = { -1.0f, -1.0f };
        for (const std::pair<const GUID, wil::com_ptr<utctc::ICuratedTileGroup>>& pair : _groups)
        {
            wf::Point location;
            THROW_IF_FAILED(pair.second->get_Location(&location)); // 594
            if ((location.Y > endmostLocation.Y || (location.Y == endmostLocation.Y && location.X > endmostLocation.X))
                && CanPinToGroup(pair.second.get(), bSingleGroupMode))
            {
                endmostLocation = location;
                targetGroup = pair.second.get();
            }
        }
        if (targetGroup != nullptr)
        {
            SetGroupAsLastGroup(targetGroup.get());
        }
    }

    if (targetGroup == nullptr || (!bSingleGroupMode && !CanPinToGroup(targetGroup.get(), false)))
    {
        CreateNewLastGroup(nullptr, &targetGroup);
        SetGroupAsLastGroup(targetGroup.get());
    }

    return targetGroup;
}

void StartTileCollection::CreateNewLastGroup(const GUID* groupId, utctc::ICuratedTileGroup** outResult)
{
    *outResult = nullptr;

#if !NUKE_SHAREDSTARTLAYOUT
    EnsureLayoutFactory();
    THROW_IF_FAILED(_layoutFactory->CreatePortraitLayoutResolver(&_lastGroupLayoutResolver)); // 627
    THROW_IF_FAILED(_lastGroupLayoutResolver->SetMaxCellBounds(_transformerRoot->GetGroupCellWidth(), -1)); // 628
#endif

    GUID groupIdLocal;
    if (groupId != nullptr)
    {
        groupIdLocal = *groupId;
    }
    else
    {
        THROW_IF_FAILED(CoCreateGuid(&groupIdLocal)); // 633
    }

#if NUKE_SHAREDSTARTLAYOUT
    POINT location = { -1, -1 };
#else
    POINT location;
    try
    {
        wil::com_ptr<IItemLayoutResolver> collectionLayoutResolver = CreateLayoutResolverForCurrentCollection();
        THROW_IF_FAILED(collectionLayoutResolver->AddNewContainer(groupIdLocal, _lastGroupLayoutResolver.get())); // 646

        RECT bounds;
        THROW_IF_FAILED(collectionLayoutResolver->GetItemBounds(groupIdLocal, &bounds)); // 650
        location = { bounds.left, bounds.top };
    }
    catch (...)
    {
        location = { -1, -1 };
    }
#endif

    std::shared_ptr<dsct::CuratedGroup> transformerGroup = _transformerRoot->CreateGroup(groupIdLocal);
    transformerGroup->SetLocation(location);

    wil::com_ptr<utctc::ICuratedTileGroup> group;
    THROW_IF_FAILED(wrl::MakeAndInitialize<CuratedTileGroup>(&group, transformerGroup, this)); // 671

    _groups[transformerGroup->GetLayoutId()] = group.get();
    group.copy_to(outResult);
}

#if !NUKE_SHAREDSTARTLAYOUT
wil::com_ptr<IItemLayoutResolver> StartTileCollection::CreateLayoutResolverForCurrentCollection()
{
    EnsureLayoutFactory();

    wil::com_ptr<IItemLayoutResolver> collectionLayoutResolver;
    _layoutFactory->CreateGroupsLayoutResolver(&collectionLayoutResolver);

    THROW_IF_FAILED(collectionLayoutResolver->SetMaxCellBounds(_transformerRoot->GetGroupColumnCount(), -1)); // 682
    THROW_IF_FAILED(collectionLayoutResolver->SetContainerMargins({ 0, 1, 0, 0 })); // 683

    for (const std::pair<const GUID, wil::com_ptr<utctc::ICuratedTileGroup>>& pair : _groups)
    {
        wil::com_ptr_nothrow<IItemLayoutResolver> groupLayoutResolver = CreateLayoutResolverForGroup(pair.second.get()); // @Note: There's a temporary here

        wf::Point location;
        THROW_IF_FAILED(pair.second->get_Location(&location)); // 691

        collectionLayoutResolver->AddContainer(pair.first, groupLayoutResolver.get(), { (int)location.X, (int)location.Y });
    }

    return collectionLayoutResolver;
}

wil::com_ptr<IItemLayoutResolver> StartTileCollection::CreateLayoutResolverForGroup(utctc::ICuratedTileGroup* group)
{
    EnsureLayoutFactory();

    wil::com_ptr<IItemLayoutResolver> groupLayoutResolver;
    _layoutFactory->CreateDesktopPortraitLayoutResolver(&groupLayoutResolver);

    groupLayoutResolver->SetMaxCellBounds(_transformerRoot->GetGroupCellWidth(), -1);

    wil::com_ptr<wfc::IMapView<GUID, utctc::ICuratedTile*>> tiles;
    THROW_IF_FAILED(group->GetTiles(&tiles)); // 706

    auto tilesIterable = tiles.query<wfc::IIterable<wfc::IKeyValuePair<GUID, utctc::ICuratedTile*>*>>();

    wil::com_ptr<wfc::IIterator<wfc::IKeyValuePair<GUID, utctc::ICuratedTile*>*>> tilesIterator;
    tilesIterable->First(&tilesIterator);

    boolean bTileHasCurrent = FALSE;
    THROW_IF_FAILED(tilesIterator->get_HasCurrent(&bTileHasCurrent)); // 713
    while (bTileHasCurrent)
    {
        wil::com_ptr<wfc::IKeyValuePair<GUID, utctc::ICuratedTile*>> pair;
        THROW_IF_FAILED(tilesIterator->get_Current(&pair)); // 717

        GUID key;
        THROW_IF_FAILED(pair->get_Key(&key)); // 720

        wil::com_ptr<utctc::ICuratedTile> value;
        THROW_IF_FAILED(pair->get_Value(&value)); // 722

        wf::Point location;
        THROW_IF_FAILED(value->get_Location(&location)); // 725

        wf::Size size;
        THROW_IF_FAILED(value->get_Size(&size)); // 727

        if (location.X != -1.0f && location.Y != -1.0f)
        {
            LOG_IF_FAILED(groupLayoutResolver->AddItem(key, {
                (int)location.X, (int)location.Y,
                (int)location.X + (int)size.Width, (int)location.Y + (int)size.Height
            })); // 734
        }

        tilesIterator->MoveNext(&bTileHasCurrent);
    }

    wil::com_ptr<wfc::IMapView<GUID, utctc::ICuratedTileGroup*>> groups;
    THROW_IF_FAILED(group->GetGroups(&groups)); // 741

    auto groupsIterable = groups.query<wfc::IIterable<wfc::IKeyValuePair<GUID, utctc::ICuratedTileGroup*>*>>();

    wil::com_ptr<wfc::IIterator<wfc::IKeyValuePair<GUID, utctc::ICuratedTileGroup*>*>> groupsIterator;
    groupsIterable->First(&groupsIterator);

    boolean bGroupHasCurrent = FALSE;
    THROW_IF_FAILED(groupsIterator->get_HasCurrent(&bGroupHasCurrent)); // 748
    while (bGroupHasCurrent)
    {
        wil::com_ptr<wfc::IKeyValuePair<GUID, utctc::ICuratedTileGroup*>> pair;
        THROW_IF_FAILED(groupsIterator->get_Current(&pair)); // 752

        GUID key;
        THROW_IF_FAILED(pair->get_Key(&key)); // 755

        wil::com_ptr<utctc::ICuratedTileGroup> value;
        THROW_IF_FAILED(pair->get_Value(&value)); // 757

        wf::Point location;
        THROW_IF_FAILED(value->get_Location(&location)); // 760

        wf::Size size;
        THROW_IF_FAILED(value->get_Size(&size)); // 762

        if (size.Width != 0.0f && size.Height != 0.0f)
        {
            LOG_IF_FAILED(groupLayoutResolver->AddItem(key, {
                (int)location.X, (int)location.Y,
                (int)location.X + (int)size.Width, (int)location.Y + (int)size.Height
            })); // 767
        }

        groupsIterator->MoveNext(&bGroupHasCurrent);
    }

    return groupLayoutResolver;
}
#endif

bool StartTileCollection::CanPinToGroup(utctc::ICuratedTileGroup* group, bool bSingleGroupMode)
{
    bool bCanPinToGroup = false;

    if (bSingleGroupMode)
    {
        LOG_HR_IF(E_UNEXPECTED, _groups.size() > 1); // 782
        bCanPinToGroup = true;
    }
    else
    {
        wil::unique_hstring displayName;
        THROW_IF_FAILED(group->get_DisplayName(&displayName)); // 790
        if (WindowsIsStringEmpty(displayName.get()))
        {
            boolean bLockedForCustomization = FALSE;
            THROW_IF_FAILED(group->get_LockedForCustomization(&bLockedForCustomization)); // 800
#if NUKE_SHAREDSTARTLAYOUT
            bCanPinToGroup = !bLockedForCustomization;
#else
            if (!bLockedForCustomization)
            {
                if (_lastGroupLayoutResolver == nullptr)
                {
                    _lastGroupLayoutResolver = CreateLayoutResolverForGroup(group);
                }

                RECT boundsBefore;
                THROW_IF_FAILED(_lastGroupLayoutResolver->GetLayoutBounds(&boundsBefore)); // 817
                if (boundsBefore.bottom - boundsBefore.top > 0)
                {
                    _lastGroupLayoutResolver->AddNewItem(
                        GUID_NULL, _tilePinSize == utctc::TilePinSize_Tile4x2 ? SIZE{ 4, 2 } : SIZE{ 2, 2 });

                    RECT boundsAfter;
                    THROW_IF_FAILED(_lastGroupLayoutResolver->GetLayoutBounds(&boundsAfter)); // 825

                    bCanPinToGroup = boundsAfter.bottom - boundsAfter.top <= 8;

                    _lastGroupLayoutResolver->RemoveItemUncommitted(GUID_NULL);
                    THROW_IF_FAILED(_lastGroupLayoutResolver->CommitChanges()); // 835
                }
            }
#endif
        }
    }

    return bCanPinToGroup;
}

void StartTileCollection::SetGroupAsLastGroup(utctc::ICuratedTileGroup* group)
{
    GUID groupId;
    THROW_IF_FAILED(group->get_UniqueId(&groupId)); // 854

    _transformerRoot->SetLastGroupId(groupId);
}

void StartTileCollection::PinTileToGroup(
    ut::IUnifiedTileIdentifier* identifier, utctc::ICuratedTileGroup* group, const RECT* bounds)
{
    wil::com_ptr<utctc::ICuratedTile> tile;
    RECT tileBounds;
    bool bShouldScrollIntoView = true;

    if (bounds != nullptr)
    {
        group->AddTile(identifier, &tile);
        tileBounds = *bounds;
    }
    else
    {
#if NUKE_SHAREDSTARTLAYOUT
        group->AddTile(identifier, &tile);
        SIZE size = _tilePinSize == utctc::TilePinSize_Tile4x2 ? SIZE{ 4, 2 } : SIZE{ 2, 2 };
        tileBounds = { -1, -1, size.cx - 1, size.cy - 1 };
#else
        wil::com_ptr<IItemLayoutResolver> groupLayoutResolver = CreateLayoutResolverForGroup(group);

        group->AddTile(identifier, &tile);

        GUID tileId;
        tile->get_UniqueId(&tileId);

        THROW_IF_FAILED(groupLayoutResolver->AddNewItem(
            tileId, _tilePinSize == utctc::TilePinSize_Tile4x2 ? SIZE{ 4, 2 } : SIZE{ 2, 2 })); // 880

        THROW_IF_FAILED(groupLayoutResolver->GetItemBounds(tileId, &tileBounds)); // 882
#endif
    }
    THROW_IF_FAILED(tile->put_Location({ (float)tileBounds.left, (float)tileBounds.top })); // 893
    THROW_IF_FAILED(tile->put_Size({(float)(tileBounds.right - tileBounds.left), (float)(tileBounds.bottom - tileBounds.top)})); // 894

    wrl::ComPtr<cdsp::ICDSTilePropertiesBatchedFactory> cdsBatchedFactory;
    THROW_IF_FAILED(wf::GetActivationFactory(
        wrlw::HStringReference(RuntimeClass_WindowsInternal_Shell_CDSProperties_CDSTilePropertiesBatched).Get(),
        &cdsBatchedFactory)); // 897

    cdsp::CDSTilePropertiesKinds kinds;
    if (SUCCEEDED(CallerIdentity::EnsureCallingProcessIsShellExperience()))
    {
        kinds = cdsp::CDSTilePropertiesKinds_Local | cdsp::CDSTilePropertiesKinds_Roamed;
    }
    else
    {
        bShouldScrollIntoView = false;
        kinds = cdsp::CDSTilePropertiesKinds_Roamed;
    }

    wrl::ComPtr<cdsp::ICDSTilePropertiesBatched> cdsBatched;
    THROW_IF_FAILED(cdsBatchedFactory->Create(_user.get(), kinds, &cdsBatched)); // 907

    wil::unique_hstring serializedId;
    THROW_IF_FAILED(identifier->get_SerializedIdentifier(&serializedId)); // 910

    if (bShouldScrollIntoView)
    {
        THROW_IF_FAILED(tile->SetCustomProperty(
            wrlw::HStringReference(L"ShouldScrollIntoView").Get(), wrlw::HStringReference(L"True").Get())); // 914
    }
    else
    {
        boolean bHas;
        LOG_IF_FAILED(tile->HasCustomProperty(wrlw::HStringReference(L"ShouldScrollIntoView").Get(), &bHas)); // 919
        if (bHas)
        {
            LOG_IF_FAILED(tile->RemoveCustomProperty(wrlw::HStringReference(L"ShouldScrollIntoView").Get())); // 923
        }
    }

    wrl::ComPtr<cdsp::ICDSTilePropertiesItem> cdsTilePropertiesItem;
    THROW_IF_FAILED(cdsBatched->GetRoamedProperties(serializedId.get(), &cdsTilePropertiesItem)); // 929

    wrl::ComPtr<cdsp::ICDSRoamedTileProperties> cdsRoamedTileProperties;
    THROW_IF_FAILED(cdsTilePropertiesItem.As(&cdsRoamedTileProperties)); // 932

    boolean bIsLiveTileDisabled;
    THROW_IF_FAILED(cdsRoamedTileProperties->get_IsLiveTileDisabled(&bIsLiveTileDisabled)); // 935

    TileNotificationHelpers::EnableNotifications(
        identifier, UserHelpers::GetUserContextToken(_user.get()), !bIsLiveTileDisabled);
}

bool IsPrimaryTile(ut::IPackagedUnifiedTileIdentifier* packagedTileIdentifier);

void StartTileCollection::UnpinFromStartInternal(utctc::ICuratedTile* const tile, utctc::ICuratedTileGroup* const group)
{
    wrl::ComPtr<ICuratedTilePrivate> tilePriv;
    THROW_IF_FAILED(tile->QueryInterface(IID_PPV_ARGS(&tilePriv))); // 956

    std::shared_ptr<dsct::CuratedTile> transformerTile = tilePriv->GetTransformerData();
    wil::com_ptr<ut::IUnifiedTileIdentifier> identifier = transformerTile->GetTileIdentifier();

    TileNotificationHelpers::EnableNotifications(identifier.get(), UserHelpers::GetUserContextToken(_user.get()), false);

    bool bIsPrimaryTile = true;

    ut::UnifiedTileIdentifierKind kind;
    THROW_IF_FAILED(identifier->get_Kind(&kind)); // 968
    if (kind == ut::UnifiedTileIdentifierKind_Packaged)
    {
        auto packagedIdentifier = identifier.query<ut::IPackagedUnifiedTileIdentifier>();
        bIsPrimaryTile = IsPrimaryTile(packagedIdentifier.get());
    }

    THROW_IF_FAILED(group->DeleteTile(transformerTile->GetLayoutId())); // 976

    if (!bIsPrimaryTile)
    {
        auto lock = _tilesPendingUnpinLock.lock_exclusive();
        _tilesPendingUnpin.emplace_back(transformerTile);
    }

    wrl::ComPtr<cdsp::ICDSTilePropertiesBatchedFactory> cdsBatchedFactory;

    THROW_IF_FAILED(wf::GetActivationFactory(
        wrlw::HStringReference(RuntimeClass_WindowsInternal_Shell_CDSProperties_CDSTilePropertiesBatched).Get(),
        &cdsBatchedFactory)); // 991

    wrl::ComPtr<cdsp::ICDSTilePropertiesBatched> cdsBatched;
    THROW_IF_FAILED(cdsBatchedFactory->Create(_user.get(), cdsp::CDSTilePropertiesKinds_Roamed, &cdsBatched)); // 994

    wil::unique_hstring serializedId;
    THROW_IF_FAILED(identifier->get_SerializedIdentifier(&serializedId)); // 997

    wrl::ComPtr<cdsp::ICDSTilePropertiesItem> cdsTilePropertiesItem;
    THROW_IF_FAILED(cdsBatched->GetRoamedProperties(serializedId.get(), &cdsTilePropertiesItem)); // 1001

    wrl::ComPtr<cdsp::ICDSRoamedTileProperties> cdsRoamedTileProperties;
    THROW_IF_FAILED(cdsTilePropertiesItem.As(&cdsRoamedTileProperties)); // 1004

    boolean bIsUserPinned = FALSE;
    if (SUCCEEDED_LOG(cdsRoamedTileProperties->get_IsUserPinned(&bIsUserPinned)) && bIsUserPinned) // 1007
    {
        auto userPinBrokerStatics = wil::GetActivationFactory<utp::IUnifiedTileUserPinBrokerStatics>(
            RuntimeClass_WindowsInternal_Shell_UnifiedTile_Private_UnifiedTileUserPinBroker);
        THROW_IF_FAILED(userPinBrokerStatics->ReleaseUserPinnedShortcutReference(identifier.get())); // 1013
    }
}

wil::com_ptr<utctc::ICuratedTileCollection> __declspec(dllexport) Create_StartTileGridCollection(
    CuratedTileCollectionOptionsInternal options, ABI::Windows::System::IUser* user)
{
    wil::com_ptr<utctc::ICuratedTileCollection> instance;
    THROW_IF_FAILED(Microsoft::WRL::MakeAndInitialize<StartTileCollection>(&instance, options, user)); // 1020
    return instance;
}

#undef Create_StartTileGridCollectionInitializer
}
