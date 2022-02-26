message("im_service_render---${PROJECT_SOURCE_DIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

add_library(im_service_render SHARED
        IMRenderQueueServiceDll.cpp
        IMRenderQueueService.cpp ../inputRenderItem/IMWriter.cpp)