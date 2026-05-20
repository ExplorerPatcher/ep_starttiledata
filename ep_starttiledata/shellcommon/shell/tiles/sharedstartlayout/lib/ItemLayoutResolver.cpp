#include "pch.h"

#include "ItemLayoutResolver.h"

#include <wil/common.h>
#include <wil/resource.h>

using namespace Microsoft::WRL;

// @Note
// wil::ScopeExit is the precursor to wil::scope_exit
// e.g:
// auto cleanupOnFailure = wil::ScopeExit([this] { LOG_IF_FAILED(_StopBatchingItemBoundsChangeUpdatesAndNotify()); });
// is the same as
// auto cleanupOnFailure = wil::scope_exit([this] { LOG_IF_FAILED(_StopBatchingItemBoundsChangeUpdatesAndNotify()); });
// in modern code.
// Example taken from CItemLayoutResolver::ResizeItemUncommitted()

class ItemLayoutResolverProxy
    : public RuntimeClass<RuntimeClassFlags<ClassicCom>
        , ICellArrayManagerCallback
        , IGroupBoundsChangeNotification
    >
{
public:
    ItemLayoutResolverProxy();

    HRESULT RuntimeClassInitialize(CItemLayoutResolver* callback);

    void Reset(CItemLayoutResolver* callback);

    //~ Begin ICellArrayManagerCallback Interface
    STDMETHODIMP_(void) ItemBoundsUpdated(REFGUID itemID, const Geometry::CRect& rcItemBoundsCells) override;
    STDMETHODIMP_(void) CellArrayBoundsUpdated(const Geometry::CRect& rcLayoutBounds) override;
    STDMETHODIMP_(void) ItemRemovedPending(REFGUID itemID) override;
    STDMETHODIMP_(void) ItemRemoved(REFGUID itemID) override;
    //~ End ICellArrayManagerCallback Interface

    //~ Begin IGroupBoundsChangeNotification Interface
    STDMETHODIMP_(void) NewItemAddedBegin() override;
    STDMETHODIMP_(void) NewItemAddedEnd() override;
    STDMETHODIMP_(void) OnItemsMigrated(IItemLayoutResolver* pDestinationLayoutResolver, REFGUID groupID) override;
    STDMETHODIMP GroupBoundsChanged(REFGUID groupID) override;
    STDMETHODIMP_(void) GroupEmptiedPending(REFGUID groupID) override;
    STDMETHODIMP_(void) GroupEmptied() override;
    //~ End IGroupBoundsChangeNotification Interface

private:
    CItemLayoutResolver* m_callbackNoRef;
};

ItemLayoutResolverProxy::ItemLayoutResolverProxy()
    : m_callbackNoRef(nullptr)
{
}

HRESULT ItemLayoutResolverProxy::RuntimeClassInitialize(CItemLayoutResolver* callback)
{
    m_callbackNoRef = callback;
    return S_OK;
}

void ItemLayoutResolverProxy::Reset(CItemLayoutResolver* callback)
{
    m_callbackNoRef = callback;
}

void ItemLayoutResolverProxy::ItemBoundsUpdated(REFGUID itemID, const Geometry::CRect& rcItemBoundsCells)
{
    if (m_callbackNoRef)
        m_callbackNoRef->ItemBoundsUpdated(itemID, rcItemBoundsCells);
}

void ItemLayoutResolverProxy::CellArrayBoundsUpdated(const Geometry::CRect& rcLayoutBounds)
{
    if (m_callbackNoRef)
        m_callbackNoRef->CellArrayBoundsUpdated(rcLayoutBounds);
}

void ItemLayoutResolverProxy::ItemRemovedPending(REFGUID itemID)
{
    if (m_callbackNoRef)
        m_callbackNoRef->ItemRemovedPending(itemID);
}

void ItemLayoutResolverProxy::ItemRemoved(REFGUID itemID)
{
    if (m_callbackNoRef)
        m_callbackNoRef->ItemRemoved(itemID);
}

void ItemLayoutResolverProxy::NewItemAddedBegin()
{
    if (m_callbackNoRef)
        m_callbackNoRef->NewItemAddedBegin();
}

void ItemLayoutResolverProxy::NewItemAddedEnd()
{
    if (m_callbackNoRef)
        m_callbackNoRef->NewItemAddedEnd();
}

void ItemLayoutResolverProxy::OnItemsMigrated(IItemLayoutResolver* pDestinationLayoutResolver, REFGUID groupID)
{
    if (m_callbackNoRef)
        m_callbackNoRef->OnItemsMigrated(pDestinationLayoutResolver, groupID);
}

HRESULT ItemLayoutResolverProxy::GroupBoundsChanged(REFGUID groupID)
{
    if (m_callbackNoRef)
        return m_callbackNoRef->GroupBoundsChanged(groupID);
    else
        return S_OK;
}

void ItemLayoutResolverProxy::GroupEmptiedPending(REFGUID groupID)
{
    if (m_callbackNoRef)
        m_callbackNoRef->GroupEmptiedPending(groupID);
}

void ItemLayoutResolverProxy::GroupEmptied()
{
    if (m_callbackNoRef)
        m_callbackNoRef->GroupEmptied();
}

CItemLayoutResolver::CItemLayoutResolver()
    : _options(LRO_DISPLACE_INTO_NEGATIVE_SPACE)
    , m_isCollapsed(false)
    , m_isUnk1(true)
{
}

CItemLayoutResolver::~CItemLayoutResolver()
{
    if (m_itemLayoutResolverProxy != nullptr)
    {
        if (_spCellArrayManager != nullptr)
        {
            _spCellArrayManager->UnregisterCallback(m_itemLayoutResolverProxy.Get());
        }
        m_itemLayoutResolverProxy->Reset(nullptr);
    }
}

HRESULT CItemLayoutResolver::RuntimeClassInitialize()
{
    RETURN_IF_FAILED(MakeAndInitialize<CCellArrayManager>(&_spCellArrayManager)); // 36

    RETURN_IF_FAILED(MakeAndInitialize<ItemLayoutResolverProxy>(&m_itemLayoutResolverProxy, this)); // 38
    RETURN_IF_FAILED(_spCellArrayManager->RegisterCallback(m_itemLayoutResolverProxy.Get())); // 39

    _isBatchingItemBoundsChangeUpdates = false;
    return S_OK;
}

HRESULT CItemLayoutResolver::RegisterCallback(IItemLayoutResolverCallback* callback)
{
    return _htCallbacks.AddItem(callback, callback);
}

HRESULT CItemLayoutResolver::UnregisterCallback(IItemLayoutResolverCallback* callback)
{
    return _htCallbacks.DeleteItem(callback);
}

HRESULT CItemLayoutResolver::RegisterInternalCallback(IItemLayoutResolverInternalCallback* callback)
{
    return _htInternalCallbacks.AddItem(callback, callback);
}

HRESULT CItemLayoutResolver::UnregisterInternalCallback(IItemLayoutResolverInternalCallback* callback)
{
    return _htInternalCallbacks.DeleteItem(callback);
}

HRESULT CItemLayoutResolver::AddNewItem(REFGUID itemID, const SIZE sizeItemCells)
{
    _NotifyNewItemAddedBegin();

    Geometry::CRect rcItemCellBounds;
    HRESULT hr = _GetCellAssignor()->CalculateLocationForNewItem(sizeItemCells, &rcItemCellBounds);
    if (SUCCEEDED(hr))
    {
        hr = _spCellArrayManager->SetItem(itemID, rcItemCellBounds, SIO_SEND_BOUNDS_UPDATE);
    }

    _NotifyNewItemAddedEnd();
    return hr;
}

HRESULT CItemLayoutResolver::AddItem(REFGUID itemID, const RECT rcItemBoundsCells)
{
    return _spCellArrayManager->SetItem(itemID, rcItemBoundsCells, SIO_NONE);
}

HRESULT CItemLayoutResolver::InsertItemUncommitted(REFGUID itemID, const RECT rcDestination)
{
    auto cleanupOnFailure = wil::scope_exit([this]
    {
        LOG_IF_FAILED(_StopBatchingItemBoundsChangeUpdatesAndNotify()); // 251
    });

    RETURN_IF_FAILED(_StartBatchingItemBoundsChangeUpdates()); // 254
    RETURN_IF_FAILED(_spCellArrayManager->FixCoordinatesToBeNonNegative()); // 255
    RETURN_IF_FAILED(_ModifyItemUncommittedInternal(itemID, rcDestination, ModificationOperation::Insert)); // 256
    RETURN_IF_FAILED(_StopBatchingItemBoundsChangeUpdatesAndNotify()); // 257

    cleanupOnFailure.release();
    return S_OK;
}

BOOL CItemLayoutResolver::IsCollapsed()
{
    return m_isCollapsed != 0;
}

BOOL CItemLayoutResolver::IsEmpty()
{
    return _spCellArrayManager->IsEmpty();
}

// 22621 verified.
HRESULT CItemLayoutResolver::RepairLayoutUncommitted()
{
    _StartBatchingItemBoundsChangeUpdates();
    const auto cleanupOnFailure = wil::scope_exit([this] // @MOD Use modern wil::scope_exit
    {
        LOG_IF_FAILED(_StopBatchingItemBoundsChangeUpdatesAndNotify()); // 355
    });

    RETURN_IF_FAILED(_RepairLayout()); // 358
    _StopBatchingItemBoundsChangeUpdatesAndNotify();
    return S_OK;
}

void CItemLayoutResolver::_NotifyNewItemAddedBegin()
{
    (void)_htInternalCallbacks.Enum([](const ComPtr<IItemLayoutResolverInternalCallback>& callback)
    {
        callback->NewItemAddedBegin();
    });
}

void CItemLayoutResolver::_NotifyNewItemAddedEnd()
{
    (void)_htInternalCallbacks.Enum([](const ComPtr<IItemLayoutResolverInternalCallback>& callback)
    {
        callback->NewItemAddedEnd();
    });
}

// 22621 verified.
HRESULT CItemLayoutResolver::_StartBatchingItemBoundsChangeUpdates()
{
    RETURN_HR_IF(E_FAIL, _isBatchingItemBoundsChangeUpdates); // 704
    _isBatchingItemBoundsChangeUpdates = true;
    _batchedUpdates.RemoveAll();
    return S_OK;
}

