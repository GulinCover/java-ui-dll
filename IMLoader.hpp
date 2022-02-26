#pragma once

#include <utility>

#include "IMCommon.hpp"
#include "IMLogger.hpp"
#include "IMNative.hpp"

namespace MWD::Loader {

    class IMLoaderEvent {
    public:
        IMLoaderEvent() = default;

    public:
        void SetWindow(const std::shared_ptr<MWD::Native::IMWindow>& window) {
            m_window = window;
        }

        void SetWindow(std::shared_ptr<MWD::Native::IMWindow>&& window) {
            m_window = window;
        }

        std::shared_ptr<MWD::Native::IMWindow> GetWindow() {
            return m_window;
        }

        void SetRender(std::shared_ptr<MWD::Render::IMRender> render) {
            m_render = std::move(render);
        }

        void SetSwapChain(std::shared_ptr<MWD::SwapChain::IMSwapQueue> swap) {
            m_swap = std::move(swap);
        }

        std::shared_ptr<MWD::Render::IMRender> GetRender() {
            return m_render;
        }

        std::shared_ptr<MWD::SwapChain::IMSwapQueue> GetSwapChain() {
            return m_swap;
        }

    private:
        std::shared_ptr<MWD::Native::IMWindow> m_window = nullptr;
        std::shared_ptr<MWD::SwapChain::IMSwapQueue> m_swap = nullptr;
        std::shared_ptr<MWD::Render::IMRender> m_render = nullptr;
    };

    typedef std::shared_ptr<IMLoaderEvent>(* IMGetLoaderEvent)();

    typedef void(* IMSetLoaderEvent)(std::shared_ptr<IMLoaderEvent>);


    /********************************************************
     dll加载器类
    *********************************************************/
    class IMLoader {
    public:
        explicit IMLoader() = default;

        explicit IMLoader(MString root) : m_rootPath(std::move(root)) {}

        virtual ~IMLoader() = default;
    public:
        virtual MBoolean FreeDllLibrary(HMODULE) = 0;

        virtual std::shared_ptr<IMLoaderEvent> LoadDllLibrary(MString file, HMODULE* hmodule) = 0;

        virtual void SetLibraryArgs(MString file, std::shared_ptr<IMLoaderEvent> args, HMODULE* hmodule) = 0;

        virtual MString GetRootPath() {
            return m_rootPath;
        }

        virtual void SetRootPath(MString root) {
            m_rootPath = std::move(root);
        }

    private:
        MString m_rootPath;
    };

    /********************************************************
     win平台dll加载器类
    *********************************************************/
    class WindowsLoader : public IMLoader {
    public:
        MBoolean FreeDllLibrary(HMODULE h) override {
            return h ? FreeLibrary(h) : false;
        }

        std::shared_ptr<IMLoaderEvent> LoadDllLibrary(MString file, HMODULE* hmodule) override {
            HMODULE h = LoadLibraryA(file.c_str());
            if (h) {
                auto func = (IMGetLoaderEvent) GetProcAddress(h, "IMGetLoaderEvent");
                IM_LOG_INFO("Success to load the dynamic library function 'IMGetLoaderEvent'",
                            "WindowsLoader::LoadLibrary");

                *hmodule = h;

                return func();
            }

            IM_LOG_ERROR("Failed to load the dynamic library function 'IMGetLoaderEvent'",
                         "WindowsLoader::LoadLibrary");

            return nullptr;
        }

        void SetLibraryArgs(MString file, std::shared_ptr<IMLoaderEvent> args, HMODULE* hmodule) override {
            HMODULE h = LoadLibraryA(file.c_str());
            if (h) {
                auto func = (IMSetLoaderEvent) GetProcAddress(h, "IMSetLoaderEvent");
                func(args);

                *hmodule = h;

                IM_LOG_INFO("Success to load the dynamic library function 'IMSetLoaderEvent'",
                            "WindowsLoader::SetLibraryArgs");
            }

            IM_LOG_ERROR("Failed to load the dynamic library function 'IMSetLoaderEvent'",
                         "WindowsLoader::SetLibraryArgs");
        }
    };

    /********************************************************
     加载器资源管理
    *********************************************************/
    class IMLoaderResourcesManager {
    public:
        static std::shared_ptr<IMLoader> GetDllLoader() {

#ifdef WINDOWS_OS
            return MWD::GM::IMSingletonPtr<WindowsLoader>::GetInstance();

#elif LINUX_OS
            return MWD::GM::IMSingletonPtr<LinuxLoader>::GetInstance();
#elif MACOS
            return MWD::GM::IMSingletonPtr<MacOSLoader>::GetInstance();
#endif
        }

        static void SetLoaderRootPath(MString root) {
            GetDllLoader()->SetRootPath(std::move(root));
        }
    };

#define IM_LOADER_GET_LOADER \
    MWD::Loader::IMLoaderResourcesManager::GetDllLoader();

#define IM_LOADER_SET_ROOT(FILE) \
    MWD::Loader::IMLoaderResourcesManager::SetLoaderRootPath(FILE);

}//MWD::Loader
