#include "pch.h"

#include "ItemLayoutResolver.h"

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

// 22621 verified.
HRESULT CItemLayoutResolver::_StartBatchingItemBoundsChangeUpdates()
{
    RETURN_HR_IF(E_FAIL, _isBatchingItemBoundsChangeUpdates); // 704
    _isBatchingItemBoundsChangeUpdates = true;
    _batchedUpdates.RemoveAll();
    return S_OK;
}

