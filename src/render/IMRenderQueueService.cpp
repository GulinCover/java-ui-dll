#include "IMRenderQueueService.h"

namespace MWD::Render {
    /// <summary>
    /// 背景渲染器miid
    /// </summary>
    MIID MIID_IM_BACKGROUND_RENDER_ITEM_MODULE{
            {'\0', '\0', '\0', '\0', '\0', '\0', '\0', 1},
            {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
             '\0', '\0', '\0', '\0', '\0', '\0', '\0', 1}
    };
    //////////////////////////////////////////////////
    // 渲染器队列Impl
    /// <summary>
    /// 背景渲染器miid
    /// </summary>
    SIID SIID_IM_RENDER_QUEUE_SERVICE{
            {0, 0, 0, 0, 0, 0, 0, 1},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}
    };
    /********************************************************
     IModule方法实现
    *********************************************************/
    /// <summary>
    /// 测试是否可转
    /// </summary>
    MBoolean STDMETHODCALLTYPE MRenderItemQueue::QueryInterface(
            /* [in] */ SIID siid
    ) {
        return siid == SIID_IM_RENDER_QUEUE_SERVICE;
    }

    /// <summary>
    /// 获取iid
    /// </summary>
    SIID STDMETHODCALLTYPE MRenderItemQueue::GetSIID(
            /* [in] */char Data2[16]
    ) {
        MInt ret = 0;
        for (MInt i = 0; i < sizeof(SIID::Data2); ++i) {
            if (Data2[i] == SIID_IM_RENDER_QUEUE_SERVICE.Data2[i]) {
                ++ret;
            } else {
                break;
            }
        }
        if (ret == sizeof(SIID::Data2))
            return SIID_IM_RENDER_QUEUE_SERVICE;
        return SIID_SERVICE_INVALID;
    }
    /// <summary>
    /// 添加模块
    /// </summary>
    MBoolean STDMETHODCALLTYPE MRenderItemQueue::AppendModule(
            /* [in] */MIID miid,
            /* [in] */IModule::ptr module
    ) {
        for (auto iter = m_srcModuleItems->begin(); iter != m_srcModuleItems->end(); ++iter) {
            if (iter->second->GetMIID() == miid) {
                return false;
            }
        }

        m_srcModuleItems->insert(MakePair(miid, module));
        return true;
    }

    /// <summary>
    /// 获取需要添加的module的miid
    /// </summary>
    VectorPtr<MIID> STDMETHODCALLTYPE MRenderItemQueue::GetMIIDS(
    ) {
        return m_dependencies;
    }

    /********************************************************
     IMRenderItemQueue方法实现
    *********************************************************/
    /// <summary>
    /// 初始化
    /// </summary>
    void STDMETHODCALLTYPE MRenderItemQueue::Init(
            /* [in] */HWND hwnd
    ) {
        ACCESS(!hwnd);
        m_hwnd = hwnd;

        // d2d
        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
        ACCESS(SUCCEEDED(hr));
        GetClientRect(m_hwnd, &m_rect);
        hr = m_pD2DFactory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(
                        m_hwnd, D2D1::SizeU(m_rect.right - m_rect.left, m_rect.bottom - m_rect.top)
                ),
                &m_pRT);
        ACCESS(SUCCEEDED(hr));

        // 图片相关
        hr = CoCreateInstance(
                CLSID_WICImagingFactory,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_IWICImagingFactory,
                (LPVOID*) &m_pWicFactory);
        ACCESS(SUCCEEDED(hr));

        // 文字渲染工厂
        hr = DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(&m_pWriteFactory)
        );
        ACCESS(SUCCEEDED(hr));

        // 自定义文字渲染器
        m_imWriter = CComPtr<IMWriter>(new IMWriter());
        m_imWriter->Init(m_pD2DFactory, m_pRT);

        // 默认抗锯齿文字和
        m_pRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        m_pRT->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_DEFAULT);

        // 渲染器参数赋值
        m_sharedRenderParam->m_pRT = m_pRT;
        m_sharedRenderParam->m_pD2DFactory = m_pD2DFactory;
        m_sharedRenderParam->m_pWicFactory = m_pWicFactory;
        m_sharedRenderParam->m_hwnd = m_hwnd;

        //m_sharedRenderParam->m_route = m_route;
        //m_sharedRenderParam->m_editor = m_editor;
        //m_sharedRenderParam->m_monitor = m_monitor;

        // 初始化renderItem
        m_renderItems = MakeMap(MString, IMRenderItem::ptr);
        // 初始化特殊renderItem
        m_specialRenderItems = MakeMap(MString, IMSpecialRenderItem::ptr);
        // 初始化原始模块moduleItem
        m_srcModuleItems = MakeMap(MIID, IModule::ptr);
        // 初始化模块依赖miid
        m_dependencies = MakeVec(MIID);
    }

    /// <summary>
    /// 默认模块依赖
    /// </summary>
    void STDMETHODCALLTYPE MRenderItemQueue::DefaultDependModule() {
        m_dependencies->push_back(MIID_IM_BACKGROUND_RENDER_ITEM_MODULE);
    }

    /// <summary>
    /// 自定义模块依赖
    /// </summary>
    void STDMETHODCALLTYPE MRenderItemQueue::CustomDependModule(
            /* [in] */VectorPtr<MIID> vec
    ) {
        m_dependencies = vec;
    }

    /// <summary>
    /// 渲染元素帧
    /// </summary>
    void STDMETHODCALLTYPE MRenderItemQueue::Render(
            /* [in] */const IMFrame::ptr& frame
    ) {
        if (!frame->font->writeFactory
            || !frame->font->textFormat
            || !frame->font->textLayout) {
            frame->font->writeFactory = m_pWriteFactory;

            CComPtr<IDWriteTextFormat> textFormat;
            CComPtr<IDWriteTextLayout> textLayout;

            // 创建textFormat
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

            // 创建textLayout
            m_pWriteFactory->CreateTextLayout(
                    frame->font->GetText().c_str(),
                    frame->font->GetText().length(),
                    textFormat,
                    frame->width,
                    frame->height,
                    &textLayout
            );

            frame->font->textFormat = textFormat;
            frame->font->textLayout = textLayout;
        }

        // 循序执行渲染器
        for (MString& item : m_itemManifest) {
            auto findIter = m_renderItems->find(item);
            if (findIter != m_renderItems->end()) {
                findIter->second->Render(frame, m_sharedRenderParam);
            }
        }
    }

    /// <summary>
    /// 打开画布准备渲染
    /// </summary>
    void STDMETHODCALLTYPE MRenderItemQueue::DrawBegin() {
        m_pRT->BeginDraw();
    }

    /// <summary>
    /// 关闭画布开始渲染
    /// </summary>
    void STDMETHODCALLTYPE MRenderItemQueue::DrawEnd() {
        m_pRT->EndDraw();
    }
}

//////////////////////////////////////////////////
// 导出函数
GM_DEFINE_REGISTER_FUNC_AND_DESTROY(MWD::Render::MRenderItemQueue, MWD::Render::SIID_IM_RENDER_QUEUE_SERVICE);
