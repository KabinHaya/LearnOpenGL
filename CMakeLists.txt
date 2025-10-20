# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("LearningOpenGL")

include_directories(${CMAKE_SOURCE_DIR}/include)
set(MYLIB_INCLUDE_DIR 
    ${CMAKE_SOURCE_DIR}/include
    CACHE PATH 
        "Header directory of OpenGL"
)

find_library(MYLIB_LIBRARY
    NAMES 
        glfw3
        assimp-vc143-mtd
    PATHS 
        set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib)
        
    NO_DEFAULT_PATH
)

set(MYLIB_LIBRARY 
    ${MYLIB_LIBRARY} 
    CACHE FILEPATH 
        "Path to OpenGL" 
    FORCE
)
set(GLAD_SRC_PATH 
    ${CMAKE_SOURCE_DIR}/glad.c
)
# 包含子项目。
add_subdirectory (01.HelloWindow)
add_subdirectory (02.HelloTriangle)
add_subdirectory (03.HelloTriangleExercise)
add_subdirectory (04.ShaderGLSL)
add_subdirectory (05.ShaderClass)