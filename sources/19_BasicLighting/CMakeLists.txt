# 将源代码添加到此项目的可执行文件。
set(PROJECT_NAME 19_BasicLighting)
set(COPY_RESOURCES 19_CopyResources)

add_executable (${PROJECT_NAME}
    main.cpp
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME}
    PROPERTY
        CXX_STANDARD 20
  )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${MYLIB_INCLUDE_DIR}

)
target_link_libraries(${PROJECT_NAME}
    ${MYLIB_LIBRARY}
    glad
    imgui
)

# 将shader文件复制到exe文件的所在目录
set(RESOURCES_SRC ${CMAKE_CURRENT_SOURCE_DIR}/shader)
set(RESOURCES_DST ${CMAKE_CURRENT_BINARY_DIR}/shader)


file(GLOB_RECURSE SHADER_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/shader/*.glsl"
)

add_custom_target(${COPY_RESOURCES} ALL
    COMMAND
        ${CMAKE_COMMAND} -E copy_directory "${RESOURCES_SRC}" "${RESOURCES_DST}"
    DEPENDS
        ${SHADER_FILES}
    BYPRODUCTS
        "${RESOURCES_DST}"
)
# message(STATUS "${PROJECT_NAME}: Shader resources will be copied to: ${RESOURCES_DST}")
add_dependencies(${PROJECT_NAME} 
    ${COPY_RESOURCES}
)