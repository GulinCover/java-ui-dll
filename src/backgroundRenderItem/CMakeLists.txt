message("im_module_background_render_item---${PROJECT_SOURCE_DIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

add_library(im_module_background_render_item SHARED
        IMBackgroundRenderItemModuleDll.cpp
        IMBackgroundRenderItemModule.cpp
        ../IMCommon.cpp)