message("im_service_regedit---${PROJECT_SOURCE_DIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

add_library(im_service_regedit SHARED
        ../IMCommon.cpp
        IMRegeditServiceDll.cpp
        IMRegeditService.cpp)