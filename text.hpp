#include <Windows.h>

int www() {
    HIMC hImc = ImmGetContext(hWnd);
    if (!hImc) {
        hImc = ImmCreateContext();
        ImmAssociateContext(hWnd, hImc);
    }
    ImmSetOpenStatus(hImc, TRUE);
    SetTextColor(hDCMem, RGB(240, 240, 240));
    SetBkMode(hDCMem, TRANSPARENT);

    MSG Msg;

    while (TRUE) {
        if (PeekMessage(&Msg, hWnd,0, 0, PM_REMOVE)) {
            if (Msg.message == WM_QUIT) {
                return Msg.
                        wParam;
            } else if (Msg.message == WM_SIZE) {
                DeleteObject(hbmBitmap);
                hbmBitmap = CreateCompatibleBitmap(hDC, nWidth, nHeight);
                SelectObject(hDCMem, hbmBitmap
                );
            } else {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }
        } else {
            RECT TmpRc;
            GetClientRect(hWnd, &TmpRc
            );
            FillRect(hDCMem, &TmpRc, wc
                    .hbrBackground);
            DrawTextW(hDCMem, lpcwInput,
                      -1, &TmpRc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
            BitBlt(hDC,
                   0, 0, nWidth, nHeight, hDCMem, 0, 0, SRCCOPY);
            Sleep(1);
        }
    }


}

//WNDPROC部分：
LRESULT CALLBACK
WndProc(HWND
        hWnd,
        UINT
        Message, WPARAM
        wParam,
        LPARAM
        lParam
) {
    HIMC hImc;
    switch (Message) {
        case (WM_CREATE) :
            return 0;
        case (WM_CLOSE) :
            PostQuitMessage(0);
            return 0;
        case (WM_DESTROY) :
            return 0;
        case (WM_SIZE) :
            RECT WndRc;
            GetClientRect(hWnd, &WndRc
            );
            nWidth = WndRc.right;
            nHeight = WndRc.bottom;
            PostMessage(hWnd, MWM_RESIZED,
                        0, 0);
            return 0;
        case (WM_IME_CONTROL) :
            return 0;
        case (WM_IME_SETCONTEXT) :
            return 0;
        case (WM_CHAR):
            lpcwInput = (LPCWSTR) GlobalAlloc(GPTR, sizeof(WCHAR) * 2);
            WCHAR wwww[2];
            wwww[0] = (wchar_t) (char)
                    wParam;
            wwww[1] = 0;
            memcpy((void*) lpcwInput, wwww, sizeof(WCHAR) * 2);
            return 0;
        case (WM_IME_CHAR) :
            lpcwInput = (LPCWSTR) GlobalAlloc(GPTR, sizeof(WCHAR) * 2);
            WCHAR www[2];
            www[0] = wParam;
            www[1] = 0;
            memcpy((void*) lpcwInput, www, sizeof(WCHAR) * 2);
            return 0;
        case (WM_IME_COMPOSITION):
            if (
                    lParam & GCS_RESULTSTR
                    ) {
                hImc = ImmGetContext(hWnd);
                DWORD dwSize = ImmGetCompositionStringW(hImc, GCS_RESULTSTR, NULL, 0);
                dwSize += sizeof(WCHAR);
                LPCWSTR lpcwString = (LPCWSTR) GlobalAlloc(GPTR, dwSize);
                ImmGetCompositionStringW(hImc, GCS_RESULTSTR, (LPVOID)
                        lpcwString, dwSize);
                lpcwInput = (LPCWSTR) GlobalAlloc(GPTR, dwSize);
                memcpy((void*) lpcwInput, lpcwString, dwSize);
                return 0;
            } else {
                return
                        DefWindowProc(hWnd, Message, wParam, lParam
                        );
            }
        default:
            return
                    DefWindowProc(hWnd, Message, wParam, lParam
                    );
    }
}