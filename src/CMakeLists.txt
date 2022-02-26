# 添加编译子目录
add_subdirectory(./core)
add_subdirectory(./logger)
add_subdirectory(./render)
add_subdirectory(./regedit)
add_subdirectory(./timer)
add_subdirectory(./backgroundRenderItem)
add_subdirectory(./inputRenderItem)

add_executable(Test TestMain.cpp IMCommon.cpp)