#pragma once

#include "IMCommon.h"
#include "IModule.h"

namespace MWD::Service {
    using namespace MWD::Module;

    //////////////////////////////////////////////////
    // iid
    typedef struct _siid {
        unsigned char Data1[8];
        unsigned char Data2[16];
        /// <summary>
        /// 判断两个SIID是否相同
        /// </summary>
        MBoolean operator==(const _siid& right) const {
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

        MBoolean operator<(const _siid& right) const {
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

        MBoolean operator>(const _siid& right) const {
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
    } SIID;

    //////////////////////////////////////////////////
    // 无效siid
#define SIID_SERVICE_INVALID    \
        SIID {  \
            0,                  \
            0                  \
        };

#define INVALID_SERVICE nullptr

    //////////////////////////////////////////////////
    // 服务实例接口
    interface IMService {
    public:
        /// <summary>
        /// 智能指针
        /// </summary>
        using ptr = std::shared_ptr<IMService>;
    public:
        virtual ~IMService() = default;

    public:
        /// <summary>
        /// 是否可转
        /// </summary>
        virtual MBoolean STDMETHODCALLTYPE QueryInterface(
                /* [in] */SIID siid
        ) PURE;

        /// <summary>
        /// 获取siid
        /// </summary>
        virtual SIID STDMETHODCALLTYPE GetSIID(
                /* [in] */char Data2[16]
        ) PURE;
    };

    //////////////////////////////////////////////////
    // 高级服务实例接口
    interface IMAdvancedService : public IMService {
    public:
        /// <summary>
        /// 智能指针
        /// </summary>
        using ptr = std::shared_ptr<IMAdvancedService>;
    public:
        /// <summary>
        /// 添加模块
        /// </summary>
        virtual MBoolean STDMETHODCALLTYPE AppendModule(
                /* [in] */MIID miid,
                /* [in] */IModule::ptr module
        ) PURE;

        /// <summary>
        /// 获取需要添加的module的miid
        /// </summary>
        virtual VectorPtr<MIID> STDMETHODCALLTYPE GetMIIDS(
        ) PURE;
    };

    //////////////////////////////////////////////////
    // 服务实例接口
    interface IMServiceManager {
    public:
        /// <summary>
        /// 智能指针
        /// </summary>
        using ptr = std::shared_ptr<IMServiceManager>;
    public:
        virtual ~IMServiceManager() = default;

    public:
        /// <summary>
        /// 加载服务实例到服务管理器
        /// </summary>
        virtual MBoolean STDMETHODCALLTYPE LoadService(
                /* [in] */SIID siid,
                /* [in] */IMService** ppService
        ) PURE;

        /// <summary>
        /// 删除服务实例从服务管理器
        /// </summary>
        virtual MBoolean STDMETHODCALLTYPE DestroyService(
                /* [in] */SIID siid
        ) PURE;
    };
} //MWD::Service

typedef MBoolean (* GMRegisterServiceFunc)(
        /* [in][服务siid] */MWD::Service::SIID siid,
        /* [in][系统服务] */const MWD::Service::IMServiceManager::ptr& sysService
);

typedef MBoolean (* GMUnregisterServiceFunc)(
        /* [in][自身服务] */MWD::Service::SIID siid
);

//////////////////////////////////////////////////
// 导出函数
extern "C" __declspec(dllexport) MBoolean GMRegisterService(
        /* [in][服务siid] */MWD::Service::SIID siid,
        /* [in][系统服务] */const MWD::Service::IMServiceManager::ptr& sysService
);
extern "C" __declspec(dllexport) MBoolean GMUnregisterService(
        /* [in][自身服务] */MWD::Service::SIID siid,
        /* [in][系统服务] */const MWD::Service::IMServiceManager::ptr& sysService
);

#define GM_DEFINE_REGISTER_FUNC_AND_DESTROY(CLASS, ISIID) \
static CLASS* g_pService;  \
MBoolean GMRegisterService(  \
        /* [in][服务siid] */MWD::Service::SIID siid, \
        /* [in][系统服务] */const MWD::Service::IMServiceManager::ptr& sysService \
) { \
    if (siid == ISIID) { \
        auto pService = new CLASS; \
        MBoolean hr = sysService->LoadService(siid, (MWD::Service::IMService**)&pService);\
        if (!hr) {\
            delete pService;\
            return false;\
        } else {\
            g_pService = pService;\
            return true;\
        }\
    }\
    return false;\
}\
MBoolean GMUnregisterService(\
        /* [in] */MWD::Service::SIID siid,\
        /* [in][系统服务] */const MWD::Service::IMServiceManager::ptr& sysService\
) {\
    if (siid == ISIID) {\
        MBoolean hr = sysService->DestroyService(ISIID);\
        if (hr) {\
            if (g_pService)\
                delete g_pService;\
            return true;\
        }\
    }\
    return false;\
}
