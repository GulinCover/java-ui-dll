#include "IMRegeditService.h"

namespace MWD::Service::Regedit {
    //////////////////////////////////////////////////
    // 注册表配置服务
    SIID SIID_IM_SERVICE_REGEDIT{
            {0, 0, 0, 0, 0, 0, 0, 1},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}
    };
    /********************************************************
     IMService方法实现
    *********************************************************/
    /// <summary>
    /// 是否可转
    /// </summary>
    MBoolean STDMETHODCALLTYPE MRegeditService::QueryInterface(
            /* [in] */SIID siid
    ) {
        return SIID_IM_SERVICE_REGEDIT == siid;
    }

    /// <summary>
    /// 获取siid
    /// </summary>
    SIID STDMETHODCALLTYPE MRegeditService::GetSIID(
            /* [in] */char Data2[16]
    ) {
        for (int i = 0; i < sizeof(SIID_IM_SERVICE_REGEDIT.Data2); ++i) {
            if (Data2[i] != SIID_IM_SERVICE_REGEDIT.Data2[i]) {
                return SIID_SERVICE_INVALID;
            }
        }

        return SIID_IM_SERVICE_REGEDIT;
    }

    /********************************************************
     IMRegedit方法实现
    *********************************************************/
    /// <summary>
    /// 更改当前日志配置
    /// </summary>
    void STDMETHODCALLTYPE MRegeditService::UpdateConfig(
            /* [in][要更改的配置] */const MString& key,
            /* [in][要更改的配置] */const MString& value
    ) {

    }

    void STDMETHODCALLTYPE MRegeditService::Init(const MString& root) {
        m_regedit = MakeVec(IMRegeditHandle);

        m_rootPath = root;
    }

    /// <summary>
    /// 解析类容
    /// </summary>
    MBoolean STDMETHODCALLTYPE MRegeditService::AnalysisReg() {
        std::ifstream ifs;

        ifs.open(m_rootPath + "\\regedit.reg", std::ios::binary);
        if (ifs.is_open()) {
            while (!ifs.eof()) {
                IMRegeditHandle handle{};
                ifs.read((MChar*) &handle, sizeof(IMRegeditHandle));
                m_regedit->emplace_back(handle);
            }

            ifs.close();
        } else {
            throw std::invalid_argument("The regedit file does not exist");
        }

        return true;
    }

    /// <summary>
    /// 查询内容
    /// </summary>
    IMRegeditHandle STDMETHODCALLTYPE MRegeditService::SelectReg(
            /* [in] */MUChar* cs,
            /* [in] */MInt len,
            /* [in] */MBoolean temp
    ) {
        if (len == sizeof(IMRegeditHandle::Data1)) {
            MInt ret = 0;
            for (IMRegeditHandle& regeditHandle : *m_regedit) {
                ret = 0;
                for (MInt i = 0; i < sizeof(regeditHandle.Data1); ++i) {
                    if (cs[i] == regeditHandle.Data1[i]) {
                        ++ret;
                    } else {
                        break;
                    }
                }

                if (ret == len)
                    return regeditHandle;
            }
        } else if (len == sizeof(IMRegeditHandle::Data2)) {
            MInt ret = 0;
            for (IMRegeditHandle& regeditHandle : *m_regedit) {
                ret = 0;
                for (MInt i = 0; i < sizeof(regeditHandle.Data2); ++i) {
                    if (cs[i] == regeditHandle.Data2[i]) {
                        ++ret;
                    } else {
                        break;
                    }
                }

                if (ret == len)
                    return regeditHandle;
            }
        }

        return IM_INVALID_REGEDIT;
    }

    /// <summary>
    /// 更改内容
    /// </summary>
    MBoolean STDMETHODCALLTYPE MRegeditService::UpdateReg(
            /* [in] */MUChar* cs,
            /* [in] */MInt len,
            /* [in] */MBoolean temp
    ) {
        IMRegeditHandle handle = SelectReg(cs, len, temp);
        if (handle != IM_INVALID_REGEDIT) {
            switch (len) {
                case sizeof(IMRegeditHandle::Data2):
                    memcpy(handle.Data2, cs, sizeof(IMRegeditHandle::Data2));
                    break;
                case sizeof(IMRegeditHandle::Data3):
                    memcpy(handle.Data3, cs, sizeof(IMRegeditHandle::Data3));
                    break;
                case sizeof(IMRegeditHandle::Data4):
                    memcpy(handle.Data4, cs, sizeof(IMRegeditHandle::Data4));
                    break;
                default:
                    return false;
            }

            MInt line = -1;
            for (IMRegeditHandle& regeditHandle : *m_regedit) {
                ++line;
                MInt ret = 0;
                for (MInt i = 0; i < sizeof(IMRegeditHandle::Data1); ++i) {
                    if (regeditHandle.Data1[i] == handle.Data1[i]) {
                        ++ret;
                    } else {
                        break;
                    }
                }

                if (ret == sizeof(IMRegeditHandle::Data1)) {
                    memcpy(regeditHandle.Data1, handle.Data1, ret);
                }
            }

            // 真实写入
            if (temp) {
                std::ofstream ofs;
                ofs.open(m_rootPath + "\\regedit.reg", std::ios::binary);

                if (ofs.is_open()) {
                    ofs.seekp(line * sizeof(IMRegeditHandle), std::ios::beg);
                    auto* content = new MChar[sizeof(IMRegeditHandle)];
                    memcpy(content, &handle, sizeof(IMRegeditHandle));
                    ofs.write(content, sizeof(IMRegeditHandle));
                    delete[] content;

                    ofs.close();

                } else {
                    return false;
                }
            }

            return true;
        }

        return false;
    }

    /// <summary>
    /// 追加内容
    /// </summary>
    MBoolean STDMETHODCALLTYPE MRegeditService::AppendReg(
            /* [in] */MUChar* cs,
            /* [in] */MInt len,
            /* [in] */MBoolean temp
    ) {
        if (len == sizeof(IMRegeditHandle)) {
            MULong max = 0;
            auto* Data2 = new MUChar[sizeof(IMRegeditHandle::Data2)];
            memcpy(Data2, cs + sizeof(IMRegeditHandle::Data1), sizeof(IMRegeditHandle::Data2));

            for (IMRegeditHandle& regeditHandle : *m_regedit) {
                MInt ret = 0;
                for (MInt i = 0; i < sizeof(IMRegeditHandle::Data2); ++i) {
                    if (Data2[i] == regeditHandle.Data2[i]) {
                        ++ret;
                    }
                }

                auto* min = reinterpret_cast<MULong*>(&(regeditHandle.Data1));
                if (ret == sizeof(IMRegeditHandle::Data2))
                    return true;

                if (max < *min)
                    max = *min;
            }

            ++max;
            IMRegeditHandle tempHandle{};
            memcpy(tempHandle.Data1, &max, sizeof(IMRegeditHandle::Data1));
            memcpy(tempHandle.Data2, cs, sizeof(IMRegeditHandle::Data2));
            memcpy(tempHandle.Data3, cs + sizeof(IMRegeditHandle::Data2), sizeof(IMRegeditHandle::Data3));
            memcpy(tempHandle.Data4, cs + sizeof(IMRegeditHandle::Data3), sizeof(IMRegeditHandle::Data4));
            m_regedit->push_back(tempHandle);

            delete[] Data2;

            if (temp) {
                std::ofstream ofs;
                ofs.open(m_rootPath + "\\regedit.reg", std::ios::binary | std::ios::app);

                if (ofs.is_open()) {
                    auto* css = new MChar[sizeof(IMRegeditHandle)];
                    memcpy(css, &tempHandle, sizeof(IMRegeditHandle));
                    ofs.write(css, sizeof(IMRegeditHandle));
                    delete[] css;

                    ofs.close();
                }
            }

            return true;
        }

        return false;
    }

    /// <summary>
    /// 删除内容
    /// </summary>
    MBoolean STDMETHODCALLTYPE MRegeditService::DeleteReg(
            /* [in] */MUChar* cs,
            /* [in] */MInt len,
            /* [in] */MBoolean temp
    ) {
        IMRegeditHandle handle = SelectReg(cs, len, temp);

        if (handle != IM_INVALID_REGEDIT) {
            for (auto iter = m_regedit->begin(); iter != m_regedit->end(); ++iter) {
                if (*iter == handle) {
                    m_regedit->erase(iter);
                    break;
                }
            }

            if (temp) {
                std::ofstream ofs;
                ofs.open(m_rootPath + "\\regedit.reg", std::ios::binary);
                if (ofs.is_open()) {
                    ofs.clear();
                    ofs.flush();

                    auto* css = new MUChar[sizeof(IMRegeditHandle)];
                    for (IMRegeditHandle& regeditHandle : *m_regedit) {
                        memcpy(css, &regeditHandle, sizeof(IMRegeditHandle));
                        ofs << css;
                    }
                    delete[] css;
                    ofs.close();
                }
            }

            return true;
        }

        return false;
    }

    IMRegeditHandle STDMETHODCALLTYPE MRegeditService::CheckOutData(
            /* [in] */const MUChar* name,
            /* [in] */int len
    ) {
        for (IMRegeditHandle handle : *m_regedit) {
            MBoolean flag = true;

            if (len == sizeof(SIID::Data1)) {
                for (int i = 0; i < sizeof(SIID::Data1); ++i) {
                    if (name[i] != handle.Data2[i]) {
                        flag = false;
                        break;
                    }
                }
            } else {
                for (int i = 0; i < sizeof(SIID::Data2); ++i) {
                    if (name[i] != handle.Data2[i + sizeof(SIID::Data1)]) {
                        flag = false;
                        break;
                    }
                }
            }

            if (flag)
                return handle;
        }

        return IM_INVALID_REGEDIT;
    }

    IMRegeditHandle STDMETHODCALLTYPE MRegeditService::CheckOutData(
            /* [in] */SIID siid
    ) {
        for (IMRegeditHandle handle : *m_regedit) {
            MUChar cs[sizeof(SIID)];
            memset(cs, '\0', sizeof(SIID));
            memcpy(cs, &siid, sizeof(SIID));
            MBoolean flag = true;

            for (int i = 0; i < sizeof(SIID); ++i) {
                if (handle.Data2[i] != cs[i]) {
                    flag = false;
                    break;
                }
            }

            if (flag)
                return handle;
        }

        return IM_INVALID_REGEDIT;
    }
} // MWD::Service::Regedit

/// <summary>
/// regedit服务siid
/// </summary>
MWD::Service::SIID SIID_IM_REGEDIT_SERVICE{
        {0, 0, 0, 0, 0, 0, 0, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}
};

/// <summary>
/// dll函数定义
/// </summary>
GM_DEFINE_REGISTER_FUNC_AND_DESTROY(MWD::Service::Regedit::MRegeditService, SIID_IM_REGEDIT_SERVICE)


