#include "IMRenderItem.h"
#include "IModule.h"

namespace MWD::Render {
    using namespace MWD::Module;

    //////////////////////////////////////////////////
    // 背景渲染器
    /// <summary>
    /// 背景渲染器miid
    /// </summary>
    MIID MIID_IM_BACKGROUND_RENDER_ITEM_MODULE{
            {'\0', '\0', '\0', '\0', '\0', '\0', '\0', 1},
            {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
             '\0', '\0', '\0', '\0', '\0','\0','\0', 1}

    };
    /********************************************************
     IModule方法实现
    *********************************************************/
    /// <summary>
    /// 测试是否可转
    /// </summary>
    MBoolean STDMETHODCALLTYPE MBackgroundRenderItem::QueryInterface(
            /* [in] */ MIID miid
    ) {
        return miid == MIID_IM_BACKGROUND_RENDER_ITEM_MODULE;
    }

    /// <summary>
    /// 获取iid
    /// </summary>
    MIID STDMETHODCALLTYPE MBackgroundRenderItem::GetMIID() {
        return MIID_IM_BACKGROUND_RENDER_ITEM_MODULE;
    }

    /********************************************************
     IMRenderItem方法实现
    *********************************************************/
    void STDMETHODCALLTYPE MBackgroundRenderItem::Init(
            /* [in][元素帧属性] */const IMFrame::ptr&,
            /* [in][渲染器渲染必要对象] */const IMRenderItemEvent::ptr&) {
        m_isInit = false;
    }

    void STDMETHODCALLTYPE MBackgroundRenderItem::Render(
            /* [in][元素帧属性] */const std::shared_ptr<IMFrame>& frame,
            /* [in][渲染器渲染必要对象] */const IMRenderItemEvent::ptr& e
    ) {
        CComPtr<ID2D1SolidColorBrush> pBrush;

        e->m_pRT->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF(
                        frame->background->backgroundColor->R,
                        frame->background->backgroundColor->G,
                        frame->background->backgroundColor->B,
                        frame->background->backgroundColor->A)),
                &pBrush
        );
        e->m_pRT->FillRoundedRectangle(
                D2D1::RoundedRect(D2D1::RectF(frame->innerX + frame->offX, frame->innerY + frame->offY,
                                              frame->innerX + frame->width + frame->offX + frame->paddingLeft +
                                              frame->paddingRight,
                                              frame->innerY + frame->height + frame->offY + frame->paddingTop +
                                              frame->paddingBottom),
                                  frame->border->borderRadius, frame->border->borderRadius), pBrush
        );

        // 渲染选中的文本背景颜色
        if (!frame->font->textSelectedRect->empty()
            && frame->isInput) {
            auto iter = frame->font->textSelectedRect->begin();
            CComPtr<ID2D1SolidColorBrush> fontBackBrush;
            e->m_pRT->CreateSolidColorBrush(
                    D2D1::ColorF(0, 0, .8, .4),
                    &fontBackBrush
            );
            for (; iter != frame->font->textSelectedRect->end();
                   ++iter) {
                D2D1_RECT_F rect = *iter;
                if (iter->left < frame->innerX + frame->paddingLeft + frame->offX)
                    rect.left = frame->innerX + frame->paddingLeft;

                if (iter->top < frame->innerY + frame->paddingTop + frame->offY)
                    rect.top = frame->innerY + frame->paddingTop;

                if (iter->right >
                    frame->innerX + frame->paddingLeft + frame->paddingRight + frame->width + frame->offX)
                    rect.right = frame->innerX + frame->paddingLeft + frame->paddingRight + frame->width + frame->offX;

                if (iter->bottom >
                    frame->innerY + frame->paddingTop + frame->paddingBottom + frame->height + frame->offY)
                    rect.bottom =
                            frame->innerY + frame->paddingTop + frame->paddingBottom + frame->height + frame->offY;

                e->m_pRT->FillRectangle(rect, fontBackBrush);
            }
        }
    }

} // MWD::Render

//////////////////////////////////////////////////
// 导出函数
GM_DEFINE_LOAD_FUNC_AND_DESTROY(MWD::Render::MBackgroundRenderItem, MWD::Render::MIID_IM_BACKGROUND_RENDER_ITEM_MODULE)
