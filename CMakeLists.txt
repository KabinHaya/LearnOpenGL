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

project (LearningOpenGL)

# 将资源路径通过宏定义进行传递
set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)

# 设置include路径
set(MYLIB_INCLUDE_DIR 
    ${CMAKE_SOURCE_DIR}/third_party/include 
    ${CMAKE_SOURCE_DIR}/third_party/imgui_backends
)
include_directories(${MYLIB_INCLUDE_DIR})

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)
find_package(glm REQUIRED)
find_package(glad REQUIRED)
find_package(imgui REQUIRED)

# 设置 imgui 后端源文件
set(IMGUI_BACKEND_SOURCES
    ${CMAKE_SOURCE_DIR}/third_party/imgui_backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/third_party/imgui_backends/imgui_impl_opengl3.cpp
)

add_subdirectory (src)