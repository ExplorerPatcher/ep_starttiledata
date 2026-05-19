#include "pch.h"

#include "ItemLayoutResolver.h"

#include <wil/common.h>
#include <wil/resource.h>

// @Note
// wil::ScopeExit is the precursor to wil::scope_exit
// e.g:
// auto cleanupOnFailure = wil::ScopeExit([this] { LOG_IF_FAILED(_StopBatchingItemBoundsChangeUpdatesAndNotify()); });
// is the same as
// auto cleanupOnFailure = wil::scope_exit([this] { LOG_IF_FAILED(_StopBatchingItemBoundsChangeUpdatesAndNotify()); });
// in modern code.
// Example taken from CItemLayoutResolver::ResizeItemUncommitted()

CItemLayoutResolver::CItemLayoutResolver()
    : _options(LRO_DISPLACE_INTO_NEGATIVE_SPACE)
    , m_isCollapsed(false)
{
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

// 22621 verified.
HRESULT CItemLayoutResolver::_StartBatchingItemBoundsChangeUpdates()
{
    RETURN_HR_IF(E_FAIL, _isBatchingItemBoundsChangeUpdates); // 704
    _isBatchingItemBoundsChangeUpdates = true;
    _batchedUpdates.RemoveAll();
    return S_OK;
}

