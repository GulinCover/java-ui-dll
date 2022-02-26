#include "IMWriter.h"
#include "IModule.h"
#include "IMRenderItem.h"

namespace MWD::Render {
    MWD::Module::MIID MIID_IM_INPUT_RENDER_ITEM_MODULE {
            {'\0', '\0', '\0', '\0', '\0', '\0', '\0', 1},
            {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
             '\0', '\0', '\0', '\0', '\0','\0','\0', 2}

    };
    /********************************************************
     IModule方法实现
    *********************************************************/
    /// <summary>
    /// 测试是否可转
    /// </summary>
    MBoolean STDMETHODCALLTYPE MInputRenderItem::QueryInterface(
            /* [in] */ MIID miid
    ) {
        return miid == MIID_IM_INPUT_RENDER_ITEM_MODULE;
    }

    /// <summary>
    /// 获取iid
    /// </summary>
    MIID STDMETHODCALLTYPE MInputRenderItem::GetMIID() {
        return MIID_IM_INPUT_RENDER_ITEM_MODULE;
    }
    /********************************************************
     IMRenderItem方法实现
    *********************************************************/
    /// <summary>
    /// 初始化
    /// </summary>
    void STDMETHODCALLTYPE MInputRenderItem::Init(
            /* [in][元素帧属性] */const IMFrame::ptr&,
            /* [in][渲染器渲染必要对象] */const IMRenderItemEvent::ptr&
    ) {
        // 初始化文本操作渲染器
        m_writerRenderItem = CComPtr<IMWriter>(new IMWriter) ;
    }

    /// <summary>
    /// 执行渲染,渲染文本
    /// </summary>
    void STDMETHODCALLTYPE MInputRenderItem::Render(
            /* [in][元素帧属性] */const IMFrame::ptr& frame,
            /* [in][渲染器渲染必要对象] */const IMRenderItemEvent::ptr& e
    ) {
        CComPtr<ID2D1Brush> brush;
        std::shared_ptr<IMColor> color = frame->font->fontColor;
        e->m_pRT->CreateSolidColorBrush(D2D1::ColorF(color->R, color->G, color->B, color->A),
                                       (ID2D1SolidColorBrush**) (&brush));

        m_writerRenderItem->SetFillBrush(brush);
        m_writerRenderItem->SetOutlineBrush(brush);

        if (frame->font->isSingleRow)
            frame->font->textLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);


        if (!frame->font->isActiveStageProperties) {
            frame->font->textLayout->Draw(nullptr, m_writerRenderItem, frame->innerX + frame->paddingLeft + frame->offX + frame->textOffX,
                                          frame->innerY + frame->paddingTop + frame->offY + frame->textOffY);
            return;
        }

        // 多文本片段分段渲染
        for (auto iter = frame->font->stageProperties->begin();iter != frame->font->stageProperties->end();++iter) {

        }
    }
} // MWD::Render

GM_DEFINE_LOAD_FUNC_AND_DESTROY(MWD::Render::MInputRenderItem, MWD::Render::MIID_IM_INPUT_RENDER_ITEM_MODULE)

