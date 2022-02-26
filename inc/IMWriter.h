#pragma once

#include "IMCommon.h"

namespace MWD::Writer {
    //////////////////////////////////////////////////
    // 文本渲染类
    abstract IMWriter : public IDWriteTextRenderer {
    public:
        IMWriter() : m_cRef(0) {}
        virtual ~IMWriter() = default;
    public:
        COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE DrawGlyphRun(
                _In_opt_ void* clientDrawingContext,
                FLOAT baselineOriginX,
                FLOAT baselineOriginY,
                DWRITE_MEASURING_MODE measuringMode,
                _In_ DWRITE_GLYPH_RUN const* glyphRun,
                _In_ DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
                _In_opt_ IUnknown* clientDrawingEffect
        ) override;

        COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE DrawInlineObject(
                _In_opt_ void* clientDrawingContext,
                FLOAT originX,
                FLOAT originY,
                _In_ IDWriteInlineObject* inlineObject,
                BOOL isSideways,
                BOOL isRightToLeft,
                _In_opt_ IUnknown* clientDrawingEffect
        ) override;

        COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE DrawStrikethrough(
                _In_opt_ void* clientDrawingContext,
                FLOAT baselineOriginX,
                FLOAT baselineOriginY,
                _In_ DWRITE_STRIKETHROUGH const* strikethrough,
                _In_opt_ IUnknown* clientDrawingEffect
        ) override;

        COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE DrawUnderline(
                _In_opt_ void* clientDrawingContext,
                FLOAT baselineOriginX,
                FLOAT baselineOriginY,
                _In_ DWRITE_UNDERLINE const* underline,
                _In_opt_ IUnknown* clientDrawingEffect
        ) override;

        COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetPixelsPerDip(
                _In_opt_ void* clientDrawingContext,
                _Out_ FLOAT* pixelsPerDip
        ) override;

        COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetCurrentTransform(
                _In_opt_ void* clientDrawingContext,
                _Out_ DWRITE_MATRIX* transform
        ) override;

        COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE IsPixelSnappingDisabled(
                _In_opt_ void* clientDrawingContext,
                _Out_ BOOL* isDisabled
        ) override;

        HRESULT STDMETHODCALLTYPE QueryInterface(
                /* [in] */ REFIID riid,
                /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;

        ULONG STDMETHODCALLTYPE AddRef() override;

        ULONG STDMETHODCALLTYPE Release() override;

    public:
        virtual void Init(const CComPtr<ID2D1Factory>& factory, const CComPtr<ID2D1HwndRenderTarget>& RT);

        virtual void SetFontWidth(MFloat stroke);

        virtual void SetFillBrush(CComPtr<ID2D1Brush> brush);

        virtual void SetOutlineBrush(CComPtr<ID2D1Brush> brush);

        virtual void SetUnderlineBrush(CComPtr<ID2D1Brush> brush);

        virtual void SetStrikethroughBrush(CComPtr<ID2D1Brush> brush);

    private:
        MULong m_cRef;
        CComPtr<ID2D1Factory> m_pD2DFactory;
        CComPtr<ID2D1RenderTarget> m_pRT;
        CComPtr<ID2D1Brush> m_pOutlineBrush;
        CComPtr<ID2D1Brush> m_pFillBrush;

        MFloat m_stroke = 0;

        CComPtr<ID2D1Brush> m_pUnderlineBrush;
        CComPtr<ID2D1Brush> m_pStrikethroughBrush;
        MFloat m_underlineWidth = 0;
        MFloat m_strikethroughWidth = 0;
        MFloat m_weight = 400;
    };
}
