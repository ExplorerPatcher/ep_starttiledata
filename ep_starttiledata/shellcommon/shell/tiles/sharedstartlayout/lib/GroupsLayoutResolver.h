#pragma once

#include "ItemLayoutResolver.h"

class CGroupsLayoutResolver : public CItemLayoutResolver
{
public:
    CGroupsLayoutResolver();
    ~CGroupsLayoutResolver() override;

    HRESULT RuntimeClassInitialize() override;

    //~ Begin IItemLayoutResolver Interface
    STDMETHODIMP ResizeItemUncommitted(REFGUID itemID, const SIZE sizeItemCells) override;
    STDMETHODIMP AddNewContainer(REFGUID containerID, IItemLayoutResolver* resolver) override;
    STDMETHODIMP AddContainer(REFGUID containerID, IItemLayoutResolver* resolver, const POINT destination) override;
    STDMETHODIMP AddSizedContainer(REFGUID containerID, IItemLayoutResolver* pResolver, const RECT destination) override;
    STDMETHODIMP InsertContainerUncommitted(REFGUID containerID, IItemLayoutResolver* resolver, const POINT destination) override;
    STDMETHODIMP GetLayoutResolverForContainer(REFGUID containerID, IItemLayoutResolver** ppResolver) override;
    STDMETHODIMP RemoveItemUncommitted(REFGUID itemID) override;
    STDMETHODIMP SetContainerMargins(const RECT containerMargins) override;
    STDMETHODIMP GetContainerMargins(RECT* containerMargins) override;
    STDMETHODIMP MigrateItems(IItemLayoutResolver* pSourceLayout, const LayoutMigrationOptions migrationOptions) override;
    STDMETHODIMP GetContainerSizeWithMargins(REFGUID containerID, SIZE* size) override;
    STDMETHODIMP CommitChanges() override;
    //~ End IItemLayoutResolver Interface

    //~ Begin IGroupBoundsChangeNotification Interface
    STDMETHODIMP_(void) NewItemAddedBegin() override;
    STDMETHODIMP_(void) NewItemAddedEnd() override;
    STDMETHODIMP_(void) OnItemsMigrated(IItemLayoutResolver* pDestinationLayout) override;
    STDMETHODIMP GroupBoundsChanged(REFGUID groupID) override;
    STDMETHODIMP_(void) GroupEmptiedPending(REFGUID groupID) override;
    STDMETHODIMP_(void) GroupEmptied() override;
    //~ End IGroupBoundsChangeNotification Interface

    //~ Begin ICellArrayManagerCallback Interface
    STDMETHODIMP_(void) ItemRemoved(REFGUID itemID) override;
    //~ End ICellArrayManagerCallback Interface

    bool IsGroupPendingRemoval();

protected:
    //~ Begin CItemLayoutResolver Interface
    HRESULT _FindTargetDestinationForNewSize(REFGUID itemId, const SIZE& sizeItemCells, Geometry::CRect* rcItemBounds) override;
    IItemCellAssignor* _GetCellAssignor() override;
    HRESULT _PrepareLayoutBeforeOperation(const Geometry::CRect& rcSourceCells, const Geometry::CRect& rcTargetCells) override;
    HRESULT _CleanupLayoutAfterOperation(const Geometry::CRect& rcSourceCells, const Geometry::CRect& rcTargetCells) override;
    HRESULT _RepairLayout() override;
    //~ End CItemLayoutResolver Interface

private:
    struct GroupResolverInternal
    {
        Microsoft::WRL::ComPtr<IItemLayoutResolver> resolver;
        Microsoft::WRL::ComPtr<class CGroupsLayoutResolverCallbackListener> resolverCallback;
    };

    HRESULT _RegisterForCallbacksWithSubresolver(IItemLayoutResolver* resolver, REFGUID containerID);
    HRESULT _UnregisterFromCallback(REFGUID groupID);
    HRESULT _GetGroupSizeWithMarginFromLayoutResolver(IItemLayoutResolver* resolver, Geometry::CSize* size);
    HRESULT _PositionGroupAtPoint(IItemLayoutResolver* resolver, const Geometry::CPoint& destination, Geometry::CRect* rect);
    HRESULT _GetResolverBoundsWithMargins(REFGUID itemID, Geometry::CRect* bounds);
    HRESULT _EnsureConsistencyWithResolverBounds(REFGUID groupID);

    static constexpr int c_groupWidth = 69696969; // todo real value

    bool m_newItemBeingAdded;
    GUID m_pendingRemovedGroup;
    RECT m_containerMargins;
    Microsoft::WRL::ComPtr<IItemCellAssignor> m_cellAssignor;
    CSimpleHashTable<GUID, GroupResolverInternal> m_groupResolvers;
};
