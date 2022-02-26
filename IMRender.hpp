#pragma once

#include "IMFrame.hpp"
#include "IMSwapChain.hpp"
#include "WindowsNative.hpp"
#include "IMLogger.hpp"
#include "IMTimer.hpp"
#include "IMonitor.hpp"
#include "IMTextInput.hpp"

//#pragma comment(lib, "comsuppw.lib")

namespace MWD::Render {
    using namespace MWD::Frame;
    using namespace MWD::SwapChain;
    using namespace MWD::WindowsNative;
    using namespace MWD::GM;
    using namespace MWD::Log;
    using namespace MWD::Timer;
    using namespace MWD::Monitor;
    using namespace MWD::TextInput;

    /********************************************************
     渲染器参数
    *********************************************************/
    class IMRenderItemEvent {
    public:
        IMRenderItemEvent() {
            m_pD2DFactory = nullptr;
            m_pRT = nullptr;
            m_imWriter = nullptr;
            m_pWriteFactory = nullptr;
            m_pWicFactory = nullptr;
            m_monitor = nullptr;
            m_editor = nullptr;
        }

    public:
        CComPtr<ID2D1Factory> m_pD2DFactory;
        CComPtr<ID2D1HwndRenderTarget> m_pRT;
        CComPtr<IMWriter> m_imWriter;
        CComPtr<IDWriteFactory> m_pWriteFactory;
        CComPtr<IWICImagingFactory> m_pWicFactory;

        std::shared_ptr<IMonitor> m_monitor;

        std::shared_ptr<IMEditorManager> m_editor;

        HWND m_hwnd;

        MString m_route;
    };

    /********************************************************
     本地方法
    *********************************************************/
    class IMethod {
    public:
        /**
         * 计算字宽高
         * @param text
         */


        /**
         * 获取本地字体合集
         * @return
         */
#define IM_SUCCEEDED(HR, RESULT) \
    if (HR != S_OK) return RESULT;

        static std::shared_ptr<IMFontCollection> GetFontCollection(CComPtr<IDWriteFactory> pWriteFactory) {
            CComPtr<IDWriteFontCollection> collection;
            HRESULT hr = pWriteFactory->GetSystemFontCollection(&collection);
            IM_SUCCEEDED(hr, nullptr);

            std::shared_ptr<IMFontCollection> fontCollection = std::make_shared<IMFontCollection>();

            for (int i = 0; i < collection->GetFontFamilyCount(); ++i) {
                CComPtr<IDWriteFontFamily> family;
                hr = collection->GetFontFamily(i, &family);
                IM_SUCCEEDED(hr, nullptr);

                CComPtr<IDWriteLocalizedStrings> localizedStrings;
                hr = family->GetFamilyNames(&localizedStrings);
                IM_SUCCEEDED(hr, nullptr);

                UINT32 index = 0;
                BOOL exists = false;
                wchar_t localeName[LOCALE_NAME_MAX_LENGTH];
                int defaultLocaleSuccess = GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH);

                if (defaultLocaleSuccess) {
                    hr = localizedStrings->FindLocaleName(localeName, &index, &exists);
                    IM_SUCCEEDED(hr, nullptr);
                }

                if (!exists) {
                    hr = localizedStrings->FindLocaleName(L"en-us", &index, &exists);
                    IM_SUCCEEDED(hr, nullptr);
                }

                if (!exists)
                    index = 0;
                UINT32 length = 0;

                hr = localizedStrings->GetStringLength(index, &length);
                IM_SUCCEEDED(hr, nullptr);

                wchar_t* name = new(std::nothrow) wchar_t[length + 1];

                hr = localizedStrings->GetString(index, name, length + 1);
                IM_SUCCEEDED(hr, nullptr);

                MInt len = length + 1;
                MInt l = WideCharToMultiByte(CP_UTF8, 0, name, len, nullptr, 0, nullptr, nullptr);
                MChar* cs = new MChar[l + 1];

                WideCharToMultiByte(CP_UTF8, 0, name, len, cs, l, nullptr, nullptr);
                cs[len] = '\0';

                MString ret;
                ret.append(cs);

                delete[] cs;
                delete[] name;

                if (ret.empty())
                    continue;

                fontCollection->AddName(ret);
            }

            return fontCollection;
        }
    };

    /********************************************************
     渲染器item类
    *********************************************************/
    enum EVENT_IID {
        MOUSE_ENTER = 0,
        MOUSE_OUTER = 1,
        MOUSE_LEFT_CLICK_DOWN = 2,
        MOUSE_LEFT_CLICK_UP = 3,
        MOUSE_RIGHT_CLICK_DOWN = 4,
        MOUSE_RIGHT_CLICK_UP = 5,
        MOUSE_LEFT_DOUBLE_CLICK = 6,
        MOUSE_RIGHT_DOUBLE_CLICK = 7,
        MOUSE_MOVE = 8,
        KEY_DOWN = 9,
        KEY_UP = 10,
        KEY_ALPHA_DOWN = 11,
        KEY_ALPHA_UP = 12,
        KEY_IME_DOWN = 13,
        KEY_IME_UP = 14,
    };

    class IMRenderItem {
    public:
        static MInt ToInt(EVENT_IID iid) {
            return static_cast<MInt>(iid);
        }

    public:
        virtual ~IMRenderItem() = default;

    public:
        virtual void Init(const std::shared_ptr<IMFrame>&, IMRenderItemEvent) = 0;

        virtual void Render(const std::shared_ptr<IMFrame>&, const IMRenderItemEvent&) = 0;

        virtual MBoolean IsInit() {
            return m_isInit;
        }

    protected:
        MBoolean m_isInit = true;
    };

    /********************************************************
     裁切器
    *********************************************************/
    class IMClipRender {
    public:
        void ClipBegin(const std::shared_ptr<IMFrame>& frame, IMRenderItemEvent e) const {
            CComPtr<ID2D1PathGeometry> pPathGeometry;
            e.m_pD2DFactory->CreatePathGeometry(&pPathGeometry);

            CComPtr<ID2D1GeometrySink> pSink;
            pPathGeometry->Open(&pSink);

            MFloat radiusLT = frame->border->borderTopLeftRadius;
            MFloat radiusLB = frame->border->borderBottomLeftRadius;
            MFloat radiusRT = frame->border->borderTopRightRadius;
            MFloat radiusRB = frame->border->borderBottomRightRadius;

            pSink->BeginFigure(D2D1::Point2F(frame->innerX + radiusLT + frame->offX, frame->innerY + frame->offY),
                               D2D1_FIGURE_BEGIN_FILLED);

            pSink->AddLine(
                    D2D1::Point2F(frame->innerX + frame->paddingLeft + frame->paddingRight + frame->width - radiusRT +
                                  frame->offX, frame->innerY + frame->offY));

            pSink->AddArc(
                    D2D1::ArcSegment(
                            D2D1::Point2F(frame->innerX + frame->paddingLeft + frame->paddingRight + frame->width +
                                          frame->offX,
                                          frame->innerY + radiusRT + frame->offY),
                            D2D1::SizeF(radiusRT, radiusRT),
                            0.0f, // rotation angle
                            D2D1_SWEEP_DIRECTION_CLOCKWISE,
                            D2D1_ARC_SIZE_SMALL
                    ));

            pSink->AddLine(
                    D2D1::Point2F(frame->innerX + frame->paddingLeft + frame->paddingRight + frame->width + frame->offX,
                                  frame->innerY + frame->paddingTop + frame->paddingBottom + frame->height - radiusRB +
                                  frame->offY));

            pSink->AddArc(
                    D2D1::ArcSegment(
                            D2D1::Point2F(
                                    frame->innerX + frame->paddingLeft + frame->paddingRight + frame->width - radiusRB +
                                    frame->offX,
                                    frame->innerY + frame->paddingTop + frame->paddingBottom + frame->height +
                                    frame->offY),
                            D2D1::SizeF(radiusRB, radiusRB),
                            0.0f,
                            D2D1_SWEEP_DIRECTION_CLOCKWISE,
                            D2D1_ARC_SIZE_SMALL
                    ));

            pSink->AddLine(
                    D2D1::Point2F(frame->innerX + radiusLB + frame->offX,
                                  frame->innerY + frame->paddingTop + frame->paddingBottom + frame->height +
                                  frame->offY));

            pSink->AddArc(
                    D2D1::ArcSegment(
                            D2D1::Point2F(frame->innerX + frame->offX,
                                          frame->innerY + frame->paddingTop + frame->paddingBottom + frame->height -
                                          radiusLB + frame->offY),
                            D2D1::SizeF(radiusLB, radiusLB),
                            0.0f,
                            D2D1_SWEEP_DIRECTION_CLOCKWISE,
                            D2D1_ARC_SIZE_SMALL
                    ));

            pSink->AddLine(D2D1::Point2F(frame->innerX + frame->offX, frame->innerY + radiusLT + frame->offY));

            pSink->AddArc(
                    D2D1::ArcSegment(
                            D2D1::Point2F(frame->innerX + radiusLT + frame->offX, frame->innerY + frame->offY),
                            D2D1::SizeF(radiusLT, radiusLT),
                            0.0f,
                            D2D1_SWEEP_DIRECTION_CLOCKWISE,
                            D2D1_ARC_SIZE_SMALL
                    ));

            pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
            pSink->Close();

            CComPtr<ID2D1Layer> pLayer;
            e.m_pRT->CreateLayer(nullptr, &pLayer);
            e.m_pRT->PushLayer(
                    D2D1::LayerParameters(D2D1::InfiniteRect(), pPathGeometry),
                    pLayer
            );
        }

        void ClipEnd(const IMRenderItemEvent& e) const {
            e.m_pRT->PopLayer();
        }
    };

    /********************************************************
     背景渲染器
    *********************************************************/
    class IMBackgroundRenderItem : public IMRenderItem {
    public:
        ~IMBackgroundRenderItem() override = default;

    public:
        void Init(const std::shared_ptr<IMFrame>&, IMRenderItemEvent) override {
            m_isInit = false;
        }

        void Render(const std::shared_ptr<IMFrame>& frame, const IMRenderItemEvent& e) override {
            CComPtr<ID2D1SolidColorBrush> pBrush;

            e.m_pRT->CreateSolidColorBrush(
                    D2D1::ColorF(D2D1::ColorF(
                            frame->background->backgroundColor->R,
                            frame->background->backgroundColor->G,
                            frame->background->backgroundColor->B,
                            frame->background->backgroundColor->A)),
                    &pBrush);
            e.m_pRT->FillRoundedRectangle(
                    D2D1::RoundedRect(D2D1::RectF(frame->innerX + frame->offX, frame->innerY + frame->offY,
                                                  frame->innerX + frame->width + frame->offX + frame->paddingLeft +
                                                  frame->paddingRight,
                                                  frame->innerY + frame->height + frame->offY + frame->paddingTop +
                                                  frame->paddingBottom),
                                      frame->border->borderRadius, frame->border->borderRadius), pBrush);

            // 渲染选中的文本
            if (!frame->font->textSelectedRect->empty() && frame->isInput) {
                auto iter = frame->font->textSelectedRect->begin();
                CComPtr<ID2D1SolidColorBrush> fontBackBrush;
                e.m_pRT->CreateSolidColorBrush(D2D1::ColorF(0, 0, .8, .4), &fontBackBrush);
                for (; iter != frame->font->textSelectedRect->end(); ++iter) {
                    D2D1_RECT_F rect = *iter;
                    if (iter->left < frame->innerX + frame->paddingLeft + frame->offX)
                        rect.left = frame->innerX + frame->paddingLeft;

                    if (iter->top < frame->innerY + frame->paddingTop + frame->offY)
                        rect.top = frame->innerY + frame->paddingTop;

                    if (iter->right >
                        frame->innerX + frame->paddingLeft + frame->paddingRight + frame->width + frame->offX)
                        rect.right =
                                frame->innerX + frame->paddingLeft + frame->paddingRight + frame->width + frame->offX;

                    if (iter->bottom >
                        frame->innerY + frame->paddingTop + frame->paddingBottom + frame->height + frame->offY)
                        rect.bottom =
                                frame->innerY + frame->paddingTop + frame->paddingBottom + frame->height + frame->offY;

                    e.m_pRT->FillRectangle(rect, fontBackBrush);
                }
            }
        }
    };

    /********************************************************
     边框渲染器
    *********************************************************/
    class IMBorderRenderItem : public IMRenderItem {
    public:
        ~IMBorderRenderItem() override = default;

    private:
        enum ARC_DIRECT {
            LEFT_TOP,
            RIGHT_TOP,
            RIGHT_BOTTOM,
            LEFT_BOTTOM,
        };

        D2D1_POINT_2F _calcArcPoint(MFloat x, MFloat y, MFloat radius, ARC_DIRECT direct) {
            MDouble diff = std::sqrt(2) / 2.0f * radius;
            D2D1_POINT_2F f;

            switch (direct) {
                case LEFT_TOP:
                    f = D2D1::Point2F(x + radius - diff, y - diff);
                    break;
                case RIGHT_TOP:
                    f = D2D1::Point2F(x + diff, y + radius - diff);
                    break;
                case RIGHT_BOTTOM:
                    f = D2D1::Point2F(x - radius + diff, y + diff);
                    break;
                case LEFT_BOTTOM:
                    f = D2D1::Point2F(x - diff, y - radius + diff);
                    break;
                default:
                    break;
            }

            return f;
        }

    public:
        void Init(const std::shared_ptr<IMFrame>&, IMRenderItemEvent) override {
            m_isInit = false;
        }

        void Render(const std::shared_ptr<IMFrame>& frame, const IMRenderItemEvent& e) override {
            // arc point
            D2D1_POINT_2F ltPoint = _calcArcPoint(frame->innerX + frame->offX,
                                                  frame->innerY + frame->offY + frame->border->borderTopLeftRadius,
                                                  frame->border->borderTopLeftRadius, LEFT_TOP);
            D2D1_POINT_2F rtPoint = _calcArcPoint(
                    frame->innerX + frame->offX + frame->width - frame->border->borderTopRightRadius,
                    frame->innerY + frame->offY, frame->border->borderTopRightRadius, RIGHT_TOP);
            D2D1_POINT_2F rbPoint = _calcArcPoint(frame->innerX + frame->offX + frame->width,
                                                  frame->innerY + frame->offY + frame->height -
                                                  frame->border->borderBottomRightRadius,
                                                  frame->border->borderBottomRightRadius, RIGHT_BOTTOM);
            D2D1_POINT_2F lbPoint = _calcArcPoint(frame->innerX + frame->offX + frame->border->borderBottomLeftRadius,
                                                  frame->innerY + frame->offY + frame->height,
                                                  frame->border->borderBottomLeftRadius,
                                                  LEFT_BOTTOM);



            // top line
            {
                CComPtr<ID2D1Brush> brush;
                CComPtr<ID2D1PathGeometry> pPathGeometry;
                CComPtr<ID2D1GeometrySink> pGeometrySink;

                e.m_pD2DFactory->CreatePathGeometry(&pPathGeometry);
                pPathGeometry->Open(&pGeometrySink);
                pGeometrySink->BeginFigure(ltPoint, D2D1_FIGURE_BEGIN_HOLLOW);
                pGeometrySink->AddArc(
                        D2D1::ArcSegment(
                                D2D1::Point2F(frame->innerX + frame->border->borderTopLeftRadius + frame->offX,
                                              frame->innerY + frame->offY),
                                D2D1::SizeF(frame->border->borderTopLeftRadius, frame->border->borderTopLeftRadius),
                                0.0f,
                                D2D1_SWEEP_DIRECTION_CLOCKWISE,
                                D2D1_ARC_SIZE_SMALL
                        ));
                pGeometrySink->AddLine(
                        D2D1::Point2F(frame->innerX + frame->width - frame->border->borderTopRightRadius + frame->offX,
                                      frame->innerY + frame->offY));
                pGeometrySink->AddArc(
                        D2D1::ArcSegment(
                                rtPoint,
                                D2D1::SizeF(frame->border->borderTopRightRadius, frame->border->borderTopRightRadius),
                                0.0f,
                                D2D1_SWEEP_DIRECTION_CLOCKWISE,
                                D2D1_ARC_SIZE_SMALL
                        ));
                pGeometrySink->EndFigure(D2D1_FIGURE_END_OPEN);
                pGeometrySink->Close();

                switch (frame->border->borderStyle) {
                    case Frame::LINEAR_GRADIENT:
                        //e.m_pRT->CreateLinearGradientBrush();
                        break;
                    case Frame::RADIAL_GRADIENT:
                        //e.m_pRT->CreateRadialGradientBrush();
                        break;
                    default:
                        e.m_pRT->CreateSolidColorBrush(
                                D2D1::ColorF(frame->border->borderTopColor->R, frame->border->borderTopColor->G,
                                             frame->border->borderTopColor->B, frame->border->borderTopColor->A),
                                (ID2D1SolidColorBrush**) &brush);
                        break;
                }
                e.m_pRT->DrawGeometry(pPathGeometry, brush, frame->border->borderTopWidth);
            }

            // right line
            {
                CComPtr<ID2D1Brush> brush;
                CComPtr<ID2D1PathGeometry> pPathGeometry;
                CComPtr<ID2D1GeometrySink> pGeometrySink;

                e.m_pD2DFactory->CreatePathGeometry(&pPathGeometry);
                pPathGeometry->Open(&pGeometrySink);
                pGeometrySink->BeginFigure(rtPoint, D2D1_FIGURE_BEGIN_HOLLOW);
                pGeometrySink->AddArc(
                        D2D1::ArcSegment(
                                D2D1::Point2F(frame->innerX + frame->width + frame->offX,
                                              frame->innerY + frame->border->borderTopRightRadius + frame->offY),
                                D2D1::SizeF(frame->border->borderTopRightRadius, frame->border->borderTopRightRadius),
                                0.0f,
                                D2D1_SWEEP_DIRECTION_CLOCKWISE,
                                D2D1_ARC_SIZE_SMALL
                        ));
                pGeometrySink->AddLine(
                        D2D1::Point2F(frame->innerX + frame->width + frame->offX, frame->innerY + frame->height -
                                                                                  frame->border->borderBottomRightRadius +
                                                                                  frame->offY));
                pGeometrySink->AddArc(
                        D2D1::ArcSegment(
                                rbPoint,
                                D2D1::SizeF(frame->border->borderBottomRightRadius,
                                            frame->border->borderBottomRightRadius),
                                0.0f,
                                D2D1_SWEEP_DIRECTION_CLOCKWISE,
                                D2D1_ARC_SIZE_SMALL
                        ));
                pGeometrySink->EndFigure(D2D1_FIGURE_END_OPEN);
                pGeometrySink->Close();

                switch (frame->border->borderStyle) {
                    case Frame::LINEAR_GRADIENT:
                        //e.m_pRT->CreateLinearGradientBrush();
                        break;
                    case Frame::RADIAL_GRADIENT:
                        //e.m_pRT->CreateRadialGradientBrush();
                        break;
                    default:
                        e.m_pRT->CreateSolidColorBrush(
                                D2D1::ColorF(frame->border->borderRightColor->R, frame->border->borderRightColor->G,
                                             frame->border->borderRightColor->B, frame->border->borderRightColor->A),
                                (ID2D1SolidColorBrush**) &brush);
                        break;
                }
                e.m_pRT->DrawGeometry(pPathGeometry, brush, frame->border->borderRightWidth);
            }

            // bottom line
            {
                CComPtr<ID2D1Brush> brush;
                CComPtr<ID2D1PathGeometry> pPathGeometry;
                CComPtr<ID2D1GeometrySink> pGeometrySink;

                e.m_pD2DFactory->CreatePathGeometry(&pPathGeometry);
                pPathGeometry->Open(&pGeometrySink);
                pGeometrySink->BeginFigure(rbPoint, D2D1_FIGURE_BEGIN_HOLLOW);
                pGeometrySink->AddArc(
                        D2D1::ArcSegment(
                                D2D1::Point2F(frame->innerX + frame->width - frame->border->borderBottomRightRadius +
                                              frame->offX,
                                              frame->innerY + frame->height + frame->offY),
                                D2D1::SizeF(frame->border->borderBottomRightRadius,
                                            frame->border->borderBottomRightRadius),
                                0.0f,
                                D2D1_SWEEP_DIRECTION_CLOCKWISE,
                                D2D1_ARC_SIZE_SMALL
                        ));
                pGeometrySink->AddLine(
                        D2D1::Point2F(frame->innerX + frame->border->borderBottomLeftRadius + frame->offX,
                                      frame->innerY + frame->height + frame->offY));
                pGeometrySink->AddArc(
                        D2D1::ArcSegment(
                                lbPoint,
                                D2D1::SizeF(frame->border->borderBottomLeftRadius,
                                            frame->border->borderBottomLeftRadius),
                                0.0f,
                                D2D1_SWEEP_DIRECTION_CLOCKWISE,
                                D2D1_ARC_SIZE_SMALL
                        ));
                pGeometrySink->EndFigure(D2D1_FIGURE_END_OPEN);
                pGeometrySink->Close();

                switch (frame->border->borderStyle) {
                    case Frame::LINEAR_GRADIENT:
                        //e.m_pRT->CreateLinearGradientBrush();
                        break;
                    case Frame::RADIAL_GRADIENT:
                        //e.m_pRT->CreateRadialGradientBrush();
                        break;
                    default:
                        e.m_pRT->CreateSolidColorBrush(
                                D2D1::ColorF(frame->border->borderBottomColor->R, frame->border->borderBottomColor->G,
                                             frame->border->borderBottomColor->B, frame->border->borderBottomColor->A),
                                (ID2D1SolidColorBrush**) &brush);
                        break;
                }
                e.m_pRT->DrawGeometry(pPathGeometry, brush, frame->border->borderBottomWidth);
            }

            // left line
            {
                CComPtr<ID2D1Brush> brush;
                CComPtr<ID2D1PathGeometry> pPathGeometry;
                CComPtr<ID2D1GeometrySink> pGeometrySink;

                e.m_pD2DFactory->CreatePathGeometry(&pPathGeometry);
                pPathGeometry->Open(&pGeometrySink);
                pGeometrySink->BeginFigure(lbPoint, D2D1_FIGURE_BEGIN_HOLLOW);
                pGeometrySink->AddArc(
                        D2D1::ArcSegment(
                                D2D1::Point2F(frame->innerX + frame->offX,
                                              frame->innerY + frame->height - frame->border->borderBottomLeftRadius +
                                              frame->offY),
                                D2D1::SizeF(frame->border->borderBottomLeftRadius,
                                            frame->border->borderBottomLeftRadius),
                                0.0f,
                                D2D1_SWEEP_DIRECTION_CLOCKWISE,
                                D2D1_ARC_SIZE_SMALL
                        ));
                pGeometrySink->AddLine(
                        D2D1::Point2F(frame->innerX + frame->offX,
                                      frame->innerY + frame->border->borderTopLeftRadius + frame->offY));
                pGeometrySink->AddArc(
                        D2D1::ArcSegment(
                                ltPoint,
                                D2D1::SizeF(frame->border->borderTopLeftRadius, frame->border->borderTopLeftRadius),
                                0.0f,
                                D2D1_SWEEP_DIRECTION_CLOCKWISE,
                                D2D1_ARC_SIZE_SMALL
                        ));
                pGeometrySink->EndFigure(D2D1_FIGURE_END_OPEN);
                pGeometrySink->Close();

                switch (frame->border->borderStyle) {
                    case Frame::LINEAR_GRADIENT:
                        //e.m_pRT->CreateLinearGradientBrush();
                        break;
                    case Frame::RADIAL_GRADIENT:
                        //e.m_pRT->CreateRadialGradientBrush();
                        break;
                    default:
                        e.m_pRT->CreateSolidColorBrush(
                                D2D1::ColorF(frame->border->borderLeftColor->R, frame->border->borderLeftColor->G,
                                             frame->border->borderLeftColor->B, frame->border->borderLeftColor->A),
                                (ID2D1SolidColorBrush**) &brush);
                        break;
                }
                e.m_pRT->DrawGeometry(pPathGeometry, brush, frame->border->borderLeftWidth);
            }
        }
    };

    /********************************************************
     滑动块渲染器
    *********************************************************/
    class IMScrollBarRenderItem : public IMRenderItem {
    public:
        ~IMScrollBarRenderItem() override = default;

    public:
        void Init(const std::shared_ptr<IMFrame>&, IMRenderItemEvent) override {
            m_isInit = false;
        }

        void Render(const std::shared_ptr<IMFrame>& frame, const IMRenderItemEvent& e) override {
            //ID2D1SolidColorBrush* brush;
            //e.m_pRT->CreateSolidColorBrush(D2D1::ColorF(0, 0, 1.0f), &brush);
            //e.m_pRT->FillRectangle(
            //        D2D1::RectF(frame->innerX + frame->width - 5, frame->innerY + 6, frame->innerX + frame->width,
            //                    frame->innerY + frame->height - 6), brush);
            //brush->Release();
        }
    };

    /********************************************************
     文本渲染器
    *********************************************************/
    class IMTextRenderItem : public IMRenderItem {
    public:
        ~IMTextRenderItem() override = default;

    public:
        void Init(const std::shared_ptr<IMFrame>& frame, IMRenderItemEvent e) override {

            m_isInit = false;
        }

        void Render(const std::shared_ptr<IMFrame>& frame, const IMRenderItemEvent& e) override {
            CComPtr<ID2D1Brush> brush;
            std::shared_ptr<IMColor> color = frame->font->fontColor;
            e.m_pRT->CreateSolidColorBrush(D2D1::ColorF(color->R, color->G, color->B, color->A),
                                           (ID2D1SolidColorBrush**) (&brush));

            e.m_imWriter->SetFillBrush(brush);
            e.m_imWriter->SetOutlineBrush(brush);

            if (frame->font->isSingleRow)
                frame->textLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

            frame->textLayout->Draw(nullptr, e.m_imWriter, frame->innerX + frame->paddingLeft + frame->offX + frame->textOffX,
                              frame->innerY + frame->paddingTop + frame->offY + frame->textOffY);
        }
    };

    /********************************************************
     图片渲染器
    *********************************************************/
    class IMImageRenderItem : public IMRenderItem {
    public:
        ~IMImageRenderItem() override = default;

    public:
        void Init(const std::shared_ptr<IMFrame>&, IMRenderItemEvent e) override {
            m_isInit = false;
        }

        void Render(const std::shared_ptr<IMFrame>& frame, const IMRenderItemEvent& e) override {
            if (frame->background->backgroundImageUrl.empty()) return;

            CComPtr<IWICBitmapDecoder> pWicDecoder;
            CComPtr<IWICBitmapFrameDecode> pWicFrame;
            CComPtr<IWICFormatConverter> pWicConverter;
            CComPtr<ID2D1Bitmap> pBmp;

            WIN32_FIND_DATAA wfd;
            HANDLE hf = FindFirstFileA(frame->background->backgroundImageUrl.c_str(), &wfd);
            if (INVALID_HANDLE_VALUE == hf) {
                IM_LOG_ERROR("The image file does not exist", "IMImageRenderItem::Render");
                return;
            }

            MInt len = MultiByteToWideChar(CP_UTF8, 0, frame->background->backgroundImageUrl.c_str(),
                                           frame->background->backgroundImageUrl.length(), nullptr, 0);
            WCHAR* imagePath = new wchar_t[len + 1];
            MultiByteToWideChar(CP_UTF8, 0, frame->background->backgroundImageUrl.c_str(), -1, imagePath, len);
            imagePath[len] = '\0';

            e.m_pWicFactory->CreateDecoderFromFilename(
                    imagePath,
                    nullptr,
                    GENERIC_READ,
                    WICDecodeMetadataCacheOnLoad,
                    &pWicDecoder
            );

            delete[] imagePath;

            pWicDecoder->GetFrame(0, &pWicFrame);

            e.m_pWicFactory->CreateFormatConverter(&pWicConverter);

            pWicConverter->Initialize(
                    pWicFrame,
                    GUID_WICPixelFormat32bppPBGRA,
                    WICBitmapDitherTypeNone,
                    nullptr,
                    0.0,
                    WICBitmapPaletteTypeCustom
            );

            e.m_pRT->CreateBitmapFromWicBitmap(pWicConverter, nullptr, &pBmp);

            D2D1_RECT_F rect;
            rect.left = frame->innerX + frame->offX;
            rect.top = frame->innerY + frame->offY;
            rect.right = frame->innerX + frame->width + frame->offX;
            rect.bottom = frame->innerY + frame->height + frame->offY;

            e.m_pRT->DrawBitmap(
                    pBmp,
                    rect,
                    (frame->opacity > 1 ? 1 : frame->opacity) < 0 ? 0 : frame->opacity,
                    D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
                    D2D1::RectF(0.0f, 0.0f, pBmp->GetSize().width, pBmp->GetSize().height)
            );
        }
    };

    /********************************************************
     输入法文本预览
    *********************************************************/
    class IMPrimaryTextRenderItem : public IMRenderItem {
    public:
        void Init(const std::shared_ptr<IMFrame>&, IMRenderItemEvent) override {
            m_isInit = false;
        }

        void Render(const std::shared_ptr<IMFrame>& frame, const IMRenderItemEvent& e) override {
            if (frame->font->primaryText.empty())return;

            CComPtr<IDWriteTextLayout> pTextLayout;

            frame->writeFactory->CreateTextLayout(frame->font->primaryText.c_str(), frame->font->primaryText.length(),
                                                frame->textFormat, frame->width, frame->height,
                                                &pTextLayout);

            CComPtr<ID2D1Brush> brush;
            std::shared_ptr<IMColor> color = frame->font->fontColor;
            e.m_pRT->CreateSolidColorBrush(D2D1::ColorF(color->R, color->G, color->B, color->A),
                                           (ID2D1SolidColorBrush**) (&brush));

            e.m_imWriter->SetFillBrush(brush);
            e.m_imWriter->SetOutlineBrush(brush);

            pTextLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

            pTextLayout->Draw(nullptr, e.m_imWriter, frame->caretPosX,
                              frame->caretPosY);
        }
    };

    /********************************************************
     光标闪烁渲染
    *********************************************************/
    class IMCaretRenderItem : public IMRenderItem {
    public:
        IMCaretRenderItem() : m_flush(true) {

        }

    public:
        void Init(const std::shared_ptr<IMFrame>& frame, IMRenderItemEvent e) override {
            MULLong id;
            std::function<void(void*)> func = [&](void* args) {
                m_flush = !m_flush;
            };
            IM_TIMER_SET_TIMER(id, 0, 450, nullptr, func);
            m_isInit = false;
        }

        void Render(const std::shared_ptr<IMFrame>& frame, const IMRenderItemEvent& e) override {
            std::shared_ptr<IMFrame> f = IMFocusManager::GetInstance()->GetFocusFrame(e.m_route);
            if (m_flush && f && f->isInput && f->uuid == frame->uuid) {
                CComPtr<ID2D1SolidColorBrush> brush;

                e.m_pRT->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0, 0), &brush);
                e.m_pRT->DrawLine(
                        D2D1::Point2F(f->caretPosX, f->caretPosY),
                        D2D1::Point2F(f->caretPosX, f->caretPosY + frame->font->fontSize * 1.35),
                        brush
                );
            }
        }

    private:
        MBoolean m_flush;
    };

    /********************************************************
     文本输入器
    *********************************************************/
    class IMTextInputRenderItem : public IMRenderItem {
    public:
        ~IMTextInputRenderItem() override = default;

    public:
        void Init(const std::shared_ptr<IMFrame>& frame, IMRenderItemEvent e) override {
            m_isInit = false;
            if (!frame->isInput) return;
        }

        void Render(const std::shared_ptr<IMFrame>& frame, const IMRenderItemEvent& e) override {
            if (!frame->isFocus && !frame->isInput) return;

            e.m_editor->ChangeFrame(frame);
        }
    };

    /********************************************************
     默认事件添加器
    *********************************************************/
#define IM_FRAME_DEFAULT_CLICK_EVENT_IID 1024*1024

    class IMEventRenderItem : public IMRenderItem {
    public:
        void Init(const std::shared_ptr<IMFrame>& frame, IMRenderItemEvent e) override {
            m_isInit = false;
        }

        void Render(const std::shared_ptr<IMFrame>& frame, const IMRenderItemEvent& e) override {
            if (frame->eventYet->Equal(IM_FRAME_DEFAULT_CLICK_EVENT_IID, SYNC_QUEUE, MOUSE_LEFT_BUTTON_DOWN))
                return;

            auto switchFunc = std::make_shared<std::function<void(
                    std::shared_ptr<IMonitorEvent>, void*)>>(
                    [](const std::shared_ptr<IMonitorEvent>& initArgs, void* args) {
                        auto focusManager = IMFocusManager::GetInstance();
                        auto f = focusManager->GetFocusFrame("route1");

                        if (f) {
                            if (f->uuid != initArgs->frame->uuid) {
                                f->isFocus = false;
                                initArgs->frame->isFocus = true;
                                focusManager->SetFocusFrame("route1", initArgs->frame);
                            }
                        } else {
                            initArgs->frame->isFocus = true;
                            focusManager->SetFocusFrame("route1", initArgs->frame);
                        }
                    });
            auto pp = std::make_shared<IMonitorEvent>();
            pp->frame = frame;

            e.m_monitor->AddTask(frame, Monitor::MOUSE_LEFT_BUTTON_DOWN, SYNC_QUEUE, switchFunc, pp,
                                 IM_FRAME_DEFAULT_CLICK_EVENT_IID);
            frame->eventYet->Add(IM_FRAME_DEFAULT_CLICK_EVENT_IID, SYNC_QUEUE, MOUSE_LEFT_BUTTON_DOWN);
        }
    };

    /********************************************************
     渲染器类
    *********************************************************/
    class IMRender {
    public:
        IMRender() {
            m_renderItems = std::make_shared<Map<MString, std::shared_ptr<IMRenderItem>>>();
            m_imWriter = CComPtr<IMWriter>(new IMWriter());
            m_pRT = nullptr;
            m_pD2DFactory = nullptr;
            m_pWriteFactory = nullptr;
            m_pWicFactory = nullptr;
            m_hwnd = nullptr;
        }

    public:
        void Clean() const {
            m_renderItems->clear();
        }

        void CleanScreen(D2D1::ColorF f) {
            m_pRT->Clear(f);
        }

        void Resize(D2D1_SIZE_U rect) {
            m_pRT->Resize(rect);
        }

        void AddRenderItem(MString name, const std::shared_ptr<IMRenderItem>& item) const {
            m_renderItems->insert(std::make_pair(std::move(name), item));
        }

        [[nodiscard]] MBoolean UpdateRenderItem(const MString& name, const std::shared_ptr<IMRenderItem>& item) const {
            auto it = m_renderItems->find(name);
            if (it != m_renderItems->end()) {
                it->second = item;
                return true;
            }
            return false;
        }

        void SetHwnd(HWND hwnd) {
            m_hwnd = hwnd;
        }

        void SetMonitor(const std::shared_ptr<IMonitor>& monitor) {
            m_monitor = monitor;
        }

        virtual void Render(const std::shared_ptr<IMFrame>& frame) {
            if (!frame->writeFactory) {
                InitSwapFrame(frame);
                m_editor->SetEditorFocus(true);
            }

            if (m_isInit) {
                m_params.m_pRT = m_pRT;
                m_params.m_pD2DFactory = m_pD2DFactory;
                m_params.m_imWriter = m_imWriter;
                m_params.m_pWicFactory = m_pWicFactory;
                m_params.m_monitor = m_monitor;
                m_params.m_route = m_route;
                m_params.m_editor = m_editor;
                m_params.m_hwnd = m_hwnd;
                m_isInit = false;
            }

            for (auto& i : m_itemSort) {
                auto item = m_renderItems->find(i);

                if (item != m_renderItems->end()) {
                    if (item->second->IsInit())
                        item->second->Init(frame, m_params);

                    if (frame->overflow & IM_FRAME_OVERFLOW::HIDDEN || frame->isInput) {
                        m_clipRender->ClipBegin(frame, m_params);
                    }

                    item->second->Render(frame, m_params);

                    if (frame->overflow & IM_FRAME_OVERFLOW::HIDDEN || frame->isInput) {
                        m_clipRender->ClipEnd(m_params);
                    }
                }
            }

            for (auto& iter : *m_elseRenders) {
                if (iter->IsInit())
                    iter->Init(frame, m_params);
                iter->Render(frame, m_params);
            }
        }

        void InitSwapFrame(const std::shared_ptr<IMFrame>& frame) {
            frame->writeFactory = m_pWriteFactory;
            frame->eventYet = std::make_shared<IMEventYet>();

            CComPtr<IDWriteTextFormat> textFormat;
            CComPtr<IDWriteTextLayout> textLayout;
            m_pWriteFactory->CreateTextFormat(
                    frame->font->fontFamily.c_str(),
                    nullptr,
                    IMFont::ToWeight(frame->font->fontWeight),
                    IMFont::ToStyle(frame->font->isItalic),
                    IMFont::ToStretch(),
                    frame->font->fontSize,
                    L"en-us",
                    &textFormat
                    );

            m_pWriteFactory->CreateTextLayout(
                    frame->font->GetText().c_str(),
                    frame->font->GetText().length(),
                    textFormat,
                    frame->width,
                    frame->height,
                    &textLayout
                    );

            frame->textFormat = textFormat;
            frame->textLayout = textLayout;
        }

        MBoolean Init(MString route) {
            m_route = std::move(route);

            // 文本输入器
            m_editor = std::make_shared<IMEditorManager>();
            m_editor->SetEditor(std::make_shared<IMTextInput>());
            m_editor->Init(m_hwnd);

            // d2d
            HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
            if (hr != S_OK) return false;

            RECT rect;
            GetClientRect(m_hwnd, &rect);

            hr = m_pD2DFactory->CreateHwndRenderTarget(
                    D2D1::RenderTargetProperties(),
                    D2D1::HwndRenderTargetProperties(
                            m_hwnd, D2D1::SizeU(rect.right - rect.left, rect.bottom - rect.top)
                    ),
                    &m_pRT);
            if (hr != S_OK) return false;

            // 图片相关
            hr = CoCreateInstance(
                    CLSID_WICImagingFactory,
                    nullptr,
                    CLSCTX_INPROC_SERVER,
                    IID_IWICImagingFactory,
                    (LPVOID*) &m_pWicFactory);
            if (hr != S_OK) return false;

            // 文字渲染工厂
            hr = DWriteCreateFactory(
                    DWRITE_FACTORY_TYPE_SHARED,
                    __uuidof(IDWriteFactory),
                    reinterpret_cast<IUnknown**>(&m_pWriteFactory)
            );

            m_imWriter->Init(m_pD2DFactory, m_pRT);

#define IM_ADD_RENDER_ITEM(KEY, CLASS) \
    m_renderItems->insert(std::make_pair(KEY, std::make_shared<CLASS>()));

            IM_ADD_RENDER_ITEM("text_render", IMTextRenderItem);
            IM_ADD_RENDER_ITEM("background_render", IMBackgroundRenderItem);
            IM_ADD_RENDER_ITEM("border_render", IMBorderRenderItem);
            IM_ADD_RENDER_ITEM("scroll_bar_render", IMScrollBarRenderItem);
            IM_ADD_RENDER_ITEM("image_render", IMImageRenderItem);
            IM_ADD_RENDER_ITEM("input_render", IMTextInputRenderItem);
            IM_ADD_RENDER_ITEM("caret_render", IMCaretRenderItem);

#undef IM_ADD_RENDER_ITEM

            m_elseRenders = std::make_shared<Vector<std::shared_ptr<IMRenderItem>>>();

#define IM_ADD_ELSE_RENDER_ITEM(CLASS) \
    m_elseRenders->push_back(std::make_shared<CLASS>());

            IM_ADD_ELSE_RENDER_ITEM(IMPrimaryTextRenderItem);
            IM_ADD_ELSE_RENDER_ITEM(IMCaretRenderItem);
            IM_ADD_ELSE_RENDER_ITEM(IMEventRenderItem);

#undef IM_ADD_ELSE_RENDER_ITEM

            m_pRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
            m_pRT->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_DEFAULT);

            return hr == S_OK;
        }

        void DrawBegin() {
            m_pRT->BeginDraw();
        }

        void DrawEnd() {
            m_pRT->EndDraw();
        }

    public:
        std::shared_ptr<IMEditorManager> m_editor;
        MString m_route;
        IMRenderItemEvent m_params;
        MBoolean m_isInit = true;
        MBoolean m_isInitSwapFrame = true;

        std::shared_ptr<Map<MString, std::shared_ptr<IMRenderItem>>> m_renderItems;

        HWND m_hwnd;

        CComPtr<ID2D1Factory> m_pD2DFactory;
        CComPtr<ID2D1HwndRenderTarget> m_pRT;

        CComPtr<IDWriteFactory> m_pWriteFactory;
        CComPtr<IWICImagingFactory> m_pWicFactory;

        CComPtr<IMWriter> m_imWriter;

        std::shared_ptr<IMClipRender> m_clipRender;
        std::shared_ptr<Vector<std::shared_ptr<IMRenderItem>>> m_elseRenders;
        std::shared_ptr<IMonitor> m_monitor;

        MString m_itemSort[6] = {"background_render", "image_render", "border_render",
                                 "text_render", "input_render", "scroll_bar_render"};
    };
}//MWD::Render
