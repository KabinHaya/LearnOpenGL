# 将源代码添加到此项目的可执行文件。
set(TARGET_NAME 1_05_ShaderGLSL)

add_executable (${TARGET_NAME}
    ${IMGUI_BACKEND_SOURCES}
    main1.cpp
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)
endif()


target_link_libraries(${TARGET_NAME}
    opengl::opengl  # 系统 OpenGL 库
    glfw            # GLFW 库
    assimp::assimp  # Assimp 库
    glm::glm        # GLM 头文件库
    glad::glad      # GLAD 库
    imgui::imgui    # ImGui 库
)