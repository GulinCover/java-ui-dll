#include "IMWriter.h"

namespace MWD::Writer {
    //////////////////////////////////////////////////
    // 文本渲染类
    /********************************************************
     IDWriteTextRenderer方法实现
    *********************************************************/
    COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE IMWriter::DrawGlyphRun(
            _In_opt_ void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_MEASURING_MODE measuringMode,
            _In_ DWRITE_GLYPH_RUN const* glyphRun,
            _In_ DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
            _In_opt_ IUnknown* clientDrawingEffect
    ) {
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

    COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE IMWriter::DrawInlineObject(
            _In_opt_ void* clientDrawingContext,
            FLOAT originX,
            FLOAT originY,
            _In_ IDWriteInlineObject* inlineObject,
            BOOL isSideways,
            BOOL isRightToLeft,
            _In_opt_ IUnknown* clientDrawingEffect
    ) {
        return E_NOTIMPL;
    }

    COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE IMWriter::DrawStrikethrough(
            _In_opt_ void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            _In_ DWRITE_STRIKETHROUGH const* strikethrough,
            _In_opt_ IUnknown* clientDrawingEffect
    ) {
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

    COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE IMWriter::DrawUnderline(
            _In_opt_ void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            _In_ DWRITE_UNDERLINE const* underline,
            _In_opt_ IUnknown* clientDrawingEffect
    ) {
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

    COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE IMWriter::GetPixelsPerDip(
            _In_opt_ void* clientDrawingContext,
            _Out_ FLOAT* pixelsPerDip
    ) {
        MFloat x, y;
        m_pRT->GetDpi(&x, &y);
        *pixelsPerDip = x / 96;
        return S_OK;
    }

    COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE IMWriter::GetCurrentTransform(
            _In_opt_ void* clientDrawingContext,
            _Out_ DWRITE_MATRIX* transform
    ) {
        m_pRT->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(transform));
        return S_OK;
    }

    COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE IMWriter::IsPixelSnappingDisabled(
            _In_opt_ void* clientDrawingContext,
            _Out_ BOOL* isDisabled
    ) {
        *isDisabled = FALSE;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE IMWriter::QueryInterface(
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) {
        if (riid == IID_IUnknown) {
            *ppvObject = this;
            AddRef();
            return NOERROR;
        }

        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE IMWriter::AddRef() {
        return ++m_cRef;
    }

    ULONG STDMETHODCALLTYPE IMWriter::Release() {
        MULong ref = --m_cRef;
        if (0 == m_cRef) {
            delete this;
        }
        return ref;
    }

    void IMWriter::Init(const CComPtr<ID2D1Factory>& factory, const CComPtr<ID2D1HwndRenderTarget>& RT) {
        m_pD2DFactory = factory;
        m_pRT = RT;
    }

    void IMWriter::SetFontWidth(MFloat stroke) {
        m_stroke = stroke;
    }

    void IMWriter::SetFillBrush(CComPtr<ID2D1Brush> brush) {
        m_pFillBrush = std::move(brush);
    }

    void IMWriter::SetOutlineBrush(CComPtr<ID2D1Brush> brush) {
        m_pOutlineBrush = std::move(brush);
    }

    void IMWriter::SetUnderlineBrush(CComPtr<ID2D1Brush> brush) {
        m_pUnderlineBrush = std::move(brush);
    }

    void IMWriter::SetStrikethroughBrush(CComPtr<ID2D1Brush> brush) {
        m_pStrikethroughBrush = std::move(brush);
    }

}

