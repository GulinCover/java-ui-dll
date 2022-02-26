#pragma once

#include "IMCommon.hpp"

namespace MWD::Frame {
    /********************************************************
     点
    *********************************************************/
    struct IMPoint {
        MDouble x;
        MDouble y;
    };

    /********************************************************
     元素帧版本
    *********************************************************/
    enum IM_FRAME_VERSION {
        VERSION_1 = 1,
        VERSION_2 = 2,
        VERSION_3 = 4,
        VERSION_4 = 8,
        VERSION_5 = 16,
        VERSION_6 = 32,
        VERSION_7 = 64,
        VERSION_8 = 128,
        VERSION_9 = 256
    };

    enum IM_FRAME_POSITION {
        POSITION_RELATIVE = 1,
        POSITION_ABSOLUTE = 2,
    };

    enum IM_FRAME_DISPLAY {
        STATIC = 1,
        FLEX = 2,
        GRID = 3
    };

    enum IM_FRAME_OVERFLOW {
        NONE = 1,
        HIDDEN = 2,
        HIDDEN_X_SCROLL_BAR = 4,
        HIDDEN_Y_SCROLL_BAR = 8,
    };

    /********************************************************
     颜色类
    *********************************************************/
    class IMColor {
    public:
        IMColor(MInt _r, MInt _g, MInt _b, MInt _a = 255) : r(_r), g(_g), b(_b), a(_a) {
            R = (MDouble) _r / 255.0f;
            G = (MDouble) _g / 255.0f;
            B = (MDouble) _b / 255.0f;
            A = (MDouble) _a / 255.0f;
        }

        IMColor(MDouble _r, MDouble _g, MDouble _b, MDouble _a = 1.0f) : R(_r), G(_g), B(_b), A(_a) {
            r = floor(_r * 255);
            g = floor(_g * 255);
            b = floor(_b * 255);
            a = floor(_a * 255);
        }

    public:
        void UpdateR(MInt _r) {
            r = _r;
            R = (MDouble) _r / 255.0f;
        }

        void UpdateR(MDouble _r) {
            R = _r;
            r = floor(_r * 255);
        }

        void UpdateG(MInt _g) {
            g = _g;
            G = (MDouble) _g / 255.0f;
        }

        void UpdateG(MDouble _g) {
            R = _g;
            r = floor(_g * 255);
        }

        void UpdateB(MInt _b) {
            b = _b;
            B = (MDouble) _b / 255.0f;
        }

        void UpdateB(MDouble _b) {
            R = _b;
            r = floor(_b * 255);
        }

        void UpdateA(MInt _a) {
            a = _a;
            A = (MDouble) _a / 255.0f;
        }

        void UpdateA(MDouble _a) {
            A = _a;
            a = floor(_a * 255);
        }

    public:
        MInt r;
        MInt g;
        MInt b;
        MInt a;
        MDouble R;
        MDouble G;
        MDouble B;
        MDouble A;
    };

    /********************************************************
     边框类
    *********************************************************/
    enum IM_BORDER_COLOR_STYLE {
        LINEAR_GRADIENT,
        RADIAL_GRADIENT,
        SOLID,
    };

    class IMBorder {
    public:
        MInt borderTopWidth;
        MInt borderRightWidth;
        MInt borderLeftWidth;
        MInt borderBottomWidth;

        std::shared_ptr<IMColor> borderTopColor;
        std::shared_ptr<IMColor> borderRightColor;
        std::shared_ptr<IMColor> borderLeftColor;
        std::shared_ptr<IMColor> borderBottomColor;

        MInt borderTopLeftRadius;
        MInt borderTopRightRadius;
        MInt borderBottomLeftRadius;
        MInt borderBottomRightRadius;

        IM_BORDER_COLOR_STYLE borderStyle;

        MInt borderRadius;
    };

    /********************************************************
     字体类
    *********************************************************/
    class IMFont {
    public:
        MDouble fontSize;
        MWString fontFamily = L"Microsoft Yahei";
        MInt fontWeight;
        std::shared_ptr<IMColor> fontColor;
        std::shared_ptr<Vector<MWChar>> text;
        MBoolean isItalic = false;
        MInt rowNumber = 1;
        MInt isSingleRow = true;
        MLong fontNumber;
        std::shared_ptr<Vector<D2D1_RECT_F>> textSelectedRect;
        MLong textSelectedStartPos;
        MLong textSelectedEndPos;
        MLong insertStringPos;
        MDouble textWidth;//单行文本专用
        MDouble textHeight;
        MWString primaryText;

        /********************************************************
         高级属性
        *********************************************************/
        std::shared_ptr<IMColor> throughLineColor;
        MDouble throughLineWidth;
        MBoolean activeThroughLine = false;

        std::shared_ptr<IMColor> underLineColor;
        MDouble underLineWidth;
        MBoolean activeUnderLine = false;

        std::shared_ptr<IMColor> fontInnerColor;
        std::shared_ptr<IMColor> fontOuterColor;
        MBoolean activeWireframeMode = false;

    public:
        virtual MWString GetText() {
            std::wstringstream ss;

            for (auto& s : *text) {
                ss << s;
            }

            return ss.str();
        }

        static DWRITE_FONT_WEIGHT ToWeight(MInt weight = 0) {
            MInt ret = std::floor(weight / 100);
            ret = ret * 100 >= 950 ? 950 : (ret * 100 <= 100 ? 100 : ret * 100);

            DWRITE_FONT_WEIGHT retWeight;
            switch (ret) {
                case DWRITE_FONT_WEIGHT_THIN:
                    retWeight = DWRITE_FONT_WEIGHT_THIN;
                    break;
                case DWRITE_FONT_WEIGHT_EXTRA_LIGHT:
                    retWeight = DWRITE_FONT_WEIGHT_EXTRA_LIGHT;
                    break;
                case DWRITE_FONT_WEIGHT_LIGHT:
                    retWeight = DWRITE_FONT_WEIGHT_LIGHT;
                    break;
                case DWRITE_FONT_WEIGHT_SEMI_LIGHT:
                    retWeight = DWRITE_FONT_WEIGHT_SEMI_LIGHT;
                    break;
                case DWRITE_FONT_WEIGHT_MEDIUM:
                    retWeight = DWRITE_FONT_WEIGHT_MEDIUM;
                    break;
                case DWRITE_FONT_WEIGHT_DEMI_BOLD:
                    retWeight = DWRITE_FONT_WEIGHT_DEMI_BOLD;
                    break;
                case DWRITE_FONT_WEIGHT_BOLD:
                    retWeight = DWRITE_FONT_WEIGHT_BOLD;
                    break;
                case DWRITE_FONT_WEIGHT_EXTRA_BOLD:
                    retWeight = DWRITE_FONT_WEIGHT_EXTRA_BOLD;
                    break;
                case DWRITE_FONT_WEIGHT_BLACK:
                    retWeight = DWRITE_FONT_WEIGHT_BLACK;
                    break;
                case DWRITE_FONT_WEIGHT_EXTRA_BLACK:
                    retWeight = DWRITE_FONT_WEIGHT_EXTRA_BLACK;
                    break;
                default:
                    retWeight = DWRITE_FONT_WEIGHT_NORMAL;
                    break;
            }

            return retWeight;
        }

        static DWRITE_FONT_STYLE ToStyle(MBoolean isItalic = false) {
            DWRITE_FONT_STYLE ret = DWRITE_FONT_STYLE_NORMAL;

            if (isItalic) {
                ret = DWRITE_FONT_STYLE_ITALIC;
            }

            return ret;
        }

        /**
         * 暂不支持
         * @return
         */
        static DWRITE_FONT_STRETCH ToStretch() {
            return DWRITE_FONT_STRETCH_NORMAL;
        }
    };

    /********************************************************
     背景类
    *********************************************************/
    class IMBackground {
    public:
        IMBackground() {
            backgroundGradientColor = std::make_shared<Vector<std::shared_ptr<IMColor>>>();
        }

    public:
        std::shared_ptr<IMColor> backgroundColor;
        std::shared_ptr<Vector<std::shared_ptr<IMColor>>> backgroundGradientColor;
        MString backgroundImageUrl;
        MDouble opacity = 1.0f;
    };

    /********************************************************
     滑块类
    *********************************************************/
    class IMScrollBar {
    public:
        std::shared_ptr<IMBackground> background;
        std::shared_ptr<IMBorder> border;
    };

    /********************************************************
     已经存在的监听事件
    *********************************************************/
    class IMEventYet {
    public:
        IMEventYet() {
            m_eventYet = std::make_shared<Vector<std::shared_ptr<EventYet>>>();
        }
    public:
        MBoolean Equal(MInt _iid, MInt _queueId, MInt _event) {
            for (auto& e : *m_eventYet) {
                if (e->iid == _iid && e->queueId == _queueId && e->event == _event)
                    return true;
            }

            return false;
        }

        void Add(MInt _iid, MInt _queueId, MInt _event) {
            if (Equal(_iid, _queueId, _event)) return;

            std::shared_ptr<EventYet> eventYet = std::make_shared<EventYet>();
            eventYet->iid = _iid;
            eventYet->queueId = _queueId;
            eventYet->event = _event;

            m_eventYet->push_back(eventYet);
        }

        void Del(MInt _iid, MInt _queueId, MInt _event) {
            for (auto i = m_eventYet->begin(); i != m_eventYet->end();++i) {
                if ((*i)->iid == _iid && (*i)->queueId == _queueId && (*i)->event == _event) {
                    m_eventYet->erase(i);
                    break;
                }
            }
        }
    private:
        class EventYet {
        public:
            MInt iid;
            MInt queueId;
            MInt event;
        };
    private:
        std::shared_ptr<Vector<std::shared_ptr<EventYet>>> m_eventYet;
    };

    /********************************************************
     元素帧
    *********************************************************/
    class IMFrame {
    public:
        /// <summary>
        /// 内部使用请勿直接赋值更改
        /// </summary>
        MDouble offX = 0;
        MDouble offY = 0;
        MDouble maxOffX = 0;
        MDouble maxOffY = 0;
        MDouble textOffX = 0;
        MDouble textOffY = 0;
        MInt ref = 0;
        MInt event = 0;
        MBoolean isInput = false;
        MDouble caretPosX = 0;
        MDouble caretPosY = 0;

        /// <summary>
        /// 内部赋值请勿赋值
        /// </summary>
        std::shared_ptr<IMEventYet> eventYet;
        CComPtr<IDWriteTextFormat> textFormat;
        CComPtr<IDWriteTextLayout> textLayout;
        CComPtr<IDWriteFactory> writeFactory;
        MBoolean isClkDown = false;
        /// <summary>
        /// 文本点击位置
        /// </summary>
        IMPoint clkDownPoint;
        IMPoint clkUpPoint;

        /// <summary>
        /// 外部使用
        /// </summary>
        IM_FRAME_VERSION major;
        IM_FRAME_VERSION minor;
        MLong deep;
        MBoolean isFocus;

        MString uuid;
        MLong zIndex = 0;

        MDouble width;
        MDouble height;

        MDouble innerX;
        MDouble innerY;
        MDouble outerX;
        MDouble outerY;

        MDouble marginTop;
        MDouble marginBottom;
        MDouble marginLeft;
        MDouble marginRight;

        MDouble paddingTop;
        MDouble paddingBottom;
        MDouble paddingLeft;
        MDouble paddingRight;

        MDouble left;
        MDouble right;
        MDouble top;
        MDouble bottom;

        MDouble opacity = 1.0f;

        std::shared_ptr<IMBorder> border;

        std::shared_ptr<IMFont> font;

        std::shared_ptr<IMBackground> background;

        IM_FRAME_OVERFLOW overflow;
        std::shared_ptr<IMScrollBar> scrollBar;

        IM_FRAME_POSITION position;
        IM_FRAME_DISPLAY display;
    };

    /********************************************************
     字体列表
    *********************************************************/
    class IMFontCollection {
    public:
        IMFontCollection() {
            m_names = std::make_shared<Vector<MString>>();
        }

    public:
        [[nodiscard]] MInt getCount() const {
            return m_names->size();
        }

        std::shared_ptr<Vector<MString>> getNames() {
            return m_names;
        }

        void AddName(MString name) {
            m_names->push_back(std::move(name));
        }

    private:
        std::shared_ptr<Vector<MString>> m_names;
    };


    /********************************************************
     焦点管理器
    *********************************************************/
    class IMFocusManager {
    public:
        static std::shared_ptr<IMFocusManager> GetInstance() {
            return MWD::GM::IMSingletonPtr<IMFocusManager>::GetInstance();
        }

        static void SetInstance(const std::shared_ptr<IMFocusManager>& manager) {
            MWD::GM::IMSingletonPtr<IMFocusManager>::SetInstance(manager);
        }

    public:
        IMFocusManager() {
            m_focusFrameItems = std::make_shared<Map<MString, std::shared_ptr<IMFrame>>>();
        }

        virtual ~IMFocusManager() = default;

    public:
        void SetFocusFrame(const MString& route, const std::shared_ptr<IMFrame>& frame) {
            auto item = m_focusFrameItems->find(route);
            if (item == m_focusFrameItems->end()) {
                m_focusFrameItems->insert(std::make_pair(route, frame));
                frame->isFocus = true;
                return;
            }

            item->second->isFocus = false;
            item->second->font->primaryText.clear();
            item->second = frame;
            item->second->isFocus = true;
        }

        std::shared_ptr<IMFrame> GetFocusFrame(const MString& route) {
            auto item = m_focusFrameItems->find(route);
            if (item != m_focusFrameItems->end()) {
                return item->second;
            }
            return nullptr;
        }

    private:
        std::shared_ptr<Map<MString, std::shared_ptr<IMFrame>>> m_focusFrameItems;
    };

}//MWD::Frame
