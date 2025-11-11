# 将源代码添加到此项目的可执行文件。
set(TARGET_NAME 4_09_RenderingNormal)

add_executable (${TARGET_NAME}
    RenderingNormal.cpp
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${TARGET_NAME}
    PROPERTY
        CXX_STANDARD 20
  )
endif()

target_include_directories(${TARGET_NAME} PRIVATE 
    ${MYLIB_INCLUDE_DIR}
)
target_link_libraries(${TARGET_NAME}
    ${GLFW_LIBRARY}
    ${ASSIMP_LIBRARY}
    ${ZLIB_LIBRARY}
    glad
    imgui
)

# 将目录路径作为编译时宏传递
target_compile_definitions(${TARGET_NAME} PRIVATE
    ASSETS_DIR="${ASSETS_DIR}"
    SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shader"
)
