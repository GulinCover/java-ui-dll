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
#include <tuple>
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
#include <iomanip>
#include <random>
#include <filesystem>
#include <fstream>

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
#include <direct.h>
#include <io.h>
#include <winerror.h>


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
#undef interface
#define interface class
#define abstract class
#define S_FAILED -1
template<class T>
using List = std::list<T>;
template<class T>
using Vector = std::vector<T>;
template<class K, class V>
using Map = std::map<K, V>;
template<class K, class...V>
using Tuple = std::tuple<K, V...>;
template<class K>
using Set = std::set<K>;
template<class T>
using ListPtr = std::shared_ptr<std::list<T>>;
template<class T>
using VectorPtr = std::shared_ptr<std::vector<T>>;
template<class K, class V>
using MapPtr = std::shared_ptr<std::map<K, V>>;
template<class K, class...V>
using TuplePtr = std::shared_ptr<std::tuple<K, V...>>;
template<class K>
using SetPtr = std::shared_ptr<Set<K>>;

#define MakeShared(C) std::make_shared<C>();
#define MakeMap(K, V) std::make_shared<Map<K, V>>();
#define MakeTuple(K, ARGS, ...) std::make_shared<Tuple<K, ##ARGS>>();
#define MakeVec(C) std::make_shared<Vector<C>>();
#define MakePair(K, V) std::make_pair(K, V)

#define ACCESS(EXPRESSION) \
    if (!EXPRESSION) {      \
       throw std::invalid_argument("Expression is not valid");\
    };

/********************************************************
 获取当前进程ID
*********************************************************/
MULLong GMGetCurrentProcessId();