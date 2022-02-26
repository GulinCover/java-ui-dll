#pragma once
/********************************************************
 c++标准库
*********************************************************/
#include <string>
#include <memory>
#include <list>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <share.h>
#include <ctime>
#include <thread>
#include <fstream>
#include <mutex>
#include <functional>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <algorithm>
#include <direct.h>
#include <iomanip>
#include <random>

/********************************************************
 windows库
*********************************************************/
#include <d2d1_3.h>
#pragma comment(lib, "d2d1.lib")
#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")
#include <dwrite_3.h>
#pragma comment(lib, "dwrite.lib")
#include <msctf.h>
#include <atlcomcli.h>
#include <comutil.h>
#include <utility>

/******************************************************
base type define
******************************************************/

#define MInt int
#define MUInt unsigned int
#define MChar char
#define MWChar wchar_t
#define MUChar unsigned char
#define MByte unsigned char
#define MUByte unsignel byte
#define MBoolean bool
#define MLong long
#define MULong unsigned long
#define MLLong long long
#define MULLong unsigned long long
#define MFloat float
#define MDouble double
#define MUFloat unsigned float
#define MUDouble unsigned double
#define MString std::string
#define MWString std::wstring

template<class T>
using List = std::list<T>;
template<class T>
using Vector = std::vector<T>;
template<class K, class V>
using Map = std::map<K, V>;

template<class K>
using Set = std::set<K>;

#define WINDOWS_OS

#ifdef WINDOWS_OS

#include <Windows.h>

#endif // WINDOWS_OS

namespace MWD::GM {

    /********************************************************
     获取当前线程ID
    *********************************************************/
    MULLong GMGetCurrentThreadId() {
        MULLong ret = -1;

#ifdef WINDOWS_OS
        ret = GetCurrentThreadId();
#elif MAC_OS
#elif LINUX_OS
#endif

        return ret;
    }

    /********************************************************
     获取当前进程ID
    *********************************************************/
    MULLong GMGetCurrentProcessId() {
        MULLong ret = -1;

#ifdef WINDOWS_OS
        ret = GetCurrentProcessId();
#elif MAC_OS
#elif LINUX_OS
#endif

        return ret;
    }

    /******************************************************
    根接口类
    ******************************************************/
    class IMInterface {
    };

    class IMOrder {
    public:
        virtual int GetOrder() {
            return 0;
        }
    };

    /******************************************************
    单例类
    ******************************************************/
    template<class T, class X = void, int N = 0>
    class IMSingleton {
    public:
        T* GetInstance() {
            static T v;
            return &v;
        }
    };

    /******************************************************
    单例指针类
    ******************************************************/
    template<class T, class X = void, int N = 0>
    static std::shared_ptr<T> g_v = nullptr;

    template<class T, class X = void, int N = 0>
    class IMSingletonPtr {
    public:
        static std::shared_ptr<T> GetInstance() {
            if (!g_v<T>) {
                g_v<T> = std::shared_ptr<T>(new T);
            }
            return g_v<T>;
        }

        static void SetInstance(std::shared_ptr<T> ptr) {
            g_v<T> = std::move(ptr);
        }
    };

    /********************************************************
     工具类
    *********************************************************/
    class IMTools {
    public:
        /// <summary>
        /// 字符串分割
        /// </summary>
        static std::shared_ptr<Vector<MString>> StringSplit(const MString& str, const MChar separator) {
            std::shared_ptr<Vector<MString>> vec = std::make_shared<Vector<MString>>();
            MInt pos = 0;

            for (MInt i = 0; i < str.length(); ++i) {
                if (str[i] == separator) {
                    vec->push_back(str.substr(pos, i - pos));
                    pos = i + 1;
                    continue;
                }

                if (i == str.length() - 1) {
                    vec->push_back(str.substr(pos, i - pos + 1));
                }
            }

            return vec;
        }

        /// <summary>
        /// gbk2utf8
        /// </summary>
        static MString GBKToUTF8(const MString& strGBK) {
            MString strOutUTF8;
            int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, nullptr, 0);

            WCHAR* str1 = new WCHAR[n + 1];

            MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n + 1);
            str1[n] = '\0';

            n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, nullptr, 0, nullptr, NULL);

            char* str2 = new char[n + 1];

            WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, nullptr, nullptr);
            str2[n] = '\0';

            strOutUTF8 = str2;

            delete[] str1;
            delete[] str2;

            return strOutUTF8;
        }

        /// <summary>
        /// gbk2utf16
        /// </summary>
        static MWString GBKToUTF16(const MChar* strGBK) {
            MWString strOutUTF16;
            int n = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, nullptr, 0);

            MWChar* str = new MWChar[n + 1];

            MultiByteToWideChar(CP_ACP, 0, strGBK, -1, str, n + 1);
            str[n] = '\0';

            strOutUTF16.append(str);

            delete[] str;

            return strOutUTF16;
        }
    };

    /********************************************************
     工具
    *********************************************************/
#ifdef WINDOWS_OS

    //void WideToMulti(wchar_t* w, char* c) {
    //    int len = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
    //    if (len) {
    //        WideCharToMultiByte(CP_UTF8, 0, w, -1, c, len + 1, nullptr, nullptr);
    //        c[len] = '\0';
    //    }
    //}

    //void MultiToWide(char* c, wchar_t* w) {
    //    int len = MultiByteToWideChar(CP_UTF8, 0, c, -1, nullptr, 0);
    //    if (len) {
    //        MultiByteToWideChar(CP_UTF8, 0, c, -1, w, len + 1);
    //        w[len] = '\0';
    //    }
    //}

#endif
} // namespace MWD::GM
