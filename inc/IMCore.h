#pragma once

#include "IMCommon.h"
#include "IMFile.h"
#include "IModule.h"
#include "IMService.h"
#include "IMRegeditService.h"

namespace MWD::Core {
    using namespace MWD::File;
    using namespace MWD::Module;
    using namespace MWD::Service;
    using namespace MWD::Service::Regedit;

    //////////////////////////////////////////////////
    // 核心类
    abstract IMCore {
    public:
        using ptr = std::shared_ptr<IMCore>;
    public:
        virtual ~IMCore() = default;

    public:
        /// <summary>
        /// 初始化win32相关配置
        /// </summary>
        virtual HRESULT STDMETHODCALLTYPE InitWin32Config(
                /* [in][win32dll实例句柄] */HMODULE hmodule
        ) PURE;

        /// <summary>
        /// 加载根路径
        /// </summary>
        virtual HRESULT STDMETHODCALLTYPE LoadRootPath(
                /* [in][default=运行时根路径] */ const MString& root_path
        ) PURE;

        /// <summary>
        /// 加载配置文件
        /// </summary>
        virtual HRESULT STDMETHODCALLTYPE LoadConfig() PURE;

        /// <summary>
        /// 加载配置文件
        /// </summary>
        virtual HRESULT STDMETHODCALLTYPE DynamicLoadDll(
                /* [in][default=root/lib] */ const MString& dll_path
        ) PURE;

        /// <summary>
        /// 销毁所有资源
        /// </summary>
        virtual HRESULT STDMETHODCALLTYPE Destroy() PURE;
    };

    //////////////////////////////////////////////////
    // 核心类实现
    class MCore : public IMCore,
                  public MDefaultFileImpl,
                  public IModuleManager,
                  public IMServiceManager
                  {
    public:
        using ptr = std::shared_ptr<IMCore>;
    private:
        abstract IMHModule {
        public:
            using ptr = std::shared_ptr<IMHModule>;
        public:
            MIID miid;
            HMODULE hmodule;
            IModule** iModule;
            MString dllPath;
        };

        abstract IMHService {
        public:
            using ptr = std::shared_ptr<IMHService>;
        public:
            SIID siid;
            HMODULE hmodule;
            IMService** ppService;
            MString dllPath;
        };

    private:
        /// <summary>
        /// 循环加载dll
        /// </summary>
        void _loopLoadDll(const MString& name);
        /// <summary>
        /// 循环加载service_dll
        /// </summary>
        void _loopLoadService(const MString& name);

        /// <summary>
        /// 加载指定service
        /// </summary>
        void _loadSpecialService(const MString& path);
    public:
        /********************************************************
         IMServiceManager方法实现
        *********************************************************/
        /// <summary>
        /// 加载服务实例到服务管理器
        /// </summary>
        MBoolean STDMETHODCALLTYPE LoadService(
                /* [in] */SIID siid,
                /* [in] */IMService** ppService
        ) override;

        /// <summary>
        /// 删除服务实例从服务管理器
        /// </summary>
        MBoolean STDMETHODCALLTYPE DestroyService(
                /* [in] */SIID siid
        ) override;

        /********************************************************
         IModuleManager方法实现
        *********************************************************/
        /// <summary>
        /// 加载模块
        /// </summary>
        MBoolean STDMETHODCALLTYPE LoadModule(
                /* [in] */ MIID miid,
                /* [in] */ IModule** pModule
        ) override;

        /// <summary>
        /// 卸载模块
        /// </summary>
        MBoolean STDMETHODCALLTYPE DestroyModule(
                /* [in] */ MIID miid
        ) override;

        /********************************************************
         IMCore方法实现
        *********************************************************/
        /// <summary>
        /// 初始化win32相关配置
        /// </summary>
        HRESULT STDMETHODCALLTYPE InitWin32Config(
                /* [in][win32dll实例句柄] */HMODULE hmodule
        ) override;

        /// <summary>
        /// 加载根路径
        /// </summary>
        HRESULT STDMETHODCALLTYPE LoadRootPath(
                /* [in][default=运行时根路径] */ const MString& root_path
        ) override;

        /// <summary>
        /// 加载配置文件
        /// </summary>
        HRESULT STDMETHODCALLTYPE LoadConfig() override;

        /// <summary>
        /// 加载配置文件
        /// </summary>
        HRESULT STDMETHODCALLTYPE DynamicLoadDll(
                /* [in][default=root/lib] */ const MString& dll_path
        ) override;

        /// <summary>
        /// 销毁所有资源
        /// </summary>
        HRESULT STDMETHODCALLTYPE Destroy() override;

    private:
        HMODULE m_hmodule;
        MString m_rootPath;
        IMFileStruct::ptr m_configKeyValue;
        MapPtr<MIID, IMHModule::ptr> m_moduleManager;
        MapPtr<SIID, IMHService::ptr> m_serviceManager;
        IMRegedit* m_regeditService;
        SIID regedit_siid{
                {0,0,0,0,0,0,0,1},
                {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}
        };
    };
}
