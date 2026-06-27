#pragma once

#include "DataStoreCache_MoveMe.h"
#include "InitializationPipelineObjectModel.h"

interface IItemLayoutResolver;

namespace CommonStartTelemetry
{
typedef void (__thiscall *LogAllTilesActivity_Dtor_t)(class LogAllTilesActivity* _this);
EXTERN_C __declspec(dllexport) LogAllTilesActivity_Dtor_t g_pfnLogAllTilesActivity_Dtor;

class LogAllTilesActivity
{
public:
    ~LogAllTilesActivity()
    {
        if (g_pfnLogAllTilesActivity_Dtor)
        {
            g_pfnLogAllTilesActivity_Dtor(this);
        }
    }

private:
    char data[344]; ///< NI: 320 bytes; GE: 344 (+24) bytes
};
}

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
class TileInitializationHandlerManager;

MIDL_INTERFACE("8fea4543-90d5-4ebd-9831-64b31f83e85d")
ICollectionWriter : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE WriteCollection(std::shared_ptr<Internal::LayoutRoot>, std::shared_ptr<TileInitializationHandlerManager>, CommonStartTelemetry::LogAllTilesActivity, ABI::Windows::Foundation::IAsyncAction**) = 0;
    virtual bool STDMETHODCALLTYPE CanInitializeTiles() = 0;
    virtual void STDMETHODCALLTYPE ClearCollection(std::vector<std::shared_ptr<Internal::LayoutTile>>&, ABI::Windows::Foundation::IAsyncAction**) = 0;
    virtual bool STDMETHODCALLTYPE IsCollectionEmpty() = 0;
    virtual bool STDMETHODCALLTYPE DoesCollectionExist() = 0;
    virtual std::wstring STDMETHODCALLTYPE GetCollectionName() = 0;
    virtual std::shared_ptr<CollectionContext> STDMETHODCALLTYPE GetContext() = 0;
};

MIDL_INTERFACE("2da4f607-aff6-4324-a7c5-9a0ee03d59c9")
IStartCollectionWriter : IUnknown
{
    virtual void STDMETHODCALLTYPE OverrideGroupColumnCount(UINT) = 0;
};

namespace wf = ABI::Windows::Foundation;

// Created by StartTileGridCollectionInitializer::CreateStartCollectionPipeline()
// -> Hook MakeAndInitialize<CDSStartCollectionWriter> in this function
class CDSStartCollectionWriter
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , ICollectionWriter
        , IStartCollectionWriter
    >
{
public:
    CDSStartCollectionWriter();

    HRESULT RuntimeClassInitialize(
        std::wstring collectionName, bool b, const std::shared_ptr<CollectionContext>& context);

    //~ Begin WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICollectionWriter Interface
    STDMETHODIMP WriteCollection(
        std::shared_ptr<Internal::LayoutRoot> root,
        std::shared_ptr<TileInitializationHandlerManager> tileInitializationHandlerManager,
        CommonStartTelemetry::LogAllTilesActivity activity, wf::IAsyncAction** action) override;
    STDMETHODIMP_(bool) CanInitializeTiles() override;
    STDMETHODIMP_(void) ClearCollection(
        std::vector<std::shared_ptr<Internal::LayoutTile>>& removedTiles, wf::IAsyncAction** action) override;
    STDMETHODIMP_(bool) IsCollectionEmpty() override;
    STDMETHODIMP_(bool) DoesCollectionExist() override;
    STDMETHODIMP_(std::wstring) GetCollectionName() override;
    STDMETHODIMP_(std::shared_ptr<CollectionContext>) GetContext() override;
    //~ End WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICollectionWriter Interface

    //~ Begin WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICollectionWriter Interface
    STDMETHODIMP_(void) OverrideGroupColumnCount(UINT columnCount) override;
    //~ End WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::ICollectionWriter Interface

private:
    void ClearCollection(std::vector<std::shared_ptr<Internal::LayoutTile>>& removedTiles);
    HRESULT WriteCollection(
        std::shared_ptr<Internal::LayoutRoot> root,
        std::shared_ptr<TileInitializationHandlerManager> tileInitializationHandlerManager,
        CommonStartTelemetry::LogAllTilesActivity logAllTilesActivity);
    void WriteStartGroup(
        const std::shared_ptr<Internal::LayoutGroup>& group,
        const std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedRoot>& transformerRoot);
    void WriteStartFolder(
        const std::shared_ptr<Internal::LayoutFolder>& folder,
        const std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup>& transformerGroup
#if !NUKE_SHAREDSTARTLAYOUT
        , IItemLayoutResolver* portraitLayoutResolver
#endif
    );
    void WriteStartTile(
        const std::shared_ptr<Internal::LayoutTile>& tile,
        const std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedGroup>& transformerGroup
#if !NUKE_SHAREDSTARTLAYOUT
        , IItemLayoutResolver* portraitLayoutResolver
#endif
    );

    // char _logAllTilesActivity[320];
    // char _writingStartLayoutToStorageTelemetry[320];
#if !NUKE_SHAREDSTARTLAYOUT
    wil::com_ptr<IItemLayoutResolver> _groupsLayoutResolver;
#endif
    std::shared_ptr<CollectionContext> _context;
    std::shared_ptr<TileInitializationHandlerManager> _tileInitializationHandlerManager;
    bool _b;
    int _columnCount;
    std::wstring _collectionName;
};

}
