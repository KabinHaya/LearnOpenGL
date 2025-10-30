# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT 
    "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>"
)
endif()

if(MSVC)
    add_compile_options(/utf-8) # MSVC使用utf8字符集
endif()

project ("LearningOpenGL")

set(MYLIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/include)
include_directories(${MYLIB_INCLUDE_DIR})

find_library(MYLIB_LIBRARY
    NAMES 
        glfw3
        assimp-vc143-mtd
    PATHS 
        set(LIB_DIR ${CMAKE_SOURCE_DIR}/third_party/lib)
        
    NO_DEFAULT_PATH
)

add_subdirectory (sources)
add_subdirectory (third_party)