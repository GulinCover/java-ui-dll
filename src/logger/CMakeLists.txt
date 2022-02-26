message("im_service_logger---${PROJECT_SOURCE_DIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

add_library(im_service_logger SHARED
        ../IMCommon.cpp
        IMLoggerService.cpp
        IMLoggerServiceDll.cpp)