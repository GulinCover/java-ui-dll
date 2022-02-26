#pragma once

#include "IMRender.h"
#include "IMWriter.h"

namespace MWD::Render {
    using namespace MWD::Writer;
    //////////////////////////////////////////////////
    // 背景渲染器
    class MBackgroundRenderItem : public IMRenderItem {
    public:
        using ptr = std::shared_ptr<MBackgroundRenderItem>;
    public:
        ~MBackgroundRenderItem() override = default;

    public:
        /********************************************************
         IModule方法实现
        *********************************************************/
        /// <summary>
        /// 测试是否可转
        /// </summary>
        MBoolean STDMETHODCALLTYPE QueryInterface(
                /* [in] */ MIID miid
        ) override;

        /// <summary>
        /// 获取iid
        /// </summary>
        MIID STDMETHODCALLTYPE GetMIID() override;

        /********************************************************
         IMRenderItem方法实现
        *********************************************************/
        void STDMETHODCALLTYPE Init(
                /* [in][元素帧属性] */const IMFrame::ptr&,
                /* [in][渲染器渲染必要对象] */const IMRenderItemEvent::ptr&)
        override;

        void STDMETHODCALLTYPE Render(
                /* [in][元素帧属性] */const std::shared_ptr<IMFrame>& frame,
                /* [in][渲染器渲染必要对象] */const IMRenderItemEvent::ptr& e
        ) override;
    };

    //////////////////////////////////////////////////
    // 文本编辑和文本显示渲染器
    class MInputRenderItem : public IMRenderItem {
    public:
        using ptr = std::shared_ptr<MInputRenderItem>;
    public:
        /********************************************************
         IModule方法实现
        *********************************************************/
        /// <summary>
        /// 测试是否可转
        /// </summary>
        MBoolean STDMETHODCALLTYPE QueryInterface(
                /* [in] */ MIID miid
        ) override;

        /// <summary>
        /// 获取iid
        /// </summary>
        MIID STDMETHODCALLTYPE GetMIID() override;
    public:
        /********************************************************
         IMRenderItem方法实现
        *********************************************************/
        /// <summary>
        /// 初始化
        /// </summary>
        void STDMETHODCALLTYPE Init(
                /* [in][元素帧属性] */const IMFrame::ptr&,
                /* [in][渲染器渲染必要对象] */const IMRenderItemEvent::ptr&
        ) override;

        /// <summary>
        /// 执行渲染
        /// </summary>
        void STDMETHODCALLTYPE Render(
                /* [in][元素帧属性] */const IMFrame::ptr&,
                /* [in][渲染器渲染必要对象] */const IMRenderItemEvent::ptr&
        ) override;
    private:
        CComPtr<IMWriter> m_writerRenderItem;
    };
} // MWD::Render


