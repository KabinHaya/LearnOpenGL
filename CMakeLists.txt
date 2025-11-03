# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。

cmake_minimum_required (VERSION 3.12)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT 
    "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>"
)
endif()

# 设置MSVC的字符集
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/execution-charset:gbk>")

project ("LearningOpenGL")

# 将资源路径通过宏定义进行传递
set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)

# 设置include路径
set(MYLIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/include)
include_directories(${MYLIB_INCLUDE_DIR})

# 设置静态库路径
find_library(GLFW_LIBRARY
    NAMES
        glfw3
    PATHS
        ${CMAKE_SOURCE_DIR}/third_party/lib
    NO_DEFAULT_PATH
)

find_library(ASSIMP_LIBRARY
    NAMES
        assimp-vc145-mtd
    PATHS
        ${CMAKE_SOURCE_DIR}/third_party/lib
    NO_DEFAULT_PATH
)

find_library(ZLIB_LIBRARY
    NAMES
        zlib
    PATHS
        ${CMAKE_SOURCE_DIR}/third_party/lib
    NO_DEFAULT_PATH
)

if(NOT GLFW_LIBRARY)
    message(FATAL_ERROR "GLFW library not found")
endif()

if(NOT ASSIMP_LIBRARY)
    message(FATAL_ERROR "Assimp library not found")
endif()

if(NOT ZLIB_LIBRARY)
    message(FATAL_ERROR "ZLib library not found")
endif()

add_subdirectory (src)
add_subdirectory (third_party)