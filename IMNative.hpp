#pragma once

#include <utility>

#include "IMRender.hpp"
#include "IMSwapChain.hpp"
#include "IMFrame.hpp"
#include "IMLogger.hpp"
#include "IMConfig.hpp"
#include "IMonitor.hpp"

namespace MWD::Native {
    using namespace MWD::Frame;
    using namespace MWD::SwapChain;
    using namespace MWD::Render;
    using namespace MWD::Config;
    using namespace MWD::Log;
    using namespace MWD::Monitor;

    /********************************************************
     windows主窗口类
    *********************************************************/
    class IMWindow {
    public:
        virtual void SetSwapChain(const std::shared_ptr<IMSwapQueue>&) = 0;

        virtual void SetRender(const std::shared_ptr<IMRender>&) = 0;

        virtual void SetDefaultBackgroundColor() = 0;

        virtual void SetDefaultWindowTitle() = 0;

        virtual void Init() = 0;

        virtual void ClearScreen() = 0;

        virtual void Resize(D2D1_SIZE_U) = 0;

        virtual void ExecListen(HWND, IM_MONITOR_EVENT, void*) = 0;

        virtual MInt Startup() {
            IM_LOG_INFO("start render", "IMWindow::Startup");
            return 0;
        }

        virtual void SetLoggerPtr(std::shared_ptr<IMLogger> ptr) {
            MWD::GM::IMSingletonPtr<IMLogger>::SetInstance(std::move(ptr));
        }

        virtual void SetConfigPtr(std::shared_ptr<IMConfig> ptr) {
            MWD::GM::IMSingletonPtr<IMConfig>::SetInstance(std::move(ptr));
        }

        virtual void SetTimerPtr(std::shared_ptr<IMTimer> ptr) {
            MWD::GM::IMSingletonPtr<IMTimer>::SetInstance(std::move(ptr));
        }
    };
}//MWD::Native
