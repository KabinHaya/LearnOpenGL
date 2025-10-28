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

include_directories(${CMAKE_SOURCE_DIR}/ThirdParty/include)
set(MYLIB_INCLUDE_DIR 
    ${CMAKE_SOURCE_DIR}/ThirdParty/include
    CACHE PATH 
        "Header directory of OpenGL"
)

find_library(MYLIB_LIBRARY
    NAMES 
        glfw3
        assimp-vc143-mtd
    PATHS 
        set(LIB_DIR ${CMAKE_SOURCE_DIR}/ThirdParty/lib)
        
    NO_DEFAULT_PATH
)

set(MYLIB_LIBRARY 
    ${MYLIB_LIBRARY} 
    CACHE FILEPATH 
        "Path to OpenGL" 
    FORCE
)

file(GLOB GLAD_SRC
    "${CMAKE_SOURCE_DIR}/ThirdParty/src/glad/glad.c"
)

# TODO: 这里应该需要修改，可编译成库
file(GLOB IMGUI_SRC
    "${CMAKE_SOURCE_DIR}/ThirdParty/include/imgui/*.cpp"
)

# 包含子项目。
add_subdirectory (01_HelloWindow)
add_subdirectory (02_HelloTriangle)
add_subdirectory (03_HelloTriangleExercise)
add_subdirectory (04_ShaderGLSL)
add_subdirectory (05_ShaderClass)
add_subdirectory (06_GLSLExercise)
add_subdirectory (07_LoadTexture)
add_subdirectory (08_LoadTextureExercise)
add_subdirectory (09_Transform)
add_subdirectory (10_UsePlaneGeometry)
add_subdirectory (11_UseSphereGeometry)
add_subdirectory (12_UseBoxGeometry)
add_subdirectory (13_ModelViewProjection)
add_subdirectory (14_UseImageUI)
add_subdirectory (15_MvpMatrixExercise)