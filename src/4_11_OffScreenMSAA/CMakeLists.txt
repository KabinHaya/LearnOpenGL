# 将源代码添加到此项目的可执行文件。
set(TARGET_NAME 4_11_OffScreenMSAA)

add_executable (${TARGET_NAME}
    ${IMGUI_BACKEND_SOURCES}
    OffScreenMSAA.cpp
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${TARGET_NAME}
    PROPERTY
        CXX_STANDARD 20
  )
endif()


target_link_libraries(${TARGET_NAME}
    opengl::opengl  # 系统 OpenGL 库
    glfw            # GLFW 库
    assimp::assimp  # Assimp 库
    glm::glm        # GLM 头文件库
    glad::glad      # GLAD 库
    imgui::imgui    # ImGui 库
)

# 将目录路径作为编译时宏传递
target_compile_definitions(${TARGET_NAME} PRIVATE
    ASSETS_DIR="${ASSETS_DIR}"
    SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shader"
)
