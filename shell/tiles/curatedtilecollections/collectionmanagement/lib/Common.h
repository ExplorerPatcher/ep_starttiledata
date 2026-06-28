#pragma once

#include "../../collectioninitialization/lib/DataStoreCache_MoveMe.h"

namespace wf = ABI::Windows::Foundation;
namespace wfc = ABI::Windows::Foundation::Collections;
namespace ut = ABI::WindowsInternal::Shell::UnifiedTile;
namespace utctc = ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections;
namespace utp = ABI::WindowsInternal::Shell::UnifiedTile::Private;

namespace dsct = DataStoreCache::CuratedTileCollectionTransformer;

struct hashGUIDCuratedTileCollections
{
    size_t operator()(const GUID& guid) const noexcept
    {
        const uint64_t* p = reinterpret_cast<const uint64_t*>(&guid);
        return std::hash<uint64_t>{}(p[0]) ^ std::hash<uint64_t>{}(p[1]);
    }
};

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
enum UnparentItemOptions
{
    UnparentItemOptions_Remove,
    UnparentItemOptions_Delete,
};
}
