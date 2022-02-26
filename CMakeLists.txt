cmake_minimum_required(VERSION 3.17)
project(test_dll)

set(CMAKE_CXX_STANDARD 20)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_library(test_dll SHARED IMTimer.hpp IMConfig.hpp IMSwapChain.hpp IMNative.hpp IMRender.hpp IMCommon.hpp IMLoader.hpp IMFrame.hpp IMLogger.hpp WindowsLibrary.cpp WindowsLibrary.h WindowsNative.hpp IMonitor.hpp IMTextInput.hpp IMMainWindowEvent.hpp)