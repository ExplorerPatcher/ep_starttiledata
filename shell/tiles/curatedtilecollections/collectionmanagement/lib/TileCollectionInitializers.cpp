#include "pch.h"

#include "TileCollectionInitializers.h"

namespace wrl = Microsoft::WRL;

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{

struct CollectionInitializationPolicy;

std::shared_ptr<CollectionInitializationPolicy> GetAlwaysInitPolicy(
    const std::shared_ptr<CollectionContext>&, const std::vector<wil::com_ptr<IInitialCollectionPostProcessor>>&);

BaseTileCollectionInitializer::BaseTileCollectionInitializer(ABI::Windows::System::IUser* user)
    : _user(user)
{
}

void BaseTileCollectionInitializer::InitializeCollection(const std::wstring& a2, bool* a3)
{
    _context = std::make_shared<CollectionContext>(_user.get()); // @MOD
    THROW_IF_FAILED(wrl::MakeAndInitialize<GenericCollectionWriter>(_writer.get(), a2.c_str(), &_context)); // 31

    wil::com_ptr<ICollectionInitializationPipeline> pipeline;
    THROW_IF_FAILED(Create_AssignedAccessCollectionInitializationPipeline({}, _writer.get(),&pipeline)); // 35
    THROW_IF_FAILED(pipeline->EnsureCollectionInitialized(a3, nullptr)); // 36
}

void BaseTileCollectionInitializer::CheckForUpdate(const std::wstring& a2, bool* outResult)
{
    *outResult = false;
}

void BaseTileCollectionInitializer::ReinitializeCollection(
    const std::wstring& a2, ABI::Windows::Foundation::IAsyncAction** outAction)
{
    std::shared_ptr<CollectionContext> context = std::make_shared<CollectionContext>(_user.get());
    THROW_IF_FAILED(wrl::MakeAndInitialize<GenericCollectionWriter>(_writer.get(), a2.c_str(), &context)); // 48

    wil::com_ptr<ICollectionInitializationPipeline> pipeline;
    THROW_IF_FAILED(Create_AssignedAccessCollectionInitializationPipeline(
            { GetAlwaysInitPolicy(context, {}) },
            _writer.get(),
            &pipeline)
    ); // 55

    THROW_IF_FAILED(pipeline->ResetCollection(outAction)); // 57
}
}
