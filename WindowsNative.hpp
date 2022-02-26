#pragma once

#include <utility>

#include "IMCommon.hpp"

namespace MWD::WindowsNative {
    /********************************************************
     文本渲染类
    *********************************************************/
    class IMWriter : public IDWriteTextRenderer {
    public:
        IMWriter() : m_cRef(0) {}
    public:
        COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE DrawGlyphRun(
                _In_opt_ void* clientDrawingContext,
                FLOAT baselineOriginX,
                FLOAT baselineOriginY,
                DWRITE_MEASURING_MODE measuringMode,
                _In_ DWRITE_GLYPH_RUN const* glyphRun,
                _In_ DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
                _In_opt_ IUnknown* clientDrawingEffect
        ) override {
            CComPtr<ID2D1PathGeometry> pPathGeometry;
            m_pD2DFactory->CreatePathGeometry(
                    &pPathGeometry
            );

            CComPtr<ID2D1GeometrySink> pSink;
            pPathGeometry->Open(
                    &pSink
            );

            glyphRun->fontFace->GetGlyphRunOutline(
                    glyphRun->fontEmSize,
                    glyphRun->glyphIndices,
                    glyphRun->glyphAdvances,
                    glyphRun->glyphOffsets,
                    glyphRun->glyphCount,
                    glyphRun->isSideways,
                    glyphRun->bidiLevel % 2,
                    pSink
            );

            pSink->Close();

            D2D1::Matrix3x2F const matrix = D2D1::Matrix3x2F(
                    1.0f, 0.0f,
                    0.0f, 1.0f,
                    baselineOriginX, baselineOriginY
            );

            CComPtr<ID2D1TransformedGeometry> pTransformedGeometry;
            m_pD2DFactory->CreateTransformedGeometry(
                    pPathGeometry,
                    &matrix,
                    &pTransformedGeometry
            );

            m_pRT->DrawGeometry(
                    pTransformedGeometry,
                    m_pOutlineBrush,
                    m_stroke == 0 ? 0.1f : m_stroke
            );

            //Fill in the glyph run
            m_pRT->FillGeometry(
                    pTransformedGeometry,
                    m_pFillBrush
            );
            return S_OK;
        }

        COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE DrawInlineObject(
                _In_opt_ void* clientDrawingContext,
                FLOAT originX,
                FLOAT originY,
                _In_ IDWriteInlineObject* inlineObject,
                BOOL isSideways,
                BOOL isRightToLeft,
                _In_opt_ IUnknown* clientDrawingEffect
        ) override {
            return E_NOTIMPL;
        }

        COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE DrawStrikethrough(
                _In_opt_ void* clientDrawingContext,
                FLOAT baselineOriginX,
                FLOAT baselineOriginY,
                _In_ DWRITE_STRIKETHROUGH const* strikethrough,
                _In_opt_ IUnknown* clientDrawingEffect
        ) override {
            D2D1_RECT_F rect = D2D1::RectF(
                    0,
                    strikethrough->offset,
                    m_strikethroughWidth == 0 ? strikethrough->width : m_strikethroughWidth,
                    strikethrough->offset + strikethrough->thickness
            );

            CComPtr<ID2D1RectangleGeometry> pRectangleGeometry = nullptr;
            m_pD2DFactory->CreateRectangleGeometry(
                    &rect,
                    &pRectangleGeometry
            );

            D2D1::Matrix3x2F const matrix = D2D1::Matrix3x2F(
                    1.0f, 0.0f,
                    0.0f, 1.0f,
                    baselineOriginX, baselineOriginY
            );

            CComPtr<ID2D1TransformedGeometry> pTransformedGeometry = nullptr;
            m_pD2DFactory->CreateTransformedGeometry(
                    pRectangleGeometry,
                    &matrix,
                    &pTransformedGeometry
            );

            m_pRT->DrawGeometry(
                    pTransformedGeometry,
                    m_pStrikethroughBrush
            );

            // Fill in the glyph run
            m_pRT->FillGeometry(
                    pTransformedGeometry,
                    m_pStrikethroughBrush
            );

            return S_OK;
        }

        COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE DrawUnderline(
                _In_opt_ void* clientDrawingContext,
                FLOAT baselineOriginX,
                FLOAT baselineOriginY,
                _In_ DWRITE_UNDERLINE const* underline,
                _In_opt_ IUnknown* clientDrawingEffect
        ) override {
            D2D1_RECT_F rect = D2D1::RectF(
                    0,
                    underline->offset,
                    m_underlineWidth == 0 ? underline->width : m_underlineWidth,
                    underline->offset + underline->thickness
            );

            CComPtr<ID2D1RectangleGeometry> pRectangleGeometry = nullptr;
            m_pD2DFactory->CreateRectangleGeometry(
                    &rect,
                    &pRectangleGeometry
            );

            D2D1::Matrix3x2F const matrix = D2D1::Matrix3x2F(
                    1.0f, 0.0f,
                    0.0f, 1.0f,
                    baselineOriginX, baselineOriginY
            );

            CComPtr<ID2D1TransformedGeometry> pTransformedGeometry = nullptr;
            m_pD2DFactory->CreateTransformedGeometry(
                    pRectangleGeometry,
                    &matrix,
                    &pTransformedGeometry
            );

            m_pRT->DrawGeometry(
                    pTransformedGeometry,
                    m_pUnderlineBrush
            );

            // Fill in the glyph run
            m_pRT->FillGeometry(
                    pTransformedGeometry,
                    m_pUnderlineBrush
            );

            return S_OK;
        }

        COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetPixelsPerDip(
                _In_opt_ void* clientDrawingContext,
                _Out_ FLOAT* pixelsPerDip
        ) override {
            MFloat x, y;

            m_pRT->GetDpi(&x, &y);
            *pixelsPerDip = x / 96;
            return S_OK;
        }

        COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetCurrentTransform(
                _In_opt_ void* clientDrawingContext,
                _Out_ DWRITE_MATRIX* transform
        ) override {
            m_pRT->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(transform));
            return S_OK;
        }

        COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE IsPixelSnappingDisabled(
                _In_opt_ void* clientDrawingContext,
                _Out_ BOOL* isDisabled
        ) override {
            *isDisabled = FALSE;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE QueryInterface(
                /* [in] */ REFIID riid,
                /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override {
            if (riid == IID_IUnknown) {
                *ppvObject = this;
                AddRef();
                return NOERROR;
            }

            return E_NOINTERFACE;
        }

        ULONG STDMETHODCALLTYPE AddRef() override {
            return ++m_cRef;
        }

        ULONG STDMETHODCALLTYPE Release() override {
            MULong ref = --m_cRef;
            if (0 == m_cRef) {
                delete this;
            }
            return ref;
        }

    public:
        void Init(const CComPtr<ID2D1Factory>& factory, const CComPtr<ID2D1HwndRenderTarget>& RT) {
            m_pD2DFactory = factory;
            m_pRT = RT;
        }

        void SetFontWidth(MFloat stroke) {
            m_stroke = stroke;
        }

        void SetFillBrush(CComPtr<ID2D1Brush> brush) {
            m_pFillBrush = brush;
        }

        void SetOutlineBrush(CComPtr<ID2D1Brush> brush) {
            m_pOutlineBrush = brush;
        }

        void SetUnderlineBrush(CComPtr<ID2D1Brush> brush) {
            m_pUnderlineBrush = brush;
        }

        void SetStrikethroughBrush(CComPtr<ID2D1Brush> brush) {
            m_pStrikethroughBrush = brush;
        }

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
