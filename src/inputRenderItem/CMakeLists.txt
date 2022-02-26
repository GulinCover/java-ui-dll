message("im_module_input_render_item---${PROJECT_SOURCE_DIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

add_library(im_module_input_render_item SHARED
        IMInputRenderItemModule.cpp
        IMInputRenderItemModuleDll.cpp
        IMWriter.cpp
        ../IMCommon.cpp)