#include "IMCore.h"
//////////////////////////////////////////////////
// Dll入口
BOOL WINAPI DllMain(HINSTANCE
        hInstance,
        DWORD dwReason, LPVOID
        pvReserved) {
    MWD::Core::IMCore::ptr core = MakeShared(MWD::Core::MCore);

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
        {
            core->InitWin32Config(hInstance);
            core->LoadRootPath("");
            core->LoadConfig();
        }
            break;
        case DLL_PROCESS_DETACH:
        {
            core->Destroy();
        }
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;
}

