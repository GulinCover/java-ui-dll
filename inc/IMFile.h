#pragma once

#include "IMCommon.h"

namespace MWD::File {
    //////////////////////////////////////////////////
    // 配置文件解析结果集
    interface IMFileStruct {
    public:
        using ptr = std::shared_ptr<IMFileStruct>;
    public:
        /// <summary>
        /// 单值获取
        /// </summary>
        virtual MString STDMETHODCALLTYPE RequireValue(
                /* [in][key] */const MString& key
        ) PURE;

        /// <summary>
        /// 数组获取
        /// </summary>
        virtual VectorPtr<MString> STDMETHODCALLTYPE RequireValues(
                /* [in][key] */const MString& key
        ) PURE;
    };

    //////////////////////////////////////////////////
    // 配置文件解析结果集Impl
    class MFileStructImpl : public IMFileStruct {
    public:
        using ptr = std::shared_ptr<IMFileStruct>;
    public:
        /// <summary>
        /// 单值获取
        /// </summary>
        MString STDMETHODCALLTYPE RequireValue(
                /* [in][key] */const MString& key
        ) override;

        /// <summary>
        /// 数组获取
        /// </summary>
        VectorPtr<MString> STDMETHODCALLTYPE RequireValues(
                /* [in][key] */const MString& key
        ) override;
    };


    //////////////////////////////////////////////////
    // 文件相关操作
    interface IMFile {
    public:
        using ptr = std::shared_ptr<IMFile>;
    public:
        /// <summary>
        /// 判断目录是否存在
        /// </summary>
        virtual MBoolean STDMETHODCALLTYPE IsDirExist(
                /* [in] */const MString& path
        ) PURE;

        /// <summary>
        /// 判断文件是否存在
        /// </summary>
        virtual MBoolean STDMETHODCALLTYPE IsFileExist(
                /* [in] */const MString& path
        ) PURE;

        /// <summary>
        /// 获取目录下所有一级文件名
        /// </summary>
        virtual VectorPtr<MString> STDMETHODCALLTYPE RequireDirFiles(
                /* [in] */const MString& path
        ) PURE;

        /// <summary>
        /// 获取当前运行根路径
        /// </summary>
        virtual MString STDMETHODCALLTYPE RequireCurrentExecRootPath(
                /* [in][当前句柄] */HMODULE hmodule
        ) PURE;

        /// <summary>
        /// .properties文件解析
        /// </summary>
        virtual IMFileStruct::ptr STDMETHODCALLTYPE AnalysisPropertiesFile(
                /* [in][文件路径] */const MString& file
        ) PURE;

        /// <summary>
        /// .yml文件解析
        /// </summary>
        virtual IMFileStruct::ptr STDMETHODCALLTYPE AnalysisYmlFile(
                /* [in][文件路径] */const MString& file
        ) PURE;

        /// <summary>
        /// .xml文件解析
        /// </summary>
        virtual IMFileStruct::ptr STDMETHODCALLTYPE AnalysisXmlFile(
                /* [in][文件路径] */const MString& file
        ) PURE;
    };

    //////////////////////////////////////////////////
    // 文件相关操作Impl
    class MDefaultFileImpl : public IMFile {
    public:
        using ptr = std::shared_ptr<MDefaultFileImpl>;
    public:
        /// <summary>
        /// 判断目录是否存在
        /// </summary>
        MBoolean STDMETHODCALLTYPE IsDirExist(
                /* [in] */const MString& path
        ) override;

        /// <summary>
        /// 判断文件是否存在
        /// </summary>
        MBoolean STDMETHODCALLTYPE IsFileExist(
                /* [in] */const MString& path
        ) override;

        /// <summary>
        /// 获取目录下所有一级文件名
        /// </summary>
        VectorPtr<MString> STDMETHODCALLTYPE RequireDirFiles(
                /* [in] */const MString& path
        ) override;

        /// <summary>
        /// 获取当前运行根路径
        /// </summary>
        MString STDMETHODCALLTYPE RequireCurrentExecRootPath(
                /* [in][当前句柄] */HMODULE hmodule
        ) override;

        /// <summary>
        /// .properties文件解析
        /// </summary>
        IMFileStruct::ptr STDMETHODCALLTYPE AnalysisPropertiesFile(
                /* [in][文件路径] */const MString& file
        ) override;

        /// <summary>
        /// .yml文件解析
        /// </summary>
        IMFileStruct::ptr STDMETHODCALLTYPE AnalysisYmlFile(
                /* [in][文件路径] */const MString& file
        ) override;

        /// <summary>
        /// .xml文件解析
        /// </summary>
        IMFileStruct::ptr STDMETHODCALLTYPE AnalysisXmlFile(
                /* [in][文件路径] */const MString& file
        ) override;
    };
} // MWD::File