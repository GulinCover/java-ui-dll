#pragma once

#include "IMCommon.h"

namespace MWD::Module {
    //////////////////////////////////////////////////
    // iid
    typedef struct _miid {
        unsigned char Data1[8];
        unsigned char Data2[16];
        /// <summary>
        /// 判断两个MIID是否相同
        /// </summary>
        MBoolean operator==(const _miid& right) const {
            for (int i = 0; i < sizeof(Data2); ++i) {
                if (Data2[i] != right.Data2[i]) {
                    return false;
                }
            }

            for (int i = 0; i < sizeof(Data1); ++i) {
                if (Data1[i] != right.Data1[i]) {
                    return false;
                }
            }
            return true;
        }

        MBoolean operator<(const _miid& right) const {
            for (int i = 0; i < sizeof(Data2); ++i) {
                if (Data2[i] != right.Data2[i]) {
                    return false;
                }
            }

            for (int i = 0; i < sizeof(Data1); ++i) {
                if (Data1[i] != right.Data1[i]) {
                    return false;
                }
            }
            return true;
        }

        MBoolean operator>(const _miid& right) const {
            for (int i = 0; i < sizeof(Data2); ++i) {
                if (Data2[i] != right.Data2[i]) {
                    return false;
                }
            }

            for (int i = 0; i < sizeof(Data1); ++i) {
                if (Data1[i] != right.Data1[i]) {
                    return false;
                }
            }
            return true;
        }
    } MIID;
    //////////////////////////////////////////////////
    // 模块类接口
    interface IModule {
    public:
        using ptr = std::shared_ptr<IModule>;
    public:
        virtual ~IModule() = default;

    public:
        /// <summary>
        /// 测试是否可转
        /// </summary>
        virtual MBoolean STDMETHODCALLTYPE QueryInterface(
                /* [in] */ MIID miid
        ) PURE;

        /// <summary>
        /// 获取iid
        /// </summary>
        virtual MIID STDMETHODCALLTYPE GetMIID() PURE;
    };

    //////////////////////////////////////////////////
    // 模块管理器类接口
    interface IModuleManager {
    public:
        using ptr = std::shared_ptr<IModuleManager>;
    public:
        virtual ~IModuleManager() = default;

    public:
        /// <summary>
        /// 加载模块
        /// </summary>
        virtual MBoolean STDMETHODCALLTYPE LoadModule(
                /* [in] */ MIID miid,
                /* [in] */ IModule** pModule
        ) PURE;

        /// <summary>
        /// 卸载模块
        /// </summary>
        virtual MBoolean STDMETHODCALLTYPE DestroyModule(
                /* [in] */ MIID miid
        ) PURE;
    };
} // MWD::Module

typedef MBoolean (* GMLoadModuleFunc)(
        /*[in]*/MWD::Module::MIID miid,
        /*[in]*/const MWD::Module::IModuleManager::ptr& sysModule
);

typedef MBoolean (* GMUnloadModuleFunc)(
        /*[in]*/MWD::Module::MIID miid
);

//////////////////////////////////////////////////
// 导出函数
extern "C" __declspec(dllexport) MBoolean GMLoadModule(
        /*[in]*/MWD::Module::MIID miid,
        /*[in]*/const MWD::Module::IModuleManager::ptr& sysModule
);
extern "C" __declspec(dllexport) MBoolean GMUnloadModule(
        /*[in]*/MWD::Module::MIID miid,
        /* [in] */const MWD::Module::IModuleManager::ptr& sysModule
);

#define GM_DEFINE_LOAD_FUNC_AND_DESTROY(CLASS, IID) \
static CLASS* g_renderItem = nullptr; \
MBoolean GMLoadModule(\
        /*[in]*/MWD::Module::MIID miid,\
        /*[in]*/const MWD::Module::IModuleManager::ptr& sysModule\
) {\
    if (miid == IID) {\
        auto* pModule = new CLASS;\
        MBoolean hr = sysModule->LoadModule(miid, (MWD::Module::IModule**) &pModule);\
        if (hr) {\
            g_renderItem = pModule;\
            return true;\
        } else {\
            delete pModule;\
        }\
    }\
    return false;\
}\
MBoolean GMUnloadModule(\
        /*[in]*/MWD::Module::MIID miid,\
        /*[in]*/const MWD::Module::IModuleManager::ptr& sysModule\
) {\
    if (miid == IID) {\
        MBoolean hr = sysModule->DestroyModule(miid);\
        if (hr) {\
            if (g_renderItem)\
                delete g_renderItem;\
            return true;\
        }\
    }\
    return false;\
}\
