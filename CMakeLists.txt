cmake_minimum_required(VERSION 3.17)
cmake_policy(SET CMP0048 NEW)
project(MWD_UI VERSION "1.0.0.0")

set(CMAKE_CXX_STANDARD 20)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# 头文件目录
include_directories(${PROJECT_SOURCE_DIR}/inc)
# 指定源码目录
aux_source_directory(${PROJECT_SOURCE_DIR}/src src_dirs)
# 指定库目录
link_directories(${PROJECT_SOURCE_DIR}/lib)
# 指定编译子目录
add_subdirectory(./src)

# 指定可执行文件目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
# 指定可执行文件名及源码
#add_executable(MWD_UI ${src_dirs})

target_link_libraries(core)
target_link_libraries(im_service_render)
target_link_libraries(im_service_regedit)
target_link_libraries(im_service_timer)
target_link_libraries(im_service_logger)
target_link_libraries(im_module_background_render_item)
target_link_libraries(im_module_input_render_item)