#pragma once

#include "DataStoreCache_MoveMe.h"
#include "InitializationPipelineObjectModel.h"
#include "InternalAsync.h"
#include "../../../inc/ExternalFunctions.h"

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
class TransformerHelpers
{
public:
    static std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedRoot> EnsureTransformerRoot(
        const WCHAR* collectionName, const std::shared_ptr<CollectionContext>& context)
    {
        using namespace DataStoreCache::CuratedTileCollectionTransformer;

        wil::com_ptr<ICuratedTileCollectionTransformer> transformer;
        CreateCuratedTileCollectionTransformer(nullptr, context->_user.get(), &transformer);

        std::shared_ptr<CuratedRoot> transformerRoot;
        {
            transformerRoot = transformer->HasLayoutRoot(collectionName)
               ? transformer->GetCuratedTileCollectionRoot(collectionName, CuratedTileCollectionTransformerOptions_0)
               : transformer->CreateNewLayoutRoot(collectionName, CuratedTileCollectionTransformerOptions_0);
        }
        return transformerRoot;
    }

    static bool HasTransformerRoot(const WCHAR* collectionName, const std::shared_ptr<CollectionContext>& context)
    {
        using namespace DataStoreCache::CuratedTileCollectionTransformer;

        wil::com_ptr<ICuratedTileCollectionTransformer> transformer;
        CreateCuratedTileCollectionTransformer(nullptr, context->_user.get(), &transformer);

        return transformer->HasLayoutRoot(collectionName);
    }

    static std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedRoot> GetTransformerRoot(
        const WCHAR* collectionName, const std::shared_ptr<CollectionContext>& context)
    {
        using namespace DataStoreCache::CuratedTileCollectionTransformer;

        wil::com_ptr<ICuratedTileCollectionTransformer> transformer;
        CreateCuratedTileCollectionTransformer(nullptr, context->_user.get(), &transformer);

        return transformer->GetCuratedTileCollectionRoot(collectionName, CuratedTileCollectionTransformerOptions_0);
    }

    static HRESULT BatchAction(
        const WCHAR* collectionName, const std::shared_ptr<CollectionContext>& context,
        const wistd::function<HRESULT ()>& action, ABI::Windows::Foundation::IAsyncAction** result)
    {
        using namespace DataStoreCache::CuratedTileCollectionTransformer;
        using namespace Windows::Internal;

        *result = nullptr;

        std::shared_ptr<CuratedRoot> transformerRoot = EnsureTransformerRoot(collectionName, context);

        std::shared_ptr<ICuratedCollectionBatchCookieImpl> cookie = transformerRoot->BeginBatchUpdate();
        RETURN_IF_FAILED(action()); // 51
        Concurrency::task<void> task = transformerRoot->EndBatchUpdate(cookie);

        RETURN_IF_FAILED(MakeAsyncAction<Microsoft::WRL::DisableCausality>(
            ComTaskPoolHandler(TaskApartment::Calling, TaskOptions::None), result, BaseTrust,
            [task](CNoResult&) -> HRESULT
            {
                (void)WaitTask(task);
                return S_OK;
            })
        ); // 62

        return S_OK;
    }
};
}
