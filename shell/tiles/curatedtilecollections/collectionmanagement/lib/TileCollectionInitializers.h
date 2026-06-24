#pragma once

#include <memory>

#include <Windows.Foundation.h>
#include <Windows.System.h>

#include <wil/com.h>

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
class BaseTileCollectionInitializer
{
public:
    BaseTileCollectionInitializer(ABI::Windows::System::IUser* user);

    virtual void InitializeCollection(const std::wstring& a2, bool* a3);
    virtual void CheckForUpdate(const std::wstring& a2, bool* a3);
    virtual void ReinitializeCollection(const std::wstring& a2, ABI::Windows::Foundation::IAsyncAction** outAction);

private:
    wil::com_ptr<ABI::Windows::System::IUser> _user;
    wil::com_ptr<ABI::Windows::System::IUser /*???*/> _unk1;
    std::shared_ptr<struct CollectionContext /*???*/> _unk2;
};
}
