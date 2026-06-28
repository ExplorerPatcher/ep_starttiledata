#pragma once

#include "CuratedTileCollection.h"

#if !NUKE_SHAREDSTARTLAYOUT
interface IItemLayoutResolver;
interface IStartLayoutFactory;
#endif

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
enum StartCollectionUpdateOptions
{
    StartCollectionUpdateOptions_None = 0,
    StartCollectionUpdateOptions_ResetGroupPolicyLayoutFileTimestamp = 0x1,
};

DEFINE_ENUM_FLAG_OPERATORS(StartCollectionUpdateOptions);

MIDL_INTERFACE("cfc51442-aa2d-418b-9a43-98bdbd743347")
IStartTileCollectionUpdater : IInspectable
{
    virtual HRESULT STDMETHODCALLTYPE CheckForUpdateWithOptions(StartCollectionUpdateOptions) = 0;
};

class StartTileCollection final
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRt>
        , Microsoft::WRL::FtmBase
        , CuratedTileCollectionBase
        , utctc::IStartTileCollection
        , IStartTileCollectionUpdater
    >
{
    InspectableClass(L"CuratedTileCollections.StartTileCollection", BaseTrust);

public:
    StartTileCollection();
    ~StartTileCollection() override;

    HRESULT RuntimeClassInitialize(CuratedTileCollectionOptionsInternal options, ABI::Windows::System::IUser* user);

    //~ Begin utctc::ICuratedTileCollection Interface
    STDMETHODIMP CommitAsync(wf::IAsyncAction** outResult) override;
    STDMETHODIMP ResetToDefault() override;
    STDMETHODIMP ResetToDefaultAsync(wf::IAsyncAction** outResult) override;
    STDMETHODIMP CheckForUpdate() override;
    //~ End utctc::ICuratedTileCollection Interface

    //~ Begin utctc::ICuratedTileCollectionInternal Interface
    STDMETHODIMP ResurrectTile(std::shared_ptr<dsct::CuratedTile> transformerTile, const GUID& tileId) override;
    //~ End utctc::ICuratedTileCollectionInternal Interface

    //~ Begin utctc::IStartTileCollection Interface
    STDMETHODIMP PinToStart(ut::IUnifiedTileIdentifier* identifier, utctc::TilePinSize size) override;
    STDMETHODIMP PinToStartAtLocation(
        ut::IUnifiedTileIdentifier* identifier, utctc::ICuratedTileGroup* group, wf::Point location, wf::Size size) override;
    STDMETHODIMP UnpinFromStart(ut::IUnifiedTileIdentifier* identifier) override;
    STDMETHODIMP ReplaceTinyOrMediumTile(
        ut::IUnifiedTileIdentifier* identifier, ut::IUnifiedTileIdentifier* identifier2) override;
    STDMETHODIMP get_LastGroupId(GUID* outResult) override;
    STDMETHODIMP put_LastGroupId(GUID value) override;
    STDMETHODIMP get_CustomizationRestriction(utctc::StartCollectionCustomizationRestrictionType* outResult) override;
    STDMETHODIMP put_CustomizationRestriction(utctc::StartCollectionCustomizationRestrictionType value) override;
    STDMETHODIMP get_GroupCellWidth(UINT* outResult) override;
    STDMETHODIMP put_GroupCellWidth(UINT value) override;
    STDMETHODIMP get_PreferredColumnCount(UINT* outResult) override;
    STDMETHODIMP put_PreferredColumnCount(UINT value) override;
    STDMETHODIMP get_CurrentColumnCount(UINT* outResult) override;
    STDMETHODIMP put_CurrentColumnCount(UINT value) override;
    //~ End utctc::IStartTileCollection Interface

    //~ Begin IStartTileCollectionUpdater Interface
    STDMETHODIMP CheckForUpdateWithOptions(StartCollectionUpdateOptions options) override;
    //~ End IStartTileCollectionUpdater Interface

private:
#if !NUKE_SHAREDSTARTLAYOUT
    void EnsureLayoutFactory();
#endif
    wil::com_ptr<utctc::ICuratedTileGroup> FindTargetGroup(bool bSingleGroupMode);
    void CreateNewLastGroup(const GUID* groupId, utctc::ICuratedTileGroup** outResult);
#if !NUKE_SHAREDSTARTLAYOUT
    wil::com_ptr<IItemLayoutResolver> CreateLayoutResolverForCurrentCollection();
    wil::com_ptr<IItemLayoutResolver> CreateLayoutResolverForGroup(utctc::ICuratedTileGroup* group);
#endif
    bool CanPinToGroup(utctc::ICuratedTileGroup* group, bool bSingleGroupMode);
    void SetGroupAsLastGroup(utctc::ICuratedTileGroup* group);
    void PinTileToGroup(ut::IUnifiedTileIdentifier* identifier, utctc::ICuratedTileGroup* group, const RECT* bounds);
    void UnpinFromStartInternal(utctc::ICuratedTile* const tile, utctc::ICuratedTileGroup* const group);

#if !NUKE_SHAREDSTARTLAYOUT
    wil::com_ptr<IStartLayoutFactory> _layoutFactory;
    wil::com_ptr<IItemLayoutResolver> _lastGroupLayoutResolver;
#endif
    wil::com_ptr<dsct::ICuratedTileCollectionTransformer> _transformer;
    utctc::TilePinSize _tilePinSize;
    std::vector<std::shared_ptr<dsct::CuratedTile>> _tilesPendingUnpin;
    wil::srwlock _tilesPendingUnpinLock;
};
}
