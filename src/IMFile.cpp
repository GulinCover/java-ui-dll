#include "IMFile.h"

namespace MWD::File {
    //////////////////////////////////////////////////
    // 配置文件解析结果集Impl
    /********************************************************
     MFileStruct方法实现
    *********************************************************/
    /// <summary>
    /// 单值获取
    /// </summary>
    MString STDMETHODCALLTYPE MFileStructImpl::RequireValue(
            /* [in][key] */const MString& key
    ) {
        return "";
    }

    /// <summary>
    /// 数组获取
    /// </summary>
    VectorPtr<MString> STDMETHODCALLTYPE MFileStructImpl::RequireValues(
            /* [in][key] */const MString& key
    ) {
        return MakeVec(MString);
    }

    //////////////////////////////////////////////////
    // 文件相关操作Impl
    /********************************************************
     IMFile方法实现
    *********************************************************/
    /// <summary>
    /// 判断目录是否存在
    /// </summary>
    MBoolean STDMETHODCALLTYPE MDefaultFileImpl::IsDirExist(
            /* [in] */const MString& path
    ) {
        return std::filesystem::exists(path) && std::filesystem::is_directory(path);
    }

    /// <summary>
    /// 判断文件是否存在
    /// </summary>
    MBoolean STDMETHODCALLTYPE MDefaultFileImpl::IsFileExist(
            /* [in] */const MString& path
    ) {
        return std::filesystem::exists(path) && !std::filesystem::is_directory(path);
    }

    /// <summary>
    /// 获取目录下所有一级文件名
    /// </summary>
    VectorPtr<MString> STDMETHODCALLTYPE MDefaultFileImpl::RequireDirFiles(
            /* [in] */const MString& path
    ) {
        VectorPtr<MString> vec = MakeShared(Vector<MString>);

        _finddata64i32_t fileInfo{};
        MString fullPath = path;
        MLong h;
        h = _findfirst64i32(fullPath.append("\\*").c_str(), &fileInfo);

        if (h != -1) {
            do {
                // 如果是目录则获取文件夹下所有文件名
                if (strcmp(fileInfo.name, ".") != 0 && strcmp(fileInfo.name, "..") != 0) {
                    vec->push_back(fileInfo.name);
                }
            } while (_findnext64i32(h, &fileInfo) == 0);
            _findclose(h);
        }
        return vec;
    }

    /// <summary>
    /// 获取当前运行根路径
    /// </summary>
    MString STDMETHODCALLTYPE MDefaultFileImpl::RequireCurrentExecRootPath(HMODULE hmodule) {
        MChar rootPath[MAX_PATH];
        MString path;

        hmodule ? GetModuleFileNameA(hmodule, rootPath, MAX_PATH) : GetModuleFileNameA(nullptr, rootPath, MAX_PATH);

        path = rootPath;
        MInt pos = path.find_last_of('\\', path.length());
        ACCESS((pos != -1));

        path = path.substr(0, pos - 4);
        if (!IsDirExist(path)) {
            throw std::invalid_argument("Project path error, "
                                        "please restore the original "
                                        "project all file paths to ensure "
                                        "correct operation");
        }
        return path;
    }

    /// <summary>
    /// .properties文件解析
    /// </summary>
    IMFileStruct::ptr STDMETHODCALLTYPE MDefaultFileImpl::AnalysisPropertiesFile(
            /* [in][文件路径] */const MString& file
    ) {
        IMFileStruct::ptr result = MakeShared(MFileStructImpl);

        return result;
    }

    /// <summary>
    /// .yml文件解析
    /// </summary>
    IMFileStruct::ptr STDMETHODCALLTYPE MDefaultFileImpl::AnalysisYmlFile(
            /* [in][文件路径] */const MString& file
    ) {
        IMFileStruct::ptr result = MakeShared(MFileStructImpl);

        return result;
    }

    /// <summary>
    /// .xml文件解析
    /// </summary>
    IMFileStruct::ptr STDMETHODCALLTYPE MDefaultFileImpl::AnalysisXmlFile(
            /* [in][文件路径] */const MString& file
    ) {
        IMFileStruct::ptr result = MakeShared(MFileStructImpl);

        return result;
    }

} // MWD::File
