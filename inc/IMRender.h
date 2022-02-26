#pragma once

#include "IModule.h"
#include "IMFrame.h"

namespace MWD::Render {
    using namespace MWD::Module;
    using namespace MWD::Frame;

    //////////////////////////////////////////////////
    // 渲染器参数
    abstract IMRenderItemEvent {
    public:
        using ptr = std::shared_ptr<IMRenderItemEvent>;
    public:
        IMRenderItemEvent() {
            m_pD2DFactory = nullptr;
            m_pRT = nullptr;
            m_pWicFactory = nullptr;
            //m_monitor = nullptr;
            m_hwnd = nullptr;
        }

        virtual ~IMRenderItemEvent() = default;

    public:
        CComPtr<ID2D1Factory> m_pD2DFactory;
        CComPtr<ID2D1HwndRenderTarget> m_pRT;
        CComPtr<IWICImagingFactory> m_pWicFactory;

        //std::shared_ptr<IMonitor> m_monitor;

        HWND m_hwnd;

        MString m_route;
    };

    //////////////////////////////////////////////////
    // 渲染器基类
    abstract IMRenderItem : public IModule {
    public:
        using ptr = std::shared_ptr<IMRenderItem>;
    public:
        /// <summary>
        /// 初始化
        /// </summary>
        virtual void STDMETHODCALLTYPE Init(
                /* [in][元素帧属性] */const IMFrame::ptr&,
                /* [in][渲染器渲染必要对象] */const IMRenderItemEvent::ptr&
        ) PURE;

        /// <summary>
        /// 执行渲染
        /// </summary>
        virtual void STDMETHODCALLTYPE Render(
                /* [in][元素帧属性] */const IMFrame::ptr&,
                /* [in][渲染器渲染必要对象] */const IMRenderItemEvent::ptr&
        ) PURE;

        /// <summary>
        /// 决定是否多次初始化
        /// </summary>
        virtual MBoolean STDMETHODCALLTYPE IsInit() {
            return m_isInit;
        }

    protected:
        MBoolean m_isInit = true;
    };

    //////////////////////////////////////////////////
    // 特殊渲染器基类
    abstract IMSpecialRenderItem : public IMRenderItem {
    public:
        using ptr = std::shared_ptr<IMSpecialRenderItem>;
    public:
        /// <summary>
        /// 渲染前执行
        /// </summary>
        virtual void STDMETHODCALLTYPE RenderBefore(
                /* [in][元素帧属性] */const IMFrame::ptr&,
                /* [in][渲染器渲染必要对象] */const IMRenderItemEvent::ptr&
        ) PURE;

        /// <summary>
        /// 渲染后执行
        /// </summary>
        virtual void STDMETHODCALLTYPE RenderAfter(
                /* [in][元素帧属性] */const IMFrame::ptr&,
                /* [in][渲染器渲染必要对象] */const IMRenderItemEvent::ptr&
        ) PURE;
    };

    //////////////////////////////////////////////////
    // 高级渲染器基类
    abstract IMAdvancedRenderItem : public IMSpecialRenderItem {
    public:
        using ptr = std::shared_ptr<IMAdvancedRenderItem>;
    public:
        /// <summary>
        /// 渲染前执行
        /// </summary>
        virtual void STDMETHODCALLTYPE ExecAdvance(
                /* [in][元素帧属性] */const IMFrame::ptr&,
                /* [in][渲染器渲染必要对象] */const IMRenderItemEvent::ptr&
        ) PURE;
    };
} // MWD::Render