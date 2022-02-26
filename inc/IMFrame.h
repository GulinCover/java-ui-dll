#pragma once

#include "IMCommon.h"

namespace MWD::Frame {
    //////////////////////////////////////////////////
    // 点
    struct IMPoint {
        MDouble x;
        MDouble y;
    };

    /********************************************************
     帧版本
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
    /********************************************************
     布局属性
    *********************************************************/
    enum IM_FRAME_POSITION {
        POSITION_RELATIVE = 1,
        POSITION_ABSOLUTE = 2,
    };

    /********************************************************
     排版属性
    *********************************************************/
    enum IM_FRAME_DISPLAY {
        STATIC = 1,
        FLEX = 2,
        GRID = 3,
        NONE = 4
    };
    /********************************************************
     溢出属性
    *********************************************************/
    enum IM_FRAME_OVERFLOW {
        HIDDEN = 1,
        HIDDEN_X_SCROLL_BAR = 2,
        HIDDEN_Y_SCROLL_BAR = 4,
        OVERFLOW_NONE = 8
    };

    //////////////////////////////////////////////////
    // 颜色属性
    class IMColor {
    public:
        using ptr = std::shared_ptr<IMColor>;
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

    //////////////////////////////////////////////////
    // 边框属性
    class IMBorder {
    public:
        using ptr = std::shared_ptr<IMBorder>;
    public:
        /********************************************************
         边框线条样式
        *********************************************************/
        enum IM_BORDER_COLOR_STYLE {
            LINEAR_GRADIENT,
            RADIAL_GRADIENT,
            SOLID,
        };
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

    //////////////////////////////////////////////////
    // 字体属性
    abstract IMFont {
    public:
        using ptr = std::shared_ptr<IMFont>;
    public:
        /********************************************************
         字体片段属性样式合集
        *********************************************************/
        abstract IMFontStageStyleCollection {
        public:
            using ptr = std::shared_ptr<IMFontStageStyleCollection>;
        public:
            virtual ~IMFontStageStyleCollection() = default;
        public:
            CComPtr<IDWriteTextFormat> textFormat;
            CComPtr<IDWriteTextLayout> textLayout;
        };
        /********************************************************
         字体片段属性
        *********************************************************/
        abstract IMFontStage {
        public:
            using ptr = std::shared_ptr<IMFontStage>;
        public:
            virtual ~IMFontStage() = default;
        public:
            MLong textStart;
            MLong textEnd;
            IMColor::ptr color;
            IM_FRAME_DISPLAY display;
            IMFont::ptr proxyText;
            MWString alternateText;
            MInt styleId;
        };
    public:
        virtual ~IMFont() = default;
    public:
        VectorPtr<MWChar> text;

        MDouble fontSize;
        MWString fontFamily = L"Microsoft Yahei";
        MInt fontWeight;
        IMColor::ptr fontColor;
        MBoolean isItalic = false;
        MInt isSingleRow = true;
        MLong fontNumber;
        VectorPtr<D2D1_RECT_F> textSelectedRect;
        MLong textSelectedStartPos;
        MLong textSelectedEndPos;
        MLong insertStringPos;
        MDouble textWidth;//单行文本专用
        MDouble textHeight;
        MWString primaryText;

        VectorPtr<IMFontStage::ptr> stageProperties;
        VectorPtr<IMFontStageStyleCollection::ptr> stageStyleCollection;
        MBoolean isActiveStageProperties = false;

        /********************************************************
         高级属性
        *********************************************************/
        IMColor::ptr throughLineColor;
        MDouble throughLineWidth;
        MBoolean activeThroughLine = false;

        IMColor::ptr underLineColor;
        MDouble underLineWidth;
        MBoolean activeUnderLine = false;

        IMColor::ptr fontInnerColor;
        IMColor::ptr fontOuterColor;
        MBoolean activeWireframeMode = false;

        /********************************************************
         内部使用请勿直接赋值
        *********************************************************/
        MInt rowNumber = 1;
        CComPtr<IDWriteTextFormat> textFormat;
        CComPtr<IDWriteTextLayout> textLayout;
        CComPtr<IDWriteFactory> writeFactory;

    public:
        virtual MWString STDMETHODCALLTYPE GetText() {
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

    //////////////////////////////////////////////////
    // 背景属性
    class IMBackground {
    public:
        using ptr = std::shared_ptr<IMBackground>;
    public:
        IMBackground() {
            backgroundGradientColor = MakeShared(Vector<IMColor::ptr>);
        }

    public:
        IMColor::ptr backgroundColor;
        VectorPtr<IMColor::ptr> backgroundGradientColor;
        MString backgroundImageUrl;
        MDouble opacity = 1.0f;
    };

    //////////////////////////////////////////////////
    // 滑块属性
    class IMScrollBar {
    public:
        using ptr = std::shared_ptr<IMScrollBar>;
    public:
        IMBackground::ptr background;
        IMBorder::ptr border;
    };

    //////////////////////////////////////////////////
    // 已监听的事件属性
    class IMEventYet {
    public:
        using ptr = std::shared_ptr<IMEventYet>;
    public:
        class EventYet {
        public:
            using ptr = std::shared_ptr<EventYet>;
        public:
            MInt sid;
            MInt queueId;
            MInt event;
        };

    public:
        IMEventYet() {
            m_eventYet = MakeShared(Vector<EventYet::ptr>);
        }

    public:
        /// <summary>
        /// 判断是否有监听
        /// </summary>
        MBoolean Equal(MInt _sid, MInt _queueId, MInt _event) {
            for (auto& e : *m_eventYet) {
                if (e->sid == _sid && e->queueId == _queueId && e->event == _event)
                    return true;
            }

            return false;
        }

        /// <summary>
        /// 添加监听
        /// </summary>
        void Add(MInt _sid, MInt _queueId, MInt _event) {
            if (Equal(_sid, _queueId, _event)) return;

            EventYet::ptr eventYet = MakeShared(EventYet);
            eventYet->sid = _sid;
            eventYet->queueId = _queueId;
            eventYet->event = _event;

            m_eventYet->push_back(eventYet);
        }

        /// <summary>
        /// 删除监听
        /// </summary>
        void Del(MInt _sid, MInt _queueId, MInt _event) {
            for (auto i = m_eventYet->begin(); i != m_eventYet->end(); ++i) {
                if ((*i)->sid == _sid && (*i)->queueId == _queueId && (*i)->event == _event) {
                    m_eventYet->erase(i);
                    break;
                }
            }
        }

    private:
        VectorPtr<EventYet::ptr> m_eventYet;
    };

    //////////////////////////////////////////////////
    // 渲染元素帧
    class IMFrame {
    public:
        using ptr = std::shared_ptr<IMFrame>;
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
        IMEventYet::ptr eventYet;
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
        IM_FRAME_OVERFLOW overflow;
        IM_FRAME_POSITION position;
        IM_FRAME_DISPLAY display;
        MLong deep;
        MBoolean isFocus;

        MString fid;
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
        /// <summary>
        /// 边框属性
        /// </summary>
        IMBorder::ptr border;
        /// <summary>
        /// 字体属性
        /// </summary>
        IMFont::ptr font;
        /// <summary>
        /// 背景属性
        /// </summary>
        IMBackground::ptr background;
        /// <summary>
        /// 滑块属性
        /// </summary>
        IMScrollBar::ptr scrollBar;
    };

    //////////////////////////////////////////////////
    // 字体合集
    class IMFontCollection {
    public:
        using ptr = std::shared_ptr<IMFontCollection>;
    public:
        IMFontCollection() {
            m_names = MakeShared(Vector<MString>);
        }

    public:
        /// <summary>
        /// 获取数量
        /// </summary>
        MInt getCount() {
            return m_names->size();
        }

        /// <summary>
        /// 获取字体集合所有名称
        /// </summary>
        VectorPtr<MString> getNames() {
            return m_names;
        }

        /// <summary>
        /// 添加字体
        /// </summary>
        void AddName(MString name) {
            m_names->push_back(std::move(name));
        }

    private:
        VectorPtr<MString> m_names;
    };

    //////////////////////////////////////////////////
    // 焦点管理器
    abstract IMFocusManager {
    public:
        using ptr = std::shared_ptr<IMFocusManager>;
    public:
        //////////////////////////////////////////////////
        // 获取单例
        static IMFocusManager::ptr GetInstance() {
            if (s_self) {
                return s_self;
            }
            s_self = MakeShared(IMFocusManager);
            return s_self;
        }
        //////////////////////////////////////////////////
        // 设置单例
        static void SetInstance(const IMFocusManager::ptr& manager) {
            s_self = manager;
        }

    private:
        static IMFocusManager::ptr s_self;
    public:
        IMFocusManager() {
            m_focusFrameItems = MakeMap(MString, IMFrame::ptr);
        };
    public:
        virtual ~IMFocusManager() = default;

    public:
        /// <summary>
        /// 设置聚焦元素帧
        /// </summary>
        virtual void STDMETHODCALLTYPE SetFocusFrame(
                /* [in] */const MString& route,
                /* [in] */const IMFrame::ptr& frame) {
            auto item = m_focusFrameItems->find(route);
            if (item == m_focusFrameItems->end()) {
                m_focusFrameItems->insert(MakePair(route, frame));
                frame->isFocus = true;
                return;
            }

            item->second->isFocus = false;
            item->second->font->primaryText.clear();
            item->second = frame;
            item->second->isFocus = true;
        }

        /// <summary>
        /// 获取当前聚焦元素帧
        /// </summary>
        virtual IMFrame::ptr STDMETHODCALLTYPE GetFocusFrame(
                /* [in] */const MString& route) {
            auto item = m_focusFrameItems->find(route);
            if (item != m_focusFrameItems->end()) {
                return item->second;
            }
            return nullptr;
        }

    private:
        MapPtr<MString, IMFrame::ptr> m_focusFrameItems;
    };

    static IMFocusManager::ptr s_self = nullptr;
} // MWD::Frame












