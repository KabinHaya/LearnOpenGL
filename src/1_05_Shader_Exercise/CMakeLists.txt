# 将源代码添加到此项目的可执行文件。
set(TARGET_NAME 1_05_Shader_Exercise)

add_executable (${TARGET_NAME}
    ${IMGUI_BACKEND_SOURCES}
    main.cpp
 )

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${TARGET_NAME}  PROPERTY CXX_STANDARD 20)
endif()

target_include_directories(${TARGET_NAME} PRIVATE 
    ${MYLIB_INCLUDE_DIR}
)
target_link_libraries(${TARGET_NAME}
    OpenGL::GL      # 系统 OpenGL 库
    glfw            # GLFW 库
    assimp::assimp  # Assimp 库
    glm::glm        # GLM 头文件库
    glad::glad      # GLAD 库
    imgui::imgui    # ImGui 库
)

# 将shader文件复制到exe文件的所在目录
set(RESOURCES_SRC ${CMAKE_CURRENT_SOURCE_DIR}/shader)
set(RESOURCES_DST ${CMAKE_CURRENT_BINARY_DIR}/shader)

file(GLOB_RECURSE SHADER_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/shader/*.glsl"
)


add_custom_target(06_CopyResources ALL
    COMMAND 
        ${CMAKE_COMMAND} -E copy_directory "${RESOURCES_SRC}" "${RESOURCES_DST}"
    DEPENDS 
        ${SHADER_FILES}
    BYPRODUCTS 
        "${RESOURCES_DST}"
)
# message(STATUS "${TARGET_NAME}: Shader resources will be copied to: ${RESOURCES_DST}")
add_dependencies(${TARGET_NAME} 
    06_CopyResources
)