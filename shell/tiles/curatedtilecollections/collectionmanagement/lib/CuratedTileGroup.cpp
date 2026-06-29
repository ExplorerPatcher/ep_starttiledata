#include "pch.h"

#include "CuratedTileGroup.h"

#include "CuratedTile.h"
#include "CuratedTileCollection.h"
#include "windowscollections.h"

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
HRESULT CuratedTileGroup::RuntimeClassInitialize(
    std::shared_ptr<dsct::CuratedGroup> transformerGroup, utctc::ICuratedTileCollection* collection)
{
    _collectionWeak = wil::com_weak_query(collection);
    _transformerGroup = transformerGroup;

    PopulateFromTransformerData();

    return S_OK;
}

HRESULT CuratedTileGroup::get_UniqueId(GUID* value)
{
    try
    {
        *value = _transformerGroup->GetLayoutId();
        return S_OK;
    } CATCH_RETURN() // 34
}

HRESULT CuratedTileGroup::get_DisplayName(HSTRING* value)
{
    try
    {
        Microsoft::WRL::Wrappers::HString displayName;
        displayName.Set(_transformerGroup->GetDisplayName()->c_str());
        *value = displayName.Detach();
        return S_OK;
    } CATCH_RETURN() // 43
}

HRESULT CuratedTileGroup::put_DisplayName(HSTRING value)
{
    try
    {
        _transformerGroup->SetDisplayName(WindowsGetStringRawBuffer(value, nullptr));
        return S_OK;
    } CATCH_RETURN() // 50
}

HRESULT CuratedTileGroup::get_Attributes(utctc::GroupAttributes* value)
{
    if (_transformerGroup->GetIsLockedForCustomization())
    {
        *value |= utctc::GroupAttributes_LockedFromEditing;
    }

    return S_OK;
}

HRESULT CuratedTileGroup::put_Attributes(utctc::GroupAttributes value)
{
    _transformerGroup->SetIsLockedForCustomization((value & utctc::GroupAttributes_LockedFromEditing) != 0);
    return S_OK;
}

HRESULT CuratedTileGroup::get_Location(wf::Point* value)
{
    try
    {
        *value = { (float)_transformerGroup->GetLocation().x, (float)_transformerGroup->GetLocation().y };
        return S_OK;
    } CATCH_RETURN() // 77
}

HRESULT CuratedTileGroup::put_Location(wf::Point value)
{
    try
    {
        _transformerGroup->SetLocation({ (int)value.X, (int)value.Y });
        return S_OK;
    } CATCH_RETURN() // 84
}

HRESULT CuratedTileGroup::get_Size(wf::Size* value)
{
    try
    {
        SIZE size = _transformerGroup->GetSize();
        *value = { (float)size.cx, (float)size.cy };
        return S_OK;
    } CATCH_RETURN() // 92
}

HRESULT CuratedTileGroup::put_Size(wf::Size value)
{
    try
    {
        _transformerGroup->SetSize({ (int)value.Width, (int)value.Height });
        return S_OK;
    } CATCH_RETURN() // 99
}

HRESULT CuratedTileGroup::get_LockedForCustomization(boolean* value)
{
    *value = _transformerGroup->GetIsLockedForCustomization();
    return S_OK;
}

HRESULT CuratedTileGroup::put_LockedForCustomization(boolean value)
{
    _transformerGroup->SetIsLockedForCustomization(value != 0);
    return S_OK;
}

HRESULT CuratedTileGroup::GetGroups(wfc::IMapView<GUID, ICuratedTileGroup*>** result)
{
    *result = nullptr;

    wil::com_ptr<Windows::Foundation::Collections::Internal::HashMap<GUID, ICuratedTileGroup*>> groups;
    Windows::Foundation::Collections::Internal::HashMap<GUID, ICuratedTileGroup*>::Make(&groups);

    for (const std::pair<const GUID, wil::com_ptr<ICuratedTileGroup>>& pair : _groups)
    {
        boolean bReplaced;
        RETURN_IF_FAILED(groups->Insert(pair.first, pair.second.get(), &bReplaced)); // 126
    }

    RETURN_HR(groups->GetView(result)); // 128
}

HRESULT CuratedTileGroup::GetTiles(wfc::IMapView<GUID, utctc::ICuratedTile*>** result)
{
    *result = nullptr;

    Microsoft::WRL::ComPtr<Windows::Foundation::Collections::Internal::HashMap<GUID, utctc::ICuratedTile*>> tiles;
    Windows::Foundation::Collections::Internal::HashMap<GUID, utctc::ICuratedTile*>::Make(&tiles);

    for (const std::pair<const GUID, wil::com_ptr<utctc::ICuratedTile>>& pair : _tiles)
    {
        boolean bReplaced;
        RETURN_IF_FAILED(tiles->Insert(pair.first, pair.second.get(), &bReplaced)); // 142
    }

    RETURN_HR(tiles->GetView(result)); // 144
}

HRESULT CuratedTileGroup::CreateNewGroup(ICuratedTileGroup** result)
{
    *result = nullptr;

    try
    {
        std::shared_ptr<dsct::CuratedGroup> transformerGroup = _transformerGroup->CreateGroup();

        wil::com_ptr<ICuratedTileGroup> group;
        THROW_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CuratedTileGroup>(&group, transformerGroup, _collectionWeak.query<utctc::ICuratedTileCollection>().get())); // 154

        _groups[transformerGroup->GetLayoutId()] = group.get();
        group.copy_to(result);

        return S_OK;
    } CATCH_RETURN() // 158
}

HRESULT CuratedTileGroup::GetGroup(GUID groupId, ICuratedTileGroup** result)
{
    *result = nullptr;

    auto it = _groups.find(groupId);
    try
    {
        if (it != _groups.end())
        {
            it->second.copy_to(result);
        }
        RETURN_HR_IF(E_INVALIDARG, *result == nullptr); // 170
        return S_OK;
    } CATCH_RETURN() // 171
}

HRESULT CuratedTileGroup::DeleteGroup(GUID groupId)
{
    return UnparentGroup(groupId, UnparentItemOptions_Delete);
}

HRESULT CuratedTileGroup::RemoveGroup(GUID groupId)
{
    return UnparentGroup(groupId, UnparentItemOptions_Remove);
}

HRESULT CuratedTileGroup::AddTile(ut::IUnifiedTileIdentifier* identifier, utctc::ICuratedTile** result)
{
    *result = nullptr;

    try
    {
        std::shared_ptr<dsct::CuratedTile> transformerTile = _transformerGroup->CreateTile(identifier);

        wil::com_ptr<utctc::ICuratedTile> tile;
        RETURN_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CuratedTile>(&tile, transformerTile)); // 212
        _tiles[transformerTile->GetLayoutId()] = tile.get();

        LOG_IF_FAILED(_collectionWeak.query<ICuratedTileCollectionInternal>()->OnTileAddedWithinCollection(identifier)); // 215

        tile.copy_to(result);

        return S_OK;
    } CATCH_RETURN() // 218
}

HRESULT CuratedTileGroup::AddTileWithId(ut::IUnifiedTileIdentifier* identifier, GUID tileId, utctc::ICuratedTile** result)
{
    *result = nullptr;

    try
    {
        std::shared_ptr<dsct::CuratedTile> transformerTile = _transformerGroup->CreateTile(tileId, identifier);

        wil::com_ptr<utctc::ICuratedTile> tile;
        RETURN_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CuratedTile>(&tile, transformerTile)); // 227
        _tiles[transformerTile->GetLayoutId()] = tile.get();

        LOG_IF_FAILED(_collectionWeak.query<ICuratedTileCollectionInternal>()->OnTileAddedWithinCollection(identifier)); // 230

        tile.copy_to(result);

        return S_OK;
    } CATCH_RETURN() // 233
}

HRESULT CuratedTileGroup::GetTile(GUID tileId, utctc::ICuratedTile** result)
{
    *result = nullptr;

    auto it = _tiles.find(tileId);
    try
    {
        if (it != _tiles.end())
        {
            it->second.copy_to(result);
        }
        return *result != nullptr ? S_OK : E_INVALIDARG;
    } CATCH_RETURN() // 248
}

HRESULT CuratedTileGroup::DeleteTile(GUID tileId)
{
    return UnparentTile(tileId, UnparentItemOptions_Delete);
}

HRESULT CuratedTileGroup::RemoveTile(GUID tileId)
{
    return UnparentTile(tileId, UnparentItemOptions_Remove);
}

HRESULT CuratedTileGroup::GetCustomProperty(HSTRING key, HSTRING* result)
{
    try
    {
        Microsoft::WRL::Wrappers::HString value;
        value.Set(_transformerGroup->GetCustomProperty(WindowsGetStringRawBuffer(key, nullptr)).c_str());
        *result = value.Detach();
        return S_OK;
    } CATCH_RETURN() // 296
}

HRESULT CuratedTileGroup::HasCustomProperty(HSTRING key, boolean* result)
{
    try
    {
        *result = _transformerGroup->HasCustomProperty(WindowsGetStringRawBuffer(key, nullptr));
        return S_OK;
    } CATCH_RETURN() // 303
}

HRESULT CuratedTileGroup::RemoveCustomProperty(HSTRING key)
{
    try
    {
        _transformerGroup->RemoveCustomProperty(WindowsGetStringRawBuffer(key, nullptr));
        return S_OK;
    } CATCH_RETURN() // 310
}

HRESULT CuratedTileGroup::SetCustomProperty(HSTRING key, HSTRING value)
{
    try
    {
        _transformerGroup->SetCustomProperty(WindowsGetStringRawBuffer(key, nullptr), WindowsGetStringRawBuffer(value, nullptr));
        return S_OK;
    } CATCH_RETURN() // 319
}

HRESULT CuratedTileGroup::AddTile(ICuratedTilePrivate* tile)
{
    try
    {
        _transformerGroup->AddTile(tile->GetTransformerData());
        _tiles[tile->GetTransformerData()->GetLayoutId()] = wil::com_query<utctc::ICuratedTile>(tile);
        return S_OK;
    } CATCH_RETURN() // 327
}

HRESULT CuratedTileGroup::AddGroup(ICuratedTileGroupPrivate* group)
{
    try
    {
        _transformerGroup->AddGroup(group->GetTransformerData());
        _groups[group->GetTransformerData()->GetLayoutId()] = wil::com_query<utctc::ICuratedTileGroup>(group);
        return S_OK;
    } CATCH_RETURN() // 327
}

std::shared_ptr<dsct::CuratedGroup> CuratedTileGroup::GetTransformerData()
{
    return _transformerGroup;
}

void CuratedTileGroup::PopulateFromTransformerData()
{
    for (const std::pair<const GUID, std::shared_ptr<dsct::CuratedTile>>& pair : _transformerGroup->GetTiles())
    {
        wil::com_ptr<utctc::ICuratedTile> tile;
        THROW_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CuratedTile>(&tile, pair.second)); // 347
        _tiles[pair.first] = tile.get();
    }

    for (const std::pair<const GUID, std::shared_ptr<dsct::CuratedGroup>>& pair : _transformerGroup->GetGroups())
    {
        wil::com_ptr<ICuratedTileGroup> group;
        THROW_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CuratedTileGroup>(
            &group, pair.second, _collectionWeak.query<utctc::ICuratedTileCollection>().get())); // 354
        _groups[pair.first] = group.get();
    }
}

HRESULT CuratedTileGroup::UnparentTile(const GUID& tileId, UnparentItemOptions options)
{
    auto it = _tiles.find(tileId);
    try
    {
        if (it != _tiles.end())
        {
            wil::com_ptr<ut::IUnifiedTileIdentifier> identifier;
            THROW_IF_FAILED(it->second->get_Identifier(&identifier)); // 269

            _tiles.erase(tileId);

            if (options == UnparentItemOptions_Remove)
            {
                _transformerGroup->RemoveTile(tileId);
            }
            else
            {
                _transformerGroup->DeleteTile(tileId);
            }

            LOG_IF_FAILED(_collectionWeak.query<ICuratedTileCollectionInternal>()->OnTileRemovedWithinCollection(identifier.get())); // 281

            return S_OK;
        }

        RETURN_HR(E_INVALIDARG); // 286
    } CATCH_RETURN() // 287
}

HRESULT CuratedTileGroup::UnparentGroup(const GUID& groupId, UnparentItemOptions options)
{
    auto it = _groups.find(groupId);
    if (it != _groups.end())
    {
        _groups.erase(groupId);
        try
        {
            if (options == UnparentItemOptions_Remove)
            {
                _transformerGroup->RemoveGroup(groupId);
            }
            else
            {
                _transformerGroup->DeleteGroup(groupId);
            }

            return S_OK;
        } CATCH_RETURN() // 203
    }
    RETURN_HR(E_INVALIDARG); // 202
}
}
