#include <Windows.h>


#define MAX_LOADSTRING 100

#include <windows.h>
#include <ctffunc.h>
// C 运行时头文件
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <chrono>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <thread>
#include <queue>
#include <share.h>
#include <functional>

using namespace std;

#include <d2d1_3.h>

#pragma comment(lib, "d2d1.lib")

#include <wincodec.h>

#pragma comment(lib, "windowscodecs.lib")

#include <dwrite_3.h>

#pragma comment(lib, "dwrite.lib")

#include <msctf.h>
#include <imm.h>

#pragma comment(lib, "Imm32.lib")

#define _INTERFACECOVER_
#define _MEMBERVARIABLE_
#define _PUBLICMETHODS_
#define _PRIVATEMETHODS_

#define _IN_PARAM_
#define _OUT_PARAM_
#define _IN_OUT_PARAM_
#define MRESULT HRESULT
#define MAX_TEXT 1024*1024*10

typedef float Float;
typedef double Double;
typedef char Char;
typedef bool Boolean;
typedef int Integer;
typedef long Long;
typedef unsigned char Byte;
typedef void Void;
typedef std::string String;

static bool g_flag = true;
HWND hEdit;
HWND g_edit;
wchar_t* sztitle = new wchar_t[256];
char* sztitle2 = new char[256];


string GBKToUTF8(const string& strGBK) {

    string strOutUTF8;

    WCHAR* str1;

    int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, nullptr, 0);

    str1 = new WCHAR[n];

    MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n);

    n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, nullptr, 0, nullptr, NULL);

    char* str2 = new char[n];

    WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, nullptr, nullptr);

    strOutUTF8 = str2;

    delete[]str1;

    str1 = nullptr;

    delete[]str2;

    str2 = nullptr;

    return strOutUTF8;

}

static std::string codePointToUTF8(unsigned int cp) {
    std::string result;

// based on description from http://en.wikipedia.org/wiki/UTF-8

    if (cp <= 0x7f) {
        result.resize(1);
        result[0] = static_cast<char>(cp);
    } else if (cp <= 0x7FF) {
        result.resize(2);
        result[1] = static_cast<char>(0x80 | (0x3f & cp));
        result[0] = static_cast<char>(0xC0 | (0x1f & (cp >> 6)));
    } else if (cp <= 0xFFFF) {
        result.resize(3);
        result[2] = static_cast<char>(0x80 | (0x3f & cp));
        result[1] = 0x80 | static_cast<char>((0x3f & (cp >> 6)));
        result[0] = 0xE0 | static_cast<char>((0xf & (cp >> 12)));
    } else if (cp <= 0x10FFFF) {
        result.resize(4);
        result[3] = static_cast<char>(0x80 | (0x3f & cp));
        result[2] = static_cast<char>(0x80 | (0x3f & (cp >> 6)));
        result[1] = static_cast<char>(0x80 | (0x3f & (cp >> 12)));
        result[0] = static_cast<char>(0xF0 | (0x7 & (cp >> 18)));
    }

    return result;
}


struct Candidate {
    DWORD dwState;                 // 当前状态

    UINT uIndex;                   // 当前候选序号
    UINT uCount;                   // 总候选个数
    UINT uCurrentPage;             // 当前候选页序号
    UINT uPageCnt;                 // 候选页数
    DWORD dwPageStart;             // 当前页第一个候选在所有候选中的序号
    DWORD dwPageSize;              // 当前页候选个数

    wchar_t szComposing[256];      // 编码
    wchar_t szCandidate[10][256];  // 当前页候选
};

HWND g_hwndMain = NULL;
static bool g_bChineseIME;
static bool g_bUILessMode = false;
static bool g_bCandList = false;

#define IMEUI_STATE_OFF        0
#define IMEUI_STATE_ON        1
#define IMEUI_STATE_ENGLISH    2


ITfThreadMgr* m_tm;
DWORD m_dwUIElementSinkCookie = TF_INVALID_COOKIE;
DWORD m_dwThreadMgrCookie = TF_INVALID_COOKIE;
DWORD m_dwTextEditCookie = TF_INVALID_COOKIE;
DWORD m_dwAlpnSinkCookie = TF_INVALID_COOKIE;
DWORD m_dwOpenModeSinkCookie = TF_INVALID_COOKIE;
DWORD m_dwConvModeSinkCookie = TF_INVALID_COOKIE;
int m_nCandidateRefCount = NULL;
Candidate g_Candidate = {0};

TfClientId ptid;
DWORD m_dwCookie;
DWORD m_dwCookie1;
DWORD m_dwCookie2;
DWORD m_dwCookie3;
DWORD m_dwCookie4;
DWORD m_dwCookie5;
DWORD m_dwCookie6;

void WCout(wchar_t* w) {
    int len = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
    if (len) {
        char* cs = new char[len + 1];
        WideCharToMultiByte(CP_UTF8, 0, w, -1, cs, len + 1, nullptr, nullptr);
        cs[len] = '\0';

        cout << cs << endl;

        delete[] cs;
    }
}

void WCout(const wchar_t* w) {
    int len = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
    if (len) {
        char* cs = new char[len + 1];
        WideCharToMultiByte(CP_UTF8, 0, w, -1, cs, len + 1, nullptr, nullptr);
        cs[len] = '\0';

        cout << cs << endl;

        delete[] cs;
    }
}

ITfUIElement* GetUIElement(DWORD dwUIElementId) {
    ITfUIElementMgr* puiem;
    ITfUIElement* pElement;

    if (SUCCEEDED(m_tm->QueryInterface(__uuidof(ITfUIElementMgr), (void**) &puiem))) {
        puiem->GetUIElement(dwUIElementId, &pElement);
        puiem->Release();
    }

    return pElement;
}

void MakeReadingInformationString(ITfReadingInformationUIElement* preading) {
    UINT cchMax;
    UINT uErrorIndex = 0;
    BOOL fVertical;
    DWORD dwFlags;

    preading->GetUpdatedFlags(&dwFlags);
    preading->GetMaxReadingStringLength(&cchMax);
    preading->GetErrorIndex(&uErrorIndex);    // errorIndex is zero-based
    preading->IsVerticalOrderPreferred(&fVertical);

    BSTR bstr;
    if (SUCCEEDED(preading->GetString(&bstr))) {
        if (bstr) {
            wcscpy_s(g_Candidate.szComposing, bstr);

            //应注意TSF框架要求输入内部必须使用SysAlloc() 分配候选列表字符串保存空间
            //在调用pcandidate->GetString 之后，必须使用SysFreeString(bstr)释放
            SysFreeString(bstr);
        }
    }

    WCout(g_Candidate.szComposing);
}

void MakeCandidateStrings(ITfCandidateListUIElement* pcandidate) {
    if (pcandidate) {
        BSTR bstr;
        UINT* pIndexList = NULL;
        pcandidate->GetSelection(&g_Candidate.uIndex);            //获取当前选中状态的候选序号（可设置高亮显示，一般为第一候选）
        pcandidate->GetCount(&g_Candidate.uCount);                //当前候选列表总数
        pcandidate->GetCurrentPage(&g_Candidate.uCurrentPage);    //当前候选列表所在的页

        g_bCandList = true;

        pcandidate->GetPageIndex(NULL, 0, &g_Candidate.uPageCnt); //获取候选列表页每一页对应的起始序号
        if (g_Candidate.uPageCnt > 0) {
            pIndexList = (UINT*) malloc(sizeof(UINT) * g_Candidate.uPageCnt);
            if (pIndexList) {
                pcandidate->GetPageIndex(pIndexList, g_Candidate.uPageCnt, &g_Candidate.uPageCnt);
                g_Candidate.dwPageStart = pIndexList[g_Candidate.uCurrentPage];
                g_Candidate.dwPageSize = (g_Candidate.uCurrentPage < g_Candidate.uPageCnt - 1) ?
                                         min(g_Candidate.uCount, pIndexList[g_Candidate.uCurrentPage + 1]) -
                                         g_Candidate.dwPageStart :
                                         g_Candidate.uCount - g_Candidate.dwPageStart;
            }
        }

        UINT uCandPageSize = min(g_Candidate.dwPageSize, 10);  // 本示例的g_Candidate.szCandidate最大个数为10, 因此min处理
        for (UINT i = g_Candidate.dwPageStart, j = 0; (DWORD) i < g_Candidate.uCount && j < uCandPageSize; i++, j++) {
            if (SUCCEEDED(pcandidate->GetString(i, &bstr)))  //获取候选列表的第i个候选串
            {
                if (bstr) {
                    wcscpy_s(g_Candidate.szCandidate[j], bstr);

                    // 应注意TSF框架要求输入内部必须使用SysAlloc() 分配候选列表字符串保存空间
                    //在调用pcandidate->GetString 之后，必须使用SysFreeString(bstr)释放
                    SysFreeString(bstr);
                }
            }
        }
        if (pIndexList) {
            free(pIndexList);
        }
    }
}

static void CloseCandidateList() {
    g_bCandList = false;
    memset(&g_Candidate, 0, sizeof(g_Candidate));
}

IEnumITfCompositionView* GetCompViewEnum(ITfContext* pCtx) {
    // Make sure there is a composition context
    ITfContextComposition* pctxcomp = NULL;
    pCtx->QueryInterface(IID_ITfContextComposition, (void**) &pctxcomp);
    if (!pctxcomp) return NULL;

    // Obtain composition view enumerator
    IEnumITfCompositionView* enum_view = NULL;
    pctxcomp->EnumCompositions(&enum_view);
    pctxcomp->Release();
    return enum_view;
}

ITfRange* CombineCompRange(ITfContext* pCtx, TfEditCookie cookie) {
    // Make sure there is a composition view enumerator
    IEnumITfCompositionView* pEnumview = GetCompViewEnum(pCtx);
    if (!pEnumview) return NULL;

    // Combine composition ranges from all views
    ITfRange* range = NULL;
    ITfCompositionView* view = NULL;
    while (pEnumview->Next(1, &view, NULL) == S_OK) {
        ITfRange* prange = NULL;
        if (view->GetRange(&prange) == S_OK) {
            if (!range) {
                prange->Clone(&range);
            } else {
                range->ShiftEndToRange(cookie, prange, TF_ANCHOR_END);
            }
            prange->Release();
        }
        view->Release();
    }
    pEnumview->Release();
    return range;
}

bool fetchRangeExtent(ITfRange* pRange, long* start, ULONG* length) {
    HRESULT res = S_OK;
    if (!pRange) return false;
    ITfRangeACP* pRangeACP = NULL;
    res = pRange->QueryInterface(IID_ITfRangeACP, (void**) &pRangeACP);
    if (res != S_OK || !pRangeACP) return false;
    res = pRangeACP->GetExtent(start, (long*) length);
    pRangeACP->Release();
    return true ? (res == S_OK) : false;
}

BOOL GetCompartments(ITfCompartmentMgr** ppcm, ITfCompartment** ppTfOpenMode,
                     ITfCompartment** ppTfConvMode) {
    ITfCompartmentMgr* pcm = NULL;
    ITfCompartment* pTfOpenMode = NULL;
    ITfCompartment* pTfConvMode = NULL;

    static GUID _GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION =
            {
                    0xCCF05DD8, 0x4A87, 0x11D7, 0xA6, 0xE2, 0x00, 0x06, 0x5B, 0x84, 0x43, 0x5C
            };

    HRESULT hr;
    if (SUCCEEDED(hr = m_tm->QueryInterface(IID_ITfCompartmentMgr, (void**) &pcm))) {
        if (SUCCEEDED(hr = pcm->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pTfOpenMode))) {
            if (SUCCEEDED(hr = pcm->GetCompartment(_GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION,
                                                   &pTfConvMode))) {
                *ppcm = pcm;
                *ppTfOpenMode = pTfOpenMode;
                *ppTfConvMode = pTfConvMode;
                return TRUE;
            }
            pTfOpenMode->Release();
        }
        pcm->Release();
    }
    return FALSE;
}

BOOL SetupCompartmentSinks(BOOL bRemoveOnly, ITfCompartment* pTfOpenMode,
                           ITfCompartment* pTfConvMode);


void UpdateImeState(BOOL bResetCompartmentEventSink) {
    ITfCompartmentMgr* pcm;
    ITfCompartment* pTfOpenMode = NULL;
    ITfCompartment* pTfConvMode = NULL;
    if (GetCompartments(&pcm, &pTfOpenMode, &pTfConvMode)) {
        VARIANT valOpenMode;
        VARIANT valConvMode;
        pTfOpenMode->GetValue(&valOpenMode);
        pTfConvMode->GetValue(&valConvMode);
        if (valOpenMode.vt == VT_I4) {
            if (g_bChineseIME) {
                g_Candidate.dwState =
                        valOpenMode.lVal != 0 && valConvMode.lVal != 0 ? IMEUI_STATE_ON : IMEUI_STATE_ENGLISH;
            } else {
                g_Candidate.dwState = valOpenMode.lVal != 0 ? IMEUI_STATE_ON : IMEUI_STATE_OFF;
            }
        }
        VariantClear(&valOpenMode);
        VariantClear(&valConvMode);

        if (bResetCompartmentEventSink) {
            SetupCompartmentSinks(FALSE, pTfOpenMode, pTfConvMode);    // Reset compartment sinks
        }
        pTfOpenMode->Release();
        pTfConvMode->Release();
        pcm->Release();

    }
}

class CUIElementSink : public ITfUIElementSink,
                       public ITfInputProcessorProfileActivationSink,
                       public ITfCompartmentEventSink,
                       public ITfTextEditSink,
                       public ITfThreadMgrEventSink,

                       public ITfActiveLanguageProfileNotifySink,
                       public ITfLanguageProfileNotifySink,

                       public ITfContextKeyEventSink,

                       public ITfContextOwnerCompositionSink {
public:
    CUIElementSink() : _cRef(1) {};

    ~CUIElementSink() = default;


    HRESULT STDMETHODCALLTYPE OnStartComposition(
            /* [in] */ __RPC__in_opt ITfCompositionView* pComposition,
            /* [out] */ __RPC__out BOOL* pfOk) override {
        cout << "OnStartComposition" << endl;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnUpdateComposition(
            /* [in] */ __RPC__in_opt ITfCompositionView* pComposition,
            /* [in] */ __RPC__in_opt ITfRange* pRangeNew) override {
        cout << "OnUpdateComposition" << endl;

        return S_OK;

    }

    HRESULT STDMETHODCALLTYPE OnEndComposition(
            /* [in] */ __RPC__in_opt ITfCompositionView* pComposition) override {
        cout << "OnEndComposition" << endl;

        return S_OK;

    }


    HRESULT STDMETHODCALLTYPE OnKeyDown(
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam,
            /* [out] */ BOOL* pfEaten) override {
        cout << wParam << endl;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnKeyUp(
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam,
            /* [out] */ BOOL* pfEaten) override {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnTestKeyDown(
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam,
            /* [out] */ BOOL* pfEaten) override {
        cout << wParam << endl;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnTestKeyUp(
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam,
            /* [out] */ BOOL* pfEaten) override {
        return S_OK;
    }


    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) override {
        if (ppvObj == nullptr)
            return E_INVALIDARG;

        *ppvObj = nullptr;

        if (IsEqualIID(riid, IID_IUnknown)) {
            *ppvObj = reinterpret_cast<IUnknown*>(this);
        } else if (IsEqualIID(riid, __uuidof(ITfUIElementSink))) {
            // 输入法候选列表更新时回调
            *ppvObj = (ITfUIElementSink*) this;
        } else if (IsEqualIID(riid, __uuidof(ITfInputProcessorProfileActivationSink))) {
            // 输入法激活时回调
            *ppvObj = (ITfInputProcessorProfileActivationSink*) this;
        } else if (IsEqualIID(riid, __uuidof(ITfCompartmentEventSink))) {
            // 输入法中/英状态更改时回调
            *ppvObj = (ITfCompartmentEventSink*) this;
        } else if (IsEqualIID(riid, IID_ITfTextEditSink)) {
            *ppvObj = (ITfTextEditSink*) this;
        }
        if (*ppvObj) {
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG)
    AddRef() override {
        return ++_cRef;
    }

    STDMETHODIMP_(ULONG)
    Release() override {
        LONG cr = --_cRef;

        if (_cRef == 0) {
            delete this;
        }

        return cr;
    }

    // ITfUIElementSink
    // Notifications for Reading Window events. We could process candidate as well, but we'll use IMM for simplicity sake.
    STDMETHODIMP BeginUIElement(DWORD dwUIElementId, BOOL* pbShow) override {
        ITfUIElement* pElement = GetUIElement(dwUIElementId);
        if (!pElement)
            return E_INVALIDARG;

        ITfReadingInformationUIElement* preading = NULL;
        ITfCandidateListUIElement* pcandidate = NULL;
        //*pbShow = FALSE;
        *pbShow = TRUE;
        if (!g_bCandList && SUCCEEDED(pElement->QueryInterface(__uuidof(ITfReadingInformationUIElement),
                                                               (void**) &preading))) {
            MakeReadingInformationString(preading);
            preading->Release();
        } else if (SUCCEEDED(pElement->QueryInterface(__uuidof(ITfCandidateListUIElement),
                                                      (void**) &pcandidate))) {
            m_nCandidateRefCount++;
            MakeCandidateStrings(pcandidate);
            pcandidate->Release();
        }

        pElement->Release();

        cout << "BeginUIElement" << endl;
        return S_OK;
    }

    STDMETHODIMP UpdateUIElement(DWORD dwUIElementId) override {
        //ITfUIElement* pElement = GetUIElement(dwUIElementId);
        //if (!pElement)
        //    return E_INVALIDARG;
        //
        //ITfReadingInformationUIElement* preading = NULL;
        //ITfCandidateListUIElement* pcandidate = NULL;
        //if (!g_bCandList && SUCCEEDED(pElement->QueryInterface(__uuidof(ITfReadingInformationUIElement),
        //                                                       (void**) &preading))) {
        //    MakeReadingInformationString(preading);
        //    preading->Release();
        //} else if (SUCCEEDED(pElement->QueryInterface(__uuidof(ITfCandidateListUIElement),
        //                                              (void**) &pcandidate))) {
        //    MakeCandidateStrings(pcandidate);
        //    pcandidate->Release();
        //}
        //
        //pElement->Release();

        //WCout(g_Candidate.szCandidate[g_Candidate.uIndex - g_Candidate.dwPageStart]);

        ITfUIElement* pElement = GetUIElement(dwUIElementId);
        ITfCandidateListUIElement* pcandidate;
        pElement->QueryInterface(__uuidof(ITfCandidateListUIElement), (void**) &pcandidate);
        UINT puIndex;
        pcandidate->GetSelection(&puIndex);
        cout << "UpdateUIElement: " << puIndex << endl;
        return S_OK;
    }

    STDMETHODIMP EndUIElement(DWORD dwUIElementId) override {
        ITfUIElement* pElement = GetUIElement(dwUIElementId);
        if (!pElement)
            return E_INVALIDARG;

        ITfReadingInformationUIElement* preading = NULL;
        if (!g_bCandList && SUCCEEDED(pElement->QueryInterface(__uuidof(ITfReadingInformationUIElement),
                                                               (void**) &preading))) {
            MakeReadingInformationString(preading);

            preading->Release();
        }

        ITfCandidateListUIElement* pcandidate;
        ITfCandidateListUIElementBehavior* pcb;
        pElement->QueryInterface(__uuidof(ITfCandidateListUIElementBehavior),
                                 (void**) &pcb);

        if (SUCCEEDED(pElement->QueryInterface(__uuidof(ITfCandidateListUIElement),
                                               (void**) &pcandidate))) {
            //MakeCandidateStrings(pcandidate);
            m_nCandidateRefCount--;
            if (m_nCandidateRefCount == 0)
                CloseCandidateList();
            //pcandidate->Release();
        }
        UINT uindex;
        UINT uindex2;
        pcandidate->GetSelection(&uindex);//获取当前选中状态的候选序号（可设置高亮显示，一般为第一候选）
        pcb->GetSelection(&uindex2);
        pcandidate->GetCount(&g_Candidate.uCount);                //当前候选列表总数
        pcandidate->GetCurrentPage(&g_Candidate.uCurrentPage);    //当前候选列表所在的页

        //UINT* pIndexList = (UINT*) malloc(sizeof(UINT) * g_Candidate.uPageCnt);
        pcandidate->GetPageIndex(nullptr, 0, &g_Candidate.uPageCnt); //获取候选列表页每一页对应的起始序号

        //pcandidate->GetPageIndex(pIndexList, g_Candidate.uPageCnt, &g_Candidate.uPageCnt);

        //g_Candidate.dwPageSize = (g_Candidate.uCurrentPage < g_Candidate.uPageCnt - 1) ?
        //                         min(g_Candidate.uCount, pIndexList[g_Candidate.uCurrentPage + 1]) -
        //                         g_Candidate.dwPageStart :
        //                         g_Candidate.uCount - g_Candidate.dwPageStart;

        BSTR bstr;
        pcandidate->GetString(uindex, &bstr);
        cout << uindex << endl
             << uindex2 << endl
             << g_Candidate.uCount << endl
             << g_Candidate.uCurrentPage << endl
             << g_Candidate.uPageCnt << endl;
        WCout(bstr);
        SysFreeString(bstr);

        //UINT uCandPageSize = min(g_Candidate.dwPageSize, 10);
        //for (UINT i = g_Candidate.dwPageStart, j = 0;
        //     (DWORD) i < g_Candidate.uCount && j < uCandPageSize; i++, j++) {
        //    if (SUCCEEDED(pcandidate->GetString(i, &bstr)))
        //    {
        //        if (bstr) {
        //            wcscpy_s(g_Candidate.szCandidate[j], bstr);
        //
        //            SysFreeString(bstr);
        //        }
        //    }
        //}
        //
        //WCout(g_Candidate.szCandidate[0]);
        //
        //cout << g_Candidate.uIndex << endl
        //     << g_Candidate.uCount << endl
        //     << g_Candidate.uCurrentPage << endl
        //     << g_Candidate.uPageCnt << endl;
        //
        //free(pIndexList);
        pcandidate->Release();
        pElement->Release();

        cout << "EndUIElement" << endl;
        return S_OK;
    }

    // ITfInputProcessorProfileActivationSink
    STDMETHODIMP OnActivated(DWORD dwProfileType, LANGID langid, REFCLSID clsid, REFGUID catid,
                             REFGUID guidProfile, HKL hkl, DWORD dwFlags) override {

        if (IsEqualIID(catid, GUID_TFCAT_TIP_KEYBOARD) && (dwFlags & TF_IPSINK_FLAG_ACTIVE)) {
#define LANG_CHS MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)
#define LANG_CHT MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL)
            g_bChineseIME = (dwProfileType & TF_PROFILETYPE_INPUTPROCESSOR) && langid == LANG_CHT;
            if (dwProfileType & TF_PROFILETYPE_INPUTPROCESSOR) {
                UpdateImeState(TRUE);
            } else
                g_Candidate.dwState = IMEUI_STATE_OFF;
        }
        return S_OK;
    }

    // ITfCompartmentEventSink
    // Notification for open mode (toggle state) change
    STDMETHODIMP OnChange(REFGUID rguid) override {
        cout << "OnChange" << endl;
        return S_OK;
    }

    // ITfThreadMgrEventSink methods
    // 不同线程的输入法线程切换时
    STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr*) override {
        cout << "OnInitDocumentMgr" << endl;
        return S_OK;
    }

    STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr*) override {
        cout << "OnUninitDocumentMgr" << endl;
        return S_OK;
    }

    STDMETHODIMP OnSetFocus(ITfDocumentMgr*, ITfDocumentMgr*) override {
        cout << "OnSetFocus" << endl;
        return S_OK;
    }

    STDMETHODIMP OnPushContext(ITfContext*) override {
        cout << "OnPushContext" << endl;
        return S_OK;
    }

    STDMETHODIMP OnPopContext(ITfContext*) override {
        cout << "OnPopContext" << endl;
        return S_OK;
    }

    // ITfTextEditSink methods
    STDMETHODIMP OnEndEdit(ITfContext* context, TfEditCookie cookie, ITfEditRecord* record) override {
        // TSF input processor performing composition
        //ITfRange* pRange = CombineCompRange(context, cookie);
        //if (!pRange) {
        //    return S_OK;
        //}
        //ULONG len = ARRAYSIZE(g_Candidate.szComposing) - 1;
        //pRange->GetText(cookie, 0, g_Candidate.szComposing, len, &len);
        //g_Candidate.szComposing[min(len, 255)] = L'\0';
        //long compStart = 0;
        //fetchRangeExtent(pRange, &compStart, &len);
        //long selStart = compStart;
        //long selEnd = compStart;
        //TF_SELECTION tfSelection{};
        //if (context->GetSelection(cookie, TF_DEFAULT_SELECTION, 1, &tfSelection, &len) == S_OK && tfSelection.range) {
        //    if (fetchRangeExtent(tfSelection.range, &selStart, &len)) {
        //        selEnd = selStart + len;
        //    }
        //    tfSelection.range->Release();
        //}
        //selStart = max(0, selStart - compStart);
        //selEnd = max(0, selEnd - compStart);

        ITfRange* pRange = CombineCompRange(context, cookie);
        ULONG len = 256;
        wchar_t* bstr = new wchar_t[len + 1];

        if (pRange)
            pRange->GetText(cookie, 0, bstr, len + 1, &len);

        bstr[len] = L'\0';
        WCout(bstr);

        delete[] bstr;
        cout << "OnEndEdit" << endl;
        return S_OK;
    }


    // ITfActiveLanguageProfileNotifySink
    HRESULT STDMETHODCALLTYPE OnActivated(
            /* [in] */ __RPC__in REFCLSID clsid,
            /* [in] */ __RPC__in REFGUID guidProfile,
            /* [in] */ BOOL fActivated) override {
        cout << "OnActivated2" << endl;
        return S_OK;
    }

    // ITfLanguageProfileNotifySink
    HRESULT STDMETHODCALLTYPE OnLanguageChange(
            /* [in] */ LANGID langid,
            /* [out] */ __RPC__out BOOL* pfAccept) override {
        cout << "OnLanguageChange" << endl;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnLanguageChanged() override {
        cout << "OnLanguageChanged" << endl;
        return S_OK;
    }

public:
    void UpdateTextEditSink(ITfDocumentMgr* docMgr) {
        if (!docMgr) return;
        ITfContext* ctx;
        HRESULT hr = docMgr->GetBase(&ctx);
        if (hr == S_OK) {
            ITfSource* src;
            if (SUCCEEDED(hr = ctx->QueryInterface(__uuidof(ITfSource), (void**) &src))) {
                src->AdviseSink(__uuidof(ITfTextEditSink), (ITfTextEditSink*) this, &m_dwTextEditCookie);
                src->Release();
            }
            ctx->Release();
        }
    }

private:
    LONG _cRef;
};

CUIElementSink* m_TsfSink = new CUIElementSink;

BOOL SetupCompartmentSinks(BOOL bRemoveOnly, ITfCompartment* pTfOpenMode,
                           ITfCompartment* pTfConvMode) {
    bool bLocalCompartments = false;
    ITfCompartmentMgr* pcm = NULL;
    BOOL bRc = FALSE;
    HRESULT hr = E_FAIL;

    if (!pTfOpenMode && !pTfConvMode) {
        bLocalCompartments = true;
        GetCompartments(&pcm, &pTfOpenMode, &pTfConvMode);
    }
    if (!(pTfOpenMode && pTfConvMode)) {
        // Invalid parameters or GetCompartments() has failed.
        return FALSE;
    }
    ITfSource* srcOpenMode = NULL;
    if (SUCCEEDED(hr = pTfOpenMode->QueryInterface(IID_ITfSource, (void**) &srcOpenMode))) {
        // Remove existing sink for open mode
        if (m_dwOpenModeSinkCookie != TF_INVALID_COOKIE) {
            srcOpenMode->UnadviseSink(m_dwOpenModeSinkCookie);
            m_dwOpenModeSinkCookie = TF_INVALID_COOKIE;
        }
        // Setup sink for open mode (toggle state) change
        if (bRemoveOnly || SUCCEEDED(hr = srcOpenMode->AdviseSink(IID_ITfCompartmentEventSink,
                                                                  (ITfCompartmentEventSink*) m_TsfSink,
                                                                  &m_dwOpenModeSinkCookie))) {
            ITfSource* srcConvMode = NULL;
            if (SUCCEEDED(hr = pTfConvMode->QueryInterface(IID_ITfSource, (void**) &srcConvMode))) {
                // Remove existing sink for open mode
                if (m_dwConvModeSinkCookie != TF_INVALID_COOKIE) {
                    srcConvMode->UnadviseSink(m_dwConvModeSinkCookie);
                    m_dwConvModeSinkCookie = TF_INVALID_COOKIE;
                }
                // Setup sink for open mode (toggle state) change
                if (bRemoveOnly || SUCCEEDED(hr = srcConvMode->AdviseSink(IID_ITfCompartmentEventSink,
                                                                          (ITfCompartmentEventSink*) m_TsfSink,
                                                                          &m_dwConvModeSinkCookie))) {
                    bRc = TRUE;
                }
                srcConvMode->Release();
            }
        }
        srcOpenMode->Release();
    }
    if (bLocalCompartments) {
        pTfOpenMode->Release();
        pTfConvMode->Release();
        pcm->Release();
    }
    return bRc;
}

class Graphics {
public:

#pragma region G1

    Graphics() {
        m_pFactory = NULL;
        m_pRenderTarget = NULL;
        m_pBrush = NULL;

        m_pWicFactory = NULL;
        m_pBmp = NULL;
        m_pWicDecoder = NULL;
        m_pWicFrame = NULL;
        m_pWicConverter = NULL;
    }

    ~Graphics() {
        if (m_pFactory)m_pFactory->Release();
        if (m_pRenderTarget)m_pRenderTarget->Release();
        if (m_pBrush)m_pBrush->Release();

        if (m_pWicFactory)m_pWicFactory->Release();
        if (m_pBmp)m_pBmp->Release();
        if (m_pWicDecoder)m_pWicDecoder->Release();
        if (m_pWicFrame)m_pWicFrame->Release();
        if (m_pWicConverter)m_pWicConverter->Release();
    }

    //bool init(HWND windowHandle) {
    //	HRESULT res = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pFactory);
    //	if (res != S_OK) return false;

    //	RECT rect;
    //	GetClientRect(windowHandle, &rect);

    //	res = m_pFactory->CreateHwndRenderTarget(
    //		D2D1::RenderTargetProperties(),
    //		D2D1::HwndRenderTargetProperties(
    //			windowHandle, D2D1::SizeU(rect.right - rect.left, rect.bottom - rect.top)
    //		),
    //		&m_pRenderTarget);

    //	//DWriteCreateFactory(
    //	//	DWRITE_FACTORY_TYPE_SHARED,
    //	//	__uuidof(IDWriteFactory3),
    //	//	reinterpret_cast<IUnknown**>(&m_pWriteFactory)
    //	//);

    //	m_pFactory->CreatePathGeometry(&m_pPathGeometry);

    //	return res != S_OK ? false : true;
    //}

    void beginDraw() {
        m_pRenderTarget->BeginDraw();
    }

    void endDraw() {
        m_pRenderTarget->EndDraw();
    }

    void clearScreen(float r, float g, float b) {
        m_pRenderTarget->Clear(D2D1::ColorF(r, g, b));
    }

    void drawCircle(float x, float y, float radius, float r, float g, float b) {

        m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(r, g, b), (ID2D1SolidColorBrush**) &m_pBrush);
        m_pRenderTarget->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius), m_pBrush, 1.0f);
    }

    void drawRoundRect(float left, float top, float right, float bottom, float radius, float r, float g, float b) {
        m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(r, g, b), (ID2D1SolidColorBrush**) &m_pBrush);
        m_pRenderTarget->DrawRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(left, top, right, bottom), radius, radius),
                                              m_pBrush);

        //m_pRenderTarget->FillRectangle(D2D1::RectF(left, top, right, bottom), m_pBrush);
    }

    //void drawImage(LPCWSTR fileName, D2D1_RECT_F rect) {

    //	CoCreateInstance(
    //		CLSID_WICImagingFactory,
    //		NULL,
    //		CLSCTX_INPROC_SERVER,
    //		IID_IWICImagingFactory,
    //		(LPVOID*)&m_pWicFactory);

    //	m_pWicFactory->CreateDecoderFromFilename(
    //		fileName,
    //		NULL,
    //		GENERIC_READ,
    //		WICDecodeMetadataCacheOnLoad,
    //		&m_pWicDecoder
    //	);

    //	m_pWicDecoder->GetFrame(0, &m_pWicFrame);

    //	m_pWicFactory->CreateFormatConverter(&m_pWicConverter);

    //	m_pWicConverter->Initialize(
    //		m_pWicFrame,
    //		GUID_WICPixelFormat32bppPBGRA,
    //		WICBitmapDitherTypeNone,
    //		NULL,
    //		0.0,
    //		WICBitmapPaletteTypeCustom
    //	);

    //	m_pRenderTarget->CreateBitmapFromWicBitmap(m_pWicConverter, NULL, &m_pBmp);

    //	//m_pRenderTarget->DrawBitmap(
    //	//	m_pBmp,
    //	//	rect,
    //	//	1.0f,
    //	//	D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
    //	//	D2D1::RectF(0.0f, 0.0f, m_pBmp->GetSize().width, m_pBmp->GetSize().height)
    //	//);


    //	m_pWriteFactory->CreateTextFormat(
    //		L"Microsoft Yahei",
    //		NULL,
    //		DWRITE_FONT_WEIGHT_REGULAR,
    //		DWRITE_FONT_STYLE_NORMAL,
    //		DWRITE_FONT_STRETCH_NORMAL,
    //		16.0f,
    //		L"cn-zh",
    //		&m_pTextFormat
    //	);
    //	//m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    //	//m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    //	m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    //	m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0, 0), &m_pBrush);

    //	LPCWSTR wszText_ = L"Hello World using  DirectWrite!Hello World using  DirectWrite!";
    //	UINT32 cTextLength_ = (UINT32)wcslen(wszText_);

    //	//m_pRenderTarget->DrawTextA(
    //	//	wszText_,
    //	//	cTextLength_,
    //	//	m_pTextFormat,
    //	//	rect,
    //	//	m_pBrush
    //	//);


    //	ID2D1BitmapBrush* brush;
    //	ID2D1Bitmap* bmp;
    //	m_pRenderTarget->CreateBitmap(
    //		D2D1::SizeU(10, 10),
    //		D2D1::BitmapProperties(
    //			D2D1::PixelFormat(
    //				DXGI_FORMAT_B5G6R5_UNORM,
    //				D2D1_ALPHA_MODE_IGNORE)
    //		),
    //		&bmp
    //	);
    //	m_pRenderTarget->CreateBitmapBrush(bmp, &brush);



    //	IDWriteTextLayout* m_textLayout;
    //	m_pWriteFactory->CreateTextLayout(
    //		wszText_,      // The string to be laid out and formatted.
    //		cTextLength_,  // The length of the string.
    //		m_pTextFormat,  // The text format to apply to the string (contains font information, etc).
    //		400,         // The width of the layout box.
    //		400,        // The height of the layout box.
    //		&m_textLayout  // The IDWriteTextLayout interface pointer.
    //	);

    //	IDWriteTextLayout3* m_textLayout3 = (IDWriteTextLayout3*)m_textLayout;

    //	DWRITE_TEXT_RANGE textRange = { 0,        // Start index where "DirectWrite" appears.
    //								6 };
    //	//m_textLayout->SetFontSize(100.0f, textRange);

    //	//m_textLayout->SetUnderline(TRUE, textRange);

    //	textRange = { 0,
    //						   11 };
    //	//m_textLayout->SetFontWeight(DWRITE_FONT_WEIGHT_EXTRA_BOLD, textRange);

    //	IDWriteTypography* pTypography;
    //	m_pWriteFactory->CreateTypography(&pTypography);

    //	DWRITE_FONT_FEATURE fontFeature = { DWRITE_FONT_FEATURE_TAG_SUPERSCRIPT,
    //							   1 };
    //	pTypography->AddFontFeature(fontFeature);

    //	textRange = { 0,
    //						   cTextLength_ };
    //	//m_textLayout->SetTypography(pTypography, textRange);


    //	//m_textLayout->SetFontStretch(DWRITE_FONT_STRETCH_ULTRA_EXPANDED, textRange);


    //	m_textLayout3->SetMaxWidth(static_cast<FLOAT>(400.0f / 1));
    //	m_textLayout3->SetMaxHeight(static_cast<FLOAT>(400.0f / 1));

    //	m_textLayout3->SetCharacterSpacing(0.0f, 10.0f, 0.1f, textRange);
    //	m_textLayout3->SetLineSpacing(DWRITE_LINE_SPACING_METHOD_UNIFORM, 26.0f, 16.0f);

    //	//((IDWriteTextLayout3*)m_textLayout)->SetCharacterSpacing();

    //	D2D1_POINT_2F origin = D2D1::Point2F(
    //		static_cast<FLOAT>(0 / 1),
    //		static_cast<FLOAT>(0 / 1)
    //	);

    //	m_pRenderTarget->DrawTextLayout(
    //		origin,
    //		m_textLayout3,
    //		m_pBrush
    //	);

    //	DWRITE_TEXT_METRICS textMetrics;
    //	m_textLayout3->GetMetrics(&textMetrics);

    //	D2D1_SIZE_F re = D2D1::SizeF(ceil(textMetrics.widthIncludingTrailingWhitespace), ceil(textMetrics.height));
    //	if (flag) {
    //		OutputDebugString("\n=========================================\n");
    //		CHAR cs[1024];
    //		sprintf_s(cs, "%f-%f-%u\n", re.width, re.height, textMetrics.lineCount);
    //		OutputDebugString(cs);
    //		flag = FALSE;
    //	}

    //	ID2D1SolidColorBrush* b;
    //	m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0, 1.0f, 0), &b);
    //	CustomWriter* cwt = new CustomWriter(
    //		m_pFactory,
    //		m_pRenderTarget,
    //		m_pBrush,
    //		b
    //	);

    //	m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(200.f, 20.f));
    //	//m_textLayout->Draw(
    //	//	NULL,
    //	//	cwt,  // Custom text renderer.
    //	//	0,
    //	//	0
    //	//);

    //}

    void resize(int w, int h) {
        m_pRenderTarget->Resize(D2D1::SizeU(w, h));
    }

    void draw() {
        ID2D1GeometrySink* pSink;
        m_pPathGeometry->Open(&pSink);

        pSink->BeginFigure(D2D1::Point2F(0, 0), D2D1_FIGURE_BEGIN_FILLED);

        pSink->AddLine(D2D1::Point2F(200, 0));

        pSink->AddBezier(
                D2D1::BezierSegment(
                        D2D1::Point2F(150, 50),
                        D2D1::Point2F(150, 150),
                        D2D1::Point2F(200, 200))
        );

        pSink->AddLine(D2D1::Point2F(0, 200));

        pSink->AddBezier(
                D2D1::BezierSegment(
                        D2D1::Point2F(50, 150),
                        D2D1::Point2F(50, 50),
                        D2D1::Point2F(0, 0))
        );

        pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
        pSink->Close();


        static const D2D1_GRADIENT_STOP stops[] =
                {
                        {0.f, {0.f, 1.f, 1.f, 0.25f}},
                        {1.f, {0.f, 0.f, 1.f, 1.f}},
                };
        ID2D1SolidColorBrush* brush;

        m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Black),
                &brush
        );

        ID2D1GradientStopCollection* pGradientStops;
        ID2D1LinearGradientBrush* m_pLGBrush;

        m_pRenderTarget->CreateGradientStopCollection(
                stops,
                ARRAYSIZE(stops),
                &pGradientStops
        );

        m_pRenderTarget->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                        D2D1::Point2F(100, 0),
                        D2D1::Point2F(100, 200)),
                D2D1::BrushProperties(),
                pGradientStops,
                &m_pLGBrush
        );

        m_pRenderTarget->SetTransform(
                D2D1::Matrix3x2F::Translation(20.f, 20.f)
        );

        // Draw the hour glass geometry at the upper left corner of the client area.
        m_pRenderTarget->DrawGeometry(m_pPathGeometry, brush, 10.f);
        m_pRenderTarget->FillGeometry(m_pPathGeometry, m_pLGBrush);
    }

    void draw1(int x, int y) {
        ID2D1PathGeometry* m_pPathGeometry;
        ID2D1GeometrySink* pSink;

        m_pFactory->CreatePathGeometry(&m_pPathGeometry);
        m_pPathGeometry->Open(&pSink);

        pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

        float radiusXY = 20.0f;
        float radius = 20.0f;

        pSink->BeginFigure(D2D1::Point2F(0, radiusXY), D2D1_FIGURE_BEGIN_FILLED);
        // 太阳顶部圆弧
        pSink->AddArc(
                D2D1::ArcSegment(
                        D2D1::Point2F(radiusXY, 0), // end point
                        D2D1::SizeF(radius, radius),
                        0.0f, // rotation angle
                        D2D1_SWEEP_DIRECTION_CLOCKWISE,
                        D2D1_ARC_SIZE_SMALL
                ));

        pSink->AddLine(D2D1::Point2F(400 - radiusXY, 0));

        pSink->AddArc(
                D2D1::ArcSegment(
                        D2D1::Point2F(400, radiusXY), // end point
                        D2D1::SizeF(radius, radius),
                        0.0f, // rotation angle
                        D2D1_SWEEP_DIRECTION_CLOCKWISE,
                        D2D1_ARC_SIZE_SMALL
                ));

        pSink->AddLine(D2D1::Point2F(400, 400 - radiusXY));

        pSink->AddArc(
                D2D1::ArcSegment(
                        D2D1::Point2F(400 - radiusXY, 400), // end point
                        D2D1::SizeF(radius, radius),
                        0.0f, // rotation angle
                        D2D1_SWEEP_DIRECTION_CLOCKWISE,
                        D2D1_ARC_SIZE_SMALL
                ));

        pSink->AddLine(D2D1::Point2F(radiusXY, 400));

        pSink->AddArc(
                D2D1::ArcSegment(
                        D2D1::Point2F(0, 400 - radiusXY), // end point
                        D2D1::SizeF(radius, radius),
                        0.0f, // rotation angle
                        D2D1_SWEEP_DIRECTION_CLOCKWISE,
                        D2D1_ARC_SIZE_SMALL
                ));

        //pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

        // 太阳光芒曲线
        //pSink->BeginFigure(
        //	D2D1::Point2F(299, 182),
        //	D2D1_FIGURE_BEGIN_HOLLOW
        //);
        //pSink->AddBezier(
        //	D2D1::BezierSegment(
        //		D2D1::Point2F(299, 182),
        //		D2D1::Point2F(294, 176),
        //		D2D1::Point2F(285, 178)
        //	));
        //pSink->AddBezier(
        //	D2D1::BezierSegment(
        //		D2D1::Point2F(276, 179),
        //		D2D1::Point2F(272, 173),
        //		D2D1::Point2F(272, 173)
        //	));

        //pSink->AddLine(D2D1::Point2F(130, 50));
        //pSink->AddLine(D2D1::Point2F(20, 130));
        //pSink->AddLine(D2D1::Point2F(80, 0));
        //pSink->AddLine(D2D1::Point2F(130, 130));

        pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

        pSink->Close();

        ID2D1Layer* pLayer = NULL;
        m_pRenderTarget->CreateLayer(NULL, &pLayer);

        m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(x, y));

        // Push the layer with the geometric mask.
        m_pRenderTarget->PushLayer(
                D2D1::LayerParameters(D2D1::InfiniteRect(), m_pPathGeometry),
                pLayer
        );
        //m_pRenderTarget->DrawBitmap(
        //	m_pBmp,
        //	D2D1::RectF(x, y, 400.0f, 400.0f),
        //	1.0f,
        //	D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
        //	D2D1::RectF(x, y, m_pBmp->GetSize().width, m_pBmp->GetSize().height)
        //);

        m_pRenderTarget->FillRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(0, 0, 400.0f, 400.0f), 0.0f, 0.0f),
                                              m_pBrush);

        m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::LightBlue),
                (ID2D1SolidColorBrush**) &m_pBrush
        );
        //m_pRenderTarget->DrawLine(D2D1::Point2F(270, 255), D2D1::Point2F(440, 255), m_pBrush);

        //m_pRenderTarget->FillRectangle(D2D1::RectF(0.f, 0.f, 25.f, 25.f), m_pBrush);
        //m_pRenderTarget->FillRectangle(D2D1::RectF(25.f, 25.f, 50.f, 50.f), m_pBrush);
        //m_pRenderTarget->FillRectangle(D2D1::RectF(50.f, 50.f, 75.f, 75.f), m_pBrush);
        //m_pRenderTarget->FillRectangle(D2D1::RectF(75.f, 75.f, 100.f, 100.f), m_pBrush);
        //m_pRenderTarget->FillRectangle(D2D1::RectF(100.f, 100.f, 125.f, 125.f), m_pBrush);
        //m_pRenderTarget->FillRectangle(D2D1::RectF(125.f, 125.f, 150.f, 150.f), m_pBrush);


        m_pRenderTarget->PopLayer();
    }

    void draw2() {

    }


    _PUBLICMETHODS_
public:
    Boolean Init(HWND hwnd) {
        HRESULT res = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pFactory);
        if (res != S_OK) return false;

        RECT rect;
        GetClientRect(hwnd, &rect);

        CoCreateInstance(CLSID_TF_ThreadMgr, nullptr, CLSCTX_INPROC_SERVER,
                         IID_ITfThreadMgr, (void**) &m_pThreadMgr);

        res = m_pFactory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(
                        hwnd, D2D1::SizeU(rect.right - rect.left, rect.bottom - rect.top)
                ),
                &m_pRenderTarget);

        //m_pFactory->CreatePathGeometry(&m_pPathGeometry);

        // 图片相关
        //CoCreateInstance(
        //	CLSID_WICImagingFactory,
        //	NULL,
        //	CLSCTX_INPROC_SERVER,
        //	IID_IWICImagingFactory,
        //	(LPVOID*)&m_pWicFactory);
    }

    VOID ClearScreen(float r, float g, float b) {
        m_pRenderTarget->Clear(D2D1::ColorF(r, g, b));
    }

    VOID BeginDraw() {
        m_pRenderTarget->BeginDraw();
    }

    VOID EndDraw() {
        m_pRenderTarget->EndDraw();
    }

    VOID BeginLayout() {
        m_pFactory->CreatePathGeometry(&m_pPathGeometry);
    }

    VOID ClipRoundRectRange() {
        Float radiusXY = 12;
        Float radius = 12;

        ID2D1GeometrySink* pSink;
        m_pPathGeometry->Open(&pSink);

        pSink->BeginFigure(D2D1::Point2F(0, radiusXY), D2D1_FIGURE_BEGIN_FILLED);
        // 太阳顶部圆弧
        pSink->AddArc(
                D2D1::ArcSegment(
                        D2D1::Point2F(radiusXY, 0), // end point
                        D2D1::SizeF(radius, radius),
                        0.0f, // rotation angle
                        D2D1_SWEEP_DIRECTION_CLOCKWISE,
                        D2D1_ARC_SIZE_SMALL
                ));

        pSink->AddLine(D2D1::Point2F(400 - radiusXY, 0));

        pSink->AddArc(
                D2D1::ArcSegment(
                        D2D1::Point2F(400, radiusXY), // end point
                        D2D1::SizeF(radius, radius),
                        0.0f, // rotation angle
                        D2D1_SWEEP_DIRECTION_CLOCKWISE,
                        D2D1_ARC_SIZE_SMALL
                ));

        pSink->AddLine(D2D1::Point2F(400, 400 - radiusXY));

        pSink->AddArc(
                D2D1::ArcSegment(
                        D2D1::Point2F(400 - radiusXY, 400), // end point
                        D2D1::SizeF(radius, radius),
                        0.0f, // rotation angle
                        D2D1_SWEEP_DIRECTION_CLOCKWISE,
                        D2D1_ARC_SIZE_SMALL
                ));

        pSink->AddLine(D2D1::Point2F(radiusXY, 400));

        pSink->AddArc(
                D2D1::ArcSegment(
                        D2D1::Point2F(0, 400 - radiusXY), // end point
                        D2D1::SizeF(radius, radius),
                        0.0f, // rotation angle
                        D2D1_SWEEP_DIRECTION_CLOCKWISE,
                        D2D1_ARC_SIZE_SMALL
                ));

        pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
        pSink->Close();

        pSink->Release();

        ID2D1Layer* pLayer = NULL;
        m_pRenderTarget->CreateLayer(NULL, &pLayer);

        // Push the layer with the geometric mask.
        m_pRenderTarget->PushLayer(
                D2D1::LayerParameters(D2D1::InfiniteRect(), m_pPathGeometry),
                pLayer
        );

        pLayer->Release();
    }

    VOID EndLayout() {
        m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(0, 0));

        m_pRenderTarget->PopLayer();
        m_pPathGeometry->Release();
    }

    VOID DrawTest() {
        IDWriteTextFormat* textFormat;
        ID2D1SolidColorBrush* brush;
        ID2D1SolidColorBrush* brush2;
        IDWriteTextLayout* textLayout;
        IDWriteFactory* m_pWriteFactory;

        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0, 1.0f), &brush);
        m_pRenderTarget->FillRectangle(D2D1::RectF(319.82, 0, 322.82, 21.1172), brush);

        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray), &brush2);
        m_pRenderTarget->FillRectangle(D2D1::RectF(0, 0, 400, 400), brush);
        brush->Release();

        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0, 0), &brush);

        D2D1_POINT_2F origin;
        origin.x = 0;
        origin.y = 0;

        DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(&m_pWriteFactory)
        );

        m_pWriteFactory->CreateTextFormat(
                L"Microsoft Yahei",
                nullptr,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                16.0f,
                L"en-us",
                &textFormat
        );

        m_pWriteFactory->CreateTextLayout(
                L"测试专用文本abcdefghijklmnopqrstuvwxyz",
                wcslen(L"测试专用文本abcdefghijklmnopqrstuvwxyz"),
                textFormat,
                32,
                16,
                &textLayout
        );

        textLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

        if (g_flag) {
            //DWRITE_TEXT_METRICS textMetrics{};
            //textLayout->GetMetrics(&textMetrics);
            //
            //cout << textMetrics.widthIncludingTrailingWhitespace << " : " << textMetrics.height << endl;
            //cout << textMetrics.width << " : " << textMetrics.height << endl;
            //cout << textMetrics.layoutWidth << " : " << textMetrics.layoutHeight << endl;
            //cout << textMetrics.left << " : " << textMetrics.top << endl;
            //cout << textMetrics.lineCount << " : " << textMetrics.maxBidiReorderingDepth << endl;

            DWRITE_HIT_TEST_METRICS hitTestMetrics{};
            FLOAT x;
            FLOAT y;
            textLayout->HitTestTextPosition(
                    31,
                    true,
                    &x,
                    &y,
                    &hitTestMetrics
            );

            cout << "x" << x << endl;
            cout << "y" << y << endl;
            cout << "width" << hitTestMetrics.width << endl;
            cout << "left" << hitTestMetrics.left << endl;
            cout << "height" << hitTestMetrics.height << endl;
            cout << "length" << hitTestMetrics.length << endl;
            cout << "bidiLevel" << hitTestMetrics.bidiLevel << endl;
            cout << "isText" << hitTestMetrics.isText << endl;
            cout << "isTrimmed" << hitTestMetrics.isTrimmed << endl;
            cout << "textPosition" << hitTestMetrics.textPosition << endl;
            cout << "top" << hitTestMetrics.top << endl;
            cout << "=========================================" << endl;

            hitTestMetrics = {};

            FLOAT x1 = -77;
            FLOAT y1 = -77;
            BOOL isTrailingHit;
            BOOL isInside;
            textLayout->HitTestPoint(
                    x1,
                    y1,
                    &isTrailingHit,
                    &isInside,
                    &hitTestMetrics
            );

            cout << "isTrailingHit" << isTrailingHit << endl;
            cout << "isInside" << isInside << endl;
            cout << "x" << x1 << endl;
            cout << "y" << y1 << endl;
            cout << "width" << hitTestMetrics.width << endl;
            cout << "left" << hitTestMetrics.left << endl;
            cout << "height" << hitTestMetrics.height << endl;
            cout << "length" << hitTestMetrics.length << endl;
            cout << "bidiLevel" << hitTestMetrics.bidiLevel << endl;
            cout << "isText" << hitTestMetrics.isText << endl;
            cout << "isTrimmed" << hitTestMetrics.isTrimmed << endl;
            cout << "textPosition" << hitTestMetrics.textPosition << endl;
            cout << "top" << hitTestMetrics.top << endl;
            cout << "=========================================" << endl;

            DWRITE_HIT_TEST_METRICS hitTestMetrics1{};
            UINT32 actualHitTestMetricsCount;
            textLayout->HitTestTextRange(
                    0,
                    2,
                    0,
                    0,
                    &hitTestMetrics1,
                    32,
                    &actualHitTestMetricsCount
            );

            cout << "width" << hitTestMetrics1.width << endl;
            cout << "left" << hitTestMetrics1.left << endl;
            cout << "height" << hitTestMetrics1.height << endl;
            cout << "length" << hitTestMetrics1.length << endl;
            cout << "bidiLevel" << hitTestMetrics1.bidiLevel << endl;
            cout << "isText" << hitTestMetrics1.isText << endl;
            cout << "isTrimmed" << hitTestMetrics1.isTrimmed << endl;
            cout << "textPosition" << hitTestMetrics1.textPosition << endl;
            cout << "top" << hitTestMetrics1.top << endl;
            cout << "=========================================" << endl;


            g_flag = false;
        }

        m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(D2D1::SizeF(-100, -100)));

        m_pRenderTarget->DrawTextLayout(origin, textLayout, brush);

        brush->Release();
        textFormat->Release();
        textLayout->Release();
        m_pWriteFactory->Release();
    }

    VOID SetSolidBrushColor(Float r, Float g, Float b, Float a = 1.0f) {
        m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(r, g, b), (ID2D1SolidColorBrush**) &m_pBrush);
    }

    VOID SetLinearGradientBrushColor(D2D1_GRADIENT_STOP gradientStops[], D2D1_POINT_2F start, D2D1_POINT_2F end) {
        m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

        ID2D1GradientStopCollection* pGradientStops = NULL;

        m_pRenderTarget->CreateGradientStopCollection(
                gradientStops,
                sizeof(gradientStops),
                D2D1_GAMMA_2_2,
                D2D1_EXTEND_MODE_CLAMP,
                &pGradientStops
        );

        m_pRenderTarget->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                        start,
                        end),
                pGradientStops,
                (ID2D1LinearGradientBrush**) &m_pBrush);
    }

    VOID SetRadialGradientBrushColor(D2D1_GRADIENT_STOP gradientStops[], D2D1_POINT_2F center, D2D1_POINT_2F off,
                                     Float radius) {
        m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

        ID2D1GradientStopCollection* pGradientStops = NULL;
        m_pRenderTarget->CreateGradientStopCollection(
                gradientStops,
                sizeof(gradientStops),
                D2D1_GAMMA_2_2,
                D2D1_EXTEND_MODE_CLAMP,
                &pGradientStops
        );

        m_pRenderTarget->CreateRadialGradientBrush(
                D2D1::RadialGradientBrushProperties(
                        center,
                        off,
                        radius,
                        radius),
                pGradientStops,
                (ID2D1RadialGradientBrush**) &m_pBrush);
    }

    VOID DrawImage(LPCWSTR fileName, D2D1_RECT_F rect) {
        m_pWicFactory->CreateDecoderFromFilename(
                fileName,
                NULL,
                GENERIC_READ,
                WICDecodeMetadataCacheOnLoad,
                &m_pWicDecoder
        );

        m_pWicDecoder->GetFrame(0, &m_pWicFrame);

        m_pWicFactory->CreateFormatConverter(&m_pWicConverter);

        m_pWicConverter->Initialize(
                m_pWicFrame,
                GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone,
                NULL,
                0.0,
                WICBitmapPaletteTypeCustom
        );

        m_pRenderTarget->CreateBitmapFromWicBitmap(m_pWicConverter, NULL, &m_pBmp);

        m_pRenderTarget->DrawBitmap(
                m_pBmp,
                rect,
                1.0f,
                D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
                D2D1::RectF(0.0f, 0.0f, m_pBmp->GetSize().width, m_pBmp->GetSize().height)
        );
    }

#pragma endregion

#pragma region G2

    void Init() {
        HRESULT hr = CoCreateInstance(CLSID_TF_ThreadMgr, nullptr, CLSCTX_INPROC_SERVER, __uuidof(ITfThreadMgr),
                                      (void**) &m_tm);
        m_TsfSink = new CUIElementSink();
        //m_tm->Activate(&ptid);
        //
        //ITfContext* ctx;
        //ITfDocumentMgr* doc_mgr;
        //hr = m_tm->GetFocus(&doc_mgr);
        //
        //m_tm->SetFocus(doc_mgr);
        //doc_mgr->CreateContext(ptid, 0, nullptr, &ctx, &m_dwCookie1);

        //hr = doc_mgr->GetBase(&ctx);
        //
        //ITfContextView* pView;
        //ctx->GetActiveView(&pView);
        //pView->GetWnd(&g_edit);

        ITfSource* src3;
        m_tm->QueryInterface(IID_ITfSource, (void**) &src3);
        src3->AdviseSink(__uuidof(ITfUIElementSink), (ITfUIElementSink*) m_TsfSink, &m_dwCookie3);

        //src3->AdviseSink(__uuidof(ITfContextOwner), (ITfContextOwner*) m_TsfSink,
        //                 &m_dwCookie3);

        //ITfSource* src2;
        //hr = m_tm->QueryInterface(IID_ITfSource, (void**) &src2);
        //src2->AdviseSink(__uuidof(ITfUIElementSink), (ITfUIElementSink*) m_TsfSink, &m_dwCookie3);
        //src2->AdviseSink(__uuidof(ITfContextKeyEventSink), (ITfContextKeyEventSink*) m_TsfSink, &m_dwCookie4);

        //src2->AdviseSink(__uuidof(ITfInputProcessorProfileActivationSink),
        //                (ITfInputProcessorProfileActivationSink*) m_TsfSink, &m_dwCookie4);

        //src2->AdviseSink(__uuidof(ITfThreadMgrEventSink), (ITfThreadMgrEventSink*) m_TsfSink, &m_dwCookie6);


        //src2->AdviseSink(__uuidof(ITfActiveLanguageProfileNotifySink), (ITfActiveLanguageProfileNotifySink*) m_TsfSink,
        //                 &m_dwCookie2);

        //ITfCompartmentMgr* pcm;
        //m_tm->QueryInterface(IID_ITfCompartmentMgr, (void**) &pcm);
        //ITfCompartment* pTfOpenMode;
        //ITfCompartment* pTfConvMode;
        //pcm->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pTfOpenMode);
        //pcm->GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &pTfConvMode);

        //ITfSource* srcOpenMode;
        //pTfOpenMode->QueryInterface(IID_ITfSource, (void**) &srcOpenMode);
        //srcOpenMode->AdviseSink(IID_ITfCompartmentEventSink,
        //                        (ITfCompartmentEventSink*)m_TsfSink,
        //                        &m_dwOpenModeSinkCookie);
        //srcOpenMode->Release();
        //
        //ITfSource* srcConvMode;
        //pTfConvMode->QueryInterface(IID_ITfSource, (void**) &srcConvMode);

        //srcConvMode->AdviseSink(__uuidof(ITfCompartmentEventSink), (ITfCompartmentEventSink*) m_TsfSink, &m_dwCookie5);
        //srcConvMode->Release();

        ITfSource* src;
        //hr = ctx->QueryInterface(IID_ITfSource, (void**) &src);
        ITfDocumentMgr* documentMgr;
        m_pThreadMgr->GetFocus(&documentMgr);
        ITfContext* context;
        documentMgr->GetBase(&context);
        context->QueryInterface(IID_ITfSource, (void**) &src);
        hr = src->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink*) m_TsfSink, &m_dwTextEditCookie);

        ITfInputProcessorProfiles* profiles;
        CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfiles,
                         (LPVOID*) &profiles);
        ITfSource* source;
        profiles->QueryInterface(IID_ITfSource, (LPVOID*) &source);

        //source->AdviseSink(__uuidof(ITfLanguageProfileNotifySink), (ITfLanguageProfileNotifySink*) m_TsfSink,
        //                &m_dwCookie1);


        source->Release();
        //src->Release();
        //ctx->Release();
        //doc_mgr->Release();
    }

    LPCWSTR name = L"微软拼音";

    void DrawInput() {
        ITfInputProcessorProfiles* pProfiles;
        CoCreateInstance(CLSID_TF_InputProcessorProfiles,
                         nullptr,
                         CLSCTX_INPROC_SERVER,
                         IID_ITfInputProcessorProfiles,
                         (LPVOID*) &pProfiles);

        IEnumTfLanguageProfiles* pEnumProf;
        //PunCha：枚举所有输入法
        pProfiles->EnumLanguageProfiles(0x804, &pEnumProf);
        TF_LANGUAGEPROFILE proArr[2];
        ULONG feOut = 0;


        //PunCha：其实proArr这里应该写成 &proArr[0]，因为里面只需要一个TF_LANGUAGEPROFILE变量！而且，proArr[1]都没用到过！
        while (S_OK == pEnumProf->Next(1, proArr, &feOut)) {
            //PunCha：获取他的名字
            BSTR bstrDest;
            pProfiles->GetLanguageProfileDescription(proArr[0].clsid, 0x804, proArr[0].guidProfile, &bstrDest);

            BOOL bEnable = false;
            pProfiles->IsEnabledLanguageProfile(proArr[0].clsid, 0x804, proArr[0].guidProfile, &bEnable);


            if (wcscmp(bstrDest, name) == 0) {
                HRESULT hr = pProfiles->ActivateLanguageProfile(proArr[0].clsid, 0x804, proArr[0].guidProfile);
                cout << (hr == S_OK) << endl;
                WCout(bstrDest);
            }

            SysFreeString(bstrDest);
        }

        wcscmp(L"微软五笔", name) == 0 ? name = L"微软拼音" : name = L"微软五笔";

        pProfiles->Release();
    }

#pragma endregion G2


    _MEMBERVARIABLE_
private:
    ITfDocumentMgr* m_pdim;
    ITfThreadMgr* m_pThreadMgr;

    ID2D1Factory* m_pFactory;

    ID2D1HwndRenderTarget* m_pRenderTarget;
    ID2D1Brush* m_pBrush;

    IWICImagingFactory* m_pWicFactory;
    ID2D1Bitmap* m_pBmp;
    IWICBitmapDecoder* m_pWicDecoder;
    IWICBitmapFrameDecode* m_pWicFrame;
    IWICFormatConverter* m_pWicConverter;

    ID2D1PathGeometry* m_pPathGeometry;
    ID2D1GeometrySink* m_pSink;
};

// 全局变量:
HINSTANCE hInst;                                // 当前实例
static string title = "test";
static LPCSTR szTitle = "test";                  // 标题栏文本
static LPCSTR szWindowClass = "testClass";            // 主窗口类名
static Graphics* graphics = new Graphics();

// 此代码模块中包含的函数的前向声明:
ATOM MyRegisterClass(HINSTANCE hInstance);

BOOL InitInstance(HINSTANCE, int);

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass() {
    WNDCLASSA wcex;

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandleA(nullptr);
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;

    return RegisterClassA(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
static HWND g_hwnd;
HFONT g_hfont = 0;

BOOL InitInstance() {
    sztitle[0] = '\n';
    sztitle2[0] = '\n';

    HWND hWnd = CreateWindowA(szWindowClass, title.c_str(), WS_OVERLAPPEDWINDOW,
                              400, 400, 720, 480, nullptr, nullptr, GetModuleHandleA(nullptr), nullptr);

    if (!hWnd) {
        return FALSE;
    }

    //HWND edit = CreateWindowA("edit", "", WS_CHILD | WS_VISIBLE | ES_MULTILINE,
    //                          10, 10, 200, 200, hWnd, nullptr, GetModuleHandleA(nullptr), nullptr);


    g_hwnd = hWnd;

    //SetFocus(edit);
    SetFocus(hWnd);

    graphics->Init(hWnd);
    //graphics->Init();

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);


    return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
static std::queue<BYTE> s_isUniChar;
static BYTE s_lobyte;
static BYTE s_hibyte;
static BSTR s_ch;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            break;
        case WM_COMMAND:
            if (HIWORD(wParam) == EN_CHANGE) {
            }
            break;
        case WM_ERASEBKGND:
            return FALSE;
        case WM_LBUTTONDOWN: {
            //CreateCaret(g_hwnd, nullptr, 16, 16);
            ////SetCaretBlinkTime(1000);
            //SetCaretPos(LOWORD(lParam), HIWORD(lParam));
            //ShowCaret(g_hwnd);
            break;
        }
        case WM_IME_CHAR: {
            vector<unsigned char> vec;
            vec.push_back((unsigned char) (wParam >> 8));
            vec.push_back((unsigned char) wParam);
            vec.push_back('\0');
            vec.resize(3);

            char* ss = (char*) &(vec[0]);

            int n = MultiByteToWideChar(CP_ACP, 0, ss, -1, nullptr, 0);

            WCHAR* str1 = new WCHAR[n + 1];
            str1[n] = L'\0';

            MultiByteToWideChar(CP_ACP, 0, ss, -1, str1, n);

            cout << __LINE__ << " : " << (int) (unsigned char) (wParam >> 8) << endl;
            cout << __LINE__ << " : " << (int) (unsigned char) wParam << endl;
            cout << __LINE__ << " : " << wParam << endl;
            //cout << __LINE__ << " : " << GBKToUTF8(ss) << endl;

            WCout(str1);
        }
            break;
            //case WM_IME_NOTIFY:
            //    if (wParam == IMC_SETSTATUSWINDOWPOS) {
            //        cout << lParam << endl;
            //    }
            //    break;
        case WM_CHAR: {
            title.push_back((char) wParam);
            SetWindowTextA(hWnd, title.c_str());
        }
            //wcout << __LINE__ << " : " << (wchar_t )wParam << endl;
            //cout << __LINE__ << " : " << (char)wParam << endl;
            break;
        case WM_IME_SETCONTEXT:
            cout << __LINE__ << " : " << wParam << endl;
            break;
            //case WM_INPUTLANGCHANGE:
            //    cout << __LINE__ << " : " << wParam << endl;
            //    break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int win() {
    system("chcp 65001");

    // 初始化全局字符串
    MyRegisterClass();

    // 执行应用程序初始化:
    if (!InitInstance()) {
        return FALSE;
    }

    MSG msg;

    static int s = 0;
    static int i = 0;
    static double fps = 0;

    // 主消息循环:
    auto prev = std::chrono::high_resolution_clock::now();
    //while (true) {
    //    if (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
    //        if (msg.message == WM_QUIT)
    //            break;
    //
    //        TranslateMessage(&msg);
    //        DispatchMessageA(&msg);
    //    } else {
    //        auto cur = std::chrono::high_resolution_clock::now();
    //        auto diff = std::chrono::duration<double, std::milli>(cur - prev);
    //        if (diff.count() >= 200) {
    //            prev = cur;
    //            fps = (i - s) / 200;
    //            s = i;
    //            std::stringstream ss;
    //            ss << "fps: ";
    //            ss << std::setiosflags(std::ios::fixed) << std::setprecision(2) << fps;
    //            SendMessageA(g_hwnd, WM_SETTEXT, 0, (LPARAM) (ss.str().c_str()));
    //        }
    //        ++i;
    //
    //
    //        graphics->BeginDraw();
    //        graphics->ClearScreen(1, 1, 1);
    //        graphics->DrawTest();
    //        graphics->EndDraw();
    //    }
    //}


    while (GetMessageA(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return (int) msg.wParam;
}

#include <io.h>

struct iidd {
    unsigned char a[8];
    unsigned char b[16];
};

#define TEST(ARGS, ...) a##ARGS

struct AAA {
    unsigned char Data1[4];
    unsigned char Data2[4];

    bool operator==(const AAA& r) {
        int ret = 0;
        for (int i = 0; i < 4; ++i) {
            if (Data1[i] == r.Data1[i]) {
                ++ret;
            } else {
                break;
            }

            if (Data2[i] == r.Data2[i]) {
                ++ret;
            } else {
                break;
            }
        }

        return ret == 8;
    }
};

#define INVALID AAA {0,0}

#include <fstream>

int main() {
#pragma region Test
    //win();

    //LoadLibraryA("F:\\c++projects\\Dll1\\Dll1\\Debug\\Dll1.dll");

    //
    //char rootPath[MAX_PATH];
    //string path;
    //GetModuleFileNameA(nullptr, rootPath, MAX_PATH);
    //path = rootPath;
    //int pos = path.find_last_of('\\', path.length());
    //cout << path.substr(0, pos) << endl;


    //_finddata64i32_t fileInfo{};
    //
    //long h = 0;
    //h = _findfirst64i32("D:\\env\\*", &fileInfo);
    //if (h != -1) {
    //    do {
    //        cout << fileInfo.name << endl;
    //    } while (_findnext64i32(h, &fileInfo) == 0);
    //    _findclose(h);
    //}

    //string str1 = "abc\\";
    //string str2 = "abc/";
    //
    //int pos1 = str1.find_last_of('\\');
    //int pos2 = str2.find_last_of('\\');
    //int pos3 = str1.find_last_of('/');
    //int pos4 = str2.find_last_of('/');
    //
    //cout << pos1 << " : " << pos2 << endl;
    //cout << pos3 << " : " << pos4 << endl;
#pragma endregion
    //unsigned char a[8] = {1, 2, 3, 4};
    //unsigned char b[16] = {1, 2, 3, 4};
    //iidd d{};
    //memcpy(d.a, a, 8);
    //memcpy(d.b, b, 16);
    //
    //unsigned char aa[24];
    //memset(aa, '\0', 24);
    //memcpy(aa, &d, sizeof(d));
    ////memcpy(&aa[8], b, 16);
    //
    //aa;
    //d;

    //ofstream ofs;
    //ofs.open("C:\\Users\\12086\\Desktop\\test.txt");
    //
    //ofs.clear();
    //
    //ofs.close();
    char cs[125];
    cs;
}

