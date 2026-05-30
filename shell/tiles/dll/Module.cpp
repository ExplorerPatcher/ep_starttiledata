#include "pch.h"

using namespace Microsoft::WRL;

extern "C" HRESULT WINAPI DllCanUnloadNow()
{
    return Module<InProc>::GetModule().Terminate() ? S_OK : S_FALSE;
}

extern "C" HRESULT WINAPI DllGetClassObject(REFCLSID objectClsid, REFIID requestedInterface, LPVOID* returnedInterface)
{
    return Module<InProc>::GetModule().GetClassObject(objectClsid, requestedInterface, returnedInterface);
}

BOOL APIENTRY DllMain(HMODULE instance, DWORD reason, LPVOID)
{
    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(instance);
            break;
    }
    return TRUE;
}
