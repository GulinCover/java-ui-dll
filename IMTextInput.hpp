#pragma once

#include "IMCommon.hpp"
#include "IMFrame.hpp"

namespace MWD::TextInput {
    using namespace MWD::Frame;
    using namespace MWD::GM;

    /********************************************************
     获取输入法类容
    *********************************************************/
    class IMeProxy : public ITfUIElementSink,
                     public ITfTextEditSink {
    public:
        IMeProxy() : m_ref(0) {

        }

        /// <summary>
        /// IUnknown
        /// </summary>
    public:
        HRESULT STDMETHODCALLTYPE QueryInterface(
                /* [in] */ REFIID riid,
                /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObj) override {
            if (ppvObj == nullptr)
                return E_INVALIDARG;

            *ppvObj = nullptr;

            if (IsEqualIID(riid, IID_IUnknown)) {
                *ppvObj = reinterpret_cast<IUnknown*>(this);
            } else if (IsEqualIID(riid, __uuidof(ITfUIElementSink))) {
                // 输入法候选列表更新时回调
                *ppvObj = (ITfUIElementSink*) this;
            }
            //else if (IsEqualIID(riid, __uuidof(ITfInputProcessorProfileActivationSink)))
            //{
            //    // 输入法激活时回调
            //    *ppvObj = (ITfInputProcessorProfileActivationSink*)this;
            //}
            //else if (IsEqualIID(riid, __uuidof(ITfCompartmentEventSink)))
            //{
            //    // 输入法中/英状态更改时回调
            //    *ppvObj = (ITfCompartmentEventSink*)this;
            //}
            else if (IsEqualIID(riid, IID_ITfTextEditSink)) {
                *ppvObj = (ITfTextEditSink*)this;
            }

            if (*ppvObj) {
                AddRef();
                return S_OK;
            }

            return E_NOINTERFACE;
        }

        ULONG STDMETHODCALLTYPE AddRef() override {
            return ++m_ref;
        }

        ULONG STDMETHODCALLTYPE Release() override {
            MULong ref = --m_ref;
            if (m_ref == 0) {
                delete this;
            }
            return ref;
        }


        /// <summary>
        /// ITfUIElementSink
        /// </summary>
    public:
        HRESULT STDMETHODCALLTYPE BeginUIElement(
                /* [in] */ DWORD dwUIElementId,
                /* [out][in] */ BOOL* pbShow) override {
            *pbShow = true;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE UpdateUIElement(
                /* [in] */ DWORD dwUIElementId) override {
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE EndUIElement(
                /* [in] */ DWORD dwUIElementId) override {
            return S_OK;
        }

        /// <summary>
        /// ITfTextEditSink
        /// </summary>
    public:
        HRESULT STDMETHODCALLTYPE OnEndEdit(
                /* [in] */ __RPC__in_opt ITfContext* pic,
                /* [in] */ TfEditCookie ecReadOnly,
                /* [in] */ __RPC__in_opt ITfEditRecord* pEditRecord) override {
            CComPtr<ITfContextComposition> pctxcomp;
            if (SUCCEEDED(pic->QueryInterface(IID_ITfContextComposition, (void**) &pctxcomp))) {
                CComPtr<IEnumITfCompositionView> enum_view;
                if (SUCCEEDED(pctxcomp->EnumCompositions(&enum_view))) {
                    CComPtr<ITfRange> range;
                    ITfCompositionView* view;
                    while (enum_view->Next(1, &view, nullptr) == S_OK) {
                        CComPtr<ITfRange> prange;
                        if (view->GetRange(&prange) == S_OK) {
                            if (!range) {
                                prange->Clone(&range);
                            } else {
                                range->ShiftEndToRange(ecReadOnly, prange, TF_ANCHOR_END);
                            }
                        }
                        view->Release();
                    }

                    wmemset(m_composing, '\0', 256);
                    MULong len = 255;
                    if (range)
                        range->GetText(ecReadOnly, 0, m_composing, len + 1, &len);
                    m_composing[min(len, 255)] = L'\0';

                    auto focusManager = IMFocusManager::GetInstance();
                    std::shared_ptr<IMFrame> f = focusManager->GetFocusFrame("route1");

                    f->font->primaryText = m_composing;
                }
            }
            return S_OK;
        }

    private:
        MULong m_ref;
        wchar_t m_composing[256];
    };

    /********************************************************
     文本输入接口
    *********************************************************/
    class IMEditorProxy {
    public:
        virtual ~IMEditorProxy() = default;

    public:
        virtual void Init(HWND) = 0;

        virtual void MoveInputPos(MDouble x, MDouble y) = 0;

        virtual void ChangeFrame(const std::shared_ptr<IMFrame>& frame) = 0;

        virtual void SetEditorFocus() = 0;

        virtual void KillEditorFocus() = 0;
    };

    /********************************************************
     文本输入接口impl
    *********************************************************/
    class IMTextInput : public IMEditorProxy {
    public:
        ~IMTextInput() override {
            DestroyCaret();
        }

    public:
        void Init(HWND hwnd) override {
            m_hwnd = hwnd;
        }

        void MoveInputPos(MDouble x, MDouble y) override {
            SetCaretPos(x, y);
        }

        void ChangeFrame(const std::shared_ptr<IMFrame>& frame) override {
            if (m_frame) {
                DestroyCaret();
            }

            CreateCaret(m_hwnd, nullptr, 2, frame->font->fontSize);
            m_frame = frame;
        }

        void SetEditorFocus() override {
            //ShowCaret(m_hwnd);
        }

        void KillEditorFocus() override {
            HideCaret(m_hwnd);
        }

    private:
        HWND m_hwnd;
        std::shared_ptr<IMFrame> m_frame;
    };

    /********************************************************
     文本输入管理器
    *********************************************************/
    class IMEditorManager {
    public:
        ~IMEditorManager() {
            if (m_source) {
                m_source->UnadviseSink(m_UIElementCookie);
                m_contextSource->UnadviseSink(m_TextEditCookie);
            }
        }

    public:
        void Init(HWND hwnd) {
            m_editor->Init(hwnd);
            m_ime = CComPtr<IMeProxy>(new IMeProxy);

            SetFocus(hwnd);

            HRESULT hr = CoCreateInstance(CLSID_TF_ThreadMgr, nullptr, CLSCTX_INPROC_SERVER,
                                          IID_PPV_ARGS(&m_threadMgr));

            if (SUCCEEDED(hr)) {
                hr = m_threadMgr->QueryInterface(IID_ITfSource, (void**) &m_source);
                if (SUCCEEDED(hr)) {
                    m_source->AdviseSink(__uuidof(ITfUIElementSink), (ITfUIElementSink*)m_ime, &m_UIElementCookie);

                    CComPtr<ITfDocumentMgr> documentMgr;
                    if (SUCCEEDED(m_threadMgr->GetFocus(&documentMgr))) {
                        CComPtr<ITfContext> context;
                        if (SUCCEEDED(documentMgr->GetBase(&context))) {
                            if (SUCCEEDED(context->QueryInterface(IID_ITfSource, (void**) &m_contextSource))) {
                                m_contextSource->AdviseSink(__uuidof(ITfTextEditSink), (ITfTextEditSink*)m_ime, &m_TextEditCookie);
                            }
                        }
                    }
                }
            }
        }

        void SetEditorFocus(MBoolean isFocus) {
            isFocus ? m_editor->SetEditorFocus() : m_editor->KillEditorFocus();
        }

        void ChangeFrame(const std::shared_ptr<IMFrame>& frame) {
            m_editor->ChangeFrame(frame);
        }

        void UpdateEditorPos(MDouble x, MDouble y) {
            m_editor->MoveInputPos(x, y);
        }

        void SetEditor(const std::shared_ptr<IMEditorProxy>& editor) {
            m_editor = editor;
        }

    private:
        std::shared_ptr<IMEditorProxy> m_editor;
        CComPtr<IMeProxy> m_ime;
        CComPtr<ITfThreadMgr> m_threadMgr;
        CComPtr<ITfSource> m_source;
        CComPtr<ITfSource > m_contextSource;
        DWORD m_UIElementCookie;
        DWORD m_TextEditCookie;
    };

}
