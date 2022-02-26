#include "IMCommon.h"

BOOL WINAPI DllMain(HINSTANCE
                    hInstance,
                    DWORD dwReason, LPVOID
                    pvReserved) {

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
        case DLL_PROCESS_DETACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;
}


