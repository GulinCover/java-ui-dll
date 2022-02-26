#pragma once

#include "IMService.h"

namespace MWD::Service::Regedit {

    struct IMRegeditHandle {
        /// <summary>
        /// id
        /// </summary>
        MUChar Data1[8];
        /// <summary>
        /// siid
        /// </summary>
        MUChar Data2[24];
        /// <summary>
        /// 描述
        /// </summary>
        MUChar Data3[256];
        /// <summary>
        /// 描述
        /// </summary>
        MUChar Data4[512];

        MBoolean operator==(const IMRegeditHandle& right) {
            auto* dcs = new MUChar[sizeof(IMRegeditHandle)];
            auto* dcs2 = new MUChar[sizeof(IMRegeditHandle)];
            memset(dcs, '\0', sizeof(IMRegeditHandle));
            memset(dcs2, '\0', sizeof(IMRegeditHandle));

            memcpy(dcs, &right, sizeof(IMRegeditHandle));
            memcpy(dcs2, this, sizeof(IMRegeditHandle));
            MInt ret = 0;
            for (MInt i = 0; i < sizeof(IMRegeditHandle); ++i) {
                if (dcs[i] == dcs2[i]) {
                    ++ret;
                } else {
                    break;
                }
            }

            delete[] dcs;
            delete[] dcs2;
            return ret == sizeof(IMRegeditHandle);
        }

        MBoolean operator>(const IMRegeditHandle& right) {
            auto* dcs = new MUChar[sizeof(IMRegeditHandle)];
            auto* dcs2 = new MUChar[sizeof(IMRegeditHandle)];
            memset(dcs, '\0', sizeof(IMRegeditHandle));
            memset(dcs2, '\0', sizeof(IMRegeditHandle));

            memcpy(dcs, &right, sizeof(IMRegeditHandle));
            memcpy(dcs2, this, sizeof(IMRegeditHandle));
            MInt ret = 0;
            for (MInt i = 0; i < sizeof(IMRegeditHandle); ++i) {
                if (dcs[i] == dcs2[i]) {
                    ++ret;
                } else {
                    break;
                }
            }

            delete[] dcs;
            delete[] dcs2;
            return ret == sizeof(IMRegeditHandle);
        }

        MBoolean operator<(const IMRegeditHandle& right) {
            auto* dcs = new MUChar[sizeof(IMRegeditHandle)];
            auto* dcs2 = new MUChar[sizeof(IMRegeditHandle)];
            memset(dcs, '\0', sizeof(IMRegeditHandle));
            memset(dcs2, '\0', sizeof(IMRegeditHandle));

            memcpy(dcs, &right, sizeof(IMRegeditHandle));
            memcpy(dcs2, this, sizeof(IMRegeditHandle));
            MInt ret = 0;
            for (MInt i = 0; i < sizeof(IMRegeditHandle); ++i) {
                if (dcs[i] == dcs2[i]) {
                    ++ret;
                } else {
                    break;
                }
            }

            delete[] dcs;
            delete[] dcs2;
            return ret == sizeof(IMRegeditHandle);
        }

        MBoolean operator!=(const IMRegeditHandle& right) {
            auto* dcs = new MUChar[sizeof(IMRegeditHandle)];
            auto* dcs2 = new MUChar[sizeof(IMRegeditHandle)];
            memset(dcs, '\0', sizeof(IMRegeditHandle));
            memset(dcs2, '\0', sizeof(IMRegeditHandle));

            memcpy(dcs, &right, sizeof(IMRegeditHandle));
            memcpy(dcs2, this, sizeof(IMRegeditHandle));
            MInt ret = 0;
            for (MInt i = 0; i < sizeof(IMRegeditHandle); ++i) {
                if (dcs[i] == dcs2[i]) {
                    ++ret;
                } else {
                    break;
                }
            }

            delete[] dcs;
            delete[] dcs2;
            return ret != sizeof(IMRegeditHandle);
        }
    };

    static IMRegeditHandle IM_INVALID_REGEDIT{
        '\0','\0','\0','\0'
    };

    //////////////////////////////////////////////////
    // 注册表配置接口
    interface IMRegedit : public IMService {
    public:
        using ptr = std::shared_ptr<IMRegedit>;
    public:
        /// <summary>
        /// 更改当前日志配置
        /// </summary>
        virtual void STDMETHODCALLTYPE UpdateConfig(
                /* [in][要更改的配置] */const MString& key,
                /* [in][要更改的配置] */const MString& value
        ) PURE;

        virtual void STDMETHODCALLTYPE Init(
                /* [in] */const MString& root
        ) PURE;

        /// <summary>
        /// 解析类容
        /// </summary>
        virtual MBoolean STDMETHODCALLTYPE AnalysisReg() PURE;

        /// <summary>
        /// 查询内容
        /// </summary>
        virtual IMRegeditHandle STDMETHODCALLTYPE SelectReg(
                /* [in] */MUChar* cs,
                /* [in] */MInt len,
                /* [in][not use] */MBoolean temp
        ) PURE;

        /// <summary>
        /// 更改内容
        /// </summary>
        virtual MBoolean STDMETHODCALLTYPE UpdateReg(
                /* [in] */MUChar* cs,
                /* [in] */MInt len,
                /* [in] */MBoolean temp
        ) PURE;

        /// <summary>
        /// 追加内容
        /// </summary>
        virtual MBoolean STDMETHODCALLTYPE AppendReg(
                /* [in] */MUChar* cs,
                /* [in] */MInt len,
                /* [in] */MBoolean temp
        ) PURE;

        /// <summary>
        /// 删除内容
        /// </summary>
        virtual MBoolean STDMETHODCALLTYPE DeleteReg(
                /* [in] */MUChar* cs,
                /* [in] */MInt len,
                /* [in] */MBoolean temp
        ) PURE;

        virtual IMRegeditHandle STDMETHODCALLTYPE CheckOutData(
                /* [in] */const MUChar* name,
                /* [in] */int len
        ) PURE;

        virtual IMRegeditHandle STDMETHODCALLTYPE CheckOutData(
                /* [in] */SIID siid
        ) PURE;
    };

    //////////////////////////////////////////////////
    // 注册表配置服务
    class MRegeditService : public IMRegedit {
    public:
        using ptr = std::shared_ptr<MRegeditService>;
    public:
        /********************************************************
         IMService方法实现
        *********************************************************/
        /// <summary>
        /// 是否可转
        /// </summary>
        MBoolean STDMETHODCALLTYPE QueryInterface(
                /* [in] */SIID siid
        ) override;

        /// <summary>
        /// 获取siid
        /// </summary>
        SIID STDMETHODCALLTYPE GetSIID(
                /* [in] */char Data2[16]
        ) override;

    public:
        /********************************************************
         IMRegedit方法实现
        *********************************************************/
        /// <summary>
        /// 更改当前日志配置
        /// </summary>
        void STDMETHODCALLTYPE UpdateConfig(
                /* [in][要更改的配置] */const MString& key,
                /* [in][要更改的配置] */const MString& value
        ) override;

        void STDMETHODCALLTYPE Init(
                /* [in] */const MString& root
        ) override;

        /// <summary>
        /// 解析类容
        /// </summary>
        MBoolean STDMETHODCALLTYPE AnalysisReg() override;

        /// <summary>
        /// 查询内容
        /// </summary>
        IMRegeditHandle STDMETHODCALLTYPE SelectReg(
                /* [in] */MUChar* cs,
                /* [in] */MInt len,
                /* [in][not use] */MBoolean temp
        ) override;

        /// <summary>
        /// 更改内容
        /// </summary>
        MBoolean STDMETHODCALLTYPE UpdateReg(
                /* [in] */MUChar* cs,
                /* [in] */MInt len,
                /* [in] */MBoolean temp
        ) override;

        /// <summary>
        /// 追加内容
        /// </summary>
        MBoolean STDMETHODCALLTYPE AppendReg(
                /* [in] */MUChar* cs,
                /* [in] */MInt len,
                /* [in] */MBoolean temp
        ) override;

        /// <summary>
        /// 删除内容
        /// </summary>
        MBoolean STDMETHODCALLTYPE DeleteReg(
                /* [in] */MUChar* cs,
                /* [in] */MInt len,
                /* [in] */MBoolean temp
        ) override;

        IMRegeditHandle STDMETHODCALLTYPE CheckOutData(
                /* [in] */const MUChar* name,
                /* [in] */int len
        ) override;

        IMRegeditHandle STDMETHODCALLTYPE CheckOutData(
                /* [in] */SIID siid
        ) override;

    private:
        VectorPtr<IMRegeditHandle> m_regedit;
        MString m_rootPath;
    };
} // MWD::Service::Regedit
