#include "pch.h"

#include "GroupsLayoutCollapseHandler.h"

using namespace Microsoft::WRL;

HRESULT CGroupsLayoutCollapseHandler::SetCellArray(ICellArrayManager* cellArrayManager)
{
    m_cellArrayManager = cellArrayManager;
    return S_OK;
}
