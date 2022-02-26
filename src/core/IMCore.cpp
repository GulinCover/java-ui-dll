#include "IMCore.h"

namespace MWD::Core {
    //////////////////////////////////////////////////
    // 核心类实现
    /********************************************************
     内部方法
    *********************************************************/
    /// <summary>
    /// 循环加载dll
    /// </summary>
    void MCore::_loopLoadDll(const MString& name) {
        MIID miid{};

        // 添加dll
        MString fullPath = m_rootPath + "\\lib\\" + name;
        HMODULE h = LoadLibraryA(fullPath.c_str());
        if (h) {
            auto func = (GMLoadModuleFunc)GetProcAddress(h, "GMLoadModule");

            MBoolean hr = func(miid, std::shared_ptr<IModuleManager>(this));

            if (hr) {
                auto findIter = m_moduleManager->find(miid);
                if (findIter != m_moduleManager->end()) {
                    findIter->second->hmodule = h;
                    findIter->second->dllPath = name;
                }
            }
        }
    }

    /// <summary>
    /// 循环加载service_dll
    /// </summary>
    void MCore::_loopLoadService(const MString& name) {
        MUChar cs[sizeof(SIID::Data2)];
        memset(cs, '\0', sizeof(SIID::Data2));
        memcpy(cs, name.c_str(), sizeof(SIID::Data2));
        IMRegeditHandle handle = m_regeditService->CheckOutData(cs, sizeof(SIID::Data2));

        // 添加dll
        MString fullPath = m_rootPath + "\\lib\\" + name;
        HMODULE h = LoadLibraryA(fullPath.c_str());
        if (h) {
            auto func = (GMRegisterServiceFunc)GetProcAddress(h, "GMRegisterService");

            SIID siid{};
            memset(&siid, '\0', sizeof(SIID));
            memcpy(&siid, handle.Data2, sizeof(SIID));
            MBoolean hr = func(siid, std::shared_ptr<IMServiceManager>(this));

            if (hr) {
                auto findIter = m_serviceManager->find(siid);
                if (findIter != m_serviceManager->end()) {
                    findIter->second->hmodule = h;
                    findIter->second->dllPath = name;
                }
            }
        }
    }

    /// <summary>
    /// 加载指定service
    /// </summary>
    void MCore::_loadSpecialService(const MString& path) {
        HMODULE h = LoadLibraryA(path.c_str());
        if (h) {
            auto func = (GMRegisterServiceFunc)GetProcAddress(h, "GMRegisterService");

            MBoolean hr = func(regedit_siid, std::shared_ptr<IMServiceManager>(this));

            if (hr) {
                auto findIter = m_serviceManager->find(regedit_siid);
                if (findIter != m_serviceManager->end()) {
                    findIter->second->hmodule = h;
                    findIter->second->dllPath = "im_service_regedit.dll";
                }
            }
        }
    }

    /********************************************************
     IMServiceManager方法实现
    *********************************************************/
    /// <summary>
    /// 加载服务实例到服务管理器
    /// </summary>
    MBoolean STDMETHODCALLTYPE MCore::LoadService(
            /* [in] */SIID siid,
            /* [in] */IMService** ppService
    ) {
        auto findIter = m_serviceManager->find(siid);
        if (findIter != m_serviceManager->end()) {
            return false;
        }

        IMHService::ptr imhService = MakeShared(IMHService);
        imhService->siid = siid;
        imhService->ppService = ppService;
        m_serviceManager->insert(MakePair(siid, imhService));

        return true;
    }

    /// <summary>
    /// 删除服务实例从服务管理器
    /// </summary>
    MBoolean STDMETHODCALLTYPE MCore::DestroyService(
            /* [in] */SIID siid
    ) {
        auto findIter = m_serviceManager->find(siid);
        if (findIter != m_serviceManager->end()) {
            m_serviceManager->erase(findIter);
            return true;
        }

        return false;
    }

    /********************************************************
     IModuleManager方法实现
    *********************************************************/
    /// <summary>
    /// 加载模块
    /// </summary>
    MBoolean STDMETHODCALLTYPE MCore::LoadModule(
            /* [in] */ MIID miid,
            /* [in] */ IModule** pModule
    ) {
        auto findIter = m_moduleManager->find(miid);
        if (findIter != m_moduleManager->end()) {
            return false;
        }

        IMHModule::ptr imhModule = MakeShared(IMHModule);
        imhModule->miid = miid;
        imhModule->iModule = pModule;
        m_moduleManager->insert(MakePair(miid, imhModule));

        return true;
    }

    /// <summary>
    /// 卸载模块
    /// </summary>
    MBoolean STDMETHODCALLTYPE MCore::DestroyModule(
            /* [in] */ MIID miid
    ) {
        auto findIter = m_moduleManager->find(miid);
        if (findIter != m_moduleManager->end()) {
            m_moduleManager->erase(findIter);
            return true;
        }

        return false;
    }

    /********************************************************
     继承IMCore方法实现
    *********************************************************/
    /// <summary>
    /// 初始化win32相关配置
    /// </summary>
    HRESULT STDMETHODCALLTYPE MCore::InitWin32Config(
            /* [in][win32dll实例句柄] */HMODULE hmodule
            ) {
        m_hmodule = hmodule;

        // 初始化dll管理器
        m_moduleManager = MakeMap(MIID, IMHModule::ptr);
        m_serviceManager = MakeMap(SIID, IMHService::ptr);

        return  S_OK;
    }
    /// <summary>
    /// 加载配置文件
    /// </summary>
    HRESULT STDMETHODCALLTYPE MCore::LoadRootPath(
            /* [in][default=运行时根路径] */ const MString& root_path
    ) {
        if (root_path.empty()) {
            m_rootPath = RequireCurrentExecRootPath(m_hmodule);
        } else {
            if (_access_s(root_path.c_str(), _A_RDONLY) == -1) {
                m_rootPath = RequireCurrentExecRootPath(m_hmodule);
            } else {
                m_rootPath = root_path;
            }
        }

        return S_OK;
    }

    /// <summary>
    /// 加载配置文件
    /// </summary>
    HRESULT STDMETHODCALLTYPE MCore::LoadConfig() {
        ACCESS(!m_rootPath.empty());

        MString configPath = m_rootPath.append("\\mwdui");

        m_configKeyValue = AnalysisPropertiesFile(configPath);
        if (m_configKeyValue)
            return S_OK;

        m_configKeyValue = AnalysisYmlFile(configPath);
        if (m_configKeyValue)
            return S_OK;

        m_configKeyValue = AnalysisXmlFile(configPath);
        if (m_configKeyValue)
            return S_OK;

        return S_FALSE;
    }

    /// <summary>
    /// 加载配置文件
    /// </summary>
    HRESULT STDMETHODCALLTYPE MCore::DynamicLoadDll(
            /* [in][default=root/lib] */ const MString& dll_path
    ) {
        MString dllPath;
        if (dll_path.empty()) {
            dllPath = m_rootPath.append("\\lib");
        } else {
            dllPath = dll_path;
        }

        // 动态加载lib下所有符合名称格式的dll
        if (IsDirExist(dllPath)) {
            _loadSpecialService(dllPath + "\\im_service_regedit.dll");
            m_regeditService = (IMRegedit*)*(m_serviceManager->find(regedit_siid)->second->ppService);
            m_regeditService->Init(m_rootPath);
            if (!m_regeditService->AnalysisReg()) {
                throw std::invalid_argument("The dynamic library file is missing");
            }

            auto files = RequireDirFiles(dllPath);

            if (!files->empty()) {
                for (MString name : *files) {
                    if (strcmp(name.substr(0, 10).c_str(), "im_module_") == 0) {
                        _loopLoadDll(name);
                    }

                    if (strcmp(name.substr(0, 10).c_str(), "im_service_") == 0) {
                        _loopLoadService(name);
                    }
                }
            }
        }

        // 加载配置文件里指定的dll

        return S_OK;
    }

    /// <summary>
    /// 销毁所有资源
    /// </summary>
    HRESULT STDMETHODCALLTYPE MCore::Destroy() {
        return S_OK;
    }
}

