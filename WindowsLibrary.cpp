#include "WindowsLibrary.h"

static std::shared_ptr<MWD::Loader::IMLoaderEvent> g_event;
static std::shared_ptr<MWD::Windows::IMWindowLibrary> g_window;

/********************************************************
 全局变量
*********************************************************/

static MWChar* g_cs = new MWChar[2];
static MWD::MainWindow::IMMainWindowEvent g_mainWindowEvent{};
HWND g_hwnd;
static int FPS_S = 0;
static int FPS_I = 0;
static double FPS = 0;
auto FPS_PREV = std::chrono::high_resolution_clock::now();


/********************************************************
 函数实现
*********************************************************/
std::shared_ptr<MWD::Loader::IMLoaderEvent> IMGetLoaderEvent() {
    if (!g_event)
        g_event = std::make_shared<MWD::Loader::IMLoaderEvent>();

    if (!g_window)
        g_window = std::make_shared<MWD::Windows::IMWindowLibrary>();
    g_event->SetWindow(g_window);
    return g_event;
}

void IMSetLoaderEvent(const std::shared_ptr<MWD::Loader::IMLoaderEvent>& event) {
    g_window->SetRender(event->GetRender());
    g_window->SetSwapChain(event->GetSwapChain());
}

ATOM MWD::Windows::IMWindowLibrary::_registerWindowClass(HINSTANCE hInstance) {
    WNDCLASSA wcex;

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = m_windowClass;

    return RegisterClassA(&wcex);
}

MBoolean MWD::Windows::IMWindowLibrary::_initInstance(HINSTANCE hInstance) {
    //CW_USEDEFAULT & ~WS_CAPTION
    HWND hWnd = CreateWindowA(m_windowClass, m_title, CW_USEDEFAULT & ~WS_CAPTION,
                              400, 400, 720, 480, nullptr, nullptr, hInstance, nullptr);
    g_hwnd = hWnd;
    m_hwnd = hWnd;
    if (!hWnd) {
        IM_LOG_FATAL("HWND create a failure", "IMWindowLibrary::_initInstance");
        return false;
    }

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    return true;
}

void MWD::Windows::IMWindowLibrary::_calcFps(const std::shared_ptr<IMFrame>& frame) {
    auto cur = std::chrono::high_resolution_clock::now();
    auto diff = std::chrono::duration<double, std::milli>(cur - FPS_PREV);
    if (diff.count() >= 200) {
        FPS_PREV = cur;
        FPS = (FPS_I - FPS_S) / .2;
        FPS_S = FPS_I;
        //std::stringstream ss;
        std::wstringstream g_ss;
        g_ss << "fps: ";
        g_ss << std::setiosflags(std::ios::fixed) << std::setprecision(2) << FPS;
        frame->font->text->clear();
        for (auto& s : g_ss.str()) {
            frame->font->text->emplace_back(s);
        }

        CComPtr<IDWriteTextLayout> textLayout{};
        frame->writeFactory->CreateTextLayout(
                frame->font->GetText().c_str(),
                frame->font->GetText().length(),
                frame->textFormat,
                frame->width,
                frame->height,
                &textLayout
        );
        frame->textLayout = textLayout;
        if (frame->font->isSingleRow)
            frame->textLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

        DWRITE_TEXT_METRICS textMetrics{};
        frame->textLayout->GetMetrics(&textMetrics);

        frame->font->textWidth = textMetrics.widthIncludingTrailingWhitespace;
        frame->font->textHeight = textMetrics.height;
    }
    ++FPS_I;
}

void MWD::Windows::IMWindowLibrary::_render() {
    m_render->DrawBegin();
    m_render->CleanScreen(D2D1::ColorF(1.0f, 1.0f, 1.0f));

    auto iter = m_swap->GetBeginIter();
    while (iter != m_swap->GetEndIter()) {
        std::shared_ptr<IMFrame> frame = m_swap->GetFrame(iter);
        if (frame != nullptr) {
            m_render->Render(frame);
        }

        ++iter;
    }

    m_render->DrawEnd();
}

void MWD::Windows::IMWindowLibrary::_initEvent() {
    m_monitor = std::make_shared<IMonitor>();
    m_monitor->Init(m_hwnd);

    m_render->SetMonitor(m_monitor);
}

void MWD::Windows::IMWindowLibrary::Init() {
    _registerWindowClass(GetCurrentInstance());

    _initInstance(GetCurrentInstance());

    _initEvent();
}

void MWD::Windows::IMWindowLibrary::ClearScreen() {
    D2D1::ColorF f(D2D1::ColorF::White);
    std::shared_ptr<Vector<MString>> colors = IM_CONFIG_GET_ARGS("mwd.backgroundColor");
    if (!colors->empty()) {
        for (MInt i = 0; i < 4 - colors->size(); ++i) {
            colors->push_back(std::to_string(1));
        }

        std::istringstream iss_r(colors->at(0));
        std::istringstream iss_g(colors->at(1));
        std::istringstream iss_b(colors->at(2));
        std::istringstream iss_a(colors->at(3));
        MFloat r;
        MFloat g;
        MFloat b;
        MFloat a;
        iss_r >> r;
        iss_g >> g;
        iss_b >> b;
        iss_a >> a;
        f = D2D1::ColorF(r, g, b, a);
    }
    m_render->CleanScreen(f);
}

void MWD::Windows::IMWindowLibrary::Resize(D2D1_SIZE_U rect) {
    m_render->Resize(rect);
}

void MWD::Windows::IMWindowLibrary::ExecListen(HWND hwnd, IM_MONITOR_EVENT e, void* args) {
    m_monitor->LoopExec(hwnd, e, args);
}

static MWD::Windows::IMWindow* g_SELF = nullptr;

MInt MWD::Windows::IMWindowLibrary::Startup() {
    MSG g_message;
    ZeroMemory(&g_message, sizeof(g_message));

    m_render->SetHwnd(m_hwnd);

    if (!m_render->Init("route1")) return -1;
    g_SELF = this;
    g_mainWindowEvent.SetRoute("route1");

    SetWindowLongA(m_hwnd, GWLP_USERDATA, (LONG) this);

    std::shared_ptr<IMFrame> frame = std::make_shared<IMFrame>();
    std::shared_ptr<IMFrame> fpsFrame = std::make_shared<IMFrame>();
    std::shared_ptr<IMBackground> background = std::make_shared<IMBackground>();
    std::shared_ptr<IMFont> font = std::make_shared<IMFont>();
    std::shared_ptr<IMBorder> border = std::make_shared<IMBorder>();
    std::shared_ptr<IMColor> color = std::make_shared<IMColor>(255, 0, 0, 255);

    font->fontSize = 16.0f;
    font->text = std::make_shared<Vector<MWChar>>();
    font->text->emplace_back(L'测');
    font->text->emplace_back(L'试');
    font->text->emplace_back(L'专');
    font->text->emplace_back(L'用');
    font->text->emplace_back(L'文');
    font->text->emplace_back(L'本');

    font->fontColor = color;
    font->textSelectedRect = std::make_shared<Vector<D2D1_RECT_F>>();

    border->borderLeftWidth = 0;
    border->borderRightWidth = 0;
    border->borderTopWidth = 0;
    border->borderBottomWidth = 0;

    border->borderRadius = 0;
    border->borderTopLeftRadius = 0;
    border->borderTopRightRadius = 0;
    border->borderBottomRightRadius = 0;
    border->borderBottomLeftRadius = 0;
    border->borderTopColor = std::make_shared<IMColor>(255, 0, 0, 255);
    border->borderRightColor = std::make_shared<IMColor>(0, 255, 0, 255);
    border->borderBottomColor = std::make_shared<IMColor>(0, 0, 255, 255);
    border->borderLeftColor = std::make_shared<IMColor>(255, 255, 0, 255);
    border->borderStyle = Frame::SOLID;

    //background->backgroundImageUrl = "C:\\Users\\12086\\Pictures\\Microsoft.Windows.Photos_8wekyb3d8bbwe!App\\q_avatar.jpg";

    background->backgroundColor = std::make_shared<IMColor>(166, 166, 166, 255);

    frame->uuid = "qwer-qwer-qwer-asdf-zxcv";
    frame->width = 200;
    frame->height = 24;
    frame->innerX = 20;
    frame->innerY = 20;
    frame->offX = 0;
    frame->offY = 0;
    frame->paddingLeft = 5;
    frame->paddingRight = 5;
    frame->paddingTop = 2;
    frame->paddingBottom = 2;

    frame->isInput = true;
    frame->isFocus = true;

    frame->font = font;
    frame->border = border;
    frame->background = background;
    frame->overflow = IM_FRAME_OVERFLOW::NONE;

    std::shared_ptr<IMBackground> background2 = std::make_shared<IMBackground>();
    std::shared_ptr<IMBorder> border2 = std::make_shared<IMBorder>();
    std::shared_ptr<IMColor> color2 = std::make_shared<IMColor>(255, 0, 0, 255);
    std::shared_ptr<IMFont> font2 = std::make_shared<IMFont>();

    background2->backgroundColor = std::make_shared<IMColor>(160, 160, 160, 255);

    border2->borderLeftWidth = 0;
    border2->borderRightWidth = 0;
    border2->borderTopWidth = 0;
    border2->borderBottomWidth = 0;

    border2->borderRadius = 0;
    border2->borderTopLeftRadius = 0;
    border2->borderTopRightRadius = 0;
    border2->borderBottomRightRadius = 0;
    border2->borderBottomLeftRadius = 0;
    border2->borderTopColor = std::make_shared<IMColor>(255, 0, 0, 255);
    border2->borderRightColor = std::make_shared<IMColor>(0, 255, 0, 255);
    border2->borderBottomColor = std::make_shared<IMColor>(0, 0, 255, 255);
    border2->borderLeftColor = std::make_shared<IMColor>(255, 255, 0, 255);
    border2->borderStyle = Frame::SOLID;

    font2->fontSize = 16.0f;
    font2->text = std::make_shared<Vector<MWChar>>();

    font2->fontColor = color2;
    font2->textSelectedRect = std::make_shared<Vector<D2D1_RECT_F>>();

    fpsFrame->uuid = "abcd-abcd-abcd-abcd-abcd";
    fpsFrame->width = 200;
    fpsFrame->height = 24;
    fpsFrame->innerX = 200;
    fpsFrame->innerY = 200;
    fpsFrame->offX = 0;
    fpsFrame->offY = 0;
    fpsFrame->paddingLeft = 5;
    fpsFrame->paddingRight = 5;
    fpsFrame->paddingTop = 2;
    fpsFrame->paddingBottom = 2;

    fpsFrame->isInput = false;
    fpsFrame->isFocus = true;

    fpsFrame->font = font2;
    fpsFrame->border = border2;
    fpsFrame->background = background2;
    fpsFrame->overflow = IM_FRAME_OVERFLOW::NONE;


    std::shared_ptr<std::function<void(IMonitorEvent*, void*)>> f = std::make_shared<std::function<void(IMonitorEvent*,
                                                                                                        void*)>>(
            [](IMonitorEvent* initArgs, void* args) {
                if (args) {
                    auto point = (LPPOINT) args;

                }
            });

    //IMonitorEvent* pp = new IMonitorEvent;
    //m_monitor->AddTask(frame, Monitor::MOUSE_ENTER, SYNC_QUEUE, f, pp);
    //m_monitor->AddTask(frame, Monitor::MOUSE_LEAVE, SYNC_QUEUE, f, pp);

    m_swap->AddFrame(frame);
    m_swap->AddFrame(fpsFrame);

    //auto focusManager = IMFocusManager::GetInstance();
    //focusManager->SetFocusFrame("route1", frame);

    MBoolean enableFPS = false;
    MString ret = IM_CONFIG_GET_ARG("mwd.config.pfs.enable");
    std::transform<MString::iterator, MString::iterator, int(int)>(ret.begin(), ret.end(), ret.begin(), std::toupper);
    if (ret == "TRUE") {
        enableFPS = true;
    }

    // 主消息循环:
    while (g_message.message != WM_QUIT) {
        if (PeekMessageA(&g_message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&g_message);
            DispatchMessageA(&g_message);
            continue;
        }

        _render();

        if (enableFPS) _calcFps(fpsFrame);
    }

    //while (GetMessageA(&g_message, nullptr, 0, 0)) {
    //    TranslateMessage(&g_message);
    //    DispatchMessageA(&g_message);
    //}

    return (int) g_message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    //static MWD::Windows::IMWindow* g_SELF = nullptr;
    //
    //if (!g_SELF) {
    //    g_SELF = (MWD::Windows::IMWindow*) GetWindowLongA(hWnd, GWLP_USERDATA);
    //}
    switch (message) {
        case WM_CREATE:
            break;
        case WM_COMMAND:
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            if (g_cs)
                delete[] g_cs;
            break;
        case WM_SIZE:
            if (g_SELF) {
                g_SELF->Resize({LOWORD(lParam), HIWORD(lParam)});
            }
            break;
        case WM_ERASEBKGND:
            return true;
        case WM_IME_NOTIFY:
        case WM_IME_SETCONTEXT:
            break;
        case WM_MOUSEMOVE:
            if (g_SELF) {
                auto p = new POINT;
                p->x = LOWORD(lParam);
                p->y = HIWORD(lParam);
                g_SELF->ExecListen(hWnd, MWD::Monitor::IM_MONITOR_EVENT::MOUSE_ENTER, p);
                g_SELF->ExecListen(hWnd, MWD::Monitor::IM_MONITOR_EVENT::MOUSE_MOVE, p);
                g_SELF->ExecListen(hWnd, MWD::Monitor::IM_MONITOR_EVENT::MOUSE_LEAVE, p);
                g_mainWindowEvent.ExecMainMoveEvent(*p);
                if (p)
                    delete p;
            }
            break;
        case WM_LBUTTONDOWN: {
            if (g_SELF) {
                auto p = new POINT;
                p->x = LOWORD(lParam);
                p->y = HIWORD(lParam);
                g_SELF->ExecListen(hWnd, MWD::Monitor::IM_MONITOR_EVENT::MOUSE_LEFT_BUTTON_DOWN, p);
                g_mainWindowEvent.ExecMainClickDownEvent(*p);
                if (p)
                    delete p;
            }
        }
            break;
        case WM_LBUTTONUP:
            if (g_SELF) {
                auto p = new POINT;
                p->x = LOWORD(lParam);
                p->y = HIWORD(lParam);
                g_SELF->ExecListen(hWnd, MWD::Monitor::IM_MONITOR_EVENT::MOUSE_LEFT_BUTTON_UP, p);
                g_mainWindowEvent.ExecMainClickUpEvent(*p);
                if (p)
                    delete p;
            }
            break;
        case WM_LBUTTONDBLCLK:
            if (g_SELF) {
                g_SELF->ExecListen(hWnd, MWD::Monitor::IM_MONITOR_EVENT::MOUSE_LEFT_BUTTON_DOUBLE, nullptr);
            }
            break;
        case WM_RBUTTONDOWN:
            if (g_SELF) {
                g_SELF->ExecListen(hWnd, MWD::Monitor::IM_MONITOR_EVENT::MOUSE_RIGHT_BUTTON_DOWN, nullptr);
            }
            break;
        case WM_RBUTTONUP:
            if (g_SELF) {
                g_SELF->ExecListen(hWnd, MWD::Monitor::IM_MONITOR_EVENT::MOUSE_RIGHT_BUTTON_UP, nullptr);
            }
            break;
        case WM_RBUTTONDBLCLK:
            if (g_SELF) {
                g_SELF->ExecListen(hWnd, MWD::Monitor::IM_MONITOR_EVENT::MOUSE_RIGHT_BUTTON_DOUBLE, nullptr);
            }
            break;
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            if (g_SELF) {
                MChar c = wParam;
                g_SELF->ExecListen(hWnd, MWD::Monitor::IM_MONITOR_EVENT::KEYBOARD_KEY_DOWN, &c);
                g_mainWindowEvent.ExecMainKeyDownEvent(c);
            }
            break;
        case WM_SYSKEYUP:
        case WM_KEYUP:
            if (g_SELF) {
                MChar c = wParam;
                g_SELF->ExecListen(hWnd, MWD::Monitor::IM_MONITOR_EVENT::KEYBOARD_KEY_UP, &c);
            }
            std::cout << "--" << wParam << std::endl;
            break;
        case WM_CHAR:
            if (g_SELF) {
                g_cs[0] = (MChar) wParam;
                g_cs[1] = '\0';
                g_SELF->ExecListen(hWnd, MWD::Monitor::IM_MONITOR_EVENT::KEYBOARD_ALPHA_KEY, g_cs);
                g_mainWindowEvent.ExecMainCharDownEvent((MChar) wParam);
            }
            std::cout << (MChar) wParam << std::endl;
            break;
        case WM_IME_CHAR:
            if (g_SELF) {
                MUInt c = wParam;
                g_SELF->ExecListen(hWnd, MWD::Monitor::IM_MONITOR_EVENT::KEYBOARD_IME_KEY, &c);
                g_mainWindowEvent.ExecMainIMEEvent(&c);
            }
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
