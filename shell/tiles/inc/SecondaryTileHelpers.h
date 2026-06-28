#pragma once

#include <Windows.UI.StartScreen.h>
#include <Windows.Internal.UI.StartScreen.h>

#include "usermodelptc.h"
#include "WaitForCompletion.h"
#include "windowscollections.h"

namespace WindowsInternal::Shell::UnifiedTile::CuratedTileCollections::SecondaryTileHelpers
{
inline void FindAndRemoveSecondaryTile(
    ABI::WindowsInternal::Shell::UnifiedTile::IPackagedUnifiedTileIdentifier* packagedIdentifier,
    ABI::Windows::System::IUser* user)
{
    namespace wf = ABI::Windows::Foundation;
    namespace wfc = ABI::Windows::Foundation::Collections;

    using namespace ABI::Windows::UI::StartScreen;
    using namespace ABI::Windows::Internal::UI::StartScreen;

    wil::unique_hstring aumid;
    THROW_IF_FAILED(packagedIdentifier->get_AppUserModelId(&aumid)); // 18

    wil::unique_hstring tileId;
    THROW_IF_FAILED(packagedIdentifier->get_TileId(&tileId)); // 20

    wil::com_ptr<ISecondaryTileStatics> secondaryTileStatics;
    THROW_IF_FAILED(Windows::Foundation::GetActivationFactoryAsUser(
        Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Windows_UI_StartScreen_SecondaryTile).Get(),
        user, &secondaryTileStatics)); // 23

    auto secondaryTileStaticsPriv = secondaryTileStatics.query<ISecondaryTileStaticsPrivate>();

    wil::com_ptr<wf::IAsyncOperation<wfc::IVectorView<SecondaryTile*>*>> allSecondaryTilesOperation;
    THROW_IF_FAILED(secondaryTileStaticsPriv->FindAllForAUMIDAsync(aumid.get(), &allSecondaryTilesOperation)); // 27

    wil::com_ptr<wfc::IVectorView<SecondaryTile*>> allSecondaryTiles;
    THROW_IF_FAILED(WaitForCompletionAndGetResults(allSecondaryTilesOperation.get(), &allSecondaryTiles, COWAIT_DISPATCH_CALLS)); // 29

    UINT numSecondaryTiles = 0;
    THROW_IF_FAILED(allSecondaryTiles->get_Size(&numSecondaryTiles)); // 32
    for (UINT i = 0; i < numSecondaryTiles; ++i)
    {
        wil::com_ptr<ISecondaryTile> secondaryTile;
        THROW_IF_FAILED(allSecondaryTiles->GetAt(i, &secondaryTile)); // 36

        wil::unique_hstring otherTileId;
        THROW_IF_FAILED(secondaryTile->get_TileId(&otherTileId)); // 39

        int compare;
        THROW_IF_FAILED(WindowsCompareStringOrdinal(tileId.get(), otherTileId.get(), &compare)); // 42
        if (compare == 0)
        {
            // Feature_EdgePinnedSecondaryTiles forced on
            wil::com_ptr<wf::IAsyncOperation<bool>> deleteOperation;
            if (SUCCEEDED_LOG(secondaryTileStaticsPriv->DeleteIfNotPinnedAsync(aumid.get(), otherTileId.get(), &deleteOperation))) // 48
            {
                boolean bDeleted = FALSE;
                if (SUCCEEDED_LOG(WaitForCompletionAndGetResults(deleteOperation.get(), &bDeleted, COWAIT_DISPATCH_CALLS))) // 51
                {
                    LOG_HR_IF(E_FAIL, !bDeleted); // 53
                }
            }

            break;
        }
    }
}
}
