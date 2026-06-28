#include "pch.h"

#include "CuratedTile.h"

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
HRESULT CuratedTile::RuntimeClassInitialize(const std::shared_ptr<dsct::CuratedTile>& transformerTile)
{
    _transformerTile = transformerTile;
    return S_OK;
}

HRESULT CuratedTile::get_Identifier(ut::IUnifiedTileIdentifier** value)
{
    wil::com_ptr<ut::IUnifiedTileIdentifier> identifier = _transformerTile->GetTileIdentifier();
    *value = identifier.get();
    identifier->AddRef();
    return S_OK;
}

HRESULT CuratedTile::get_UniqueId(GUID* value)
{
    try
    {
        *value = _transformerTile->GetLayoutId();
        return S_OK;
    } CATCH_RETURN() // 27
}

HRESULT CuratedTile::get_Location(wf::Point* value)
{
    try
    {
        *value = { (float)_transformerTile->GetLocation().x, (float)_transformerTile->GetLocation().y };
        return S_OK;
    } CATCH_RETURN() // 35
}

HRESULT CuratedTile::put_Location(wf::Point value)
{
    try
    {
        _transformerTile->SetLocation({ (int)value.X, (int)value.Y });
        return S_OK;
    } CATCH_RETURN() // 42
}

HRESULT CuratedTile::get_Size(wf::Size* value)
{
    try
    {
        *value = { (float)_transformerTile->GetSize().cx, (float)_transformerTile->GetSize().cy };
        return S_OK;
    } CATCH_RETURN() // 50
}

HRESULT CuratedTile::put_Size(wf::Size value)
{
    try
    {
        SIZE size = { (int)value.Width, (int)value.Height };
        _transformerTile->SetSize(size);
        return S_OK;
    } CATCH_RETURN() // 57
}

HRESULT CuratedTile::GetCustomProperty(HSTRING key, HSTRING* result)
{
    Microsoft::WRL::Wrappers::HString value;
    value.Set(_transformerTile->GetCustomProperty(WindowsGetStringRawBuffer(key, nullptr)).c_str());
    *result = value.Detach();
    return S_OK;
}

HRESULT CuratedTile::HasCustomProperty(HSTRING key, boolean* result)
{
    try
    {
        *result = _transformerTile->HasCustomProperty(WindowsGetStringRawBuffer(key, nullptr));
        return S_OK;
    } CATCH_RETURN() // 73
}

HRESULT CuratedTile::RemoveCustomProperty(HSTRING key)
{
    try
    {
        _transformerTile->RemoveCustomProperty(WindowsGetStringRawBuffer(key, nullptr));
        return S_OK;
    } CATCH_RETURN() // 80
}

HRESULT CuratedTile::SetCustomProperty(HSTRING key, HSTRING value)
{
    try
    {
        _transformerTile->SetCustomProperty(
            WindowsGetStringRawBuffer(key, nullptr), WindowsGetStringRawBuffer(value, nullptr));
        return S_OK;
    } CATCH_RETURN() // 89
}

std::shared_ptr<dsct::CuratedTile> CuratedTile::GetTransformerData()
{
    return _transformerTile;
}
}
