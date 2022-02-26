message("im_service_timer---${PROJECT_SOURCE_DIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

add_library(im_service_timer SHARED
        ../IMCommon.cpp
        IMTimerServiceDll.cpp
        IMTimerService.cpp)