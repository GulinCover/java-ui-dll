message("core---${PROJECT_SOURCE_DIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

add_library(core SHARED
        IMCoreDll.cpp
        IMCore.cpp
        ../IMFile.cpp
        ../IMCommon.cpp)