#pragma once

#include "IMSwapChain.hpp"
#include "IMRender.hpp"
#include "IMNative.hpp"
#include "IMCommon.hpp"
#include "IMLoader.hpp"
#include "IMFrame.hpp"
#include "IMonitor.hpp"
#include "IMMainWindowEvent.hpp"

extern "C" __declspec(dllexport) std::shared_ptr<MWD::Loader::IMLoaderEvent> IMGetLoaderEvent();

extern "C" __declspec(dllexport) void IMSetLoaderEvent(const std::shared_ptr<MWD::Loader::IMLoaderEvent>&);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

namespace MWD::Windows {
    using namespace MWD::Native;
    using namespace MWD::Monitor;
    using namespace MWD::Config;
    using namespace MWD::Monitor;
    using namespace MWD::MainWindow;

    class IMWindowLibrary : public IMWindow {
    private:
        ATOM _registerWindowClass(HINSTANCE hInstance);

        MBoolean _initInstance(HINSTANCE hInstance);

        void _initEvent();

    public:
        void _render();

        void _calcFps(const std::shared_ptr<IMFrame>&);

    public:
        void SetSwapChain(const std::shared_ptr<IMSwapQueue>& swap) override {
            m_swap = swap;
        }

        void SetRender(const std::shared_ptr<IMRender>& render) override {
            m_render = render;
        }

        void SetDefaultBackgroundColor() override {

        }

        void SetDefaultWindowTitle() override {

        }

        void ClearScreen() override;

        void Resize(D2D1_SIZE_U) override;

        void ExecListen(HWND, IM_MONITOR_EVENT, void*) override;

        void Init() override;

        MInt Startup() override;

    public:
        static HINSTANCE GetCurrentInstance() {
            static HINSTANCE g_hInstance;
            if (g_hInstance) {
                return g_hInstance;
            }
            g_hInstance = GetModuleHandleA(nullptr);
            return g_hInstance;
        }

    private:
        LPCSTR m_title = "MWD_UI";
        LPCSTR m_windowClass = "mwd_class";
        HWND m_hwnd = nullptr;
        std::shared_ptr<IMSwapQueue> m_swap;
        std::shared_ptr<IMRender> m_render;
        std::shared_ptr<IMonitor> m_monitor;
    };
}
