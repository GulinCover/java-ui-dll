#pragma once

#include "IMCommon.h"
#include "IModule.h"
#include "IMFrame.h"
#include "IMRender.h"
#include "IMRenderItem.h"
#include "IMWriter.h"
#include "IMService.h"

namespace MWD::Render {
    using namespace MWD::Frame;
    using namespace MWD::Module;
    using namespace MWD::Writer;
    using namespace MWD::Service;
    //////////////////////////////////////////////////
    // 渲染器队列基类
    interface IMRenderQueueService : public IMAdvancedService {
    public:
        using ptr = std::shared_ptr<IMRenderQueueService>;
    public:
        /// <summary>
        /// 初始化
        /// </summary>
        virtual void STDMETHODCALLTYPE Init(
                /* [in] */HWND hwnd
        ) PURE;

        /// <summary>
        /// 默认模块依赖
        /// </summary>
        virtual void STDMETHODCALLTYPE DefaultDependModule() PURE;

        /// <summary>
        /// 自定义模块依赖
        /// </summary>
        virtual void STDMETHODCALLTYPE CustomDependModule(
                /* [in] */VectorPtr<MIID> vec
        ) PURE;

        /// <summary>
        /// 渲染元素帧
        /// </summary>
        virtual void STDMETHODCALLTYPE Render(
                /* [in] */const IMFrame::ptr& f
        ) PURE;

        /// <summary>
        /// 打开画布准备渲染
        /// </summary>
        virtual void STDMETHODCALLTYPE DrawBegin() PURE;

        /// <summary>
        /// 关闭画布开始渲染
        /// </summary>
        virtual void STDMETHODCALLTYPE DrawEnd() PURE;
    };

    //////////////////////////////////////////////////
    // 渲染器队列Impl
    class MRenderItemQueue : public IMRenderQueueService {
    public:
        /********************************************************
         IModule实现
        *********************************************************/
        /// <summary>
        /// 测试是否可转
        /// </summary>
        MBoolean STDMETHODCALLTYPE QueryInterface(
                /* [in] */ SIID siid
        ) override;

        /// <summary>
        /// 获取iid
        /// </summary>
        SIID STDMETHODCALLTYPE GetSIID(
                /* [in] */char Data2[16]
        ) override;

        /// <summary>
        /// 添加模块
        /// </summary>
        MBoolean STDMETHODCALLTYPE AppendModule(
                /* [in] */MIID miid,
                /* [in] */IModule::ptr module
        ) override;

        /// <summary>
        /// 获取需要添加的module的miid
        /// </summary>
        VectorPtr<MIID> STDMETHODCALLTYPE GetMIIDS(
        ) override;

        /********************************************************
         IMRenderItemQueue实现
        *********************************************************/
        /// <summary>
        /// 初始化
        /// </summary>
        void STDMETHODCALLTYPE Init(
                /* [in] */HWND hwnd
        ) override;

        /// <summary>
        /// 默认模块依赖
        /// </summary>
        void STDMETHODCALLTYPE DefaultDependModule() override;

        /// <summary>
        /// 自定义模块依赖
        /// </summary>
        void STDMETHODCALLTYPE CustomDependModule(
                /* [in] */VectorPtr<MIID> vec
        ) override;

        /// <summary>
        /// 渲染元素帧
        /// </summary>
        void STDMETHODCALLTYPE Render(
                /* [in] */const IMFrame::ptr& frame
        ) override;

        /// <summary>
        /// 打开画布准备渲染
        /// </summary>
        void STDMETHODCALLTYPE DrawBegin() override;

        /// <summary>
        /// 关闭画布开始渲染
        /// </summary>
        void STDMETHODCALLTYPE DrawEnd() override;

    private:
        /// <summary>
        /// 当前窗口大小
        /// </summary>
        RECT m_rect;
        /// <summary>
        /// 当前窗口句柄
        /// </summary>
        HWND m_hwnd;
        /// <summary>
        /// d2d工厂
        /// </summary>
        CComPtr<ID2D1Factory> m_pD2DFactory;
        /// <summary>
        /// 渲染对象
        /// </summary>
        CComPtr<ID2D1HwndRenderTarget> m_pRT;
        /// <summary>
        /// renderItem所需参数
        /// </summary>
        IMRenderItemEvent::ptr m_sharedRenderParam;
        /// <summary>
        /// 文字相关工厂
        /// </summary>
        CComPtr<IDWriteFactory> m_pWriteFactory;
        /// <summary>
        /// 自定义文字渲染器
        /// </summary>
        CComPtr<IMWriter> m_imWriter;
        /// <summary>
        /// 图片相关工厂
        /// </summary>
        CComPtr<IWICImagingFactory> m_pWicFactory;
        /// <summary>
        /// 特殊renderItem
        /// </summary>
        MapPtr<MString, IMSpecialRenderItem::ptr> m_specialRenderItems;
        /// <summary>
        /// renderItem
        /// </summary>
        MapPtr<MString, IMRenderItem::ptr> m_renderItems;
        /// <summary>
        /// renderItem执行顺序
        /// </summary>
        MString m_itemManifest[6] = {
                "background_render",
                "image_render",
                "border_render",
                "text_render",
                "input_render",
                "scroll_bar_render"};

        MapPtr<MIID, IModule::ptr> m_srcModuleItems;
        VectorPtr<MIID> m_dependencies;
    };

} // MWD::Render