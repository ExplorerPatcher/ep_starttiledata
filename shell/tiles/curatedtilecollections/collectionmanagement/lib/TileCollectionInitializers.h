#pragma once

#include <memory>

#include <Windows.Foundation.h>
#include <Windows.System.h>

#include <wil/com.h>

#include "../../collectioninitialization/lib/CDSStartCollectionWriter.h"
#include "../../collectioninitialization/lib/InitializationPipelineObjectModel.h"

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{

interface ILayoutModificationXMLParser;

MIDL_INTERFACE("88734a32-4faf-4a40-a9d2-21c2341f2b36")
ICollectionInitializationPipeline : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE EnsureCollectionInitialized(bool*, ABI::Windows::Foundation::IAsyncAction**) = 0;
    virtual HRESULT STDMETHODCALLTYPE CheckForUpdate(bool*) = 0;
    virtual HRESULT STDMETHODCALLTYPE ResetCollection(ABI::Windows::Foundation::IAsyncAction**) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetAllowLayoutOptions(int) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetModificationParser(ILayoutModificationXMLParser**) = 0;
};

MIDL_INTERFACE("7a554b29-b633-4ebf-ad2a-919da582c85e")
IInitialCollectionPostProcessor : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE PostProcessLayout(
        const SelectionData&, std::shared_ptr<Internal::LayoutRoot>&) = 0;
    virtual const WCHAR* STDMETHODCALLTYPE GetPostProcessorLoggingName() = 0;
};

class BaseTileCollectionInitializer
{
public:
    BaseTileCollectionInitializer(ABI::Windows::System::IUser* user);

    virtual void InitializeCollection(const std::wstring& a2, bool* a3);
    virtual void CheckForUpdate(const std::wstring& a2, bool* outResult);
    virtual void ReinitializeCollection(const std::wstring& a2, ABI::Windows::Foundation::IAsyncAction** outAction);

private:
    wil::com_ptr<ABI::Windows::System::IUser> _user;
    wil::com_ptr<ICollectionWriter> _writer;
    std::shared_ptr<CollectionContext> _context;
};
}
